#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"
#include "disaster.h"

int powerleft=0;


void PowerMoveOnFromThisPoint(unsigned long pos);
void DoTaxes(void);
void DoUpkeep(void);
unsigned long PowerMoveOn(unsigned long pos, int direction);
int PowerNumberOfSquaresAround(unsigned long pos);
int PowerFieldCanCarry(unsigned long pos);
void UpgradeZones(void);
void UpgradeZone(long unsigned pos);
void DowngradeZone(long unsigned pos);
int DoTheRoadTrip(long unsigned int startPos);
signed long GetZoneScore(long unsigned int pos);
signed int GetScoreFor(unsigned char iamthis, unsigned char what);
long unsigned int GetRandomZone(void);
void FindZonesForUpgrading(void);
int FindScoreForZones(void);

extern void Sim_DistributePower(void)
{
    unsigned long i,j;

    // reset powergrid
    LockWorldFlags();
    for (j=0; j<mapsize*mapsize; j++) { SetWorldFlags(j, GetWorldFlags(j) & 0xfc); }


    // Find all the powerplants and move out from there
    LockWorld(); // this lock locks for ALL power subs
    for (i=0; i<mapsize*mapsize; i++)
    {
        if (GetWorld(i) == 60 || GetWorld(i) == 61)
        {
            if ((GetWorldFlags(i) & 0x01) == 0) // have we already processed this powerplant?
            {

                powerleft=0;
                PowerMoveOnFromThisPoint(i);
                for (j=0; j<mapsize*mapsize; j++) { SetWorldFlags(j, GetWorldFlags(j) & 0xfd); }
            }

        }

    }
    UnlockWorld();
    UnlockWorldFlags();

}

void PowerMoveOnFromThisPoint(unsigned long pos)
{
    // a recursive function
    char cross = 0;
    char direction = 0;


    do
    {

        if ((GetWorldFlags(pos) & 0x01) == 0)
        {
            powerleft--;
            if (GetWorld(pos) == 60) { powerleft += 100; }
            if (GetWorld(pos) == 61) { powerleft +=300; }
        }

        SetWorldFlags(pos, GetWorldFlags(pos) | 0x03);

        if (powerleft < 1) { powerleft = 0; return; }

        cross = PowerNumberOfSquaresAround(pos);

        if ((cross & 0x0f) == 0)
        {
            return;
        }


        if ((cross & 0x0f) == 1)
        {
            // just a single way out
            if ((cross & 0x10) == 0x10)	     { direction = 0; }
            else if ((cross & 0x20) == 0x20) { direction = 1; }
            else if ((cross & 0x40) == 0x40) { direction = 2; }
            else if ((cross & 0x80) == 0x80) { direction = 3; }
        }

        if ((cross & 0x0f) != 1)
        {
            // we are at a power section
            if ((cross & 0x10) == 0x10) { PowerMoveOnFromThisPoint(pos-mapsize); }
            if ((cross & 0x20) == 0x20) { PowerMoveOnFromThisPoint(pos+1); }
            if ((cross & 0x40) == 0x40) { PowerMoveOnFromThisPoint(pos+mapsize); }
            if ((cross & 0x80) == 0x80) { PowerMoveOnFromThisPoint(pos-1); }
            return;
        }

        pos = PowerMoveOn(pos, direction);



    } while (PowerFieldCanCarry(pos));

}

int PowerFieldCanCarry(unsigned long pos)
{
    if ((GetWorldFlags(pos) & 0x02) == 0x02) { return 0; } // allready been here with this plant
    return CarryPower(GetWorld(pos));
}

int PowerNumberOfSquaresAround(unsigned long pos)
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


    if (PowerFieldCanCarry(PowerMoveOn(pos, 0))) { retval |= 0x10; number++; }
    if (PowerFieldCanCarry(PowerMoveOn(pos, 1))) { retval |= 0x20; number++; }
    if (PowerFieldCanCarry(PowerMoveOn(pos, 2))) { retval |= 0x40; number++; }
    if (PowerFieldCanCarry(PowerMoveOn(pos, 3))) { retval |= 0x80; number++; }

    retval |= number;

    return retval;
}



unsigned long PowerMoveOn(unsigned long pos, int direction)
{

    switch (direction)
    {
        case 0: // up
            if (pos < mapsize) { return pos; }
            pos -= mapsize;
            break;

        case 1: // right
            if ((pos+1) >= mapsize*mapsize) { return pos; }
            pos++;
            break;

        case 2: // down
            if ((pos+mapsize) >= mapsize*mapsize) { return pos; }
            pos += mapsize;
            break;

        case 3: //left
            if (pos == 0) { return pos; }
            pos--;
            break;
    }

    return pos;

}

////////// Zones upgrade/downgrade //////////

typedef struct
{
    long unsigned pos;
    long signed score;
    int used;
} ZoneScore;

ZoneScore zones[256];

void FindZonesForUpgrading()
{
    int i;
    long signed int randomZone;

    int max = mapsize*3;
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
        BuildCount[COUNT_COMMERCIAL]--;
    }
    else if (type >= 40 && type <= 49)
    {
        SetWorld(pos, (type == 40) ? 2 : type-1);
        BuildCount[COUNT_RESIDENTIAL]--;
    }
    else if (type >= 50 && type <= 59)
    {
        SetWorld(pos, (type == 50) ? 3 : type-1);
        BuildCount[COUNT_INDUSTRIAL]--;
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
        BuildCount[COUNT_COMMERCIAL]++;
    } else if (type == 2 || (type >= 40 && type <= 48)) {
        SetWorld(pos, (type == 2) ? 40 : type+1);
        BuildCount[COUNT_RESIDENTIAL]++;
    } else if (type == 3 || (type >= 50 && type <= 58)) {
        SetWorld(pos, (type == 3) ? 50 : type+1);
        BuildCount[COUNT_INDUSTRIAL]++;
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
    int x = pos%mapsize;
    int y = pos/mapsize;
    signed int i,j;
    int bRoad = 0;
    int type = 0;

    LockWorld();
    type = GetWorld(pos); //temporary holder
    type = ((IsZone(type, 1) != 0) ? 1 : ((IsZone(type,2) != 0) ? 2 : 3));

    LockWorldFlags();
    if ((GetWorldFlags(pos) & 0x01) == 0) { UnlockWorldFlags(); UnlockWorld(); return -1; } // whoops, no power
    UnlockWorldFlags();

    if (type != ZONE_RESIDENTIAL)  {
        // see if there's actually enough residential population to support
        // a new zone of ind or com

        long signed int availPop = 
                (BuildCount[COUNT_RESIDENTIAL]*25)
                - (BuildCount[COUNT_COMMERCIAL]*25 + BuildCount[COUNT_INDUSTRIAL]*25);
        if (availPop <= 0) { UnlockWorld(); return -1; } // whoops, missing population

    } else if (type == ZONE_RESIDENTIAL) {
        // the population can't skyrocket all at once, we need a cap
        // somewhere - note, this should be fine tuned somehow
        // A factor might be the number of (road/train/airplane) connections to
        // the surrounding world - this would bring more potential residents
        // into our little city

        long signed int availPop = 
                ((TimeElapsed*TimeElapsed)/35+30)
                - (BuildCount[COUNT_RESIDENTIAL]);
        if (availPop <= 0) { UnlockWorld(); return -1; } // hmm - need more children
    }

    if (type == ZONE_COMMERCIAL) {
        // and what is a store without something to sell? therefore we need
        // enough industrial zones before commercial zones kick in ;)

        long signed int availGoods =
                (BuildCount[COUNT_INDUSTRIAL]/3*2)
                - (BuildCount[COUNT_COMMERCIAL]);
        if (availGoods <= 0) { UnlockWorld(); return -1; } // darn, nothing to sell here
    }


    // take a look around at the enviroment ;)
    for (i=x-3; i<4+x; i++)
    {
        for (j=y-3; j<4+y; j++)
        {
            if (!(i<0 || i>=mapsize || j<0 || j>=mapsize)) {
                
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
        pos = GetRandomNumber(mapsize*mapsize);
        type = GetWorld(pos);
        if ((type >= 1 && type <= 3) || (type >= 30 && type <= 59)) {
            UnlockWorld();
            return pos;
        }
    }

    UnlockWorld();
    return -1;
}

void DoTaxes()
{
    credits += (
            BuildCount[COUNT_RESIDENTIAL]*INCOME_RESIDENTIAL +
            BuildCount[COUNT_COMMERCIAL]*INCOME_COMMERCIAL   +
            BuildCount[COUNT_INDUSTRIAL]*INCOME_INDUSTRIAL
            )*tax/100;
}

void DoUpkeep()
{

    // roads
    if (credits < BuildCount[COUNT_ROADS]*UPKEEP_ROAD) {
        // can't pay
        credits = 0;
        DoNastyStuffTo(TYPE_ROAD,1);
    } else {
        credits -= BuildCount[COUNT_ROADS]*UPKEEP_ROAD;
    }

    // power lines
    if (credits < BuildCount[COUNT_POWERLINES]*UPKEEP_POWERLINE) {
        // can't pay
        credits = 0;
        DoNastyStuffTo(TYPE_POWER_LINE,5);
    } else {
        credits -= BuildCount[COUNT_POWERLINES]*UPKEEP_POWERLINE;
    }

    // power plants
    if (credits < (BuildCount[COUNT_NUCLEARPLANTS]*UPKEEP_NUCLEARPLANT + BuildCount[COUNT_POWERPLANTS]*UPKEEP_POWERPLANT)) {
        // can't pay
        credits = 0;
        DoNastyStuffTo(TYPE_POWER_PLANT,15);
        DoNastyStuffTo(TYPE_NUCLEAR_PLANT,30);
    } else {
        credits -= (BuildCount[COUNT_NUCLEARPLANTS]*UPKEEP_NUCLEARPLANT + BuildCount[COUNT_POWERPLANTS]*UPKEEP_POWERPLANT);
    }
    
}

extern int Sim_DoPhase(int nPhase)
{
    switch (nPhase)
    {
        case 1:
            if (updatePowerGrid != 0) {
                Sim_DistributePower();
                updatePowerGrid = 0;
            }
            nPhase =2;
            break;
        case 2:
            FindZonesForUpgrading();
            nPhase=3;
            break;
        case 3:
            if (FindScoreForZones() == 0)
                  nPhase=4;
            break;
        case 4:
            UpgradeZones();
            nPhase=5;
            break;
        case 5:
            UpdateDisasters();
            DoRandomDisaster();
            nPhase = 6;
            break;
        case 6:
            DoTaxes();
            TimeElapsed++;
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
