#include <PalmOS.h>
#include <SysEvtMgr.h>
#include <StringMgr.h>
#include <KeyMgr.h>
#include "simcity.h"
#include "../source/zakdef.h"
#include "../source/ui.h"
#include "../source/drawing.h"
#include "../source/build.h"
#include "../source/handler.h"
#include "../source/globals.h"
#include "../source/simulation.h"
#include <string.h>

MemHandle worldHandle;
MemHandle worldFlagsHandle;
MemPtr worldPtr;
MemPtr worldFlagsPtr;
RectangleType rPlayGround;
unsigned char nSelectedBuildItem = 0;
unsigned char nPreviousBuildItem = 0;

short int lowShown = 0;
short int noShown = 0;
short int oldROM = 0;
short int building = 0;
short int game_in_progress = 0;
short int savegame_index = 0;

UInt32 timeStamp = 0;
short simState = 0;
short DoDrawing = 0;
unsigned short XOFFSET =0;
unsigned short YOFFSET =15;

static Boolean hPocketCity(EventPtr event);
static Boolean hBudget(EventPtr event);
static Boolean hMap(EventPtr event);
static Boolean hFiles(EventPtr event);
void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth);
void _UIUpdateSaveGameList(void);
void _UICreateNewSaveGame(void);
void _UICleanSaveGameList(void);
void _UIDeleteFromList(void);
int  _UILoadFromList(void);
void _UIClearAutoSaveSlot(void);
void _PalmInit(void);
void UISaveGame(UInt16 index);
void UIDeleteGame(UInt16 index);
int  UILoadGame(UInt16 index);
int  UILoadAutoGame(void);
void UINewGame(void);
void DrawMap(void);

void BudgetInit(void);
void BudgetFreeMem(void);
void _UIGetFieldToBuildOn(int x, int y);
Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags);

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    EventType event;
    short err;
    FormPtr form;
    int formID;
    UInt32 timeTemp;
    UInt16 error;

    error = RomVersionCompatible (0x03503000, launchFlags);
    if (error) return (error);

    if (cmd == sysAppLaunchCmdNormalLaunch) {
        _PalmInit();
        PCityMain();
        if (UILoadAutoGame()) {
            FrmGotoForm(formID_pocketCity);
        } else {
            FrmGotoForm(formID_files);
        }
        
        do {
            EvtGetEvent(&event, 1);

            if (event.eType == keyDownEvent) {
                if (FrmDispatchEvent(&event)) continue;
            }

            if (SysHandleEvent(&event)) continue;

            if (MenuHandleEvent((void*)0, &event, &err)) continue;

            if (event.eType == frmLoadEvent) {
                formID = event.data.frmLoad.formID;
                form = FrmInitForm(formID);
                FrmSetActiveForm(form);

                switch (formID)
                {
                    case formID_pocketCity:
                        FrmSetEventHandler(form, hPocketCity);
                        break;
                    case formID_budget:
                        FrmSetEventHandler(form, hBudget);
                        break;
                    case formID_map:
                        FrmSetEventHandler(form, hMap);
                        break;
                    case formID_files:
                        FrmSetEventHandler(form, hFiles);
                        break;
                }
            }
            else if (event.eType == winExitEvent) {
                if (event.data.winExit.exitWindow == (WinHandle) FrmGetFormPtr(formID_pocketCity)) {
                    DoDrawing = 0;
                }
            }
            else if (event.eType == winEnterEvent) {
                if (event.data.winEnter.enterWindow == (WinHandle) FrmGetFormPtr(formID_pocketCity) && event.data.winEnter.enterWindow == (WinHandle) FrmGetFirstForm())
                {
                    DoDrawing = 1;
                }
            }

            if (FrmDispatchEvent(&event)) continue;

            // the almighty homemade >>"multithreader"<<
            if (game_in_progress == 1 && building == 0 && SIM_GAME_LOOP_SECONDS != SPEED_PAUSED) {
                if (simState == 0) {
                    timeTemp = TimGetSeconds();
                    if (timeTemp >= timeStamp+SIM_GAME_LOOP_SECONDS)
                    {
                        simState = 1;
                        timeStamp = timeTemp;
                    }
                } else  {
                    simState = Sim_DoPhase(simState);
                    if (simState == 0) {
                        RedrawAllFields();
                    }
                }
            }


        } while (event.eType != appStopEvent);

        if (game_in_progress) {
            UISaveGame(0);
        }
    }

    return 0;
}


void _PalmInit(void)
{
    UInt32 depth;
    timeStamp = TimGetSeconds();

    rPlayGround.topLeft.x = 0;
    rPlayGround.topLeft.y = 15;
    rPlayGround.extent.x = 16*10;
    rPlayGround.extent.y = 16*8;

    // set screen mode to colors if supported
    if (oldROM != 1) {  // must be v3.5+ for some functions in here
        WinScreenMode(winScreenModeGetSupportedDepths, 0, 0, &depth, 0);
        if ((depth & 0x80) != 0 ) {
            // 8pps is supported
            depth = 8;
            WinScreenMode(winScreenModeSet,0,0,&depth,0);
        }
    }


}



static Boolean hBudget(EventPtr event)
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
            BudgetFreeMem();
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


static Boolean hMap(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType)
    {
        case penDownEvent:
            if (event->screenX >= 1  && event->screenX <= mapsize+1 &&
                event->screenY >= 17 && event->screenY <= mapsize+17) {
                map_xpos = event->screenX-1-(visible_x/2);
                map_ypos = event->screenY-17-(visible_y/2);
                if (map_ypos < 0) { map_ypos = 0; }
                if (map_ypos > mapsize-visible_y) { map_ypos = mapsize - visible_y; }
                if (map_xpos < 0) { map_xpos = 0; }
                if (map_xpos > mapsize-visible_x) { map_xpos = mapsize - visible_x; }
                FrmGotoForm(formID_pocketCity);
                handled = 1;
            }
            // check for other 'penclicks' here
            break;
        case frmOpenEvent:
            form = FrmGetActiveForm();
            FrmDrawForm(form);
            DrawMap();
            handled = 1;
            break;
        case frmCloseEvent:
            break;
        case menuEvent:
            switch (event->data.menu.itemID)
            {
                case menuitemID_MapBack:
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

void DrawMap(void)
{
    int i,j;

    LockWorld();
    UILockScreen();
    _UIDrawRect(17,1,100,100);
    
    if (!oldROM) {
        for(i=0; i<mapsize; i++) {
            for(j=0; j<mapsize; j++) {
                if (GetWorld(WORLDPOS(i,j)) != TYPE_DIRT) {
                    WinDrawPixel(i+1,j+17);
                }
            }
        }
    }

    UIUnlockScreen();
    UnlockWorld();
}



static Boolean hFiles(EventPtr event)
{
    FormPtr form;
    int handled = 0;

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
                    _UICreateNewSaveGame();
                    _UICleanSaveGameList();
                    _UIUpdateSaveGameList();
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
    sprintf(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_res)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_COMMERCIAL]*INCOME_COMMERCIAL*tax/100;
    cashflow += change;
    sprintf(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_com)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_INDUSTRIAL]*INCOME_INDUSTRIAL*tax/100;
    cashflow += change;
    sprintf(temp,"%lu",  change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_ind)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_ROADS]*UPKEEP_ROAD;
    cashflow -= change;
    sprintf(temp,"%lu", change);
    CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tra)), temp);

    temp = MemPtrNew(12);
    change = BuildCount[COUNT_POWERLINES]*UPKEEP_POWERLINE +
             BuildCount[COUNT_NUCLEARPLANTS]*UPKEEP_NUCLEARPLANT +
             BuildCount[COUNT_POWERPLANTS]*UPKEEP_POWERPLANT;
    cashflow -= change;
    sprintf(temp,"%lu", change);
   CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_pow)), temp);


   temp = MemPtrNew(12);
   sprintf(temp,"%+li", cashflow);
   CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_tot)), temp);

   temp = MemPtrNew(12);
   sprintf(temp,"%li", credits+cashflow);
   CtlSetLabel(FrmGetObjectPtr(form,FrmGetObjectIndex(form, labelID_budget_bal)), temp);

   temp = MemPtrNew(12);
   sprintf(temp,"%li", credits);
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
}

static Boolean hPocketCity(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType)
    {
        case frmOpenEvent:
            form = FrmGetActiveForm();
            FrmDrawForm(form);
            DrawGame(1);
            handled = 1;
            game_in_progress = 1;
            break;
        case frmCloseEvent:
            break;
        case penDownEvent:
            if (RctPtInRectangle(event->screenX, event->screenY, &rPlayGround)) {
                // click was on the playground
                _UIGetFieldToBuildOn(event->screenX, event->screenY);
            } else if (event->screenX >= 150 && event->screenY >= 150) {
                // click was on change speed
                switch (SIM_GAME_LOOP_SECONDS)
                {
                    case SPEED_PAUSED:
                        SIM_GAME_LOOP_SECONDS = SPEED_MEDIUM;
                        break;
                    default:
                        SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
                        break;
                }
                UIDrawPop();
            } else if (event->screenX >= 140 && event->screenY >= 150) {
                // click was on change production
                if (nSelectedBuildItem == BUILD_BULLDOZER) {
                    nSelectedBuildItem = nPreviousBuildItem;
                } else {
                    nPreviousBuildItem = nSelectedBuildItem;
                    nSelectedBuildItem = BUILD_BULLDOZER;
                }
                UIUpdateBuildIcon();
            }
            // check for other 'penclicks' here
            break;
        case penMoveEvent:
            if (RctPtInRectangle(event->screenX, event->screenY, &rPlayGround))
            {
                _UIGetFieldToBuildOn(event->screenX, event->screenY);
                handled = 1;
                building = 1;
            }
            break;
        case penUpEvent:
            building = 0;
            handled = 1;
            break;
        case menuEvent:
            if (event->data.menu.itemID >= menuitemID_buildBulldoze)
            {
                nSelectedBuildItem = event->data.menu.itemID - menuitemID_buildBulldoze;
                UIUpdateBuildIcon();
                handled = 1;
            } else {
                switch (event->data.menu.itemID)
                {
                    case menuitemID_Funny:
                        handled = 1;
                        break;
                    case menuitemID_Map:
                        SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
                        FrmGotoForm(formID_map);
                        handled = 1;
                        break;
                    case menuitemID_Budget:
                        SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
                        FrmGotoForm(formID_budget);
                        handled = 1;
                        break;
                    case menuitemID_about:
                         FrmAlert(alertID_about);
                         handled = 1;
                         break;
                    case menuitemID_loadGame:
                         switch(FrmAlert(alertID_loadGame))
                         {
                             case 0: // save game
                                 UISaveGame(savegame_index);
                                 _UIClearAutoSaveSlot();
                                 FrmGotoForm(formID_files);
                                 break;
                             case 1: // don't save
                                 _UIClearAutoSaveSlot();
                                 FrmGotoForm(formID_files);
                                 break;
                             case 2: // cancel
                                 break;
                         }
                         handled = 1;
                         break;
                    case menuitemID_saveGame:
                         if (FrmAlert(alertID_saveGame) == 0) {
                             UISaveGame(savegame_index);
                             FrmAlert(alertID_gameSaved);
                         }
                         handled = 1;
                         break;
                    case menuID_SlowSpeed:
                        SIM_GAME_LOOP_SECONDS = SPEED_SLOW;
                        UIDrawPop();
                        handled = 1;
                        break;
                    case menuID_MediumSpeed:
                        SIM_GAME_LOOP_SECONDS = SPEED_MEDIUM;
                        UIDrawPop();
                        handled = 1;
                        break;
                    case menuID_FastSpeed:
                        SIM_GAME_LOOP_SECONDS = SPEED_FAST;
                        UIDrawPop();
                        handled = 1;
                        break;
                    case menuID_TurboSpeed:
                        SIM_GAME_LOOP_SECONDS = SPEED_TURBO;
                        UIDrawPop();
                        handled = 1;
                        break;
                    case menuID_PauseSpeed:
                        SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
                        UIDrawPop();
                        handled = 1;
                        break;

                }
            }
            break;
        case keyDownEvent:
            switch (event->data.keyDown.chr)
            {
                case pageUpChr:
                    /* scroll map up */
                    ScrollMap(0);
                    handled = 1;
                    break;
                case pageDownChr:
                    /* scroll map down */
                    ScrollMap(2);
                    handled = 1;
                    break;
                case vchrHard2:
                    /* scroll map left */
                    ScrollMap(3);
                    handled = 1;
                    break;
                case vchrHard3:
                    /* scroll map right */
                    ScrollMap(1);
                    handled = 1;
                    break;
            }
        default:
            break;
    }

    return handled;
}

extern unsigned char UIGetSelectedBuildItem(void) { return nSelectedBuildItem; }


void _UIGetFieldToBuildOn(int x, int y)
{
    RectangleType rect;
    int i,j;
    rect.extent.x = TILE_SIZE;
    rect.extent.y = TILE_SIZE;

    for (i=0; i<visible_x; i++)
    {
        for (j=0; j<visible_y; j++)
        {
            rect.topLeft.x = XOFFSET+i*TILE_SIZE;
            rect.topLeft.y = YOFFSET+j*TILE_SIZE;
            if (RctPtInRectangle(x,y,&rect))
            {
                BuildSomething(i+map_xpos,j+map_ypos);
                return;
            }
        }
    }
}





extern int UIDisplayError(int nError)
{
    switch (nError)
    {
        case ERROR_OUT_OF_MEMORY: FrmAlert(alertID_errorOutOfMemory); break;
        case ERROR_OUT_OF_MONEY: FrmAlert(alertID_outMoney); break;
        case ERROR_FIRE_OUTBREAK: FrmAlert(alertID_fireOutBreak); break;
        case ERROR_PLANT_EXPLOSION: FrmAlert(alertID_plantExplosion); break;
    }
    return 0;
}


extern void UIInitDrawing(void) {};
extern void UIFinishDrawing(void) {};

// by using these two APIs we can get faster, flickerless allscreen updating
extern void UILockScreen(void)
{
    if (oldROM != 1) {
        WinScreenLock(winLockCopy); 
    }
}

extern void UIUnlockScreen(void)
{
    if (oldROM != 1) {
        WinScreenUnlock();
    }
}

void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth)
{
    // draws a rect on screen: note, the rect within the border will be exactly nHeight*nWidth pxls
    // the frame's left border will be at nTop-1 and so on

    RectangleType rect;

    if (DoDrawing == 0) { return; }

    rect.topLeft.x = nLeft;
    rect.topLeft.y = nTop;
    rect.extent.x = nWidth;
    rect.extent.y = nHeight;

    WinDrawRectangleFrame(1, &rect);
}

extern void UIDrawBorder()
{
    if (DoDrawing == 0) return;

    // border
    _UIDrawRect(YOFFSET,XOFFSET,visible_y*TILE_SIZE,visible_x*TILE_SIZE);
}


extern void UISetUpGraphic(void)
{
}

extern void UIDrawCursor(int xpos, int ypos)
{
}

extern void UIDrawPowerLoss(int xpos, int ypos)
{
    MemHandle  bitmaphandle;
    BitmapPtr  bitmap;
    int overlayID = bitmapID_PowerLossOverlay;
    int powerID = bitmapID_PowerLoss;

    if (DoDrawing == 0) { return; }
    if (oldROM == 1) { return; } // must be v3.5+ for some functions in here

    WinPushDrawState();


    //	if (TILE_SIZE == 32) { overlayID = bitmapID_PowerLossOverlay; powerID = bitmapID_PowerLoss; };
    if (TILE_SIZE == 16) { overlayID = bitmapID_PowerLossOverlay2; powerID = bitmapID_PowerLoss2; };

    // first draw the overlay
    WinSetDrawMode(winErase);
    bitmaphandle = DmGet1Resource( TBMP, overlayID);
    bitmap = MemHandleLock(bitmaphandle);
    WinPaintBitmap(bitmap, xpos*TILE_SIZE+XOFFSET, ypos*TILE_SIZE+YOFFSET);
    MemHandleUnlock(bitmaphandle);

    // now draw the 's'
    WinSetDrawMode(winOverlay);
    bitmaphandle = DmGet1Resource( TBMP , powerID);
    bitmap = MemHandleLock(bitmaphandle);
    WinPaintBitmap(bitmap, xpos*TILE_SIZE+XOFFSET, ypos*TILE_SIZE+YOFFSET);
    MemHandleUnlock(bitmaphandle);


    WinPopDrawState();
}


extern void UIDrawField(int xpos, int ypos, unsigned char nGraphic)
{
    MemHandle  bitmaphandle;
    BitmapPtr  bitmap;
    int startID = 0;

    if (DoDrawing == 0) { return; }

    //	if (TILE_SIZE == 32) { startID = bitmapID_DirtBmp; };
    if (TILE_SIZE == 16) { startID = bitmapID_DirtBmp2; };

    bitmaphandle = DmGet1Resource( TBMP , nGraphic + startID);
    bitmap = MemHandleLock(bitmaphandle);
    WinDrawBitmap(bitmap, xpos*TILE_SIZE+XOFFSET, ypos*TILE_SIZE+YOFFSET);
    MemHandleUnlock(bitmaphandle);
}


extern void UIScrollMap(int direction)
{
    WinHandle screen;
    RectangleType rect;
    int to_x, to_y, i;

    UILockScreen();

    rect.topLeft.x = XOFFSET + TILE_SIZE*(direction == 1);
    rect.topLeft.y = YOFFSET + TILE_SIZE*(direction == 2);
    rect.extent.x = (visible_x - 1*(direction == 1 || direction == 3 )) * TILE_SIZE;
    rect.extent.y = (visible_y - 1*(direction == 0 || direction == 2 )) * TILE_SIZE;
    to_x = XOFFSET + TILE_SIZE*(direction == 3);
    to_y = YOFFSET + TILE_SIZE*(direction == 0);
    

    screen = WinGetActiveWindow();
    WinCopyRectangle(screen, screen, &rect,to_x, to_y, winPaint);

    // and lastly, fill the gap
    LockWorld();
    LockWorldFlags();
    UIInitDrawing();
    
    if (direction == 1 || direction == 3) {
        for (i=map_ypos; i<visible_y+map_ypos; i++) {
            DrawFieldWithoutInit(map_xpos+(visible_x-1)*(direction == 1),i);
        }
    } else {
        for (i=map_xpos; i<visible_x+map_xpos; i++) {
            DrawFieldWithoutInit(i,map_ypos+(visible_y-1)*(direction == 2));
        }
    }

    UIDrawCursor(cursor_xpos-map_xpos, cursor_ypos-map_ypos);
    UIDrawCredits();
    UIDrawPop();

    UIUnlockScreen();
    UIFinishDrawing();
    UnlockWorldFlags();
    UnlockWorld();
    

    
}


extern unsigned long GetRandomNumber(unsigned long max)
{
    if (max == 0) return 0;
    return (UInt16)SysRandom(0) % (UInt16)max;
}



extern void UIDrawCredits(void)
{
    char temp[23];
    RectangleType rect;

    if (DoDrawing == 0) { return; }

    StrPrintF(temp, "$: %ld", credits);

    rect.topLeft.x = 66;
    rect.topLeft.y = 1;
    rect.extent.x = 94;
    rect.extent.y = 11;

    WinEraseRectangle(&rect,0);

    WinDrawChars((char*)temp,StrLen(temp),66,1);
    GetDate((char*)temp);
    WinDrawChars((char*)temp,StrLen(temp),120,1);
}

extern void UIUpdateBuildIcon(void)
{
    MemHandle bitmaphandle;
    BitmapPtr bitmap;
    if (DoDrawing == 0) { return; }

    bitmaphandle = DmGet1Resource(TBMP,bitmapID_iconBulldoze + nSelectedBuildItem);
    bitmap = MemHandleLock(bitmaphandle);
    WinDrawBitmap(bitmap,140,150);
    MemHandleUnlock(bitmaphandle);
}


extern void UIDrawPop(void)
{
    char temp[50];
    MemHandle  bitmaphandle;
    BitmapPtr  bitmap;
    RectangleType rect;

    if (DoDrawing == 0) { return; }


    StrPrintF(temp, "(%02u,%02u) Pop: %-9li",
            map_xpos,
            map_ypos,
            (BuildCount[COUNT_RESIDENTIAL]*150));


    rect.topLeft.x = 3;
    rect.topLeft.y = 148;
    rect.extent.x = 137;
    rect.extent.y = 11;

    WinEraseRectangle(&rect,0);
    WinDrawChars((char*)temp,StrLen(temp),3,148);


    bitmaphandle = DmGet1Resource( TBMP, bitmapID_SpeedPaused + SIM_GAME_LOOP_SECONDS);
    bitmap = MemHandleLock(bitmaphandle);
    WinDrawBitmap(bitmap, 150,150);
    MemHandleUnlock(bitmaphandle);
}

extern void UICheckMoney(void)
{
    if(credits == 0) {
        if(noShown == 0) {
            FrmAlert(alertID_outMoney);
            noShown = 1;
        } else {
            return;
        }
    } else if((credits <= 1000) || (credits == 1000)) {
        if(lowShown==0) {
            FrmAlert(alertID_lowFunds);
            lowShown=1;
        } else {
            return;
        }
    }
}





/*** memory handlers ***/
extern int InitWorld(void)
{
    worldHandle = MemHandleNew(10);
    worldFlagsHandle = MemHandleNew(10);

    if (worldHandle == 0 || worldFlagsHandle == 0)
    {
        UIDisplayError(0);
        return 0;
    }
    return 1;
}


extern int ResizeWorld(long unsigned size)
{
    int i;

    if (MemHandleResize(worldHandle, size) != 0 || MemHandleResize(worldFlagsHandle, size) != 0)
    {
        UIDisplayError(0);
        //QuitGameError();
        return 0;
    }

    LockWorld();
    LockWorldFlags();

    for (i=0; i<size; i++) { *(((unsigned char*)worldPtr)+i) = 0; }
    for (i=0; i<size; i++) { *(((unsigned char*)worldFlagsPtr)+i) = 0; }

    UnlockWorld();
    UnlockWorldFlags();

    return 1;
}


extern void LockWorld() { worldPtr = MemHandleLock(worldHandle); }
extern void UnlockWorld() { MemHandleUnlock(worldHandle); }
extern void LockWorldFlags() { worldFlagsPtr = MemHandleLock(worldFlagsHandle); }
extern void UnlockWorldFlags() { MemHandleUnlock(worldFlagsHandle); }



extern unsigned char GetWorldFlags(long unsigned int pos)
{
    // NOTE: LockWorld() MUST have been called before this is used!!!
    if (pos > (mapsize*mapsize)) { return 0; }
    return ((unsigned char*)worldFlagsPtr)[pos];
}

extern void SetWorldFlags(long unsigned int pos, unsigned char value)
{
    if (pos > mapsize*mapsize) { return; }
    ((unsigned char*)worldFlagsPtr)[pos] = value;
}


extern unsigned char GetWorld(unsigned long pos)
{
    // NOTE: LockWorld() MUST have been called before this is used!!!
    if (pos > (mapsize*mapsize)) { return 0; }
    return ((unsigned char*)worldPtr)[pos];
}

extern void SetWorld(unsigned long pos, unsigned char value)
{
    if (pos > mapsize*mapsize) { return; }
    ((unsigned char*)worldPtr)[pos] = value;
}


/*** end ***/














Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32 romVersion;
    char temp[20];

    // See if we're on in minimum required version of the ROM or later.
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    LongToString(romVersion,(char*)temp);

    if (romVersion < requiredVersion)
    {
        if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            if (FrmAlert (alertID_RomIncompatible) == 1)
            { // try anyway
                oldROM = 1;
                return (0);
            }

            // Pilot 1.0 will continuously relaunch this app unless we switch to 
            // another safe one.
            if (romVersion < 0x02000000)
            {
                AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
            }
        }

        return (sysErrRomIncompatible);
    }

    return (0);
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

void UISaveGame(UInt16 index) 
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
        rec = DmNewRecord(db,&index, mapsize*mapsize+200);
        if (rec) {
            pRec = MemHandleLock(rec);
            LockWorld();
            // write the header and some globals
            DmWrite(pRec,0,"PC03",4);
            DmWrite(pRec,4,&credits,4);
            DmWrite(pRec,8,&map_xpos,1);
            DmWrite(pRec,9,&map_ypos,1);
            DmWrite(pRec,10,&mapsize,1);
            DmWrite(pRec,11,&TimeElapsed,4);
            
            DmWrite(pRec,20,&BuildCount[0],80);
            DmWrite(pRec,100,"Old savegame\0",13);
            DmWrite(pRec,200,(void*)(unsigned char*)worldPtr,mapsize*mapsize);
            UnlockWorld();
            MemHandleUnlock(rec);
            DmReleaseRecord(db,index,true);
        }
        
        DmCloseDatabase(db);
    }
}

int UILoadAutoGame(void)
{
    return UILoadGame(0);
}

int UILoadGame(UInt16 index)
{
    DmOpenRef db;
    MemHandle rec;
    unsigned char * pTemp;
    short int loaded = 0;
    char name[20];

    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadOnly);
    if (!db) {
        return 0; // no database
    }
    rec = DmQueryRecord(db, index);
    if (rec) {
        pTemp = (unsigned char*)MemHandleLock(rec);
        if (strncmp("PC03",(char*)pTemp,4) == 0) { // version check
            LockWorld();
            memcpy((void*)&credits,(void*)pTemp+4,4);
            memcpy((void*)&map_xpos,(void*)pTemp+8,1);
            memcpy((void*)&map_ypos,(void*)pTemp+9,1);
            memcpy((void*)&mapsize,(void*)pTemp+10,1);
            memcpy((void*)&TimeElapsed,(void*)pTemp+11,4);

            memcpy((void*)&BuildCount[0],(void*)pTemp+20,80);
            memcpy((void*)name, (void*)pTemp+100,20);
            memcpy((void*)worldPtr,(void*)pTemp+200,mapsize*mapsize);
            UnlockWorld();
            // update the power grid:
            Sim_DistributePower();
            loaded = 1;
        } else if (strncmp("PC00",(char*)pTemp,4) == 0) { // flagged to create new game
            UINewGame();
            UISaveGame(index); // save the newly created map
            loaded = 2;
        } else if (strncmp("PCNO",(char*)pTemp,4) == 0) { // flagged to be an empty save game
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


void UINewGame(void)
{
    // reset all vars
    TimeElapsed = 0;
    map_xpos = 50;
    map_ypos = 50;
    credits = 50000;
    memset((void*)&BuildCount[0],0,40);
    mapsize = 100;
    ResizeWorld(mapsize*mapsize);
    SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
    CreateFullRiver();
    CreateForests();
    DrawGame(1);
    game_in_progress = 1;
}

char * pArray[50];

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
    // saves the game in a new slot
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
                DmWrite(pRec,100,"New game\0",9);
                MemHandleUnlock(rec);
                DmReleaseRecord(db,index,true);
            }
        }
        DmCloseDatabase(db);
    }
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

void _UIClearAutoSaveSlot(void)
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
