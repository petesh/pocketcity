
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
#include <simulation.h>

/*! \brief Defines the build function pointer type */
typedef void (*BuildF)(Int16 xpos, Int16 ypos, welem_t type);

static void Build_Road(Int16 xpos, Int16 ypos, welem_t type);
static void Build_PowerLine(Int16 xpos, Int16 ypos, welem_t type);
static void Build_WaterPipe(Int16 xpos, Int16 ypos, welem_t type);
static void Build_Generic(Int16 xpos, Int16 ypos, welem_t type);
static void Build_Generic4(Int16 xpos, Int16 ypos, welem_t type);
static void Build_Defence(Int16 xpos, Int16 ypos, welem_t type);

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
	welem_t type;	/*!< Type of the item (for buildcounts) */
	UInt16 gridsToUpdate; /*!< Grids to update as a result of adding
				the item to the simulation */
} buildStructure[] = {
	{ Be_Bulldozer, Build_Bulldoze, 0, GRID_ALL },
	{ Be_Zone_Residential, Build_Generic, Z_RESIDENTIAL_SLUM, GRID_ALL },
	{ Be_Zone_Commercial, Build_Generic, Z_COMMERCIAL_SLUM, GRID_ALL},
	{ Be_Zone_Industrial, Build_Generic, Z_INDUSTRIAL_SLUM, GRID_ALL},
	{ Be_Road, Build_Road, 0, 0 },
	{ Be_Power_Plant, Build_Generic4, Z_COALPLANT, GRID_ALL },
	{ Be_Nuclear_Plant, Build_Generic4, Z_NUCLEARPLANT, GRID_ALL },
	{ Be_Power_Line, Build_PowerLine, 0, GRID_ALL },
	{ Be_Water_Pump, Build_Generic, Z_PUMP, GRID_ALL },
	{ Be_Water_Pipe, Build_WaterPipe, 0, GRID_WATER },
	{ Be_Tree, Build_Generic, Z_FAKETREE, 0 },
	{ Be_Water, Build_Generic, Z_FAKEWATER, 0 },
	{ Be_Fire_Station, Build_Generic4, Z_FIRESTATION, GRID_ALL },
	{ Be_Police_Station, Build_Generic4, Z_POLICEDEPT, GRID_ALL },
	{ Be_Military_Base, Build_Generic4, Z_ARMYBASE, GRID_ALL },
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
			DrawCross(game.units[i].x, game.units[i].y, 1, 1);
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
		DrawCross(game.units[i].x, game.units[i].y, 1, 1);
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
	{ DuPolice, bc_police_departments, DEF_POLICE_START, DEF_POLICE_END },
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
Build_Defence(Int16 xpos, Int16 ypos, welem_t type)
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
	DrawCross(oldx, oldy, 1, 1);
	DrawCross(xpos, ypos, 1, 1);
	UnlockWorld();
}

static int
CantBulldoze(welem_t type)
{
	return (type == Z_DIRT || type == Z_FIRE1 || type == Z_FIRE2 ||
	    type == Z_FIRE3 || type == Z_REALWATER || type == Z_CRATER);
}

static UInt16
blockSize(welem_t type)
{
	/* This works because all the multiblocks are sequential */
	if (type >= Z_COALPLANT_START && type <= Z_ARMYBASE_END)
		return (4);
	return (1);
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
Build_Bulldoze(Int16 xpos, Int16 ypos, welem_t _type __attribute__((unused)))
{
	welem_t type;

	LockWorld();
	type = GetWorld(WORLDPOS(xpos, ypos));
	if (CantBulldoze(type))
		goto end;
	if (SpendMoney(BUILD_COST_BULLDOZER * blockSize(type))) {
		Build_Destroy(xpos, ypos);
	} else {
		UIDisplayError(enOutOfMoney);
	}
	RemoveDefence(xpos, ypos);
end:
	UnlockWorld();
}

/*!
 * \brief Determine the offsetting for destroying a node - 2x2 *only*
 * \param base the value of the base node type
 * \param node the node's value
 * \param x the x position
 * \param y the y position
 */
void
Doff(welem_t base, welem_t node, Int16 *x, Int16 *y)
{
	*x -= (node - base) % 2;
	*y -= (node - base) / 2;
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
	welem_t type;
	/* Destroy a 1x1 square area */
	int x_destroy = 1;
	int tx_destroy = 1;
	int y_destroy = 1;
	int ty_destroy = 1;

	type = GetWorld(WORLDPOS(xpos, ypos));
	RemoveDefence(xpos, ypos);

	if ((type >= Z_COMMERCIAL_MIN) && (type <= Z_COMMERCIAL_MAX)) {
		vgame.BuildCount[bc_count_commercial]--;
		vgame.BuildCount[bc_value_commercial] -= ZoneValue(type);
		goto finish;
	}
	if ((type >= Z_RESIDENTIAL_MIN) && (type <= Z_RESIDENTIAL_MAX)) {
		vgame.BuildCount[bc_count_residential]--;
		vgame.BuildCount[bc_value_residential] -= ZoneValue(type);
		goto finish;
	}
	if ((type >= Z_INDUSTRIAL_MIN) && (type <= Z_INDUSTRIAL_MAX)) {
		vgame.BuildCount[bc_count_industrial]--;
		vgame.BuildCount[bc_value_industrial] -= ZoneValue(type);
		goto finish;
	}
	if (type == Z_FAKETREE) {
		vgame.BuildCount[bc_count_trees]--;
		goto finish;
	}
	if (type == Z_FAKEWATER) {
		vgame.BuildCount[bc_water]--;
		goto finish;
	}
	if (type == Z_WASTE) {
		vgame.BuildCount[bc_waste]--;
		goto finish;
	}
	if (type >= Z_FIRE1 && type <= Z_FIRE3) {
		vgame.BuildCount[bc_fire]--;
		goto finish;
	}
	
	if (type >= Z_COALPLANT_START && type <= Z_COALPLANT_END) {
		vgame.BuildCount[bc_coalplants]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_COALPLANT_START, type, &xpos, &ypos);
		goto finish;
	}
	if ((type >= Z_NUCLEARPLANT_START) && 
	    (type <= Z_NUCLEARPLANT_END)) {
		vgame.BuildCount[bc_nuclearplants]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_NUCLEARPLANT_START, type, &xpos, &ypos);
		goto finish;
	}
	if ((type >= Z_FIRESTATION_START) &&
	    (type <= Z_FIRESTATION_END)) {
		vgame.BuildCount[bc_fire_stations]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_FIRESTATION_START, type, &xpos, &ypos);
		goto finish;
	}
	if (type >= Z_POLICEDEPT_START &&
	    type <= Z_POLICEDEPT_END) {
		vgame.BuildCount[bc_police_departments]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_POLICEDEPT_START, type, &xpos, &ypos);
		goto finish;
	}
	if (type >= Z_ARMYBASE_START &&
	    type <= Z_ARMYBASE_END) {
		vgame.BuildCount[bc_military_bases]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_ARMYBASE_START, type, &xpos, &ypos);
		goto finish;
	}
	if (type == Z_PUMP) {
		vgame.BuildCount[bc_waterpumps]--;
		goto finish;
	}
	
	if (IsRoad(type)) {
		vgame.BuildCount[bc_count_roads]--;
		vgame.BuildCount[bc_value_roads] -= ZoneValue(type);
	}
	if (IsPowerLine(type)) {
		vgame.BuildCount[bc_powerlines]--;
	}
	if (IsWaterPipe(type)) {
		vgame.BuildCount[bc_waterpipes]--;
	}
	if (IsRoadWater(type)) {
		vgame.BuildCount[bc_count_roads]--;
		vgame.BuildCount[bc_waterpipes]--;
		vgame.BuildCount[bc_value_roads] -= ZoneValue(type);
	}
	if (IsRoadPower(type)) {
		vgame.BuildCount[bc_count_roads]--;
		vgame.BuildCount[bc_powerlines]--;
		vgame.BuildCount[bc_value_roads] -= ZoneValue(type);
	}
	if (IsPowerWater(type)) {
		vgame.BuildCount[bc_waterpipes]--;
		vgame.BuildCount[bc_powerlines]--;
	}
	    
finish:
	AddGridUpdate(GRID_ALL);

	if (IsBridge(type) || type == Z_REALWATER) {
		/* A bridge turns into real_water when detroyed */
		SetWorld(WORLDPOS(xpos, ypos), Z_REALWATER);
	} else {
		if ((x_destroy != 1) || (y_destroy != 1)) {
			ty_destroy = y_destroy;
			while(ty_destroy) {
				tx_destroy=x_destroy;
				while(tx_destroy) {
					SetWorld(WORLDPOS(xpos - 1 + tx_destroy,
					    ypos - 1 + ty_destroy), Z_DIRT);
					tx_destroy--;
				}
				ty_destroy--;
			}
		} else {
			SetWorld(WORLDPOS(xpos, ypos), Z_DIRT);
		}
	}

	/* Locks the world flags itself */
	DrawCross(xpos, ypos, x_destroy, y_destroy);
}

/*!
 * \brief Mapping of zone to cost of building on a zone
 */
static const struct _costMappings {
	UInt16 type; /*!< type of zone that is built */
	UInt32 cost; /*!< cost of building the zone in question */
	Int16 count; /*!< counter to affect as a result of building this */
} genericMappings[] = {
	{ Z_RESIDENTIAL_SLUM, BUILD_COST_ZONE, -1 },
	{ Z_INDUSTRIAL_SLUM, BUILD_COST_ZONE, -1 },
	{ Z_COMMERCIAL_SLUM, BUILD_COST_ZONE, -1 },
	{ Z_COALPLANT, BUILD_COST_POWER_PLANT, bc_coalplants },
	{ Z_NUCLEARPLANT, BUILD_COST_NUCLEAR_PLANT, bc_nuclearplants },
	{ Z_FAKEWATER, BUILD_COST_WATER, bc_water },
	{ Z_FAKETREE, BUILD_COST_TREE, bc_count_trees },
	{ Z_FIRESTATION, BUILD_COST_FIRE_STATION, bc_fire_stations },
	{ Z_POLICEDEPT, BUILD_COST_POLICE_STATION, bc_police_departments },
	{ Z_ARMYBASE, BUILD_COST_MILITARY_BASE, bc_military_bases },
	{ Z_PUMP, BUILD_COST_WATER_PUMP, bc_waterpumps },
	{ 0, 0, -1 }
};

/*!
 * \brief check if a zone is bulldozable
 *
 * \return true if the zone can be bulldozed.
 * \todo add the extra field types
 */
static Int16
IsBulldozable(welem_t zone)
{
	return ((zone == Z_DIRT) ||
	    ((zone == Z_REALTREE) && getAutoBulldoze()));
}

/*!
 * \brief Build a generic 4-zone item
 *
 * type to build is based on the type passed in
 * \param xpos the xposition to build the item at
 * \param ypos the yposition to build the item at
 * \param type type of item to be built
 */
static void
Build_Generic4(Int16 xpos, Int16 ypos, welem_t type)
{
	unsigned char worldItem;
	unsigned long toSpend = 0;
	int loopx, loopy;
	int canbuild = 1;

	struct _costMappings *cmi = (struct _costMappings *)getIndexOf(
	    (char *)&genericMappings[0], sizeof (genericMappings[0]), type);
#ifdef PALM
	ErrFatalDisplayIf(cmi == NULL, "No generic->item mapping");
#else
	assert(cmi != NULL);
#endif
	if (cmi == NULL)
		return;
	LockWorld();

	toSpend = cmi->cost;

	for (loopx = xpos; loopx < xpos + 2; loopx++) {
		for (loopy = ypos; loopy < ypos + 2; loopy++) {
			worldItem = GetWorld(WORLDPOS(loopx, loopy));
			if (IsBulldozable(worldItem)) {
				if (worldItem == Z_REALTREE)
					toSpend += BUILD_COST_BULLDOZER;
			} else {
				canbuild = 0;
			}
		}
	}
	if (!canbuild)
		return;

	if (SpendMoney(toSpend)) {
		for (loopy = ypos; loopy < ypos + 2; loopy++) {
			for (loopx = xpos; loopx < xpos + 2; loopx++)
				SetWorld(WORLDPOS(loopx, loopy), type++);
			}

		if (cmi->count != -1)
			vgame.BuildCount[cmi->count]++;

		DrawCross(xpos, ypos, 2, 2);
	} else {
		UIDisplayError(enOutOfMoney);
	}
	UnlockWorld();
}

/*!
 * \brief Build a generic item
 *
 * type to build is based on the type passed in
 * \param xpos the xposition to build the item at
 * \param ypos the yposition to build the item at
 * \param type type of item to be built
 */
static void
Build_Generic(Int16 xpos, Int16 ypos, welem_t type)
{
	unsigned char worldItem;
	unsigned long toSpend = 0;

	struct _costMappings *cmi = (struct _costMappings *)getIndexOf(
	    (char *)&genericMappings[0], sizeof (genericMappings[0]), type);
#ifdef PALM
	ErrFatalDisplayIf(cmi == NULL, "No generic->item mapping");
#else
	assert(cmi != NULL);
#endif
	if (cmi == NULL)
		return;
	LockWorld();

	toSpend = cmi->cost;

	worldItem = GetWorld(WORLDPOS(xpos, ypos));

	if ((type == Z_FAKETREE) && ((worldItem == Z_REALTREE) ||
		    (worldItem == Z_FAKETREE))) {
		UnlockWorld();
		return;
	}

	if (IsBulldozable(worldItem)) {
		if (worldItem == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			SetWorld(WORLDPOS(xpos, ypos), (unsigned char)type);
			DrawCross(xpos, ypos, 1, 1);

			/*  update counter */
			if (IsRoad(type)) {
				vgame.BuildCount[bc_count_roads]++;
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
 */
void
Build_Road(Int16 xpos, Int16 ypos, welem_t type __attribute__((unused)))
{
	int old;
	unsigned long toSpend = 0;

	LockWorld();
	old = GetWorld(WORLDPOS(xpos, ypos));
	toSpend = BUILD_COST_ROAD;
	if (old == Z_POWERLINE || old == Z_PIPE) {
		welem_t tobuil = 0;
		switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos))) {
		case Z_POWERLINE: /* straight power line - Horizontal */
			tobuil = Z_POWERROAD_PHOR;
			break;
		case Z_POWERLINE+1: /* Straight power line - Vertical */
			tobuil = Z_POWERROAD_PVER;
			break;
		case Z_PIPE_START: /* Straight water pipe - Horizontal */
			tobuil = Z_PIPEROAD_PVER;
			break;
		case Z_PIPE_START+1: /* Straight water pipe - Vertical */
			tobuil = Z_PIPEROAD_PHOR;
			break;
		}
		if (SpendMoney(toSpend)) {
			SetWorld(WORLDPOS(xpos, ypos), tobuil);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_roads]++;
		} else {
			UIDisplayError(enOutOfMoney);
		}
	} else if (old == Z_REALWATER) {
		/* build a bridge */
		if (SpendMoney(toSpend)) {
			SetWorld(WORLDPOS(xpos, ypos), Z_BRIDGE);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_roads]++;
		} else {
			UIDisplayError(enOutOfMoney);
		}
	} else if (IsBulldozable(old)) {
		if (old == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			SetWorld(WORLDPOS(xpos, ypos), Z_ROAD);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_roads]++;
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
 */
static void
Build_PowerLine(Int16 xpos, Int16 ypos, welem_t type __attribute__((unused)))
{
	int old;
	unsigned long toSpend = 0;

	LockWorld();

	old = GetWorld(WORLDPOS(xpos, ypos));
	toSpend = BUILD_COST_POWER_LINE;
	if (IsBulldozable(old) || IsRoad(old) || IsPipe(old)) {
		if (IsRoad(old) || IsPipe(old)) {
			welem_t tobuil = 0;
			switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos))) {
			/* straight road - horizontal, vertical power line */
			case Z_ROAD:
				tobuil = Z_POWERROAD_PVER;
				break;
			case Z_ROAD+1: /* straight road - vertical */
				tobuil = Z_POWERROAD_PHOR;
				break;
			case Z_PIPE: /* straight pipe - horizontal */
				tobuil = Z_POWER_WATER_PVER;
				break;
			case Z_PIPE+1:
				tobuil = Z_POWER_WATER_PHOR;
				break;
			}
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos), tobuil);
				DrawCross(xpos, ypos, 1, 1);
				vgame.BuildCount[bc_powerlines]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
		} else {
			if (old == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos), Z_POWERLINE);
				DrawCross(xpos, ypos, 1, 1);
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
 */
static void
Build_WaterPipe(Int16 xpos, Int16 ypos, welem_t type __attribute__((unused)))
{
	int old;
	unsigned long toSpend = 0;
	welem_t elt = 0;

	LockWorld();

	toSpend = BUILD_COST_WATER_PIPE;
	old = GetWorld(WORLDPOS(xpos, ypos));
	if (IsBulldozable(old) || IsRoad(old) || IsPowerLine(old)) {
		if (IsRoad(old) || IsPowerLine(old)) {
			switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos))) {
			case Z_ROAD_START: /* straight road - Horizontal */
				elt = Z_PIPEROAD_PVER;
				break;
			case Z_ROAD_START+1: /* straight road - Vertical */
				elt = Z_PIPEROAD_PHOR;
				break;
			case Z_POWERLINE_START: /* powerline - Horizontal */
				elt = Z_POWER_WATER_PHOR;
				break;
			case Z_POWERLINE_START+1: /* powerline - Vertical */
				elt = Z_POWER_WATER_PVER;
				break;
			}
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos), elt);
				DrawCross(xpos, ypos, 1, 1);
				vgame.BuildCount[bc_waterpipes]++;
			} else {
				UIDisplayError(enOutOfMoney);
			}
		} else {
			if (old == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
			if (SpendMoney(toSpend)) {
				SetWorld(WORLDPOS(xpos, ypos), Z_PIPE);
				DrawCross(xpos, ypos, 1, 1);
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
	if (howMuch > (UInt32)getCredits())
		return (0);

	WriteLog("Spend Money: %ld\n", howMuch);
	decCredits(howMuch);

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
	j = GetRandomNumber(GetMapWidth());
	axis = GetRandomNumber(1);
	LockWorld();

	for (i = 0; i < GetMapWidth(); i++) {
		for (k = j; k < (width + j); k++) {
			if ((k > 0) && (k < GetMapWidth())) {
				if (axis)
					SetWorld(WORLDPOS(i, k),
					    Z_REALWATER);
				else
					SetWorld(WORLDPOS(k, i),
					    Z_REALWATER);
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
			if (j < GetMapWidth())
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
		pos = GetRandomNumber(MapMul());
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

	x = pos % GetMapWidth();
	y = pos / GetMapWidth();
	LockWorld();
	i = x;
	j = y;

	for (i = x - size; i <= x + size; i++) {
		for (j = y - size; j <= y + size; j++) {
			if (i < 0 || i >= GetMapWidth() || j < 0 ||
			    j >= GetMapWidth())
				continue;
			if (GetWorld(WORLDPOS(i, j)) != Z_DIRT)
				continue;
			s = ((y > j) ? (y - j) : (j - y)) +
			    ((x > i) ? (x - i) : (i - x));
			if (GetRandomNumber(s) < 2) {
				/*! \todo count_trees or count_real_trees */
				SetWorld(WORLDPOS(i, j), Z_REALTREE);
				vgame.BuildCount[bc_count_trees]++;
			}
		}
	}
	UnlockWorld();
}
