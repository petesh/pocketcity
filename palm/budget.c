#include <PalmOS.h>
#include <unix_stdlib.h>
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

void BudgetInit(void)
{
    FormPtr form;
    char * temp;
    long signed int cashflow = 0;
    long unsigned int change = 0;
    
    form = FrmGetActiveForm();

    temp = MemPtrNew(12);
    change = game.BuildCount[COUNT_RESIDENTIAL]*INCOME_RESIDENTIAL*game.tax/100;
    cashflow += change;
    StrPrintF(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_res)), temp);

    temp = MemPtrNew(12);
    change = game.BuildCount[COUNT_COMMERCIAL]*INCOME_COMMERCIAL*game.tax/100;
    cashflow += change;
    StrPrintF(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_com)), temp);

    temp = MemPtrNew(12);
    change = game.BuildCount[COUNT_INDUSTRIAL]*INCOME_INDUSTRIAL*game.tax/100;
    cashflow += change;
    StrPrintF(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_ind)), temp);

    temp = MemPtrNew(12);
    change = game.BuildCount[COUNT_ROADS]*UPKEEP_ROAD;
    change = (change*game.upkeep[UPKEEPS_TRAFFIC])/100;
    cashflow -= change;
    StrPrintF(temp,"%lu", change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tra)), temp);

    temp = MemPtrNew(12);
    change = game.BuildCount[COUNT_POWERLINES]*UPKEEP_POWERLINE +
             game.BuildCount[COUNT_NUCLEARPLANTS]*UPKEEP_NUCLEARPLANT +
             game.BuildCount[COUNT_POWERPLANTS]*UPKEEP_POWERPLANT;
    change = (change*game.upkeep[UPKEEPS_POWER])/100;
    cashflow -= change;
    StrPrintF(temp,"%lu", change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_pow)), temp);

    temp = MemPtrNew(12);
    change = game.BuildCount[COUNT_FIRE_STATIONS]*UPKEEP_FIRE_STATIONS; 
    change = (change*game.upkeep[UPKEEPS_DEFENCE])/100;
    cashflow -= change;
    StrPrintF(temp,"%lu", change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_def)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%+li", cashflow);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tot)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%li", game.credits+cashflow);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_bal)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%li", game.credits);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_now)), temp);


    // set up editable upkeep fields
    {
        MemHandle texthandle;
        MemPtr text;
        int i;
        for (i=0; i<3; i++) {
            texthandle = MemHandleNew(4);
            text = MemHandleLock(texthandle);
            StrPrintF(text, "%u", game.upkeep[i]);
            MemHandleUnlock(texthandle);
            FldSetTextHandle(FrmGetObjectPtr(form,FrmGetObjectIndex(form,fieldID_budget_tra+i)) , texthandle);
        }
    }
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

    // don't forget to save the upkeep settings ;)
    {
        int i,j;
        for (i=0; i<3; i++) {
            j = atol(FldGetTextPtr(FrmGetObjectPtr(form, FrmGetObjectIndex(form, fieldID_budget_tra+i))));
            if (j < 0)   {j = 0;}
            if (j > 100) {j = 100;}
            game.upkeep[i] = j;
        }
    }
}
