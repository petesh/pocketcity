
#include "build.h"
#include "globals.h"
#include "ui.h"
#include "zakdef.h"
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
		break;
	case BUILD_ZONE_RESIDENTIAL:
		Build_Generic(xpos, ypos, BUILD_COST_ZONE, ZONE_RESIDENTIAL);
		break;
	case BUILD_ZONE_COMMERCIAL:
		Build_Generic(xpos, ypos, BUILD_COST_ZONE, ZONE_COMMERCIAL);
		break;
	case BUILD_ZONE_INDUSTRIAL:
		Build_Generic(xpos, ypos, BUILD_COST_ZONE, ZONE_INDUSTRIAL);
		break;
	case BUILD_ROAD:
		Build_Road(xpos, ypos);
		break;
	case BUILD_POWER_LINE:
		Build_PowerLine(xpos, ypos);
		break;
	case BUILD_POWER_PLANT:
		Build_Generic(xpos, ypos, BUILD_COST_POWER_PLANT, 60);
		break;
	case BUILD_NUCLEAR_PLANT:
		Build_Generic(xpos, ypos, BUILD_COST_NUCLEAR_PLANT, 61);
		break;
	case BUILD_WATER:
		Build_Generic(xpos, ypos, BUILD_COST_WATER, 22);
		break;
	case BUILD_TREE:
		Build_Generic(xpos, ypos, BUILD_COST_TREE, 21);
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
	
	SetWorld(WORLDPOS(xpos,ypos),0);
	UnlockWorld();

	DrawCross(xpos, ypos);
}

void Build_Generic(int xpos, int ypos, long unsigned int nCost, unsigned char nType)
{
	LockWorld();
	if (GetWorld(WORLDPOS(xpos, ypos)) == 0)
	{
		if (SpendMoney(nCost))
		{
			SetWorld(WORLDPOS(xpos,ypos),nType);
			DrawCross(xpos, ypos);

			//  update counter
			BuildCount[COUNT_ROADS] += IsRoad(nType);
			BuildCount[COUNT_TREES] += (nType == 21);
			BuildCount[COUNT_WATER] += (nType == 22);
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
                    }
                    break;
                case 71:
                    if (SpendMoney(BUILD_COST_ROAD))
                    {
                        SetWorld(WORLDPOS(xpos, ypos),7);
                        DrawCross(xpos, ypos);
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
                    }
                    break;
                case 11:
                    if (SpendMoney(BUILD_COST_POWER_LINE)) 
                    {
                        SetWorld(WORLDPOS(xpos, ypos),6);
                        DrawCross(xpos, ypos);
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
            }
        }
	}
	UnlockWorld();
}

int SpendMoney(unsigned long howMuch)
{
	if (howMuch > credits)
	{
		return 0;
	}

	credits -= howMuch;

	// now redraw the credits
	UIInitDrawing();
	UIDrawCredits();
	UIFinishDrawing();
	return 1;
}
