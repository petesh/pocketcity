/*! \file
 * \brief routines that are used for event dispatching
 *
 * These routines are intended to multiplex the individual steps in the
 * game.
 */
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

/*!
 * \brief Application level initialization routines
 */
void
PCityMain(void)
{
	InitWorld();
	SetMapSize(100,100);
	ResizeWorld(WorldSize());
	SetUpGraphic();
	setLoopSeconds(SPEED_PAUSED);
}

/*!
 * \brief set up a game
 *
 * Setting up a new game ... initialize the variables
 * the difficulty level should be set before calling this.
 */
void
SetupNewGame(void)
{
	memset((void *)&vgame, 0, sizeof (vgame));
	AddGridUpdate(GRID_ALL);
	setTimeElapsed(0);
	setMapXPos(50);
	setMapYPos(50);
	switch (GetDifficultyLevel()) {
	case 0:
		setCredits(50000);
		break;
	case 1:
		setCredits(30000);
		break;
	case 2:
		setCredits(15000);
		break;
	default:
		setCredits(10000);
		break;
	}
	memset((void *)game.objects, 0,
	    sizeof (MoveableObject) * NUM_OF_OBJECTS);
	memset((void *)game.units, 0, sizeof (DefenceUnit) * NUM_OF_UNITS);
	setGameVersion(SAVEGAMEVERSION);
	setUpkeep(0, 100);
	setUpkeep(1, 100);
	setUpkeep(2, 100);
	SetMapSize(100, 100);
	setTax(8);
	LockWorld();
	ResizeWorld(WorldSize());
	UnlockWorld();
	setLoopSeconds(SPEED_PAUSED);
	setAutoBulldoze(1);
	CreateFullRiver();
	CreateForests();
	DrawGame(1);
}

/*!
 * \brief Draw the entire game arena
 * \param full unused.
 */
void
DrawGame(Int8 full __attribute__((unused)))
{
	UIInitDrawing();

	UIDrawBorder();
	RedrawAllFields();

	UIFinishDrawing();
}

/*!
 * \brief Things to do after loading the game.
 */
void
PostLoadGame(void)
{
	memset((void *)&vgame.BuildCount, 0, sizeof (vgame.BuildCount));
	SetMapSize(GetMapWidth(), GetMapHeight());
	UpdateVolatiles();
}
