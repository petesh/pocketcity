
/* Handler for the query form */
#include <PalmTypes.h>
#include <Chars.h>
#include <StringMgr.h>
#include <query.h>
#include <simcity.h>
#include <ui.h>
#include <simcity_resconsts.h>
#include <zakdef.h>

static void zonetoPtr(Char *zonemesg, UInt8 tile, UInt16 length) MAP_SECTION;
static FormPtr querySetup(void) MAP_SECTION;
static void queryCleanup(void) MAP_SECTION;

/*
 * Handler for the query form
 * takes care of the set-up, cleanup app-button selection and cleanup
 */
Boolean
hQuery(EventPtr event)
{
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		PauseGame();
		WriteLog("query opened\n");
		FrmDrawForm(querySetup());
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("Query close\n");
		queryCleanup();
		break;
	case keyDownEvent:
		switch (event->data.keyDown.chr) {
		case vchrLaunch:
			FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		}
	case ctlSelectEvent:
		switch (event->data.ctlEnter.controlID) {
		case buttonID_OK:
			FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		}
	default:
		break;
	}

	return (handled);
}

static const struct type_zone {
	UInt8	tile;
	UInt16	zonestring;
} type_zones[] = {
	{ TYPE_DIRT, si_empty_land },
	{ TYPE_POWER_LINE, si_power_line },
	{ TYPE_ROAD, si_road },
	{ TYPE_REAL_WATER, si_real_water },
	{ TYPE_TREE, si_forest },
	{ 0, 0 }
};

/*
 * Convert a zone type into a display string.
 * Probably should have an array of all zone entries -> zonetype
 */
static void
zonetoPtr(Char *zonemsg, UInt8 tile, UInt16 maxlen)
{
	struct type_zone *tzone = (struct type_zone *)&type_zones[0];

	while (tzone->zonestring != 0) {
		if (tzone->tile == tile)
			break;
		tzone++;
	}

	if (tzone->zonestring == 0) {
		StrIToA(zonemsg, tile);
	} else {
		ResGetString(tzone->zonestring, zonemsg, maxlen);
	}
}

/*
 * Set up the display items for the query form.
 */
static FormPtr
querySetup(void)
{
	Char *temp;
	FormPtr form;
	ControlPtr ctl;

	form = FrmGetActiveForm();
	temp = (Char *)MemPtrNew(255);
	zonetoPtr(temp, GetItemClicked(), 255);
	ctl = (ControlPtr)GetObjectPtr(form, labelID_zonetype);
	CtlSetLabel(ctl, temp);
	return (form);
}

/*
 * Tidy up the memory allocated to the form.
 */
static void
queryCleanup(void)
{
	FormPtr form;
	Char *temp;

	form = FrmGetActiveForm();

	temp = (char *)CtlGetLabel((ControlPtr)GetObjectPtr(form,
	    labelID_zonetype));
	if (temp) MemPtrFree(temp);
}
