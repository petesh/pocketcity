#include <PalmOS.h>
#include "../palm/simcity.h"
#include "build.h"
#include "globals.h"
#include "ui.h"
#include "drawing.h"

void Build_Destroy(int xpos, int ypos);
void Build_Road(int xpos, int ypos);
void Build_PowerLine(int xpos, int ypos);
void Build_Generic(int xpos, int ypos, long unsigned int nCost, unsigned char nType);

int SpendMoney(unsigned long howMuch);

extern void BuildSomething(int xpos, int ypos)
{
    switch (UIGetSelectedBuildItem())
    {
        case BUILD_BULLDOZER:
            Build_Bulldoze(xpos, ypos);
            updatePowerGrid = 1;
            break;
        case BUILD_ZONE_RESIDENTIAL:
            Build_Generic(xpos, ypos, BUILD_COST_ZONE, ZONE_RESIDENTIAL);
            updatePowerGrid = 1;
            break;
        case BUILD_ZONE_COMMERCIAL:
            Build_Generic(xpos, ypos, BUILD_COST_ZONE, ZONE_COMMERCIAL);
            updatePowerGrid = 1;
            break;
        case BUILD_ZONE_INDUSTRIAL:
            Build_Generic(xpos, ypos, BUILD_COST_ZONE, ZONE_INDUSTRIAL);
            updatePowerGrid = 1;
            break;
        case BUILD_ROAD:
            Build_Road(xpos, ypos);
            break;
        case BUILD_POWER_LINE:
            Build_PowerLine(xpos, ypos);
            updatePowerGrid = 1;
            break;
        case BUILD_POWER_PLANT:
            Build_Generic(xpos, ypos, BUILD_COST_POWER_PLANT, TYPE_POWER_PLANT);
            updatePowerGrid = 1;
            break;
        case BUILD_NUCLEAR_PLANT:
            Build_Generic(xpos, ypos, BUILD_COST_NUCLEAR_PLANT, TYPE_NUCLEAR_PLANT);
            updatePowerGrid = 1;
            break;
        case BUILD_WATER:
            Build_Generic(xpos, ypos, BUILD_COST_WATER, TYPE_WATER);
            break;
        case BUILD_TREE:
            Build_Generic(xpos, ypos, BUILD_COST_TREE, TYPE_TREE);
            break;
    }
}


extern void Build_Bulldoze(int xpos, int ypos)
{
    LockWorld();
    if (GetWorld(WORLDPOS(xpos, ypos)) != 0)
    {
        if (SpendMoney(BUILD_COST_BULLDOZER))
        {
            Build_Destroy(xpos, ypos);
        } else {
            UIDisplayError(ERROR_OUT_OF_MONEY);
        }

    }
    UnlockWorld();
}


void Build_Destroy(int xpos, int ypos)
{
    unsigned char type;

    LockWorld();
    type = GetWorld(WORLDPOS(xpos,ypos));

    BuildCount[COUNT_COMMERCIAL] -= (type >= (ZONE_COMMERCIAL*10+20) && type <= (ZONE_COMMERCIAL*10+29)) ? (type%10)+1 : 0;
    BuildCount[COUNT_RESIDENTIAL] -= (type >= (ZONE_RESIDENTIAL*10+20) && type <= (ZONE_RESIDENTIAL*10+29)) ? (type%10)+1 : 0;
    BuildCount[COUNT_INDUSTRIAL] -= (type >= (ZONE_INDUSTRIAL*10+20) && type <= (ZONE_INDUSTRIAL*10+29)) ? (type%10)+1 : 0;
    BuildCount[COUNT_ROADS] -= IsRoad(type);
    BuildCount[COUNT_TREES] -= (type == 21);
    BuildCount[COUNT_WATER] -= (type == 22);
    BuildCount[COUNT_POWERPLANTS] -= (type == 60);
    BuildCount[COUNT_NUCLEARPLANTS] -= (type == 61);
    BuildCount[COUNT_POWERLINES] -= ((type == 7) || (type == 6) || (type == 5));

    SetWorld(WORLDPOS(xpos,ypos),0);
    UnlockWorld();

    DrawCross(xpos, ypos);
}

void Build_Generic(int xpos, int ypos, long unsigned int nCost, unsigned char nType)
{
    LockWorld();
    if (GetWorld(WORLDPOS(xpos, ypos)) == 0) {

        if (SpendMoney(nCost)) {
            SetWorld(WORLDPOS(xpos,ypos),nType);
            DrawCross(xpos, ypos);

            //  update counter
            BuildCount[COUNT_ROADS] += IsRoad(nType);
            BuildCount[COUNT_TREES] += (nType == TYPE_TREE);
            BuildCount[COUNT_WATER] += (nType == TYPE_WATER);
            BuildCount[COUNT_POWERPLANTS] += (nType == TYPE_POWER_PLANT);
            BuildCount[COUNT_NUCLEARPLANTS] += (nType == TYPE_NUCLEAR_PLANT);
        } else {
            UIDisplayError(ERROR_OUT_OF_MONEY);
        }

    }
    UnlockWorld();
}


void Build_Road(int xpos, int ypos)
{
    int old;
    LockWorld();
    old = GetWorld(WORLDPOS(xpos, ypos));
    if (old == 0 || old == 5)
    {
        if (old == 5)
        {
            switch (GetSpecialGraphicNumber(WORLDPOS(xpos, ypos),1))
            {
                case 70:
                    if (SpendMoney(BUILD_COST_ROAD))
                    {
                        SetWorld(WORLDPOS(xpos, ypos),6);
                        DrawCross(xpos, ypos);
                        BuildCount[COUNT_ROADS]++;
                    } else {
                        UIDisplayError(ERROR_OUT_OF_MONEY);
                    }
                    break;
                case 71:
                    if (SpendMoney(BUILD_COST_ROAD))
                    {
                        SetWorld(WORLDPOS(xpos, ypos),7);
                        DrawCross(xpos, ypos);
                        BuildCount[COUNT_ROADS]++;
                    } else {
                        UIDisplayError(ERROR_OUT_OF_MONEY);
                    }

                    break;
                default:
                    break;
            }
        }
        else
        {
            if (SpendMoney(BUILD_COST_ROAD))
            {
                SetWorld(WORLDPOS(xpos, ypos), 4);
                DrawCross(xpos, ypos);
                BuildCount[COUNT_ROADS]++;
            } else {
                UIDisplayError(ERROR_OUT_OF_MONEY);
            }

        }
    }
    UnlockWorld();
}

void Build_PowerLine(int xpos, int ypos)
{
    int old;
    LockWorld();

    old = GetWorld(WORLDPOS(xpos, ypos));
    if (old == 0 || old == 4)
    {
        if (old == 4)
        {
            switch(GetSpecialGraphicNumber(WORLDPOS(xpos, ypos),0))
            {
                case 10:
                    if (SpendMoney(BUILD_COST_POWER_LINE)) 
                    {
                        SetWorld(WORLDPOS(xpos, ypos),7);
                        DrawCross(xpos, ypos);
                        BuildCount[COUNT_POWERLINES]++;
                    } else {
                        UIDisplayError(ERROR_OUT_OF_MONEY);
                    }

                    break;
                case 11:
                    if (SpendMoney(BUILD_COST_POWER_LINE)) 
                    {
                        SetWorld(WORLDPOS(xpos, ypos),6);
                        DrawCross(xpos, ypos);
                        BuildCount[COUNT_POWERLINES]++;
                    } else {
                        UIDisplayError(ERROR_OUT_OF_MONEY);
                    }

                    break;
            }
        }
        else
        {
            if (SpendMoney(BUILD_COST_POWER_LINE)) 
            {
                SetWorld(WORLDPOS(xpos, ypos),5);
                DrawCross(xpos, ypos);
                BuildCount[COUNT_POWERLINES]++;
            } else {
                UIDisplayError(ERROR_OUT_OF_MONEY);
            }

        }
    }
    UnlockWorld();
}

int SpendMoney(unsigned long howMuch)
{
    if (howMuch > credits) {
        return 0;
    }

    credits -= howMuch;

    // now redraw the credits
    UIInitDrawing();
    UIDrawCredits();
    UIFinishDrawing();
    return 1;
}