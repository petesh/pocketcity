
/* Handler for the query form */
#include <PalmTypes.h>
#include <Chars.h>
#include <StringMgr.h>
#include <query.h>
#include <simcity.h>
#include <ui.h>
#include <simcity_resconsts.h>
#include <zakdef.h>

static void zonetoPtr(Char *zonemesg, UInt8 tile) MAP_SECTION;
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

/*
 * Convert a zone type into a display string.
 * Probably should have an array of all zone entries -> zonetype
 */
static void
zonetoPtr(Char *zonemsg, UInt8 tile)
{
	switch (tile) {
	case TYPE_DIRT:
		StrCopy(zonemsg, "Empty land");
		break;
	case TYPE_POWER_LINE:
		StrCopy(zonemsg, "Power line");
		break;
	case TYPE_ROAD:
		StrCopy(zonemsg, "Road");
		break;
	case TYPE_REAL_WATER:
		StrCopy(zonemsg, "Water");
		break;
	case TYPE_TREE:
		StrCopy(zonemsg, "Forest");
		break;
	default:
		StrIToA(zonemsg, tile);
		break;
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
	ControlType *ctl;

	form = FrmGetActiveForm();
	temp = MemPtrNew(255);
	zonetoPtr(temp, GetItemClicked());
	ctl = GetObjectPtr(form, labelID_zonetype);
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

	temp = (char *)CtlGetLabel(GetObjectPtr(form, labelID_zonetype));
	if (temp) MemPtrFree(temp);
}
