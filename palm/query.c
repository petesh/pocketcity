
/* Handler for the query form */
#include <PalmTypes.h>
#include <Chars.h>
#include <StringMgr.h>
#include <query.h>
#include <simcity.h>
#include <ui.h>
#include <simcity_resconsts.h>

static void zonetoPtr(UInt8 type, Char *zonemesg) MAP_SECTION;
static FormPtr querySetup(void) MAP_SECTION;
static void queryCleanup(void) MAP_SECTION;

/*
 * Handler for the query form
 * takes care of the set-up, cleanup app-button selection and cleanup
 */
Boolean
hQuery(EventPtr event)
{
    Boolean handled = 0;

    switch (event->eType) {
    case frmOpenEvent:
        PauseGame();
        WriteLog("query opened\n");
        FrmDrawForm(querySetup());
        handled = 1;
        break;
    case frmCloseEvent:
        WriteLog("Query close\n");
        queryCleanup();
        break;
    case keyDownEvent:
        switch (event->data.keyDown.chr) {
        case vchrLaunch:
            FrmGotoForm(formID_pocketCity);
            handled = 1;
            break;
        }
    case ctlSelectEvent:
        switch (event->data.ctlEnter.controlID) {
        case buttonID_OK:
            FrmGotoForm(formID_pocketCity);
            handled = 1;
            break;
        }
    default:
        break;
    }

    return (handled);
}

/*
 * Convert a zone type into a display string.
 */
static void
zonetoPtr(UInt8 type, Char *zonemesg)
{
    StrPrintF(zonemesg, "wakka!");
}

/*
 * Set up the display items for the query form.
 */
static FormPtr
querySetup(void)
{
    UInt8 type;
    Char *temp;
    FormPtr form;
    ControlType *ctl;

    type = GetItemClicked();
    form = FrmGetActiveForm();
    temp = MemPtrNew(255);
    zonetoPtr(type, temp);
    ctl = FrmGetObjectPtr(form, FrmGetObjectIndex(form, labelID_zonetype));
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

    temp = (char *)CtlGetLabel(FrmGetObjectPtr(form,
          FrmGetObjectIndex(form, labelID_zonetype)));
    if (temp) MemPtrFree(temp);
}

