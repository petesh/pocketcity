/*! \file
 * \brief the simulation routines
 *
 * This consists of the outines that do all the simulation work, for example
 * deciding that certain zones are to be improved or deteriorated, where
 * monsters go to; etc,
 */

#include <handler.h>
#include <drawing.h>
#include <zakdef.h>
#include <ui.h>
#include <globals.h>
#include <handler.h>
#include <disaster.h>
#include <simulation.h>
#include <stack.h>
#include <compilerpragmas.h>
#include <mem_compat.h>

/*! \brief short and out bits */
#define	SHORT_BIT	1
#define	OUT_BIT		2

#define DONTPAINT	(unsigned char)(1U<<7)

/*! \brief Structure for performing distribution */
typedef struct _distrib {
	carryfn_t	doescarry; /*!< Does the node carry the item */
	/*! Is the node a supplier */
	problem_t	error_flag; /*!< Error flag */
	Int16 (*isplant)(welem_t, UInt32, selem_t);
	void *needSourceList; /*!< list of nodes that need to be powered */
	void *unvisitedNodes; /*!< nodes that have not been visited */
	selem_t flagToSet; /*!< flag that is being set in this loop */
	Int16 SourceLeft; /*!< amount of source left */
	Int16 SourceTotal; /*!< total source available */
	Int16 NodesTotal; /*!< nodes visited */
	Int16 NodesSupplied; /*!< nodes supplied */
	Int16 ShortOrOut; /*!< was short or out of the chosen item */
} distrib_t;

static void DoTaxes(void);
static void DoUpkeep(void);
static Int16 DistributeNumberOfSquaresAround(distrib_t *distrib, UInt32 pos);

static void reGradeZones(void);
static void UpgradeZone(UInt32 pos);
static void DowngradeZone(UInt32 pos);
static Int16 DoTheRoadTrip(UInt32 startPos);
static UInt32 DistributeMoveOn(UInt32 pos, dirType direction);
static void DistributeUnvisited(distrib_t *distrib);

static long GetZoneScore(UInt32 pos);
static Int16 GetScoreFor(zoneType iamthis, welem_t what);
static Int32 GetRandomZone(void);
static void FindZonesForUpgrading(void);
static Int16 FindScoreForZones(void);
static void AddNeighbors(distrib_t *distrib, UInt32 pos);
static UInt8 ExistsNextto(UInt32 pos, UInt8 dirs, welem_t what);

/*
 * The power/water grid is updated using the following mechanism:
 * 1: While there are more plants to consume do:
 * a: While you can visit more points do:
 * i: If it's a source then add it's donation to the supply
 *	  - if the 'to be powered' list has members then supply them
 *	  - go to step (1.a.iii)
 * ii: if you've power then mark the point as visited, supplied & decrement
 *	  - otherwise put it on the 'to be powered' list
 * iii: Find all the possible connections to visit and put them on the trip list
 * b: Purge the 'to be powered list'; they can't be b/c of no connections
 *
 * The WorldFlag are used as a bitfield for this, every tile has a byte:
 *  8765 4321
 *  |||| |||`- 1 = this tile is powered
 *  |||| ||`-- 1 = this tile is watered
 *  |||| |`---
 *  |||| `----
 *  |||`------
 *  ||`-------
 *  |`--------
 *  `--------- 1 = Scratch / Visited
 *
 *  don't use any of the free flags without asking (thanks)
 *  please note that the flags are saved, adding an extra 10k to the savegame
 *  structure. It could be collapsed during save to the number of bits
 *  that are occupied, but that seems like too much effort.
 *
 *  How to recreate:
 *	  call the distribution routine... it knows how to do each type
 */
static void DoDistribute(Int16 grid);

void
Sim_Distribute(void)
{
	int paint = 0;
	if (NeedsUpdate(GRID_POWER)) {
		paint = 1;
		DoDistribute(GRID_POWER | DONTPAINT);
		ClearUpdate(GRID_POWER);
	}
	if (NeedsUpdate(GRID_WATER)) {
		paint = 1;
		DoDistribute(GRID_WATER | DONTPAINT);
		ClearUpdate(GRID_WATER);
	}
	if (paint)
		UIDrawPlayArea();
}

/*!
 * \brief Do a grid distribution for the grid(s) specified
 * \param gridonly the grid to do
 */
static void
Sim_Distribute_Specific(Int16 gridonly)
{
	if (gridonly == 0) gridonly = GRID_ALL;

	if ((gridonly & GRID_POWER)) {
		DoDistribute(GRID_POWER);
		ClearUpdate(GRID_POWER);
	}
	if ((gridonly & GRID_WATER)) {
		DoDistribute(GRID_WATER);
		ClearUpdate(GRID_WATER);
	}
	if (!(gridonly & DONTPAINT))
		UIDrawPlayArea();
}

/*!
 * \brief Check if the node is a power plant (Nuclear/Coal)
 * \param point the value at the point
 * \param coord unused
 * \param flags unused
 * \return wheter it is one or not.
 */
static Int16
IsItAPowerPlant(welem_t point, UInt32 coord __attribute__((unused)),
    selem_t flags __attribute__((unused)))
{
	switch (point) {
	case Z_COALPLANT_START:
		return (SUPPLY_POWER_PLANT);
	case Z_NUCLEARPLANT_START:
		return (SUPPLY_NUCLEAR_PLANT);
	default:
		return (0);
	}
}

/*!
 * \brief Check if the node is a Water Pump.
 *
 * It is a usable water pump if it has power.
 * \param point node to test
 * \param coord coordiate of the point
 * \param flags flag byte of point.
 * \return if it is a good water pump.
 *
 */
static Int16
IsItAWaterPump(welem_t point, UInt32 coord, selem_t flags)
{
	if ((point == Z_PUMP) && (flags & POWEREDBIT) &&
	    ExistsNextto(coord, DIR_ALL, Z_REALWATER))
		return (SUPPLY_WATER_PUMP);
	return (0);
}

/*!
 * \brief Set the supplied bit for the point specified
 * \param distrib distribution glob to use
 * \param point location in array of point
 */
static void
SetSupplied(distrib_t *distrib, UInt32 point)
{
	distrib->NodesSupplied++;
	if (!(getWorldFlags(point) & distrib->flagToSet)) {
		orWorldFlags(point, distrib->flagToSet);
	}
}

/*!
 * \brief Add source to the grid.
 * \param distrib the distribution structure
 * \param pos index into array of node
 * \param point node value
 * \param status status of node (powered/watered)
 * \return whether node was supplied
 */
Int16
SupplyIfPlant(distrib_t *distrib, UInt32 pos, welem_t point, selem_t status)
{
	Int16 pt;
	if (!(pt = distrib->isplant(point, pos, status)))
		return (0);
	if (getScratch(pos))
		return (0);
	SetSupplied(distrib, pos);
	setScratch(pos);
	distrib->NodesTotal++;
	distrib->SourceLeft += pt;
	distrib->SourceTotal += pt;
	if (!StackIsEmpty(distrib->needSourceList)) {
		while (distrib->SourceLeft &&
		    !StackIsEmpty(distrib->needSourceList)) {
			pos = StackPop(distrib->needSourceList);
			distrib->SourceLeft--;
			SetSupplied(distrib, pos);
		}
	}
	return (pt);
}

/*!
 * \brief Do Distribution of the grid (water/power)
 * \param grid grid to perform distribution on
 */
static void
DoDistribute(Int16 grid)
{
	/* type == GRID_POWER | GRID_POWER */
	Int32 i, j;
	Int8 gw;
	distrib_t *distrib = gMalloc(sizeof (distrib_t));

	distrib->SourceLeft = 0;
	distrib->SourceTotal = 0;
	distrib->NodesTotal = 0;
	distrib->NodesSupplied = 0;
	distrib->ShortOrOut = 0;
	distrib->needSourceList = StackNew();
	distrib->unvisitedNodes = StackNew();

	/* Step 1: Find all the powerplants and move out from there */
	if (grid == GRID_POWER) {
		distrib->isplant = &IsItAPowerPlant;
		distrib->doescarry = &CarryPower;
		distrib->flagToSet = POWEREDBIT;
		distrib->error_flag = peFineOnPower;
	} else {
		distrib->isplant = &IsItAWaterPump;
		distrib->doescarry = &CarryWater;
		distrib->flagToSet = WATEREDBIT;
		distrib->error_flag = peFineOnWater;
	}

	LockZone(lz_world); /* this lock locks for ALL power subs */
	for (j = 0; j < MapMul(); j++) {
		if (distrib->doescarry(getWorld(j)))
			andWorldFlags(j,
			    ~(distrib->flagToSet | SCRATCHBIT | PAINTEDBIT));
	}

	for (i = 0; i < MapMul(); i++) {
		gw = getWorld(i);
		if (!getScratch(i)) {
			if (SupplyIfPlant(distrib, i, gw, getWorldFlags(i))) {
				AddNeighbors(distrib, i);
				DistributeUnvisited(distrib);
				/* unpowered points are removed */
				StackDoEmpty(distrib->needSourceList);
				WriteLog("Grid#%d Supplied Nodes: %d/%d "
				    "SrcRemain: %d/%d\n", (int)grid,
				    (int)distrib->NodesSupplied,
				    (int)distrib->NodesTotal,
				    (int)distrib->SourceLeft,
				    (int)distrib->SourceTotal);
				if (distrib->SourceLeft < 25) {
					distrib->ShortOrOut |= SHORT_BIT;
					if (distrib->SourceLeft == 0)
						distrib->ShortOrOut |= OUT_BIT;
				}
				distrib->SourceLeft = 0;
				distrib->SourceTotal = 0;
				distrib->NodesSupplied = 0;
				distrib->NodesTotal = 0;
			}
		}
	}
	UnlockZone(lz_world);
	StackDelete(distrib->needSourceList);
	StackDelete(distrib->unvisitedNodes);
	if (distrib->ShortOrOut & OUT_BIT) {
		UIProblemNotify(distrib->error_flag + 2);
	} else if (distrib->ShortOrOut & SHORT_BIT) {
		UIProblemNotify(distrib->error_flag + 1);
	} else {
		UIProblemNotify(distrib->error_flag);
	}
	gFree(distrib);
}

/*!
 * \brief Distribute power to the unvisited list.
 */
static void
DistributeUnvisited(distrib_t *distrib)
{
	UInt32 pos;
	UInt8 flag;

	while (!StackIsEmpty(distrib->unvisitedNodes)) {
		pos = StackPop(distrib->unvisitedNodes);
		flag = getWorldFlags(pos);
		if (SupplyIfPlant(distrib, pos, getWorld(pos), flag)) {
			goto nextneighbor;
		}

		if (distrib->SourceLeft && ((flag & distrib->flagToSet) == 0)) {
			/*
			 * if this field hasn't been powered,
			 * we need to "use" some power to move further along
			 */
			distrib->SourceLeft--;
		}

		/* do we have more power left? */
		if (distrib->SourceLeft <= 0)
			StackPush(distrib->needSourceList, pos);
		else
			SetSupplied(distrib, pos);

		/* now, set the flags to indicate we've been here */
		setScratch(pos);

nextneighbor:
		/* find the possible ways we can move on from here */
		AddNeighbors(distrib, pos);
	};
}

/*!
 * \brief Add all the neighbors to this node to the unvisited list.
 * \param distrib the distribution structure
 * \param pos location of node on list
 */
static void
AddNeighbors(distrib_t *distrib, UInt32 pos)
{
	char cross = DistributeNumberOfSquaresAround(distrib, pos);

	/* if there's "no way out", return */
	if ((cross & 0x0f) == 0)
		return;

	distrib->NodesTotal += cross & 0x0f;

	if ((cross & 0x10) == 0x10) {
		StackPush(distrib->unvisitedNodes, pos-getMapWidth());
	}
	if ((cross & 0x20) == 0x20) {
		StackPush(distrib->unvisitedNodes, pos+1);
	}
	if ((cross & 0x40) == 0x40) {
		StackPush(distrib->unvisitedNodes, pos+getMapWidth());
	}
	if ((cross & 0x80) == 0x80) {
		StackPush(distrib->unvisitedNodes, pos-1);
	}
}

/*
 * \brief check that the node carries something.
 *
 * Note that this function is used internally in the power distribution
 * routine. Therefore it will return false for tiles we've already been at,
 * to avoid backtracking any nodes we've already encountered.
 * \param pos location of item.
 */
static Int16
Carries(Int16 (*doescarry)(welem_t), UInt32 pos)
{
	if (getScratch(pos))
		return (0);
	return (doescarry(getWorld(pos)));
}

/*!
 * \brief gives a status of the situation around us
 *
 * Checks all the nodes around this position to see if they should be
 * visited to supply them with the appropriate supply item.
 * 0001 00xx if up
 * 0010 00xx if right
 * 0100 00xx if down
 * 1000 00xx if left
 * xx = number of directions
 * \param distrib the distribution glob to use for searching
 * \param pos map location to perfrm distribution on.
 * \return fields & quantity in an overloaded array
 */
static Int16
DistributeNumberOfSquaresAround(distrib_t *distrib, UInt32 pos)
{
	Int16 retval = 0;
	Int8 number = 0;
	Int16 (*carries)(welem_t) = distrib->doescarry;

	if (Carries(carries, DistributeMoveOn(pos, dtUp))) {
		retval |= 0x10;
		number++;
	}
	if (Carries(carries, DistributeMoveOn(pos, dtRight))) {
		retval |= 0x20;
		number++;
	}
	if (Carries(carries, DistributeMoveOn(pos, dtDown))) {
		retval |= 0x40;
		number++;
	}
	if (Carries(carries, DistributeMoveOn(pos, dtLeft))) {
		retval |= 0x80;
		number++;
	}

	retval |= number;

	return (retval);
}

/*!
 * \brief Distribute supply to locations around this node.
 * 
 * this function take a position and a direction and
 * moves the position in the direction, but won't move
 * behind map borders
 * \param pos location in map to source from
 * \param direction direction move to
 * \return the location to move to, or the same position
 */
static UInt32
DistributeMoveOn(UInt32 pos, dirType direction)
{
	switch (direction) {
	case dtUp:
		if (pos < getMapWidth())
			return (pos);
		pos -= getMapWidth();
		break;
	case dtRight:
		if (((pos%getMapWidth()) + 1) >= getMapWidth())
			return (pos);
		pos++;
		break;
	case dtDown:
		if ((pos+getMapWidth()) >= MapMul())
			return (pos);
		pos += getMapWidth();
		break;
	case dtLeft:
		if (pos % getMapWidth() == 0)
			return (pos);
		pos--;
		break;
	}
	return (pos);
}

/*!
 * \brief check if a node exists next to other types of nodes
 * \param pos index into map
 * \param dirs directions to check
 * \param what the node type to compare it against
 * \return true if the node is next to it.
 */
static UInt8
ExistsNextto(UInt32 pos, UInt8 dirs, welem_t what)
{
	UInt8 rv = 0;

	if ((dirs & DIR_UP) && (pos > getMapWidth()) &&
	    (what == getWorld(pos - getMapWidth())))
		rv |= DIR_UP;
	if ((dirs & DIR_DOWN) && (pos < (UInt32)(MapMul() - getMapWidth())) &&
	    (what == getWorld(pos + getMapWidth())))
		rv |= DIR_DOWN;
	if ((dirs & DIR_LEFT) && (pos % getMapWidth()) && 
	    (what == getWorld(pos - 1)))
		rv |= DIR_LEFT;
	if ((dirs & DIR_RIGHT) && (((pos % getMapWidth()) + 1) < getMapWidth())
		&& (what == getWorld(pos + 1)))
		rv |= DIR_RIGHT;
	return (rv);
}


/* Zones upgrade/downgrade */

/*! \brief Zone scores */
typedef struct {
	Int32 pos; /*!< position of the node */
	Int32 score; /*!< score of the node */
} ZoneScore;

/*! \brief random zone list */
static ZoneScore *ran_zone;

/*! \brief Current position in list (either random or linear) */
static Int32 at_pos;

/*! \brief counter to ensure we don't spend too much time looping */
static UInt16 counter;

/*! \brief maximum size of the random list */
static UInt16 max_list;

/*!
 * \brief Find a bunch of zones and decide to upgrade/downgrade them
 *
 * We use a list containing a number of random zones numbering
 * three times the width or height of the map whichever is biggest.
 */
static void
FindZonesForUpgrading(void)
{
	UInt16 i;
	Int32 randomZone;

	if (ran_zone == NULL) {
		max_list = (getMapWidth() < getMapHeight() ? getMapHeight() :
		    getMapWidth()) * 3;
		ran_zone = (ZoneScore *)gCalloc(max_list, sizeof (ZoneScore));
	}

	at_pos = 0;

	/* find some random zones */
	for (i = 0; i < max_list; i++) {
		ran_zone[at_pos].pos = -1;
		randomZone = GetRandomZone();
		if (randomZone != -1) { /* -1 means we didn't find a zone */
			ran_zone[at_pos++].pos = randomZone;
		}
	}
}


/*!
 * \brief find the scores that apply to the scoring zones
 *
 * The score finding routine is divided into small bits of 10 zones per run.
 * This is to free the program flow to take care of user interaction.
 * All functions in the simulation part should complete in under 3/4 second,
 * or the user might see the program as being slow.
 * \return true of more zones need to be processed.
 */
Int16
FindScoreForZones(void)
{
	Int16 i;
	Int32 score;

	counter += 20;

	LockZone(lz_world);

	for (i = counter - 20; i < (signed)counter; i++) {
		if (i >= at_pos) {
			counter = 0;
			UnlockZone(lz_world);
			return (0);
		}
		score = GetZoneScore(ran_zone[i].pos);
		if (score != -1) {
			ran_zone[i].score = score;
		} else {
			ran_zone[i].score = -1;
			WriteLog("Instadowngrade (%d,%d)\n",
			    (int)(ran_zone[i].pos % getMapWidth()),
			    (int)(ran_zone[i].pos / getMapWidth()));
			DowngradeZone(ran_zone[i].pos);
			ran_zone[i].pos = -1;
		}
	}
	UnlockZone(lz_world);
	return (1); /* there's still more zones that need a score. */
}

/*!
 * \brief compare two zones for the quicksort
 * \param a pointer to first element
 * \param b pointer to second element
 * \param other other value
 */
static Int16
zoneCmpFn(void *a, void *b, Int32 other __attribute__((unused)))
{
	ZoneScore *zs1 = (ZoneScore *)a;
	ZoneScore *zs2 = (ZoneScore *)b;

	if (zs1->score > zs2->score)
		return (1);
	if (zs1->score < zs2->score)
		return (-1);
	return (0);
}

/*!
 * \brief Regrade the zones
 * 
 * sort the zones by score. Upgrade the 12 highest scoring zones,
 * downgrade the 10 lowest scoring zones.
 */
void
reGradeZones(void)
{
	Int16 i;
	Int16 downCount = 10;
	Int16 upCount = 12;

	QSort(ran_zone, at_pos, sizeof (ZoneScore), zoneCmpFn);
	WriteLog("Used: %ld\n", at_pos);
	/* upgrade the upCount best */
	for (i = at_pos - 1; i >= 0 && i > (at_pos - upCount); i--) {
		if (ran_zone[i].pos == -1) continue;

		/* upgrade him/her/it/whatever */
		UpgradeZone(ran_zone[i].pos);
		WriteLog("Upgrade (%d, %d)(%ld)\n",
		    (int)(ran_zone[i].pos % getMapWidth()),
		    (int)(ran_zone[i].pos / getMapWidth()),
		    ran_zone[i].score);
		ran_zone[i].pos = -1;
	}

	/* downgrade the downCount worst */
	for (i = 0; i < downCount && i < (at_pos - upCount); i++) {
		/* downgrade him/her/it/whatever */
		if (ran_zone[i].pos == -1) continue;
		if (ran_zone[i].score > 0) continue;
		DowngradeZone(ran_zone[i].pos);
		WriteLog("Downgrade (%d, %d)(%ld)\n",
		    (int)(ran_zone[i].pos % getMapWidth()),
		    (int)(ran_zone[i].pos / getMapWidth()),
		    ran_zone[i].score);
		ran_zone[i].pos = -1;
	}
	at_pos = 0;
}

/*!
 * \brief Downgrade the zone at the position specified
 * \param pos the location on the map to downgrade.
 */
static void
DowngradeZone(UInt32 pos)
{
	welem_t type;
	welem_t ntype;
	Int16 xpos = pos % getMapWidth();
	Int16 ypos = pos / getMapWidth();

	LockZone(lz_world);
	type = getWorld(pos);
	ntype = type;
	if (IsCommercial(type) && type != Z_COMMERCIAL_SLUM) {
		ntype = (type == Z_COMMERCIAL_MIN) ?
		    Z_COMMERCIAL_SLUM : type - 1;
		vgame.BuildCount[bc_value_commercial]--;
		if (ntype == Z_COMMERCIAL_SLUM)
			vgame.BuildCount[bc_count_commercial]--;
	} else if (IsResidential(type) && type != Z_RESIDENTIAL_SLUM) {
		ntype = (type == Z_RESIDENTIAL_MIN) ?
		    Z_RESIDENTIAL_SLUM : type - 1;
		vgame.BuildCount[bc_value_residential]--;
		if (ntype == Z_RESIDENTIAL_SLUM)
			vgame.BuildCount[bc_count_residential]--;
	} else if (IsIndustrial(type) && type != Z_INDUSTRIAL_SLUM) {
		ntype = (type == Z_INDUSTRIAL_MIN) ?
		    Z_INDUSTRIAL_SLUM : type - 1;
		vgame.BuildCount[bc_value_industrial]--;
		if (ntype == Z_INDUSTRIAL_SLUM)
			vgame.BuildCount[bc_count_industrial]--;
	}
	if (ntype != type) {
		setWorld(pos, ntype);

		DrawFieldWithoutInit(xpos, ypos);
	}

	UnlockZone(lz_world);
}

/*
 * \brief Upgrade the zone at the position
 * \param pos the location on the map to upgrade
 */
static void
UpgradeZone(UInt32 pos)
{
	welem_t type, ntype;
	Int16 xpos = pos % getMapWidth();
	Int16 ypos = pos / getMapWidth();

	LockZone(lz_world);
	type = getWorld(pos);
	ntype = type;
	if (IsCommercial(type) && type < Z_COMMERCIAL_MAX) {
		ntype = (type == Z_COMMERCIAL_SLUM) ?
		    Z_COMMERCIAL_MIN : type + 1;
		vgame.BuildCount[bc_value_commercial]++;
		if (type == Z_COMMERCIAL_SLUM)
			vgame.BuildCount[bc_count_commercial]++;
	} else if (IsResidential(type) && type < Z_RESIDENTIAL_MAX) {
		ntype = (type == Z_RESIDENTIAL_SLUM) ?
		    Z_RESIDENTIAL_MIN : type + 1;
		vgame.BuildCount[bc_value_residential]++;
		if (type == Z_RESIDENTIAL_SLUM)
			vgame.BuildCount[bc_count_residential]++;
	} else if (IsIndustrial(type) && type < Z_INDUSTRIAL_MAX) {
		ntype = (type == Z_INDUSTRIAL_SLUM) ?
		    Z_INDUSTRIAL_MIN : type + 1;
		vgame.BuildCount[bc_value_industrial]++;
		if (type == Z_RESIDENTIAL_SLUM)
			vgame.BuildCount[bc_count_industrial]++;
	}
	if (ntype != type) {
		setWorld(pos, ntype);

		DrawFieldWithoutInit(xpos, ypos);
	}

	UnlockZone(lz_world);
}

/*!
 * \brief Walk the road, looking for things at the end
 * \todo actually do the road trip
 */
static Int16
DoTheRoadTrip(UInt32 startPos __attribute__((unused)))
{
	return (1); /* for now */
}

/*!
 * \brief Get the score for this zone.
 *
 * \param pos location on map to get the score of
 * \return -1 if the zone needs to be downgraded because of a lack of
 * water/power.
 */
long
GetZoneScore(UInt32 pos)
{
	long score = -1; /* Will downgrade to begin with */
	int x = pos % getMapWidth();
	int y = pos / getMapWidth();
	int ax, ay;
	int maxx, maxy;
	int bRoad = 0;
	zoneType type = ztWhat;
	UInt8 zone;

	LockZone(lz_world);
	zone = getWorld(pos);
	type = (IsZone(zone, ztCommercial) ? ztCommercial :
	    (IsZone(zone, ztResidential) ? ztResidential : ztIndustrial));

	if (((getWorldFlags(pos) & POWEREDBIT) == 0) ||
	    ((getWorldFlags(pos) & WATEREDBIT) == 0)) {
		/* whoops, no power or no water */
		WriteLog("No Power || Water\n");
		goto unlock_ret;
	}

	if (IsSlum(zone)) {
		score = 50;
		goto unlock_ret;
	}

	if ((type == ztIndustrial) || (type == ztCommercial))  {
		/*
		 * see if there's actually enough residential population
		 * to support a new zone of ind or com
		 */

		long availPop = (vgame.BuildCount[bc_value_residential]*25)
		    - (vgame.BuildCount[bc_value_commercial]*25
		    + vgame.BuildCount[bc_value_industrial]*25);
		/* pop is too low */
		if (availPop <= 0) {
			WriteLog("Pop too low to promote ind || comm\n");
			goto unlock_ret;
		}
	} else if (type == ztResidential) {
		/*
		 * the population can't skyrocket all at once, we need a cap
		 * somewhere - note, this should be fine tuned somehow
		 * A factor might be the number of (road/train/airplane)
		 * connections to the surrounding world - this would
		 * bring more potential residents into our little city
		 */
		long availPop = ((getMonthsElapsed() * getMonthsElapsed()) /
		    35) + 30 - vgame.BuildCount[bc_value_residential];
		/* hmm - need more children */
		if (availPop <= 0) {
			WriteLog("No People\n");
			goto unlock_ret;
		}
	}

	if (type == ztCommercial) {
		/*
		 * and what is a store without something to sell? We need
		 * enough industrial zones before commercial zones kick in.
		 */

		long int availGoods = (vgame.BuildCount[bc_value_industrial] /
		    3 * 2) - (vgame.BuildCount[bc_value_commercial]);
		/* darn, nothing to sell here */
		if (availGoods <= 0) {
			WriteLog("No Goods\n");
			goto unlock_ret;
		}
	}

	/* take a look around at the enviroment */
	maxx = (4 + x < getMapWidth()) ? 4 + x : getMapWidth() - 1;
	maxy = (4 + y < getMapHeight()) ? 4 + y : getMapHeight() - 1;
	ax = (x - 3 > 0) ? x - 3 : 0;
	while (ax < maxx) {
		ay = (y - 3 > 0) ? y - 3 : 0;
		while (ay < maxy) {
			score += GetScoreFor(type, getWorld(WORLDPOS(ax, ay)));
			if (IsRoad(getWorld(WORLDPOS(ax, ay))) && bRoad == 0) {
				/*
				 * can we reach all kinds of
				 * zones from here?
				 */
				bRoad = DoTheRoadTrip(WORLDPOS(ax, ay));
				if (!bRoad) {
					score = -1;
					goto unlock_ret;
				}
			}
			ay++;
		}
		ax++;
	}

unlock_ret:
	UnlockZone(lz_world);
	return (score);
}

/*!
 * \brief Get the score for the zone specified.
 * \param iamthis current zone
 * \param what adjacent zone
 * \return the score for an indvidual zone
 */
Int16
GetScoreFor(zoneType iamthis, welem_t what)
{
	if (IsZone(what, ztCommercial)) {
		return (iamthis == ztCommercial) ? 1 :
		    ((iamthis == ztResidential) ? 50 :
		    ((iamthis == ztIndustrial) ? 50 : 50));
	}
	if (IsZone(what, ztResidential)) {
		return (iamthis == ztCommercial) ? 50 :
		    ((iamthis == ztResidential) ? 1 :
		    ((iamthis == ztIndustrial) ? 50 : 50));
	}
	if (IsZone(what, ztIndustrial)) {
		return (iamthis == ztCommercial) ? (-25) :
		    ((iamthis == ztResidential) ? (-75) :
		    ((iamthis == ztIndustrial) ? 1 : (-50)));
	}
	if (IsRoad(what)) {
		return (iamthis == ztCommercial) ? 75 :
		    ((iamthis == ztResidential) ? 50 :
		    ((iamthis == ztIndustrial) ? 75 : 66));
	}
	if (IsCoalPlant(what)) {
		return (iamthis == ztCommercial) ? (-75) :
			((iamthis == ztResidential) ? (-100) :
			((iamthis == ztIndustrial) ? 30 : (-75)));
	}
	if (IsNukePlant(what)) {
		return (iamthis == ztCommercial) ? (-150) :
			((iamthis == ztResidential) ? (-200) :
			((iamthis == ztIndustrial) ? 15 : (-175)));
	}
	if (IsRealTree(what)) {
		return (iamthis == ztCommercial) ? 50 :
			((iamthis == ztResidential) ? 85 :
			((iamthis == ztIndustrial)? 25 : 50));
	}
	if (IsFakeTree(what)) {
		return (iamthis == ztCommercial) ? 25 :
			((iamthis == ztResidential) ? 42 :
			((iamthis == ztIndustrial)? 12 : 25));
	}
	if (IsRealWater(what)) {
		return (iamthis == ztCommercial) ? 175 :
			((iamthis == ztResidential) ? 550 :
			((iamthis == ztIndustrial) ? 95 : 250));
	}
	if (IsFakeWater(what)) {
		return (iamthis == ztCommercial) ? 80 :
			((iamthis == ztResidential) ? 80 :
			((iamthis == ztIndustrial) ? 45 : 25));
	}
	return (0);
}

/*!
 * \brief Get a zone on the world.
 *
 * Must be one of the Residential / Industrial /commercial zones
 * \return the zone picked, or -1 for no zone found
 */
static Int32
GetRandomZone(void)
{
	UInt32 pos = 0;
	UInt16 i;
	UInt8 type;

	LockZone(lz_world);
	for (i = 0; i < 5; i++) { /* try five times to hit a valid zone */
		pos = GetRandomNumber(MapMul());
		type = getWorld(pos);
		if (IsGrowable(type)) {
			UnlockZone(lz_world);
			return (pos);
		}
	}

	UnlockZone(lz_world);
	return (-1);
}

/*! \brief mapping of item counts and their associated costs */
static const struct countCosts {
	BuildCount	count; /*!< Item count to affect */
	UInt32	cost;	/*!< Cost of associated item */
} countCosts[] = {
	{ bc_value_residential, INCOME_RESIDENTIAL },
	{ bc_value_commercial, INCOME_COMMERCIAL },
	{ bc_value_industrial, INCOME_INDUSTRIAL },
	{ bc_count_roads, UPKEEP_ROAD },
	{ bc_count_trees, 0 },
	{ bc_water, 0 },
	{ bc_powerlines, UPKEEP_POWERLINE },
	{ bc_coalplants, UPKEEP_POWERPLANT },
	{ bc_nuclearplants, UPKEEP_NUCLEARPLANT },
	{ bc_waste, 0 },
	{ bc_fire, 0 },
	{ bc_fire_stations, UPKEEP_FIRE_STATIONS },
	{ bc_police_departments, UPKEEP_POLICE_STATIONS },
	{ bc_military_bases, UPKEEP_MILITARY_BASES },
	{ bc_waterpipes, 0 },
	{ bc_waterpumps, 0 }
};

#define CCSIZE	(sizeof (countCosts) / sizeof (countCosts[0]))

/*!
 * \brief get the costs of a specific node
 * \param item the item who's cost we wish to extract
 * \return the cost/benefit associated with the node
 */
static Int32
costIt(BuildCode item)
{
	UInt16 i;
	for (i = 0; i < CCSIZE; i++)
		if (countCosts[i].count == item)
			return (vgame.BuildCount[item] * countCosts[i].cost);
	WriteLog("Fell off the end of costIt here (%d)\n", item);
	return (0);
}

Int32
BudgetGetNumber(BudgetNumber type)
{
	Int32 ret = 0;
	switch (type) {
	case bnResidential:
		ret = (Int32)costIt(bc_value_residential) * getTax()/100;
		break;
	case bnCommercial:
		ret = (Int32)costIt(bc_value_commercial) * getTax()/100;
		break;
	case bnIndustrial:
		ret = (Int32)costIt(bc_value_industrial) * getTax()/100;
		break;
	case bnIncome:
		ret = ((costIt(bc_value_residential) + 
		    costIt(bc_value_commercial) +
		    costIt(bc_value_industrial)) * getTax()) / 100;
		break;
	case bnTraffic:
		ret = (Int32)(costIt(bc_count_roads) *
		    getUpkeep(ue_traffic)) / 100;
		break;
	case bnPower:
		ret = (Int32)((costIt(bc_powerlines) +
		    costIt(bc_nuclearplants) + costIt(bc_coalplants)) *
		    getUpkeep(ue_power)) / 100;
		break;
	case bnDefence:
		ret = (Int32)((costIt(bc_fire_stations) +
		    costIt(bc_police_departments) +
		    costIt(bc_military_bases)) *
		    getUpkeep(ue_defense)) / 100;
		break;
	case bnCurrentBalance:
		ret = getCredits();
		break;
	case bnChange:
		ret = (Int32)BudgetGetNumber(bnIncome)
			- BudgetGetNumber(bnTraffic)
			- BudgetGetNumber(bnPower)
			- BudgetGetNumber(bnDefence);
		break;
	case bnNextMonth:
		ret = (Int32)BudgetGetNumber(bnCurrentBalance) +
			BudgetGetNumber(bnChange);
		break;
	}
	return (ret);
}

/*!
 * \brief Add money because of taxes
 */
void
DoTaxes(void)
{
	incCredits(BudgetGetNumber(bnIncome));
}

/*!
 * \brief Take away money because of upkeep costs
 */
void
DoUpkeep(void)
{
	UInt32 upkeep;

	upkeep = BudgetGetNumber(bnTraffic) + BudgetGetNumber(bnPower) +
	    BudgetGetNumber(bnDefence);

	if (upkeep <= (UInt32)getCredits()) {
		WriteLog("Upkeep: %lu\n", (unsigned long)upkeep);
		decCredits(upkeep);
		return;
	} else {
		WriteLog("*** Negative Cashflow\n");
		WriteLog("Upkeep: %lu\n", (unsigned long)upkeep);
	}
	setCredits(0);

	/* roads */
	DoNastyStuffTo(Z_ROAD, 1);
	DoNastyStuffTo(Z_POWERLINE, 5);
	DoNastyStuffTo(Z_COALPLANT, 15);
	DoNastyStuffTo(Z_NUCLEARPLANT, 50);
	DoNastyStuffTo(Z_FIRESTATION, 10);
	DoNastyStuffTo(Z_POLICEDEPT, 12);
	DoNastyStuffTo(Z_ARMYBASE, 35);
}

/*!
 * \todo break into separate functions and a jump table.
 */
Int16
Sim_DoPhase(Int16 nPhase)
{
	switch (nPhase) {
	case 1:
		if (NeedsUpdate(GRID_POWER)) {
			WriteLog("Simulation phase 1 - power grid\n");
			Sim_Distribute_Specific(GRID_POWER);
			ClearUpdate(GRID_POWER);
		}
		nPhase = 2;
		break;
	case 2:
		if (NeedsUpdate(GRID_WATER)) {
			WriteLog("Simulation phase 2 - water grid\n");
			Sim_Distribute_Specific(GRID_WATER);
			ClearUpdate(GRID_WATER);
		}
		nPhase = 3;
		break;
	case 3:
		WriteLog("Simulation phase 3 - Find zones for upgrading\n");
		FindZonesForUpgrading();
		nPhase = 4;
		/* this can't be below */
		WriteLog("Simulation phase 4 - Find score for zones\n");
		break;
	case 4:
		if (FindScoreForZones() == 0)
			nPhase = 5;
		break;
	case 5:
		WriteLog("Simulation phase 5 - Regrade Zones\n");
		reGradeZones();
		nPhase = 6;
		break;
	case 6:
		WriteLog("Simulation phase 6 - Update disasters\n");
		/* UpdateDisasters(); */
		DoRandomDisaster();
		nPhase = 7;
		break;
	case 7:
		WriteLog("Simulation phase 7 - Economics\n");
		DoTaxes();
		incrementTimeElapsed(4);
		nPhase = 0;
		UIInitDrawing();
		UIDrawCredits();
		UIDrawPop();
		UICheckMoney();
		DoUpkeep();
		UIFinishDrawing();
		break;
	}

	return (nPhase);
}

void
UpdateVolatiles(void)
{
	UInt32 p;

	LockZone(lz_world);

	for (p = 0; p < MapMul(); p++) {
		UInt8 elt = getWorld(p);

		/* Gahd this is terrible. I need to fix it. */
		if (IsCommercial(elt)) {
			vgame.BuildCount[bc_count_commercial]++;
			vgame.BuildCount[bc_value_commercial] += ZoneValue(elt);
		}
		if (IsResidential(elt)) {
			vgame.BuildCount[bc_count_residential]++;
			vgame.BuildCount[bc_value_residential] +=
			    ZoneValue(elt);
		}
		if (IsIndustrial(elt)) {
			vgame.BuildCount[bc_count_industrial]++;
			vgame.BuildCount[bc_value_industrial] += ZoneValue(elt);
		}
		if (IsRoad(elt)) {
			vgame.BuildCount[bc_count_roads]++;
			vgame.BuildCount[bc_value_roads] += ZoneValue(elt);
		}
		if (IsFakeTree(elt)) {
			vgame.BuildCount[bc_count_trees]++;
		}
		if (IsFakeWater(elt)) {
			vgame.BuildCount[bc_water]++;
		}
		if (IsRoadPower(elt)) {
			vgame.BuildCount[bc_powerlines]++;
			vgame.BuildCount[bc_count_roads]++;
			vgame.BuildCount[bc_value_roads] += ZoneValue(elt);
		}
		if (IsPowerLine(elt))
			vgame.BuildCount[bc_powerlines]++;
		if (elt == Z_COALPLANT)
			vgame.BuildCount[bc_coalplants]++;
		if (elt == Z_NUCLEARPLANT)
			vgame.BuildCount[bc_nuclearplants]++;
		if (elt == Z_WASTE) vgame.BuildCount[bc_waste]++;
		if (elt >= Z_FIRE1 && elt <= Z_FIRE3)
			vgame.BuildCount[bc_fire]++;
		if (elt == Z_FIRESTATION)
			vgame.BuildCount[bc_fire_stations]++;
		if (elt == Z_POLICEDEPT)
			vgame.BuildCount[bc_police_departments]++;
		if (elt == Z_ARMYBASE)
			vgame.BuildCount[bc_military_bases]++;
		if (IsWaterPipe(elt))
			vgame.BuildCount[bc_waterpipes]++;
		if (IsRoadPipe(elt)) {
			vgame.BuildCount[bc_waterpipes]++;
			vgame.BuildCount[bc_count_roads]++;
			vgame.BuildCount[bc_value_roads] += ZoneValue(elt);
		}
		if (IsPowerWater(elt)) {
			vgame.BuildCount[bc_waterpipes]++;
			vgame.BuildCount[bc_powerlines]++;
		}
		if (IsPump(elt))
			vgame.BuildCount[bc_waterpumps]++;
	}
	UnlockZone(lz_world);
}

/*!
 * \brief shuffle an individual set of statistics
 * 
 * This moves an entire set of statistics 'down' one unit; which corresponds
 * to 3 months (12/4) for the 10 year graph and
 * to 2yr 6 months (10y/4) for the 100 year graph
 * 
 * \param ary the array of entries to shuffle
 * \param load the value to load on to the start of the stats.
 * \return the value that shuffled off the end
 */
static UInt16
ShuffleIndividualStatistic(UInt16 *ary, UInt16 load)
{
	int atItem = STAT_ENTRIES - 1;
	UInt16 newvalue;

	newvalue = ary[atItem];
	for (atItem = STAT_ENTRIES - 1; atItem > 0; atItem--) {
		ary[atItem] = ary[atItem-1];
	}
	ary[0] = load;
	return (newvalue);
}

void
UpdateCounters(void)
{
	/* cashflow: */
	if (vgame.prior_credit == 0) {
		vgame.prior_credit = getCredits();
	}
	/*
	 * Don't worry about the overflow on the last bit, the upper 16 bits
	 * of this value will be stripped automatically. What we end up doing
	 * is adding even more to this (possibly negative) value.
	 */
	vgame.BuildCount[bc_cashflow] = (OFFSET_FOR_CASHFLOW_BC -
	    vgame.prior_credit) + getCredits();
	/* Update the prior_credit field */
	vgame.prior_credit = getCredits();
	/*! \note
	 * How we calculate the pollution:
	 *   Pollution is based on the density of a zone. The higher the
	 *   density of the zone, the higher the pollution.
	 *   Take the density of the industrial areas (as a sum)
	 *   Subtract the density of residential areas
	 *   Park areas subtract 'massive' pollution from the map
	 *   Commercial zones do not add or subtract from the pollution
	 */
	 
	 /*! \note
	  * How we calculate the criminal level
	  *   Residential areas contribute based on their value
	  *   Industrial areas contribute based on the inverse of their value
	  *   Commercial areas contribute based on their value
	  */
}

/*!
 * This involves compositing the month delta into the graph. This is done by
 * taking the old average; multiplying it by 3, adding the current month's
 * value and dividing by 4. We have to be careful of overflow.
 */
void
RecordStatistics(void)
{
	StatisticItem item;
	stat_item *stat;
	BuildCount offset;
	UInt16 stat_value;
	UInt32 tmpval;
	
	for (item = st_cashflow; item < st_tail; item++) {
		offset = statvalues[item].offset;
		stat_value = vgame.BuildCount[offset];
		stat = getStatistics(offset);
		
		tmpval = (UInt32)stat->last_ten[0] * 3 + stat_value;
		tmpval >>= 2;
		/* overflow */
		if (tmpval > (UInt16)MAX_UINT16) {
			stat->last_ten[0] = MAX_UINT16;
		} else {
			stat->last_ten[0] = tmpval;
		}
		/* XXX: fixme! */
		/* shuffle the statistics every month */
		if ((getMonthsElapsed() & 3) == 3) {
			(void)ShuffleIndividualStatistic(
			    &stat->last_ten[0], stat_value);
		}
		if ((getMonthsElapsed() & (3*12)) == (3 * 12)) {
		}
	}
}

UInt32
getPopulation(void)
{
	return ((vgame.BuildCount[bc_value_residential] +
		(vgame.BuildCount[bc_value_commercial] * 8) +
		(vgame.BuildCount[bc_value_industrial] * 8)) * 20);
}

/*!
 * This works this way because all the nodes that carry water are in sequence
 */
Int16
CarryPower(welem_t x)
{
	return 
	/* ((IsPump(x) || IsPowerLine(x) || IsPowerWater(x) ||
	    IsSlum(x) || IsCoalPlant(x) || IsNuclearPlant(x) ||
	    IsFireStation(x) || IsPoliceDept(x) || IsArmyBase(x) ||
	    IsCommercial(x) || IsResidential(x) || IsIndustrial(x) ||
	    IsPowerRoad(x) || IsRailPower(x)) ? 1 : 0); */
	    ((IsPump(x)) ||
	    ((x >= Z_POWERLINE) && (x <= Z_POWERROAD_PVER)) ||
	    (IsRailPower(x)) ? 1 : 0);
}

/*!
 * This is slightly more complicated because the nodes that carry water are
 * not in the same order as the nodes that carry power.
 * 
 * This is the tradeoff that has to be made. One is faster at the expense of
 * the other.
 */
Int16
CarryWater(welem_t x)
{
	return (IsPump(x) || IsWaterPipe(x) ||
	    ((x >= Z_POWERWATER_START) && (x <= Z_INDUSTRIAL_MAX)) ||
	    IsRoadPipe(x) || IsRailPipe(x) ? 1 : 0);
}

Int16
IsPowerLine(welem_t x)
{
	return ((((x >= Z_POWERLINE_START) && (x <= Z_POWERLINE_END))) ? 1 : 0);
}

Int16
IsRoad(welem_t x)
{
	return (((x >= Z_ROAD_START) && (x <= Z_ROAD_END)) ? 1 : 0);
}

Int16
IsRoadBridge(welem_t x)
{
	return (((x >= Z_BRIDGE_START) && (x <= Z_BRIDGE_END)) ? 1 : 0);
}
 
Int16
IsWaterPipe(welem_t x)
{
	return (((x >= Z_PIPE_START) && (x <= Z_PIPE_END)) ? 1 : 0);
}

Int16
ZoneValue(welem_t x)
{
	if ((x >= Z_COMMERCIAL_SLUM) && (x <= Z_INDUSTRIAL_SLUM))
		return (0);
	if ((x >= Z_COMMERCIAL_MIN) && (x <= Z_INDUSTRIAL_MAX)) {
		return (1 + ((x - Z_COMMERCIAL_MIN) % 10));
	}
	if (IsRoad(x))
		return ((x - Z_ROAD_START) + 1);
	if (IsRail(x))
		return ((x - Z_RAIL_START) + 1);
	else
		return (0);
}
 
Int16
IsRoadPipe(welem_t x)
{
	return (((x >= Z_PIPEROAD_START) && (x <= Z_PIPEROAD_END)) ? 1 : 0);
}

Int16
IsRoadPower(welem_t x)
{
	return (((x >= Z_POWERROAD_START) && (x <= Z_POWERROAD_END)) ? 1 : 0);
}

Int16
IsPowerWater(welem_t x)
{
	return (((x >= Z_POWERWATER_START) && (x <= Z_POWERWATER_END)) ? 1 : 0);
}

Int16
IsZone(welem_t x, zoneType nType)
{
	/* Slum handling - the slums & idents are in the same order */
	if (x == nType)
		return (1);
	nType -= Z_COMMERCIAL_SLUM;
	if ((x >= (welem_t)(nType * 10 + Z_COMMERCIAL_MIN)) && 
	    (x <= (welem_t)(nType * 10 + Z_COMMERCIAL_MAX)))
		return (1);
	return (0);
}

Int16
IsRoadOrBridge(welem_t x)
{
	return (IsRoad(x) || IsRoadBridge(x));
}

Int16
IsRail(welem_t x)
{
	return (((x >= Z_RAIL_START) && (x <= Z_RAIL_END)) ? 1 : 0);
}

Int16
IsRailPower(welem_t x)
{
	return (((x >= Z_RAILPOWER_START) && (x <= Z_RAILPOWER_END)) ? 1 : 0);
}

Int16
IsRailPipe(welem_t x)
{
	return (((x >= Z_RAILPIPE_START) && (x <= Z_RAILPIPE_END)) ? 1 : 0);
}

Int16
IsRailTunnel(welem_t x)
{
	return (((x >= Z_RAILTUNNEL_START) && (x <= Z_RAILTUNNEL_END)) ? 1 : 0);
}

Int16
IsRailOrTunnel(welem_t x)
{
	return (IsRail(x) || IsRailTunnel(x));
}

Int16
IsRailOvRoad(welem_t x)
{
	return (((x >= Z_RAILOVROAD_START) && (x <= Z_RAILOVROAD_END)) ? 1 : 0);
}

Int16
IsOccupied(welem_t x)
{
	return (!((x <= Z_REALWATER) || (x > Z_ENDMARKER)));
}

UInt8
CheckNextTo(Int32 pos, UInt8 dirs, Int16 (*checkfn)(welem_t))
{
	UInt8 rv = 0;

	if ((dirs & DIR_UP) && (pos > getMapWidth()) &&
	    checkfn(getWorld(pos - getMapWidth())))
		rv |= DIR_UP;
	if ((dirs & DIR_DOWN) && (pos < (MapMul() - getMapWidth())) &&
	    checkfn(getWorld(pos + getMapWidth())))
		rv |= DIR_DOWN;
	if ((dirs & DIR_LEFT) && (pos % getMapWidth()) && 
	    checkfn(getWorld(pos - 1)))
		rv |= DIR_LEFT;
	if ((dirs & DIR_RIGHT) && (((pos % getMapWidth()) + 1) < getMapWidth())
		&& checkfn(getWorld(pos + 1)))
		rv |= DIR_RIGHT;
	return (rv);
}

UInt8
CheckNextTo1(Int32 pos, UInt8 dirs, carryfnarg_t checkfn, void *cfarg)
{
	UInt8 rv = 0;

	if ((dirs & DIR_UP) && (pos > getMapWidth()) &&
	    checkfn(getWorld(pos - getMapWidth()), cfarg))
		rv |= DIR_UP;
	if ((dirs & DIR_DOWN) && (pos < (MapMul() - getMapWidth())) &&
	    checkfn(getWorld(pos + getMapWidth()), cfarg))
		rv |= DIR_DOWN;
	if ((dirs & DIR_LEFT) && (pos % getMapWidth()) && 
	    checkfn(getWorld(pos - 1), cfarg))
		rv |= DIR_LEFT;
	if ((dirs & DIR_RIGHT) && (((pos % getMapWidth()) + 1) < getMapWidth())
		&& checkfn(getWorld(pos + 1), cfarg))
		rv |= DIR_RIGHT;
	return (rv);
}

void
endSimulation(void)
{
	if (ran_zone != NULL) {
		gFree(ran_zone);
		ran_zone = NULL;
	}
}
