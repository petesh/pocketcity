/*!
 * \file
 * \brief the palm budget UI code
 *
 * This code deals with the code for the budget form from the pocketcity
 * application
 */
#include <PalmOS.h>
#include <unix_stdlib.h>
#include <stddef.h>

#include <budget.h>
#include <palmutils.h>
#include <simcity.h>
#include <simcity_resconsts.h>
#include <globals.h>
#include <logging.h>
#include <ui.h>
#include <simulation.h>
#include <resCompat.h>
#include <repeathandler.h>

static FormPtr budgetSetup(FormPtr form) BUDGET_SECTION;
static void dealRepeats(EventPtr event) BUDGET_SECTION;
static void dealFieldContentChange(UInt16 fieldID) BUDGET_SECTION;
static void updateBudgetValue(FormPtr form, UInt16 label, const Char *format,
    long value) BUDGET_SECTION;
static void updateBudgetNumber(BudgetNumber bn) BUDGET_SECTION;
static void post_fieldhandler(UInt16 field, buttonmapping_t *map,
    Int32 newValue) BUDGET_SECTION;

/*! \brief button mapping for the spinners */
static buttonmapping_t budget_map[] = {
	{ rbutton_taxdown, rbutton_taxup, fieldID_taxrate,
		0, 20, bnIncome, offsetof(GameStruct, tax) },
	{ rbutton_trafdown, rbutton_trafup, fieldID_budget_tra,
		0, 100, bnTraffic, offsetof(GameStruct, upkeep[0]) },
	{ rbutton_powdown, rbutton_powup, fieldID_budget_pow,
		0, 100, bnPower, offsetof(GameStruct, upkeep[1]) },
	{ rbutton_defdown, rbutton_defup, fieldID_budget_def,
		0, 100, bnDefence, offsetof(GameStruct, upkeep[2]) },
	{ 0, 0, 0, 0, 0, 0, 0 }
};

/*!
 * \brief deal with the new value that has been passed into a structure
 * \param field the field that was changed
 * \param map the button that was pressed
 * \param newValue the new value of the field in question
 */
static void
post_fieldhandler(UInt16 field __attribute__((unused)), buttonmapping_t *map,
    Int32 newValue)
{
	((UInt8 *)(&game))[map->special2] = (UInt8)newValue;
	if ((BudgetNumber)map->special1 != bnChange)
		updateBudgetNumber(bnChange);
	updateBudgetNumber((BudgetNumber)map->special1);
	updateBudgetNumber(bnNextMonth);
}

/*!
 * \brief deal with the repeat event
 * \param event the event that was passed in
 */
static void
dealRepeats(EventPtr event)
{
	UInt16 control = event->data.ctlRepeat.controlID;
	(void) processRepeater(budget_map, control, true, post_fieldhandler);
}

/*!
 * \brief deal with the manual change of the contents of a field
 * \param fieldID the id# of the field that was changed
 */
static void
dealFieldContentChange(UInt16 fieldID)
{
	(void) processRepeater(budget_map, fieldID, false, post_fieldhandler);
}
/*!
 * Takes care of events directed at the form.
 */
Boolean
hBudget(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	UInt16 fieldID;
	WChar chr;

	switch (event->eType) {
	case frmOpenEvent:
		PauseGame();
		form = FrmGetActiveForm();
		SetSilkResizable(form, true);
		collapseMove(form, CM_DEFAULT, NULL, NULL);
		WriteLog("opening budget\n");
		FrmDrawForm(budgetSetup(form));
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("closing budget\n");
		SetSilkResizable(NULL, false);
		break;
	case keyDownEvent:
		chr = event->data.keyDown.chr;
		if (chr == vchrLaunch) {
			FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		}
		/* Cheat ... enqueue a ~ character to post-process the text */
		if ((chr >= chrDigitZero && chr <= chrDigitNine) ||
		    (chr == chrBackspace)) {
			EvtEnqueueKey(chrTilde, 0, 0);
			break;
		}
		if (chr == chrTilde) {
			form = FrmGetActiveForm();
			fieldID = FrmGetObjectId(form, FrmGetFocus(form));
			dealFieldContentChange(fieldID);
			break;
		}
		break;
	case menuEvent:
		switch (event->data.menu.itemID) {
		case menuitemID_BudgetBack:
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
	case ctlRepeatEvent:
		/* Deal with the repeating controls ... */
		dealRepeats(event);
		break;
#if defined(HRSUPPORT)
	case winDisplayChangedEvent:
#if defined(SONY_CLIE)
	case vchrSilkResize:
#endif
		form = FrmGetActiveForm();
		if (collapseMove(form, CM_DEFAULT, NULL, NULL)) {
			FrmDrawForm(form);
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
 * \brief A collection of all the labels and their related items.
 *
 * Shrinks the code and makes it more consistent.
 */
static const struct updateentity {
	const BudgetNumber	item; /*!< the budget item */
	const char		*formatstr; /*!< string for formatting */
	const UInt16		label; /*!< ID of label to alter */
} entity[] = {
	{ bnIncome, "%lu%c", labelID_budget_inc },
	{ bnTraffic, "%lu%c", labelID_budget_tra },
	{ bnPower, "%lu%c", labelID_budget_pow },
	{ bnDefence, "%lu%c", labelID_budget_def },
	{ bnCurrentBalance, "%li%c", labelID_budget_now },
	{ bnChange, "%+li%c", labelID_budget_tot },
	{ bnNextMonth, "%li%c", labelID_budget_bal },
	{ bnResidential, NULL, 0 }
};

/*! \brief count of budget number entities */
#define ENTITY_COUNT	(sizeof (entity) / sizeof (entity[0]))

/*!
 * \brief update the value of a budget number field.
 *
 * Scales the number with the K,M... scaling.
 * \param form the form containing the items
 * \param label the label to alter
 * \param format the format of the output
 * \param value the value of the output
 */
static void
updateBudgetValue(FormPtr form, UInt16 label, const Char *format, long value)
{
	Char temp[12];
	Char scale;

	*temp='$';
	value = (Int32)scaleNumber((UInt32)value, &scale);

	StrPrintF(temp+1, format, value, scale);
	FrmCopyLabel(form, label, temp);
}

/*!
 * \update the field in question
 * \param item the item to update
 */
static void
updateBudgetNumber(const BudgetNumber item)
{
	long value = BudgetGetNumber(item);
	struct updateentity *entityp = (struct updateentity *)&entity[0];
	FormPtr form;

	form = FrmGetActiveForm();

	while (entityp->formatstr != NULL) {
		if (entityp->item != item) {
			entityp++;
			continue;
		}
		updateBudgetValue(form, entityp->label, entityp->formatstr,
		    value);
		break;
	}
}

/*!
 * \brief Set up the budget form.
 * \param form the form to use
 * \return the form that was passed in.
 *
 * configures all the text strings to read the values from the main
 * simulation, as well as adding in space for the text fields to
 * choose the %age to give over to each service.
 */
static FormPtr
budgetSetup(FormPtr form)
{
	MemHandle	texthandle;
	MemPtr		text;
	int		i;
	struct updateentity *entityp = (struct updateentity *)&entity[0];

	/*
	 * Allocate 12 characters for each budget label in one place
	 */
	while (entityp->formatstr != NULL) {
		long value = BudgetGetNumber(entityp->item);
		updateBudgetValue(form, entityp->label, entityp->formatstr,
		    value);

		entityp++;
	}

	/* set up editable upkeep fields */
	for (i = 0; i < 3; i++) {
		texthandle = MemHandleNew(5);
		text = MemHandleLock(texthandle);
		StrPrintF((char *)text, "%u", game.upkeep[i]);
		MemHandleUnlock(texthandle);
		FldSetTextHandle((FieldPtr)GetObjectPtr(form,
		    (UInt16)(fieldID_budget_tra + i)), texthandle);
	}
	texthandle = MemHandleNew(5);
	text = MemHandleLock(texthandle);
	StrPrintF((char *)text, "%u", game.tax);
	MemHandleUnlock(texthandle);
	FldSetTextHandle((FieldPtr)GetObjectPtr(form,
		    fieldID_taxrate), texthandle);
	return (form);
}
