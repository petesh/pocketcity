#include <PalmOS.h>
#include <SysEvtMgr.h>
#include <StringMgr.h>
#include <KeyMgr.h>
#include <StdIOPalm.h>
#include "simcity.h"
#include "savegame.h"
#include "map.h"
#include "budget.h"
#include "../source/zakdef.h"
#include "../source/ui.h"
#include "../source/drawing.h"
#include "../source/build.h"
#include "../source/handler.h"
#include "../source/globals.h"
#include "../source/simulation.h"
#include "../source/disaster.h"
#include "resCompat.h"

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
unsigned short XOFFSET = 0;
unsigned short YOFFSET = 15;
#ifdef SONY_CLIE
UInt16 jog_lr = 0;
#endif

static Boolean hPocketCity(EventPtr event);
static Boolean hQuickList(EventPtr event);
static Boolean hExtraList(EventPtr event);
static Boolean hOptions(EventPtr event);
static void _PalmInit(void);
static void UIDoQuickList(void);
static void UIPopUpExtraBuildList(void);
void UIDrawPop(void);
static void CleanUpExtraBuildForm(void);
static FieldType * UpdateDescription(int sel);
static void initTextPositions(void);

static void _UIGetFieldToBuildOn(int x, int y);
static Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags);
extern void UIDrawLoc(void);
static void cycleSpeed(void);

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    EventType event;
    short err;
    FormPtr form;
    int formID;
    UInt32 timeTemp;
    UInt16 error;

    error = RomVersionCompatible (
      sysMakeROMVersion(3, 5, 0, sysROMStageRelease, 0), launchFlags);
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
                    case formID_options:
                        FrmSetEventHandler(form, hOptions);
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
            if (game_in_progress == 1 && building == 0 && game.gameLoopSeconds != SPEED_PAUSED) {
                if (simState == 0) {
                    timeTemp = TimGetSeconds();
                    if (timeTemp >= timeStamp+game.gameLoopSeconds)
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
#ifdef DEBUG
                    { // block so we can initialize more vars
                        // this will print the BuildCount array
                        // don't forget to keep an eye on it
                        // in the log - should be up-to-date
                        // AT ALL TIMES!
                        char temp[10];
                        int q;
                        for (q=0; q<20; q++) {
                            sprintf(temp,"%li",game.BuildCount[q]);
                            UIWriteLog(temp);
                            UIWriteLog(" ");
                        }
                        UIWriteLog("\n");

                    } // end debug block
#endif
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
        MemPtrFree(worldPtr);
        MemPtrFree(worldFlagsPtr);
        restoreDepthRes();
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

    // set screen mode to colors if supported
    if (oldROM != 1) {  // must be v3.5+ for some functions in here
        WinScreenMode(winScreenModeGetSupportedDepths, 0, 0, &depth, 0);
        if ((depth & (1 << (8-1))) != 0 ) {
            // 8bpp (color) is supported
            changeDepthRes(8);
        } else if ((depth & (1 << (4-1))) != 0) {
            // 4bpp (greyscale) is supported
            changeDepthRes(4);
        }
        // falls through if you've no color
    }
    // The 'playground'... built by the size of the screen
    rPlayGround.topLeft.x = 0;
    rPlayGround.topLeft.y = 15; // Padding for the menubar
    rPlayGround.extent.x = sWidth;
    rPlayGround.extent.y = sHeight - 2*16; // Space on the bottom

    // create an offscreen window, and copy the zones.bmp
    // to be used later
    bitmaphandle = DmGet1Resource(TBMP , bitmapID_zones);
    bitmap = MemHandleLock(bitmaphandle);
    //space for 64*2=128 zones
    winZones = _WinCreateOffscreenWindow(1024, 32, genericFormat, &err);
    if (err != errNone) {
        // TODO: alert user, and quit program
        UIWriteLog("Offscreen window for zones failed\n");
    }
    winHandle = WinSetDrawWindow(winZones);
    // draw the bitmap into the offscreen window
    _WinDrawBitmap(bitmap, 0, 0);
    MemHandleUnlock(bitmaphandle);


    // now, ditto for the monsters
    bitmaphandle = DmGet1Resource(TBMP, bitmapID_monsters);
    bitmap = MemHandleLock(bitmaphandle);
    winMonsters = _WinCreateOffscreenWindow(128, 64, genericFormat, &err);
    if (err != errNone) {
        // TODO: alert user, and quit program
        UIWriteLog("Offscreen window for monsters failed\n");
    }
    WinSetDrawWindow(winMonsters); // note we don't save the old winhandle here
    _WinDrawBitmap(bitmap, 0, 0);
    MemHandleUnlock(bitmaphandle);

    // and, at last, the units
    bitmaphandle = DmGet1Resource(TBMP, bitmapID_units);
    bitmap = MemHandleLock(bitmaphandle);
    winUnits = _WinCreateOffscreenWindow(48, 32, genericFormat,&err);
    if (err != errNone) {
        // TODO: alert user, and quit program
        UIWriteLog("Offscreen window for units failed\n");
    }

    initTextPositions();
    WinSetDrawWindow(winUnits); // note we don't save the old winhandle here
    _WinDrawBitmap(bitmap, 0, 0);
    MemHandleUnlock(bitmaphandle);

    // clean up
    WinSetDrawWindow(winHandle);
}

static Boolean hPocketCity(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType) {
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
        scaleEvent(event);
        if (RctPtInRectangle(event->screenX, event->screenY, &rPlayGround)) {
            // click was on the playground
            _UIGetFieldToBuildOn(event->screenX, event->screenY);
            handled = 1;
            break;
        } else if ((event->screenX >= (sWidth - 12)) && (event->screenY < 12)) {
            // click was on change speed
            cycleSpeed();
            UIDrawPop();
        } else if (event->screenX < 12 && event->screenY < 12) {
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
        scaleEvent(event);
        if (RctPtInRectangle(event->screenX, event->screenY, &rPlayGround)) {
            _UIGetFieldToBuildOn(event->screenX, event->screenY);
            handled = 1;
            building = 1;
        }
        break;
    case penUpEvent:
        building = 0;
        timeStamp = TimGetSeconds()-game.gameLoopSeconds+2;
        // so the simulation routine won't kick in right away
        timeStampDisaster = timeStamp-game.gameLoopSeconds+1; // ditto
        handled = 1;
        break;
    case menuEvent:
        if (event->data.menu.itemID >= menuitemID_buildBulldoze) {
            if (event->data.menu.itemID == menuitemID_buildExtra) {
                UIPopUpExtraBuildList();
            } else {
                nSelectedBuildItem = event->data.menu.itemID -
                    menuitemID_buildBulldoze;
                UIUpdateBuildIcon();
            }
            handled = 1;
        } else {
            switch (event->data.menu.itemID) {
            case menuitemID_Funny:
                // change this to whatever testing you're doing ;)
                // just handy with a 'trigger' button for testing
                // ie. disaters
#ifdef CHEAT
                game.credits += 100000;
#endif
#ifdef DEBUG
                MeteorDisaster(20,20);
#endif
                handled = 1;
                break;
            case menuitemID_removeDefence:
                RemoveAllDefence();
                handled = 1;
                break;
            case menuitemID_Map:
                game.gameLoopSeconds = SPEED_PAUSED;
                FrmGotoForm(formID_map);
                handled = 1;
                break;
            case menuitemID_Budget:
                game.gameLoopSeconds = SPEED_PAUSED;
                FrmGotoForm(formID_budget);
                handled = 1;
                break;
            case menuitemID_about:
                {
                    MemHandle mhp;
                    MemPtr mp = NULL;

                    mhp = DmGetResource('tver', 1);
                    if (mhp != NULL) mp = MemHandleLock(mhp);
                    if (mp == NULL) mp = "??";
                    FrmCustomAlert(alertID_about, mp, NULL, NULL);
                    if (mhp) {
                        MemPtrUnlock(mp);
                        DmReleaseResource(mhp);
                    }
                    handled = 1;
                }
                break;
            case menuitemID_Configuration:
                FrmGotoForm(formID_options);
                handled = 1;
                break;
            case menuitemID_loadGame:
                switch(FrmAlert(alertID_loadGame)) {
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
                game.gameLoopSeconds = SPEED_SLOW;
                UIDrawPop();
                handled = 1;
                break;
            case menuID_MediumSpeed:
                game.gameLoopSeconds = SPEED_MEDIUM;
                UIDrawPop();
                handled = 1;
                break;
            case menuID_FastSpeed:
                game.gameLoopSeconds = SPEED_FAST;
                UIDrawPop();
                handled = 1;
                break;
            case menuID_TurboSpeed:
                game.gameLoopSeconds = SPEED_TURBO;
                UIDrawPop();
                handled = 1;
                break;
            case menuID_PauseSpeed:
                game.gameLoopSeconds = SPEED_PAUSED;
                UIDrawPop();
                handled = 1;
                break;
            }
        }
        break;
    case keyDownEvent:
        switch (event->data.keyDown.chr) {
        case vchrCalc:
            /* popup a quicksheet */
            UIDoQuickList();
            handled = 1;
            break;
        case vchrFind:
            /* goto map */
            game.gameLoopSeconds = SPEED_PAUSED;
            FrmGotoForm(formID_map);
            handled = 1;
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
#ifdef SONY_CLIE
        case vchrJogUp:
            if (jog_lr) ScrollMap(3); else ScrollMap(0); handled = 1; break;
        case vchrJogDown:
            if (jog_lr) ScrollMap(1); else ScrollMap(2); handled = 1; break;
        case vchrJogRelease:
            jog_lr = 1 - jog_lr; UIDrawLoc(); handled = 1; break;
#endif
        }
    default:
        break;
    }

    return handled;
}


extern void
UIGotoForm(int n)
{
    switch (n) {
    case 0:
        FrmGotoForm(formID_budget);
        break;
    case 1:
        FrmGotoForm(formID_map);
        break;
    default:
        break;
    }
}

void UIPopUpExtraBuildList(void)
{
    FormType * ftList;

    ftList = FrmInitForm(formID_extraBuild);
    FrmSetEventHandler(ftList, hExtraList);
    UpdateDescription(0);
    switch (FrmDoDialog(ftList)) {
    case buttonID_extraBuildSelect:
        nSelectedBuildItem = LstGetSelection(FrmGetObjectPtr(ftList,
              FrmGetObjectIndex(ftList, listID_extraBuildList)));
        break;
    case buttonID_extraBuildFireMen:
        nSelectedBuildItem = 250;
        break;
    case buttonID_extraBuildPolice:
        nSelectedBuildItem = 251;
        break;
    case buttonID_extraBuildMilitary:
        nSelectedBuildItem = 252;
        break;
    default:
        break;
    }
    CleanUpExtraBuildForm();
    UIUpdateBuildIcon();
    FrmDeleteForm(ftList);
}

FieldType * UpdateDescription(int sel)
{
    char * temp = MemPtrNew(256);
    UInt16 index = FrmGetObjectIndex(FrmGetFormPtr(formID_extraBuild),
      labelID_extraBuildDescription);
    FieldType * ctl = FrmGetObjectPtr(FrmGetFormPtr(formID_extraBuild), index);

    SysStringByIndex(strID_Descriptions, sel, temp, 256);
    FldSetTextPtr(ctl, temp);
    FldRecalculateField(ctl, true);
    return ctl;
}

void CleanUpExtraBuildForm(void)
{
    FormPtr form = FrmGetFormPtr(formID_extraBuild);
    void * ptr = (void*)FldGetTextPtr(FrmGetObjectPtr(
                form, FrmGetObjectIndex(form, labelID_extraBuildDescription)));
    if (ptr != 0)
        MemPtrFree(ptr);
}


static Boolean hExtraList(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType) {
    case frmOpenEvent:
        game_in_progress = 0;
        form = FrmGetActiveForm();
        UIWriteLog("open\n");
        FrmDrawForm(form);
        handled = 1;
        break;
    case frmCloseEvent:
        UIWriteLog("frmcloseeven\n");
        break;
    case lstSelectEvent:
        if ((event->data.lstSelect.listID) == listID_extraBuildList) {
            // clear old mem
            FormPtr form = FrmGetActiveForm();
            void * ptr = (void*)FldGetTextPtr(FrmGetObjectPtr(form,
                  FrmGetObjectIndex(form, labelID_extraBuildDescription)));
            FldDrawField(UpdateDescription(event->data.lstSelect.selection));
            if (ptr != 0)
                MemPtrFree(ptr);
            handled = 1;
        }
        break;
    case keyDownEvent:
        switch (event->data.keyDown.chr) {
        case vchrCalc:
            CtlHitControl(FrmGetObjectPtr(FrmGetActiveForm(),
                  FrmGetObjectIndex(FrmGetActiveForm(),
                      buttonID_extraBuildCancel)));
            handled = 1;
            break;
        case pageUpChr:
            LstScrollList(FrmGetObjectPtr(FrmGetActiveForm(),
                  FrmGetObjectIndex(FrmGetActiveForm(),
                      listID_extraBuildList)), winUp, 4);
            handled = 1;
            break;
        case pageDownChr:
            LstScrollList(FrmGetObjectPtr(FrmGetActiveForm(),
                  FrmGetObjectIndex(FrmGetActiveForm(),
                      listID_extraBuildList)), winDown, 4);
            handled = 1;
            break;
        }
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
        if (nSelectedBuildItem == 10)
            UIPopUpExtraBuildList();
        UIUpdateBuildIcon();
        FrmDeleteForm(ftList);
    } else {
        // darn, I hate that 3.1 - can't do bitmapped buttons
        // so I'll just throw the ExtraBuildList up
        UIPopUpExtraBuildList();
    }
}


static Boolean hQuickList(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType) {
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
        switch (event->data.keyDown.chr) {
        case vchrCalc:
            /* close the quicksheet - simulate we pushed the bulldozer */
            CtlHitControl(FrmGetObjectPtr(FrmGetActiveForm(),
                  FrmGetObjectIndex(FrmGetActiveForm(),
                  menuitemID_buildBulldoze)));
            handled = 1;
            break;
        }
    default:
        break;
    }

    return handled;
}

static Boolean hOptions(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType) {
    case frmOpenEvent:
        form = FrmGetActiveForm();
        FrmDrawForm(form);
        CtlSetValue(FrmGetObjectPtr(form, FrmGetObjectIndex(form,
                  buttonID_dis_off+game.disaster_level)), 1);
        handled = 1;
        break;
    case frmCloseEvent:
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
        break;
    case keyDownEvent:
        UIWriteLog("Key down\n");
        switch (event->data.keyDown.chr) {
        case vchrLaunch:
            FrmGotoForm(formID_pocketCity);
            handled = 1;
            break;
        }
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


extern unsigned char UIGetSelectedBuildItem(void)
{
    return nSelectedBuildItem;
}

void _UIGetFieldToBuildOn(int x, int y)
{
    RectangleType rect;
    int i,j;
    rect.extent.x = game.tileSize;
    rect.extent.y = game.tileSize;

    for (i = 0; i < game.visible_x; i++) {
        for (j = 0; j < game.visible_y; j++) {
            rect.topLeft.x = XOFFSET + i * game.tileSize;
            rect.topLeft.y = YOFFSET + j * game.tileSize;
            if (RctPtInRectangle(x, y, &rect)) {
                BuildSomething(i + game.map_xpos, j + game.map_ypos);
                return;
            }
        }
    }
}





extern int UIDisplayError(int nError)
{
    // TODO: use FrmCustomAlert here for this stuff
    switch (nError) {
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

UInt8 *didLock = NULL;
// The WinScreenLock is 'optional' depending on how much memory you have
// the call may or may not succeed.
// by using these two APIs we can get faster, flickerless allscreen updating
extern void UILockScreen(void)
{
    if ((oldROM != 1) && !didLock)
        didLock = WinScreenLock(winLockCopy);
}

extern void UIUnlockScreen(void)
{
    if ((oldROM != 1) && didLock) {
        WinScreenUnlock();
        didLock = 0;
    }
}

extern void _UIDrawRect(int nTop, int nLeft, int nHeight, int nWidth)
{
    // draws a rect on screen: note, the rect within the border will
    // be exactly nHeight*nWidth pxls
    // the frame's left border will be at nTop-1 and so on

    RectangleType rect;

//    if (DoDrawing == 0) { return; }

    rect.topLeft.x = nLeft;
    rect.topLeft.y = nTop;
    rect.extent.x = nWidth;
    rect.extent.y = nHeight;

    _WinDrawRectangleFrame(1, &rect);
}

extern void UIDrawBorder()
{
    if (DoDrawing == 0) return;

    // border
    _UIDrawRect(YOFFSET, XOFFSET, game.visible_y * game.tileSize,
      game.visible_x * game.tileSize);
}


extern void UISetUpGraphic(void)
{
}

extern void UIDrawCursor(int xpos, int ypos)
{
}

extern void UIDrawWaterLoss(int xpos, int ypos)
{
    RectangleType rect;

    if (DoDrawing == 0) { return; }

    rect.topLeft.x = 80;
    rect.topLeft.y = 0;
    rect.extent.x = game.tileSize;
    rect.extent.y = game.tileSize;

    // copy/paste the graphic from the offscreen image
    // first draw the overlay
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winErase);
    // now draw the powerloss icon
    rect.topLeft.x = 64;
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winOverlay);
}

extern void UIDrawPowerLoss(int xpos, int ypos)
{
    RectangleType rect;

    if (DoDrawing == 0) { return; }

    rect.topLeft.x = 144;
    rect.topLeft.y = 0;
    rect.extent.x = game.tileSize;
    rect.extent.y = game.tileSize;

    // copy/paste the graphic from the offscreen image
    // first draw the overlay
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winErase);
    // now draw the powerloss icon
    rect.topLeft.x = 128;
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winOverlay);
}

extern void UIDrawSpecialUnit(int i, int xpos, int ypos)
{
    RectangleType rect;
    if (DoDrawing == 0) { return; }

    rect.topLeft.x = game.units[i].type*game.tileSize;
    rect.topLeft.y = game.tileSize;
    rect.extent.x = game.tileSize;
    rect.extent.y = game.tileSize;

    _WinCopyRectangle(
            winUnits,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winErase);
    rect.topLeft.y = 0;
    _WinCopyRectangle(
            winUnits,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winOverlay);

}

extern void UIDrawSpecialObject(int i, int xpos, int ypos)
{
    RectangleType rect;
    if (DoDrawing == 0) { return; }

    rect.topLeft.x = (game.objects[i].dir)*game.tileSize;
    rect.topLeft.y = ((i*2)+1)*game.tileSize;
    rect.extent.x = game.tileSize;
    rect.extent.y = game.tileSize;

    _WinCopyRectangle(
            winMonsters,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winErase);
    rect.topLeft.y -= 16;
    _WinCopyRectangle(
            winMonsters,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winOverlay);

}

extern void UIDrawField(int xpos, int ypos, unsigned char nGraphic)
{
    RectangleType rect;

    if (DoDrawing == 0) { return; }

    rect.topLeft.x = (nGraphic%64)*game.tileSize;
    rect.topLeft.y = (nGraphic/64)*game.tileSize;
    rect.extent.x = game.tileSize;
    rect.extent.y = game.tileSize;

    // copy/paste the graphic from the offscreen image
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*game.tileSize+XOFFSET,
            ypos*game.tileSize+YOFFSET,
            winPaint);
}


extern void UIScrollMap(int direction)
{
    WinHandle screen;
    RectangleType rect;
    int to_x, to_y, i;

    if (DoDrawing == 0) { return; }

    UILockScreen();

    rect.topLeft.x = XOFFSET + game.tileSize * (direction == 1);
    rect.topLeft.y = YOFFSET + game.tileSize * (direction == 2);
    rect.extent.x = (game.visible_x - 1 * (direction == 1 || direction == 3 ))
        * game.tileSize;
    rect.extent.y = (game.visible_y - 1 * (direction == 0 || direction == 2 ))
        * game.tileSize;
    to_x = XOFFSET + game.tileSize*(direction == 3);
    to_y = YOFFSET + game.tileSize*(direction == 0);


    screen = WinGetActiveWindow();
    _WinCopyRectangle(screen, screen, &rect,to_x, to_y, winPaint);

    // and lastly, fill the gap
    LockWorld();
    LockWorldFlags();
    UIInitDrawing();

    if (direction == 1 || direction == 3) {
        for (i = game.map_ypos; i < game.visible_y + game.map_ypos; i++) {
            DrawFieldWithoutInit(game.map_xpos + (game.visible_x - 1) *
              (direction == 1), i);
        }
    } else {
        for (i = game.map_xpos; i < game.visible_x + game.map_xpos; i++) {
            DrawFieldWithoutInit(i, game.map_ypos + (game.visible_y - 1) *
              (direction == 2));
        }
    }

    UIDrawCursor(game.cursor_xpos - game.map_xpos,
      game.cursor_ypos - game.map_ypos);
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

/*
 * Layout (current|lores)
 * [Tool]             [date]         [J][Speed]
 * [                                          ]
 * [            Play Area                     ]
 * [                                          ]
 * [money]            [pop]               [loc]
 *
 * Layout (current|hires)
 * [tool]                            [J][Speed]
 * [            Play Area                     ]
 * [Date]    [Money]     [pop]            [loc]
 *
 * The J is for 'jog' item it's either uplt or ltrt
 */


#define DATELOC 0
#define CREDITSLOC 1
#define POPLOC 2
#define POSITIONLOC 3

#define MIDX    1
#define MIDY    2
#define ENDX    4
#define ENDY    8

struct StatusPositions {
    RectangleType rect;
    PointType offset;
    UInt32 extents;
};

// extent.x is filled in automatically

#ifdef SONY_CLIE
static struct StatusPositions hrpositions[] = {
    { { {2, 0}, {0, 11} }, { 0, 1 }, ENDY },  // DATELOC
    { { {80, 0}, {0, 11} }, {0, 1}, ENDY }, // CREDITSLOC
    { { {160, 0}, {0, 11} }, {0, 1}, ENDY }, // POPLOC
    { { {280, 0}, {0, 11} }, {0, 1}, ENDY }, // POSITIONLOC
};
#endif
static struct StatusPositions lrpositions[] = {
    { { {0, 0}, {0, 11} }, { 0, 1 }, MIDX },  // DATELOC
    { { {0, 0}, {0, 11} }, {0, 1}, ENDY }, // CREDITSLOC
    { { {0, 0}, {0, 11} }, {0, 1}, MIDX | ENDY }, // POPLOC
    { { {0, 0}, {0, 11} }, {0, 1}, ENDX | ENDY }, // POSITIONLOC
};
#ifdef SONY_CLIE
static struct StatusPositions *
posAt(int pos)
{
    static struct StatusPositions *sp = NULL;
    if (sp != NULL) return (&(sp[pos]));
    if (isHires())
        sp = &(hrpositions[0]);
    else
        sp = &(lrpositions[0]);
    return (&(sp[pos]));
}
#else
#define posAt(x)        &(lrpositions[(x)])
#endif
#define MAXLOC          (sizeof (lrpositions) / sizeof (lrpositions[0]))

// only the topLeft.y location is a 'constant'
void
initTextPositions(void)
{
    int i;
    for (i = 0; i < MAXLOC; i++) {
        struct StatusPositions *pos = posAt(i);
        if (pos->extents & ENDY)
            pos->rect.topLeft.y = sHeight -
                (pos->rect.extent.y + pos->offset.y);
    }
}

void UIDrawItem(int location, char *text)
{
    struct StatusPositions *pos;
    Int16 sl;
    Coord tx;
    ErrFatalDisplayIf((location < 0) || (location >= MAXLOC),
      "Location is too large");
    pos = posAt(location);
    if (pos->rect.extent.x && pos->rect.extent.y) {
        _WinEraseRectangle(&(pos->rect), 0);
    }

    if (isHires())
        _FntSetFont(boldFont);
    sl = StrLen(text);
    tx = FntCharsWidth(text, sl);
    switch (pos->extents & (MIDX | ENDX)) {
    case MIDX:
        pos->rect.topLeft.x = (sWidth - tx) / 2 + pos->offset.x;
        break;
    case ENDX:
        pos->rect.topLeft.x = sWidth - (tx + pos->offset.x);
        break;
    }
    pos->rect.extent.x = tx;
    _WinDrawChars(text, sl, pos->rect.topLeft.x, pos->rect.topLeft.y);
    if (isHires())
        _FntSetFont(stdFont);
}

extern void UIDrawDate(void)
{
    char temp[23];

    if (DoDrawing == 0) { return; }

    GetDate((char*)temp);
    UIDrawItem(DATELOC, temp);
}

extern void UIDrawCredits(void)
{
    char temp[23];
#ifdef SONY_CLIE
    MemHandle bitmapHandle;
    BitmapPtr bitmap;
#endif

    if (DoDrawing == 0) { return; }

    StrPrintF(temp, "$: %ld", game.credits);
    UIDrawItem(CREDITSLOC, temp);
#ifdef SONY_CLIE
    if (isHires()) {
        bitmapHandle = DmGet1Resource(TBMP, bitmapID_coin);
        if (bitmapHandle == NULL) return;
        bitmap = MemHandleLock(bitmapHandle);
        _WinDrawBitmap(bitmap, 68, sHeight - 11);
        MemPtrUnlock(bitmap);
        DmReleaseResource(bitmapHandle);
    }
#endif
    UIDrawDate();
}

extern void UIDrawLoc(void)
{
    char temp[25];
#ifdef SONY_CLIE
    MemHandle bitmapHandle;
    BitmapPtr bitmap;
#endif

    if (DoDrawing == 0) { return; }

#ifdef SONY_CLIE
    if (isHires()) {
        StrPrintF(temp, "%02u,%02u", game.map_xpos, game.map_ypos);
        bitmapHandle = DmGet1Resource(TBMP, bitmapID_loca);
        if (bitmapHandle == NULL) return;
        bitmap = MemHandleLock(bitmapHandle);
        _WinDrawBitmap(bitmap, 270, sHeight - 11);
        MemPtrUnlock(bitmap);
        DmReleaseResource(bitmapHandle);
    } else
#endif
        StrPrintF(temp, "(%02u,%02u)", game.map_xpos, game.map_ypos);
#ifdef SONY_CLIE
    bitmapHandle = DmGet1Resource(TBMP, bitmapID_updn + jog_lr);
    // place at rt - (12 + 8), 1
    if (bitmapHandle) {
        bitmap = MemHandleLock(bitmapHandle);
        if (bitmap) {
            _WinDrawBitmap(bitmap, sWidth - (12 + 8), 1);
            MemPtrUnlock(bitmap);
        }
        DmReleaseResource(bitmapHandle);

    }
#endif
    UIDrawItem(POSITIONLOC, temp);
}

extern void UIUpdateBuildIcon(void)
{
    MemHandle bitmaphandle;
    BitmapPtr bitmap;
    if (DoDrawing == 0) { return; }

    bitmaphandle = DmGet1Resource(TBMP,bitmapID_iconBulldoze +
            ((nSelectedBuildItem <= 9 ||
              (nSelectedBuildItem >= 250 && nSelectedBuildItem <= 252)) ?
             nSelectedBuildItem : 10));
    if (bitmaphandle == NULL) { return; } // TODO: onscreen error? +save?
    bitmap = MemHandleLock(bitmaphandle);
    _WinDrawBitmap(bitmap, 2, 2);
    MemPtrUnlock(bitmap);
    DmReleaseResource(bitmaphandle);
}

extern void UIDrawSpeed(void)
{
    MemHandle  bitmaphandle;
    BitmapPtr  bitmap;

    bitmaphandle = DmGet1Resource(TBMP,
      bitmapID_SpeedPaused + game.gameLoopSeconds);
    if (bitmaphandle == NULL) { return; } // TODO: onscreen error? +save?
    bitmap = MemHandleLock(bitmaphandle);
    _WinDrawBitmap(bitmap, sWidth - 12, 2);
    MemPtrUnlock(bitmap);
    DmReleaseResource(bitmaphandle);
}

extern void UIDrawPop(void)
{
    char temp[25];
#ifdef SONY_CLIE
    MemHandle bitmapHandle;
    BitmapPtr bitmap;
#endif

    if (DoDrawing == 0) { return; }

    StrPrintF(temp, "Pop: %lu", game.BuildCount[COUNT_RESIDENTIAL] * 150);
    UIDrawItem(POPLOC, temp);
    UIDrawLoc();
    UIDrawSpeed();
#ifdef SONY_CLIE
    if (isHires()) {
        bitmapHandle = DmGet1Resource(TBMP, bitmapID_popu);
        if (bitmapHandle == NULL) return;
        bitmap = MemHandleLock(bitmapHandle);
        _WinDrawBitmap(bitmap, 146, sHeight - 11);
        MemPtrUnlock(bitmap);
        DmReleaseResource(bitmapHandle);
    }
#endif
}

extern void UICheckMoney(void)
{
    if (game.credits == 0) {
        if(noShown == 0) {
            FrmAlert(alertID_outMoney);
            noShown = 1;
        } else {
            return;
        }
    } else if ((game.credits <= 1000) || (game.credits == 1000)) {
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
    UIWriteLog("Allocation initial 20 bytes\n");

    if (worldHandle == 0 || worldFlagsHandle == 0) {
        UIDisplayError(0);
        UIWriteLog("FAILED!\n");
        return 0;
    }
    return 1;
}


extern int ResizeWorld(long unsigned size)
{
    int i;
#ifdef DEBUG
    char temp[20];
    StrPrintF(temp,"%i\n",size);
    UIWriteLog("Allocating bytes: ");
    UIWriteLog(temp);
#endif


    if (MemHandleResize(worldHandle, size) != 0 || MemHandleResize(worldFlagsHandle, size) != 0) {
        UIDisplayError(0);
        //QuitGameError();
        UIWriteLog("FAILED!\n");
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

extern void MapHasJumped(void) { }


extern void LockWorld() { worldPtr = MemHandleLock(worldHandle); }
extern void UnlockWorld() { MemHandleUnlock(worldHandle); }
extern void LockWorldFlags() { worldFlagsPtr = MemHandleLock(worldFlagsHandle); }
extern void UnlockWorldFlags() { MemHandleUnlock(worldFlagsHandle); }



extern unsigned char GetWorldFlags(long unsigned int pos)
{
    // NOTE: LockWorld() MUST have been called before this is used!!!
    if (pos > (game.mapsize*game.mapsize)) { return 0; }
    return ((unsigned char*)worldFlagsPtr)[pos];
}

extern void SetWorldFlags(long unsigned int pos, unsigned char value)
{
    if (pos > game.mapsize*game.mapsize) { return; }
    ((unsigned char*)worldFlagsPtr)[pos] = value;
}


extern unsigned char GetWorld(unsigned long pos)
{
    // NOTE: LockWorld() MUST have been called before this is used!!!
    if (pos > (game.mapsize*game.mapsize)) { return 0; }
    return ((unsigned char*)worldPtr)[pos];
}

extern void SetWorld(unsigned long pos, unsigned char value)
{
    if (pos > game.mapsize*game.mapsize) { return; }
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

    if (romVersion < requiredVersion) {
        if ((launchFlags &
              (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {
            //if (FrmAlert (alertID_RomIncompatible) == 1)
            if (romVersion > sysMakeROMVersion(3, 1, 0, 0, 0)) {
                oldROM = 1;
                return (0);
            }

            // Pilot 1.0 will continuously relaunch this app unless we switch to
            // another safe one.
            if (romVersion < sysMakeROMVersion(2, 0, 0, 0, 0)) {
                AppLaunchWithCommand(sysFileCDefaultApp,
                  sysAppLaunchCmdNormalLaunch, NULL);
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

static void
cycleSpeed(void)
{
    static struct fromto {
        UInt32 from, to;
    } ft[] = {
        {SPEED_PAUSED, SPEED_SLOW},
        {SPEED_SLOW, SPEED_MEDIUM},
        {SPEED_MEDIUM, SPEED_FAST},
        {SPEED_FAST, SPEED_TURBO},
        {SPEED_TURBO, SPEED_PAUSED}
    };

    int i;
    for (i = 0; i < (sizeof (ft) / sizeof (ft[0])); i++) {
        if (ft[i].from == game.gameLoopSeconds) {
            game.gameLoopSeconds = ft[i].to;
            break;
        }
    }
}
