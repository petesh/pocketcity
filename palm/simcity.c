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
#include "simcity_resconsts.h"

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

BuildCodes nSelectedBuildItem = Be_Bulldozer;
BuildCodes nPreviousBuildItem = Be_Bulldozer;
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
static void _PalmFini(void);
static void UIDoQuickList(void);
static void UIPopUpExtraBuildList(void);
void UIDrawPop(void);
static void CleanUpExtraBuildForm(void);
static FieldType * UpdateDescription(int sel);
static void initTextPositions(void);

static void _UIGetFieldToBuildOn(int x, int y);
static Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags);
static void EventLoop(void);
static void cycleSpeed(void);
static void DoAbout(void);

#if defined(SONY_CLIE)
static void HoldHook(UInt32);
static void toolBarCheck(Coord);
static void UIDrawToolBar(void);
#endif

extern void UIDrawLoc(void);

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    UInt16 error;

    if (cmd != sysAppLaunchCmdNormalLaunch) return (0);

    error = RomVersionCompatible (
      sysMakeROMVersion(3, 5, 0, sysROMStageRelease, 0), launchFlags);
    if (error) return (error);

    UIWriteLog("Starting Pocket City\n");
    _PalmInit();
    PCityMain();
    if (UILoadAutoGame()) {
	FrmGotoForm(formID_pocketCity);
    } else {
	FrmGotoForm(formID_files);
    }
    
    EventLoop();

    if (game_in_progress) {
	UISaveAutoGame();
    }

    _PalmFini();
    return (0);
}

static void
EventLoop(void)
{
    EventType event;
    FormPtr form;
    int formID;
    UInt32 timeTemp;
    short err;
    static int oldFormID = -1;

    do {
        EvtGetEvent(&event, 1);

       if (event.eType == keyDownEvent) {
           UIWriteLog("keydown event\n");
           if (FrmDispatchEvent(&event)) continue;
       }

       if (SysHandleEvent(&event)) continue;

       if (MenuHandleEvent((void*)0, &event, &err)) continue;

       if (event.eType == frmLoadEvent) {
           char c[20];
           UIWriteLog("Main::frmLoadEvent:");
           formID = event.data.frmLoad.formID;
           StrPrintF(c, "%d -> %d\n", oldFormID, formID);
           UIWriteLog(c);
           oldFormID = formID;
           form = FrmInitForm(formID);
           FrmSetActiveForm(form);
           
           switch (formID) {
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
           if (event.data.winExit.exitWindow ==
             (WinHandle) FrmGetFormPtr(formID_pocketCity)) {
               UIWriteLog("Setting drawing to 0\n");
               DoDrawing = 0;
           }
       }
       else if (event.eType == winEnterEvent) {
           if (event.data.winEnter.enterWindow ==
             (WinHandle) FrmGetFormPtr(formID_pocketCity) &&
             event.data.winEnter.enterWindow == (WinHandle) FrmGetFirstForm()) {
               UIWriteLog("Setting drawing to 1\n");
               DoDrawing = 1;
               RedrawAllFields(); /* update screen after menu etc. */
           } else {
               UIWriteLog("Setting drawing to 0\n");
               DoDrawing = 0;
           }
       }
       
       if (FrmDispatchEvent(&event)) continue;
       
       /* the almighty homemade >>"multithreader"<< */
       if (game_in_progress == 1 && building == 0 &&
         game.gameLoopSeconds != SPEED_PAUSED) {
           if (simState == 0) {
               timeTemp = TimGetSeconds();
               if (timeTemp >= timeStamp+game.gameLoopSeconds) {
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
       
       /*
	* Do a disaster update every 2 secound nomatter what.
	* We don't want the (l)user to get help by pausing the game
        * (not realistic enough)
	* TODO: Pause should be enabled on 'easy' difficulty.
	*/
       if (game_in_progress == 1 && building == 0) {
           timeTemp = TimGetSeconds();

           if (timeTemp >= timeStampDisaster+SIM_GAME_LOOP_DISASTER) {
               MoveAllObjects();
#ifdef DEBUG
               {
                   /*
		    * This will print the BuildCount array
                    * don't forget to keep an eye on it
                    * in the log - should be up-to-date
                    * AT ALL TIMES!
		    */
                   char temp[10];
                   int q;
                   for (q=0; q<20; q++) {
                       sprintf(temp,"%li",vgame.BuildCount[q]);
                       UIWriteLog(temp);
                       UIWriteLog(" ");
                   }
                   UIWriteLog("\n");
                   
               } /* end debug block */
#endif
               if (UpdateDisasters()) {
                   RedrawAllFields();
               }
               timeStampDisaster = timeTemp;
               /* TODO: This would be the place to create animation...
		* perhaps with a second offscreen window for the
                * second animation frame of each zone
                * Then swap the winZones between the two,
                * and do the RedrawAllFields() from above here
		*/
           }
       }
    } while (event.eType != appStopEvent);

}

static const struct _bmphandles {
    WinHandle *handle;
    Coord width, height;
    DmResID resourceID;
} handles[] = {
	/* space for 64*2=128 zones */
    { &winZones, 1024, 32, bitmapID_zones },
    { &winMonsters, 128, 64, bitmapID_monsters },
    { &winUnits, 48, 32, bitmapID_units }
};

void
_PalmInit(void)
{
    UInt32 depth;
    UInt16 err;
    int	i;
    MemHandle  bitmaphandle;
    BitmapPtr  bitmap;
    WinHandle  winHandle = NULL;
    WinHandle	privhandle;

    timeStamp = TimGetSeconds();
    timeStampDisaster = timeStamp;

    /* set screen mode to colors if supported */
    if (oldROM != 1) {  /* must be v3.5+ for some functions in here */
        WinScreenMode(winScreenModeGetSupportedDepths, 0, 0, &depth, 0);
        if ((depth & (1 << (8-1))) != 0 ) {
            /* 8bpp (color) is supported */
            changeDepthRes(8);
        } else if ((depth & (1 << (4-1))) != 0) {
            /* 4bpp (greyscale) is supported */
            changeDepthRes(4);
        }
        /* falls through if you've no color */
    }
    /* The 'playground'... built by the size of the screen */
    rPlayGround.topLeft.x = 0;
    rPlayGround.topLeft.y = 15; /* Padding for the menubar */
    rPlayGround.extent.x = sWidth;
    rPlayGround.extent.y = sHeight - 2*16; /* Space on the bottom */

    /* create an offscreen window, and copy the zones to be used later */
    for (i = 0; i < (sizeof (handles) / sizeof (handles[0])); i++) {
    	bitmaphandle = DmGet1Resource('Tbmp', handles[i].resourceID);
	if (bitmaphandle == NULL) {
	    char c[20];
	    StrPrintF(c, "%ld]\n", (long)handles[i].resourceID);
	    UIWriteLog("could not get bitmap handle[");
	    UIWriteLog(c);
	}
    	bitmap = MemHandleLock(bitmaphandle);
	privhandle = _WinCreateOffscreenWindow(handles[i].width,
	    handles[i].height, genericFormat, &err);
    	if (err != errNone) {
	    /* TODO: alert user, and quit program */
    	    UIWriteLog("Offscreen window for zones failed\n");
	}
	if (winHandle == NULL) winHandle = WinSetDrawWindow(privhandle);
	else WinSetDrawWindow(privhandle);
    	_WinDrawBitmap(bitmap, 0, 0);
	MemHandleUnlock(bitmaphandle);
	*(handles[i].handle) = privhandle;
    }
    initTextPositions();

    /* clean up */
    if (winHandle) WinSetDrawWindow(winHandle);

    hookHoldSwitch(HoldHook);
}

static void
_PalmFini(void)
{
    unhookHoldSwitch();

    /* clean up */
    WinDeleteWindow(winZones,0);
    WinDeleteWindow(winMonsters,0);
    WinDeleteWindow(winUnits,0);
    MemPtrFree(worldPtr);
    MemPtrFree(worldFlagsPtr);
    restoreDepthRes();
    /* Close the forms */
    FrmCloseAllForms();
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
            /* click was on the playground */
            _UIGetFieldToBuildOn(event->screenX, event->screenY);
            handled = 1;
            break;
        }
        if (event->screenY < 12) {
            handled = 1;
            if (event->screenX >= (sWidth - 12)) {
                /* click was on change speed */
                cycleSpeed();
                UIDrawPop();
                break;
            }
            if (event->screenX < 12) {
                /* click was on toggle production */
                if (nSelectedBuildItem == Be_Bulldozer) {
                    nSelectedBuildItem = nPreviousBuildItem;
                } else {
                    nPreviousBuildItem = nSelectedBuildItem;
                    nSelectedBuildItem = Be_Bulldozer;
                }
                UIUpdateBuildIcon();
                break;
            }
#if defined(SONY_CLIE)
            if (isHires())
                toolBarCheck(event->screenX);
#endif
            /* check for other 'penclicks' here */
        }
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
        /* so the simulation routine won't kick in right away */
        timeStampDisaster = timeStamp-game.gameLoopSeconds+1;
        handled = 1;
        break;
    case menuEvent:
#if defined(DEBUG)
	{ char c[20]; StrPrintF(c, "mi: %d", event->data.menu.itemID);
		UIWriteLog(c);}
#endif
    	switch (event->data.menu.itemID) {
	case menuitemID_loadGame:
	    switch(FrmAlert(alertID_loadGame)) {
	    case 0: /* save game */
		UISaveMyCity();
		UIClearAutoSaveSlot();
		FrmGotoForm(formID_files);
		break;
	    case 1: /* don't save */
		UIClearAutoSaveSlot();
		FrmGotoForm(formID_files);
		break;
	    default: /* cancel */
		break;
	    }
	    handled = 1;
	    break;
	case menuitemID_saveGame:
	    if (FrmAlert(alertID_saveGame) == 0) {
		UISaveMyCity();
	    }
	    handled = 1;
	    break;
	case menuitemID_Budget:
	    SaveSpeed();
	    FrmGotoForm(formID_budget);
	    handled = 1;
	    break;
	case menuitemID_Map:
	    SaveSpeed();
	    FrmGotoForm(formID_map);
	    handled = 1;
	    break;
#if defined(CHEAT) || defined(DEBUG)
        case menuitemID_Funny:
	    /*
	     * change this to whatever testing you're doing.
	     * just handy with a 'trigger' button for testing
	     * ie. disaters... this item is erased if you've
	     * not compiled with CHEAT or DEBUG
	     */
#ifdef CHEAT
	    game.credits += 100000;
#endif
#ifdef DEBUG
	    MeteorDisaster(20,20);
#endif
	    handled = 1;
	    break;
#endif
	case menuitemID_Configuration:
	    SaveSpeed();
	    FrmGotoForm(formID_options);
	    handled = 1;
	    break;

	case mi_removeDefence:
	    RemoveAllDefence();
	    handled = 1;
	    break;

	case gi_buildExtra:
	    UIPopUpExtraBuildList();
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

	case mi_CauseFire:
	case mi_CauseMeltDown:
	case mi_CauseMonster:
	case mi_CauseDragon:
	case mi_CauseMeteor:
	    DoSpecificDisaster(event->data.menu.itemID - mi_CauseFire +
			    diFireOutbreak);
	    handled = 1;
	    break;

	}

	case menuitemID_about:
	    DoAbout();
	    handled = 1;
	    break;
	case menuitemID_tips:
	    FrmHelp(StrID_tips);
	    handled = 1;
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
            SaveSpeed();
            FrmGotoForm(formID_map);
            handled = 1;
            break;
        case pageUpChr:
            /* scroll map up */
            ScrollMap(dtUp);
            handled = 1;
            break;
        case pageDownChr:
            /* scroll map down */
            ScrollMap(dtDown);
            handled = 1;
            break;
        case vchrHard2:
            /* scroll map left */
            ScrollMap(dtLeft);
            handled = 1;
            break;
        case vchrHard3:
            /* scroll map right */
            ScrollMap(dtRight);
            handled = 1;
            break;
#ifdef SONY_CLIE
        case vchrJogUp:
            if (!IsDrawWindowMostOfScreen()) break;
            if (jog_lr)
                ScrollMap(dtLeft);
            else
                ScrollMap(dtUp);
            handled = 1;
            break;
        case vchrJogDown:
            if (!IsDrawWindowMostOfScreen()) break;
            if (jog_lr)
                ScrollMap(dtRight);
            else
                ScrollMap(dtDown);
            handled = 1;
            break;
        case vchrJogRelease:
            if (!IsDrawWindowMostOfScreen()) break;
            jog_lr = 1 - jog_lr;
            UIDrawLoc();
            handled = 1;
            break;
#endif
        }
    default:
        break;
    }

    return handled;
}

static void
DoAbout(void)
{
    MemHandle vh;
    MemPtr vs = NULL;
    MemHandle bh;
    MemPtr bs = NULL;

    vh = DmGetResource('tver', 1);
    if (vh != NULL) vs = MemHandleLock(vh);
    if (vh == NULL) vs = "Unknown Version";
    bh = DmGetResource('tSTR', StrID_build);
    if (bh != NULL) bs = MemHandleLock(bh);
    if (bh == NULL) bs = "Unknown Build Time";
    FrmCustomAlert(alertID_about, vs, bs, NULL);
    if (vh) {
        MemPtrUnlock(vs);
        DmReleaseResource(vh);
    }
    if (bh) {
        MemPtrUnlock(bs);
        DmReleaseResource(bh);
    }
}

extern void
UIGotoForm(int n)
{
    UInt16 formid = FrmGetActiveFormID();
    switch (n) {
    case 0:
        if (formid != formID_budget) FrmGotoForm(formID_budget);
        break;
    case 1:
        if (formid != formID_map) FrmGotoForm(formID_map);
        break;
    default:
        break;
    }
}

void
UIPopUpExtraBuildList(void)
{
    FormType * ftList;
    int sfe;

    ftList = FrmInitForm(formID_extraBuild);
    FrmSetEventHandler(ftList, hExtraList);
    UpdateDescription(0);
    sfe = FrmDoDialog(ftList);
    switch (sfe) {
    case buttonID_extraBuildSelect:
        /* List entries must match entries in BuildCodes 0 .. */
        nSelectedBuildItem = LstGetSelection(FrmGetObjectPtr(ftList,
              FrmGetObjectIndex(ftList, listID_extraBuildList)));
        break;
    case buttonID_extraBuildFireMen:
        nSelectedBuildItem = Be_Defence_Fire;
        break;
    case buttonID_extraBuildPolice:
        nSelectedBuildItem = Be_Defence_Police;
        break;
    case buttonID_extraBuildMilitary:
        nSelectedBuildItem = Be_Defence_Military;
        break;
    default:
        break;
    }
#if defined(DEBUG)
    { char c[20]; sprintf(c, "sfe = %u, bi = %u\n", sfe, nSelectedBuildItem);
    UIWriteLog(c);}
#endif
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


static Boolean
hExtraList(EventPtr event)
{
    FormPtr form;
    int handled = 0;

    switch (event->eType) {
    case frmOpenEvent:
        game_in_progress = 0;
        form = FrmGetActiveForm();
        UIWriteLog("open hExtraList\n");
        FrmDrawForm(form);
        handled = 1;
        break;
    case frmCloseEvent:
        UIWriteLog("frmcloseeven\n");
        break;
    case lstSelectEvent:
        if ((event->data.lstSelect.listID) == listID_extraBuildList) {
            /* clear old mem */
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

void
UIDoQuickList(void)
{
    FormType * ftList;

    if (!oldROM) {
        ftList = FrmInitForm(formID_quickList);
        FrmSetEventHandler(ftList, hQuickList);
        nSelectedBuildItem = FrmDoDialog(ftList) - gi_buildBulldoze;

        if (nSelectedBuildItem >= OFFSET_EXTRA)
            UIPopUpExtraBuildList();
        UIUpdateBuildIcon();
        FrmDeleteForm(ftList);
    } else {
        /*
	 * darn, I hate that 3.1 - can't do bitmapped buttons.
	 * I'll just throw the ExtraBuildList up
	 */
        UIPopUpExtraBuildList();
    }
}

static Boolean
hQuickList(EventPtr event)
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
                  FrmGetObjectIndex(FrmGetActiveForm(), gi_buildBulldoze)));
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
    static char okHit = 0;

    switch (event->eType) {
    case frmOpenEvent:
        form = FrmGetActiveForm();
        FrmDrawForm(form);
        CtlSetValue(FrmGetObjectPtr(form, FrmGetObjectIndex(form,
                  buttonID_dis_off+game.disaster_level)), 1);
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
        }
        RestoreSpeed()
        break;
    case keyDownEvent:
        UIWriteLog("Key down\n");
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


unsigned char
UIGetSelectedBuildItem(void)
{
    return (nSelectedBuildItem);
}

void
_UIGetFieldToBuildOn(int x, int y)
{
    RectangleType rect;
    int i,j;
    rect.extent.x = vgame.tileSize;
    rect.extent.y = vgame.tileSize;

    for (i = 0; i < vgame.visible_x; i++) {
        for (j = 0; j < vgame.visible_y; j++) {
            rect.topLeft.x = XOFFSET + i * vgame.tileSize;
            rect.topLeft.y = YOFFSET + j * vgame.tileSize;
            if (RctPtInRectangle(x, y, &rect)) {
                BuildSomething(i + game.map_xpos, j + game.map_ypos);
                return;
            }
        }
    }
}

int
UIDisplayError(erdiType nError)
{
    if (nError < diFireOutbreak)
	switch (nError) {
        case enOutOfMemory: FrmAlert(alertID_errorOutOfMemory); break;
	case enOutOfMoney: FrmAlert(alertID_outMoney); break;
    	default: ErrFatalDisplay("UIDisplayError called with unknown error");
	}
    else if (nError <= diMeteor) {
        char string[512];
        SysStringByIndex(st_disasters, nError - diFireOutbreak, string,
            511);
        if (*string == '\0') StrPrintF(string, "generic disaster??");
        
        FrmCustomAlert(alertID_generic_disaster, string, 0, 0);
        return (0);
    }
    ErrFatalDisplay("UIDisplayError called with unknown error");
    return (0);
}


void UIInitDrawing(void) {};
void UIFinishDrawing(void) {};

UInt8 *didLock = NULL;
/*
 * The WinScreenLock is 'optional' depending on how much memory you have.
 * the call may or may not succeed.
 * by using these two APIs we can get faster, flickerless allscreen updating
 */
void
UILockScreen(void)
{
    if ((oldROM != 1) && !didLock)
        didLock = WinScreenLock(winLockCopy);
}

void
UIUnlockScreen(void)
{
    if ((oldROM != 1) && didLock) {
        WinScreenUnlock();
        didLock = 0;
    }
}

void
_UIDrawRect(int nTop, int nLeft, int nHeight, int nWidth)
{
    /*
     * draws a rect on screen: note, the rect within the border will
     * be exactly nHeight*nWidth pxls
     * the frame's left border will be at nTop-1 and so on
     */

    RectangleType rect;

    rect.topLeft.x = nLeft;
    rect.topLeft.y = nTop;
    rect.extent.x = nWidth;
    rect.extent.y = nHeight;

    _WinDrawRectangleFrame(1, &rect);
}

void
UIDrawBorder()
{
    if (DoDrawing == 0) return;

    /* border */
    _UIDrawRect(YOFFSET, XOFFSET, vgame.visible_y * vgame.tileSize,
      vgame.visible_x * vgame.tileSize);
}


void UISetUpGraphic(void) {}

void UIDrawCursor(int xpos, int ypos) { }

void UIDrawWaterLoss(int xpos, int ypos)
{
    RectangleType rect;

    if (DoDrawing == 0) { return; }

    rect.topLeft.x = 80;
    rect.topLeft.y = 0;
    rect.extent.x = vgame.tileSize;
    rect.extent.y = vgame.tileSize;

    /* copy/paste the graphic from the offscreen image */
    /* first draw the overlay */
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*vgame.tileSize+XOFFSET,
            ypos*vgame.tileSize+YOFFSET,
            winErase);
    /* now draw the powerloss icon */
    rect.topLeft.x = 64;
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*vgame.tileSize+XOFFSET,
            ypos*vgame.tileSize+YOFFSET,
            winOverlay);
}

void UIDrawPowerLoss(int xpos, int ypos)
{
    RectangleType rect;

    if (DoDrawing == 0) { return; }

    rect.topLeft.x = 144;
    rect.topLeft.y = 0;
    rect.extent.x = vgame.tileSize;
    rect.extent.y = vgame.tileSize;

    /* copy/paste the graphic from the offscreen image */
    /* first draw the overlay */
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*vgame.tileSize+XOFFSET,
            ypos*vgame.tileSize+YOFFSET,
            winErase);
    /* now draw the powerloss icon */
    rect.topLeft.x = 128;
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos*vgame.tileSize+XOFFSET,
            ypos*vgame.tileSize+YOFFSET,
            winOverlay);
}

void
UIDrawSpecialUnit(int i, int xpos, int ypos)
{
    RectangleType rect;
    if (DoDrawing == 0) { return; }

    rect.topLeft.x = game.units[i].type*vgame.tileSize;
    rect.topLeft.y = vgame.tileSize;
    rect.extent.x = vgame.tileSize;
    rect.extent.y = vgame.tileSize;

    _WinCopyRectangle(
            winUnits,
            WinGetActiveWindow(),
            &rect,
            xpos*vgame.tileSize+XOFFSET,
            ypos*vgame.tileSize+YOFFSET,
            winErase);
    rect.topLeft.y = 0;
    _WinCopyRectangle(
            winUnits,
            WinGetActiveWindow(),
            &rect,
            xpos*vgame.tileSize+XOFFSET,
            ypos*vgame.tileSize+YOFFSET,
            winOverlay);

}

void
UIDrawSpecialObject(int i, int xpos, int ypos)
{
    RectangleType rect;
    if (DoDrawing == 0) { return; }

    rect.topLeft.x = (game.objects[i].dir) * vgame.tileSize;
    rect.topLeft.y = ((i*2)+1) * vgame.tileSize;
    rect.extent.x = vgame.tileSize;
    rect.extent.y = vgame.tileSize;

    _WinCopyRectangle(
            winMonsters,
            WinGetActiveWindow(),
            &rect,
            xpos * vgame.tileSize+XOFFSET,
            ypos * vgame.tileSize+YOFFSET,
            winErase);
    rect.topLeft.y -= 16;
    _WinCopyRectangle(
            winMonsters,
            WinGetActiveWindow(),
            &rect,
            xpos * vgame.tileSize+XOFFSET,
            ypos * vgame.tileSize+YOFFSET,
            winOverlay);
}

void
UIDrawField(int xpos, int ypos, unsigned char nGraphic)
{
    RectangleType rect;

    if (DoDrawing == 0) { return; }

    rect.topLeft.x = (nGraphic%64) * vgame.tileSize;
    rect.topLeft.y = (nGraphic/64) * vgame.tileSize;
    rect.extent.x = vgame.tileSize;
    rect.extent.y = vgame.tileSize;

    /* copy/paste the graphic from the offscreen image */
    _WinCopyRectangle(
            winZones,
            WinGetActiveWindow(),
            &rect,
            xpos * vgame.tileSize+XOFFSET,
            ypos * vgame.tileSize+YOFFSET,
            winPaint);
}

/*
 * Scroll the map in the direction specified
 */
void
UIScrollMap(dirType direction)
{
    WinHandle screen;
    RectangleType rect;
    int to_x, to_y, i;

    if (DoDrawing == 0) { return; }

    UILockScreen();

    rect.topLeft.x = XOFFSET + vgame.tileSize * (direction == 1);
    rect.topLeft.y = YOFFSET + vgame.tileSize * (direction == 2);
    rect.extent.x = (vgame.visible_x - 1 * (direction == 1 || direction == 3 ))
        * vgame.tileSize;
    rect.extent.y = (vgame.visible_y - 1 * (direction == 0 || direction == 2 ))
        * vgame.tileSize;
    to_x = XOFFSET + vgame.tileSize*(direction == 3);
    to_y = YOFFSET + vgame.tileSize*(direction == 0);


    screen = WinGetActiveWindow();
    _WinCopyRectangle(screen, screen, &rect,to_x, to_y, winPaint);

    /* and lastly, fill the gap */
    LockWorld();
    LockWorldFlags();
    UIInitDrawing();

    if (direction == 1 || direction == 3) {
        for (i = game.map_ypos; i < vgame.visible_y + game.map_ypos; i++) {
            DrawFieldWithoutInit(game.map_xpos + (vgame.visible_x - 1) *
              (direction == 1), i);
        }
    } else {
        for (i = game.map_xpos; i < vgame.visible_x + game.map_xpos; i++) {
            DrawFieldWithoutInit(i, game.map_ypos + (vgame.visible_y - 1) *
              (direction == 2));
        }
    }

    UIDrawCursor(vgame.cursor_xpos - game.map_xpos,
      vgame.cursor_ypos - game.map_ypos);
    UIDrawCredits();
    UIDrawPop();

    UIUnlockScreen();
    UIFinishDrawing();
    UnlockWorldFlags();
    UnlockWorld();
}


extern unsigned long
GetRandomNumber(unsigned long max)
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

/* extent.x is filled in automatically */

static struct StatusPositions lrpositions[] = {
    { { {0, 0}, {0, 11} }, { 0, 1 }, MIDX },  /* DATELOC */
    { { {0, 0}, {0, 11} }, {0, 1}, ENDY }, /* CREDITSLOC */
    { { {0, 0}, {0, 11} }, {0, 1}, MIDX | ENDY }, /* POPLOC */
    { { {0, 0}, {0, 11} }, {0, 1}, ENDX | ENDY }, /* POSITIONLOC */
};
#ifdef SONY_CLIE

static struct StatusPositions hrpositions[] = {
    { { {2, 0}, {0, 11} }, { 0, 1 }, ENDY },  /* DATELOC */
    { { {80, 0}, {0, 11} }, {0, 1}, ENDY }, /* CREDITSLOC */
    { { {160, 0}, {0, 11} }, {0, 1}, ENDY }, /* POPLOC */
    { { {280, 0}, {0, 11} }, {0, 1}, ENDY }, /* POSITIONLOC */
};

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

/* only the topLeft.y location is a 'constant' */
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
      "Item Location is out of range");
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

extern void
UIDrawDate(void)
{
    char temp[23];

    if (DoDrawing == 0) { return; }

    GetDate((char*)temp);
    UIDrawItem(DATELOC, temp);
}


extern void
UIDrawCredits(void)
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
        bitmapHandle = DmGet1Resource('Tbmp', bitmapID_coin);
        if (bitmapHandle == NULL) return;
        bitmap = MemHandleLock(bitmapHandle);
        _WinDrawBitmap(bitmap, 68, sHeight - 11);
        MemPtrUnlock(bitmap);
        DmReleaseResource(bitmapHandle);
    }
#endif
    UIDrawDate();
}

extern void
UIDrawLoc(void)
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
        bitmapHandle = DmGet1Resource('Tbmp', bitmapID_loca);
        if (bitmapHandle == NULL) return;
        bitmap = MemHandleLock(bitmapHandle);
        _WinDrawBitmap(bitmap, 270, sHeight - 11);
        MemPtrUnlock(bitmap);
        DmReleaseResource(bitmapHandle);
    } else
#endif
        StrPrintF(temp, "(%02u,%02u)", game.map_xpos, game.map_ypos);
#ifdef SONY_CLIE
    bitmapHandle = DmGet1Resource('Tbmp', bitmapID_updn + jog_lr);
    /* place at rt - (12 + 8), 1 */
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

extern void
UIUpdateBuildIcon(void)
{
    MemHandle bitmaphandle;
    BitmapPtr bitmap;
    if (DoDrawing == 0) { return; }

    bitmaphandle = DmGet1Resource('Tbmp', bitmapID_iconBulldoze +
            (((nSelectedBuildItem <= Be_Extra)) ?
             nSelectedBuildItem : OFFSET_EXTRA));

    if (bitmaphandle == NULL) { return; } /* TODO: onscreen error? +save? */
    bitmap = MemHandleLock(bitmaphandle);
    _WinDrawBitmap(bitmap, 2, 2);
    MemPtrUnlock(bitmap);
    DmReleaseResource(bitmaphandle);
#if defined(SONY_CLIE)
    if (isHires())
        UIDrawToolBar();
#endif
}

extern void
UIDrawSpeed(void)
{
    MemHandle  bitmaphandle;
    BitmapPtr  bitmap;

    bitmaphandle = DmGet1Resource('Tbmp',
      bitmapID_SpeedPaused + game.gameLoopSeconds);
    if (bitmaphandle == NULL) { return; } /* TODO: onscreen error? +save? */
    bitmap = MemHandleLock(bitmaphandle);
    _WinDrawBitmap(bitmap, sWidth - 12, 2);
    MemPtrUnlock(bitmap);
    DmReleaseResource(bitmaphandle);
}

extern void
UIDrawPop(void)
{
    char temp[25];
#ifdef SONY_CLIE
    MemHandle bitmapHandle;
    BitmapPtr bitmap;
#endif

    if (DoDrawing == 0) { return; }

    StrPrintF(temp, "Pop: %lu", vgame.BuildCount[COUNT_RESIDENTIAL] * 150);
    UIDrawItem(POPLOC, temp);
    UIDrawLoc();
    UIDrawSpeed();
#ifdef SONY_CLIE
    if (isHires()) {
        bitmapHandle = DmGet1Resource('Tbmp', bitmapID_popu);
        if (bitmapHandle == NULL) return;
        bitmap = MemHandleLock(bitmapHandle);
        _WinDrawBitmap(bitmap, 146, sHeight - 11);
        MemPtrUnlock(bitmap);
        DmReleaseResource(bitmapHandle);
    }
#endif
}

extern void
UICheckMoney(void)
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
int
InitWorld(void)
{
    worldHandle = MemHandleNew(10);
    worldFlagsHandle = MemHandleNew(10);
    UIWriteLog("Allocation initial 20 bytes\n");

    if (worldHandle == 0 || worldFlagsHandle == 0) {
        UIDisplayError(enOutOfMemory);
        UIWriteLog("FAILED!\n");
        return 0;
    }
    return 1;
}


int
ResizeWorld(long unsigned size)
{
#ifdef DEBUG
    char temp[20];
    StrPrintF(temp,"%i\n",size);
    UIWriteLog("Allocating bytes: ");
    UIWriteLog(temp);
#endif

    if (MemHandleResize(worldHandle, size) != 0 ||
      MemHandleResize(worldFlagsHandle, size) != 0) {
        UIDisplayError(enOutOfMemory);
        /* QuitGameError(); */
        UIWriteLog("FAILED!\n");
        return 0;
    }

    LockWorld();
    LockWorldFlags();

    MemSet(worldPtr, size, 0);
    MemSet(worldFlagsPtr, size, 0);

    UnlockWorld();
    UnlockWorldFlags();

    return (1);
}

void
MapHasJumped(void)
{
}


void
LockWorld()
{
    worldPtr = MemHandleLock(worldHandle);
}

void
UnlockWorld() {
    MemHandleUnlock(worldHandle);
}

void
LockWorldFlags()
{
    worldFlagsPtr = MemHandleLock(worldFlagsHandle);
}

void
UnlockWorldFlags()
{
    MemHandleUnlock(worldFlagsHandle);
}

unsigned char
GetWorldFlags(long unsigned int pos)
{
    /* NOTE: LockWorld() MUST have been called before this is used!!! */
    if (pos > GetMapMul()) { return 0; }
    return ((unsigned char*)worldFlagsPtr)[pos];
}

void
SetWorldFlags(long unsigned int pos, unsigned char value)
{
    if (pos > GetMapMul()) { return; }
    ((unsigned char*)worldFlagsPtr)[pos] = value;
}

void
OrWorldFlags(unsigned long pos, unsigned char value)
{
    if (pos > GetMapMul()) { return; }
    ((unsigned char*)worldFlagsPtr)[pos] |= value;
}

void
AndWorldFlags(unsigned long pos, unsigned char value)
{
    if (pos > GetMapMul()) { return; }
    ((unsigned char*)worldFlagsPtr)[pos] &= value;
}

unsigned char
GetWorld(unsigned long pos)
{
    /* NOTE: LockWorld() MUST have been called before this is used!!! */
    if (pos > GetMapMul()) { return 0; }
    return ((unsigned char*)worldPtr)[pos];
}

void
SetWorld(unsigned long pos, unsigned char value)
{
    if (pos > GetMapMul()) { return; }
    ((unsigned char*)worldPtr)[pos] = value;
}

static Err
RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32 romVersion;
#ifdef DEBUG
    char temp[20];
#endif

    /* See if we're on in minimum required version of the ROM or later. */
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
#ifdef DEBUG
    StrPrintF(temp, "0x%lx", romVersion);
    UIWriteLog("Rom version: ");
    UIWriteLog(temp);
    UIWriteLog("\n");
#endif

    if (romVersion < requiredVersion) {
        if ((launchFlags &
              (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {
            if (romVersion > sysMakeROMVersion(3, 1, 0, 0, 0)) {
                oldROM = 1;
                return (0);
            }

            /*
	     * Pilot 1.0 will continuously relaunch this app unless we
	     * switch to another safe one.
	     */
            if (romVersion < sysMakeROMVersion(2, 0, 0, 0, 0)) {
                AppLaunchWithCommand(sysFileCDefaultApp,
                  sysAppLaunchCmdNormalLaunch, NULL);
            }
        }
        return (sysErrRomIncompatible);
    }
    return (0);
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

#ifdef SONY_CLIE

static void
UIDrawToolBar(void)
{
    Coord startx = 16;
    Coord starty = 2;
    UInt32 id;
    MemHandle hBitmap;
    MemPtr pBitmap;

    for (id = bitmapID_iconBulldoze; id <= bitmapID_iconExtra; id++) {
        hBitmap = DmGet1Resource('Tbmp', id);
        if (hBitmap == NULL) continue;
        pBitmap = MemHandleLock(hBitmap);
        if (pBitmap == NULL) {
            DmReleaseResource(hBitmap);
            continue;
        }
        _WinDrawBitmap(pBitmap, startx, starty);
        startx += 14;
        MemPtrUnlock(pBitmap);
        DmReleaseResource(hBitmap);
    }
}

static void
toolBarCheck(Coord xpos)
{
    /* We've already confirmed the y-axis. */
    int id = (xpos - 16) / 14;
    if (id == (bitmapID_iconExtra - bitmapID_iconBulldoze)) {
        UIPopUpExtraBuildList();
    } else {
        nSelectedBuildItem = id;
    }
    UIUpdateBuildIcon();
}

/* Pauses the game if you flick the 'hold' switch */
static void
HoldHook(UInt32 held)
{
    if (held) game.gameLoopSeconds = SPEED_PAUSED;
    UIDrawSpeed();
}

#endif

#ifdef DEBUG
extern void
UIWriteLog(char *s)
{
    HostFILE * hf = NULL;

    hf = HostFOpen("g:\\pcity.log", "a");
    if (hf) {
        HostFPrintF(hf, s);
        HostFClose(hf);
    }
}
#endif
