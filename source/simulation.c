#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"
#include "disaster.h"
#include "simulation.h"

int powerleft = 0;
char distributetype = 0;

void DistributeMoveOnFromThisPoint(unsigned long pos);
void DoTaxes(void);
void DoUpkeep(void);
unsigned long DistributeMoveOn(unsigned long pos, int direction);
int DistributeNumberOfSquaresAround(unsigned long pos);
int DistributeFieldCanCarry(unsigned long pos);

int ExistsNextto(unsigned long int pos, unsigned char what);

void UpgradeZones(void);
void UpgradeZone(long unsigned pos);
void DowngradeZone(long unsigned pos);
int DoTheRoadTrip(long unsigned int startPos);
signed long GetZoneScore(long unsigned int pos);
signed int GetScoreFor(unsigned char iamthis, unsigned char what);
long unsigned int GetRandomZone(void);
void FindZonesForUpgrading(void);
int FindScoreForZones(void);

/* The power grid is updated using a recursive function, here's how it
 * basicly works:
 * 1: scan the playingfield for a power plant and start there
 * 2: move up (recursivly) from this plant, decrementing the powerleft var
 * and marking every touched zone so it won't get power twice.
 * 3: when there's no more power left, go back to 1 and scan for the next
 * power plant
 *
 * The WorldFlag are used as a bitfield for this, every tile has a byte:
 *  8765 4321
 *  |||| |||`- 1 = this tile is powered
 *  |||| ||`-- used as a marked for "already been here" in several routines
 *  |||| |`--- 1 = this tile is watered
 *  |||| `----
 *  |||`------
 *  ||`-------
 *  |`--------
 *  `--------- 
 *
 *  don't use any of the free flags without asking zakarun (thanks)
 *  please note that the flags are _not_ saved, they _must_ be able to be
 *  recreated from the plain world[] array - else the savegames would be
 *  10k larger (that's A LOT ;)
 *  
 *  How to recreate:
 *  1: call Sim_Distribute(0)
 *  2: no need (only used as a temporary var)
 *  3: call Sim_Distribute(1)
 *  4:
 *  5:
 *  6:
 *  7:
 *  8:
 */

extern void Sim_Distribute(char type)
{
    // type == 0: power
    // type == 1: water
    unsigned long i,j;
    distributetype = type;

    // reset powergrid - ie, clear flags 1 & 2
    // or flags 1 & 3 for watergrid
    LockWorldFlags();
    for (j = 0; j < GetMapMul(); j++) {
        SetWorldFlags(j, GetWorldFlags(j) & (type==TYPEPOWER?
              ~(POWEREDBIT | SCRATCHBIT):~(WATEREDBIT | SCRATCHBIT)));
    }

    // Step 1: Find all the powerplants and move out from there
    LockWorld(); // this lock locks for ALL power subs
    for (i = 0; i < GetMapMul(); i++) {
        if (GetWorld(i) == TYPE_POWER_PLANT 
                || GetWorld(i) == TYPE_NUCLEAR_PLANT
                || GetWorld(i) == TYPE_WATER_PUMP) { // is this a source?
            powerleft=0;
            if (distributetype == 0) {
                if (GetWorld(i) == TYPE_POWER_PLANT)
                    powerleft += 100; // if this is a plant 
                if (GetWorld(i) == TYPE_NUCLEAR_PLANT)
                    powerleft += 300; // we get more power
            } else if (distributetype == 1) {
                if (GetWorld(i) == TYPE_WATER_PUMP
                && (GetWorldFlags(i) & POWEREDBIT) == 1 &&
                ExistsNextto(i, TYPE_REAL_WATER))
                    powerleft += 200; // pumps need power and REAL_WATER
            }

            // begin the distribution
            DistributeMoveOnFromThisPoint(i);

            // prepare for next round
            for (j = 0; j < GetMapMul(); j++) {
                SetWorldFlags(j, GetWorldFlags(j) & ~SCRATCHBIT); 
            }
        }
    }
    UnlockWorld();
    UnlockWorldFlags();
}

void DistributeMoveOnFromThisPoint(unsigned long pos)
{
    // a recursive function
    // pos: here we start, all fields will be powered from here
    char cross = 0;
    char direction = 0;

    do {
        if (((GetWorldFlags(pos) & POWEREDBIT) == 0 &&
              distributetype == TYPEPOWER) ||
            ((GetWorldFlags(pos) & WATEREDBIT) == 0 &&
             distributetype == TYPEWATER)) {
            /* if this field hasn't been powered, we need to "use" some power
             * to move further along
             */
            powerleft--;
        }

        // do we have more power left?
        if (powerleft < 1) { powerleft = 0; return; }
        // now, set the two flags, to indicate
        // 1: we've been here (look 4 lines above)
        // 2: this field is now powered
        SetWorldFlags(pos, GetWorldFlags(pos) |
          (distributetype == TYPEPOWER ?
           (POWEREDBIT | SCRATCHBIT) :
           (WATEREDBIT | SCRATCHBIT))); 


        // find the possible ways we can move on from here
        // se the function for the "strange" returned bitfield
        cross = DistributeNumberOfSquaresAround(pos);

        // if there's "no way out", return
        if ((cross & 0x0f) == 0) { return; }


        if ((cross & 0x0f) == 1) {
            // just a single way out
            if ((cross & 0x10) == 0x10)	     { direction = 0; }
            else if ((cross & 0x20) == 0x20) { direction = 1; }
            else if ((cross & 0x40) == 0x40) { direction = 2; }
            else if ((cross & 0x80) == 0x80) { direction = 3; }
        } else {
            // we are at a crossway
            // initiate some recursive functions from here ;)
            if ((cross & 0x10) == 0x10) { DistributeMoveOnFromThisPoint(pos-GetMapSize()); }
            if ((cross & 0x20) == 0x20) { DistributeMoveOnFromThisPoint(pos+1); }
            if ((cross & 0x40) == 0x40) { DistributeMoveOnFromThisPoint(pos+GetMapSize()); }
            if ((cross & 0x80) == 0x80) { DistributeMoveOnFromThisPoint(pos-1); }
            return;
        }

        // and finally, update our new position
        pos = DistributeMoveOn(pos, direction);

    } while (DistributeFieldCanCarry(pos));
}

// note that this function is used internally in the power distribution
// routine. Therefore it will return false for tiles we've already been at,
// to avoid backtracking the route we came from.
int DistributeFieldCanCarry(unsigned long pos)
{
    if ((GetWorldFlags(pos) & SCRATCHBIT) == SCRATCHBIT) {
        // already been here with this plant
        return 0;
    }
    return distributetype==TYPEPOWER ? CarryPower(GetWorld(pos)) : CarryWater(GetWorld(pos));
}

// gives a status of the situation around us
int DistributeNumberOfSquaresAround(unsigned long pos)
{
    // return:
    // 0001 00xx if up
    // 0010 00xx if right
    // 0100 00xx if down
    // 1000 00xx if left
    // xx = number of directions

    // count fields that carry power
    // do not look behind map border

    char retval=0;
    char number=0;

    if (DistributeFieldCanCarry(DistributeMoveOn(pos, 0))) { retval |= 0x10; number++; }
    if (DistributeFieldCanCarry(DistributeMoveOn(pos, 1))) { retval |= 0x20; number++; }
    if (DistributeFieldCanCarry(DistributeMoveOn(pos, 2))) { retval |= 0x40; number++; }
    if (DistributeFieldCanCarry(DistributeMoveOn(pos, 3))) { retval |= 0x80; number++; }

    retval |= number;

    return retval;
}


/* this function take a position and a direction and
 * moves the position in the direction, but won't move
 * behind map borders 
 */
unsigned long DistributeMoveOn(unsigned long pos, int direction)
{
    switch (direction) {
        case 0: // up
            if (pos < GetMapSize()) { return pos; }
            pos -= GetMapSize();
            break;
        case 1: // right
            if ((pos+1) >= GetMapMul()) { return pos; }
            pos++;
            break;
        case 2: // down
            if ((pos+GetMapSize()) >= GetMapMul()) { return pos; }
            pos += GetMapSize();
            break;
        case 3: //left
            if (pos == 0) { return pos; }
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



////////// Zones upgrade/downgrade //////////

typedef struct {
    long unsigned pos;
    long signed score;
    int used;
} ZoneScore;

ZoneScore zones[256];

void FindZonesForUpgrading()
{
    int i;
    long signed int randomZone;

    int max = GetMapSize()*3;
    if (max > 256) { max = 256; }

    // find some random zones
    for (i=0; i<max; i++)
    {
        zones[i].used = 0;
        randomZone = GetRandomZone();
        if (randomZone != -1) { // -1 means we didn't find a zone
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
    long signed int score;
    counter += 20;
    for (i=counter-20; i<counter; i++)
    {
        if (i>=256) { counter = 0; return 0; } // this was the last zone!

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
    return 1; // there's still more zones that need a score.
}


void UpgradeZones()
{
    int i,j,topscorer;
    long signed topscore;

    int downCount = 11*10+30;
    int upCount = (0-8)*10+250;

    // upgrade the bests
    for (i=0; i<256 && i<upCount; i++)
    {
        topscore = 0;
        topscorer = -1;

        // find the one with max points
        for (j=0; j<256; j++)
        {
            if (zones[j].score > topscore && zones[j].used == 1) {
                topscore = zones[j].score;
                topscorer = j;
            }
        }

        // upgrade him/her/it/whatever
        if (topscorer != -1) {
            if (zones[topscorer].used == 1) {
                zones[topscorer].used = 0;
                UpgradeZone(zones[topscorer].pos);
            }
        }
    }

    // downgrade the worst
    for (i=0; i<256 && i<downCount; i++)
    {
        topscore = -1;
        topscorer = -1;

        // find the one with min points
        for (j=0; j<256; j++)
        {
            if (zones[j].score < topscore && zones[j].used == 1) {
                topscore = zones[j].score;
                topscorer = j;
            }
        }

        // downgrade him/her/it/whatever
        if (topscorer != -1) {
            if (zones[topscorer].used == 1) {
                zones[topscorer].used = 0;
                DowngradeZone(zones[topscorer].pos);
            }
        }
    }
}



void DowngradeZone(long unsigned pos)
{
    int type;
    LockWorld();

    type = GetWorld(pos);
    if (type >= 30 && type <= 39)
    {
        SetWorld(pos, (type == 30) ? 1 : type-1);
        vgame.BuildCount[COUNT_COMMERCIAL]--;
    }
    else if (type >= 40 && type <= 49)
    {
        SetWorld(pos, (type == 40) ? 2 : type-1);
        vgame.BuildCount[COUNT_RESIDENTIAL]--;
    }
    else if (type >= 50 && type <= 59)
    {
        SetWorld(pos, (type == 50) ? 3 : type-1);
        vgame.BuildCount[COUNT_INDUSTRIAL]--;
    }

    UnlockWorld();
}

void UpgradeZone(long unsigned pos)
{
    int type;

    LockWorld();

    type = GetWorld(pos);

    if (type == 1 || (type >= 30 && type <= 38)) {
        SetWorld(pos, (type == 1) ? 30 : type+1);
        vgame.BuildCount[COUNT_COMMERCIAL]++;
    } else if (type == 2 || (type >= 40 && type <= 48)) {
        SetWorld(pos, (type == 2) ? 40 : type+1);
        vgame.BuildCount[COUNT_RESIDENTIAL]++;
    } else if (type == 3 || (type >= 50 && type <= 58)) {
        SetWorld(pos, (type == 3) ? 50 : type+1);
        vgame.BuildCount[COUNT_INDUSTRIAL]++;
    }
    UnlockWorld();
}





int DoTheRoadTrip(long unsigned int startPos)
{
    return 1; // for now
}


signed long GetZoneScore(long unsigned int pos)
{
    // return -1 to make this zone be downgraded _right now_ (ie. if missing things as power or roads)

    signed long score = 0;
    int x = pos % GetMapSize();
    int y = pos / GetMapSize();
    signed int i,j;
    int bRoad = 0;
    int type = 0;

    LockWorld();
    type = GetWorld(pos); //temporary holder
    type = ((IsZone(type, 1) != 0) ? 1 : ((IsZone(type,2) != 0) ? 2 : 3));

    LockWorldFlags();
    if ((GetWorldFlags(pos) & 0x01) == 0) { UnlockWorldFlags(); UnlockWorld(); return -1; } // whoops, no power
    if ((GetWorldFlags(pos) & 0x04) == 0) { UnlockWorldFlags(); UnlockWorld(); return -1; } // or no water :/  
    
    UnlockWorldFlags();

    if (type != ZONE_RESIDENTIAL)  {
        // see if there's actually enough residential population to support
        // a new zone of ind or com

        long signed int availPop = 
                (vgame.BuildCount[COUNT_RESIDENTIAL]*25)
                - (vgame.BuildCount[COUNT_COMMERCIAL]*25 +
                vgame.BuildCount[COUNT_INDUSTRIAL]*25);
        if (availPop <= 0) { UnlockWorld(); return -1; } // whoops, missing population

    } else if (type == ZONE_RESIDENTIAL) {
        // the population can't skyrocket all at once, we need a cap
        // somewhere - note, this should be fine tuned somehow
        // A factor might be the number of (road/train/airplane) connections to
        // the surrounding world - this would bring more potential residents
        // into our little city

        long signed int availPop = 
                ((game.TimeElapsed*game.TimeElapsed)/35+30)
                - (vgame.BuildCount[COUNT_RESIDENTIAL]);
        if (availPop <= 0) { UnlockWorld(); return -1; } // hmm - need more children
    }

    if (type == ZONE_COMMERCIAL) {
        // and what is a store without something to sell? therefore we need
        // enough industrial zones before commercial zones kick in ;)

        long signed int availGoods =
                (vgame.BuildCount[COUNT_INDUSTRIAL]/3*2)
                - (vgame.BuildCount[COUNT_COMMERCIAL]);
        if (availGoods <= 0) { UnlockWorld(); return -1; } // darn, nothing to sell here
    }


    // take a look around at the enviroment ;)
    for (i=x-3; i<4+x; i++) {
        for (j=y-3; j<4+y; j++) {
            if (!(i<0 || i>=GetMapSize() || j<0 || j>=GetMapSize())) {
                
                score += GetScoreFor(type, GetWorld(WORLDPOS(i,j)));
                
                if (IsRoad(GetWorld(WORLDPOS(i,j))) && bRoad == 0) {
                    // can we reach all kinds of zones from here?
                    bRoad = DoTheRoadTrip(WORLDPOS(i,j));
                }
            }
        }
    }

    UnlockWorld();
    if (!bRoad) return -1;
    return score;
}


signed int GetScoreFor(unsigned char iamthis, unsigned char what)
{
    if (IsZone(what,1)) { return iamthis==1?1:(iamthis==2?50:(iamthis==3?50:50)); } // com
    if (IsZone(what,2)) { return iamthis==1?50:(iamthis==2?1:(iamthis==3?50:50)); } // res
    if (IsZone(what,3)) { return iamthis==1?(0-25):(iamthis==2?(0-75):(iamthis==3?1:(0-50))); } // ind
    if (IsRoad(what)  ) { return iamthis==1?75:(iamthis==2?50:(iamthis==3?75:66)); } // roads
    if (what == TYPE_POWER_PLANT) { return iamthis==1?(0-75):(iamthis==2?(0-100):(iamthis==3?30:(0-75))); } // powerplant
    if (what == TYPE_NUCLEAR_PLANT) { return iamthis==1?(0-150):(iamthis==2?(0-200):(iamthis==3?15:(0-175))); } // nuclearplant
    if (what == TYPE_TREE) { return iamthis==1?50:(iamthis==2?85:(iamthis==3?25:50)); } // tree
    if (what == TYPE_WATER||what == TYPE_REAL_WATER) { return iamthis==1?175:(iamthis==2?550:(iamthis==3?95:250)); } // water
    return 0;
}


long unsigned int GetRandomZone()
{
    long unsigned int pos = 0;
    int i;
    unsigned char type;

    LockWorld();
    for (i=0; i<5; i++) // try five times to hit a valid zone
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

extern signed long int BudgetGetNumber(int type)
{
    signed long int ret = 0;
    switch (type) {
        case BUDGET_RESIDENTIAL:
            ret = vgame.BuildCount[COUNT_RESIDENTIAL]
                    * INCOME_RESIDENTIAL
                    * game.tax/100;
            break;
        case BUDGET_COMMERCIAL:
            ret = vgame.BuildCount[COUNT_COMMERCIAL]
                    * INCOME_COMMERCIAL
                    * game.tax/100;
            break;
        case BUDGET_INDUSTRIAL:
            ret = vgame.BuildCount[COUNT_INDUSTRIAL]
                    * INCOME_INDUSTRIAL
                    * game.tax/100;
            break;
        case BUDGET_TRAFFIC:
            ret = (vgame.BuildCount[COUNT_ROADS] * UPKEEP_ROAD
                    * game.upkeep[UPKEEPS_TRAFFIC])/100;
            break;
        case BUDGET_POWER:
            ret = ((vgame.BuildCount[COUNT_POWERLINES]*UPKEEP_POWERLINE +
                    vgame.BuildCount[COUNT_NUCLEARPLANTS]*UPKEEP_NUCLEARPLANT +
                    vgame.BuildCount[COUNT_POWERPLANTS]*UPKEEP_POWERPLANT)
                    * game.upkeep[UPKEEPS_POWER])/100;
            break;
        case BUDGET_DEFENCE:
            ret = ((vgame.BuildCount[COUNT_FIRE_STATIONS]*UPKEEP_FIRE_STATIONS +
                     vgame.BuildCount[COUNT_POLICE_STATIONS]*UPKEEP_POLICE_STATIONS +
                     vgame.BuildCount[COUNT_MILITARY_BASES]*UPKEEP_MILITARY_BASES)
                    * game.upkeep[UPKEEPS_DEFENCE])/100;
            break;
        case BUDGET_CURRENT_BALANCE:
            ret = game.credits;
            break;
        case BUDGET_CHANGE:
            ret = BudgetGetNumber(BUDGET_RESIDENTIAL)
                  + BudgetGetNumber(BUDGET_COMMERCIAL)
                  + BudgetGetNumber(BUDGET_INDUSTRIAL)
                  - BudgetGetNumber(BUDGET_TRAFFIC)
                  - BudgetGetNumber(BUDGET_POWER)
                  - BudgetGetNumber(BUDGET_DEFENCE);
            break;
        case BUDGET_NEXT_MONTH:
            ret = BudgetGetNumber(BUDGET_CURRENT_BALANCE) +
                  BudgetGetNumber(BUDGET_CHANGE);
            break;
    }

    return ret;
}

void DoTaxes()
{
    game.credits += BudgetGetNumber(BUDGET_RESIDENTIAL) +
                    BudgetGetNumber(BUDGET_COMMERCIAL) +
                    BudgetGetNumber(BUDGET_INDUSTRIAL);
}

void DoUpkeep()
{
    long unsigned int upkeep;

    upkeep = BudgetGetNumber(BUDGET_TRAFFIC) +
             BudgetGetNumber(BUDGET_POWER) +
             BudgetGetNumber(BUDGET_DEFENCE);

    if (upkeep <= game.credits) {
        game.credits -= upkeep;
        return;
    }
    UIWriteLog("*** Negative Cashflow\n");
    game.credits = 0;
    
    // roads
    DoNastyStuffTo(TYPE_ROAD,1);
    DoNastyStuffTo(TYPE_POWER_LINE,5);
    DoNastyStuffTo(TYPE_POWER_PLANT,15);
    DoNastyStuffTo(TYPE_NUCLEAR_PLANT,50);
    DoNastyStuffTo(TYPE_FIRE_STATION,10);
    DoNastyStuffTo(TYPE_POLICE_STATION,12);
    DoNastyStuffTo(TYPE_MILITARY_BASE,35);
    
}

extern int Sim_DoPhase(int nPhase)
{
    switch (nPhase)
    {
        case 1:
            if (NeedsUpdate(GRID_POWER)) {
                UIWriteLog("Simulation phase 1 - power grid\n");
                Sim_Distribute(0);
                ClearUpdate(GRID_POWER);
            }
            nPhase =2;
            break;
        case 2:
            if (NeedsUpdate(GRID_WATER)) {
                UIWriteLog("Simulation phase 2 - water grid\n");
                Sim_Distribute(1);
                ClearUpdate(GRID_WATER);
            }
            nPhase = 3;
            break;
        case 3:
            UIWriteLog("Simulation phase 3 - Find zones for upgrading\n");
            FindZonesForUpgrading();
            nPhase=4;
            UIWriteLog("Simulation phase 4 - Find score for zones\n"); // this couldn't be below
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
//            UpdateDisasters();
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
    // Updates the BuildCount array after a load game
    int x;
    int y;

    LockWorld();
    vgame.tileSize = 16;

    for (y = 0; y < game.mapsize; y++)
        for (x = 0; x < game.mapsize; x++) {
            char elt = GetWorld(WORLDPOS(x,y));
            // Gahd this is terrible. I need to fix it.
            if (elt >= ZONE_COMMERCIAL*10+20 &&
              elt <= ZONE_COMMERCIAL*20+29)
                vgame.BuildCount[COUNT_COMMERCIAL] += elt%10 + 1;
            if (elt >= ZONE_RESIDENTIAL*10+20 &&
              elt <= ZONE_RESIDENTIAL*10+29)
                vgame.BuildCount[COUNT_RESIDENTIAL] += elt%10 + 1;
            if (elt >= ZONE_INDUSTRIAL*10+20 &&
              elt <= ZONE_INDUSTRIAL*10+20)
                vgame.BuildCount[COUNT_INDUSTRIAL] += elt%10 + 1;
            if (IsRoad(elt)) vgame.BuildCount[COUNT_ROADS]++;
            if (elt == TYPE_TREE) vgame.BuildCount[COUNT_TREES]++;
            if (elt == TYPE_WATER) vgame.BuildCount[COUNT_WATER]++;
            if (elt == TYPE_POWERROAD_2 || elt == TYPE_POWERROAD_1 ||
              elt == TYPE_POWER_LINE) vgame.BuildCount[COUNT_POWERLINES]++;
            if (elt == TYPE_POWER_PLANT || elt == TYPE_NUCLEAR_PLANT)
                vgame.BuildCount[COUNT_POWERPLANTS]++;
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
