#include <PalmOS.h>
#include "simcity.h"
#include "../source/ui.h"
#include "../source/globals.h"


void DrawMap(void);


extern Boolean hMap(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType)
    {
        case penDownEvent:
            if (event->screenX >= 1  && event->screenX <= mapsize+1 &&
                event->screenY >= 17 && event->screenY <= mapsize+17) {
                Goto(event->screenX-1, event->screenY-17);
                FrmGotoForm(formID_pocketCity);
                handled = 1;
            }
            // check for other 'penclicks' here
            break;
        case frmOpenEvent:
            form = FrmGetActiveForm();
            FrmDrawForm(form);
            DrawMap();
            handled = 1;
            break;
        case frmCloseEvent:
            break;
        case keyDownEvent:
            switch (event->data.keyDown.chr)
            {
                case vchrFind:
                case vchrLaunch:
                    FrmGotoForm(formID_pocketCity);
                    handled = 1;
                    break;
            }
            break;
        case menuEvent:
            switch (event->data.menu.itemID)
            {
                case menuitemID_MapBack:
                    FrmGotoForm(formID_pocketCity);
                    handled = 1;
                    break;
            }
            break;
        case popSelectEvent:
            if (event->data.popSelect.controlID == listID_shifter_popup) {
                UIGotoForm(event->data.popSelect.selection);
                handled = 1;
            }
            break;
        default:
            break;
    }

    return handled;
}

void DrawMap(void)
{
    int i,j;

    LockWorld();
    UILockScreen();
    _UIDrawRect(17,1,100,100);

    if (!oldROM) {
        for(i=0; i<mapsize; i++) {
            for(j=0; j<mapsize; j++) {
                if (GetWorld(WORLDPOS(i,j)) != TYPE_DIRT) {
                    WinDrawPixel(i+1,j+17);
                }
            }
        }
    }

    UIUnlockScreen();
    UnlockWorld();
}

