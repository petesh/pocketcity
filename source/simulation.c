#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"
#include "disaster.h"
#include "simulation.h"
#include "stack.h"

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

void DoTaxes(void);
void DoUpkeep(void);
int DistributeNumberOfSquaresAround(unsigned long pos);
int DistributeFieldCanCarry(unsigned long pos);

int ExistsNextto(unsigned long pos, unsigned char what);

void UpgradeZones(void);
static void UpgradeZone(unsigned long pos);
static void DowngradeZone(unsigned long pos);
static int DoTheRoadTrip(unsigned long startPos);
static unsigned long DistributeMoveOn(unsigned long pos, dirType direction);
static void DistributeUnvisited(void);

signed long GetZoneScore(unsigned long pos);
signed int GetScoreFor(unsigned char iamthis, unsigned char what);
long unsigned int GetRandomZone(void);
void FindZonesForUpgrading(void);
int FindScoreForZones(void);
static void AddNeighbors(unsigned long pos);

/*
 * The power/water grid is updated using the following mechanism:
 * 1: While there are more plants to consume do:
 * a: While you can visit more points do:
 * i: If it's a source then add it's donation to the supply
 *      - if the 'to be powered' list has members then supply them
 *      - go to step (1.a.iii)
 * ii: if you've power then mark the point as visited, supplied & decrement
 *      - otherwise put it on the 'to be powered' list
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
 *      call the distribution routine... it knows how to do each type
 */
static void DoDistribute(int grid, int pc);

extern void
Sim_Distribute(void)
{
    int pc;
    if (NeedsUpdate(GRID_POWER)) {
        pc = (vgame.BuildCount[COUNT_POWERPLANTS] +
          vgame.BuildCount[COUNT_NUCLEARPLANTS]);
        DoDistribute(GRID_POWER, pc);
        ClearUpdate(GRID_POWER);
    }
    if (NeedsUpdate(GRID_WATER)) {
        pc = vgame.BuildCount[COUNT_WATER_PUMPS];
        DoDistribute(GRID_WATER, pc);
        ClearUpdate(GRID_WATER);
    }
}

void
Sim_Distribute_Specific(int gridonly)
{
    int pc;
    if (gridonly == 0) gridonly = GRID_ALL;

    if (NeedsUpdate(GRID_POWER) && (gridonly & GRID_POWER)) {
        pc = (vgame.BuildCount[COUNT_POWERPLANTS] +
          vgame.BuildCount[COUNT_NUCLEARPLANTS]);
        DoDistribute(GRID_POWER, pc);
        ClearUpdate(GRID_POWER);
    }
    if (NeedsUpdate(GRID_WATER) && (gridonly & GRID_WATER)) {
        pc = vgame.BuildCount[COUNT_WATER_PUMPS];
        DoDistribute(GRID_WATER, pc);
        ClearUpdate(GRID_WATER);
    }
}

static int
IsItAPowerPlant(unsigned char point, unsigned char flags)
{
    switch (point) {
    case TYPE_POWER_PLANT: return (SUPPLY_POWER_PLANT);
    case TYPE_NUCLEAR_PLANT: return (SUPPLY_NUCLEAR_PLANT);
    default: return (0);
    }
}

static int
IsItAWaterPump(unsigned char point, unsigned char flags)
{
    if ((point == TYPE_WATER_PUMP) && (flags & POWEREDBIT))
      return (SUPPLY_WATER_PUMP);
    return (0);
}

static int (*DoesCarry)(unsigned char);
static int (*IsPlant)(unsigned char, unsigned char);

static void *needSourceList;
static void *unvisitedNodes;
static char flagToSet;
static int SourceLeft;
static int SourceTotal;
static int NodesTotal;
static int NodesSupplied;

static void
SetSupplied(unsigned long point)
{
    NodesSupplied++;
    OrWorldFlags(point, flagToSet);
}

int
SupplyIfPlant(unsigned long pos, unsigned char point, unsigned char status)
{
    int pt;
    if (!(pt= IsPlant(point, status))) return (0);
    if (GetScratch(pos)) return (0);
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

static void
DoDistribute(int grid, int pc)
{
    /* type == GRID_POWER | GRID_POWER */
    unsigned long i, j;
    char gw;

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
#if defined(DEBUG)
    		{ char op[80]; sprintf(op, "Grid#%d Supplied Nodes: %d/%d "
                  " SrcRemain: %d/%d\n", grid,
                  NodesSupplied, NodesTotal,
                  SourceLeft, SourceTotal);
		UIWriteLog(op); }
#endif
    		SourceLeft = SourceTotal = 0;
		NodesSupplied = NodesTotal = 0;
            }
        }
    }
    UnlockWorld();
    UnlockWorldFlags();
    StackDelete(needSourceList);
    StackDelete(unvisitedNodes);
}

static void
DistributeUnvisited(void)
{
    unsigned long pos;
    unsigned char flag;

    while (!StackIsEmpty(unvisitedNodes)) {
        pos = StackPop(unvisitedNodes);
        flag = GetWorldFlags(pos);
        if (SupplyIfPlant(pos, GetWorld(pos), flag)) {
            goto nextneighbor;
        }

        if (SourceLeft && ((flag & flagToSet) == 0)) {
            /*
             * if this field hasn't been powered, we need to "use" some power
             * to move further along
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

static void
AddNeighbors(unsigned long pos)
{
    char cross = DistributeNumberOfSquaresAround(pos);

    /* if there's "no way out", return */
    if ((cross & 0x0f) == 0) { return; }

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
int
Carries(unsigned long pos)
{
    if (GetScratch(pos)) return (0);
    return (DoesCarry(GetWorld(pos)));
}

/* gives a status of the situation around us */
int DistributeNumberOfSquaresAround(unsigned long pos)
{
    /* return:
     * 0001 00xx if up
     * 0010 00xx if right
     * 0100 00xx if down
     * 1000 00xx if left
     * xx = number of directions
     *
     * count fields that carry power
     * do not look behind map border
     */

    char retval=0;
    char number=0;

    if (Carries(DistributeMoveOn(pos, dtUp))) { retval |= 0x10; number++; }
    if (Carries(DistributeMoveOn(pos, dtRight))) { retval |= 0x20; number++; }
    if (Carries(DistributeMoveOn(pos, dtDown))) { retval |= 0x40; number++; }
    if (Carries(DistributeMoveOn(pos, dtLeft))) { retval |= 0x80; number++; }

    retval |= number;

    return retval;
}

/*
 * this function take a position and a direction and
 * moves the position in the direction, but won't move
 * behind map borders
 */
static unsigned long
DistributeMoveOn(unsigned long pos, dirType direction)
{
    switch (direction) {
        case dtUp:
            if (pos < GetMapSize()) { return pos; }
            pos -= GetMapSize();
            break;
        case dtRight:
            if ((pos%GetMapSize()+1) >= GetMapSize()) { return pos; }
            pos++;
            break;
        case dtDown:
            if ((pos+GetMapSize()) >= GetMapMul()) { return pos; }
            pos += GetMapSize();
            break;
        case dtLeft:
            if (pos%GetMapSize() == 0) { return pos; }
            pos--;
            break;
    }
    return pos;
}

int ExistsNextto(unsigned long int pos, unsigned char what)
{
    if (GetWorld(pos-GetMapSize())==what && !(pos < GetMapSize())) { return 1; }
    if (GetWorld(pos+1)==what && !((pos+1) >= GetMapMul())) { return 1; }
    if (GetWorld(pos+GetMapSize())==what && !((pos+GetMapSize()) >= GetMapMul())) { return 1; }
    if (GetWorld(pos-1)==what && pos != 0) { return 1; }
    return 0;
}


/* Zones upgrade/downgrade */

typedef struct {
    long unsigned pos;
    long signed score;
    int used;
} ZoneScore;

ZoneScore zones[256];

void FindZonesForUpgrading()
{
    int i;
    long randomZone;

    int max = GetMapSize()*3;
    if (max > 256) { max = 256; }

    /* find some random zones */
    for (i=0; i<max; i++)
    {
        zones[i].used = 0;
        randomZone = GetRandomZone();
        if (randomZone != -1) { /* -1 means we didn't find a zone */
            zones[i].pos = randomZone;
            zones[i].used = 1;
        }
    }
}


/*
    The score finding routine is divided into small
    bits of 10 zones per run.
    This is to free the programflow to take care of
    user interaction - in general, all functions
    in the simulation part should complete in under 3/4 second,
    or the user might see the program as being slow.

    Still wondering why there's no support for multitaskning in PalmOS...
    */

unsigned int counter=0;
int FindScoreForZones()
{
    int i;
    long score;
    counter += 20;
    for (i=counter-20; i<counter; i++)
    {
        if (i>=256) { counter = 0; return 0; } /* this was the last zone! */

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
    return 1; /* there's still more zones that need a score. */
}


void UpgradeZones()
{
    int i,j,topscorer;
    long topscore;

    int downCount = 11*10+30;
    int upCount = (0-8)*10+250;

    /* upgrade the bests */
    for (i=0; i<256 && i<upCount; i++)
    {
        topscore = 0;
        topscorer = -1;

        /* find the one with max points */
        for (j=0; j<256; j++)
        {
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
    for (i=0; i<256 && i<downCount; i++)
    {
        topscore = -1;
        topscorer = -1;

        /* find the one with min points */
        for (j=0; j<256; j++)
        {
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



static void
DowngradeZone(unsigned long pos)
{
    int type;
    LockWorld();

    type = GetWorld(pos);
    if (type >= TYPE_COMMERCIAL_MIN && type <= TYPE_COMMERCIAL_MAX)
    {
        SetWorld(pos, (type == TYPE_COMMERCIAL_MIN) ? ZONE_COMMERCIAL : type-1);
        vgame.BuildCount[COUNT_COMMERCIAL]--;
    }
    else if (type >= TYPE_RESIDENTIAL_MIN && type <= TYPE_RESIDENTIAL_MAX)
    {
        SetWorld(pos, (type == TYPE_RESIDENTIAL_MIN) ? ZONE_RESIDENTIAL :
          type-1);
        vgame.BuildCount[COUNT_RESIDENTIAL]--;
    }
    else if (type >= TYPE_INDUSTRIAL_MIN && type <= TYPE_INDUSTRIAL_MAX)
    {
        SetWorld(pos, (type == TYPE_INDUSTRIAL_MIN) ? ZONE_INDUSTRIAL : type-1);
        vgame.BuildCount[COUNT_INDUSTRIAL]--;
    }

    UnlockWorld();
}

static void
UpgradeZone(unsigned long pos)
{
    int type;

    LockWorld();

    type = GetWorld(pos);

    if (type == ZONE_COMMERCIAL || (type >= TYPE_COMMERCIAL_MIN &&
          type <= (TYPE_COMMERCIAL_MAX - 1))) {
        SetWorld(pos, (type == ZONE_COMMERCIAL) ? TYPE_COMMERCIAL_MIN : type+1);
        vgame.BuildCount[COUNT_COMMERCIAL]++;
    } else if (type == ZONE_RESIDENTIAL || (type >= TYPE_RESIDENTIAL_MIN &&
          type <= (TYPE_RESIDENTIAL_MAX - 1))) {
        SetWorld(pos, (type == ZONE_RESIDENTIAL) ? TYPE_RESIDENTIAL_MIN :
          type+1);
        vgame.BuildCount[COUNT_RESIDENTIAL]++;
    } else if (type == ZONE_INDUSTRIAL || (type >= TYPE_INDUSTRIAL_MIN &&
          type <= (TYPE_INDUSTRIAL_MAX - 1))) {
        SetWorld(pos, (type == ZONE_INDUSTRIAL) ? TYPE_INDUSTRIAL_MIN : type+1);
        vgame.BuildCount[COUNT_INDUSTRIAL]++;
    }
    UnlockWorld();
}

static int
DoTheRoadTrip(unsigned long startPos)
{
    return (1); /* for now */
}


long
GetZoneScore(unsigned long pos)
{
    /*
     * return -1 to make this zone be downgraded _right now_
     * (ie. if missing things as power or roads)
     */

    long score = -1; /* I'm evil to begin with */
    int x = pos % GetMapSize();
    int y = pos / GetMapSize();
    int i, j;
    int bRoad = 0;
    zoneType type = 0;

    LockWorld();
    type = GetWorld(pos);
    type = (IsZone(type, ztCommercial) ? ztCommercial :
      (IsZone(type, ztResidential) ? ztResidential : ztIndustrial));

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
	 * see if there's actually enough residential population to support
         * a new zone of ind or com
	 */

        long availPop = 
                (vgame.BuildCount[COUNT_RESIDENTIAL]*25)
                - (vgame.BuildCount[COUNT_COMMERCIAL]*25 +
                vgame.BuildCount[COUNT_INDUSTRIAL]*25);
	/* pop is too low */
        if (availPop <= 0) goto unlock_ret;

    } else if (type == ztResidential) {
        /*
         * the population can't skyrocket all at once, we need a cap
         * somewhere - note, this should be fine tuned somehow
         * A factor might be the number of (road/train/airplane) connections
         * to the surrounding world - this would bring more potential
         * residents into our little city
         */
        long availPop =
                ((game.TimeElapsed*game.TimeElapsed)/35+30)
                - (vgame.BuildCount[COUNT_RESIDENTIAL]);
	/* hmm - need more children */
        if (availPop <= 0) goto unlock_ret;
    }

    if (type == ztCommercial) {
        /*
	 * and what is a store without something to sell? therefore we need
         * enough industrial zones before commercial zones kick in.
	 */

        long signed int availGoods =
                (vgame.BuildCount[COUNT_INDUSTRIAL]/3*2)
                - (vgame.BuildCount[COUNT_COMMERCIAL]);
	/* darn, nothing to sell here */
        if (availGoods <= 0) goto unlock_ret;
    }


    /* take a look around at the enviroment */
    for (i=x-3; i<4+x; i++) {
        for (j=y-3; j<4+y; j++) {
            if (!(i<0 || i>=GetMapSize() || j<0 || j>=GetMapSize())) {
                score += GetScoreFor(type, GetWorld(WORLDPOS(i,j)));
                if (IsRoad(GetWorld(WORLDPOS(i,j))) && bRoad == 0) {
                    /* can we reach all kinds of zones from here? */
                    bRoad = DoTheRoadTrip(WORLDPOS(i,j));
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


int
GetScoreFor(unsigned char iamthis, unsigned char what)
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
            ((iamthis == ztIndustrial) ? 550 :
            ((iamthis == ztIndustrial) ? 95 : 250));
    }
    return 0;
}


unsigned long
GetRandomZone()
{
    unsigned long pos = 0;
    int i;
    unsigned char type;

    LockWorld();
    for (i=0; i<5; i++) /* try five times to hit a valid zone */
    {
        pos = GetRandomNumber(GetMapMul());
        type = GetWorld(pos);
        if ((type >= 1 && type <= 3) || (type >= 30 && type <= 59)) {
            UnlockWorld();
            return pos;
        }
    }

    UnlockWorld();
    return -1;
}

long
BudgetGetNumber(BudgetNumber type)
{
    long ret = 0;
    switch (type) {
        case bnResidential:
            ret = (long)vgame.BuildCount[COUNT_RESIDENTIAL]
                    * INCOME_RESIDENTIAL
                    * game.tax/100;
            break;
        case bnCommercial:
            ret = (long)vgame.BuildCount[COUNT_COMMERCIAL]
                    * INCOME_COMMERCIAL
                    * game.tax/100;
            break;
        case bnIndustrial:
            ret = (long)vgame.BuildCount[COUNT_INDUSTRIAL]
                    * INCOME_INDUSTRIAL
                    * game.tax/100;
            break;
        case bnTraffic:
            ret = (long)(vgame.BuildCount[COUNT_ROADS] * UPKEEP_ROAD
                    * game.upkeep[UPKEEPS_TRAFFIC])/100;
            break;
        case bnPower:
            ret = (long)((vgame.BuildCount[COUNT_POWERLINES]*UPKEEP_POWERLINE +
                    vgame.BuildCount[COUNT_NUCLEARPLANTS]*UPKEEP_NUCLEARPLANT +
                    vgame.BuildCount[COUNT_POWERPLANTS]*UPKEEP_POWERPLANT)
                    * game.upkeep[UPKEEPS_POWER])/100;
            break;
        case bnDefence:
            ret = (long)((vgame.BuildCount[COUNT_FIRE_STATIONS]*UPKEEP_FIRE_STATIONS +
                     vgame.BuildCount[COUNT_POLICE_STATIONS]*UPKEEP_POLICE_STATIONS +
                     vgame.BuildCount[COUNT_MILITARY_BASES]*UPKEEP_MILITARY_BASES)
                    * game.upkeep[UPKEEPS_DEFENCE])/100;
            break;
        case bnCurrentBalance:
            ret = game.credits;
            break;
        case bnChange:
            ret = (long)BudgetGetNumber(bnResidential)
                  + BudgetGetNumber(bnCommercial)
                  + BudgetGetNumber(bnIndustrial)
                  - BudgetGetNumber(bnTraffic)
                  - BudgetGetNumber(bnPower)
                  - BudgetGetNumber(bnDefence);
            break;
        case bnNextMonth:
            ret = (long)BudgetGetNumber(bnCurrentBalance) +
                  BudgetGetNumber(bnChange);
            break;
    }

    return ret;
}

void DoTaxes()
{
    game.credits += BudgetGetNumber(bnResidential) +
                    BudgetGetNumber(bnCommercial) +
                    BudgetGetNumber(bnIndustrial);
}

void DoUpkeep()
{
    unsigned long upkeep;

    upkeep = BudgetGetNumber(bnTraffic) +
             BudgetGetNumber(bnPower) +
             BudgetGetNumber(bnDefence);

    if (upkeep <= game.credits) {
        game.credits -= upkeep;
        return;
    }
    UIWriteLog("*** Negative Cashflow\n");
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

extern int Sim_DoPhase(int nPhase)
{
    switch (nPhase)
    {
        case 1:
            if (NeedsUpdate(GRID_POWER)) {
                UIWriteLog("Simulation phase 1 - power grid\n");
                Sim_Distribute_Specific(GRID_POWER);
                ClearUpdate(GRID_POWER);
            }
            nPhase =2;
            break;
        case 2:
            if (NeedsUpdate(GRID_WATER)) {
                UIWriteLog("Simulation phase 2 - water grid\n");
                Sim_Distribute_Specific(GRID_WATER);
                ClearUpdate(GRID_WATER);
            }
            nPhase = 3;
            break;
        case 3:
            UIWriteLog("Simulation phase 3 - Find zones for upgrading\n");
            FindZonesForUpgrading();
            nPhase=4;
	    /* this can't be below */
            UIWriteLog("Simulation phase 4 - Find score for zones\n");
            break;
        case 4:
            if (FindScoreForZones() == 0) {
                nPhase=5;
            }
            break;
        case 5:
            UIWriteLog("Simulation phase 5 - Upgrade Zones\n");
            UpgradeZones();
            nPhase=6;
            break;
        case 6:
            UIWriteLog("Simulation phase 6 - Update disasters\n");
            /* UpdateDisasters(); */
            DoRandomDisaster();
            nPhase = 7;
            break;
        case 7:
            UIWriteLog("Simulation phase 7 - Economics\n");
            DoTaxes();
            game.TimeElapsed++;
            nPhase=0;
            UIInitDrawing();
            UIDrawCredits();
            UIDrawPop();
            UICheckMoney();
            DoUpkeep();
            UIFinishDrawing();
            break;
    }

    return nPhase;
}

extern void
UpdateVolatiles(void)
{
    /* Updates the BuildCount array after a load game */
    long p;

    LockWorld();

    for (p = 0; p < GetMapMul(); p++) {
        char elt = GetWorld(p);
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
        if (elt == TYPE_POWERROAD_2 || elt == TYPE_POWERROAD_1 ||
          elt == TYPE_POWER_LINE) vgame.BuildCount[COUNT_POWERLINES]++;
        if (elt == TYPE_POWER_PLANT) vgame.BuildCount[COUNT_POWERPLANTS]++;
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
        if (elt == TYPE_WATER_PIPE) vgame.BuildCount[COUNT_WATERPIPES]++;
        if (elt == TYPE_WATER_PUMP) vgame.BuildCount[COUNT_WATER_PUMPS]++; 
    }
    UnlockWorld();
}
