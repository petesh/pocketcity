#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"
#include "build.h"


void FireSpread(int x, int y);
int  BurnField(int x, int y);
void CreateWaste(int x, int y);


extern void DoNastyStuffTo(int type, unsigned int probability)
{
    long unsigned int randomTile;
    int i,x,y;

    if (GetRandomNumber(probability) != 0) { return; } // nothing happened :(

    LockWorld();
    for (i=0; i<50; i++) {
        randomTile = GetRandomNumber(mapsize*mapsize);
        if (GetWorld(randomTile) == type) {
            // wee, let's destroy something
            x = randomTile % mapsize;
            y = randomTile / mapsize;
            CreateWaste(x,y);
            UnlockWorld();
            return;
        }
    }
    UnlockWorld();
    return;
}



extern void DoRandomDisaster(void)
{
    long unsigned int randomTile;
    int i,x,y;

    LockWorld();

    for (i=0; i<100; i++) {
        randomTile = GetRandomNumber(mapsize*mapsize);
        if (GetWorld(randomTile) != TYPE_DIRT) {
            x = randomTile % mapsize;
            y = randomTile / mapsize;
            switch(GetRandomNumber(50))
            {
                case 1:
                    if (BurnField(x,y)) {
                        UIDisplayError(ERROR_FIRE_OUTBREAK);
                    }
                    break;
                default:
                    break;
            }
            UnlockWorld();
            return; // only one disaster per turn
        }
    }
    UnlockWorld();
}


extern void UpdateDisasters(void)
{
    int i,j,type;

    LockWorld();
    for (i=0; i<mapsize; i++) {
        for (j=0; j<mapsize; j++) {
            type = GetWorld(WORLDPOS(i,j));
            if (type == TYPE_FIRE2) {
                if (GetRandomNumber(5) != 0) {
                    FireSpread(i,j);
                    SetWorld(WORLDPOS(i,j),TYPE_FIRE3);
                } else {
                    CreateWaste(i,j);
                }
            } else if (type == TYPE_FIRE1) {
                SetWorld(WORLDPOS(i,j),TYPE_FIRE2);
            } else if (type == TYPE_FIRE3) {
                CreateWaste(i,j);
            }
        }
    }
    UnlockWorld();
}

void CreateWaste(int x, int y)
{
    int type;
    LockWorld();
    type = GetWorld(WORLDPOS(x,y));
    Build_Destroy(x,y);
    SetWorld(WORLDPOS(x,y), TYPE_WASTE);
    DrawCross(x,y);
    BuildCount[COUNT_WASTE]++;
    UnlockWorld();
    if (type == TYPE_POWER_PLANT || type == TYPE_NUCLEAR_PLANT)  {
        UIDisplayError(ERROR_PLANT_EXPLOSION);
        FireSpread(x,y);
    }
}

void FireSpread(int x, int y) 
{
    if (x > 0) { BurnField(x-1,y); }
    if (x < mapsize-1) { BurnField(x+1,y); }
    if (y > 0) { BurnField(x,y-1); }
    if (y < mapsize-1) { BurnField(x,y+1); }
}


int BurnField(int x, int y)
{
    int type;
    
    LockWorld();
    type = GetWorld(WORLDPOS(x,y));
    if (type != TYPE_FIRE1 &&
        type != TYPE_FIRE2 &&
        type != TYPE_FIRE3 &&
        type != TYPE_DIRT  &&
        type != TYPE_WASTE) {
        Build_Destroy(x,y);
        SetWorld(WORLDPOS(x,y), TYPE_FIRE1);
        DrawCross(x,y);
        BuildCount[COUNT_FIRE]++;
        UnlockWorld();
        return 1;
    }
    UnlockWorld();
    return 0;
}
