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
                map_xpos = event->screenX-1-(visible_x/2);
                map_ypos = event->screenY-17-(visible_y/2);
                if (map_ypos < 0) { map_ypos = 0; }
                if (map_ypos > mapsize-visible_y) { map_ypos = mapsize - visible_y; }
                if (map_xpos < 0) { map_xpos = 0; }
                if (map_xpos > mapsize-visible_x) { map_xpos = mapsize - visible_x; }
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

