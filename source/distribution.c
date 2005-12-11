/*!
 * \file
 * \brief the simulation routines
 *
 * This consists of the outines that do all the simulation work, for example
 * deciding that certain zones are to be improved or deteriorated, where
 * monsters go, etc.
 */

#include <simulation.h>
#include <distribution.h>
#include <logging.h>
#include <locking.h>
#include <ui.h>
#include <globals.h>
#include <stack.h>
#include <stdint.h>
#include <mem_compat.h>

/*! \brief short and out bits */
#define	SHORT_BIT	1
#define	OUT_BIT		2

#define	DONTPAINT	(unsigned char)(1U<<7)

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

static Int16 DistributeNumberOfSquaresAround(distrib_t *distrib, UInt32 pos);

static UInt32 DistributeMoveOn(UInt32 pos, dirType direction);
static void DistributeUnvisited(distrib_t *distrib);

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
 * If you intend to use any of the free flags for any form of permanent state
 * then you need to alter the savegame code to add the bits that are being
 * used to the savegame structure. Currently saving this costs an extra 2.5k
 * in savegame structures (10000 / 4).
 *
 *  How to recreate:
 *	  call the distribution routine... it knows how to do each type
 */
static void DoDistribute(Int16 grid);

/*!
 * \brief Do a grid distribution for the grid(s) specified
 * \param gridonly the grid to do
 */
void
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
	addGraphicUpdate(gu_playarea);
	addGraphicUpdate(gu_desires);
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
	if (IsCoalPlant(point))
		return (SUPPLY_POWER_PLANT >> 2);
	else if (IsNukePlant(point))
		return (SUPPLY_NUCLEAR_PLANT >> 2);
	else
		return (0);
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
IsItAUsableWaterPump(welem_t point, UInt32 coord, selem_t flags)
{
	if (IsPump(point) && (flags & POWEREDBIT) &&
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
static Int16
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
			pos = (UInt32)StackPop(distrib->needSourceList);
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
	UInt32 i;
	welem_t gw;
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
		distrib->isplant = &IsItAUsableWaterPump;
		distrib->doescarry = &CarryWater;
		distrib->flagToSet = WATEREDBIT;
		distrib->error_flag = peFineOnWater;
	}

	LockZone(lz_world); /* this lock locks for ALL power subs */
	LockZone(lz_flags);
	for (i = 0; i < MapMul(); i++) {
		if (distrib->doescarry(getWorld(i)))
			andWorldFlags(i,
			    (selem_t)~(distrib->flagToSet |
				SCRATCHBIT | PAINTEDBIT));
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
	UnlockZone(lz_flags);
	UnlockZone(lz_world);
	StackDelete(distrib->needSourceList);
	StackDelete(distrib->unvisitedNodes);
	if (distrib->ShortOrOut & OUT_BIT) {
		UIProblemNotify(distrib->error_flag + 2);
	} else if (distrib->ShortOrOut & SHORT_BIT) {
		UIProblemNotify(distrib->error_flag + 1);
	} else {
		/* This isn't really a problem. */
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
		pos = (UInt32)StackPop(distrib->unvisitedNodes);
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
			StackPush(distrib->needSourceList, (Int32)pos);
		else
			SetSupplied(distrib, pos);

		/* now, set the flags to indicate we've been here */
		setScratch(pos);

nextneighbor:
		/* find the possible ways we can move on from here */
		AddNeighbors(distrib, pos);
	}
}

/*!
 * \brief Add all the neighbors to this node to the unvisited list.
 * \param distrib the distribution structure
 * \param pos location of node on list
 */
static void
AddNeighbors(distrib_t *distrib, UInt32 pos)
{
	Int16 cross = DistributeNumberOfSquaresAround(distrib, pos);

	/* if there's "no way out", return */
	if ((cross & 0x0f) == 0)
		return;

	distrib->NodesTotal += cross & 0x0f;

	if ((cross & 0x10) == 0x10) {
		if (pos > getMapWidth())
			StackPush(distrib->unvisitedNodes,
			    (Int32)(pos - getMapWidth()));
	}
	if ((cross & 0x20) == 0x20) {
		if ((pos + 1) < MapMul())
			StackPush(distrib->unvisitedNodes,
			    (Int32)(pos + 1));
	}
	if ((cross & 0x40) == 0x40) {
		if ((pos + getMapWidth()) < MapMul())
    		    StackPush(distrib->unvisitedNodes,
			    (Int32)(pos + getMapWidth()));
	}
	if ((cross & 0x80) == 0x80) {
		if (pos > 0)
			StackPush(distrib->unvisitedNodes, (Int32)(pos - 1));
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
	if ((dirs & DIR_RIGHT) &&
	    (((pos % getMapWidth()) + 1) < getMapWidth()) &&
	    (what == getWorld(pos + 1)))
		rv |= DIR_RIGHT;
	return (rv);
}
