/*! \file
 * \brief routines associated with disasters
 *
 * This module contains functions that are used to create disasters in the
 * simulation.
 */
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

static void FireSpread(Int16 x, Int16 y) DISASTER_SECTION;
static void CreateWaste(Int16 x, Int16 y) DISASTER_SECTION;
static UInt16 GetDefenceValue(Int16 xpos, Int16 ypos) DISASTER_SECTION;
static UInt16 ContainsDefence(Int16 x, Int16 y) DISASTER_SECTION;
static void MonsterCheckSurrounded(Int16 i) DISASTER_SECTION;
static void CreateMeteor(Int16 x, Int16 y, Int16 size) DISASTER_SECTION;

void
DoNastyStuffTo(welem_t type, UInt16 probability)
{
	/* nasty stuff means: turn it into wasteland */
	long unsigned int randomTile;
	int i, x, y;

	if (GetRandomNumber(probability) != 0)
		return;

	LockZone(lz_world);
	for (i = 0; i < 50; i++) {
		randomTile = GetRandomNumber(MapMul());
		if (getWorld(randomTile) == type) {
			/* wee, let's destroy something */
			x = randomTile % getMapWidth();
			y = randomTile / getMapWidth();
			CreateWaste(x, y);
			break;
		}
	}
	UnlockZone(lz_world);
}

void
DoRandomDisaster(void)
{
	unsigned long randomTile;
	int i, x, y, type, random;
	int disaster_level;

	disaster_level = getDisasterLevel();
	/* for those who can't handle the game (truth?) */
	if (disaster_level == 0)
		return;

	LockZone(lz_world);

	for (i = 0; i < 100; i++) { /* 100 tries to hit a useful tile */
		randomTile = GetRandomNumber(MapMul());
		type = getWorld(randomTile);
		if (type != Z_DIRT &&
			type != Z_REALWATER &&
			type != Z_CRATER) {
			x = randomTile % getMapWidth();
			y = randomTile / getMapHeight();
			/* TODO: should depend on difficulty */
			random = GetRandomNumber(1000 / disaster_level);
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
	UnlockZone(lz_world);
}

void
DoSpecificDisaster(erdiType disaster)
{
	unsigned long randomTile;
	unsigned int x, y;
	int ce = 0;
	int i = 0;

	while (i++ < 400 && ce == 0) {
		randomTile = GetRandomNumber(MapMul());
		x = randomTile % getMapWidth();
		y = randomTile / getMapHeight();

		switch (disaster) {
		case diFireOutbreak:
			ce = BurnField(x, y, 0);
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
		UIDisplayError(disaster);
		Goto(x, y);
		MapHasJumped();
	}
}

Int16
UpdateDisasters(void)
{
	/* return false if no disasters are found */
	int i, j, type;
	int retval = 0;

	LockZone(lz_world);
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
						setWorld(WORLDPOS(i, j),
						    Z_FIRE3);
					} else {
						CreateWaste(i, j);
					}
				} else if (type == Z_FIRE1) {
					retval = 1;
					setWorld(WORLDPOS(i, j), Z_FIRE2);
				} else if (type == Z_FIRE3) {
					retval = 1;
					CreateWaste(i, j);
				}
			}
		}
	}
	UnlockZone(lz_world);
	return (retval);
}

/*!
 * \brief Cause a fire to spread out from the point chosen
 * \param x position on horizontal of map to spread fire
 * \param y position on vertical of map to spread fire
 */
static void
FireSpread(Int16 x, Int16 y)
{
	if (x > 0)
		BurnField(x - 1, y, 0);
	if (x < getMapWidth() - 1)
		BurnField(x + 1, y, 0);
	if (y > 0)
		BurnField(x, y - 1, 0);
	if (y < getMapHeight() - 1)
		BurnField(x, y + 1, 0);
}

Int16
BurnField(Int16 x, Int16 y, Int16 forceit)
{
	welem_t node;
	int rv = 0;

	LockZone(lz_world);
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
		setWorld(WORLDPOS(x, y), Z_FIRE1);
		setScratch(WORLDPOS(x, y));
		DrawCross(x, y, 1, 1);
		vgame.BuildCount[bc_fire]++;
		rv = 1;
	}
	UnlockZone(lz_world);
	return (rv);
}

Int16
CreateMonster(Int16 x, Int16 y)
{
	welem_t node;
	int rv = 0;

	LockZone(lz_world);
	node = getWorld(WORLDPOS(x, y));
	if (node != Z_REALWATER && node != Z_CRATER) {
		game.objects[obj_monster].x = x;
		game.objects[obj_monster].y = y;
		game.objects[obj_monster].dir = GetRandomNumber(8);
		game.objects[obj_monster].active = 1;
		DrawField(x, y);
		rv = 1;
	}
	UnlockZone(lz_world);
	return (rv);
}

Int16
CreateDragon(Int16 x, Int16 y)
{
	welem_t node;
	int rv = 0;

	LockZone(lz_world);
	node = getWorld(WORLDPOS(x, y));
	if (node != Z_REALWATER && node != Z_CRATER) {
		game.objects[obj_dragon].x = x;
		game.objects[obj_dragon].y = y;
		game.objects[obj_dragon].dir = GetRandomNumber(8);
		game.objects[obj_dragon].active = 1;
		DrawField(x, y);
		rv = 1;
	}
	UnlockZone(lz_world);
	return (rv);
}

/*!
 * \brief Check if a monster is surrounded by defensive units.
 * \param i index of monster to check.
 */
static void
MonsterCheckSurrounded(Int16 i)
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
GetDefenceValue(Int16 xpos, Int16 ypos)
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
ContainsDefence(Int16 x, Int16 y)
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
	int i, x, y;

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
			DrawCross(x, y, 1, 1); /* (erase it) */
			DrawField(game.objects[i].x, game.objects[i].y);
			UnlockZone(lz_world);
		}
	}
}

Int16
MeteorDisaster(Int16 x, Int16 y)
{
	int k;

	k = GetRandomNumber(3) + 1;
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
CreateMeteor(Int16 x, Int16 y, Int16 size)
{
	int i, j;

	LockZone(lz_world);
	UILockScreen();
	for (i = x - size; i <= x + size; i++) {
		for (j = y - size; j <= y + size; j++) {
			if (i >= 0 && i < getMapWidth() && j >= 0 &&
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
	setWorld(WORLDPOS(x, y), Z_CRATER);
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
CreateWaste(Int16 x, Int16 y)
{
	welem_t node;

	LockZone(lz_world);
	node = getWorld(WORLDPOS(x, y));
	Build_Destroy(x, y);
	UIUpdateMap(x, y);
	if (node == Z_REALWATER || IsRoadBridge(node)) {
		UnlockZone(lz_world);
		return;
	}
	setWorld(WORLDPOS(x, y), Z_WASTE);
	vgame.BuildCount[bc_waste]++;
	DrawCross(x, y, 1, 1);
	UnlockZone(lz_world);
	if (node == Z_COALPLANT || node == Z_NUCLEARPLANT)  {
		UIDisplayError(diPlantExplosion);
		FireSpread(x, y);
	}
}

