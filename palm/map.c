#include <PalmOS.h>
#include <Window.h>
#include <Rect.h>
#include "simcity.h"
#include "simcity_resconsts.h"
#include "../source/ui.h"
#include "../source/globals.h"
#include "../source/drawing.h"
#include "resCompat.h"

void DrawMap(void);
Boolean hMap(EventPtr event);

extern Boolean hMap(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType)
    {
        case penDownEvent:
            if (event->screenX >= 1  && event->screenX <= (game.mapsize + 1) &&
                event->screenY >= 17 &&
                event->screenY <= (game.mapsize + 17) ) {
                Goto(event->screenX - 1, event->screenY - 17);
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

// The map looks too small if we use Hi-Res calls so we use the standard
// API.

void DrawMap(void)
{
    int i, j;
    static IndexedColorType entries[5];
    static int inited = 0; // 0 == not done 1 == 
    IndexedColorType cc;
    RectangleType rect = { {1, 17}, {100, 100} };

    LockWorld();
    UILockScreen();
    WinDrawRectangleFrame(1, &rect);
    /*
     * Entries are: gnd, water, house, comm, fac, oth
     *
     */
    if (inited == 0) {
        inited++;
        if (getDepth() > 1) {
            // water
            RGBColorType rg = { 0, 0, 0, 255 };
            entries[0] = WinRGBToIndex(&rg);
            // house
            rg.g = 255; rg.b = 0;
            entries[1] = WinRGBToIndex(&rg);
            // commercial
            rg.g = 127; rg.b = 127;
            entries[2] = WinRGBToIndex(&rg);
            // industrial
            rg.r = 255; rg.g = 255; rg.b = 0;
            entries[3] = WinRGBToIndex(&rg);
            // other
            rg.r = 127; rg.g = 127; rg.b = 127;
            entries[4] = WinRGBToIndex(&rg);
            inited++;
        }
    }

    if (!oldROM) {
        for(i = 0; i < game.mapsize; i++) {
            for(j =0 ; j < game.mapsize; j++) {
                int wt = GetWorld(WORLDPOS(i,j));
                if (wt == TYPE_DIRT)
                    continue;
                if (inited > 1) {
                    switch (wt) {
                    case TYPE_WATER: cc = entries[0]; break;
                    case ZONE_RESIDENTIAL: cc = entries[1]; break;
                    case ZONE_COMMERCIAL: cc = entries[2]; break;
                    case ZONE_INDUSTRIAL: cc = entries[3]; break;
                    default: cc = entries[4]; break;
                    }
                    WinPushDrawState();
                    WinSetForeColor(cc);
                }
                WinDrawPixel(i*1, j+17);
                if (inited > 1)
                    WinPopDrawState();
            }
        }
    }

    UIUnlockScreen();
    UnlockWorld();
}

