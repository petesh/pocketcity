#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"
#include "build.h"


extern void PCityMain(void)
{
    // called on entry
    InitWorld();
    ResizeWorld(mapsize*mapsize);
    SetUpGraphic();
    SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
    CreateFullRiver();
    CreateForests();
}

extern void OnPCityEvent(int nEvent)
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
    UIUpdateBuildIcon();
    //DrawHeader();
    full = full;

    UIFinishDrawing();
}
