/*!
 * \file
 * \brief Map rendering code.
 *
 * Performs the rendering of the individual maps on the screen.
 * The only functional map is the general area.
 */

#define	ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS
#include <PalmOS.h>
#include <Window.h>
#include <Rect.h>
#include <Progress.h>
#include <simcity.h>
#include <simcity_resconsts.h>
#include <ui.h>
#include <map.h>
#include <globals.h>
#include <drawing.h>
#include <resCompat.h>
#include <palmutils.h>
#include <sections.h>

/*! \brief map type s */
typedef enum e_map_type {
	mt_fullpaint = 1, /*! <brief full painting item */
	mt_overlay /*!< overlay map */
} map_type;

/*! \brief map entries */
typedef enum e_map_entries {
	me_basemap = 0, /*!< basic map */
	me_end /*!< end entry */
} map_entry;

/*! \brief Map structure for the various maps to be displayed */
typedef struct scr_map {
	WinHandle	handle;	/*!< handle for map painting */
	map_type	type;	/*!< overlay | normal map */
} scr_map_t;

static void DrawMap(void) MAP_SECTION;
static void RenderMaps(void) MAP_SECTION;
static void freemaps(void) MAP_SECTION;
static void AddMap(WinHandle handle, map_type type, map_entry code) MAP_SECTION;

/*! \brief current map */
map_entry currmap;
/*! \brief the map structures ... it's a cheap ass array. 0 .. me_end-1 */
scr_map_t *themaps;

static const int StartX = 1;
static const int StartY = 17;

/*!
 * \brief add a map to the list of maps
 * \param handle the handle to the map
 * \param type type of map
 * \param code the code of the map
 */
static void
AddMap(WinHandle handle, map_type type, map_entry code)
{
	if (themaps == NULL) {
		themaps = (scr_map_t *)MemPtrNew((int)me_end *
		    sizeof(scr_map_t));
	}
	themaps[code].handle = handle;
	themaps[code].type = type;
}

/*! \brief release the map structures */
static void
freemaps(void)
{
	map_entry entry = me_basemap;

	if (themaps == NULL) return;
	while (entry != me_end) {
		WinDeleteWindow(themaps[entry].handle, false);
		entry++;
	}
	MemPtrFree(themaps);
	themaps = NULL;
}

/*!
 * Deals witht he pen clicks and setup/teardown for the map
 */
Boolean
hMap(EventPtr event)
{
	FormPtr form;
	Boolean handled = false;

	switch (event->eType) {
	case penDownEvent:
		if (event->screenX >= StartX &&
		    event->screenX <= (getMapWidth() + StartX) &&
		    event->screenY >= StartY &&
		    event->screenY <= (getMapHeight() + StartY)) {
			Int16 x = event->screenX - StartX;
			Int16 y = event->screenY - StartY;
			Goto(x, y, true);
			FrmGotoForm(formID_pocketCity);
			handled = true;
		}
		/* check for other 'penclicks' here */
		break;
	case frmOpenEvent:
		PauseGame();
		WriteLog("map opened\n");
		form = FrmGetActiveForm();
		SetSilkResizable(form, true);
		collapseMove(form, CM_DEFAULT, NULL, NULL);
		FrmDrawForm(form);
		RenderMaps();
		DrawMap();
		handled = true;
		break;
	case frmCloseEvent:
		freemaps();
		SetSilkResizable(NULL, false);
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
#if defined(HRSUPPORT)
	case winDisplayChangedEvent:
#if defined(SONY_CLIE)
	case vchrSilkResize:
#endif
		form = FrmGetActiveForm();
		if (collapseMove(form, CM_DEFAULT, NULL, NULL)) {
			FrmDrawForm(form);
			DrawMap();
		}
		handled = true;
		break;
#endif
	default:
		break;
	}

	return (handled);
}

/*!
 * \brief draw the active map
 */
static void
DrawMap(void)
{
	const RectangleType rect = {
		{ StartX, StartY }, { getMapWidth(), getMapHeight() }
	};
	const RectangleType grect = {
		{ 0, 0 }, { getMapWidth(), getMapHeight() }
	};

	map_entry entry = currmap;
	WinHandle swh = WinGetDrawWindow();

	WinDrawRectangleFrame(1, &rect);

	while(themaps[entry].type != mt_fullpaint) {
		entry--;
	}
	WinCopyRectangle(themaps[entry].handle, swh,
	    (RectangleType *)&grect, StartX, StartY, winPaint);
	if (entry != currmap) {
		/* Erase the bit's we're going to paint on */
		WinCopyRectangle(themaps[currmap].handle, swh,
		    (RectangleType *)&grect, StartX, StartY, winErase);
		WinCopyRectangle(themaps[currmap].handle, swh,
		    (RectangleType *)&grect, StartX, StartY, winInvert);
	}
}

/*!
 * \brief Render the Maps onto the offscreen.
 */
static void
RenderMaps(void)
{
	PointType posits = { 0, 0 };
	static IndexedColorType entries[5] = { 0, 1, 2, 3, 4 };
	static int inited = 0; /* 0 == not done 1 == */
	IndexedColorType cc = 1;
	WinHandle wh;
	WinHandle swh;
	Err e;
	char *addr = NULL;
	int shift = 0;
	UInt32 depth;
	char mapRenderString[80];
	char perc[5];

	currmap = me_basemap;

	ResGetString(si_maprender, mapRenderString, 79);

	WinPaintChars(mapRenderString, StrLen(mapRenderString), 20, 20);
	StrPrintF(perc, "%d%%", 0);
	WinPaintChars(perc, StrLen(perc), 20, 40);

	wh = WinCreateOffscreenWindow(getMapWidth(), getMapHeight(),
	    screenFormat, &e);
	if (e != errNone) {
		return;
	}

	LockZone(lz_world);

	swh = WinSetDrawWindow(wh);
	WinSetDrawWindow(swh);

	/* We are on the 'standard' window */

	if (!IsNewROM()) {
		/* Draw On The Bitmap Using direct write */
		addr = (char *)wh->displayAddrV20;
	}
	depth = getDepth();

	/*
	 * Entries are: gnd, water, house, comm, fac, oth
	 */
	if (inited == 0) {
		inited++;
		if (depth >= 4) {
			/* water */
			RGBColorType rg = { 0, 0, 0, 255 };
			entries[0] = WinRGBToIndex(&rg);
			/* house - green */
			rg.g = 255; rg.b = 0;
			entries[1] = WinRGBToIndex(&rg);
			/* commercial ~ 50% g, ~ 50% blue */
			rg.g = 102; rg.b = 102;
			entries[2] = WinRGBToIndex(&rg);
			/* industrial */
			rg.r = 242; rg.g = 103; rg.b = 57;
			entries[3] = WinRGBToIndex(&rg);
			/* other */
			//rg.r = 127; rg.g = 127; rg.b = 127;
			rg.r = 99; rg.g = 43; rg.b = 170;
			entries[4] = WinRGBToIndex(&rg);
			inited++;
		}
	}

	for (posits.y = 0; posits.y < getMapHeight(); posits.y++) {
		if (IsNewROM() && (posits.y & 0xF) == 0xF) {
			int prc = (int)(( (long)posits.y * 100 ) /
			    getMapHeight());
			StrPrintF(perc, "%d%%", prc);
			WinPaintChars(perc, StrLen(perc), 20, 40);
		}
		for (posits.x = 0; posits.x < getMapWidth(); posits.x++) {
			int wt = getWorld(WORLDPOS(posits.x, posits.y));
			if (inited >= 1) {
				switch (wt) {
				case Z_DIRT:
					cc = 0;
					break;
				case Z_FAKEWATER:
				case Z_REALWATER:
					cc = entries[0];
					break;
				case Z_RESIDENTIAL_SLUM:
					cc = entries[1];
					break;
				case Z_COMMERCIAL_SLUM:
					cc = entries[2];
					break;
				case Z_INDUSTRIAL_SLUM:
					cc = entries[3];
					break;
				default:
					cc = entries[4];
					break;
				}
			}

			if (addr != NULL) {
				switch (depth) {
				case 1:
					if (wt == Z_DIRT) {
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
					if (posits.x & 0x1) { /* Low nibble */
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
				IndexedColorType color;
				swh = WinSetDrawWindow(wh);
				color = WinSetForeColor(cc);
				WinDrawPixel(posits.x, posits.y);
				WinSetForeColor(color);
				WinSetDrawWindow(swh);
			}
		}
	}
	AddMap(wh, mt_fullpaint, me_basemap);
	UnlockZone(lz_world);
}

void
UIUpdateMap(UInt16 xpos __attribute__((unused)),
    UInt16 ypos __attribute__((unused)))
{
}

void
UIMapResize(void)
{
	/* Unused on this platform */
}

void
UIDrawMapStatus(UInt16 xpos __attribute__((unused)),
    UInt16 ypos __attribute__((unused)),
    welem_t world __attribute__((unused)),
    selem_t status __attribute__((unused)))
{
	/* Unused on this platform */
}

void
UIDrawMapField(UInt16 xpos __attribute__((unused)),
    UInt16 ypos __attribute__((unused)),
    welem_t world __attribute__((unused)))
{
	/* Unused on this platform */
}


