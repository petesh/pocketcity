/* This file handles savegames and the savegame UI list */

#include <PalmOS.h>
#include <StringMgr.h>
#include <unix_string.h>
#include <StdIOPalm.h>
#include "simcity.h"
#include "savegame.h"
#include "../source/ui.h"
#include "../source/globals.h"
#include "../source/zakdef.h"
#include "../source/drawing.h"
#include "../source/build.h"
#include "../source/handler.h"
#include "../source/simulation.h"


void _UIUpdateSaveGameList(void);
void _UICreateNewSaveGame(void);
void _UICleanSaveGameList(void);
void _UIDeleteFromList(void);
int  _UILoadFromList(void);
int  _UILoadNewestFromList(void);
void UIDeleteGame(UInt16 index);
int  UILoadGame(UInt16 index);
void UINewGame(void);

char * pArray[50];
short int savegame_index = 0;

void _UIUpdateSaveGameList(void)
{
    DmOpenRef db;
    UInt16 index = 0;
    MemHandle rec;
    unsigned char * pTemp;
    unsigned short int nRec, nsIndex=0;
    FormPtr form;


    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadOnly);
    if (!db) {
        form = FrmGetActiveForm();
        LstSetListChoices(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)), NULL, 0);
        LstDrawList(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)));
        return; // no database
    }
    nRec = DmNumRecords(db);

    for (index=1; index<nRec; index++) {
        rec = DmQueryRecord(db, index);
        if (rec) {
            pArray[nsIndex] = MemPtrNew(20);
            pTemp = (unsigned char*)MemHandleLock(rec);
            strncpy(pArray[nsIndex], (char*)pTemp+100, 20);
            MemHandleUnlock(rec);
            nsIndex++;
        }
    }
    DmCloseDatabase(db);

    // update list
    form = FrmGetActiveForm();
    LstSetListChoices(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)), pArray, nsIndex);
    LstDrawList(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)));
}


void _UICleanSaveGameList(void)
{
    int i,n;
    FormPtr form = FrmGetActiveForm();
    n = LstGetNumberOfItems(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)));
    for (i=0; i<n;i++) {
        MemPtrFree((void*)LstGetSelectionText(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)),i));
    }

}


void _UICreateNewSaveGame(void)
{
    Err err = 0;
    DmOpenRef db;
    MemHandle rec;
    void * pRec;
    
    UInt16 index = dmMaxRecordIndex;

    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadWrite);
    if (!db) {
        err = DmCreateDatabase(0, "PCitySave", 'PCit', 'DATA', false);
        if (err) {
            return; // couldn't create
        }

        db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadWrite);
        if (!db) {
            return; // couldn't open after creation
        }
    }
    // no, this should NOT be an "else if"
    if (db) {
        if (DmNumRecords(db) >= 50) {
            // TODO: alert user - max is 50 savegames
        } else {
            if (DmNumRecords(db) == 0) {
                // create an empty record in slot 0 if it's not there, this is
                // used for autosaves
                rec = DmNewRecord(db,&index, mapsize*mapsize+200);
                if (rec) {
                    DmReleaseRecord(db, index, true);
                }
            }
            index = dmMaxRecordIndex;
            rec = DmNewRecord(db,&index,mapsize*mapsize+200);
            if (rec) {
                pRec = MemHandleLock(rec);
                // write the header and some globals
                DmWrite(pRec,0,"PC00",4);
                DmWrite(pRec,100,(char*)cityname,20);
                MemHandleUnlock(rec);
                DmReleaseRecord(db,index,true);
            }
        }
        DmCloseDatabase(db);
    }
}

int _UILoadNewestFromList(void)
{
    int n;
    FormPtr form = FrmGetActiveForm();
    n = LstGetNumberOfItems(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)));
    LstSetSelection(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)),n - 1);
    UIWriteLog("..");
    { char s[20];
        sprintf(s, "%i\n", n);
        UIWriteLog(s);
    }
    


   return _UILoadFromList(); 
}

int _UILoadFromList(void)
{
    FormPtr form = FrmGetActiveForm();
    int index = LstGetSelection(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)));
    if (index >= 0) {
        return UILoadGame(index+1); // +1 is because the savegame in slot 0 isn't in the list
    } else {
        return 0;
    }
}

void _UIDeleteFromList(void)
{
    FormPtr form = FrmGetActiveForm();
    int index = LstGetSelection(FrmGetObjectPtr(form,FrmGetObjectIndex(form,listID_FilesList)));
    if (index != noListSelection) {
        return UIDeleteGame(index+1); // +1 is because the savegame in slot 0 isn't in the list
    }
}


extern void UIClearAutoSaveSlot(void)
{
    DmOpenRef db;
    MemHandle rec;
    void * pRec;

    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadWrite);
    if (!db) {
            return; // couldn't create
    }
    // no, this should NOT be an "else if"
    rec = DmGetRecord(db,0);
    if (rec) {
        pRec = MemHandleLock(rec);
        DmWrite(pRec,0,"PCNO",4);
        MemHandleUnlock(rec);
        DmReleaseRecord(db,0,true);
    }

    DmCloseDatabase(db);
}

void UINewGame(void)
{
    // reset all vars
    TimeElapsed = 0;
    map_xpos = 50;
    map_ypos = 50;
    credits = 50000;
    memset((void*)&BuildCount[0],0,80);
    memset((void*)objects,0,sizeof(MoveableObject)*NUM_OF_OBJECTS);
    memset((void*)units,0,sizeof(DefenceUnit)*NUM_OF_UNITS);
    upkeep[0] = 100;
    upkeep[1] = 100;
    upkeep[2] = 100;
    mapsize = 100;
    disaster_level = 1; // TODO: = difficulty_level
    ResizeWorld(mapsize*mapsize);
    SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
    CreateFullRiver();
    CreateForests();
    DrawGame(1);
    game_in_progress = 1;
}


extern int UILoadAutoGame(void)
{
    return UILoadGame(0);
}


void UIDeleteGame(UInt16 index)
{
    // saves the game in slot 0
    DmOpenRef db;

    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadWrite);
    if (!db) {
        return; // couldn't open after creation
    }

    if (DmNumRecords(db) < index) {
        return; // index doesn't exist
    }

    DmRemoveRecord(db, index);
    DmCloseDatabase(db);
}



int UILoadGame(UInt16 index)
{
    DmOpenRef db;
    MemHandle rec;
    unsigned char * pTemp;
    short int loaded = 0;

    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadOnly);
    if (!db) {
        return 0; // no database
    }
    rec = DmQueryRecord(db, index);
    if (rec) {
        pTemp = (unsigned char*)MemHandleLock(rec);
        if (StrNCompare("PC00",(char*)pTemp,4) == 0) { // flagged to create new game
            UINewGame();
            UISaveGame(index); // save the newly created map
            loaded = 2;
        } else if (StrNCompare("PC04",(char*)pTemp,4) == 0) { // version check
            LockWorld();
            MemMove(&credits,       pTemp+4,4);
            MemMove(&map_xpos,      pTemp+8,1);
            MemMove(&map_ypos,      pTemp+9,1);
            MemMove(&mapsize,       pTemp+10,1);
            MemMove(&TimeElapsed,   pTemp+11,4);
            MemMove(&upkeep[0],     pTemp+15,3);
            MemMove(&disaster_level,pTemp+18,1);
            //                      pTemp+19,1 is reserved for
            //                      difficulty_level
            MemMove(&BuildCount[0], pTemp+20,80);
            MemMove(cityname,       pTemp+100,20);
            MemMove(&objects,       pTemp+120,sizeof(MoveableObject)*NUM_OF_OBJECTS);
            MemMove(&units,         pTemp+200,sizeof(DefenceUnit)*NUM_OF_UNITS);
            MemMove(worldPtr,       pTemp+300,mapsize*mapsize);
            UnlockWorld();
            // update the power grid:
            Sim_DistributePower();
            loaded = 1;
        } else if (StrNCompare("PCNO",(char*)pTemp,4) == 0) { // flagged to be an empty save game
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

extern void UISaveGameToIndex(void)
{
    UISaveGame(savegame_index);
}

extern void UISaveGame(UInt16 index)
{
    // saves the game in slot 0
    Err err = 0;
    DmOpenRef db;
    MemHandle rec;
    void * pRec;

    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadWrite);
    if (!db) {
        err = DmCreateDatabase(0, "PCitySave", 'PCit', 'DATA', false);
        if (err) {
            return; // couldn't create
        }

        db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadWrite);
        if (!db) {
            return; // couldn't open after creation
        }
    }
    // no, this should NOT be an "else if"
    if (db) {
        if (DmNumRecords(db) > index) {
            DmRemoveRecord(db, index);
        }
        rec = DmNewRecord(db,&index, mapsize*mapsize+300);
        if (rec) {
            pRec = MemHandleLock(rec);
            LockWorld();
            // write the header and some globals
            DmWrite(pRec,0,"PC04",4);
            DmWrite(pRec,4,&credits,4);
            DmWrite(pRec,8,&map_xpos,1);
            DmWrite(pRec,9,&map_ypos,1);
            DmWrite(pRec,10,&mapsize,1);
            DmWrite(pRec,11,&TimeElapsed,4);
            DmWrite(pRec,15,&upkeep[0],3);
            DmWrite(pRec,18,&disaster_level,1);
            //           19,               ,1  is reserved for
            //                                 difficulty_level
            DmWrite(pRec,20,&BuildCount[0],80);
            DmWrite(pRec,100,(char*)cityname,20);
            DmWrite(pRec,120,(void*)&objects, sizeof(MoveableObject)*NUM_OF_OBJECTS); // 8 bytes each
            DmWrite(pRec,200,(void*)&units,   sizeof(DefenceUnit)*NUM_OF_UNITS); // 8 bytes each
            DmWrite(pRec,300,(void*)(unsigned char*)worldPtr,mapsize*mapsize);
            UnlockWorld();
            MemHandleUnlock(rec);
            DmReleaseRecord(db,index,true);
        }
        
        DmCloseDatabase(db);
    }
}




extern Boolean hFilesNew(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType)
    {
        case frmOpenEvent:
            game_in_progress = 0;
            form = FrmGetActiveForm();
            FrmDrawForm(form);
            handled = 1;
            break;
        case frmCloseEvent:
            break;
        case ctlSelectEvent:
            switch (event->data.ctlSelect.controlID)
            {
                case buttonID_FilesNewCreate:
                    UIWriteLog("Create pushed\n");
                    // copy the name to (char*)cityname
                    FrmReturnToForm(0);
                    handled = 1;
                    break;
                case buttonID_FilesNewCancel:
                    UIWriteLog("Cancel pushed\n");
                    // set (char*)cityname to '\0'
                    cityname[0] = '\0';
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


extern Boolean hFiles(EventPtr event)
{
    FormPtr form;
    int handled = 0;
    FormType * ftNewGame;
    char * pGameName;

    switch (event->eType)
    {
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
                    // create new game and add it to the list
                    ftNewGame = FrmInitForm(formID_filesNew);
                    if (FrmDoDialog(ftNewGame) == buttonID_FilesNewCreate) {
                        UIWriteLog("Creating new game\n");
                        // need to fetch the savegame name from the form
                        pGameName = FldGetTextPtr(FrmGetObjectPtr(ftNewGame, FrmGetObjectIndex(ftNewGame, fieldID_newGameName)));
                        if (pGameName != NULL) {
                            strcpy((char*)cityname,pGameName);
                            _UICreateNewSaveGame();
                            _UICleanSaveGameList();
                            _UIUpdateSaveGameList();
                            // and load the game right after creating it
                            if (_UILoadNewestFromList()) {
                                FrmGotoForm(formID_pocketCity);
                            }
                        } else {
                            strcpy((char*)cityname,"");
                            UIWriteLog("No name specified\n");
                        }
                    }
                    FrmDeleteForm(ftNewGame);
                    handled = 1;
                    break;
                case buttonID_FilesLoad:
                    // create new game
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
        case menuEvent:
            switch (event->data.menu.itemID)
            {
                case menuitemID_FilesNew:
                    // create new game and add it to the list
                    _UICreateNewSaveGame();
                    _UICleanSaveGameList();
                    _UIUpdateSaveGameList();
                    handled = 1;
                    break;
                case menuitemID_FilesOpen:
                    // create new game
                    if (_UILoadFromList()) {
                        FrmGotoForm(formID_pocketCity);
                    }
                    handled = 1;
                    break;
                case menuitemID_FilesDelete:
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


