#ifdef PALM
#include <PalmOS.h>
#include "../palm/simcity.h"
#else
#include <sys/types.h>
#include <stddef.h>
#include <assert.h>
#endif
#include "build.h"
#include "globals.h"
#include "ui.h"
#include "drawing.h"

typedef void (*BuildF)(int xpos, int ypos, unsigned int type);

static void Build_Road(int xpos, int ypos, unsigned int type);
static void Build_PowerLine(int xpos, int ypos, unsigned int type);
static void Build_WaterPipe(int xpos, int ypos, unsigned int type);
static void Build_Generic(int xpos, int ypos, unsigned int type);
static void Build_Defence(int xpos, int ypos, unsigned int type);

static void CreateForest(long unsigned int pos, int size);
static void RemoveDefence(int xpos, int ypos);

static int SpendMoney(unsigned long howMuch);

/* this array is dependent on mirroring the BuildCodes enumeration */
static const struct _bldStruct {
    unsigned int bt; /* build type */
    BuildF func;        /* Function to call */
    unsigned int type;
    unsigned int gridsToUpdate;
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

extern void BuildSomething(int xpos, int ypos)
{
    int item = UIGetSelectedBuildItem();
    struct _bldStruct *be = (struct _bldStruct *)&(buildStructure[item]);

#ifdef PALM
    ErrFatalDisplayIf(
      item >= (sizeof (buildStructure) / sizeof (buildStructure[0])),
          "UI item out of range");
#else
    assert((unsigned)item <
	(sizeof (buildStructure) / sizeof (buildStructure[0])));
#endif

    be->func(xpos, ypos, be->type);
    AddGridUpdate(be->gridsToUpdate);
}

void RemoveDefence(int xpos, int ypos)
{
    int i;
    for (i=0; i<NUM_OF_UNITS; i++) {
        if (game.units[i].x == xpos &&
            game.units[i].y == ypos) {
            
            game.units[i].active = 0;
            DrawCross(game.units[i].x, game.units[i].y);
        }
    }
}

extern void RemoveAllDefence(void)
{
    int i;
    for (i=0; i<NUM_OF_UNITS; i++) {
       game.units[i].active = 0;
       DrawCross(game.units[i].x, game.units[i].y);
    }
}

static void
Build_Defence(int xpos, int ypos, unsigned int type)
{
    int oldx;
    int oldy;
    int i;
    int sel=-1;
    int newactive=1;
    int end;
    int start;
    int max;
    int nCounter;

    /* XXX: this is here to make sure not too many of any item are created */
    nCounter = ((type == DuPolice) ? COUNT_POLICE_STATIONS :
      (type == DuFireman ? COUNT_FIRE_STATIONS : COUNT_MILITARY_BASES));

    if (vgame.BuildCount[nCounter] == 0) { return; } /* no special building */

    start = ((type == DuPolice) ? DEF_POLICE_START :
	(type == DuFireman ? DEF_FIREMEN_START : DEF_MILITARY_START));
    end = ((type == DuPolice) ? DEF_POLICE_END :
	(type == DuFireman ? DEF_FIREMEN_END : DEF_MILITARY_END));

    /* make sure we can't make too many objects */
    max = ((unsigned)((end-start)+1) <
	(unsigned)(vgame.BuildCount[nCounter]/3)) ? end :
	(int)(vgame.BuildCount[nCounter]/3 + start);

    /* first remove all defence on this tile */
    for (i=0; i<NUM_OF_UNITS; i++) {
        if (xpos == game.units[i].x &&
            ypos == game.units[i].y &&
            game.units[i].active != 0) {
            /* no need to build something already here */
            if (game.units[i].type == type) { return; }
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
                newactive=2;
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
        newactive=2;
    }

    oldx = game.units[sel].x;
    oldy = game.units[sel].y;

    game.units[sel].x = xpos;
    game.units[sel].y = ypos;
    game.units[sel].active = newactive;
    game.units[sel].type = type;

    DrawCross(oldx, oldy);
    DrawCross(xpos, ypos);
}

extern void
Build_Bulldoze(int xpos, int ypos, unsigned int _type __attribute__((unused)))
{
    int type;
    LockWorld();
    type = GetWorld(WORLDPOS(xpos, ypos));
    if (type != TYPE_DIRT &&
        type != TYPE_FIRE1 &&
        type != TYPE_FIRE2 &&
        type != TYPE_FIRE3 &&
        type != TYPE_REAL_WATER)
    {
        if (SpendMoney(BUILD_COST_BULLDOZER)) {
            Build_Destroy(xpos, ypos);
        } else {
            UIDisplayError(enOutOfMoney);
        }
    }
    RemoveDefence(xpos, ypos); 
    UnlockWorld();
}


extern void
Build_Destroy(int xpos, int ypos)
{
    unsigned char type;

    LockWorld();
    type = GetWorld(WORLDPOS(xpos,ypos));
    RemoveDefence(xpos, ypos);

    vgame.BuildCount[COUNT_COMMERCIAL] -= (type >= (ZONE_COMMERCIAL*10+20) && type <= (ZONE_COMMERCIAL*10+29)) ? (type%10)+1 : 0;
    vgame.BuildCount[COUNT_RESIDENTIAL] -= (type >= (ZONE_RESIDENTIAL*10+20) && type <= (ZONE_RESIDENTIAL*10+29)) ? (type%10)+1 : 0;
    vgame.BuildCount[COUNT_INDUSTRIAL] -= (type >= (ZONE_INDUSTRIAL*10+20) && type <= (ZONE_INDUSTRIAL*10+29)) ? (type%10)+1 : 0;
    vgame.BuildCount[COUNT_ROADS] -= IsRoad(type);
    vgame.BuildCount[COUNT_TREES] -= (type == TYPE_TREE) ? 1 : 0;
    vgame.BuildCount[COUNT_WATER] -= (type == TYPE_WATER) ? 1 : 0;
    vgame.BuildCount[COUNT_WASTE] -= (type == TYPE_WASTE) ? 1 : 0;
    vgame.BuildCount[COUNT_POWERPLANTS] -= (type == TYPE_POWER_PLANT);
    vgame.BuildCount[COUNT_NUCLEARPLANTS] -= (type == TYPE_NUCLEAR_PLANT);
    vgame.BuildCount[COUNT_POWERLINES] -= ((type == TYPE_POWERROAD_2) || (type == TYPE_POWERROAD_1) || (type == TYPE_POWER_LINE)) ? 1 : 0;
    vgame.BuildCount[COUNT_FIRE] -= ((type == TYPE_FIRE1) || (type == TYPE_FIRE2) || (type == TYPE_FIRE3)) ? 1 : 0;
    vgame.BuildCount[COUNT_WATERPIPES] -= ((type == TYPE_WATER_PIPE) || (type == TYPE_WATERROAD_1) || (type == TYPE_WATERROAD_2)) ? 1 : 0;
    vgame.BuildCount[COUNT_FIRE_STATIONS] -= (type == TYPE_FIRE_STATION) ? 1 : 0;
    vgame.BuildCount[COUNT_POLICE_STATIONS] -= (type == TYPE_POLICE_STATION) ? 1 : 0;
    vgame.BuildCount[COUNT_MILITARY_BASES] -= (type == TYPE_MILITARY_BASE) ? 1 : 0;
    vgame.BuildCount[COUNT_WATER_PUMPS] -= (type == TYPE_WATER_PUMP) ? 1 : 0;
    AddGridUpdate(GRID_ALL);
    if (type == TYPE_BRIDGE || type == TYPE_REAL_WATER) {
        /* A bridge turns into real_water when detroyed */
        SetWorld(WORLDPOS(xpos,ypos),TYPE_REAL_WATER);
    } else {
        SetWorld(WORLDPOS(xpos,ypos),TYPE_DIRT);
    }
    UnlockWorld();

    DrawCross(xpos, ypos);
}

static const struct _costMappings {
  unsigned int type;
  unsigned long int cost;
  int count;
} genericMappings[] = {
  { ZONE_RESIDENTIAL, BUILD_COST_ZONE, -1 },
  { ZONE_INDUSTRIAL, BUILD_COST_ZONE, -1 },
  { ZONE_COMMERCIAL, BUILD_COST_ZONE, -1 },
  { TYPE_POWER_PLANT, BUILD_COST_POWER_PLANT, -1 },
  { TYPE_NUCLEAR_PLANT, BUILD_COST_NUCLEAR_PLANT, COUNT_POWERPLANTS },
  { TYPE_WATER, BUILD_COST_WATER, COUNT_WATER },
  { TYPE_TREE, BUILD_COST_TREE, COUNT_TREES },
  { TYPE_FIRE_STATION, BUILD_COST_FIRE_STATION, COUNT_FIRE_STATIONS },
  { TYPE_POLICE_STATION, BUILD_COST_POLICE_STATION, COUNT_POLICE_STATIONS },
  { TYPE_MILITARY_BASE, BUILD_COST_MILITARY_BASE, COUNT_MILITARY_BASES },
  { TYPE_WATER_PUMP, BUILD_COST_WATER_PUMP, COUNT_WATER_PUMPS },
  { 0, 0, -1 }
};

void
Build_Generic(int xpos, int ypos, unsigned int type)
{
    struct _costMappings *cmi = (struct _costMappings *)getIndexOf(
      (char *)&genericMappings[0], sizeof (genericMappings[0]), type);
    LockWorld();
#ifdef PALM
    ErrFatalDisplayIf(cmi == NULL, "No generic->item mapping");
#else
    assert(cmi != NULL);
#endif
    if (cmi == NULL) return;

    if (GetWorld(WORLDPOS(xpos, ypos)) == TYPE_DIRT) {
        if (SpendMoney(cmi->cost)) {
            SetWorld(WORLDPOS(xpos,ypos), (unsigned char)type);
            DrawCross(xpos, ypos);

            /*  update counter */
            if (IsRoad(type)) {
                vgame.BuildCount[COUNT_ROADS]++;
            } else {
                if (cmi->count != -1) vgame.BuildCount[cmi->count]++;
            }
        } else {
            UIDisplayError(enOutOfMoney);
        }
    }
    UnlockWorld();
}


void
Build_Road(int xpos, int ypos, unsigned int type __attribute__((unused)))
{
    int old;
    LockWorld();
    old = GetWorld(WORLDPOS(xpos, ypos));
    if (old == TYPE_POWER_LINE) {
        switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos),1)) { 
            case 70: /* straight power line, we can build here */
                if (SpendMoney(BUILD_COST_ROAD)) {
                    SetWorld(WORLDPOS(xpos, ypos),TYPE_POWERROAD_1);
                    DrawCross(xpos, ypos);
                    vgame.BuildCount[COUNT_ROADS]++;
                } else {
                    UIDisplayError(enOutOfMoney);
                }
                break;
            case 71: /* ditto */
                if (SpendMoney(BUILD_COST_ROAD)) {
                    SetWorld(WORLDPOS(xpos, ypos),TYPE_POWERROAD_2);
                    DrawCross(xpos, ypos);
                    vgame.BuildCount[COUNT_ROADS]++;
                } else {
                    UIDisplayError(enOutOfMoney);
                }
                break;
        }
    } else if (old == TYPE_WATER_PIPE) {
        switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos),3)) { 
            case 92: /* straight water pipe, we can build here */
                if (SpendMoney(BUILD_COST_ROAD)) {
                    SetWorld(WORLDPOS(xpos, ypos),TYPE_WATERROAD_1);
                    DrawCross(xpos, ypos);
                    vgame.BuildCount[COUNT_ROADS]++;
                } else {
                    UIDisplayError(enOutOfMoney);
                }
                break;
            case 93: /* ditto */
                if (SpendMoney(BUILD_COST_ROAD)) {
                    SetWorld(WORLDPOS(xpos, ypos),TYPE_WATERROAD_2);
                    DrawCross(xpos, ypos);
                    vgame.BuildCount[COUNT_ROADS]++;
                } else {
                    UIDisplayError(enOutOfMoney);
                }
                break;
        }
    } else if (old == TYPE_REAL_WATER) {
        /* build a bridge across the water (yup, that's a song) */
        if (SpendMoney(BUILD_COST_BRIDGE)) {
            SetWorld(WORLDPOS(xpos, ypos), TYPE_BRIDGE);
            DrawCross(xpos, ypos);
            vgame.BuildCount[COUNT_ROADS]++;
        } else {
            UIDisplayError(enOutOfMoney);
        }
    } else if (old == TYPE_DIRT) {
        if (SpendMoney(BUILD_COST_ROAD)) {
            SetWorld(WORLDPOS(xpos, ypos),TYPE_ROAD);
            DrawCross(xpos, ypos);
            vgame.BuildCount[COUNT_ROADS]++;
        } else {
            UIDisplayError(enOutOfMoney);
        }
    }
    UnlockWorld();
}

static void
Build_PowerLine(int xpos, int ypos, unsigned int type __attribute__((unused)))
{
    int old;
    LockWorld();

    old = GetWorld(WORLDPOS(xpos, ypos));
    if (old == TYPE_DIRT || old == TYPE_ROAD) {
        if (old == TYPE_ROAD) {
            switch(GetSpecialGraphicNumber(WORLDPOS(xpos, ypos),0)) {
                case 10: /* straight road, we can build a power line */
                    if (SpendMoney(BUILD_COST_POWER_LINE)) {
                        SetWorld(WORLDPOS(xpos, ypos),TYPE_POWERROAD_2);
                        DrawCross(xpos, ypos);
                        vgame.BuildCount[COUNT_POWERLINES]++;
                    } else {
                        UIDisplayError(enOutOfMoney);
                    }
                    break;
                case 11: /* ditto */
                    if (SpendMoney(BUILD_COST_POWER_LINE)) {
                        SetWorld(WORLDPOS(xpos, ypos),TYPE_POWERROAD_1);
                        DrawCross(xpos, ypos);
                        vgame.BuildCount[COUNT_POWERLINES]++;
                    } else {
                        UIDisplayError(enOutOfMoney);
                    }
                    break;
            }
        } else {
            if (SpendMoney(BUILD_COST_POWER_LINE)) {
                SetWorld(WORLDPOS(xpos, ypos),TYPE_POWER_LINE);
                DrawCross(xpos, ypos);
                vgame.BuildCount[COUNT_POWERLINES]++;
            } else {
                UIDisplayError(enOutOfMoney);
            }
        }
    }
    UnlockWorld();
}

static void
Build_WaterPipe(int xpos, int ypos, unsigned int type __attribute__((unused)))
{
    int old;
    LockWorld();

    old = GetWorld(WORLDPOS(xpos, ypos));
    if (old == TYPE_DIRT || old == TYPE_ROAD) {
        if (old == TYPE_ROAD) {
            switch(GetSpecialGraphicNumber(WORLDPOS(xpos, ypos),0)) {
                case 10: /* straight road, we can build a power line */
                    if (SpendMoney(BUILD_COST_WATER_PIPES)) {
                        SetWorld(WORLDPOS(xpos, ypos),TYPE_WATERROAD_2);
                        DrawCross(xpos, ypos);
                        vgame.BuildCount[COUNT_WATERPIPES]++;
                    } else {
                        UIDisplayError(enOutOfMoney);
                    }
                    break;
                case 11: /* ditto */
                    if (SpendMoney(BUILD_COST_WATER_PIPES)) {
                        SetWorld(WORLDPOS(xpos, ypos),TYPE_WATERROAD_1);
                        DrawCross(xpos, ypos);
                        vgame.BuildCount[COUNT_WATERPIPES]++;
                    } else {
                        UIDisplayError(enOutOfMoney);
                    }
                    break;
            }
        } else {
            if (SpendMoney(BUILD_COST_POWER_LINE)) {
                SetWorld(WORLDPOS(xpos, ypos),TYPE_WATER_PIPE);
                DrawCross(xpos, ypos);
                vgame.BuildCount[COUNT_WATERPIPES]++;
            } else {
                UIDisplayError(enOutOfMoney);
            }
        }
    }
    UnlockWorld();
}

static int
SpendMoney(unsigned long howMuch)
{
    if (howMuch > (unsigned long)game.credits) { return 0; }

    game.credits -= howMuch;

    /* now redraw the credits */
    UIInitDrawing();
    UIDrawCredits();
    UIFinishDrawing();
    return 1;
}

/* this creates a river through the playfield
 * TODO: make this more interesting.
 */
extern void
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
                    SetWorld(WORLDPOS(i, k), TYPE_REAL_WATER);
                else
                    SetWorld(WORLDPOS(k, i), TYPE_REAL_WATER);
            }
        }

        switch (GetRandomNumber(3)) {
            case 0: if (width >  5) { width--; } break;
            case 1: if (width < 15) { width++; } break;
            default: break;
        }
        switch (GetRandomNumber(4)) {
            case 0: if (j > 0)            { j--; } break;
            case 1: if (j < GetMapSize()) { j++; } break;
            default: break;
        }
    }
    UnlockWorld();
}

/*
 * creates some "spraypainted" (someone called them that)
 * forests throughout the `wilderness`
 */
extern void
CreateForests(void)
{
    int i,j,k;
    unsigned long int pos;
    j = GetRandomNumber(6)+7;
    for (i=0; i<j; i++) {
        k = GetRandomNumber(6)+8;
        pos = GetRandomNumber(GetMapMul());
        CreateForest(pos, k);
    }

}

/* create a single forest - look above */
static void
CreateForest(long unsigned int pos, int size)
{
    int x,y,i,j,s;
    x = pos % GetMapSize();
    y = pos / GetMapSize();
    LockWorld();
    i = x;
    j = y;

    for (i=x-size; i<=x+size; i++) {
        for (j=y-size; j<=y+size; j++) {
            if (i >= 0 && i < GetMapSize() && j >= 0 && j < GetMapSize()) {
                if (GetWorld(WORLDPOS(i,j)) == TYPE_DIRT) {
                    s = ((y>j) ? (y-j) : (j-y)) +
                        ((x>i) ? (x-i) : (i-x));
                    if (GetRandomNumber(s) < 2) {
                        SetWorld(WORLDPOS(i,j), TYPE_TREE);
                        vgame.BuildCount[COUNT_TREES]++;
                    }
                }
            }
        }
    }

    UnlockWorld();
}
