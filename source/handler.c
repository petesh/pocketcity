/*!
 * \file
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
#include <initial_paint.h>

/* for the palm */
#ifdef PALM
#include <PalmOS.h>
#include <unix_string.h>
#include <unix_stdlib.h>
#else
#include <string.h>
#include <stdlib.h>
#endif

int
PCityStartup(void)
{
	InitGameStruct();
	InitWorld();
	setLoopSeconds(SPEED_PAUSED);
	return (InitializeGraphics());
}

void
setMapSize(UInt8 X, UInt8 Y)
{
	setMapVariables(X, Y);

	ResizeWorld(WorldSize());
	UIMapResize();
}

static void CleanupGameStruct(void);

void
PCityShutdown(void)
{
	endSimulation();
	CleanupGraphics();
	CleanupGameStruct();
	PurgeWorld();
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
	/* vgame.mapmul =; set by setMapSize */
	vgame.prior_credit = 0;
	memset((void *)&vgame.BuildCount, 0, sizeof (vgame.BuildCount));
	vgame.oldLoopSeconds = 0;
	vgame.gameInProgress = 0;
	vgame.playing = 0;
	if (vgame.powers != NULL)
		ListDoEmpty(vgame.powers);
	else
		vgame.powers = ListNew();
	if (vgame.waters != NULL)
		ListDoEmpty(vgame.waters);
	else
		vgame.waters = ListNew();
	memset((void *)&game, 0, sizeof (game));
	setGameVersion(SAVEGAMEVERSION);
	AddGridUpdate(GRID_ALL);
	// setTimeElapsed(0);
	setMapXPos(50);
	setMapYPos(50);
	// setDifficultyLevel(0);
	setDisasterLevel(1);
	setUpkeep(0, 100);
	setUpkeep(1, 100);
	setUpkeep(2, 100);
	setMapSize((Int16)100, (Int16)100);
	setTax(8);
	// setLoopSeconds(SPEED_PAUSED);
	SETAUTOBULLDOZE(1);
}

/*!
 * \brief cleanup the game structure
 * Makes sure that any data as part of the game structure has been released
 */
static void
CleanupGameStruct(void)
{
	if (vgame.powers != NULL) {
		ListDelete(vgame.powers);
		vgame.powers = NULL;
	}
	if (vgame.waters != NULL) {
		ListDelete(vgame.waters);
		vgame.waters = NULL;
	}
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
	PaintTheWorld();
	setGameInProgress(1);
	ResumeGame();
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
	MapHasJumped();
	setGameInProgress(1);
	DrawGame(1);
	ResumeGame();
}
