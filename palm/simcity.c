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

UInt32 timeStamp = 0;
short simState = 0;
short DoDrawing = 0;
unsigned short XOFFSET =0;
unsigned short YOFFSET =15;

static Boolean hPocketCity(EventPtr event);
static Boolean hBudget(EventPtr event);
void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth);
void _PalmInit(void);
void UISaveGame(void);
void UILoadGame(void);
void UINewGame(void);

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
        UILoadGame();

        FrmGotoForm(formID_pocketCity);
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
            if (building == 0 && SIM_GAME_LOOP_SECONDS != SPEED_PAUSED) {
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

        UISaveGame();
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
            FrmDrawForm(form);
            handled = 1;
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
                    case menuitemID_Budget:
                        SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
                        FrmGotoForm(formID_budget);
                        handled = 1;
                        break;
                    case menuitemID_about:
                         FrmAlert(alertID_about);
                         handled = 1;
                         break;
                    case menuitemID_newGame:
                         if (FrmAlert(alertID_newGame) == 0) {
                             UINewGame();
                         }
                         handled = 1;
                         break;
                    case menuitemID_loadGame:
                         if (FrmAlert(alertID_loadGame) == 0) {
                             UILoadGame();
			     DrawGame(1);
			     FrmAlert(alertID_gameLoaded);
                         }
                         handled = 1;
                         break;
                    case menuitemID_saveGame:
                         if (FrmAlert(alertID_saveGame) == 0) {
                             UISaveGame();
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



void UISaveGame() 
{
    // saves the game in slot 0
    Err err = 0;
    DmOpenRef db;
    MemHandle rec;
    void * pRec;
    
    UInt16 index = 0;

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
        rec = DmNewRecord(db,&index, mapsize*mapsize+100);
        if (rec) {
            pRec = MemHandleLock(rec);
            LockWorld();
            // write the header and some globals
            DmWrite(pRec,0,"PC01",4);
            DmWrite(pRec,4,&credits,4);
            DmWrite(pRec,8,&map_xpos,1);
            DmWrite(pRec,9,&map_ypos,1);
            DmWrite(pRec,10,&mapsize,1);
            DmWrite(pRec,11,&TimeElapsed,4);
            
            DmWrite(pRec,60,&BuildCount[0],40);
            DmWrite(pRec,100,(void*)(unsigned char*)worldPtr,mapsize*mapsize);
            UnlockWorld();
            MemHandleUnlock(rec);
            DmReleaseRecord(db,index,true);
        }
        
        DmCloseDatabase(db);
    }
}

void UILoadGame(void)
{
    // loads the game in slot 0
    DmOpenRef db;
    UInt16 index = 0;
    MemHandle rec;
    unsigned char * pTemp;

    db = DmOpenDatabaseByTypeCreator('DATA', 'PCit', dmModeReadOnly);
    if (!db) {
        return; // no database
    }
    rec = DmQueryRecord(db, index);
    if (rec) {
        pTemp = (unsigned char*)MemHandleLock(rec);
        if (strncmp("PC01",(char*)pTemp,4) == 0) { // version check
            LockWorld();
            memcpy((void*)&credits,(void*)pTemp+4,4);
            memcpy((void*)&map_xpos,(void*)pTemp+8,1);
            memcpy((void*)&map_ypos,(void*)pTemp+9,1);
            memcpy((void*)&mapsize,(void*)pTemp+10,1);
            memcpy((void*)&TimeElapsed,(void*)pTemp+11,4);

            memcpy((void*)&BuildCount[0],(void*)pTemp+60,40);
            memcpy((void*)worldPtr,(void*)pTemp+100,mapsize*mapsize);
            UnlockWorld();
            // update the power grid:
            Sim_DistributePower();
        } else {
        	FrmAlert(alertID_invalidSaveVersion);
        }
        MemHandleUnlock(rec);
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
    memset((void*)&BuildCount[0],0,40);
    mapsize = 100;
    ResizeWorld(mapsize*mapsize);
    SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
    DrawGame(1);
}
