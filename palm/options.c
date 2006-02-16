/*!
 * \file
 * \brief code for handling options
 *
 *
 * This code deals with the two options dialogs in the palm platform
 * The options for the game and the options for the system (buttons)
 */

#include <PalmOS.h>
#include <Form.h>

#include <options.h>

#include <globals.h>
#include <simcity.h>
#include <logging.h>
#include <palmutils.h>
#include <simcity_resconsts.h>

static FormPtr setupOptions(void) CONFIG_SECTION;
static void saveOptions(void) CONFIG_SECTION;
static FormPtr setupButtonConfig(void) CONFIG_SECTION;
static void saveButtonConfig(void) CONFIG_SECTION;
static void clearButtonConfig(void) CONFIG_SECTION;

/*!
 * \brief this is the event loop for the options form.
 * Makes all the appropriate calls to
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

/*!
 * brief Set the state of the various fields in the options form.
 *
 * sets the values for the combo-boxes
 * \return the form pointer
 */
static FormPtr
setupOptions(void)
{
	FormPtr form = FrmGetActiveForm();
	UInt8 tval = getDisasterLevel();

	tval = (UInt8)((tval > 3) ? 3 : tval);
	CtlSetValue((ControlPtr)GetObjectPtr(form,
		(UInt16)(buttonID_dis_off + tval)), 1);

	tval = getDifficultyLevel();
	tval = (UInt8)((tval > 2) ? 2 : tval);
	CtlSetValue((ControlPtr)GetObjectPtr(form,
	    (UInt16)(buttonID_Easy + tval)), 1);

	CtlSetValue((ControlPtr)GetObjectPtr(form, checkboxID_autobulldoze),
	    GETAUTOBULLDOZE() ? 1 : 0);

	CtlSetValue((ControlPtr)GetObjectPtr(form, checkboxID_minimapvisible),
	    GETMINIMAPVISIBLE() ? 1 : 0);

	CtlSetValue((ControlPtr)GetObjectPtr(form, checkboxID_minimapdetailed),
	    GETMINIMAPDETAILED() ? 1 : 0);
	return (form);
}

/*!
 * \brief save the options from the option dialog to the application state.
 *
 * This does not persist the configuration out, that is the responsiblity
 * of the savegame routines.
 */
static void
saveOptions(void)
{
	FormPtr form = FrmGetActiveForm();
	UInt8 level = 0;
	if (CtlGetValue((ControlPtr)GetObjectPtr(form, buttonID_dis_off))) {
		level = 0;
	} else if (CtlGetValue((ControlPtr)GetObjectPtr(form,
			    buttonID_dis_one))) {
		level = 1;
	} else if (CtlGetValue((ControlPtr)GetObjectPtr(form,
			    buttonID_dis_two))) {
		level = 2;
	} else if (CtlGetValue((ControlPtr)GetObjectPtr(form,
			    buttonID_dis_three))) {
		level = 3;
	}
	setDisasterLevel(level);

	level = 0;

	if (CtlGetValue((ControlPtr)GetObjectPtr(form, buttonID_Easy))) {
		level = 0;
	} else if (CtlGetValue((ControlPtr)GetObjectPtr(form,
			    buttonID_Medium))) {
		level = 1;
	} else if (CtlGetValue((ControlPtr)GetObjectPtr(form, buttonID_Hard))) {
		level = 2;
	}
	setDifficultyLevel(level);
	SETAUTOBULLDOZE(CtlGetValue((ControlPtr)GetObjectPtr(form,
	    checkboxID_autobulldoze)));
	SETMINIMAPVISIBLE(CtlGetValue((ControlPtr)GetObjectPtr(form,
	    checkboxID_minimapvisible)));
	SETMINIMAPDETAILED(CtlGetValue((ControlPtr)GetObjectPtr(form,
	    checkboxID_minimapdetailed)));
}


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

/*!
 * \brief The elements for the button configuration choices
 *
 * It's defined like this to make the code smaller, and hopefully
 * more easy to read and maintain. You simply add in the entities
 * in the list and you're away. Note that changing the order of items
 * in this list has a tendency to upset the application state, making
 * the buttons behave strangely.
 */
static const struct bc_chelts {
	UInt16 popup; /*!< popup in question */
	UInt16 list; /*!< list in question */
	UInt16 elt; /*!< affected buttonkey choice */
} bc_elts[] = {
	{ List_Cal_Popup, List_Cal, BkCalendar },
	{ List_Addr_Popup, List_Addr, BkAddress },
	{ List_HrUp_Popup, List_HrUp, BkHardUp },
	{ List_HrDn_Popup, List_HrDn, BkHardDown },
	{ List_ToDo_Popup, List_ToDo, BkToDo },
	{ List_Memo_Popup, List_Memo, BkMemo },
	{ List_Calc_Popup, List_Calc, BkCalc },
	{ List_Find_Popup, List_Find, BkFind },
#if defined(PALM_HIGH)
	{ List_RockerLeft_Popup, List_RockerLeft, BkHardLeft },
	{ List_RockerRight_Popup, List_RockerRight, BkHardRight },
	{ List_RockerCenter_Popup, List_RockerCenter, BkRockerCenter },
#if defined(SONY_CLIE)
	{ List_JogUp_Popup, List_JogUp, BkJogUp },
	{ List_JogDn_Popup, List_JogDn, BkJogDown },
	{ List_JogOut_Popup, List_JogOut, BkJogRelease },
#endif /* SONY_CLIE */
#endif /* HIRES */
	{ 0, 0, 0 }
};

/*! \brief Remember this between form load and form exit */
static char **Popups;

/*!
 * \brief set up the button config form.
 * \return pointer to the form
 *
 * Adds the string lists to the popups on the screen.
 * Saves having to have multiple copies in the form
 * definition. Costs a bit more at run-time, but saves in
 * application size
 */
static FormPtr
setupButtonConfig(void)
{
	FormPtr form = FrmGetActiveForm();
	UInt16 poplen;
	ButtonKey bk;

	Popups = FillStringList(StrID_Popups, &poplen);
	/* do the buttons */
	for (bk = BkCalendar; bc_elts[bk].popup != 0; bk++) {
		ListPtr lp;
		UInt16 ko = gameConfig.pc.keyOptions[bk];

		if (ko > poplen) {
			ko = poplen - 1;
		}
		CtlSetLabel((ControlPtr)GetObjectPtr(form, bc_elts[bk].popup),
		    Popups[ko]);
		lp = (ListPtr)GetObjectPtr(form, bc_elts[bk].list);
		LstSetListChoices(lp, (Char **)Popups, (Int16)poplen);
		LstSetSelection(lp, (Int16)ko);
	}
	return (form);
}

/*!
 * \brief Remember the button choices made.
 *
 * This is not persisted until the application terminates cleanly.
 */
static void
saveButtonConfig(void)
{
	ButtonKey bk;
	FormPtr form = FrmGetActiveForm();

	for (bk = BkCalendar; bc_elts[bk].popup != 0; bk++) {
		gameConfig.pc.keyOptions[bk] =
		    (keyEvent)LstGetSelection((ListPtr)GetObjectPtr(form,
				bc_elts[bk].list));
	}
}

/*!
 * \brief Clear any allocated data
 *
 * Namely the list of popup strings
 */
static void
clearButtonConfig(void)
{
	FreeStringList(Popups);
}
