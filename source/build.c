
/*! \file
 * \brief Code that deals with the building of items in the simulation.
 *
 * It is controlled by the table buildStructure, which defines all
 * the items that can be built.
 */

#ifdef PALM
#include <PalmOS.h>
#include <simcity.h>
#include <zakdef.h>
#else
#include <sys/types.h>
#include <stddef.h>
#include <assert.h>
#endif
#include <compilerpragmas.h>
#include <build.h>
#include <globals.h>
#include <ui.h>
#include <drawing.h>

/*! \brief Defines the build function pointer type */
typedef void (*BuildF)(Int16 xpos, Int16 ypos, UInt16 type);

static void Build_Road(Int16 xpos, Int16 ypos, UInt16 type);
static void Build_PowerLine(Int16 xpos, Int16 ypos, UInt16 type);
static void Build_WaterPipe(Int16 xpos, Int16 ypos, UInt16 type);
static void Build_Generic(Int16 xpos, Int16 ypos, UInt16 type);
static void Build_Defence(Int16 xpos, Int16 ypos, UInt16 type);

static void CreateForest(UInt32 pos, Int16 size);
static void RemoveDefence(Int16 xpos, Int16 ypos);

static Int16 SpendMoney(UInt32 howMuch);

/*!
 * \brief array mapping build requests to reactions.
 *
 * This structure is used to map the BuildCode to an appropriate function
 * to perform the building as well as some extra events to happen as a
 * consequence of the item being built.
 */
static const struct _bldStruct {
	BuildCode bt;	/*!< The code of the item to be built. */
	BuildF func;	/*!< Function to call. */
	UInt16 type;	/*!< Type of the item (used for buildcounts */
	UInt16 gridsToUpdate; /*!< Grids to update as a result of adding
				the item to the simulation */
} buildStructure[] = {
	{ Be_Bulldozer, Build_Bulldoze, 0, GRID_ALL },
	{ Be_Zone_Residential, Build_Generic, ZONE_RESIDENTIAL, GRID_ALL },
	{ Be_Zone_Commercial, Build_Generic, ZONE_COMMERCIAL, GRID_ALL},
	{ Be_Zone_Industrial, Build_Generic, ZONE_INDUSTRIAL, GRID_ALL},
	{ Be_Road, Build_Road, 0, 0 },
	{ Be_Power_Plant, Build_Generic, TYPE_POWER_PLANT, GRID_ALL },
	{ Be_Nuclear_Plant, Build_Generic, TYPE_NUCLEAR_PLANT, GRID_ALL },
	{ Be_Power_Line, Build_PowerLine, 0, GRID_ALL },
	{ Be_Water_Pump, Build_Generic, TYPE_WATER_PUMP, GRID_ALL },
	{ Be_Water_Pipe, Build_WaterPipe, 0, GRID_WATER },
	{ Be_Tree, Build_Generic, TYPE_TREE, 0 },
	{ Be_Water, Build_Generic, TYPE_WATER, 0 },
	{ Be_Fire_Station, Build_Generic, TYPE_FIRE_STATION, GRID_ALL },
	{ Be_Police_Station, Build_Generic, TYPE_POLICE_STATION, GRID_ALL },
	{ Be_Military_Base, Build_Generic, TYPE_MILITARY_BASE, GRID_ALL },
	{ Be_Defence_Fire, Build_Defence, DuFireman, 0 },
	{ Be_Defence_Police, Build_Defence, DuPolice, 0 },
	{ Be_Defence_Military, Build_Defence, DuMilitary, 0 },
};

/*!
 * \brief Build something at the location specified.
 *
 * Looks up the item in the buildStructure and builds it at the appropriate
 * location.
 * \param xpos the X position on the map
 * \param ypos the Y position on the map
 */
void
BuildSomething(Int16 xpos, Int16 ypos)
{
	UInt16 item = UIGetSelectedBuildItem();
	struct _bldStruct *be = (struct _bldStruct *)&(buildStructure[item]);

	if (item >= (sizeof (buildStructure) / sizeof (buildStructure[0]))) {
		UIDisplayError1("Unknown Build Item");
		return;
	}

	be->func(xpos, ypos, be->type);
	AddGridUpdate(be->gridsToUpdate);
}

/*!
 * \brief Remove a defence unit from a location
 *
 * Removes the defence item from the location specified.
 * \param xpos the X position on the map
 * \param ypos the Y position on the map
 */
static void
RemoveDefence(Int16 xpos, Int16 ypos)
{
	int i;
	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (game.units[i].x == xpos &&
		    game.units[i].y == ypos) {
			game.units[i].active = 0;
			DrawCross(game.units[i].x, game.units[i].y);
		}
	}
}

/*!
 * \brief Remove all the defences on the map
 */
void
RemoveAllDefence(void)
{
	int i;
	for (i = 0; i < NUM_OF_UNITS; i++) {
		game.units[i].active = 0;
		DrawCross(game.units[i].x, game.units[i].y);
	}
}

/*! \brief Maps build units to locations in the build array */
static const struct buildCounters {
	DefenceUnitTypes	unit; /*!< unit this corresponds to */
	UInt16			counter; /*!< counter to index */
	UInt16			start; /*!< starting index in items */
	UInt16			end; /*!< ending index in items */
} counters[] = {
	{ DuFireman, bc_fire_stations, DEF_FIREMEN_START, DEF_FIREMEN_END },
	{ DuPolice, bc_police_stations, DEF_POLICE_START, DEF_POLICE_END },
	{ DuMilitary, bc_military_bases, DEF_MILITARY_START,
		DEF_MILITARY_END }
};

/*!
 * \brief Build a defence unit.
 *
 * \param xpos item position on the x axis
 * \param ypos item position on the y axis
 * \param type type of defence unit to build
 */
static void
Build_Defence(Int16 xpos, Int16 ypos, UInt16 type)
{
	int oldx;
	int oldy;
	UInt16 i;
	int sel = -1;
	int newactive = 1;
	UInt16 start;
	UInt16 end;
	UInt16 max;
	int nCounter;
	const struct buildCounters *cnt;

	if (type > DuMilitary)
		return;

	cnt = &counters[type];

	/* this is here to make sure not too many of any item are created */
	nCounter = cnt->counter;

	if (vgame.BuildCount[nCounter] == 0)
		return; /* no special building */

	start = cnt->start;
	end = cnt->end;

	/* make sure we can't make too many objects */
	if (((unsigned)((end - start) + 1) <
		    (unsigned)(vgame.BuildCount[nCounter] / 3)))
		max = end;
	else
		max = (UInt16)(vgame.BuildCount[nCounter] / 3 + start);

	/* first remove all defence on this tile */
	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (xpos == game.units[i].x &&
			ypos == game.units[i].y &&
			game.units[i].active != 0) {
			/* no need to build something already here */
			if ((UInt16)game.units[i].type == type)
				return;
			game.units[i].active = 0;
		}
	}

	/* find an empty slot for the new defence unit */
	for (i = start; i <= max; i++) {
		if (game.units[i].active == 0) {
			sel = i;
			break;
		}
	}
	if (sel == -1) {
		/* none found - start from the beginning */
		for (i = start; i <= max; i++) {
			if (game.units[i].active == 1) {
				sel = i;
				newactive = 2;
				break;
			} else {
				game.units[i].active = 2;
			}
		}
	}
	if (sel == -1) {
		/* if STILL none found - then it's number 0 */
		for (i = start; i <= max; i++) {
			if (game.units[i].active != 0) {
				game.units[i].active = 1;
			}
		}
		sel = start;
		newactive = 2;
	}

	oldx = game.units[sel].x;
	oldy = game.units[sel].y;

	game.units[sel].x = xpos;
	game.units[sel].y = ypos;
	game.units[sel].active = newactive;
	game.units[sel].type = (DefenceUnitTypes)type;

	LockWorld();
	DrawCross(oldx, oldy);
	DrawCross(xpos, ypos);
	UnlockWorld();
}

/*!
 * \brief Bulldoze a zone
 * \param xpos X position on the map
 * \param ypos Y position on the map
 * \param _unused unused.
 *
 * \todo Prohibit the rezoning of Wasteland.
 */
void
Build_Bulldoze(Int16 xpos, Int16 ypos, UInt16 _type __attribute__((unused)))
{
	int type;

	LockWorld();
	type = GetWorld(WORLDPOS(xpos, ypos));
	if (type != TYPE_DIRT && type != TYPE_FIRE1 && type != TYPE_FIRE2 &&
	    type != TYPE_FIRE3 && type != TYPE_REAL_WATER) {
		if (SpendMoney(BUILD_COST_BULLDOZER)) {
			Build_Destroy(xpos, ypos);
		} else {
			UIDisplayError(enOutOfMoney);
		}
	}
	RemoveDefence(xpos, ypos);
	UnlockWorld();
}

/*!
 * \brief Attempt to destroy the item at the position in question
 *
 * Performed by the bulldoze and the auto bulldoze.
 * \param xpos the X position on the map
 * \param ypos the Y position on the map
 */
void
Build_Destroy(Int16 xpos, Int16 ypos)
{
	unsigned char type;

	type = GetWorld(WORLDPOS(xpos, ypos));
	RemoveDefence(xpos, ypos);

	vgame.BuildCount[bc_commercial] -=
	    (type >= (ZONE_COMMERCIAL * 10 + 20) &&
	    type <= (ZONE_COMMERCIAL * 10 + 29)) ? (type % 10) + 1 : 0;
	vgame.BuildCount[bc_residential] -=
	    (type >= (ZONE_RESIDENTIAL * 10 + 20) &&
	    type <= (ZONE_RESIDENTIAL * 10 + 29)) ? (type % 10) + 1 : 0;
	vgame.BuildCount[bc_industrial] -=
	    (type >= (ZONE_INDUSTRIAL * 10 + 20) &&
	    type <= (ZONE_INDUSTRIAL * 10 + 29)) ? (type % 10) + 1 : 0;
	vgame.BuildCount[bc_roads] -= IsRoad(type) ? 1 : 0;
	vgame.BuildCount[bc_trees] -= (type == TYPE_TREE) ? 1 : 0;
	vgame.BuildCount[bc_water] -= (type == TYPE_WATER) ? 1 : 0;
	vgame.BuildCount[bc_waste] -= (type == TYPE_WASTE) ? 1 : 0;
	vgame.BuildCount[bc_coalplants] -=
	    (type == TYPE_POWER_PLANT) ? 1 : 0;
	vgame.BuildCount[bc_nuclearplants] -=
	    (type == TYPE_NUCLEAR_PLANT) ? 1 : 0;
	vgame.BuildCount[bc_powerlines] -=
	    ((type == TYPE_POWERROAD_2) || (type == TYPE_POWERROAD_1) ||
	    (type == TYPE_POWER_LINE)) ? 1 : 0;
	vgame.BuildCount[bc_fire] -= ((type == TYPE_FIRE1) ||
	    (type == TYPE_FIRE2) || (type == TYPE_FIRE3)) ? 1 : 0;
	vgame.BuildCount[bc_waterpipes] -= ((type == TYPE_WATER_PIPE) ||
	    (type == TYPE_WATERROAD_1) || (type == TYPE_WATERROAD_2)) ? 1 : 0;
	vgame.BuildCount[bc_fire_stations] -=
	    (type == TYPE_FIRE_STATION) ? 1 : 0;
	vgame.BuildCount[bc_police_stations] -=
	    (type == TYPE_POLICE_STATION) ? 1 : 0;
	vgame.BuildCount[bc_military_bases] -=
	    (type == TYPE_MILITARY_BASE) ? 1 : 0;
	vgame.BuildCount[bc_waterpumps] -=
	    (type == TYPE_WATER_PUMP) ? 1 : 0;
	AddGridUpdate(GRID_ALL);

	if (type == TYPE_BRIDGE || type == TYPE_REAL_WATER) {
		/* A bridge turns into real_water when detroyed */
		SetWorld(WORLDPOS(xpos, ypos), TYPE_REAL_WATER);
	} else {
		SetWorld(WORLDPOS(xpos, ypos), TYPE_DIRT);
	}

	/* Locks the world flags itself */
	DrawCross(xpos, ypos);
}

/*!
 * \brief Mapping of zone to cost of building on a zone
 */
static const struct _costMappings {
	UInt16 type; /*!< type of zone that is built */
	UInt32 cost; /*!< cost of building the zone in question */
	Int16 count; /*!< counter to affect as a result of building this */
} genericMappings[] = {
	{ ZONE_RESIDENTIAL, BUILD_COST_ZONE, -1 },
	{ ZONE_INDUSTRIAL, BUILD_COST_ZONE, -1 },
	{ ZONE_COMMERCIAL, BUILD_COST_ZONE, -1 },
	{ TYPE_POWER_PLANT, BUILD_COST_POWER_PLANT, bc_coalplants },
	{ TYPE_NUCLEAR_PLANT, BUILD_COST_NUCLEAR_PLANT, bc_nuclearplants },
	{ TYPE_WATER, BUILD_COST_WATER, bc_water },
	{ TYPE_TREE, BUILD_COST_TREE, bc_trees },
	{ TYPE_FIRE_STATION, BUILD_COST_FIRE_STATION, bc_fire_stations },
	{ TYPE_POLICE_STATION, BUILD_COST_POLICE_STATION, bc_police_stations },
	{ TYPE_MILITARY_BASE, BUILD_COST_MILITARY_BASE, bc_military_bases },
	{ TYPE_WATER_PUMP, BUILD_COST_WATER_PUMP, bc_waterpumps },
	{ 0, 0, -1 }
};

/*!
 * \brief check if a zone is bulldozable
 *
 * \return true if the zone can be bulldozed.
 * \todo add the extra field types
 */
static Int16
IsBulldozable(UInt8 zone)
{
	return ((zone == TYPE_DIRT) ||
	    ((zone == TYPE_TREE) && game.auto_bulldoze));
}

/*!
 * \brief Build a generic item
 *
 * type to build is based on the type passed in
 * \param xpos the xposition to build the item at
 * \param ypos the yposition to build the item at
 * \param type type of item to be built
 */
void
Build_Generic(Int16 xpos, Int16 ypos, UInt16 type)
{
	unsigned char worldItem;
	unsigned long toSpend = 0;

	struct _costMappings *cmi = (struct _costMappings *)getIndexOf(
	    (char *)&genericMappings[0], sizeof (genericMappings[0]), type);
	LockWorld();
#ifdef PALM
	ErrFatalDisplayIf(cmi == NULL, "No generic->item mapping");
#else
	assert(cmi != NULL);
#endif
	if (cmi == NULL)
		return;

	toSpend = cmi->cost;

	worldItem = GetWorld(WORLDPOS(xpos, ypos));

	if ((type == TYPE_TREE) && (worldItem == TYPE_TREE)) {
		UnlockWorld();
		return;
	}

	if (IsBulldozable(worldItem)) {
		if (worldItem == TYPE_TREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			SetWorld(WORLDPOS(xpos, ypos), (unsigned char)type);
			DrawCross(xpos, ypos);

			/*  update counter */
			if (IsRoad(type)) {
				vgame.BuildCount[bc_roads]++;
			} else {
				if (cmi->count != -1)
					vgame.BuildCount[cmi->count]++;
			}
		} else {
			UIDisplayError(enOutOfMoney);
		}
	}
	UnlockWorld();
}

/*!
 * \brief Build a road.
 *
 * Auto bulldoze if requested
 * \param xpos xposition to build the road at
 * \param ypos yposition to build the road at
 * \param type unused in this context
 * \todo replace the constants here with symbolic values.
 */
void
Build_Road(Int16 xpos, Int16 ypos, UInt16 type __attribute__((unused)))
{
	int old;
	unsigned long toSpend = 0;

	LockWorld();
	old = GetWorld(WORLDPOS(xpos, ypos));
	toSpend = BUILD_COST_ROAD;
	if (old == TYPE_POWER_LINE) {
		switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos), 1)) {
		case 70: /* straight power line, we can build here */
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos),
				    TYPE_POWERROAD_1);
				DrawCross(xpos, ypos);
				vgame.BuildCount[bc_roads]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
			break;
		case 71: /* ditto */
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos),
				    TYPE_POWERROAD_2);
				DrawCross(xpos, ypos);
				vgame.BuildCount[bc_roads]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
			break;
		}
	} else if (old == TYPE_WATER_PIPE) {
		switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos), 3)) {
		case 92: /* straight water pipe, we can build here */
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos),
				    TYPE_WATERROAD_1);
				DrawCross(xpos, ypos);
				vgame.BuildCount[bc_roads]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
			break;
		case 93: /* ditto */
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos),
				    TYPE_WATERROAD_2);
				DrawCross(xpos, ypos);
				vgame.BuildCount[bc_roads]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
			break;
		}
	} else if (old == TYPE_REAL_WATER) {
		/* build a bridge across the water (yup, that's a song) */
		if (SpendMoney(toSpend)) {
			SetWorld(WORLDPOS(xpos, ypos), TYPE_BRIDGE);
			DrawCross(xpos, ypos);
			vgame.BuildCount[bc_roads]++;
		} else {
			UIDisplayError(enOutOfMoney);
		}
	} else if (IsBulldozable(old)) {
		if (old == TYPE_TREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			SetWorld(WORLDPOS(xpos, ypos), TYPE_ROAD);
			DrawCross(xpos, ypos);
			vgame.BuildCount[bc_roads]++;
		} else {
			UIDisplayError(enOutOfMoney);
		}
	}
	UnlockWorld();
}

/*!
 * \brief Build a power line.
 *
 * Auto bulldoze if requested.
 * \param xpos x position on map to build on
 * \param xpos y position on map to build on
 * \param type unused for this function
 * \todo Replace magic numbers with symbolic constants
 */
static void
Build_PowerLine(Int16 xpos, Int16 ypos, UInt16 type __attribute__((unused)))
{
	int old;
	unsigned long toSpend = 0;

	LockWorld();

	old = GetWorld(WORLDPOS(xpos, ypos));
	toSpend = BUILD_COST_POWER_LINE;
	if (IsBulldozable(old) || (old == TYPE_ROAD)) {
		if (old == TYPE_ROAD) {
			switch (
			    GetSpecialGraphicNumber(WORLDPOS(xpos, ypos), 0)) {
			case 10: /* straight road, we can build a power line */
				if (SpendMoney(toSpend)) {
					SetWorld(WORLDPOS(xpos, ypos),
					    TYPE_POWERROAD_2);
					DrawCross(xpos, ypos);
					vgame.BuildCount[bc_powerlines]++;
				} else {
					UIDisplayError(enOutOfMoney);
				}
				break;
			case 11: /* ditto */
				if (SpendMoney(toSpend)) {
					SetWorld(WORLDPOS(xpos, ypos),
					    TYPE_POWERROAD_1);
					DrawCross(xpos, ypos);
					vgame.BuildCount[bc_powerlines]++;
				} else {
					UIDisplayError(enOutOfMoney);
				}
				break;
			}
		} else {
			if (old == TYPE_TREE) toSpend += BUILD_COST_BULLDOZER;
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos), TYPE_POWER_LINE);
				DrawCross(xpos, ypos);
				vgame.BuildCount[bc_powerlines]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
		}
	}
	UnlockWorld();
}

/*!
 * \param Build a water pipe.
 *
 * Auto bulldoze as needed
 * \param xpos x position on map to build on
 * \param xpos y position on map to build on
 * \param type unused for this function
 * \todo replace magic numbers with symbolic constants
 */
static void
Build_WaterPipe(Int16 xpos, Int16 ypos, UInt16 type __attribute__((unused)))
{
	int old;
	unsigned long toSpend = 0;

	LockWorld();

	toSpend = BUILD_COST_WATER_PIPE;
	old = GetWorld(WORLDPOS(xpos, ypos));
	if (IsBulldozable(old) || (old == TYPE_ROAD)) {
		if (old == TYPE_ROAD) {
			switch (
			    GetSpecialGraphicNumber(WORLDPOS(xpos, ypos), 0)) {
			case 10: /* straight road, we can build a power line */
				if (SpendMoney(toSpend)) {
					SetWorld(WORLDPOS(xpos, ypos),
					    TYPE_WATERROAD_2);
					DrawCross(xpos, ypos);
					vgame.BuildCount[bc_waterpipes]++;
				} else {
					UIDisplayError(enOutOfMoney);
				}
				break;
			case 11: /* ditto */
				if (SpendMoney(toSpend)) {
					SetWorld(WORLDPOS(xpos, ypos),
					    TYPE_WATERROAD_1);
					DrawCross(xpos, ypos);
					vgame.BuildCount[bc_waterpipes]++;
				} else {
					UIDisplayError(enOutOfMoney);
				}
				break;
			}
		} else {
			if (type == TYPE_TREE) toSpend += BUILD_COST_BULLDOZER;
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos), TYPE_WATER_PIPE);
				DrawCross(xpos, ypos);
				vgame.BuildCount[bc_waterpipes]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
		}
	}
	UnlockWorld();
}

/*!
 * \brief Spend a chunk of cash.
 *
 * Won't allow you to go negative.
 * \param howMuch the amount to spend
 * \return true if I spent the money
 */
static Int16
SpendMoney(UInt32 howMuch)
{
	if (howMuch > (unsigned long)game.credits)
		return (0);

	game.credits -= howMuch;

	/* now redraw the credits */
	UIInitDrawing();
	UIDrawCredits();
	UIFinishDrawing();
	return (1);
}

/*!
 * \brief Create a river on the map
 * \todo Make the river more interesting
 */
void
CreateFullRiver(void)
{
	int i, j, k, width;
	int axis;

	width = GetRandomNumber(5)+5;
	j = GetRandomNumber(GetMapSize());
	axis = GetRandomNumber(1);
	LockWorld();

	for (i = 0; i < GetMapSize(); i++) {
		for (k = j; k < (width + j); k++) {
			if ((k > 0) && (k < GetMapSize())) {
				if (axis)
					SetWorld(WORLDPOS(i, k),
					    TYPE_REAL_WATER);
				else
					SetWorld(WORLDPOS(k, i),
					    TYPE_REAL_WATER);
			}
		}

		switch (GetRandomNumber(3)) {
		case 0:
			if (width >  5)
				width--;
			break;
		case 1:
			if (width < 15)
				width++;
			break;
		default:
			break;
		}
		switch (GetRandomNumber(4)) {
		case 0:
			if (j > 0)
				j--;
			break;
		case 1:
			if (j < GetMapSize())
				j++;
			break;
		default:
			break;
		}
	}
	UnlockWorld();
}

/*!
 * \brief Create the forests on the map.
 *
 * creates some "spraypainted" (someone called them that)
 * forests throughout the `wilderness`
 */
void
CreateForests(void)
{
	int i, j, k;
	unsigned long int pos;

	j = GetRandomNumber(6) + 7;
	for (i = 0; i < j; i++) {
		k = GetRandomNumber(6) + 8;
		pos = GetRandomNumber(GetMapMul());
		CreateForest(pos, k);
	}
}

/*!
 * \brief create a single forest at the point specified
 * \param pos Position on the map to start from
 * \param size Radius of the forest to paint.
 */
static void
CreateForest(UInt32 pos, Int16 size)
{
	int x, y, i, j, s;

	x = pos % GetMapSize();
	y = pos / GetMapSize();
	LockWorld();
	i = x;
	j = y;

	for (i = x - size; i <= x + size; i++) {
		for (j = y - size; j <= y + size; j++) {
			if (i >= 0 && i < GetMapSize() && j >= 0 &&
			    j < GetMapSize()) {
				if (GetWorld(WORLDPOS(i, j)) == TYPE_DIRT) {
					s = ((y > j) ? (y - j) : (j - y)) +
						((x > i) ? (x - i) : (i - x));
					if (GetRandomNumber(s) < 2) {
						SetWorld(WORLDPOS(i, j),
						    TYPE_TREE);
						vgame.BuildCount[bc_trees]++;
					}
				}
			}
		}
	}
	UnlockWorld();
}
