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
static void budgetCleanup(void) MAP_SECTION;
static void dealRepeats(EventPtr event) MAP_SECTION;
static const struct buttonmapping *getIndex(UInt16 buttonControl)
    MAP_SECTION;
static void updateBudgetValue(FormPtr form, UInt16 label, const Char *format,
    long value) MAP_SECTION;
static void updateBudgetNumber(BudgetNumber bn) MAP_SECTION;

static const struct buttonmapping {
	UInt16		down;
	UInt16		up;
	UInt16		field;
	Int16		min;
	Int16		max;
	UInt32		fldOffset;
} buttonmappings[] = {
	{ rbutton_taxdown, rbutton_taxup, fieldID_taxrate, 0, 20, offsetof(GameStruct, tax) }
};

#define BUTTONMAPLEN	(sizeof (buttonmappings) / sizeof (buttonmappings[0]))

static const struct buttonmapping *
getIndex(UInt16 buttonControl)
{
	UInt16 i = 0;
	for (i = 0; i < BUTTONMAPLEN; i++) {
		if (buttonControl == buttonmappings[i].down ||
		    buttonControl == buttonmappings[i].up)
			return (&buttonmappings[i]);
	}
	return (NULL);
}

static void
dealRepeats(EventPtr event)
{
	UInt16 control = event->data.ctlRepeat.controlID;
	const struct buttonmapping *bm = getIndex(control);
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
	updateBudgetNumber(bnChange);
	updateBudgetNumber(bnNextMonth);
}
/*
 * Handler for the budget form.
 * Takes care of events directed at the form.
 */
Boolean
hBudget(EventPtr event)
{
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		PauseGame();
		WriteLog("opening budget\n");
		FrmDrawForm(budgetSetup());
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("closing budget\n");
		budgetCleanup();
		break;
	case keyDownEvent:
		switch (event->data.keyDown.chr) {
		case vchrLaunch:
			FrmGotoForm(formID_pocketCity);
			handled = true;
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

/*
 * save the upkeep settings.
 * Also releases the memory that was allocated by the initialization routine.
 */
static void
budgetCleanup(void)
{
	int	i, j;
	FormPtr form = FrmGetActiveForm();

	for (i = 0; i < 3; i++) {
		Int32 j = StrAToI(FldGetTextPtr((FieldPtr)GetObjectPtr(form,
		    fieldID_budget_tra+i)));
		if (j < 0)
			j = 0;
		if (j > 100)
			j = 100;
		game.upkeep[i] = j;
	}
	j = StrAToI(FldGetTextPtr((FieldPtr)GetObjectPtr(form,
	    fieldID_taxrate)));
	if (j < 0)
		j = 0;
	if (j > 20)
		j = 20;
	game.tax = j;

	form = FrmGetActiveForm();
}
