#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"
#include "build.h"
#include "simulation.h"

// for the palm
#ifdef PALM
#include <PalmOS.h>
#include <unix_string.h>
#include <unix_stdlib.h>
#else
#include <string.h>
#include <stdlib.h>
#endif

extern void
PCityMain(void)
{
    // called on entry
    InitWorld();
    SetMapSize(100);
    ResizeWorld(GetMapMul());
    SetUpGraphic();
    game.gameLoopSeconds = SPEED_PAUSED;
}

extern void
SetupNewGame(void)
{
    game.TimeElapsed = 0;
    game.map_xpos = 50;
    game.map_ypos = 50;
    game.credits = 50000;
    //memset((void*)&game.BuildCount[0],0,80);
    memset((void*)game.objects, 0, sizeof(MoveableObject)*NUM_OF_OBJECTS);
    memset((void*)game.units, 0, sizeof(DefenceUnit)*NUM_OF_UNITS);
    (void)memcpy(game.version, SAVEGAMEVERSION, 4);
    game.upkeep[0] = 100;
    game.upkeep[1] = 100;
    game.upkeep[2] = 100;
    //game.tileSize = 16;
    SetMapSize(100);
    game.tax = 8; // TODO: changeable tax rate
    game.disaster_level = 1; // TODO: = difficulty_level
    ResizeWorld(GetMapMul());
    game.gameLoopSeconds = SPEED_PAUSED;
    CreateFullRiver();
    CreateForests();
    DrawGame(1);
}

extern void
DrawGame(int full)
{
    UIInitDrawing();

    UIDrawBorder();
    RedrawAllFields();
    UIUpdateBuildIcon();
    //DrawHeader();
    full = full;

    UIFinishDrawing();
}

extern void
PostLoadGame(void)
{
    UpdateVolatiles();
}

