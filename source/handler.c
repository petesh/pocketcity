/*! \file
 * \brief routines that are used for event dispatching
 *
 * These routines are intended to multiplex the individual steps in the
 * game.
 */

#include <handler.h>
#include <config.h>
#include <drawing.h>
#include <locking.h>
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

void
PCityMain(void)
{
	InitWorld();
	InitGraphic();
	setLoopSeconds(SPEED_PAUSED);
}

void
setMapSize(UInt8 X, UInt8 Y)
{
	setMapVariables(X, Y);

	LockZone(lz_world);
	ResizeWorld(WorldSize());
	UnlockZone(lz_world);
	UIMapResize();
}

void
PCityShutdown(void)
{
	endSimulation();
}

/*!
 * \brief initialize the game structure.
 * 
 * This is just prior to, say, making a new savegame.
 * It makes sure that all the variables are set to the correct values
 */
void
InitGameStruct(void)
{
	memset((void *)&vgame, 0, sizeof (vgame));
	memset((void *)&game, 0, sizeof (game));
	setGameVersion(SAVEGAMEVERSION);
	AddGridUpdate(GRID_ALL);
	//setTimeElapsed(0);
	setMapXPos(50);
	setMapYPos(50);
	//setDifficultyLevel(0);
	setDisasterLevel(1);
	setUpkeep(0, 100);
	setUpkeep(1, 100);
	setUpkeep(2, 100);
	setMapSize((Int16)100, (Int16)100);
	setTax(8);
	//setLoopSeconds(SPEED_PAUSED);
	SETAUTOBULLDOZE(1);
}

/*!
 * \brief set up a game
 *
 * Setting up a new game ... initialize the variables.
 * the difficulty level should be set before calling this.
 */
void
ConfigureNewGame(void)
{
	switch (getDifficultyLevel()) {
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
	RedrawAllFields();
}

/*!
 * \brief Things to do after loading the game.
 */
void
PostLoadGame(void)
{
	memset((void *)&vgame.BuildCount, 0, sizeof (vgame.BuildCount));
	setMapVariables(getMapWidth(), getMapHeight());
	UpdateVolatiles();
	AddGridUpdate(GRID_ALL);
	UIMapResize();
	UIPostLoadGame();
}
