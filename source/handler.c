#include <handler.h>
#include <drawing.h>
#include <zakdef.h>
#include <ui.h>
#include <globals.h>
#include <handler.h>
#include <build.h>
#include <simulation.h>

/* for the palm */
#ifdef PALM
#include <PalmOS.h>
#include <unix_string.h>
#include <unix_stdlib.h>
#else
#include <string.h>
#include <stdlib.h>
#endif

/*
 * Application level initialization routines
 */
void
PCityMain(void)
{
    InitWorld();
    SetMapSize(100);
    ResizeWorld(GetMapMul());
    SetUpGraphic();
    game.gameLoopSeconds = SPEED_PAUSED;
}

/*
 * Setting up a new game ... initialize the variables
 * the difficulty level should be set before calling this.
 */
void
SetupNewGame(void)
{
    memset((void *)&vgame, 0, sizeof(vgame));
    vgame.gridsToUpdate = GRID_ALL;
    game.TimeElapsed = 0;
    game.map_xpos = 50;
    game.map_ypos = 50;
    switch (GetDifficultyLevel()) {
    case 0:
        game.credits = 50000;
        break;
    case 1:
        game.credits = 30000;
        break;
    case 2:
        game.credits = 15000;
        break;
    default:
        game.credits = 10000;
        break;
    }
    memset((void *)game.objects, 0, sizeof (MoveableObject) * NUM_OF_OBJECTS);
    memset((void *)game.units, 0, sizeof (DefenceUnit) * NUM_OF_UNITS);
    (void)memcpy(game.version, SAVEGAMEVERSION, 4);
    game.upkeep[0] = 100;
    game.upkeep[1] = 100;
    game.upkeep[2] = 100;
    SetMapSize(100);
    game.tax = 8; /* TODO: changeable tax rate */
    SetDisasterLevel(GetDifficultyLevel()+1)
    ResizeWorld(GetMapMul());
    game.gameLoopSeconds = SPEED_PAUSED;
    game.auto_bulldoze = 1;
    CreateFullRiver();
    CreateForests();
    DrawGame(1);
}

/*
 * Draw the entire game arena
 */
void
DrawGame(int full)
{
    UIInitDrawing();

    UIDrawBorder();
    RedrawAllFields();
    UIUpdateBuildIcon();
    /* DrawHeader(); */
    full = full;

    UIFinishDrawing();
}

/*
 * Things to do after loading the game.
 */
void
PostLoadGame(void)
{
    memset((void *)&vgame.BuildCount, 0, sizeof(vgame.BuildCount));
    /* XXX: free lunch alert */
    vgame.gridsToUpdate = GRID_ALL;
    SetMapSize(GetMapSize());
    UpdateVolatiles();
    Sim_Distribute();
}
