#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"

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
            Build_Destroy(x,y);
            SetWorld(randomTile, TYPE_WASTE);
            UnlockWorld();
            return;
        }
    }
    UnlockWorld();
    return;
}
