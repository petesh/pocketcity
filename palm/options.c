#include <PalmOS.h>
#include <simcity_resconsts.h>
#include <sections.h>
#include <options.h>
#include <globals.h>
#include <globals.h>
#include <palmutils.h>

Boolean hOptions(EventPtr event)
{
    FormPtr form;
    int handled = 0;
    static char okHit = 0;

    switch (event->eType) {
    case frmOpenEvent:
        form = FrmGetActiveForm();
        FrmDrawForm(form);
        CtlSetValue(FrmGetObjectPtr(form, FrmGetObjectIndex(form,
                  buttonID_dis_off+game.disaster_level)), 1);
        CtlSetValue(FrmGetObjectPtr(form, FrmGetObjectIndex(form,
                  checkboxID_autobulldoze)), game.auto_bulldoze);
        okHit = 0;
        handled = 1;
        break;
    case frmCloseEvent:
        if (okHit) {
            form = FrmGetActiveForm();
            if (CtlGetValue(FrmGetObjectPtr(form,
                      FrmGetObjectIndex(form, buttonID_dis_off)))) {
                game.disaster_level = 0;
            } else if (CtlGetValue(FrmGetObjectPtr(form,
                      FrmGetObjectIndex(form, buttonID_dis_one)))) {
                game.disaster_level = 1;
            } else if (CtlGetValue(FrmGetObjectPtr(form,
                      FrmGetObjectIndex(form, buttonID_dis_two)))) {
                game.disaster_level = 2;
            } else if (CtlGetValue(FrmGetObjectPtr(form,
                      FrmGetObjectIndex(form, buttonID_dis_three)))) {
                game.disaster_level = 3;
            }
            game.auto_bulldoze = CtlGetValue(FrmGetObjectPtr(form,
                  FrmGetObjectIndex(form, checkboxID_autobulldoze)));
        }
        RestoreSpeed()
        break;
    case keyDownEvent:
        switch (event->data.keyDown.chr) {
        case vchrLaunch:
            FrmGotoForm(formID_pocketCity);
            handled = 1;
            break;
        }
    case ctlSelectEvent:
        switch (event->data.ctlEnter.controlID) {
        case buttonID_OK:
            okHit = 1;
            handled = 1;
            FrmGotoForm(formID_pocketCity);
            break;
        case buttonID_Cancel:
            okHit = 0;
            handled = 1;
            FrmGotoForm(formID_pocketCity);
            break;
        }
    default:
        break;
    }

    return (handled);
}

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

/* Handle the button configuration menu */
Boolean hButtonConfig(EventPtr event)
{
    FormPtr form;
    ButtonKey bk;
    int handled = 0;
    static char okHit = 0;
    static char **Popups;

    switch (event->eType) {
    case frmOpenEvent: {
        Int16 poplen;

        form = FrmGetActiveForm();
        FrmDrawForm(form);
        okHit = 0;

        /* do the buttons */
        for (bk = BkCalendar; bc_elts[bk].popup != 0; bk++) {
            ListType *lp;
            Popups = FillStringList(StrID_Popups, &poplen);
            CtlSetLabel(FrmGetObjectPtr(form, FrmGetObjectIndex(form, bc_elts[bk].popup)),
              Popups[gameConfig.pc.keyOptions[bk]]);
            lp = FrmGetObjectPtr(form, FrmGetObjectIndex(form, bc_elts[bk].list));
            LstSetListChoices(lp, (Char **)Popups, poplen);
            LstSetSelection(lp, gameConfig.pc.keyOptions[bk]);
        }
        handled = 1;
        break;
                       }
    case frmCloseEvent:
        if (okHit) {
            form = FrmGetActiveForm();
            for (bk = BkCalendar; bc_elts[bk].popup != 0; bk++) {
                gameConfig.pc.keyOptions[bk] =
                    LstGetSelection(FrmGetObjectPtr(form, FrmGetObjectIndex(form, bc_elts[bk].list)));
            }
            FreeStringList(Popups);
        }
        RestoreSpeed()
        break;
    case keyDownEvent:
        switch (event->data.keyDown.chr) {
        case vchrLaunch:
            FrmGotoForm(formID_pocketCity);
            handled = 1;
            break;
        }
    case ctlSelectEvent:
        switch (event->data.ctlEnter.controlID) {
        case buttonID_OK:
            okHit = 1;
            handled = 1;
            FrmGotoForm(formID_pocketCity);
            break;
        case buttonID_Cancel:
            okHit = 0;
            handled = 1;
            FrmGotoForm(formID_pocketCity);
            break;
        }
    default:
        break;
    }

    return (handled);
}

