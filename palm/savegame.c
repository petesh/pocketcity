/* This file handles savegames and the savegame UI list */

#include <PalmOS.h>
#include <StringMgr.h>
#include <unix_string.h>
#include <unix_stdlib.h>
#include <StdIOPalm.h>
#include <stddef.h>
#include "simcity.h"
#include "simcity_resconsts.h"
#include "savegame.h"
#include "../source/ui.h"
#include "../source/globals.h"
#include "../source/zakdef.h"
#include "../source/drawing.h"
#include "../source/build.h"
#include "../source/handler.h"
#include "../source/simulation.h"
#include "resCompat.h"

void _UIUpdateSaveGameList(void);
void _UICreateNewSaveGame(void);
void _UICleanSaveGameList(void);
void _UIDeleteFromList(void);
int  _UILoadFromList(void);
void UIDeleteGame(UInt16 index);
int  UILoadGame(UInt16 index);
void UINewGame(void);

char *pArray[50];
static short int savegame_index = 0;
#define LASTGAME        ((UInt16)~0)

void
_UIUpdateSaveGameList(void)
{
    DmOpenRef db;
    UInt16 index = 0;
    MemHandle rec;
    unsigned char * pTemp;
    unsigned short int nRec, nsIndex=0;
    FormPtr form;


    db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(), dmModeReadOnly);
    if (!db) {
        form = FrmGetFormPtr(formID_files);
        LstSetListChoices(FrmGetObjectPtr(form,
              FrmGetObjectIndex(form, listID_FilesList)), NULL, 0);
        if (form == FrmGetActiveForm())
            LstDrawList(FrmGetObjectPtr(form,
                  FrmGetObjectIndex(form, listID_FilesList)));
        return; /* no database */
    }
    nRec = DmNumRecords(db);

    for (index=1; index<nRec; index++) {
        rec = DmQueryRecord(db, index);
        if (rec) {
            pArray[nsIndex] = MemPtrNew(20);
            pTemp = (unsigned char*)MemHandleLock(rec);
            strncpy(pArray[nsIndex], (char*)pTemp +
              offsetof(GameStruct, cityname), 20);
            MemHandleUnlock(rec);
            nsIndex++;
        }
    }
    DmCloseDatabase(db);

    /* update list */
    form = FrmGetFormPtr(formID_files);
    LstSetListChoices(FrmGetObjectPtr(form,
          FrmGetObjectIndex(form, listID_FilesList)), pArray, nsIndex);
    if (form == FrmGetActiveForm())
        LstDrawList(FrmGetObjectPtr(form,
              FrmGetObjectIndex(form, listID_FilesList)));
}

void
_UICleanSaveGameList(void)
{
    int i,n;
    FormPtr form = FrmGetFormPtr(formID_files);
    void *fp;

    if (form == NULL) return;
    fp = FrmGetObjectPtr(form, FrmGetObjectIndex(form, listID_FilesList));
    if (fp == NULL) return;
    n = LstGetNumberOfItems(fp);
    for (i = 0; i < n; i++) {
        MemPtrFree((void*)LstGetSelectionText(fp, i));
    }
    LstSetListChoices(fp, NULL, 0);
}


void
_UICreateNewSaveGame(void)
{
    Err err = 0;
    DmOpenRef db;
    MemHandle rec;
    void * pRec;
    
    UInt16 index = dmMaxRecordIndex;

    db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(), dmModeReadWrite);
    if (!db) {
        err = DmCreateDatabase(0, SGNAME, GetCreatorID(), SGTYP, false);
        if (err) {
            return; /* couldn't create */
        }

        db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(),
          dmModeReadWrite);
        if (!db) {
            return; /* couldn't open after creation */
        }
    }
    /* no, this should NOT be an "else if" */
    if (db) {
        if (DmNumRecords(db) >= 50) {
            /* TODO: alert user - max is 50 savegames */
        } else {
            if (DmNumRecords(db) == 0) {
                /*
		 * create an empty record in slot 0 if it's not there.
		 * This is used for autosaves
		 */
                rec = DmNewRecord(db, &index, GetMapMul() + sizeof (game));
                if (rec) {
                    DmReleaseRecord(db, index, true);
                }
            }
            index = dmMaxRecordIndex;
            rec = DmNewRecord(db, &index, GetMapMul() + sizeof (game));
            if (rec) {
                pRec = MemHandleLock(rec);
                /* write the header and some globals */
                DmWrite(pRec, 0, "PC00", 4);
                DmWrite(pRec, offsetof(GameStruct, cityname),
                  (char*)game.cityname, 20);
                MemHandleUnlock(rec);
                DmReleaseRecord(db, index, true);
            }
        }
        DmCloseDatabase(db);
    }
}

int
_UILoadFromList(void)
{
    FormPtr form = FrmGetFormPtr(formID_files);
    int index = LstGetSelection(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)));
    if (index >= 0) {
        return UILoadGame(index+1); /* +1 as slot (0) is autosave */
    } else {
        return 0;
    }
}

void
_UIDeleteFromList(void)
{
    FormPtr form = FrmGetFormPtr(formID_files);
    int index = LstGetSelection(FrmGetObjectPtr(form,
          FrmGetObjectIndex(form,listID_FilesList)));
    if (index != noListSelection) {
        return UIDeleteGame(index+1); /* +1 because slot(0) is the autosave */
    }
}


void
UIClearAutoSaveSlot(void)
{
    DmOpenRef db;
    MemHandle rec;
    void * pRec;

    db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(), dmModeReadWrite);
    if (!db) {
            return; /* couldn't create */
    }
    /* no, this should NOT be an "else if" */
    rec = DmGetRecord(db,0);
    if (rec) {
        pRec = MemHandleLock(rec);
        DmWrite(pRec, 0, "PCNO", 4);
        MemHandleUnlock(rec);
        DmReleaseRecord(db, 0, true);
    }

    DmCloseDatabase(db);
}

void
UIResetViewable(void)
{
    /*
     * The UI part is responsible for setting
     * the visible_x/y vars
     * and then call SetupNewGame()
     * tile is 16 * 16
     * we reserve nothing on the x axis, but 2 tiles on the y
     */
    vgame.tileSize = 16;
    vgame.visible_x = sWidth / vgame.tileSize;
    vgame.visible_y = (sHeight / vgame.tileSize) - 2;
}

void
UINewGame(void)
{
    SetupNewGame();
    UIResetViewable();
    game_in_progress = 1;
}


int
UILoadAutoGame(void)
{
    return UILoadGame(0);
}

static DmOpenRef
OpenMyDB(void)
{
    Err err = 0;
    DmOpenRef db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(),
      dmModeReadWrite);
    if (!db) {
        err = DmCreateDatabase(0, SGNAME, GetCreatorID(), SGTYP, false);
        if (err)
            return (NULL); /* couldn't create */
        db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(),
          dmModeReadWrite);
    }
    return (db);
}

void
UIDeleteGame(UInt16 index)
{
    DmOpenRef db;

    db = OpenMyDB();
    if (!db) {
        return; /* couldn't open after creation */
    }

    if (DmNumRecords(db) < index) {
        return; /* index doesn't exist */
    }

    DmRemoveRecord(db, index);
    DmCloseDatabase(db);
}

int
UILoadGame(UInt16 index)
{
    DmOpenRef db;
    MemHandle rec;
    unsigned char * pTemp;
    short int loaded = 0;

    db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(), dmModeReadOnly);
    if (!db) {
        return (0); /* no database */
    }
    if (index == LASTGAME) index = DmNumRecords(db) - 1;
    rec = DmQueryRecord(db, index);
    if (rec) {
        pTemp = (unsigned char*)MemHandleLock(rec);
        /* flagged to create new game */
        if (StrNCompare("PC00",(char*)pTemp,4) == 0) {
            UINewGame();
            UISaveGame(index); /* save the newly created map */
            loaded = 2;
        } else if (StrNCompare(SAVEGAMEVERSION, (char*)pTemp, 4) == 0) {
            /* version check */
            LockWorld();
            MemMove((void*)&game, pTemp, sizeof(GameStruct));
            MemMove(worldPtr, pTemp+sizeof(GameStruct), GetMapMul());
            UnlockWorld();
            /* update the power and water grid: */
            PostLoadGame();
            UIResetViewable();
            loaded = 1;
        } else if (StrNCompare("PCNO", (char*)pTemp,4) == 0) {
            /* flagged to be an empty save game */
            loaded = 0;
        } else {
            FrmAlert(alertID_invalidSaveVersion);
        }

        if (loaded != 2) {
            MemHandleUnlock(rec);
        }
    }
    DmCloseDatabase(db);
    if (loaded != 0) {
        savegame_index = index;
    } else {
        savegame_index = 0;
    }
    return loaded;
}

void
UISaveMyCity(void)
{
    DmOpenRef db = NULL;
    MemHandle rec;
    GameStruct *pRec;
    unsigned short int nRec;
    unsigned short int i;

    if (savegame_index == 0) { /* I am the autosave slot... get real slot */
        db = OpenMyDB();
        if (!db) {
            FrmCustomAlert(alertID_majorbad,
              "Can't Open/Create the savegame database", NULL, NULL);
            return;
        }
        nRec = DmNumRecords(db);
        for (i = 1; i < nRec; i++) {
            rec = DmQueryRecord(db, i);
            if (rec) {
                pRec = (GameStruct *)MemHandleLock(rec);
                if (strcmp(pRec->cityname, game.cityname) == 0) {
                    savegame_index = i;
                    MemHandleUnlock(rec);
                    break;
                }
                MemHandleUnlock(rec);
            }
        }
    }
    if (db) DmCloseDatabase(db);
    if (savegame_index == 0) {
        savegame_index = LASTGAME;
    }
    UISaveGame(savegame_index);
}

void
UISaveAutoGame(void)
{
    UISaveGame(0);
}

void
UISaveGame(UInt16 index)
{
    DmOpenRef db = NULL;
    MemHandle rec;
    void * pRec;

    db = OpenMyDB();
    if (index <= DmNumRecords(db)) {
        DmRemoveRecord(db, index);
    } else {
        index = DmNumRecords(db) + 1;
        savegame_index = index;
    }
    rec = DmNewRecord(db,&index, GetMapMul() + sizeof (GameStruct));
    if (rec) {
        pRec = MemHandleLock(rec);
        LockWorld();
        /* write the header and some globals */
        DmWrite(pRec,0,(void*)&game, sizeof (GameStruct));
        DmWrite(pRec, sizeof (GameStruct), (void*)(unsigned char*)worldPtr,
            GetMapMul());
        UnlockWorld();
        MemHandleUnlock(rec);
        DmReleaseRecord(db,index,true);
    }
    { UInt16 attr = dmHdrAttrBackup;
        DmSetDatabaseInfo(0, DmFindDatabase(0, SGNAME), NULL, &attr, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }
    DmCloseDatabase(db);
}




Boolean
hFilesNew(EventPtr event)
{
    FormPtr form;
    int handled = 0;
    char * pGameName;

    switch (event->eType) {
    case frmOpenEvent:
        game_in_progress = 0;
        form = FrmGetActiveForm();
        FrmDrawForm(form);
        FrmSetFocus(form, FrmGetObjectIndex(form, fieldID_newGameName));
        handled = 1;
        break;
    case frmCloseEvent:
        break;
    case ctlSelectEvent:
        switch (event->data.ctlSelect.controlID) {
        case buttonID_FilesNewCreate:
            UIWriteLog("Create pushed\n");
            /* need to fetch the savegame name from the form */
            form = FrmGetActiveForm();
            pGameName = FldGetTextPtr(FrmGetObjectPtr(form,
                  FrmGetObjectIndex(form, fieldID_newGameName)));
            if (pGameName != NULL) {
                strcpy((char*)game.cityname,pGameName);
                _UICreateNewSaveGame();
                _UICleanSaveGameList();
                if (UILoadGame(LASTGAME)) {
                    FrmEraseForm(form);
                    form = FrmGetFormPtr(formID_files);
                    if (form)
                        FrmEraseForm(form);
                    FrmGotoForm(formID_pocketCity);
                } else {
                    _UIUpdateSaveGameList();
                }
            } else {
                strcpy((char*)game.cityname,"");
                UIWriteLog("No name specified\n");
            }
            handled = 1;
            break;
        case buttonID_FilesNewCancel:
            UIWriteLog("Cancel pushed\n");
            /* set (char*)cityname to '\0' */
            game.cityname[0] = '\0';
            FrmReturnToForm(0);
            handled = 1;
            break;
        }
        break;
    default:
        break;
    }

    return handled;
}


extern Boolean
hFiles(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType) {
    case frmOpenEvent:
        game_in_progress = 0;
        form = FrmGetActiveForm();
        FrmDrawForm(form);
        _UIUpdateSaveGameList();
        handled = 1;
        break;
    case frmCloseEvent:
        _UICleanSaveGameList();
        break;
    case ctlSelectEvent:
        switch (event->data.ctlSelect.controlID)
        {
        case buttonID_FilesNew:
            /* create new game and add it to the list */
            FrmPopupForm(formID_filesNew);
            handled = 1;
            break;
        case buttonID_FilesLoad:
            /* create new game */
            if (_UILoadFromList()) {
                FrmGotoForm(formID_pocketCity);
            }
            handled = 1;
            break;
        case buttonID_FilesDelete:
            _UIDeleteFromList();
            _UICleanSaveGameList();
            _UIUpdateSaveGameList();
            handled = 1;
            break;
        }
        break;
    default:
        break;
    }

    return handled;
}
