#define	ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS
#include <PalmOS.h>
#include <Window.h>
#include <Rect.h>
#include <simcity.h>
#include <simcity_resconsts.h>
#include <ui.h>
#include <map.h>
#include <globals.h>
#include <drawing.h>
#include <resCompat.h>
#include <palmutils.h>
#include <sections.h>

static void DrawMap(void) MAP_SECTION;

static const int StartX = 1;
static const int StartY = 17;

/*
 * Handler for the map.
 * takes care of the set-up, pen clicks and popup events.
 */
Boolean
hMap(EventPtr event)
{
	FormPtr form;
	Boolean handled = false;

	switch (event->eType) {
	case penDownEvent:
		if (event->screenX >= StartX &&
		    event->screenX <= (GetMapSize() + StartX) &&
		    event->screenY >= StartY &&
		    event->screenY <= (GetMapSize() + StartY)) {
			Goto(event->screenX - StartX, event->screenY - StartY);
			FrmGotoForm(formID_pocketCity);
			handled = true;
		}
		/* check for other 'penclicks' here */
		break;
	case frmOpenEvent:
		PauseGame();
		WriteLog("map opened\n");
		form = FrmGetActiveForm();
		FrmDrawForm(form);
		DrawMap();
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("map closed\n");
		break;
	case keyDownEvent:
		switch (event->data.keyDown.chr) {
		case vchrFind:
		case vchrLaunch:
			FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		}
		break;
	case menuEvent:
		switch (event->data.menu.itemID) {
		case menuitemID_MapBack:
			FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		}
		break;
	case popSelectEvent:
		if (event->data.popSelect.controlID == listID_shifter_popup) {
			UIGotoForm(event->data.popSelect.selection);
			handled = true;
		}
		break;
	default:
		break;
	}

	return (handled);
}

/*
 * Draw the Map.
 * Does not use the High-Resolution calls because the map would be too small
 * on a high-resolution screen.
 */
static void DrawMap(void)
{
	int x, y;
	static IndexedColorType entries[5] = { 0, 1, 2, 3, 4 };
	static int inited = 0; /* 0 == not done 1 == */
	IndexedColorType cc = 1;
	const RectangleType rect = { {StartX, StartY}, {100, 100} };
	WinHandle wh;
	WinHandle swh;
	Err e;
	char *addr = NULL;
	int shift = 0;
	UInt32 depth;

	wh = WinCreateOffscreenWindow(100, 100, screenFormat, &e);
	if (e != errNone)
		return;

	LockWorld();
	UILockScreen();

	swh = WinSetDrawWindow(wh);
	WinDrawRectangleFrame(1, &rect);

	if (!IsNewROM()) {
		/* Draw On The Bitmap Using direct write */
		addr = wh->displayAddrV20;
	}
	depth = getDepth();

	/*
	 * Entries are: gnd, water, house, comm, fac, oth
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

	if (IsNewROM()) {
		WinPushDrawState();
	}
	for (y = 0; y < GetMapSize(); y++) {
		for (x = 0; x < GetMapSize(); x++) {
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

			if (addr != NULL) {
				switch (depth) {
				case 1:
					if (wt == TYPE_DIRT) {
						*addr &=
						    (unsigned char)
						    ~(1U << shift);
					} else {
						*addr |=
						    (unsigned char)
						    (1U << shift);
					}
					shift++;
					if ((shift > 8)) {
						shift = 0;
						addr++;
					}
					break;
				case 4:
					if (x & 0x1) { /* Low nibble */
						*addr &= (unsigned char)0xf0;
						*addr |= cc;
						addr++;
					} else { /* high nibble */
						*addr &= (unsigned char)0x0f;
						*addr |= cc << 4;
					}
					break;
				default:
					*addr++ = cc;
					break;
				}
			} else {
				WinSetForeColor(cc);
				WinDrawPixel(x, y);
			}
		}
	}
	if (IsNewROM()) {
		WinPopDrawState();
	}
	WinSetDrawWindow(swh);
	WinCopyRectangle(wh, swh, (RectangleType *)&rect, 2, 18, winPaint);

	UIUnlockScreen();
	WinDeleteWindow(wh, false);
	UnlockWorld();
}
