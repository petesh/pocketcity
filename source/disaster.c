/*! \file
 * \brief routines associated with disasters
 *
 * This module contains functions that are used to create disasters in the
 * simulation.
 */

#include <config.h>

#include <handler.h>
#include <drawing.h>
#include <zakdef.h>
#include <ui.h>
#include <globals.h>
#include <handler.h>
#include <build.h>
#include <disaster.h>
#include <simulation.h>

#if defined(PALM)
#include <StringMgr.h>
#include <unix_stdio.h>
#else
#include <stdio.h>
#endif

static void FireSpread(UInt16 x, UInt16 y) DISASTER_SECTION;
static void CreateWaste(UInt16 x, UInt16 y) DISASTER_SECTION;
static UInt16 GetDefenceValue(UInt16 xpos, UInt16 ypos) DISASTER_SECTION;
static UInt16 ContainsDefence(UInt16 x, UInt16 y) DISASTER_SECTION;
static void MonsterCheckSurrounded(UInt16 i) DISASTER_SECTION;
static void CreateMeteor(UInt16 x, UInt16 y, Int16 size) DISASTER_SECTION;

void
DoCommitmentNasties(void)
{
	int i;

	for (i = 0; i < 60 - getTax(); i++) {
		UInt32 loc = GetRandomNumber(MapMul());
		welem_t world = getWorld(GetRandomNumber(MapMul()));
		int x = loc % getMapWidth();
		int y = loc % getMapHeight();

		if ((IsTransport(world)) &&
		    (GetRandomNumber(100) > getUpkeep(ue_traffic)))
			Build_Destroy(x, y);
		if ((IsWaterPipe(world) || IsPowerLine(world) ||
			    IsPowerWater(world)) &&
		    (GetRandomNumber(100) > getUpkeep(ue_power)))
			Build_Destroy(x, y);
	}
}

void
DoNastyStuffTo(welem_t type, UInt16 probability, UInt8 purge)
{
	/* nasty stuff means: turn it into wasteland */
	UInt32 randomTile;
	UInt16 i, x, y;

	if (GetRandomNumber(probability) != 0)
		return;

	LockZone(lz_world);
	LockZone(lz_flags);
	for (i = 0; i < 50; i++) {
		randomTile = GetRandomNumber(MapMul());
		if (getWorld(randomTile) == type) {
			/* wee, let's destroy something */
			x = (UInt16)(randomTile % getMapWidth());
			y = (UInt16)(randomTile / getMapWidth());
			if (!purge)
				CreateWaste(x, y);
			else
				Build_Destroy(x, y);
			break;
		}
	}
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
}

void
DoRandomDisaster(void)
{
	UInt32 randomTile;
	Int16 i, x, y, type, random;
	UInt8 disaster_level;

	disaster_level = getDisasterLevel();
	/* for those who can't handle the game (truth?) */
	if (disaster_level == 0)
		return;

	LockZone(lz_world);
	LockZone(lz_flags);

	for (i = 0; i < 100; i++) { /* 100 tries to hit a useful tile */
		randomTile = GetRandomNumber(MapMul());
		type = getWorld(randomTile);
		if (type != Z_DIRT &&
			type != Z_REALWATER &&
			type != Z_CRATER) {
			x = (Int16)(randomTile % getMapWidth());
			y = (Int16)(randomTile / getMapHeight());
			/* TODO: should depend on difficulty */
			random = (Int16)GetRandomNumber(1000 / disaster_level);
			WriteLog("Random Disaster: %d\n", (int)random);
			if (random < 10 && vgame.BuildCount[bc_fire] == 0) {
				DoSpecificDisaster(diFireOutbreak);
			} else if (random < 15 &&
			    game.objects[obj_monster].active == 0) {
				DoSpecificDisaster(diMonster);
			} else if (random < 17 &&
			    game.objects[obj_dragon].active == 0) {
				DoSpecificDisaster(diDragon);
			} else if (random < 19) {
				DoSpecificDisaster(diMeteor);
			}
			/* only one chance for disaster per turn */
			break;
		}
	}
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
}

void
DoSpecificDisaster(disaster_t disaster)
{
	UInt32 randomTile;
	UInt16 x, y;
	Int16 ce = 0;
	Int16 i = 0;

	while (i++ < 400 && ce == 0) {
		randomTile = GetRandomNumber(MapMul());
		x = (UInt16)(randomTile % getMapWidth());
		y = (UInt16)(randomTile / getMapHeight());

		switch (disaster) {
		case diFireOutbreak:
			ce = BurnField(x, y, (UInt16)0);
			break;
		case diPlantExplosion:
			ce = 0;
			break;
		case diMonster:
			ce = CreateMonster(x, y);
			break;
		case diDragon:
			ce = CreateDragon(x, y);
			break;
		case diMeteor:
			ce = MeteorDisaster(x, y);
			break;
		default: break;
		}
	}
	if (ce) {
		UIDisasterNotify(disaster);
		Goto(x, y, goto_center);
		MapHasJumped();
	}
}

Int16
UpdateDisasters(void)
{
	/* return false if no disasters are found */
	UInt16 i, j;
	welem_t type;
	int retval = 0;

	LockZone(lz_world);
	LockZone(lz_flags);
	clearScratch();
	for (i = 0; i < getMapHeight(); i++) {
		for (j = 0; j < getMapWidth(); j++) {
			type = getWorld(WORLDPOS(i, j));
			/* already looked at this one? */
			if (getScratch(WORLDPOS(i, j)) == 0) {
				if (type == Z_FIRE2) {
					retval = 1;
					if (GetRandomNumber(5) != 0) {
						/* are there any defences */
						if (GetDefenceValue(i, j) < 3) {
							FireSpread(i, j);
						}
						setWorldAndFlag(WORLDPOS(i, j),
						    Z_FIRE3, 0);
					} else {
						CreateWaste(i, j);
					}
				} else if (type == Z_FIRE1) {
					retval = 1;
					setWorldAndFlag(WORLDPOS(i, j),
					    Z_FIRE2, 0);
				} else if (type == Z_FIRE3) {
					retval = 1;
					CreateWaste(i, j);
				}
			}
		}
	}
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
	return (retval);
}

/*!
 * \brief Cause a fire to spread out from the point chosen
 * \param x position on horizontal of map to spread fire
 * \param y position on vertical of map to spread fire
 */
static void
FireSpread(UInt16 x, UInt16 y)
{
	if (x > 0)
		BurnField((UInt16)(x - 1), (UInt16)y, 0);
	if (x < (UInt16)(getMapWidth() - 1))
		BurnField((UInt16)(x + 1), (UInt16)y, 0);
	if (y > 0)
		BurnField((UInt16)x, (UInt16)(y - 1), 0);
	if (y < (UInt16)(getMapHeight() - 1))
		BurnField((UInt16)x, (UInt16)(y + 1), 0);
}

Int16
BurnField(UInt16 x, UInt16 y, Int16 forceit)
{
	welem_t node;
	int rv = 0;

	LockZone(lz_world);
	LockZone(lz_flags);
	node = getWorld(WORLDPOS(x, y));
	if ((forceit != 0 && node != Z_BRIDGE &&
	    node != Z_REALWATER) ||
	    (node != Z_FIRE1 &&
	    node != Z_FIRE2 &&
	    node != Z_FIRE3 &&
	    node != Z_DIRT &&
	    node != Z_WASTE &&
	    node != Z_FAKEWATER &&
	    node != Z_REALWATER &&
	    node != Z_CRATER &&
	    node != Z_BRIDGE &&
	    ContainsDefence(x, y) == 0)) {
		Build_Destroy(x, y);
		setWorldAndFlag(WORLDPOS(x, y), Z_FIRE1, 1);
		setScratch(WORLDPOS(x, y));
		DrawCross(x, y, 1, 1);
		vgame.BuildCount[bc_fire]++;
		rv = 1;
	}
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
	return (rv);
}

Int16
CreateMonster(UInt16 x, UInt16 y)
{
	welem_t node;
	int rv = 0;

	LockZone(lz_world);
	LockZone(lz_flags);
	node = getWorld(WORLDPOS(x, y));
	if (node != Z_REALWATER && node != Z_CRATER) {
		game.objects[obj_monster].x = x;
		game.objects[obj_monster].y = y;
		game.objects[obj_monster].dir = (UInt16)GetRandomNumber(8);
		game.objects[obj_monster].active = 1;
		DrawField(x, y);
		rv = 1;
	}
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
	return (rv);
}

Int16
CreateDragon(UInt16 x, UInt16 y)
{
	welem_t node;
	int rv = 0;

	LockZone(lz_world);
	LockZone(lz_flags);
	node = getWorld(WORLDPOS(x, y));
	if (node != Z_REALWATER && node != Z_CRATER) {
		game.objects[obj_dragon].x = x;
		game.objects[obj_dragon].y = y;
		game.objects[obj_dragon].dir = (UInt16)GetRandomNumber(8);
		game.objects[obj_dragon].active = 1;
		DrawField(x, y);
		rv = 1;
	}
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
	return (rv);
}

/*!
 * \brief Check if a monster is surrounded by defensive units.
 * \param i index of monster to check.
 */
static void
MonsterCheckSurrounded(UInt16 i)
{
	if (GetDefenceValue(game.objects[i].x, game.objects[i].y) >= 11 ||
		GetRandomNumber(50) < 2) {
		game.objects[i].active = 0; /* kill the sucker */
		WriteLog("killing a monster\n");
	}
}

/*!
 * \brief Get the value of the defence fields around this point
 * \param xpos horizontal location
 * \param ypos vertical location
 * \return the 'level' of the defence surrounding the position.
 */
static UInt16
GetDefenceValue(UInt16 xpos, UInt16 ypos)
{
	/* police = 2 */
	/* firemen = 3 */
	/* military = 6 */
	UInt16 def =
	    ContainsDefence(xpos + 1, ypos) +
	    ContainsDefence(xpos + 1, ypos + 1) +
	    ContainsDefence(xpos + 1, ypos - 1) +
	    ContainsDefence(xpos, ypos + 1) +
	    ContainsDefence(xpos, ypos - 1) +
	    ContainsDefence(xpos - 1, ypos) +
	    ContainsDefence(xpos - 1, ypos + 1) +
	    ContainsDefence(xpos - 1, ypos + 1);
	return (def);
}

/*!
 * \brief check if the node has a defence position within it.
 *
 * If it does, return the value of that defence.
 * \param x horizontal location
 * \param y vertical location
 * \return the level of defence that this node provides.
 */
static UInt16
ContainsDefence(UInt16 x, UInt16 y)
{
	int i;

	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (game.units[i].x == x &&
			game.units[i].y == y &&
			game.units[i].active != 0) {
			switch (game.units[i].type) {
			case DuPolice:
				return (2);
			case DuFireman:
				return (3);
			case DuMilitary:
				return (6);
			default:
				return (0);
			}
		}
	}
	return (0);
}

void
MoveAllObjects(void)
{
	UInt16 i, x, y;

	for (i = 0; i < NUM_OF_OBJECTS; i++) {
		if (game.objects[i].active != 0) {
			/* hmm, is this thing destructive? */
			if (i == obj_dragon) {
				if (!BurnField(game.objects[i].x,
				    game.objects[i].y, 1)) {
					CreateWaste(game.objects[i].x,
					    game.objects[i].y);
				}
				MonsterCheckSurrounded(i);
			} else if (i == obj_monster) {
				/* whoo-hoo, bingo again */
				CreateWaste(game.objects[i].x,
				    game.objects[i].y);
				MonsterCheckSurrounded(i);
			}

			x = game.objects[i].x; /* save old position */
			y = game.objects[i].y;

			switch (GetRandomNumber(OBJ_CHANCE_OF_TURNING)) {
			case 1: /* yes, clockwise */
				game.objects[i].dir = (game.objects[i].dir+1)%8;
				break;
			case 2: /* yes, counter-clockwise */
				game.objects[i].dir = (game.objects[i].dir+7)%8;
				break;
			default:
				break;
			}
			/* now move it a nod */
			switch (game.objects[i].dir) { /* first up/down */
			case 0: /* up */
			case 1: /* up-right */
			case 7: /* up-left */
				if (game.objects[i].y > 0) {
					game.objects[i].y--;
				} else {
					game.objects[i].dir = 4;
				}
				break;
			case 3: /* down-right */
			case 4: /* down */
			case 5: /* down-left */
				if (game.objects[i].y <
				    (UInt16)(getMapHeight()-1)) {
					game.objects[i].y++;
				} else {
					game.objects[i].dir = 0;
				}
				break;
			default:
				break;
			}

			switch (game.objects[i].dir) { /* then left/right */
			case 1: /* up-right */
			case 2: /* right */
			case 3: /* down-right */
				if (game.objects[i].x <
				    (UInt16)(getMapWidth()-1)) {
					game.objects[i].x++;
				} else {
					game.objects[i].dir = 6;
				}
				break;
			case 5: /* down-left */
			case 6: /* left */
			case 7: /* up-left */
				if (game.objects[i].x > 0) {
					game.objects[i].x--;
				} else {
					game.objects[i].dir = 2;
				}
				break;
			default:
				break;
			}
			LockZone(lz_world);
			LockZone(lz_flags);
			DrawCross(x, y, 1, 1); /* (erase it) */
			DrawField(game.objects[i].x, game.objects[i].y);
			UnlockZone(lz_flags);
			UnlockZone(lz_world);
		}
	}
}

Int16
MeteorDisaster(UInt16 x, UInt16 y)
{
	Int16 k;

	k = (Int16)GetRandomNumber(3) + 1;
	CreateMeteor(x, y, k);
	return (1);
}

/*!
 * \brief Create a waste zone for the meteor.
 * \param x horizontal position
 * \param y vertical position
 * \param size size of the meteor
 */
static void
CreateMeteor(UInt16 x, UInt16 y, Int16 size)
{
	UInt16 j;
	UInt16 i =  (Int16)(x - size) < 0 ? 0 : (UInt16)(x - size);

	LockZone(lz_world);
	LockZone(lz_flags);
	UILockScreen();
	for (; i <= x + size; i++) {
		j = (Int16)(y - size) < 0 ? 0 : (UInt16)(y - size);
		for (; j <= y + size; j++) {
			if (i < getMapWidth() &&
			    j < getMapHeight()) {
				if (GetRandomNumber(5) < 2) {
					if (getWorld(WORLDPOS(i, j)) !=
					    Z_REALWATER) {
						CreateWaste(i, j);
					}
				} else if (GetRandomNumber(5) < 4) {
					if (getWorld(WORLDPOS(i, j)) !=
					    Z_REALWATER &&
					    getWorld(WORLDPOS(i, j)) !=
					    Z_FAKEWATER) {
						BurnField(i, j, 1);
					}
				}
			}
		}
	}
	Build_Destroy(x, y);
	setWorldAndFlag(WORLDPOS(x, y), Z_CRATER, 0);
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
	UIUnlockScreen();
	RedrawAllFields();
}

/*!
 * \brief Turn the zone into wasteland
 * \param x x position to affect
 * \param y y position to affect
 */
static void
CreateWaste(UInt16 x, UInt16 y)
{
	welem_t node;

	LockZone(lz_world);
	LockZone(lz_flags);
	node = getWorld(WORLDPOS(x, y));
	Build_Destroy(x, y);
	if (node == Z_REALWATER || IsRoadBridge(node)) {
		UnlockZone(lz_flags);
		UnlockZone(lz_world);
		return;
	}
	setWorldAndFlag(WORLDPOS(x, y), Z_WASTE, 0);
	vgame.BuildCount[bc_waste]++;
	DrawCross(x, y, 1, 1);
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
	if (node == Z_COALPLANT || node == Z_NUCLEARPLANT)  {
		UIDisasterNotify(diPlantExplosion);
		FireSpread(x, y);
	}
}

