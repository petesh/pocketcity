#include <PalmOS.h>
#include <unix_stdlib.h>
#include <simcity.h>
#include <simcity_resconsts.h>
#include <globals.h>
#include <ui.h>
#include <simulation.h>
#include <sections.h>
#include <budget.h>

static FormPtr budgetSetup(void) MAP_SECTION;
static void budgetCleanup(void) MAP_SECTION;

/*
 * Handler for the budget form.
 * Takes care of events directed at the form.
 */
Boolean
hBudget(EventPtr event)
{
    int handled = 0;

    switch (event->eType)
    {
        case frmOpenEvent:
            PauseGame();
            WriteLog("opening budget\n");
            FrmDrawForm(budgetSetup());
            handled = 1;
            break;
        case frmCloseEvent:
            WriteLog("closing budget\n");
            budgetCleanup();
            break;
        case keyDownEvent:
            switch (event->data.keyDown.chr)
            {
                case vchrLaunch:
                    FrmGotoForm(formID_pocketCity);
                    handled = 1;
                    break;
            }
            break;
        case menuEvent:
            switch (event->data.menu.itemID)
            {
                case menuitemID_BudgetBack:
                    FrmGotoForm(formID_pocketCity);
                    handled = 1;
                    break;
            }
            break;
        case popSelectEvent:
            if (event->data.popSelect.controlID == listID_shifter_popup) {
                UIGotoForm(event->data.popSelect.selection);
                handled = 1;
            }
            break;
        default:
            break;
    }

    return handled;
}

/*
 * A collection of all the labels and their related items.
 * Shrinks the code and makes it more consistent.
 */
static const struct updateentity {
    const BudgetNumber  item;
    const char          *formatstr;
    const UInt32        label;
} entity[] = {
    { bnResidential, "%lu", labelID_budget_res },
    { bnCommercial, "%lu", labelID_budget_com },
    { bnIndustrial, "%lu", labelID_budget_ind },
    { bnTraffic, "%lu", labelID_budget_tra },
    { bnPower, "%lu", labelID_budget_pow },
    { bnDefence, "%lu", labelID_budget_def },
    { bnCurrentBalance, "%li", labelID_budget_now },
    { bnChange, "%+li", labelID_budget_tot },
    { bnNextMonth, "%li", labelID_budget_bal },
    { 0, NULL, 0 }
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
    Char        *temp;
    FormPtr     form;
    MemHandle   texthandle;
    MemPtr      text;
    int         i;
    struct updateentity *entityp = (struct updateentity *)&entity[0];

    form = FrmGetActiveForm();
    temp = MemPtrNew(12 * (sizeof (entity) / sizeof (entity[0])));

    while (entityp->formatstr != NULL) {
        StrPrintF(temp, entityp->formatstr, BudgetGetNumber(entityp->item));
        CtlSetLabel(FrmGetObjectPtr(form,
              FrmGetObjectIndex(form, entityp->label)), temp);
        temp += 12;
        entityp++;
    }

    /* set up editable upkeep fields */
    for (i = 0; i < 3; i++) {
        texthandle = MemHandleNew(5);
        text = MemHandleLock(texthandle);
        StrPrintF(text, "%u", game.upkeep[i]);
        MemHandleUnlock(texthandle);
        FldSetTextHandle(FrmGetObjectPtr(form,
              FrmGetObjectIndex(form,fieldID_budget_tra+i)), texthandle);
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
    int         i, j;
    FormPtr     form = FrmGetActiveForm();

    for (i = 0; i < 3; i++) {
        j = atol(FldGetTextPtr(FrmGetObjectPtr(form, FrmGetObjectIndex(form, fieldID_budget_tra+i))));
        if (j < 0)
            j = 0;
        if (j > 100)
            j = 100;
        game.upkeep[i] = j;
    }

    form = FrmGetActiveForm();
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_res))));
}
