#include <PalmOS.h>
#include <unix_stdlib.h>
#include "simcity.h"
#include "../source/globals.h"
#include "../source/ui.h"
#include "../source/simulation.h"

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
    
    form = FrmGetActiveForm();

    temp = MemPtrNew(12);
    StrPrintF(temp,"%lu", BudgetGetNumber(BUDGET_RESIDENTIAL));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_res)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%lu", BudgetGetNumber(BUDGET_COMMERCIAL));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_com)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%lu", BudgetGetNumber(BUDGET_INDUSTRIAL));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_ind)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%lu", BudgetGetNumber(BUDGET_TRAFFIC));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tra)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%lu", BudgetGetNumber(BUDGET_POWER));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_pow)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%lu", BudgetGetNumber(BUDGET_DEFENCE));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_def)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%li", BudgetGetNumber(BUDGET_CURRENT_BALANCE));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_now)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%+li", BudgetGetNumber(BUDGET_CHANGE));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tot)), temp);

    temp = MemPtrNew(12);
    StrPrintF(temp,"%li", BudgetGetNumber(BUDGET_NEXT_MONTH));
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_bal)), temp);


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
