#include <PalmOS.h>
#include <unix_stdlib.h>
#include <simcity.h>
#include <simcity_resconsts.h>
#include <globals.h>
#include <ui.h>
#include <simulation.h>
#include <sections.h>
#include <budget.h>

#define	MILLION 1000000

static FormPtr budgetSetup(void) MAP_SECTION;
static void budgetCleanup(void) MAP_SECTION;

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
	{ bnResidential, "%lu%c", labelID_budget_res },
	{ bnCommercial, "%lu%c", labelID_budget_com },
	{ bnIndustrial, "%lu%c", labelID_budget_ind },
	{ bnTraffic, "%lu%c", labelID_budget_tra },
	{ bnPower, "%lu%c", labelID_budget_pow },
	{ bnDefence, "%lu%c", labelID_budget_def },
	{ bnCurrentBalance, "%li%c", labelID_budget_now },
	{ bnChange, "%+li%c", labelID_budget_tot },
	{ bnNextMonth, "%li%c", labelID_budget_bal },
	{ bnResidential, NULL, 0 }
};

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
	Char		temp[12];
	Char 		cp[10];
	Char		*ap;

	ResGetString(si_cash_scale, cp, 10);
	form = FrmGetActiveForm();
	/*
	 * Allocate 12 characters for each budget label in one place
	 */
	while (entityp->formatstr != NULL) {
		long value = BudgetGetNumber(entityp->item);

		ap = (Char *)cp;
		while (value > MILLION) {
			ap++;
			value /= 1000;
		}
		StrPrintF(temp, entityp->formatstr, value, *ap);
		FrmCopyLabel(form, entityp->label, temp);
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
		j = atol(FldGetTextPtr((FieldPtr)GetObjectPtr(form,
		    fieldID_budget_tra+i)));
		if (j < 0)
			j = 0;
		if (j > 100)
			j = 100;
		game.upkeep[i] = j;
	}

	form = FrmGetActiveForm();
}
