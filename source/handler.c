
#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"


extern void ZakMain(void)
{
// called on entry
	InitWorld();
	ResizeWorld(mapsize*mapsize);
	SetUpGraphic();
    SIM_GAME_LOOP_SECONDS = 10;
}


extern void ZakStop(void)
{
	//StopSimulation();
	//UninitGame();
}

extern void OnZakEvent(int nEvent)
{
	switch (nEvent) {
	case 0://MENU_TEST_BUTTON_PRESSED:
		UIDisplayError(0);
		break;
	case 1://MENU_SECOND_TEST_BUTTON_PRESSED:
		break;
	default:
		break;
	}
}



extern void DrawGame(int full)
{
	UIInitDrawing();

	UIDrawBorder();
	RedrawAllFields();
	//DrawHeader();
	full = full;
	
	UIFinishDrawing();
}


