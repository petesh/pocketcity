#include <PalmOS.h>
#include <simcity_resconsts.h>
#include <sections.h>
#include <options.h>
#include <globals.h>
#include <simcity.h>
#include <ui.h>
#include <palmutils.h>

static FormPtr setupOptions(void) MAP_SECTION;
static void saveOptions(void) MAP_SECTION;

/*
 * Handler for the main options dialog.
 * This is the event loop for the form, making all the appropriate calls to
 * set-up and save the options providing the OK button is hit.
 */
Boolean
hOptions(EventPtr event)
{
	Boolean handled = false;
	static char okHit = 0;

	switch (event->eType) {
	case frmOpenEvent:
		PauseGame();
		WriteLog("options open\n");
		FrmDrawForm(setupOptions());
		okHit = 0;
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("options closed\n");
		if (okHit)
			saveOptions();
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
			okHit = 1;
			handled = true;
			FrmGotoForm(formID_pocketCity);
			break;
		case buttonID_Cancel:
			okHit = 0;
			handled = true;
			FrmGotoForm(formID_pocketCity);
			break;
		}
	default:
		break;
	}

	return (handled);
}

/*
 * Set the state of the various fields in the options form.
 */
static FormPtr
setupOptions(void)
{
	FormPtr form = FrmGetActiveForm();
	CtlSetValue(GetObjectPtr(form, buttonID_dis_off+GetDisasterLevel()), 1);
	CtlSetValue(GetObjectPtr(form,
	    buttonID_Easy + GetDifficultyLevel()), 1);
	CtlSetValue(GetObjectPtr(form, checkboxID_autobulldoze),
	    game.auto_bulldoze);
	return (form);
}

/*
 * save the options from the option dialog to the application state.
 * This does not persist the configuration out, that is the responsiblity
 * of the savegame routines.
 */
static void
saveOptions(void)
{
	FormPtr form = FrmGetActiveForm();
	if (CtlGetValue(GetObjectPtr(form, buttonID_dis_off))) {
		SetDisasterLevel(0);
	} else if (CtlGetValue(GetObjectPtr(form, buttonID_dis_one))) {
		SetDisasterLevel(1);
	} else if (CtlGetValue(GetObjectPtr(form, buttonID_dis_two))) {
		SetDisasterLevel(2);
	} else if (CtlGetValue(GetObjectPtr(form, buttonID_dis_three))) {
		SetDisasterLevel(3);
	}
	if (CtlGetValue(GetObjectPtr(form, buttonID_Easy))) {
		SetDifficultyLevel(0);
	} else if (CtlGetValue(GetObjectPtr(form, buttonID_Medium))) {
		SetDifficultyLevel(1);
	} else if (CtlGetValue(GetObjectPtr(form, buttonID_Hard))) {
		SetDifficultyLevel(2);
	}
	game.auto_bulldoze = CtlGetValue(GetObjectPtr(form,
	    checkboxID_autobulldoze));
}


static FormPtr setupButtonConfig(void) MAP_SECTION;
static void saveButtonConfig(void) MAP_SECTION;
static void clearButtonConfig(void) MAP_SECTION;

/*
 * Handler for the button configuration form.
 * This form allows the user to configure the buttons on the PalmOs device.
 */
Boolean
hButtonConfig(EventPtr event)
{
	Boolean handled = false;
	static char okHit = 0;

	switch (event->eType) {
	case frmOpenEvent:
		WriteLog("open buttonconfig\n");
		PauseGame();
		FrmDrawForm(setupButtonConfig());
		okHit = 0;
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("close buttonconfig\n");
		if (okHit) saveButtonConfig();
		clearButtonConfig();
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
			okHit = 1;
			handled = true;
			FrmGotoForm(formID_pocketCity);
			break;
		case buttonID_Cancel:
			okHit = 0;
			handled = true;
			FrmGotoForm(formID_pocketCity);
			break;
		}
	default:
		break;
	}

	return (handled);
}

/*
 * The elements for the button configuration choices
 * It's defined like this to make the code smaller, and hopefully
 * more easy to read and maintain. You simply add in the entities
 * in the list and you're away. Note that changing the order of items
 * in this list has a tendency to upset the application state, making
 * the buttons behave strangely.
 */
static const struct bc_chelts {
	UInt16 popup;
	UInt16 list;
	UInt16 elt;
} bc_elts[] = {
	{ List_Cal_Popup, List_Cal, BkCalendar },
	{ List_Addr_Popup, List_Addr, BkAddress },
	{ List_HrUp_Popup, List_HrUp, BkHardUp },
	{ List_HrDn_Popup, List_HrDn, BkHardDown },
	{ List_ToDo_Popup, List_ToDo, BkToDo },
	{ List_Memo_Popup, List_Memo, BkMemo },
	{ List_Calc_Popup, List_Calc, BkCalc },
	{ List_Find_Popup, List_Find, BkFind },
#ifdef SONY_CLIE
	{ List_JogUp_Popup, List_JogUp, BkJogUp },
	{ List_JogDn_Popup, List_JogDn, BkJogDown },
	{ List_JogOut_Popup, List_JogOut, BkJogRelease },
#endif
	{ 0, 0, 0 }
};

/* Remember this between form load and form exit */
static char **Popups;

/*
 * set up the button config form.
 * Adds the string lists to the popups on the screen.
 * Saves having to have multiple copies in the form
 * definition. Costs a bit more at run-time, but saves in
 * application size
 */
static FormPtr
setupButtonConfig(void)
{
	FormPtr form = FrmGetActiveForm();
	Int16 poplen;
	ButtonKey bk;

	Popups = FillStringList(StrID_Popups, &poplen);
	/* do the buttons */
	for (bk = BkCalendar; bc_elts[bk].popup != 0; bk++) {
		ListType *lp;
		CtlSetLabel(GetObjectPtr(form, bc_elts[bk].popup),
		    Popups[gameConfig.pc.keyOptions[bk]]);
		lp = GetObjectPtr(form, bc_elts[bk].list);
		LstSetListChoices(lp, (Char **)Popups, poplen);
		LstSetSelection(lp, gameConfig.pc.keyOptions[bk]);
	}
	return (form);
}

/*
 * remember the button choices made.
 * This is not persisted until the application terminates cleanly.
 */
static void
saveButtonConfig(void)
{
	ButtonKey bk;
	FormPtr form = FrmGetActiveForm();

	for (bk = BkCalendar; bc_elts[bk].popup != 0; bk++) {
		gameConfig.pc.keyOptions[bk] =
		    LstGetSelection(GetObjectPtr(form, bc_elts[bk].list));
	}
}

/*
 * Clear any allocated data
 */
static void
clearButtonConfig(void)
{
	FreeStringList(Popups);
}
