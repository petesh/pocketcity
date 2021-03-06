
/*!
 * \file
 * \brief Code that deals with the building of items in the simulation.
 *
 * It is controlled by the table buildStructure, which defines all
 * the items that can be built.
 */

#include <config.h>

#include <simcity.h>

#include <build.h>
#include <globals.h>
#include <ui.h>
#include <logging.h>
#include <locking.h>
#include <drawing.h>
#include <simulation.h>

/*!
 * \brief Defines the build function pointer type
 */
typedef int (*BuildF)(UInt16 xpos, UInt16 ypos, welem_t type);

static int Build_Road(UInt16 xpos, UInt16 ypos, welem_t type) BUILD_SECTION;
static int Build_Rail(UInt16 xpos, UInt16 ypos, welem_t type) BUILD_SECTION;
static int Build_PowerLine(UInt16 xpos, UInt16 ypos, welem_t type)
	BUILD_SECTION;
static int Build_WaterPipe(UInt16 xpos, UInt16 ypos, welem_t type)
	BUILD_SECTION;
static int Build_Generic(UInt16 xpos, UInt16 ypos, welem_t type)
    BUILD_SECTION;
static int Build_Generic4(UInt16 xpos, UInt16 ypos, welem_t type)
    BUILD_SECTION;
static int Build_Plant(UInt16 xpos, UInt16 ypos, welem_t type)
    BUILD_SECTION;
static int Build_Defence(UInt16 xpos, UInt16 ypos, welem_t type)
    BUILD_SECTION;

static void RemoveDefence(UInt16 xpos, UInt16 ypos) BUILD_SECTION;
static int CantBulldoze(welem_t type) BUILD_SECTION;
static UInt16 blockSize(welem_t type) BUILD_SECTION;
static void Doff(welem_t base, welem_t node, UInt16 *x, UInt16 *y)
	BUILD_SECTION;
static Int16 IsBulldozable(welem_t zone) BUILD_SECTION;

static Int16 SpendMoney(UInt32 howMuch) BUILD_SECTION;

/*!
 * \brief array mapping build requests to reactions.
 *
 * This structure is used to map the BuildCode to an appropriate function
 * to perform the building as well as some extra events to happen as a
 * consequence of the item being built.
 */
static const struct _bldStruct {
	/*! The code of the item to be built. */
	BuildCode bt;
	/*! Function to call. */
	BuildF func;
	/*! Type of the item (for buildcounts) */
	welem_t type;
	/*! Grids to update as a result of adding the item */
	UInt8 gridsToUpdate;
} buildStructure[] = {
	{ Be_Bulldozer, Build_Bulldoze, 0, GRID_ALL },
	{ Be_Zone_Residential, Build_Generic, Z_RESIDENTIAL_SLUM, GRID_ALL },
	{ Be_Zone_Commercial, Build_Generic, Z_COMMERCIAL_SLUM, GRID_ALL},
	{ Be_Zone_Industrial, Build_Generic, Z_INDUSTRIAL_SLUM, GRID_ALL},
	{ Be_Road, Build_Road, 0, 0 },
	{ Be_Rail, Build_Rail, 0, 0 },
	{ Be_Power_Plant, Build_Plant, Z_COALPLANT, GRID_ALL },
	{ Be_Nuclear_Plant, Build_Plant, Z_NUCLEARPLANT, GRID_ALL },
	{ Be_Power_Line, Build_PowerLine, 0, GRID_ALL },
	{ Be_Water_Pump, Build_Generic, Z_PUMP, GRID_ALL },
	{ Be_Water_Pipe, Build_WaterPipe, 0, GRID_WATER },
	{ Be_Tree, Build_Generic, Z_FAKETREE, 0 },
	{ Be_Water, Build_Generic, Z_FAKEWATER, 0 },
	{ Be_Fire_Station, Build_Generic4, Z_FIRESTATION, GRID_ALL },
	{ Be_Police_Station, Build_Generic4, Z_POLICEDEPT, GRID_ALL },
	{ Be_Military_Base, Build_Generic4, Z_ARMYBASE, GRID_ALL },
	{ Be_OOB, NULL, 0, 0 },
	{ Be_Defence_Fire, Build_Defence, DuFireman, 0 },
	{ Be_Defence_Police, Build_Defence, DuPolice, 0 },
	{ Be_Defence_Military, Build_Defence, DuMilitary, 0 },
};

/*!
 * \brief the build structure list length
 */
#define	BS_LEN	((sizeof (buildStructure) / sizeof (buildStructure[0])))

/*!
 * Looks up the item in the buildStructure and builds it at the appropriate
 * location.
 */
int
BuildSomething(UInt16 xpos, UInt16 ypos)
{
	UInt16 item = UIGetSelectedBuildItem();
	struct _bldStruct *be;

	if (item >= BS_LEN) {
		UISystemErrorNotify(seUnknownBuildItem);
		return (0);
	}
	be = (struct _bldStruct *)&(buildStructure[item]);

	if (be->bt == Be_OOB)
		return (0);

	if (be->func(xpos, ypos, be->type)) {
		welem_t elt;
		zone_lock(lz_world);
		elt = GetGraphicNumber(WORLDPOS(xpos, ypos));
		zone_unlock(lz_world);
		AddGridUpdate(be->gridsToUpdate);
		UIPaintMapField(xpos, ypos, elt);
		UIPaintMapStatus(xpos, ypos, elt, 0);
		return (1);
	}
	return (0);
}

/*!
 * \brief Remove a defence unit from a location
 *
 * Removes the defence item from the location specified.
 * \param xpos the X position on the map
 * \param ypos the Y position on the map
 */
static void
RemoveDefence(UInt16 xpos, UInt16 ypos)
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

void
RemoveAllDefence(void)
{
	int i;

	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (game.units[i].active != 0) {
			game.units[i].active = 0;
			DrawCross(game.units[i].x, game.units[i].y, 1, 1);
		}
	}
}

/*!
 * \brief Maps build units to locations in the build array
 */
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
static int
Build_Defence(UInt16 xpos, UInt16 ypos, welem_t type)
{
	UInt16 oldx;
	UInt16 oldy;
	UInt16 i;
	Int16 sel = -1;
	UInt16 newactive = 1;
	UInt16 start;
	UInt16 end;
	UInt16 max;
	UInt16 nCounter;
	const struct buildCounters *cnt;
	int rv = 0;

	if (type > DuMilitary)
		return (rv);

	cnt = &counters[type];

	/* this is here to make sure not too many of any item are created */
	nCounter = cnt->counter;

	if (vgame.BuildCount[nCounter] == 0)
		return (rv); /* no special building */

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
				return (rv);
			game.units[i].active = 0;
		}
	}

	/* find an empty slot for the new defence unit */
	for (i = start; i <= max; i++) {
		if (game.units[i].active == 0) {
			sel = (Int16)i;
			break;
		}
	}
	if (sel == -1) {
		/* none found - start from the beginning */
		for (i = start; i <= max; i++) {
			if (game.units[i].active == 1) {
				sel = (Int16)i;
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
		sel = (Int16)start;
		newactive = 2;
	}

	oldx = game.units[sel].x;
	oldy = game.units[sel].y;

	game.units[sel].x = xpos;
	game.units[sel].y = ypos;
	game.units[sel].active = newactive;
	game.units[sel].type = (DefenceUnitTypes)type;

	zone_lock(lz_world);
	rv = 1;
	DrawCross(oldx, oldy, 1, 1);
	DrawCross(xpos, ypos, 1, 1);
	zone_unlock(lz_world);
	return (rv);
}

/*! \brief check that I can't bulldoze a zone */
static int
CantBulldoze(welem_t type)
{
	return (type == Z_DIRT || type == Z_FIRE1 || type == Z_FIRE2 ||
	    type == Z_FIRE3 || type == Z_REALWATER || type == Z_CRATER);
}

/*!
 * \brief get the blocksize of the item in question (1x1, 2x2)
 * \param type the item to check
 * \return the block size
 * \todo permit non-square sizes (return isn't a plain integer)
 */
static UInt16
blockSize(welem_t type)
{
	/* This works because all the multiblocks are sequential */
	if (type >= Z_COALPLANT_START && type <= Z_ARMYBASE_END)
		return (4);
	return (1);
}

int
Build_Bulldoze(UInt16 xpos, UInt16 ypos,
    welem_t _type __attribute__((unused)))
{
	int rv = 0;
	welem_t type;

	zone_lock(lz_world);
	type = getWorld(WORLDPOS(xpos, ypos));

	WriteLog("BuildBulldoze(type=%d)\n", (int)type);
	if (CantBulldoze(type)) {
		RemoveDefence(xpos, ypos);
		goto end;
	}
	if (SpendMoney(BUILD_COST_BULLDOZER * blockSize(type))) {
		Build_Destroy(xpos, ypos);
		rv = 1;
	} else {
		UIProblemNotify(peOutOfMoney);
	}
end:
	zone_unlock(lz_world);
	return (rv);
}

/*!
 * \brief Determine the offsetting for destroying a node - 2x2 *only*
 * \param base the value of the base node type
 * \param node the node's value
 * \param x the x position
 * \param y the y position
 */
static void
Doff(welem_t base, welem_t node, UInt16 *x, UInt16 *y)
{
	*x -= (UInt16)((node - base) % 2);
	*y -= (UInt16)((node - base) / 2);
}

void
Build_Destroy(UInt16 xpos, UInt16 ypos)
{
	welem_t type;
	/* Destroy a 1x1 square area by default */
	UInt16 x_destroy = 1;
	UInt16 tx_destroy = 1;
	UInt16 y_destroy = 1;
	UInt16 ty_destroy = 1;
	lsObj_t *affected = NULL;
	UInt32 pos = WORLDPOS(xpos, ypos);

	type = getWorld(pos);
	RemoveDefence(xpos, ypos);

	if (IsCommercial(type)) {
		vgame.BuildCount[bc_count_commercial]--;
		vgame.BuildCount[bc_value_commercial] -= ZoneValue(type);
		goto finish;
	}
	if (IsResidential(type)) {
		vgame.BuildCount[bc_count_residential]--;
		vgame.BuildCount[bc_value_residential] -= ZoneValue(type);
		goto finish;
	}
	if (IsIndustrial(type)) {
		vgame.BuildCount[bc_count_industrial]--;
		vgame.BuildCount[bc_value_industrial] -= ZoneValue(type);
		goto finish;
	}
	if (IsFakeTree(type)) {
		vgame.BuildCount[bc_count_trees]--;
		goto finish;
	}
	if (IsFakeWater(type)) {
		vgame.BuildCount[bc_water]--;
		goto finish;
	}
	if (IsWaste(type)) {
		vgame.BuildCount[bc_waste]--;
		goto finish;
	}
	if (type >= Z_FIRE1 && type <= Z_FIRE3) {
		vgame.BuildCount[bc_fire]--;
		goto finish;
	}

	if (IsCoalPlant(type)) {
		vgame.BuildCount[bc_coalplants]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_COALPLANT_START, type, &xpos, &ypos);
		pos = WORLDPOS(xpos, ypos);
		affected = vgame.powers;
		goto finish;
	}
	if (IsNukePlant(type)) {
		vgame.BuildCount[bc_nuclearplants]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_NUCLEARPLANT_START, type, &xpos, &ypos);
		pos = WORLDPOS(xpos, ypos);
		affected = vgame.powers;
		goto finish;
	}
	if (IsFireStation(type)) {
		vgame.BuildCount[bc_fire_stations]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_FIRESTATION_START, type, &xpos, &ypos);
		pos = WORLDPOS(xpos, ypos);
		goto finish;
	}
	if (IsPoliceDept(type)) {
		vgame.BuildCount[bc_police_departments]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_POLICEDEPT_START, type, &xpos, &ypos);
		pos = WORLDPOS(xpos, ypos);
		goto finish;
	}
	if (IsArmyBase(type)) {
		vgame.BuildCount[bc_military_bases]--;
		x_destroy = 2;
		y_destroy = 2;
		Doff(Z_ARMYBASE_START, type, &xpos, &ypos);
		pos = WORLDPOS(xpos, ypos);
		goto finish;
	}
	if (IsPump(type)) {
		vgame.BuildCount[bc_waterpumps]--;
		affected = vgame.waters;
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
	if (IsRoadPipe(type)) {
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

	if (IsRail(type)) {
		vgame.BuildCount[bc_count_rail]--;
		vgame.BuildCount[bc_value_rail] -= ZoneValue(type);
	}
	if (IsRailPower(type)) {
		vgame.BuildCount[bc_count_rail]--;
		vgame.BuildCount[bc_value_rail] -= ZoneValue(type);
		vgame.BuildCount[bc_powerlines]--;
	}
	if (IsRailPipe(type)) {
		vgame.BuildCount[bc_count_rail]--;
		vgame.BuildCount[bc_value_rail] -= ZoneValue(type);
		vgame.BuildCount[bc_waterpipes]--;
	}
	if (IsRailOvRoad(type)) {
		vgame.BuildCount[bc_count_rail]--;
		vgame.BuildCount[bc_value_rail] -= ZoneValue(type);
		vgame.BuildCount[bc_count_roads]--;
		vgame.BuildCount[bc_value_roads] -= ZoneValue(type);
	}

finish:
	AddGridUpdate(GRID_ALL);

	zone_lock(lz_flags);

	if (affected != NULL) {
		UInt32 llen = ListNElements(affected);
		UInt32 i;

		for (i = 0; i < llen; i++) {
			if ((UInt32)ListGet(affected, i) == pos) {
				ListRemove(affected, i);
				break;
			}
		}
	}

	if (IsRoadBridge(type) || IsRailTunnel(type) || IsRealWater(type)) {
		/* A bridge turns into real_water when destroyed */
		setWorldAndFlag(pos, Z_REALWATER, 0);
	} else {
		if ((x_destroy != 1) || (y_destroy != 1)) {
			ty_destroy = y_destroy;
			while (ty_destroy) {
				tx_destroy = x_destroy;
				while (tx_destroy) {
					setWorldAndFlag(
					    WORLDPOS(xpos - 1 + tx_destroy,
					    ypos - 1 + ty_destroy), Z_DIRT, 0);
					tx_destroy--;
				}
				ty_destroy--;
			}
		} else {
			setWorldAndFlag(pos, Z_DIRT, 0);
		}
	}

	DrawCross(xpos, ypos, x_destroy, y_destroy);
	zone_unlock(lz_flags);
}

/*! \brief Mapping of zone to cost of building on a zone */
static const struct _costMappings {
	/*! type of zone that is built */
	UInt16 type;
	/*! cost of building the zone in question */
	UInt32 cost;
	/*! counter to affect as a result of building this */
	Int16 count;
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
	    ((zone == Z_REALTREE) && GETAUTOBULLDOZE()));
}

/*!
 * \brief Build a generic 4-zone item
 *
 * type to build is based on the type passed in
 * \param xpos the xposition to build the item at
 * \param ypos the yposition to build the item at
 * \param type type of item to be built
 * \param add the list of suppliers to add to
 */
static int
Build_Generic4_add(UInt16 xpos, UInt16 ypos, welem_t type, lsObj_t *add)
{
	welem_t worldItem;
	unsigned long toSpend = 0;
	UInt16 loopx, loopy;
	Int8 canbuild = 1;
	int rv = 0;
	struct _costMappings *cmi = (struct _costMappings *)getIndexOf(
	    (char *)&genericMappings[0], sizeof (genericMappings[0]), type);

#if defined (DEBUG)
#ifdef PALM
	ErrFatalDisplayIf(cmi == NULL, "No generic->item mapping");
#else
	assert(cmi != NULL);
#endif
#endif
	if (cmi == NULL)
		return (rv);
	zone_lock(lz_world);
	zone_lock(lz_flags);

	toSpend = cmi->cost;

	for (loopx = xpos; loopx < xpos + 2; loopx++) {
		for (loopy = ypos; loopy < ypos + 2; loopy++) {
			worldItem = getWorld(WORLDPOS(loopx, loopy));
			if (IsBulldozable(worldItem)) {
				if (worldItem == Z_REALTREE)
					toSpend += BUILD_COST_BULLDOZER;
			} else {
				canbuild = 0;
			}
		}
	}
	if (!canbuild) {
		zone_lock(lz_flags);
		zone_unlock(lz_world);
		return (rv);
	}

	if (SpendMoney(toSpend)) {
		for (loopy = ypos; loopy < ypos + 2; loopy++) {
			for (loopx = xpos; loopx < xpos + 2; loopx++)
				setWorldAndFlag(WORLDPOS(loopx, loopy),
				    type++, 0);
			}

		if (cmi->count != -1)
			vgame.BuildCount[cmi->count]++;
		if (add != NULL)
			ListAdd(add, WORLDPOS(xpos, ypos));

		DrawCross(xpos, ypos, 2, 2);
		rv = 1;
	} else {
		UIProblemNotify(peOutOfMoney);
	}
	zone_unlock(lz_flags);
	zone_unlock(lz_world);
	return (rv);
}

/*!
 * \brief built a generic 4 block item that doesn't affect the power
 * \param xpos the x position on the map
 * \param ypos the y position on the map
 * \param type the type of item to build
 * \return success or failure condition
 */
static int
Build_Generic4(UInt16 xpos, UInt16 ypos, welem_t type)
{
	return (Build_Generic4_add(xpos, ypos, type, NULL));
}

/*!
 * \brief Build a power plant / water pump
 *
 * type to build is based on the type passed in
 * \param xpos the xposition to build the item at
 * \param ypos the yposition to build the item at
 * \param type type of item to be built
 */
static int
Build_Plant(UInt16 xpos, UInt16 ypos, welem_t type)
{
	lsObj_t *add = vgame.powers;

	return (Build_Generic4_add(xpos, ypos, type, add));
}

/*!
 * \brief Build a generic item
 *
 * type to build is based on the type passed in
 * \param xpos the xposition to build the item at
 * \param ypos the yposition to build the item at
 * \param type type of item to be built
 */
static int
Build_Generic(UInt16 xpos, UInt16 ypos, welem_t type)
{
	welem_t worldItem;
	UInt32 toSpend = 0;
	int rv = 0;
	UInt32 pos = WORLDPOS(xpos, ypos);

	struct _costMappings *cmi = (struct _costMappings *)getIndexOf(
	    (char *)&genericMappings[0], sizeof (genericMappings[0]), type);
#if defined(DEBUG)
#ifdef PALM
	ErrFatalDisplayIf(cmi == NULL, "No generic->item mapping");
#else
	assert(cmi != NULL);
#endif
#endif
	if (cmi == NULL)
		return (rv);
	zone_lock(lz_world);
	zone_lock(lz_flags);

	toSpend = cmi->cost;

	worldItem = getWorld(pos);

	/* Don't build trees over trees (auto bulldoze) */
	if ((type == Z_FAKETREE) && ((worldItem == Z_REALTREE) ||
		    (worldItem == Z_FAKETREE))) {
		zone_unlock(lz_world);
		zone_unlock(lz_flags);
		return (rv);
	}

	if (IsBulldozable(worldItem)) {
		if (worldItem == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(pos, (welem_t)type, 0);
			DrawCross(xpos, ypos, 1, 1);

			/*  update counter */
			if (IsRoad(type)) {
				vgame.BuildCount[bc_count_roads]++;
			} else {
				if (cmi->count != -1)
					vgame.BuildCount[cmi->count]++;
				if (IsPump(type))
					ListAdd(vgame.waters, pos);
			}
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	}
	zone_unlock(lz_flags);
	zone_unlock(lz_world);
	return (rv);
}

/*!
 * \brief Build a road.
 *
 * Auto bulldoze if requested
 * \param xpos xposition to build the road at
 * \param ypos yposition to build the road at
 * \param type unused in this context
 */
static int
Build_Road(UInt16 xpos, UInt16 ypos, welem_t type __attribute__((unused)))
{
	welem_t old;
	UInt32 toSpend = 0;
	int rv = 0;

	zone_lock(lz_world);
	zone_lock(lz_flags);
	old = getWorld(WORLDPOS(xpos, ypos));
	toSpend = BUILD_COST_ROAD;
	if (IsPowerLine(old) || IsWaterPipe(old) || IsRail(old)) {
		welem_t tobuil = 0;
		switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos))) {
		case Z_POWERLINE: /* straight power line - Horizontal */
			tobuil = Z_POWERROAD_PHOR;
			break;
		case Z_POWERLINE+1: /* Straight power line - Vertical */
			tobuil = Z_POWERROAD_PVER;
			break;
		case Z_PIPE_START: /* Straight water pipe - Horizontal */
			tobuil = Z_PIPEROAD_PHOR;
			break;
		case Z_PIPE_START+1: /* Straight water pipe - Vertical */
			tobuil = Z_PIPEROAD_PVER;
			break;
		case Z_RAIL_START: /* Straight rail line - Horizontal */
			tobuil = Z_RAILOVROAD_RHOR;
			break;
		case Z_RAIL_START+1: /* Straight rail line - Vertical */
			tobuil = Z_RAILOVROAD_RVER;
			break;
		}
		if (tobuil == 0)
			goto leaveme;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), tobuil, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_roads]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	} else if (IsRealWater(old)) {
		welem_t tobuil = 0;
		UInt8 check_rd;
		UInt8 check_br;
		UInt32 wp = WORLDPOS(xpos, ypos);
		check_rd = CheckNextTo(wp, DIR_ALL, IsRoad);
		check_br = CheckNextTo(wp, DIR_ALL, IsRoadBridge);

		if ((check_rd == 0) && (check_br == 0))
			goto leaveme;
		/*
		 * build a bridge only if one of the squares around is
		 * either a bridge or a road.
		 */
		if ((check_rd & DIR_UP) || (check_rd & DIR_DOWN)) {
			tobuil = Z_BRIDGE_VER;
			goto success_build;
		}
		if ((check_rd & DIR_LEFT) || (check_rd & DIR_RIGHT)) {
			tobuil = Z_BRIDGE_HOR;
			goto success_build;
		}
		if (((check_br & DIR_LEFT) &&
		    (getWorld(WORLDPOS(xpos - 1, ypos)) == Z_BRIDGE_HOR)) ||
		    ((check_br & DIR_RIGHT) &&
		    (getWorld(WORLDPOS(xpos + 1, ypos)) == Z_BRIDGE_HOR))) {
			tobuil = Z_BRIDGE_HOR;
		} else if (((check_br & DIR_UP) &&
		    (getWorld(WORLDPOS(xpos, ypos - 1)) == Z_BRIDGE_VER)) ||
		    ((check_br & DIR_DOWN) &&
		    (getWorld(WORLDPOS(xpos, ypos + 1)) == Z_BRIDGE_VER))) {
			tobuil = Z_BRIDGE_VER;
		}
		if (tobuil == 0)
			goto leaveme;
		toSpend = BUILD_COST_BRIDGE;
success_build:
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), tobuil, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_roads]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	} else if (IsBulldozable(old)) {
		if (old == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), Z_ROAD, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_roads]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	}
leaveme:
	zone_unlock(lz_flags);
	zone_unlock(lz_world);
	return (rv);
}

/*!
 * \brief Build a rail line.
 *
 * Auto bulldoze if requested
 * \param xpos xposition to build the rail line at
 * \param ypos yposition to build the rail line at
 * \param type unused in this context
 */
static int
Build_Rail(UInt16 xpos, UInt16 ypos, welem_t type __attribute__((unused)))
{
	welem_t old;
	UInt32 toSpend = 0;
	int rv = 0;

	zone_lock(lz_world);
	zone_lock(lz_flags);
	old = getWorld(WORLDPOS(xpos, ypos));
	toSpend = BUILD_COST_RAIL;
	if (IsPowerLine(old) || IsWaterPipe(old) || IsRoad(old)) {
		welem_t tobuil = 0;
		switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos))) {
		case Z_POWERLINE: /* straight power line - Horizontal */
			tobuil = Z_RAILPOWER_RVER;
			break;
		case Z_POWERLINE+1: /* Straight power line - Vertical */
			tobuil = Z_RAILPOWER_RHOR;
			break;
		case Z_PIPE_START: /* Straight water pipe - Horizontal */
			tobuil = Z_RAILPIPE_RVER;
			break;
		case Z_PIPE_START+1: /* Straight water pipe - Vertical */
			tobuil = Z_RAILPIPE_RHOR;
			break;
		case Z_ROAD_START: /* Straight road - Horizontal */
			tobuil = Z_RAILOVROAD_RVER;
			break;
		case Z_ROAD_START+1: /* Straight road - Vertical */
			tobuil = Z_RAILOVROAD_RHOR;
			break;
		}
		if (tobuil == 0)
			goto leaveme;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), tobuil, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_rail]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	} else if (IsRealWater(old)) {
		welem_t tobuil = 0;
		UInt8 check_rd;
		UInt8 check_br;
		UInt32 wp = WORLDPOS(xpos, ypos);
		check_rd = CheckNextTo(wp, DIR_ALL, IsRail);
		check_br = CheckNextTo(wp, DIR_ALL, IsRailTunnel);

		if ((check_rd == 0) && (check_br == 0))
			goto leaveme;
		/*
		 * build a tunnel only if one of the squares around is
		 * either a tunnel or a rail line.
		 */
		if ((check_rd & DIR_UP) || (check_rd & DIR_DOWN)) {
			tobuil = Z_RAILTUNNEL_RVER;
			goto success_build;
		}
		if ((check_rd & DIR_LEFT) || (check_rd & DIR_RIGHT)) {
			tobuil = Z_RAILTUNNEL_RHOR;
			goto success_build;
		}
		if (((check_br & DIR_LEFT) &&
		    (getWorld(WORLDPOS(xpos - 1, ypos)) ==
			Z_RAILTUNNEL_RHOR)) || ((check_br & DIR_RIGHT) &&
		    (getWorld(WORLDPOS(xpos + 1, ypos)) ==
			Z_RAILTUNNEL_RHOR))) {
			tobuil = Z_RAILTUNNEL_RHOR;
		} else if (((check_br & DIR_UP) &&
		    (getWorld(WORLDPOS(xpos, ypos - 1)) ==
			Z_RAILTUNNEL_RVER)) || ((check_br & DIR_DOWN) &&
		    (getWorld(WORLDPOS(xpos, ypos + 1)) ==
			Z_RAILTUNNEL_RVER))) {
			tobuil = Z_RAILTUNNEL_RVER;
		}
		if (tobuil == 0)
			goto leaveme;
		toSpend = BUILD_COST_RAILTUNNEL;
success_build:
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), tobuil, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_rail]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	} else if (IsBulldozable(old)) {
		if (old == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), Z_RAIL, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_count_rail]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	}
leaveme:
	zone_unlock(lz_world);
	zone_unlock(lz_flags);
	return (rv);
}

/*!
 * \brief Build a power line.
 *
 * Auto bulldoze if requested.
 * \param xpos x position on map to build on
 * \param ypos y position on map to build on
 * \param type unused for this function
 */
static int
Build_PowerLine(UInt16 xpos, UInt16 ypos, welem_t type __attribute__((unused)))
{
	welem_t old;
	UInt32 toSpend = 0;
	int rv = 0;

	zone_lock(lz_world);
	zone_lock(lz_flags);

	old = getWorld(WORLDPOS(xpos, ypos));
	toSpend = BUILD_COST_POWER_LINE;
	if (IsBulldozable(old)) {
		if (old == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), Z_POWERLINE, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_powerlines]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	}
	if (IsRoad(old) || IsWaterPipe(old) || IsRail(old)) {
		welem_t tobuil = 0;
		switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos))) {
		/* straight road - horizontal, vertical power line */
		case Z_ROAD_START:
			tobuil = Z_POWERROAD_PVER;
			break;
		case Z_ROAD_START+1: /* straight road - vertical */
			tobuil = Z_POWERROAD_PHOR;
			break;
		case Z_PIPE_START: /* straight pipe - horizontal */
			tobuil = Z_POWER_WATER_PVER;
			break;
		case Z_PIPE_START+1:
			tobuil = Z_POWER_WATER_PHOR;
			break;
		case Z_RAIL_START: /* straight rail - horizontal */
			tobuil = Z_RAILPOWER_RHOR;
			break;
		case Z_RAIL_START+1: /* straight rail - vertical */
			tobuil = Z_RAILPOWER_RVER;
			break;
		}
		if (tobuil == 0)
			goto leaveme;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), tobuil, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_powerlines]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	}
leaveme:
	zone_unlock(lz_world);
	zone_unlock(lz_flags);
	return (rv);
}

/*!
 * \brief Build a water pipe.
 *
 * Auto bulldoze as needed
 * \param xpos x position on map to build on
 * \param ypos y position on map to build on
 * \param type unused for this function
 */
static int
Build_WaterPipe(UInt16 xpos, UInt16 ypos, welem_t type __attribute__((unused)))
{
	welem_t old;
	UInt32 toSpend = 0;
	welem_t elt = 0;
	int rv = 0;

	zone_lock(lz_world);
	zone_lock(lz_flags);

	toSpend = BUILD_COST_WATER_PIPE;
	old = getWorld(WORLDPOS(xpos, ypos));
	if (IsBulldozable(old)) {
		if (old == Z_REALTREE) toSpend += BUILD_COST_BULLDOZER;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), Z_PIPE, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_waterpipes]++;
			rv = 1;
			goto leaveme;
		} else {
			UIProblemNotify(peOutOfMoney);
			goto leaveme;
		}
	}
	if (IsRoad(old) || IsPowerLine(old) || IsRail(old)) {
		switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos))) {
		case Z_ROAD_START: /* straight road - Horizontal */
			elt = Z_PIPEROAD_PVER;
			break;
		case Z_ROAD_START+1: /* straight road - Vertical */
			elt = Z_PIPEROAD_PHOR;
			break;
		case Z_RAIL_START: /* straight rail - Horizontal */
			elt = Z_RAILPIPE_RHOR;
			break;
		case Z_RAIL_START+1: /* rail - Vertical */
			elt = Z_RAILPIPE_RVER;
			break;
		case Z_POWERLINE_START: /* powerline - Horizontal */
			elt = Z_POWER_WATER_PHOR;
			break;
		case Z_POWERLINE_START+1: /* powerline - Vertical */
			elt = Z_POWER_WATER_PVER;
			break;
		}
		if (elt == 0)
			goto leaveme;
		if (SpendMoney(toSpend)) {
			setWorldAndFlag(WORLDPOS(xpos, ypos), elt, 0);
			DrawCross(xpos, ypos, 1, 1);
			vgame.BuildCount[bc_waterpipes]++;
			rv = 1;
		} else {
			UIProblemNotify(peOutOfMoney);
		}
	}
leaveme:
	zone_unlock(lz_flags);
	zone_unlock(lz_world);
	return (rv);
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
	decCredits((Int32)howMuch);

	/* now redraw the credits */
	addGraphicUpdate(gu_credits);
	return (1);
}

