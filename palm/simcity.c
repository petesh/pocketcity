#include <PalmOS.h>
#include <SysEvtMgr.h>
#include <StringMgr.h>
#include <KeyMgr.h>
#include <StdIOPalm.h>
#include "simcity.h"
#include "savegame.h"
#include "../source/zakdef.h"
#include "../source/ui.h"
#include "../source/drawing.h"
#include "../source/build.h"
#include "../source/handler.h"
#include "../source/globals.h"
#include "../source/simulation.h"
#include "../source/disaster.h"

#ifdef DEBUG
#include <HostControl.h>
#endif

MemHandle worldHandle;
MemHandle worldFlagsHandle;
MemPtr worldPtr;
MemPtr worldFlagsPtr;
RectangleType rPlayGround;
WinHandle winZones;
WinHandle winMonsters;
WinHandle winUnits;
unsigned char nSelectedBuildItem = 0;
unsigned char nPreviousBuildItem = 0;
short int game_in_progress = 0;

short int lowShown = 0;
short int noShown = 0;
short int oldROM = 0;
short int building = 0;

UInt32 timeStamp = 0;
UInt32 timeStampDisaster = 0;
short simState = 0;
short DoDrawing = 0;
unsigned short XOFFSET =0;
unsigned short YOFFSET =15;

static Boolean hPocketCity(EventPtr event);
static Boolean hBudget(EventPtr event);
static Boolean hMap(EventPtr event);
static Boolean hQuickList(EventPtr event);
static Boolean hExtraList(EventPtr event);
void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth);
void _PalmInit(void);
void DrawMap(void);
void UIDoQuickList(void);
void UIPopUpExtraBuildList(void);

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
    UIWriteLog("Starting Pocket City\n");

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
                UIWriteLog("keydown event\n");
                if (FrmDispatchEvent(&event)) continue;
            }

            if (SysHandleEvent(&event)) continue;

            if (MenuHandleEvent((void*)0, &event, &err)) continue;

            if (event.eType == frmLoadEvent) {
                UIWriteLog("Main::frmLoadEvent\n");
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
                    case formID_filesNew:
                        FrmSetEventHandler(form, hFilesNew);
                        break;
                    case formID_quickList:
                        FrmSetEventHandler(form, hQuickList);
                        break;
                    case formID_extraBuild:
                        FrmSetEventHandler(form, hExtraList);
                        break;
                }
            }
            else if (event.eType == winExitEvent) {
                if (event.data.winExit.exitWindow == (WinHandle) FrmGetFormPtr(formID_pocketCity)) {
                    UIWriteLog("Setting drawing to 0\n");
                    DoDrawing = 0;
                }
            }
            else if (event.eType == winEnterEvent) {
                if (event.data.winEnter.enterWindow == (WinHandle) FrmGetFormPtr(formID_pocketCity) && event.data.winEnter.enterWindow == (WinHandle) FrmGetFirstForm())
                {
                    UIWriteLog("Setting drawing to 1\n");
                    DoDrawing = 1;
                    RedrawAllFields(); //update screen after menu etc.
                } else {
                    UIWriteLog("Setting drawing to 0\n");
                    DoDrawing = 0;
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

            // Do a disaster update every 2 secound nomatter what.
            // We don't want the (l)user to get help by pausing the game
            // (not realistic enough) - TODO: Pause should be enabled on
            // 'easy' difficulty.
            if (game_in_progress == 1 && building == 0) {
                timeTemp = TimGetSeconds();
                if (timeTemp >= timeStampDisaster+SIM_GAME_LOOP_DISASTER) {
                    MoveAllObjects();
                    if (UpdateDisasters()) {
                        RedrawAllFields();
                    }
                    timeStampDisaster = timeTemp;
                    // TODO: This would be the place to create animation...
                    // perhaps with a second offscreen window for the
                    // second animation frame of each zone
                    // Then swap the winZones between the two,
                    // and do the RedrawAllFields() from above here
                }
            }
            


        } while (event.eType != appStopEvent);

        if (game_in_progress) {
            UISaveGame(0);
        }
        // clean up
        WinDeleteWindow(winZones,0);
        WinDeleteWindow(winMonsters,0);
        WinDeleteWindow(winUnits,0);
    }

    return 0;
}


void _PalmInit(void)
{
    UInt32 depth;
    UInt16 err;
    MemHandle  bitmaphandle;
    BitmapPtr  bitmap;
    WinHandle  winHandle;
    
    timeStamp = TimGetSeconds();
    timeStampDisaster = timeStamp;

    rPlayGround.topLeft.x = 0;
    rPlayGround.topLeft.y = 15;
    rPlayGround.extent.x = 16*10;
    rPlayGround.extent.y = 16*8;

    // set screen mode to colors if supported
    if (oldROM != 1) {  // must be v3.5+ for some functions in here
        WinScreenMode(winScreenModeGetSupportedDepths, 0, 0, &depth, 0);
        if ((depth & 0x80) != 0 ) {
            // 8bpp (color) is supported
            depth = 8;
            WinScreenMode(winScreenModeSet,0,0,&depth,0);
        } else if ((depth & 0x08) != 0) {
            // 4bpp (greyscale) is supported
            depth = 4;
            WinScreenMode(winScreenModeSet,0,0,&depth,0);
        }
    }

    // create an offscreen window, and copy the zones.bmp
    // to be used later
    bitmaphandle = DmGet1Resource( TBMP , bitmapID_zones);
    bitmap = MemHandleLock(bitmaphandle);
    winZones = WinCreateOffscreenWindow(1024, 32, genericFormat,&err); //space for 64*2=128 zones
    if (err != errNone) {
        // TODO: alert user, and quit program
        UIWriteLog("Offscreen window for zones failed\n");
    }
    winHandle = WinSetDrawWindow(winZones);
    // draw the bitmap into the offscreen window
    WinDrawBitmap(bitmap, 0, 0);
    MemHandleUnlock(bitmaphandle);


    // now, ditto for the monsters
    bitmaphandle = DmGet1Resource( TBMP, bitmapID_monsters);
    bitmap = MemHandleLock(bitmaphandle);
    winMonsters = WinCreateOffscreenWindow(128,64,genericFormat,&err);
    if (err != errNone) {
        // TODO: alert user, and quit program
        UIWriteLog("Offscreen window for monsters failed\n");
    }
    WinSetDrawWindow(winMonsters); // note we don't save the old winhandle here
    WinDrawBitmap(bitmap,0,0);
    MemHandleUnlock(bitmaphandle);
    
    // and, at last, the units
    bitmaphandle = DmGet1Resource( TBMP, bitmapID_units);
    bitmap = MemHandleLock(bitmaphandle);
    winUnits = WinCreateOffscreenWindow(48,32,genericFormat,&err);
    if (err != errNone) {
        // TODO: alert user, and quit program
        UIWriteLog("Offscreen window for units failed\n");
    }
    WinSetDrawWindow(winUnits); // note we don't save the old winhandle here
    WinDrawBitmap(bitmap,0,0);
    MemHandleUnlock(bitmaphandle);
    
    // clean up 
    WinSetDrawWindow(winHandle);
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
            timeStamp = TimGetSeconds()-SIM_GAME_LOOP_SECONDS+2; 
                    // so the simulation routine won't kick in right away
            timeStampDisaster = timeStamp-SIM_GAME_LOOP_DISASTER+1; // ditto
            handled = 1;
            break;
        case menuEvent:
            if (event->data.menu.itemID >= menuitemID_buildBulldoze)
            {
                if (event->data.menu.itemID == menuitemID_buildExtra) {
                    UIPopUpExtraBuildList();
                } else {
                    nSelectedBuildItem = event->data.menu.itemID - menuitemID_buildBulldoze;
                    UIUpdateBuildIcon();
                }
                handled = 1;
            } else {
                switch (event->data.menu.itemID)
                {
                    case menuitemID_Funny:
                        // change this to whatever testing you're doing ;)
                        // just handy with a 'trigger' button for testing
                        // ie. disaters
                        BurnField(5,5,1);
			MeteorDisaster();
                        handled = 1;
                        break;
                    case menuitemID_removeDefence:
                        RemoveAllDefence();
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
                                 UISaveGameToIndex();
                                 UIClearAutoSaveSlot();
                                 FrmGotoForm(formID_files);
                                 break;
                             case 1: // don't save
                                 UIClearAutoSaveSlot();
                                 FrmGotoForm(formID_files);
                                 break;
                             case 2: // cancel
                                 break;
                         }
                         handled = 1;
                         break;
                    case menuitemID_saveGame:
                         if (FrmAlert(alertID_saveGame) == 0) {
                             UISaveGameToIndex();
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
                case vchrCalc:
                    /* popup a quicksheet */
                    if (!oldROM) {
                        UIDoQuickList();
                        handled = 1;
                    }
                    break;
                case vchrFind:
                    /* goto map */
                    SIM_GAME_LOOP_SECONDS = SPEED_PAUSED;
                    FrmGotoForm(formID_map);
                    break;
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

void UIPopUpExtraBuildList(void)
{
    FormType * ftList;

    if (oldROM != 1) {
        ftList = FrmInitForm(formID_extraBuild);
        FrmSetEventHandler(ftList, hExtraList);
        if (FrmDoDialog(ftList) == buttonID_extraBuildSelect) {
            nSelectedBuildItem = LstGetSelection(FrmGetObjectPtr(ftList,FrmGetObjectIndex(ftList,listID_extraBuildList)));
        }
        UIUpdateBuildIcon();
        FrmDeleteForm(ftList);
    }
}

static Boolean hExtraList(EventPtr event)
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
        default:
            break;
    }

    return handled;
}

void UIDoQuickList(void)
{
    FormType * ftList;

    if (oldROM != 1) {
        ftList = FrmInitForm(formID_quickList);
        FrmSetEventHandler(ftList, hQuickList);
        nSelectedBuildItem = FrmDoDialog(ftList) - menuitemID_buildBulldoze;
        if (nSelectedBuildItem == 10) {
            UIPopUpExtraBuildList();
        }            
        UIUpdateBuildIcon();
        FrmDeleteForm(ftList);
    }
}
                            

static Boolean hQuickList(EventPtr event)
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
        case keyDownEvent:
            UIWriteLog("Key down\n");
            switch (event->data.keyDown.chr)
            {
                case vchrCalc:
                    /* close the quicksheet - simulate we pushed the bulldozer */
                    CtlHitControl(FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),menuitemID_buildBulldoze)));
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
    // TODO: use FrmCustomAlert here for this stuff
    switch (nError)
    {
        case ERROR_OUT_OF_MEMORY: FrmAlert(alertID_errorOutOfMemory); break;
        case ERROR_OUT_OF_MONEY: FrmAlert(alertID_outMoney); break;
        case ERROR_FIRE_OUTBREAK: FrmAlert(alertID_fireOutBreak); break;
        case ERROR_PLANT_EXPLOSION: FrmAlert(alertID_plantExplosion); break;
        case ERROR_MONSTER: FrmAlert(alertID_monster); break;
        case ERROR_DRAGON: FrmAlert(alertID_dragon); break;
	case ERROR_METEOR: FrmAlert(alertID_meteor); break;
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

//    if (DoDrawing == 0) { return; }

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
    RectangleType rect;

    if (DoDrawing == 0) { return; }
    
    rect.topLeft.x = 144;
    rect.topLeft.y = 0;
    rect.extent.x = TILE_SIZE;
    rect.extent.y = TILE_SIZE;
    
    // copy/paste the graphic from the offscreen image
    // first draw the overlay
    WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*TILE_SIZE+XOFFSET,
            ypos*TILE_SIZE+YOFFSET,
            winErase);
    // now draw the powerloss icon
    rect.topLeft.x = 128;
    WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*TILE_SIZE+XOFFSET,
            ypos*TILE_SIZE+YOFFSET,
            winOverlay);
}

extern void UIDrawSpecialUnit(int i, int xpos, int ypos)
{
    RectangleType rect;
    if (DoDrawing == 0) { return; }
    
    rect.topLeft.x = units[i].type*TILE_SIZE;
    rect.topLeft.y = TILE_SIZE;
    rect.extent.x = TILE_SIZE;
    rect.extent.y = TILE_SIZE;

    WinCopyRectangle(
            winUnits,
            WinGetActiveWindow(),
            &rect,
            xpos*TILE_SIZE+XOFFSET,
            ypos*TILE_SIZE+YOFFSET,
            winErase);
    rect.topLeft.y = 0;
    WinCopyRectangle(
            winUnits,
            WinGetActiveWindow(),
            &rect,
            xpos*TILE_SIZE+XOFFSET,
            ypos*TILE_SIZE+YOFFSET,
            winOverlay);

}

extern void UIDrawSpecialObject(int i, int xpos, int ypos)
{
    RectangleType rect;
    if (DoDrawing == 0) { return; }
    
    rect.topLeft.x = (objects[i].dir)*TILE_SIZE;
    rect.topLeft.y = ((i*2)+1)*TILE_SIZE;
    rect.extent.x = TILE_SIZE;
    rect.extent.y = TILE_SIZE;

    WinCopyRectangle(
            winMonsters,
            WinGetActiveWindow(),
            &rect,
            xpos*TILE_SIZE+XOFFSET,
            ypos*TILE_SIZE+YOFFSET,
            winErase);
    rect.topLeft.y -= 16;
    WinCopyRectangle(
            winMonsters,
            WinGetActiveWindow(),
            &rect,
            xpos*TILE_SIZE+XOFFSET,
            ypos*TILE_SIZE+YOFFSET,
            winOverlay);

}

extern void UIDrawField(int xpos, int ypos, unsigned char nGraphic)
{
    RectangleType rect;
    
    if (DoDrawing == 0) { return; }

    rect.topLeft.x = (nGraphic%64)*TILE_SIZE;
    rect.topLeft.y = (nGraphic/64)*TILE_SIZE;
    rect.extent.x = TILE_SIZE;
    rect.extent.y = TILE_SIZE;
    
    // copy/paste the graphic from the offscreen image
    WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*TILE_SIZE+XOFFSET,
            ypos*TILE_SIZE+YOFFSET,
            winPaint);
}


extern void UIScrollMap(int direction)
{
    WinHandle screen;
    RectangleType rect;
    int to_x, to_y, i;
    
    if (DoDrawing == 0) { return; }

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

    bitmaphandle = DmGet1Resource(TBMP,bitmapID_iconBulldoze + 
            ((nSelectedBuildItem<=9 || (nSelectedBuildItem>=250 && nSelectedBuildItem<=252))?nSelectedBuildItem:10));
    if (bitmaphandle == NULL) { return; } // TODO: onscreen error? +save?
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
    if (bitmaphandle == NULL) { return; } // TODO: onscreen error? +save?
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
#ifdef DEBUG
    char temp[20];
#endif

    // See if we're on in minimum required version of the ROM or later.
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
#ifdef DEBUG
    LongToString(romVersion,(char*)temp);
    UIWriteLog("Rom version: ");
    UIWriteLog(temp);
    UIWriteLog("\n");
#endif

    if (romVersion < requiredVersion)
    {
        if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            //if (FrmAlert (alertID_RomIncompatible) == 1)
            if (romVersion > 0x03100000) {
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

extern void UIWriteLog(char * s) 
{
#ifdef DEBUG
    HostFILE * hf = NULL;

    hf = HostFOpen("pcity.log", "a");
    if (hf) {
        HostFPrintF(hf, s);
        HostFClose(hf);
    }
#endif
}
