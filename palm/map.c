#define ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS
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
            if (event->screenX >= 1  && event->screenX <= (GetMapSize() + 1) &&
                event->screenY >= 17 &&
                event->screenY <= (GetMapSize() + 17) ) {
                Goto(event->screenX - 1, event->screenY - 17);
                FrmGotoForm(formID_pocketCity);
                handled = 1;
            }
            /* check for other 'penclicks' here */
            break;
        case frmOpenEvent:
            form = FrmGetActiveForm();
            FrmDrawForm(form);
            DrawMap();
            handled = 1;
            break;
        case frmCloseEvent:
            RestoreSpeed();
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

/*
 * The map looks too small if we use Hi-Res calls so we use the standard API.
 */
void DrawMap(void)
{
    int x, y;
    static IndexedColorType entries[5] = { 0, 1, 2, 3, 4 };
    static int inited = 0; /* 0 == not done 1 == */
    IndexedColorType cc = 1;
    const RectangleType rect = { {1, 17}, {100, 100} };
    WinHandle wh;
    WinHandle swh;
    Err e;
    char *addr;
    int shift = 7;
    UInt32 depth;

    wh = WinCreateOffscreenWindow(100, 100, screenFormat, &e);
    if (e != errNone) return;

    LockWorld();
    UILockScreen();
    
    swh = WinSetDrawWindow(wh);
    WinDrawRectangleFrame(1, &rect);

    if (!oldROM) {
        /* get the bits */
        addr = BmpGetBits(WinGetBitmap(wh));
    } else {
        addr = wh->displayAddrV20;
    }
    depth = getDepth();

    /*
     * Entries are: gnd, water, house, comm, fac, oth
     *
     */
    if (inited == 0) {
        inited++;
        if (depth > 4) {
            /* water */
            RGBColorType rg = { 0, 0, 0, 255 };
            entries[0] = WinRGBToIndex(&rg);
            /* house */
            rg.g = 255; rg.b = 0;
            entries[1] = WinRGBToIndex(&rg);
            /* commercial */
            rg.g = 127; rg.b = 127;
            entries[2] = WinRGBToIndex(&rg);
            /* industrial */
            rg.r = 255; rg.g = 255; rg.b = 0;
            entries[3] = WinRGBToIndex(&rg);
            /* other */
            rg.r = 127; rg.g = 127; rg.b = 127;
            entries[4] = WinRGBToIndex(&rg);
            inited++;
        }
    }

    for(y = 0; y < GetMapSize(); y++) {
        for(x = 0 ; x < GetMapSize(); x++) {
            int wt = GetWorld(WORLDPOS(x, y));
            if (inited >= 1) {
                switch (wt) {
                case TYPE_DIRT: cc = 0; break;
                case TYPE_WATER: cc = entries[0]; break;
                case ZONE_RESIDENTIAL: cc = entries[1]; break;
                case ZONE_COMMERCIAL: cc = entries[2]; break;
                case ZONE_INDUSTRIAL: cc = entries[3]; break;
                default: cc = entries[4]; break;
                }
            }
            
            if (depth == 1) {
                if (wt == TYPE_DIRT) { /* it's dirt or other */
                    *addr &= (unsigned char)~(1U << shift);
                } else {
                    *addr |= (unsigned char)~(1U << shift);
                }
                shift--;
                if ((shift < 0) || (x >= (GetMapSize()-1))) {
                    shift = 7;
                    addr++;
                }
            } else if (depth == 4) {
                if (x & 0x1) { /* Low nibble */
                    *addr &= (unsigned char)0xf0;
                    *addr |= cc;
                    addr++;
                } else { /* high nibble */
                    *addr &= (unsigned char)0x0f;
                    *addr |= cc << 4;
                }
            } else {
                *addr++ = cc;
            }
        }
    }
    WinSetDrawWindow(swh);
    _WinCopyRectangle(wh, swh, (RectangleType *)&rect, 2, 18, winPaint);

    UIUnlockScreen();
    WinDeleteWindow(wh, false);
    UnlockWorld();
}

