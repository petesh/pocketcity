#include <PalmOS.h>
#include "simcity.h"
#include "../source/globals.h"
#include "../source/ui.h"

void BudgetInit(void);
void BudgetFreeMem(void);



extern Boolean hBudget(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType)
    {
        case frmOpenEvent:
            form = FrmGetActiveForm();
            BudgetInit();
            FrmDrawForm(form);
            handled = 1;
            break;
        case frmCloseEvent:
            UIWriteLog("closing budget\n");
            BudgetFreeMem();
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
        default:
            break;
    }

    return handled;
}

void BudgetInit(void)
{
    FormPtr form;
    char * temp;
    long signed int cashflow = 0;
    long unsigned int change = 0;
    
    form = FrmGetActiveForm();

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_RESIDENTIAL]*INCOME_RESIDENTIAL*tax/100;
    cashflow += change;
    StrPrintF(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_res)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_COMMERCIAL]*INCOME_COMMERCIAL*tax/100;
    cashflow += change;
    StrPrintF(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_com)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_INDUSTRIAL]*INCOME_INDUSTRIAL*tax/100;
    cashflow += change;
    StrPrintF(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_ind)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_ROADS]*UPKEEP_ROAD;
    cashflow -= change;
    StrPrintF(temp,"%lu", change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tra)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_POWERLINES]*UPKEEP_POWERLINE +
             BuildCount[COUNT_NUCLEARPLANTS]*UPKEEP_NUCLEARPLANT +
             BuildCount[COUNT_POWERPLANTS]*UPKEEP_POWERPLANT;
    cashflow -= change;
    StrPrintF(temp,"%lu", change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_pow)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_FIRE_STATIONS]*UPKEEP_FIRE_STATIONS; 
    cashflow -= change;
    StrPrintF(temp,"%lu", change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_def)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%+li", cashflow);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tot)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%li", credits+cashflow);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_bal)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%li", credits);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_now)), temp);
}

void BudgetFreeMem(void)
{
    FormPtr form;

    form = FrmGetActiveForm();
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_res))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_com))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_ind))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_tra))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_pow))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_def))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_tot))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_bal))));
    MemPtrFree((void*)CtlGetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form,labelID_budget_now))));
}

