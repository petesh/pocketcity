#include <PalmOS.h>
#include <unix_stdlib.h>
#include <simcity.h>
#include <simcity_resconsts.h>
#include <globals.h>
#include <ui.h>
#include <simulation.h>
#include <sections.h>
#include <budget.h>
#include <stddef.h>

#define	MILLION 1000000

static FormPtr budgetSetup(void) MAP_SECTION;
static void dealRepeats(EventPtr event) MAP_SECTION;
static void dealFieldContentChange(UInt16 fieldID) MAP_SECTION;
static const struct buttonmapping *getIndex(UInt16 buttonControl,
    Boolean isButton) MAP_SECTION;
static void updateBudgetValue(FormPtr form, UInt16 label, const Char *format,
    long value) MAP_SECTION;
static void updateBudgetNumber(BudgetNumber bn) MAP_SECTION;

static const struct buttonmapping {
	UInt16		down;
	UInt16		up;
	UInt16		field;
	BudgetNumber	affects;
	Int16		min;
	Int16		max;
	UInt32		fldOffset;
} buttonmappings[] = {
	{ rbutton_taxdown, rbutton_taxup, fieldID_taxrate, bnIncome,
		0, 20, offsetof(GameStruct, tax) },
	{ rbutton_trafdown, rbutton_trafup, fieldID_budget_tra,
		bnTraffic, 0, 100, offsetof(GameStruct, upkeep[0]) },
	{ rbutton_powdown, rbutton_powup, fieldID_budget_pow,
		bnPower, 0, 100, offsetof(GameStruct, upkeep[1]) },
	{ rbutton_defdown, rbutton_defup, fieldID_budget_def,
		bnDefence, 0, 100, offsetof(GameStruct, upkeep[2]) }
};

#define BUTTONMAPLEN	(sizeof (buttonmappings) / sizeof (buttonmappings[0]))

static const struct buttonmapping *
getIndex(UInt16 buttonControl, Boolean isButton)
{
	UInt16 i = 0;
	for (i = 0; i < BUTTONMAPLEN; i++) {
		if (isButton) {
			if (buttonControl == buttonmappings[i].down ||
			    buttonControl == buttonmappings[i].up)
				return (&buttonmappings[i]);
		} else {
			if (buttonControl == buttonmappings[i].field)
				return (&buttonmappings[i]);
		}
	}
	return (NULL);
}

static void
dealRepeats(EventPtr event)
{
	UInt16 control = event->data.ctlRepeat.controlID;
	const struct buttonmapping *bm = getIndex(control, true);
	FieldPtr fp;
	MemHandle mh;
	MemPtr mp;
	Int32 fld;

	if (bm == NULL) return;

	fp = (FieldPtr)GetObjectPtr(FrmGetActiveForm(), bm->field);
	mh = FldGetTextHandle(fp);

	FldSetTextHandle(fp, NULL);
	mp = MemHandleLock(mh);
	fld = StrAToI(mp);

	if (control == bm->down)
		fld--;
	else
		fld++;
	if (fld < bm->min)
		fld = bm->min;
	else if (fld > bm->max)
		fld = bm->max;
	StrPrintF(mp, "%ld", fld);
	MemHandleUnlock(mh);
	FldSetTextHandle(fp, mh);
	FldDrawField(fp);
	((UInt8 *)(&game))[bm->fldOffset] = (UInt8)fld;
	if (bm->affects != bnChange)
		updateBudgetNumber(bnChange);
	updateBudgetNumber(bm->affects);
	updateBudgetNumber(bnNextMonth);
}

static void
dealFieldContentChange(UInt16 fieldID)
{
	FieldPtr fp;
	MemHandle mh;
	MemPtr mp;
	Boolean limited = false;
	Int32 fld;
	const struct buttonmapping *bm = getIndex(fieldID, false);

	if (bm == NULL) return;
	fp = (FieldPtr)GetObjectPtr(FrmGetActiveForm(), bm->field);
	mh = FldGetTextHandle(fp);
	mp = MemHandleLock(mh);
	fld = StrAToI(mp);
	if (fld < bm->min) {
		limited = true;
		fld = bm->min;
	} else if (fld > bm->max) {
		limited = true;
		fld = bm->max;
	}

	if (limited) {
		FldSetTextHandle(fp, NULL);
		StrPrintF(mp, "%ld", fld);
	}
	MemHandleUnlock(mh);

	if (limited) {
		FldSetTextHandle(fp, mh);
		FldDrawField(fp);
	}

	((UInt8 *)(&game))[bm->fldOffset] = (UInt8)fld;
	updateBudgetNumber(bnChange);
	updateBudgetNumber(bnNextMonth);
	updateBudgetNumber(bm->affects);
}
/*
 * Handler for the budget form.
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
		WriteLog("opening budget\n");
		FrmDrawForm(budgetSetup());
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("closing budget\n");
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
	default:
		break;
	}

	return (handled);
}

/*
 * A collection of all the labels and their related items.
 * Shrinks the code and makes it more consistent.
 */
static const struct updateentity {
	const BudgetNumber	item;
	const char		*formatstr;
	const UInt32		label;
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

#define ENTITY_COUNT	(sizeof (entity) / sizeof (entity[0]))

static void
updateBudgetValue(FormPtr form, UInt16 label, const Char *format, long value)
{
	static Char cp[10];
	Char temp[12];
	Char	*ap;

	*temp='$';
	if (*cp == '\0')
		ResGetString(si_cash_scale, cp, 10);

	ap = (Char *)cp;
	while(value > MILLION) {
		ap++;
		value /= 1000;
	}
	StrPrintF(temp+1, format, value, *ap);
	FrmCopyLabel(form, label, temp);
}

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
		updateBudgetValue(form, entityp->label, entityp->formatstr, value);
		break;
	}
}

/*
 * Set up the budget form.
 * configures all the text strings to read the values from the main
 * simulation, as well as adding in space for the text fields to
 * choose the %age to give over to each service.
 */
static FormPtr
budgetSetup(void)
{
	FormPtr		form;
	MemHandle	texthandle;
	MemPtr		text;
	int		i;
	struct updateentity *entityp = (struct updateentity *)&entity[0];

	form = FrmGetActiveForm();
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
		    fieldID_budget_tra+i), texthandle);
	}
	texthandle = MemHandleNew(5);
	text = MemHandleLock(texthandle);
	StrPrintF((char *)text, "%u", game.tax);
	MemHandleUnlock(texthandle);
	FldSetTextHandle((FieldPtr)GetObjectPtr(form,
		    fieldID_taxrate), texthandle);
	return (form);
}
