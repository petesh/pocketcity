#include <handler.h>
#include <drawing.h>
#include <zakdef.h>
#include <ui.h>
#include <globals.h>
#include <handler.h>
#include <disaster.h>
#include <simulation.h>
#include <stack.h>

#if defined(PALM)
#include <MemoryMgr.h>
#include <unix_stdio.h>
#include <unix_string.h>
#include <unix_stdlib.h>
#include <StringMgr.h>
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif
#include <compilerpragmas.h>

static void DoTaxes(void);
static void DoUpkeep(void);
static Int16 DistributeNumberOfSquaresAround(UInt32 pos);

static void UpgradeZones(void);
static void UpgradeZone(UInt32 pos);
static void DowngradeZone(UInt32 pos);
static Int16 DoTheRoadTrip(UInt32 startPos);
static UInt32 DistributeMoveOn(UInt32 pos, dirType direction);
static void DistributeUnvisited(void);

static long GetZoneScore(UInt32 pos);
static Int16 GetScoreFor(UInt8 iamthis, UInt8 what);
static UInt32 GetRandomZone(void);
static void FindZonesForUpgrading(void);
static Int16 FindScoreForZones(void);
static void AddNeighbors(UInt32 pos);
static Int16 ExistsNextto(UInt32 pos, UInt8 what);

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
 *  don't use any of the free flags without asking zakarun (thanks)
 *  please note that the flags are _not_ saved, they _must_ be able to be
 *  recreated from the plain world[] array - else the savegames would be
 *  10k larger (that's A LOT ;)
 *
 *  How to recreate:
 *	  call the distribution routine... it knows how to do each type
 */
static void DoDistribute(Int16 grid);

/*
 * Distribute to the grids.
 * Does all the grids needed
 */
void
Sim_Distribute(void)
{
	if (NeedsUpdate(GRID_POWER)) {
		DoDistribute(GRID_POWER);
		ClearUpdate(GRID_POWER);
	}
	if (NeedsUpdate(GRID_WATER)) {
		DoDistribute(GRID_WATER);
		ClearUpdate(GRID_WATER);
	}
}

/*
 * Do a grid distribution for the grid(s) specified
 */
void
Sim_Distribute_Specific(Int16 gridonly)
{
	if (gridonly == 0) gridonly = GRID_ALL;

	if (NeedsUpdate(GRID_POWER) && (gridonly & GRID_POWER)) {
		DoDistribute(GRID_POWER);
		ClearUpdate(GRID_POWER);
	}
	if (NeedsUpdate(GRID_WATER) && (gridonly & GRID_WATER)) {
		DoDistribute(GRID_WATER);
		ClearUpdate(GRID_WATER);
	}
}

/*
 * Check if the node is a power plant (Nuclear/Coal)
 */
static Int16
IsItAPowerPlant(UInt8 point, UInt32 coord __attribute__((unused)),
    UInt8 flags __attribute__((unused)))
{
	switch (point) {
	case TYPE_POWER_PLANT:
		return (SUPPLY_POWER_PLANT);
	case TYPE_NUCLEAR_PLANT:
		return (SUPPLY_NUCLEAR_PLANT);
	default:
		return (0);
	}
}

/*
 * Check if the node is a Water Pump.
 * It is a usable water pump if it has power.
 */
static Int16
IsItAWaterPump(UInt8 point, UInt32 coord, UInt8 flags)
{
	if ((point == TYPE_WATER_PUMP) && (flags & POWEREDBIT) &&
	    ExistsNextto(coord, TYPE_REAL_WATER))
		return (SUPPLY_WATER_PUMP);
	return (0);
}

static Int16 (*DoesCarry)(UInt8);
static Int16 (*IsPlant)(UInt8, UInt32, UInt8);

static void *needSourceList;
static void *unvisitedNodes;
static Int8 flagToSet;
static Int16 SourceLeft;
static Int16 SourceTotal;
static Int16 NodesTotal;
static Int16 NodesSupplied;

/*
 * Set the supplied bit for the point specified
 */
static void
SetSupplied(UInt32 point)
{
	NodesSupplied++;
	OrWorldFlags(point, flagToSet);
}

/*
 * Add source to the grid.
 */
Int16
SupplyIfPlant(UInt32 pos, UInt8 point, UInt8 status)
{
	Int16 pt;
	if (!(pt = IsPlant(point, pos, status)))
		return (0);
	if (GetScratch(pos))
		return (0);
	SetSupplied(pos);
	SetScratch(pos);
	NodesTotal++;
	SourceLeft += pt;
	SourceTotal += pt;
	if (!StackIsEmpty(needSourceList)) {
		while (SourceLeft && !StackIsEmpty(needSourceList)) {
			pos = StackPop(needSourceList);
			SourceLeft--;
			SetSupplied(pos);
		}
	}
	return (pt);
}

/*
 * Do Distribution of the grid (water/power)
 */
static void
DoDistribute(Int16 grid)
{
	/* type == GRID_POWER | GRID_POWER */
	Int32 i, j;
	Int8 gw;

	SourceLeft = 0;
	SourceTotal = 0;
	NodesTotal = 0;
	NodesSupplied = 0;
	needSourceList = StackNew();
	unvisitedNodes = StackNew();

	/* Step 1: Find all the powerplants and move out from there */
	if (grid == GRID_POWER) {
		IsPlant = &IsItAPowerPlant;
		DoesCarry = &CarryPower;
		flagToSet = POWEREDBIT;
	} else {
		IsPlant = &IsItAWaterPump;
		DoesCarry = &CarryWater;
		flagToSet = WATEREDBIT;
	}

	LockWorld(); /* this lock locks for ALL power subs */
	LockWorldFlags();
	for (j = 0; j < GetMapMul(); j++)
		AndWorldFlags(j, ~(flagToSet | SCRATCHBIT));

	for (i = 0; i < GetMapMul(); i++) {
		gw = GetWorld(i);
		if (!GetScratch(i)) {
			if (SupplyIfPlant(i, gw, GetWorldFlags(i))) {
				AddNeighbors(i);
				DistributeUnvisited();
				/* unpowered points are removed */
				StackDoEmpty(needSourceList);
				WriteLog("Grid#%d Supplied Nodes: %d/%d "
				    "SrcRemain: %d/%d\n", grid, NodesSupplied,
				    NodesTotal, SourceLeft, SourceTotal);
				SourceLeft = SourceTotal = 0;
				NodesSupplied = NodesTotal = 0;
			}
		}
	}
	UnlockWorldFlags();
	UnlockWorld();
	StackDelete(needSourceList);
	StackDelete(unvisitedNodes);
}

/*
 * Distribute power to the unvisited list.
 */
static void
DistributeUnvisited(void)
{
	UInt32 pos;
	UInt8 flag;

	while (!StackIsEmpty(unvisitedNodes)) {
		pos = StackPop(unvisitedNodes);
		flag = GetWorldFlags(pos);
		if (SupplyIfPlant(pos, GetWorld(pos), flag)) {
			goto nextneighbor;
		}

		if (SourceLeft && ((flag & flagToSet) == 0)) {
			/*
			 * if this field hasn't been powered,
			 * we need to "use" some power to move further along
			 */
			SourceLeft--;
		}

		/* do we have more power left? */
		if (SourceLeft <= 0)
			StackPush(needSourceList, pos);
		else
			SetSupplied(pos);

		/* now, set the flags to indicate we've been here */
		SetScratch(pos);

nextneighbor:
		/* find the possible ways we can move on from here */
		AddNeighbors(pos);
	};
}

/*
 * Add all the neighbors to this node to the unvisited list.
 */
static void
AddNeighbors(UInt32 pos)
{
	char cross = DistributeNumberOfSquaresAround(pos);

	/* if there's "no way out", return */
	if ((cross & 0x0f) == 0)
		return;

	NodesTotal += cross & 0x0f;

	if ((cross & 0x10) == 0x10) {
		StackPush(unvisitedNodes, pos-GetMapSize());
	}
	if ((cross & 0x20) == 0x20) {
		StackPush(unvisitedNodes, pos+1);
	}
	if ((cross & 0x40) == 0x40) {
		StackPush(unvisitedNodes, pos+GetMapSize());
	}
	if ((cross & 0x80) == 0x80) {
		StackPush(unvisitedNodes, pos-1);
	}
}

/*
 * note that this function is used internally in the power distribution
 * routine. Therefore it will return false for tiles we've already been at,
 * to avoid backtracking any nodes we've already encountered.
 */
Int16
Carries(UInt32 pos)
{
	if (GetScratch(pos))
		return (0);
	return (DoesCarry(GetWorld(pos)));
}

/* gives a status of the situation around us */
Int16
DistributeNumberOfSquaresAround(UInt32 pos)
{
	/*
	 * return:
	 * 0001 00xx if up
	 * 0010 00xx if right
	 * 0100 00xx if down
	 * 1000 00xx if left
	 * xx = number of directions
	 *
	 * count fields that carry power
	 * do not look behind map border
	 */

	Int16 retval = 0;
	Int8 number = 0;

	if (Carries(DistributeMoveOn(pos, dtUp))) {
		retval |= 0x10;
		number++;
	}
	if (Carries(DistributeMoveOn(pos, dtRight))) {
		retval |= 0x20;
		number++;
	}
	if (Carries(DistributeMoveOn(pos, dtDown))) {
		retval |= 0x40;
		number++;
	}
	if (Carries(DistributeMoveOn(pos, dtLeft))) {
		retval |= 0x80;
		number++;
	}

	retval |= number;

	return (retval);
}

/*
 * this function take a position and a direction and
 * moves the position in the direction, but won't move
 * behind map borders
 */
static UInt32
DistributeMoveOn(UInt32 pos, dirType direction)
{
	switch (direction) {
	case dtUp:
		if (pos < GetMapSize())
			return (pos);
		pos -= GetMapSize();
		break;
	case dtRight:
		if ((pos%GetMapSize()+1) >= GetMapSize())
			return (pos);
		pos++;
		break;
	case dtDown:
		if ((pos+GetMapSize()) >= GetMapMul())
			return (pos);
		pos += GetMapSize();
		break;
	case dtLeft:
		if (pos%GetMapSize() == 0)
			return (pos);
		pos--;
		break;
	}
	return (pos);
}

static Int16
ExistsNextto(UInt32 pos, UInt8 what)
{
	if (GetWorld(pos - GetMapSize()) == what && !(pos < GetMapSize())) {
		return (1);
	}
	if (GetWorld(pos + 1) == what && !((pos+1) >= GetMapMul())) {
		return (1);
	}
	if (GetWorld(pos + GetMapSize()) == what &&
	    !((pos + GetMapSize()) >= GetMapMul())) {
		return (1);
	}
	if (GetWorld(pos - 1) == what && (pos != 0)) {
		return (1);
	}
	return (0);
}


/* Zones upgrade/downgrade */

typedef struct {
	UInt32 pos;
	Int32 score;
	Int16 used;
} ZoneScore;

ZoneScore zones[256];

/*
 * Find a bunch of zones and decide to upgrade/downgrade them
 */
void
FindZonesForUpgrading(void)
{
	Int16 i;
	Int32 randomZone;

	Int16 max = GetMapSize()*3;
	if (max > 256) { max = 256; }

	/* find some random zones */
	for (i = 0; i < max; i++) {
		zones[i].used = 0;
		randomZone = GetRandomZone();
		if (randomZone != -1) { /* -1 means we didn't find a zone */
			zones[i].pos = randomZone;
			zones[i].used = 1;
		}
	}
}

UInt16 counter = 0;

/*
 * The score finding routine is divided into small bits of 10 zones per run.
 * This is to free the programflow to take care of user interaction.
 * All functions in the simulation part should complete in under 3/4 second,
 * or the user might see the program as being slow.
 */
Int16
FindScoreForZones(void)
{
	Int16 i;
	Int32 score;
	counter += 20;

	for (i = counter-20; i < (signed)counter; i++) {
		if (i >= 256) {
		counter = 0;
		return (0);
	}

		if (zones[i].used == 1) {
			score = GetZoneScore(zones[i].pos);
			if (score != -1) {
				zones[i].score = score;
			} else {
				zones[i].used = 0;
				zones[i].score = -1;
				DowngradeZone(zones[i].pos);
			}
		}
	}
	return (1); /* there's still more zones that need a score. */
}

/*
 * Upgrade the best zones
 * XXX: this is an O(n^2) algorithm. It should be O(2n) at mo.st
 */
void
UpgradeZones(void)
{
	Int16 i, j, topscorer;
	Int32 topscore;
	Int16 downCount = 11 * 10 + 30;
	Int16 upCount = (0 - 8) * 10 + 250;

	/* upgrade the bests */
	for (i = 0; i < 256 && i < upCount; i++) {
		topscore = 0;
		topscorer = -1;

		/* find the one with max points */
		for (j = 0; j < 256; j++) {
			if (zones[j].score > topscore && zones[j].used == 1) {
				topscore = zones[j].score;
				topscorer = j;
			}
		}

		/* upgrade him/her/it/whatever */
		if (topscorer != -1) {
			if (zones[topscorer].used == 1) {
				zones[topscorer].used = 0;
				UpgradeZone(zones[topscorer].pos);
			}
		}
	}

	/* downgrade the worst */
	for (i = 0; i < 256 && i < downCount; i++) {
		topscore = -1;
		topscorer = -1;

		/* find the one with min points */
		for (j = 0; j < 256; j++) {
			if (zones[j].score < topscore && zones[j].used == 1) {
				topscore = zones[j].score;
				topscorer = j;
			}
		}

		/* downgrade him/her/it/whatever */
		if (topscorer != -1) {
			if (zones[topscorer].used == 1) {
				zones[topscorer].used = 0;
				DowngradeZone(zones[topscorer].pos);
			}
		}
	}
}

/*
 * Downgrade the zone at the position specified
 */
static void
DowngradeZone(UInt32 pos)
{
	int type;

	LockWorld();
	type = GetWorld(pos);
	if (type >= TYPE_COMMERCIAL_MIN && type <= TYPE_COMMERCIAL_MAX) {
		SetWorld(pos, (type == TYPE_COMMERCIAL_MIN) ?
		    ZONE_COMMERCIAL : type - 1);
		vgame.BuildCount[COUNT_COMMERCIAL]--;
	} else if (type >= TYPE_RESIDENTIAL_MIN &&
	    type <= TYPE_RESIDENTIAL_MAX) {
		SetWorld(pos, (type == TYPE_RESIDENTIAL_MIN) ?
		    ZONE_RESIDENTIAL : type - 1);
		vgame.BuildCount[COUNT_RESIDENTIAL]--;
	} else if (type >= TYPE_INDUSTRIAL_MIN && type <= TYPE_INDUSTRIAL_MAX) {
		SetWorld(pos, (type == TYPE_INDUSTRIAL_MIN) ?
		    ZONE_INDUSTRIAL : type - 1);
		vgame.BuildCount[COUNT_INDUSTRIAL]--;
	}
	UnlockWorld();
}

/*
 * Upgrade the zone a the position
 */
static void
UpgradeZone(UInt32 pos)
{
	int type;

	LockWorld();
	type = GetWorld(pos);
	if (type == ZONE_COMMERCIAL || (type >= TYPE_COMMERCIAL_MIN &&
	    type <= (TYPE_COMMERCIAL_MAX - 1))) {
		SetWorld(pos, (type == ZONE_COMMERCIAL) ?
		    TYPE_COMMERCIAL_MIN : type + 1);
		vgame.BuildCount[COUNT_COMMERCIAL]++;
	} else if (type == ZONE_RESIDENTIAL || (type >= TYPE_RESIDENTIAL_MIN &&
	    type <= (TYPE_RESIDENTIAL_MAX - 1))) {
		SetWorld(pos, (type == ZONE_RESIDENTIAL) ?
		    TYPE_RESIDENTIAL_MIN : type + 1);
		vgame.BuildCount[COUNT_RESIDENTIAL]++;
	} else if (type == ZONE_INDUSTRIAL || (type >= TYPE_INDUSTRIAL_MIN &&
	    type <= (TYPE_INDUSTRIAL_MAX - 1))) {
		SetWorld(pos, (type == ZONE_INDUSTRIAL) ?
		    TYPE_INDUSTRIAL_MIN : type + 1);
		vgame.BuildCount[COUNT_INDUSTRIAL]++;
	}
	UnlockWorld();
}

/*
 * Walk the road, looking for things at the end
 * XXX: actually do this
 */
static Int16
DoTheRoadTrip(UInt32 startPos __attribute__((unused)))
{
	return (1); /* for now */
}

/*
 * Describe the zone at the point specified
 * Crap: has strings, localization issues.
 * XXX: No strings.
 */
/*
void
describeZone(UInt8 zone, struct zoneTypeValue *ztv)
{
	ztv->value = 0;
	ztv->pollution = 0;
	ztv-> crime = 0;
	ztv->description[0] = '\0';
	if (zone == TYPE_DIRT)
		sprintf(ztv->description, "Dirt");
	if (zone == TYPE_COMMERCIAL)
		sprintf(ztv->description, "Commercial - seeded");
	if (zone == TYPE_RESIDENTIAL)
		sprintf(ztv->description, "Residential - seeded");
	if (zone == TYPE_INDUSTRIAL)
		sprintf(ztv->description, "Industrial - seeded");

	if (zone >= TYPE_COMMERCIAL_MIN && elt <= TYPE_COMMERCIAL_MAX) {
		sprintf(ztv->description, "Commercial");
		ztv->value = zone - TYPE_COMMERCIAL_MIN;
	}
	if (zone >= TYPE_RESIDENTIAL_MIN && elt <= TYPE_RESIDENTIAL_MAX) {
		sprintf(ztv->description, "Residential");
		ztv->value = zone - TYPE_RESIDENTIAL_MIN;
	}
	if (zone >= TYPE_INDUSTRIAL_MIN && elt <= TYPE_INDUSTRIAL_MAX) {
		sprintf(ztv->description, "Residential");
		ztv->value = zone - TYPE_INDUSTRIAL_MIN;
	}
	if (zone == TYPE_TREE) {
		sprintf(ztv->description, "Tree");
	}
	if (zone == TYPE_WATER) {
		sprintf(ztv->description, "Water");
	}
	if (zone == TYPE_REAL_WATER)
		sprintf(ztv->description, "Real Water");
	if (zone == TYPE_POWERROAD_1 || elt == TYPE_POWERROAD_2)
		sprintf(ztv->description, "Power/Road combination");
	if (zone == TYPE_POWER_LINE)
		sprintf(ztv->description, "Power Line");
	if (zone == TYPE_POWER_PLANT)
		sprintf(ztv->description, "Coal Power Plant");
	if (zone == TYPE_NUCLEAR_PLANT)
		sprintf(ztv->description, "Nuclear Power Plant");
	if (zone == TYPE_WASTE)
		sprintf(ztv->description, "Wasteland");
	if (zone == TYPE_FIRE_1 || zone == TYPE_FIRE_2 || zone == TYPE_FIRE_3)
		sprintf(ztv->description, "Fire!");
	if (zone == TYPE_FIRE_STATION)
		sprintf(ztv->description, "Fire Station");
	if (zone == TYPE_POLICE_STATION)
		sprintf(ztv->description, "Police Station");
	if (zone == TYPE_MILITARY_BASE)
		sprintf(ztv->description, "Military Base");
	if (zone == TYPE_WATER_PIPE)
		sprintf(ztv->description, "Water Pipe");
	if (zone == TYPE_WATER_PIPE)
		sprintf(ztv->description, "Water Pipe");

}
*/

/*
 * Get the score for this zone.
 * Returns -1 if the zone needs to be downgraded because of a lack of
 * water/power.
 */
long
GetZoneScore(UInt32 pos)
{
	long score = -1; /* I'm evil to begin with */
	int x = pos % GetMapSize();
	int y = pos / GetMapSize();
	int i, j;
	int bRoad = 0;
	zoneType type = ztWhat;
	UInt8 zone;

	LockWorld();
	zone = GetWorld(pos);
	type = (IsZone(zone, ztCommercial) ? ztCommercial :
	    (IsZone(zone, ztResidential) ? ztResidential : ztIndustrial));

	LockWorldFlags();
	if (((GetWorldFlags(pos) & POWEREDBIT) == 0) ||
	    ((GetWorldFlags(pos) & WATEREDBIT) == 0)) {
		/* whoops, no power | water */
		UnlockWorldFlags();
		goto unlock_ret;
	}
	UnlockWorldFlags();

	if (type != ztResidential)  {
		/*
		 * see if there's actually enough residential population
		 * to support a new zone of ind or com
		 */

		long availPop = (vgame.BuildCount[COUNT_RESIDENTIAL]*25)
		    - (vgame.BuildCount[COUNT_COMMERCIAL]*25
		    + vgame.BuildCount[COUNT_INDUSTRIAL]*25);
		/* pop is too low */
		if (availPop <= 0)
			goto unlock_ret;
	} else if (type == ztResidential) {
		/*
		 * the population can't skyrocket all at once, we need a cap
		 * somewhere - note, this should be fine tuned somehow
		 * A factor might be the number of (road/train/airplane)
		 * connections to the surrounding world - this would
		 * bring more potential residents into our little city
		 */
		long availPop = ((game.TimeElapsed*game.TimeElapsed)/35+30)
		    - (vgame.BuildCount[COUNT_RESIDENTIAL]);
		/* hmm - need more children */
		if (availPop <= 0)
			goto unlock_ret;
	}

	if (type == ztCommercial) {
		/*
		 * and what is a store without something to sell? We need
		 * enough industrial zones before commercial zones kick in.
		 */

		long int availGoods = (vgame.BuildCount[COUNT_INDUSTRIAL]/3*2)
		    - (vgame.BuildCount[COUNT_COMMERCIAL]);
		/* darn, nothing to sell here */
		if (availGoods <= 0)
			goto unlock_ret;
	}


	/* take a look around at the enviroment */
	for (i = x - 3; i < 4 + x; i++) {
		for (j = y - 3; j < 4 + y; j++) {
			if (!(i < 0 || i >= GetMapSize() || j < 0 ||
			    j >= GetMapSize())) {
				score += GetScoreFor(type,
				    GetWorld(WORLDPOS(i, j)));
				if (IsRoad(GetWorld(WORLDPOS(i, j))) &&
				    bRoad == 0) {
					/*
					 * can we reach all kinds of
					 * zones from here?
					 */
					bRoad = DoTheRoadTrip(WORLDPOS(i, j));
					if (!bRoad) {
						score = -1;
						goto unlock_ret;
					}
				}
			}
		}
	}

unlock_ret:
	UnlockWorld();
	return (score);
}

/*
 * Get the score for the zone specified.
 */
Int16
GetScoreFor(UInt8 iamthis, UInt8 what)
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
	if (what == TYPE_POWER_PLANT) {
		return (iamthis == ztCommercial) ? (-75) :
			((iamthis == ztResidential) ? (-100) :
			((iamthis == ztIndustrial) ? 30 : (-75)));
	}
	if (what == TYPE_NUCLEAR_PLANT) {
		return (iamthis == ztCommercial) ? (-150) :
			((iamthis == ztResidential) ? (-200) :
			((iamthis == ztIndustrial) ? 15 : (0-175)));
	}
	if (what == TYPE_TREE) {
		return (iamthis == ztCommercial) ? 50 :
			((iamthis == ztResidential) ? 85 :
			((iamthis == ztIndustrial)? 25 : 50));
	}
	if ((what == TYPE_WATER) || (what == TYPE_REAL_WATER)) {
		return (iamthis == ztCommercial) ? 175 :
			((iamthis == ztResidential) ? 550 :
			((iamthis == ztIndustrial) ? 95 : 250));
	}
	return (0);
}

/*
 * Get a zone on the world.
 * Must be one of the Residential / Industrial /commercial zones
 * XXX: remove the magic numbers
 */
UInt32
GetRandomZone()
{
	UInt32 pos = 0;
	UInt16 i;
	UInt8 type;

	LockWorld();
	for (i = 0; i < 5; i++) { /* try five times to hit a valid zone */
		pos = GetRandomNumber(GetMapMul());
		type = GetWorld(pos);
		if ((type >= 1 && type <= 3) || (type >= 30 && type <= 59)) {
			UnlockWorld();
			return (pos);
		}
	}

	UnlockWorld();
	return (~(UInt32)0);
}

static const struct countCosts {
	UInt32	count;
	UInt32	cost;
} countCosts[] = {
	{ COUNT_RESIDENTIAL, INCOME_RESIDENTIAL },
	{ COUNT_COMMERCIAL, INCOME_COMMERCIAL },
	{ COUNT_INDUSTRIAL, INCOME_INDUSTRIAL },
	{ COUNT_ROADS, UPKEEP_ROAD },
	{ COUNT_TREES, 0 },
	{ COUNT_WATER, 0 },
	{ COUNT_POWERLINES, UPKEEP_POWERLINE },
	{ COUNT_POWERPLANTS, UPKEEP_POWERPLANT },
	{ COUNT_NUCLEARPLANTS, UPKEEP_NUCLEARPLANT },
	{ COUNT_WASTE, 0 },
	{ COUNT_FIRE, 0 },
	{ COUNT_FIRE_STATIONS, UPKEEP_FIRE_STATIONS },
	{ COUNT_POLICE_STATIONS, UPKEEP_POLICE_STATIONS },
	{ COUNT_MILITARY_BASES, UPKEEP_MILITARY_BASES },
	{ COUNT_WATERPIPES, 0 },
	{ COUNT_WATER_PUMPS, 0 }
};

static Int32
costIt(Int32 item)
{
	return (vgame.BuildCount[item] * countCosts[item].cost);
}

/*
 * Get a number for the budget.
 */
Int32
BudgetGetNumber(BudgetNumber type)
{
	Int32 ret = 0;
	switch (type) {
	case bnResidential:
		ret = (Int32)costIt(COUNT_RESIDENTIAL) * game.tax/100;
		break;
	case bnCommercial:
		ret = (Int32)costIt(COUNT_COMMERCIAL) * game.tax/100;
		break;
	case bnIndustrial:
		ret = (Int32)costIt(COUNT_INDUSTRIAL) * game.tax/100;
		break;
	case bnTraffic:
		ret = (Int32)(costIt(COUNT_ROADS) *
		    game.upkeep[UPKEEPS_TRAFFIC]) / 100;
		break;
	case bnPower:
		ret = (Int32)((costIt(COUNT_POWERLINES) +
		    costIt(COUNT_NUCLEARPLANTS) + costIt(COUNT_POWERPLANTS)) *
		    game.upkeep[UPKEEPS_POWER]) / 100;
		break;
	case bnDefence:
		ret = (Int32)((costIt(COUNT_FIRE_STATIONS) +
		    costIt(COUNT_POLICE_STATIONS) +
		    costIt(COUNT_MILITARY_BASES)) *
		    game.upkeep[UPKEEPS_DEFENCE])/100;
		break;
	case bnCurrentBalance:
		ret = game.credits;
		break;
	case bnChange:
		ret = (Int32)BudgetGetNumber(bnResidential)
			+ BudgetGetNumber(bnCommercial)
			+ BudgetGetNumber(bnIndustrial)
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

/*
 * Add money because of taxes
 */
void
DoTaxes()
{
	game.credits += BudgetGetNumber(bnResidential) +
	    BudgetGetNumber(bnCommercial) + BudgetGetNumber(bnIndustrial);
}

/*
 * Take away money because of upkeep costs
 */
void
DoUpkeep()
{
	UInt32 upkeep;

	upkeep = BudgetGetNumber(bnTraffic) + BudgetGetNumber(bnPower) +
	    BudgetGetNumber(bnDefence);

	if (upkeep <= (UInt32)game.credits) {
		game.credits -= upkeep;
		return;
	}
	WriteLog("*** Negative Cashflow\n");
	game.credits = 0;

	/* roads */
	DoNastyStuffTo(TYPE_ROAD, 1);
	DoNastyStuffTo(TYPE_POWER_LINE, 5);
	DoNastyStuffTo(TYPE_POWER_PLANT, 15);
	DoNastyStuffTo(TYPE_NUCLEAR_PLANT, 50);
	DoNastyStuffTo(TYPE_FIRE_STATION, 10);
	DoNastyStuffTo(TYPE_POLICE_STATION, 12);
	DoNastyStuffTo(TYPE_MILITARY_BASE, 35);
}

/*
 * Perform a phase of the simulation
 * XXX: break into separate functions and a jump table.
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
		WriteLog("Simulation phase 5 - Upgrade Zones\n");
		UpgradeZones();
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
		game.TimeElapsed++;
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

/*
 * Updates the BuildCount array after a load game
 */
void
UpdateVolatiles(void)
{
	UInt32 p;

	LockWorld();

	for (p = 0; p < GetMapMul(); p++) {
		UInt8 elt = GetWorld(p);
		/* Gahd this is terrible. I need to fix it. */
		if (elt >= TYPE_COMMERCIAL_MIN && elt <= TYPE_COMMERCIAL_MAX)
			vgame.BuildCount[COUNT_COMMERCIAL] += elt%10 + 1;
		if (elt >= TYPE_RESIDENTIAL_MIN && elt <= TYPE_RESIDENTIAL_MAX)
			vgame.BuildCount[COUNT_RESIDENTIAL] += elt%10 + 1;
		if (elt >= TYPE_INDUSTRIAL_MIN && elt <= TYPE_INDUSTRIAL_MAX)
			vgame.BuildCount[COUNT_INDUSTRIAL] += elt%10 + 1;
		if (IsRoad(elt)) vgame.BuildCount[COUNT_ROADS]++;
		if (elt == TYPE_TREE) vgame.BuildCount[COUNT_TREES]++;
		if (elt == TYPE_WATER) vgame.BuildCount[COUNT_WATER]++;
		if (elt == TYPE_POWERROAD_2 || elt == TYPE_POWERROAD_1) {
			vgame.BuildCount[COUNT_POWERLINES]++;
			vgame.BuildCount[COUNT_ROADS]++;
		}
		if (elt == TYPE_POWER_LINE)
			vgame.BuildCount[COUNT_POWERLINES]++;
		if (elt == TYPE_POWER_PLANT)
			vgame.BuildCount[COUNT_POWERPLANTS]++;
		if (elt == TYPE_NUCLEAR_PLANT)
			vgame.BuildCount[COUNT_NUCLEARPLANTS]++;
		if (elt == TYPE_WASTE) vgame.BuildCount[COUNT_WASTE]++;
		if (elt == TYPE_FIRE1 || elt == TYPE_FIRE2 || elt == TYPE_FIRE3)
			vgame.BuildCount[COUNT_FIRE]++;
		if (elt == TYPE_FIRE_STATION)
			vgame.BuildCount[COUNT_FIRE_STATIONS]++;
		if (elt == TYPE_POLICE_STATION)
			vgame.BuildCount[COUNT_POLICE_STATIONS]++;
		if (elt == TYPE_MILITARY_BASE)
			vgame.BuildCount[COUNT_MILITARY_BASES]++;
		if (elt == TYPE_WATER_PIPE)
			vgame.BuildCount[COUNT_WATERPIPES]++;
		if (elt == TYPE_WATERROAD_1 || elt == TYPE_WATERROAD_2) {
			vgame.BuildCount[COUNT_WATERPIPES]++;
			vgame.BuildCount[COUNT_ROADS]++;
		}
		if (elt == TYPE_ROAD)
			vgame.BuildCount[COUNT_ROADS]++;

		if (elt == TYPE_WATER_PUMP)
			vgame.BuildCount[COUNT_WATER_PUMPS]++;
	}
	UnlockWorld();
}
