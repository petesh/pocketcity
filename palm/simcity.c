#include <PalmOS.h>
#include <SysEvtMgr.h>
#include <StringMgr.h>
#include <KeyMgr.h>
#include <StdIOPalm.h>
#include <simcity.h>
#include <savegame.h>
#include <options.h>
#include <map.h>
#include <budget.h>
#include <zakdef.h>
#include <ui.h>
#include <drawing.h>
#include <build.h>
#include <query.h>
#include <handler.h>
#include <globals.h>
#include <simulation.h>
#include <disaster.h>
#include <resCompat.h>
#include <palmutils.h>
#include <simcity_resconsts.h>

#ifdef DEBUG
#include <HostControl.h>
#endif

static MemHandle worldHandle;
static MemHandle worldFlagsHandle;
MemPtr worldPtr;
MemPtr worldFlagsPtr;
static RectangleType rPlayGround;

WinHandle winZones;
WinHandle winMonsters;
WinHandle winUnits;

BuildCodes nSelectedBuildItem = Be_Bulldozer;
BuildCodes nPreviousBuildItem = Be_Bulldozer;

UInt32 timeStamp = 0;
UInt32 timeStampDisaster = 0;
Int16 simState = 0;
UInt16 XOFFSET = 0;
UInt16 YOFFSET = 15;
#ifdef SONY_CLIE
UInt16 jog_lr = 0;
#endif

static Boolean hPocketCity(EventPtr event);
static Boolean hQuickList(EventPtr event);
static Boolean hExtraList(EventPtr event);

static Int16 _PalmInit(void);
static void _PalmFini(Int16);
static void buildSilkList(void);
static Int16 vkDoEvent(UInt16 key);
static void UIDoQuickList(void);
static void UIPopUpExtraBuildList(void);
static void CleanUpExtraBuildForm(void);
static FieldType * UpdateDescription(Int16 sel);
static void initTextPositions(void);

static void _UIGetFieldToBuildOn(Int16 x, Int16 y);
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);
static void EventLoop(void);
static void cycleSpeed(void);
static void DoAbout(void);
static void CheckTextClick(Coord x, Coord y);
static Int16 doButtonEvent(ButtonEvent key);

#if defined(SONY_CLIE)
static void HoldHook(UInt32);
static void toolBarCheck(Coord);
static void UIDrawToolBar(void);
static void clearToolbarBitmap(void);
#else
#define	clearToolbarBitmap()
#endif

extern void UIDrawLoc(void);

/* Collects what would otherwise be several variables */
static UInt8 IsBuilding(void);
static void SetBuilding(void);
static void SetNotBuilding(void);
static void SetLowNotShown(void);
static void SetOutNotShown(void);
static void SetDeferDrawing(void);
static void SetDrawing(void);
static UInt8 IsDeferDrawing(void);

void UIDrawPop(void);
void UIDrawSpeed(void);

void *
GetObjectPtr(FormType *form, UInt16 index)
{
	return (FrmGetObjectPtr(form,
	    FrmGetObjectIndex(form, index)));
}

/*
 * The PilotMain routine.
 * Needed by all palm applications. It checks the rom version and
 * Throws out to the user of it's too old.
 * Handles the auto-saving of the application at the end.
 */
UInt32
PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	UInt16 error;
	Int16 pir;

	if (cmd != sysAppLaunchCmdNormalLaunch)
		return (0);

	error = RomVersionCompatible(
	    sysMakeROMVersion(3, 5, 0, sysROMStageRelease, 0), launchFlags);
	if (error)
		return (error);

	WriteLog("Starting Pocket City\n");
	if (0 != (pir = _PalmInit())) {
		WriteLog("Init Didn't Happen right[%d]\n", (int)pir);
		_PalmFini(pir);
		return (1);
	}

	PCityMain();
	if (-1 != UILoadAutoGame()) {
		SetLowNotShown();
		SetOutNotShown();
	FrmGotoForm(formID_pocketCity);
	} else {
	FrmGotoForm(formID_files);
	}

	EventLoop();

	if (IsGameInProgress()) {
	UISaveAutoGame();
	}

	_PalmFini(pir);
	return (0);
}

/*
 * Event loop for application level form events.
 * These are events that are not dealt with by the main event loop.
 * Returns true if the event has been handled, false otherwise.
 */
static Boolean
AppEvent(EventPtr event)
{
	static UInt16 oldFormID = -1;
	FormPtr form;
	UInt16 formID;

	if (event->eType == frmLoadEvent) {
		formID = event->data.frmLoad.formID;
		WriteLog("Main::frmLoadEvent: %d -> %d\n", oldFormID, formID);
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
		case formID_ButtonConfig:
			FrmSetEventHandler(form, hButtonConfig);
			break;
		case formID_Query:
			FrmSetEventHandler(form, hQuery);
			break;
		}
		return (true);
	}

	if (event->eType == winExitEvent) {
		if (event->data.winExit.exitWindow ==
		    (WinHandle)FrmGetFormPtr(formID_pocketCity)) {
			WriteLog("Setting drawing to 0\n");
			SetDeferDrawing();
		}
		return (true);
	}

	if (event->eType == winEnterEvent) {
		if (event->data.winEnter.enterWindow ==
		    (WinHandle) FrmGetFormPtr(formID_pocketCity) &&
		    event->data.winEnter.enterWindow ==
		    (WinHandle)FrmGetFirstForm()) {
			WriteLog("Setting drawing to 1\n");
			SetDrawing();
			RedrawAllFields(); /* update screen after menu etc. */
		} else {
			WriteLog("Setting drawing to 0\n");
			SetDeferDrawing();
		}
		/* pass to form */
		return (false);
	}
	return (false);
}

/*
 * Main event loop routine.
 * The standard one ... loop until the game terminates.
 */
static void
EventLoop(void)
{
	EventType event;
	UInt32 timeTemp;
	UInt16 err;

	for (;;) {
		EvtGetEvent(&event, 1);
		if (event.eType == appStopEvent) break;

		if (event.eType == keyDownEvent)
			if (FrmDispatchEvent(&event)) continue;

		if (SysHandleEvent(&event)) continue;

		if (MenuHandleEvent((void*)0, &event, &err)) continue;

		if (AppEvent(&event)) continue;

		if (FrmDispatchEvent(&event)) continue;

		/*
		 * if we're faffing around in the savegame dialogs then
		 * we're not actually playing the game.
		 */
		if (!IsGameInProgress()) continue;

		/* Game is fully formally paused ? */
		if (!IsGamePlaying())
			continue;

		if (IsBuilding()) continue;

		/* the almighty homemade >>"multithreader"<< */
		if (game.gameLoopSeconds != SPEED_PAUSED) {
			if (simState == 0) {
				timeTemp = TimGetSeconds();
				if (timeTemp >=
				    timeStamp + game.gameLoopSeconds) {
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
		 * Do a disaster update every 2 seconds.
		 * This is only disabled when difficulty is at easy and
		 * game is paused.
		 */

		if (GetDifficultyLevel() == 0 &&
		    game.gameLoopSeconds == SPEED_PAUSED) {
			continue;
		}

		timeTemp = TimGetSeconds();

		if (timeTemp >= (timeStampDisaster + SIM_GAME_LOOP_DISASTER)) {
#ifdef DEBUG
			Int16 q;
#endif
			MoveAllObjects();
#ifdef DEBUG
			/*
			 * This will print the BuildCount array
			 * don't forget to keep an eye on it
			 * in the log - should be up-to-date
			 * AT ALL TIMES!
			 */
			for (q = 0; q < 20; q++)
				WriteLog("%li ", vgame.BuildCount[q]);
			WriteLog("\n");
#endif
			if (UpdateDisasters()) {
				RedrawAllFields();
			}
			timeStampDisaster = timeTemp;
			/*
			 * TODO: This would be the place to create animation...
			 * perhaps with a second offscreen window for the
			 * second animation frame of each zone
			 * Then swap the winZones between the two,
			 * and do the RedrawAllFields() from above here
			 */
		}
	}
}

/*
 * Handles, dimensions and resourceID's of the bitmaps
 */
static const struct _bmphandles {
	WinHandle *handle;
	const Coord width, height;
	const DmResID resourceID;
} handles[] = {
	/* space for 64*2=128 zones */
	{ &winZones, 1024, 32, (DmResID)bitmapID_zones },
	{ &winMonsters, 128, 64, (DmResID)bitmapID_monsters },
	{ &winUnits, 48, 32, (DmResID)bitmapID_units }
};

static DmOpenRef _refTiles;
/*
 * Set up the game in relation to the screen mode.
 */
static Int16
_PalmInit(void)
{
	UInt32		depth;
	UInt16		err;
	Int16		i;
	MemHandle	bitmaphandle;
	BitmapPtr	bitmap;
	WinHandle	winHandle = NULL;
	WinHandle	privhandle;
	UInt16		prefSize;
	Int16		rv = 0;

	timeStamp = TimGetSeconds();
	timeStampDisaster = timeStamp;

	prefSize = sizeof (AppConfig_t);
	err = PrefGetAppPreferences(GetCreatorID(), 0, &gameConfig, &prefSize,
	    true);
	if (err != noPreferenceFound) {
	}

	/* section (2) */

	_refTiles = DmOpenDatabaseByTypeCreator(TILEDBTYPE, GetCreatorID(),
	    dmModeReadOnly);

	if (_refTiles == 0) {
		FrmAlert(alertID_tilesMissing);
		return (2);
	}

	/* section (3) */

	/* set screen mode to colors if supported */
	if (IsNewROM()) {  /* must be v3.5+ for some functions in here */
		WinScreenMode(winScreenModeGetSupportedDepths, 0, 0, &depth, 0);
		if ((depth & (1 << (8-1))) != 0) {
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

	/* section (4) */

	/* section (5) is special ... it always gets called */

	/* create an offscreen window, and copy the zones to be used later */
	for (i = 0; i < (sizeof (handles) / sizeof (handles[0])); i++) {
		bitmaphandle = DmGetResource('Tbmp', handles[i].resourceID);
		if (bitmaphandle == NULL) {
			WriteLog("could not get bitmap handle[%d:%ld]\n",
			    (int)i, (long)handles[i].resourceID);
			if (winHandle) WinSetDrawWindow(winHandle);
			return (5);
		}
		bitmap = MemHandleLock(bitmaphandle);
		if (bitmap == NULL) {
			WriteLog("MemHandleLock Failed handle[%d:%ld]\n",
			    (int)i, (long)handles[i].resourceID);
			DmReleaseResource(bitmaphandle);
			rv = 5;
			goto returnWV;
		}

		privhandle = _WinCreateOffscreenWindow(handles[i].width,
		    handles[i].height, genericFormat, &err);
		if (err != errNone) {
			/* TODO: alert user, and quit program */
			WriteLog("Offscreen window for zone[%d] failed\n",
			    (int)i);
			MemHandleUnlock(bitmaphandle);
			DmReleaseResource(bitmaphandle);
			rv = 5;
			goto returnWV;
		}
		if (winHandle == NULL)
			winHandle = WinSetDrawWindow(privhandle);
		else
			WinSetDrawWindow(privhandle);
		_WinDrawBitmap(bitmap, 0, 0);
		MemHandleUnlock(bitmaphandle);
		DmReleaseResource(bitmaphandle);
		*(handles[i].handle) = privhandle;
	}
	initTextPositions();

	/* clean up */

	hookHoldSwitch(HoldHook);
	/* load application configuration */
	buildSilkList();

returnWV:
	if (winHandle) WinSetDrawWindow(winHandle);

	return (rv);
}

/*
 * Clean up the application.
 * Save the button configuration and delete any windows that have been
 * allocated.
 * Everything is done in reverse order from having them done in the init.
 */
static void
_PalmFini(Int16 reached)
{
	Int16 i;

	unhookHoldSwitch();
	clearToolbarBitmap();

	switch (reached) {
	case 0:
		if (worldHandle != NULL) MemHandleFree(worldHandle);
		if (worldFlagsHandle != NULL) MemHandleFree(worldFlagsHandle);
	case 5:
		/* clean up handles */
		for (i = 0; i < (sizeof (handles) / sizeof (handles[0])); i++) {
			if (*(handles[i].handle) != NULL)
				WinDeleteWindow(*(handles[i].handle), 0);
		}
	case 4:
		restoreDepthRes();
	case 3:
		DmCloseDatabase(_refTiles);
	case 2:
		PrefSetAppPreferences(GetCreatorID(), 0, CONFIG_VERSION,
		    &gameConfig, sizeof (AppConfig_t), true);
		WriteLog("saved: %ld\n", (long)sizeof (AppConfig_t));
	}
	/* Close the forms */
	FrmCloseAllForms();
}

static Boolean
DoPCityMenuProcessing(UInt16 itemID)
{
	Boolean handled = false;

	switch (itemID) {
		/* First menu ... game */
	case menuitemID_loadGame:
		switch (FrmAlert(alertID_loadGame)) {
		case 0: /* save game */
			UISaveMyCity();
			UIClearAutoSaveSlot();
			SetLowNotShown();
			SetOutNotShown();
			FrmGotoForm(formID_files);
			break;
		case 1: /* don't save */
			SetLowNotShown();
			SetOutNotShown();
			UIClearAutoSaveSlot();
			FrmGotoForm(formID_files);
			break;
		default: /* cancel */
			break;
		}
		handled = true;
		break;
	case menuitemID_saveGame:
		if (FrmAlert(alertID_saveGame) == 0) {
			UISaveMyCity();
		}
		handled = true;
		break;
	case menuitemID_Budget:
		FrmGotoForm(formID_budget);
		handled = true;
		break;
	case menuitemID_Map:
		FrmGotoForm(formID_map);
		handled = true;
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
		MeteorDisaster(20, 20);
#endif
		handled = true;
		break;
#endif
	case menuitemID_Configuration:
		FrmGotoForm(formID_options);
		handled = true;
		break;

	case menuitemID_Buttons:
		FrmGotoForm(formID_ButtonConfig);
		handled = true;
		break;

		/* next menu ... build */

	case mi_removeDefence:
		RemoveAllDefence();
		handled = true;
		break;

		/* for a reason ... */
	case gi_buildExtra:
		UIPopUpExtraBuildList();
		handled = true;
		break;

	case mi_CauseFire:
	case mi_CauseMeltDown:
	case mi_CauseMonster:
	case mi_CauseDragon:
	case mi_CauseMeteor:
		DoSpecificDisaster(itemID - mi_CauseFire + diFireOutbreak);
		handled = true;
		break;

		/* next menu ... speed */

	case menuID_SlowSpeed:
		game.gameLoopSeconds = SPEED_SLOW;
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_MediumSpeed:
		game.gameLoopSeconds = SPEED_MEDIUM;
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_FastSpeed:
		game.gameLoopSeconds = SPEED_FAST;
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_TurboSpeed:
		game.gameLoopSeconds = SPEED_TURBO;
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_PauseSpeed:
		game.gameLoopSeconds = SPEED_PAUSED;
		UIDrawSpeed();
		handled = true;
		break;

		/* next menu ... help */

	case menuitemID_about:
		DoAbout();
		handled = true;
		break;
	case menuitemID_tips:
		FrmHelp(StrID_tips);
		handled = true;
		break;
	}
	return (handled);
}

/*
 * Handler for the main pocketCity form.
 * This form performs all the updates to the main game screen.
 */
static Boolean
hPocketCity(EventPtr event)
{
	FormPtr form;
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		form = FrmGetActiveForm();
		SetGameInProgress();
		ResumeGame();
		FrmDrawForm(form);
		DrawGame(1);
		handled = true;
		break;
	case frmCloseEvent:
		break;
	case penDownEvent:
		scaleEvent(event);
		if (RctPtInRectangle(event->screenX, event->screenY,
		    &rPlayGround)) {
			/* click was on the playground */
			_UIGetFieldToBuildOn(event->screenX, event->screenY);
			handled = true;
			break;
		}
		if (event->screenY < 12) {
			handled = true;
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

		CheckTextClick(event->screenX, event->screenY);

		break;
	case penMoveEvent:
		scaleEvent(event);
		if (RctPtInRectangle(event->screenX, event->screenY,
		    &rPlayGround)) {
			_UIGetFieldToBuildOn(event->screenX, event->screenY);
			SetBuilding();
			handled = true;
		}
		break;
	case penUpEvent:
		SetNotBuilding();
		timeStamp = TimGetSeconds()-game.gameLoopSeconds+2;
		/* so the simulation routine won't kick in right away */
		timeStampDisaster = timeStamp-game.gameLoopSeconds+1;
		handled = true;
		break;
	case menuEvent:
		WriteLog("Menu Item: %d\n", event->data.menu.itemID);
		handled = DoPCityMenuProcessing(event->data.menu.itemID);


	case keyDownEvent:
		handled = vkDoEvent(event->data.keyDown.chr);
		break;
	default:
		break;
	}

	return (handled);
}

/*
 * Do the about dialog
 */
static void
DoAbout(void)
{
	MemHandle vh;
	MemPtr vs = NULL;
	MemHandle bh;
	MemPtr bs = NULL;
	const UInt8 *qq = "??";

	vh = DmGetResource('tver', 1);
	if (vh != NULL) vs = MemHandleLock(vh);
	if (vh == NULL) vs = (MemPtr)qq;
	bh = DmGetResource('tSTR', StrID_build);
	if (bh != NULL) bs = MemHandleLock(bh);
	if (bh == NULL) bs = (MemPtr)qq;
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

/*
 * Go to one of the Budget and map forms.
 * Only if we're not already at that form to begin with.
 */
extern void
UIGotoForm(Int16 n)
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

/*
 * Pop up the extra build list.
 * Contains items that are not in the quicklist tool bar.
 * This is the only choice for Pre-PalmOS 3.5 users, as they
 * can't have bitmapped buttons.
 */
void
UIPopUpExtraBuildList(void)
{
	FormType * form;
	UInt16 sfe;
	static Char **lp;
	UInt16 poplen;

	form = FrmInitForm(formID_extraBuild);
	FrmSetEventHandler(form, hExtraList);
	lp = FillStringList(strID_Items, &poplen);
	LstSetListChoices(GetObjectPtr(form, listID_extraBuildList),
	    lp, poplen);

	UpdateDescription(0);
	sfe = FrmDoDialog(form);

	FreeStringList(lp);

	switch (sfe) {
	case buttonID_extraBuildSelect:
		/* List entries must match entries in BuildCodes 0 .. */
		nSelectedBuildItem = LstGetSelection(
		    GetObjectPtr(form, listID_extraBuildList));
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

	WriteLog("sfe = %u, bi = %u\n", sfe, nSelectedBuildItem);

	CleanUpExtraBuildForm();
	UIUpdateBuildIcon();
	FrmDeleteForm(form);
}

/*
 * Update the description of the item in the extra build list.
 * XXX: Use MemHandles instead?
 */
FieldType *
UpdateDescription(Int16 sel)
{
	Int16 cost;
	Int8 * temp = MemPtrNew(256);
	Int16 *ch;
	MemHandle mh;
	FormType *fp;
	FieldType * ctl;

	fp = FrmGetFormPtr(formID_extraBuild);
	ctl = GetObjectPtr(fp, labelID_extraBuildDescription);

	SysStringByIndex(strID_Descriptions, sel, temp, 256);
	FldSetTextPtr(ctl, temp);
	FldRecalculateField(ctl, true);

	mh = DmGetResource('wrdl', wdlID_Costs);
	if (mh != NULL) {
		ch = MemHandleLock(mh);
		cost = ch[sel+1];
		MemHandleUnlock(mh);
		DmReleaseResource(mh);
	} else {
		cost = -1;
	}
	temp = MemPtrNew(16);
	ctl = GetObjectPtr(fp, labelID_extraBuildPrice);
	StrPrintF(temp, "%d", cost);
	FldSetTextPtr(ctl, temp);
	FldRecalculateField(ctl, true);

	return (ctl);
}

/*
 * Release any memory allocated to the
 * extra build list form.
 */
void
CleanUpExtraBuildForm(void)
{
	FormPtr form = FrmGetFormPtr(formID_extraBuild);
	void * ptr = (void*)FldGetTextPtr(
	    GetObjectPtr(form, labelID_extraBuildDescription));
	if (ptr != 0)
		MemPtrFree(ptr);
	ptr = (void*)FldGetTextPtr(GetObjectPtr(form, labelID_extraBuildPrice));
	if (ptr != 0)
		MemPtrFree(ptr);
}

/*
 * Handler for the Extra Build list form.
 * Doesn't clean up the description items;
 * that's done in UIPopupExtraBuildList()
 */
static Boolean
hExtraList(EventPtr event)
{
	FormPtr form;
	void *ptr, *ptr2;
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		PauseGame();
		form = FrmGetActiveForm();
		WriteLog("open hExtraList\n");
		FrmDrawForm(form);
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("close hExtraList\n");
		break;
	case lstSelectEvent:
		WriteLog("list selection\n");
		if ((event->data.lstSelect.listID) == listID_extraBuildList) {
			/* clear old mem */
			FormPtr form = FrmGetActiveForm();
			ptr = (void*)FldGetTextPtr(
			    GetObjectPtr(form, labelID_extraBuildDescription));
			ptr2 = (void*)FldGetTextPtr(
			    GetObjectPtr(form, labelID_extraBuildPrice));
			FldDrawField(
			    UpdateDescription(event->data.lstSelect.selection));
			if (ptr != 0)
				MemPtrFree(ptr);
			if (ptr2 != 0)
				MemPtrFree(ptr2);
			handled = true;
		}
		break;
	case keyDownEvent:
		switch (event->data.keyDown.chr) {
		case vchrCalc:
			CtlHitControl(GetObjectPtr(FrmGetActiveForm(),
			    buttonID_extraBuildCancel));
			handled = true;
			break;
		case pageUpChr:
			LstScrollList(GetObjectPtr(FrmGetActiveForm(),
			    listID_extraBuildList), winUp, 4);
			handled = true;
			break;
		case pageDownChr:
			LstScrollList(GetObjectPtr(FrmGetActiveForm(),
			    listID_extraBuildList), winDown, 4);
			handled = true;
			break;
		}
	default:
		break;
	}

	return (handled);
}

/*
 * Pop up the quick list.
 * Contains most of the commonly used icons in one place.
 * Will pop up the extra build list if the machine is Pre Palmos 3.5 as
 * it can't do bitmapped buttons.
 */
void
UIDoQuickList(void)
{
	FormType * ftList;

	if (IsNewROM()) {
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

/*
 * Handler for the Quick List form
 */
static Boolean
hQuickList(EventPtr event)
{
	FormPtr form;
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		WriteLog("open quicklist\n");
		PauseGame();
		form = FrmGetActiveForm();
		FrmDrawForm(form);
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("close quicklist\n");
		break;
	case keyDownEvent:
		WriteLog("Key down\n");
		switch (event->data.keyDown.chr) {
		case vchrCalc:
			/*
			 * simulate we pushed the bulldozer.
			 */
			form = FrmGetActiveForm();
			CtlHitControl(GetObjectPtr(form, gi_buildBulldoze));
			handled = true;
			break;
		}
		break;
	case penUpEvent:
		if (event->screenY < 0) {
			form = FrmGetActiveForm();
			CtlHitControl(GetObjectPtr(form, gi_buildBulldoze));
			handled = true;
		}
		break;
	default:
		break;
	}

	return (handled);
}

/*
 * Get the selected build item.
 */
UInt8
UIGetSelectedBuildItem(void)
{
	return (nSelectedBuildItem);
}

/*
 * Save the selected build item.
 */
void
UISetSelectedBuildItem(UInt8 item)
{
	nSelectedBuildItem = item;
}

/* is Building logic and data */
static UInt8 __state;

/*
 * First the Macro.
 * You Define the Bit, Clearer, Setter and Tester
 * of the Bit field you care about.
 * If you need more than 8 bits (0..7) then change the return type here and
 * in any of the enties shared in the header file.
 */
#define	BUILD_STATEBITACCESSOR(BIT, CLEARER, SETTER, TESTER, VISIBILITY) \
VISIBILITY void \
CLEARER(void) \
{ \
	__state &= ~(1<<(BIT)); \
} \
VISIBILITY void \
SETTER(void) \
{ \
	__state |= (1<<(BIT)); \
} \
VISIBILITY UInt8 \
TESTER(void) \
{ \
	return (__state & (1<<(BIT))); \
}

#define	GLOBAL

BUILD_STATEBITACCESSOR(0, PauseGame, ResumeGame, IsGamePlaying, GLOBAL)
BUILD_STATEBITACCESSOR(1, SetNotBuilding, SetBuilding, IsBuilding, static)
BUILD_STATEBITACCESSOR(2, SetGameNotInProgress, SetGameInProgress,
    IsGameInProgress, GLOBAL)
BUILD_STATEBITACCESSOR(3, SetLowNotShown, SetLowShown, IsLowShown, static)
BUILD_STATEBITACCESSOR(4, SetOutNotShown, SetOutShown, IsOutShown, static)
BUILD_STATEBITACCESSOR(5, ClearNewROM, SetNewROM, IsNewROM, GLOBAL)
BUILD_STATEBITACCESSOR(6, SetDrawing, SetDeferDrawing, IsDeferDrawing, static)

/*
 * Memory of what was clicked under the pen
 * Helps reduce the externally visible state,
 * and allows the query to run without having too much global access.
 */
static UInt8 __clicker;

/*
 * Get the item clicked on the main form last.
 */
UInt8
GetItemClicked()
{
	return (__clicker);
}

/*
 * Set the field value of the item clicked on the form.
 * Saves having to re-lookup the location in the WorldMap.
 */
void
SetItemClicked(UInt8 item)
{
	__clicker = item;
}

/*
 * Check the location clicked on screen to see if it's in the build area.
 * If it is then either build there or perform a query.
 */
void
_UIGetFieldToBuildOn(Int16 x, Int16 y)
{
	RectangleType rect;
	rect.extent.x = vgame.visible_x * vgame.tileSize;
	rect.extent.y = vgame.visible_y * vgame.tileSize;
	rect.topLeft.x = XOFFSET;
	rect.topLeft.y = YOFFSET;

	if (RctPtInRectangle(x, y, &rect)) {
		UInt32 xpos = (x - XOFFSET) / vgame.tileSize + game.map_xpos;
		UInt32 ypos = (y - YOFFSET) / vgame.tileSize + game.map_ypos;
		LockWorld();
		SetItemClicked(WORLDPOS(xpos, ypos));
		UnlockWorld();
		if (UIGetSelectedBuildItem() != Be_Query)
			BuildSomething(xpos, ypos);
		else
			FrmGotoForm(formID_Query);
	}
}

/*
 * Display an error to the user of a psecific error / disaster type
 */
Int16
UIDisplayError(erdiType nError)
{
	/* errors */
	if ((nError > enSTART) && (nError < enEND)) {
		switch (nError) {
		case enOutOfMemory:
			FrmAlert(alertID_errorOutOfMemory);
			break;
		case enOutOfMoney:
			FrmAlert(alertID_outMoney);
			break;
		default:
			break;
		}
		return (0);
	}
	if ((nError > diSTART) && (nError < diEND)) {
		char string[512];
		SysStringByIndex(st_disasters, nError - diFireOutbreak,
		    string, 511);
		if (*string == '\0') StrPrintF(string, "generic disaster??");

		FrmCustomAlert(alertID_generic_disaster, string, 0, 0);
		return (0);
	}
	return (1);
}

/*
 * Display an error that is simply an error string
 */
Int16
UIDisplayError1(char *message)
{
	FrmCustomAlert(alertID_majorbad, message, 0, 0);
	return (0);
}

/*
 * Null Function.
 */
void
UIInitDrawing(void)
{
}

/*
 * Null Function.
 */
void
UIFinishDrawing(void)
{
}

/*
 * The WinScreenLock is 'optional' depending on how much memory you have.
 * the call may or may not succeed.
 * by using these two APIs we can get faster, flickerless allscreen updating
 */
static UInt8 *didLock = NULL;

/*
 * Try to lock the screen from updates.
 * This allows bulk updates to the screen without repainting until unlocked.
 */
void
UILockScreen(void)
{
	if (IsNewROM() && !didLock)
		didLock = WinScreenLock(winLockCopy);
}

/*
 * Unlock the display.
 * Allows any pending updates to proceed.
 */
void
UIUnlockScreen(void)
{
	if (IsNewROM() && didLock) {
		WinScreenUnlock();
		didLock = 0;
	}
}

/*
 * Draw a rectangle on the screen.
 * it will be exactly nHeight*nWidth pixels in size.
 * the frame's left border will be at nTop-1 and so on
 */
void
_UIDrawRect(Int16 nTop, Int16 nLeft, Int16 nHeight, Int16 nWidth)
{
	RectangleType rect;

	rect.topLeft.x = nLeft;
	rect.topLeft.y = nTop;
	rect.extent.x = nWidth;
	rect.extent.y = nHeight;

	_WinDrawRectangleFrame(1, &rect);
}

/*
 * Draw the border around the play area
 */
void
UIDrawBorder()
{
	if (IsDeferDrawing())
		return;

	_UIDrawRect(YOFFSET, XOFFSET, vgame.visible_y * vgame.tileSize,
	    vgame.visible_x * vgame.tileSize);
}

/*
 * Null Function.
 */
void
UISetUpGraphic(void)
{
}

/*
 * Null Function.
 * Would be the tracking cursr on the screen in a bigger environment
 */
void
UIDrawCursor(Int16 xpos, Int16 ypos)
{
}

/*
 * Draw a generic loss icon.
 * It is obtained from the zones bitmap.
 * The location in the zones bitmap is stated by the tilex and tiley values.
 * This specifies the overlay. The icon itself is tile pixels before this.
 */
void
UIDrawLossIcon(Int16 xpos, Int16 ypos, Coord tilex, Coord tiley)
{
	RectangleType rect;

	if (IsDeferDrawing())
		return;

	rect.topLeft.x = tilex;
	rect.topLeft.y = tiley;
	rect.extent.x = vgame.tileSize;
	rect.extent.y = vgame.tileSize;

	/* copy/paste the graphic from the offscreen image */
	/* first draw the overlay */
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    xpos * vgame.tileSize +XOFFSET, ypos * vgame.tileSize + YOFFSET,
	    winErase);
	/* now draw the powerloss icon */
	rect.topLeft.x -= vgame.tileSize;
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    xpos * vgame.tileSize + XOFFSET, ypos * vgame.tileSize + YOFFSET,
	    winOverlay);
}

/*
 * Draw the water loss icon on a field.
 * The Field has already been determined to not have water.
 */
void
UIDrawWaterLoss(Int16 xpos, Int16 ypos)
{
	UIDrawLossIcon(xpos, ypos, 80, 0);
}

/*
 * Draw a power loss icon for the field.
 */
void
UIDrawPowerLoss(Int16 xpos, Int16 ypos)
{
	UIDrawLossIcon(xpos, ypos, 144, 0);
}

/*
 * Draw a special unit at the location chosen
 */
void
UIDrawSpecialUnit(Int16 i, Int16 xpos, Int16 ypos)
{
	RectangleType rect;
	if (IsDeferDrawing())
		return;

	rect.topLeft.x = game.units[i].type*vgame.tileSize;
	rect.topLeft.y = vgame.tileSize;
	rect.extent.x = vgame.tileSize;
	rect.extent.y = vgame.tileSize;

	_WinCopyRectangle(winUnits, WinGetActiveWindow(), &rect,
	    xpos * vgame.tileSize + XOFFSET, ypos * vgame.tileSize + YOFFSET,
	    winErase);
	rect.topLeft.y = 0;
	_WinCopyRectangle(winUnits, WinGetActiveWindow(), &rect,
	    xpos * vgame.tileSize + XOFFSET, ypos * vgame.tileSize + YOFFSET,
	    winOverlay);
}

/*
 * Draw a special object a the location chosen.
 * Mostly monsters, but it coud be a train, palin, chopper
 */
void
UIDrawSpecialObject(Int16 i, Int16 xpos, Int16 ypos)
{
	RectangleType rect;
	if (IsDeferDrawing())
		return;

	rect.topLeft.x = (game.objects[i].dir) * vgame.tileSize;
	rect.topLeft.y = ((i * 2) + 1) * vgame.tileSize;
	rect.extent.x = vgame.tileSize;
	rect.extent.y = vgame.tileSize;

	_WinCopyRectangle(winMonsters, WinGetActiveWindow(), &rect,
	    xpos * vgame.tileSize + XOFFSET, ypos * vgame.tileSize + YOFFSET,
	    winErase);
	rect.topLeft.y -= 16;
	_WinCopyRectangle(winMonsters, WinGetActiveWindow(), &rect,
	    xpos * vgame.tileSize + XOFFSET, ypos * vgame.tileSize + YOFFSET,
	    winOverlay);
}

/*
 * Paint the location on screen with the field.
 */
void
UIDrawField(Int16 xpos, Int16 ypos, UInt8 nGraphic)
{
	RectangleType rect;

	if (IsDeferDrawing())
		return;

	rect.topLeft.x = (nGraphic % 64) * vgame.tileSize;
	rect.topLeft.y = (nGraphic / 64) * vgame.tileSize;
	rect.extent.x = vgame.tileSize;
	rect.extent.y = vgame.tileSize;

	/* copy/paste the graphic from the offscreen image */
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    xpos * vgame.tileSize + XOFFSET, ypos * vgame.tileSize + YOFFSET,
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

	if (IsDeferDrawing())
		return;

	UILockScreen();

	rect.topLeft.x = XOFFSET + vgame.tileSize * (direction == 1);
	rect.topLeft.y = YOFFSET + vgame.tileSize * (direction == 2);
	rect.extent.x =
	    (vgame.visible_x - 1 * (direction == 1 || direction == 3)) *
	    vgame.tileSize;
	rect.extent.y =
	    (vgame.visible_y - 1 * (direction == 0 || direction == 2)) *
	    vgame.tileSize;
	to_x = XOFFSET + vgame.tileSize * (direction == 3);
	to_y = YOFFSET + vgame.tileSize * (direction == 0);


	screen = WinGetActiveWindow();
	_WinCopyRectangle(screen, screen, &rect, to_x, to_y, winPaint);

	/* and lastly, fill the gap */
	LockWorld();
	LockWorldFlags();
	UIInitDrawing();

	if (direction == 1 || direction == 3) {
		for (i = game.map_ypos;
		    i < vgame.visible_y + game.map_ypos; i++) {
			DrawFieldWithoutInit(game.map_xpos +
			    (vgame.visible_x - 1) * (direction == 1), i);
		}
	} else {
		for (i = game.map_xpos;
		    i < vgame.visible_x + game.map_xpos; i++) {
			DrawFieldWithoutInit(i, game.map_ypos +
			    (vgame.visible_y - 1) * (direction == 2));
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

/*
 * Get a random number.
 */
UInt32
GetRandomNumber(UInt32 max)
{
	if (max == 0)
		return (0);
	return ((UInt16)SysRandom(0) % (UInt16)max);
}

/*
 * Layout (current|lores)
 * [Tool]			 [date]		[J][Speed]
 * [							]
 * [		Play Area				]
 * [							]
 * [money]			[pop]		   [loc]
 *
 * Layout (current|hires)
 * [tool]	[	Toolbar		  ]	[J][Speed]
 * [			Play Area			]
 * [Date]	[Money]	 [pop]			[loc]
 *
 * The J is for 'jog' item it's either updn or ltrt
 */

#define	DATELOC		0
#define	CREDITSLOC	1
#define	POPLOC		2
#define	POSITIONLOC	3

#define	MIDX	1
#define	MIDY	2
#define	ENDX	4
#define	ENDY	8

struct StatusPositions {
	RectangleType rect;
	PointType offset;
	UInt32 extents;
};

/*
 * extents.x is filled in by initTextPositions
 */
static struct StatusPositions lrpositions[] = {
	{ { {0, 0}, {0, 11} }, {0, 1}, MIDX },  /* DATELOC */
	{ { {0, 0}, {0, 11} }, {0, 1}, ENDY }, /* CREDITSLOC */
	{ { {0, 0}, {0, 11} }, {0, 1}, MIDX | ENDY }, /* POPLOC */
	{ { {0, 0}, {0, 11} }, {0, 1}, ENDX | ENDY }, /* POSITIONLOC */
};
#ifdef SONY_CLIE

/*
 * High resolution version of the positions
 */
static struct StatusPositions hrpositions[] = {
	{ { {2, 0}, {0, 11} }, {0, 1}, ENDY },  /* DATELOC */
	{ { {80, 0}, {0, 11} }, {0, 1}, ENDY }, /* CREDITSLOC */
	{ { {160, 0}, {0, 11} }, {0, 1}, ENDY }, /* POPLOC */
	{ { {280, 0}, {0, 11} }, {0, 1}, ENDY }, /* POSITIONLOC */
};

/*
 * Get the valid array of status positions
 * Depends on if we're in High Resolution mode or not
 */
static struct StatusPositions *
posAt(int pos)
{
	static struct StatusPositions *sp = NULL;
	if (sp != NULL)
		return (&(sp[pos]));
	if (isHires())
		sp = &(hrpositions[0]);
	else
		sp = &(lrpositions[0]);
	return (&(sp[pos]));
}

#else
#define	posAt(x)	&(lrpositions[(x)])
#endif
#define	MAXLOC		(sizeof (lrpositions) / sizeof (lrpositions[0]))

/*
 * only the topLeft.y location is a 'constant'
 */
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

/*
 * Draw a status item at the position requested
 */
void
UIDrawItem(Int16 location, char *text)
{
	struct StatusPositions *pos;
	Int16 sl;
	Coord tx;

	if ((location < 0) || (location >= MAXLOC)) {
		Warning("UIDrawitem request for item out of bounds");
		return;
	}
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

/*
 * Check a click in the display.
 * See if it's one of the status locations. If it is then return it.
 */
static Int16
UICheckOnClick(Coord x, Coord y)
{
	int i;
	struct StatusPositions *pos;
	for (i = 0; i < MAXLOC; i++) {
		RectangleType *rt;
		pos = posAt(i);
		rt = &(pos->rect);
		if ((x >= rt->topLeft.x) &&
		    (x <= (rt->topLeft.x + rt->extent.x)) &&
		    (y >= rt->topLeft.y) &&
		    (y <= (rt->topLeft.y + rt->extent.y)))
			return (i);
	}
	return (-1);
}

/*
 * Perform an action based on the location on screen that was clicked.
 */
static void
CheckTextClick(Coord x, Coord y)
{
	int t = UICheckOnClick(x, y);
	if (t == -1)
		return;
	switch (t) {
	case DATELOC:
		break;
	case CREDITSLOC:
		doButtonEvent(BeBudget);
		break;
	case POPLOC:
		doButtonEvent(BePopulation);
		break;
	case POSITIONLOC:
		doButtonEvent(BeMap);
		break;
	default:
		break;
	}
}

/*
 * Draw the date on screen.
 * XXX: Nasty, needs localization
 */
void
UIDrawDate(void)
{
	char temp[23];

	if (IsDeferDrawing())
		return;

	GetDate((char *)temp);
	UIDrawItem(DATELOC, temp);
}

/*
 * Draw the amount of credits on screen.
 */
void
UIDrawCredits(void)
{
	char temp[23];
#ifdef SONY_CLIE
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	if (IsDeferDrawing())
		return;

	StrPrintF(temp, "$: %ld", game.credits);
	UIDrawItem(CREDITSLOC, temp);
#ifdef SONY_CLIE
	if (isHires()) {
		bitmapHandle = DmGetResource('Tbmp', bitmapID_coin);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		_WinDrawBitmap(bitmap, 68, sHeight - 11);
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	}
#endif
	UIDrawDate();
}

/*
 * Draw the map position on screen
 */
void
UIDrawLoc(void)
{
	char temp[25];
#ifdef SONY_CLIE
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	if (IsDeferDrawing())
		return;

#ifdef SONY_CLIE
	if (isHires()) {
		StrPrintF(temp, "%02u,%02u", game.map_xpos, game.map_ypos);
		bitmapHandle = DmGetResource('Tbmp', bitmapID_loca);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		_WinDrawBitmap(bitmap, 270, sHeight - 11);
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	} else
#endif
		StrPrintF(temp, "(%02u,%02u)", game.map_xpos, game.map_ypos);

#ifdef SONY_CLIE
	bitmapHandle = DmGetResource('Tbmp', bitmapID_updn + jog_lr);
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

/*
 * Update the build icon on screen
 */
void
UIUpdateBuildIcon(void)
{
	MemHandle bitmaphandle;
	BitmapPtr bitmap;
	if (IsDeferDrawing())
		return;

	bitmaphandle = DmGetResource('Tbmp',
	    bitmapID_iconBulldoze + (((nSelectedBuildItem <= Be_Extra)) ?
	    nSelectedBuildItem : OFFSET_EXTRA));

	if (bitmaphandle == NULL)
		/* TODO: onscreen error? +save? */
		return;
	bitmap = MemHandleLock(bitmaphandle);
	_WinDrawBitmap(bitmap, 2, 2);
	MemPtrUnlock(bitmap);
	DmReleaseResource(bitmaphandle);
#if defined(SONY_CLIE)
	if (isHires())
		UIDrawToolBar();
#endif
}

/*
 * Draw the speed icon on screen
 */
void
UIDrawSpeed(void)
{
	MemHandle  bitmaphandle;
	BitmapPtr  bitmap;

	bitmaphandle = DmGetResource('Tbmp',
	    bitmapID_SpeedPaused + game.gameLoopSeconds);
	if (bitmaphandle == NULL)
		/* TODO: onscreen error? +save? */
		return;
	bitmap = MemHandleLock(bitmaphandle);
	_WinDrawBitmap(bitmap, sWidth - 12, 2);
	MemPtrUnlock(bitmap);
	DmReleaseResource(bitmaphandle);
}

/*
 * Draw the population on screen.
 * As a side effect also draws the population, Location and Speed to screen
 */
void
UIDrawPop(void)
{
	char temp[25];
#ifdef SONY_CLIE
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	if (IsDeferDrawing())
		return;

	StrPrintF(temp, "Pop: %lu", vgame.BuildCount[COUNT_RESIDENTIAL] * 150);
	UIDrawItem(POPLOC, temp);
	UIDrawLoc();
	UIDrawSpeed();
#ifdef SONY_CLIE
	if (isHires()) {
		bitmapHandle = DmGetResource('Tbmp', bitmapID_popu);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		_WinDrawBitmap(bitmap, 146, sHeight - 11);
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	}
#endif
}

/*
 * Check if we've got dosh to do what we're asking to do.
 * If we're nearly broke show us a warning dialog.
 * If we're broke then show us a we're broke dialog.
 * Only show them once per game.
 */
void
UICheckMoney(void)
{
	if (game.credits == 0) {
		if (!IsOutShown()) {
			FrmAlert(alertID_outMoney);
			SetOutShown();
		} else {
			return;
		}
	} else if ((game.credits <= 1000) || (game.credits == 1000)) {
		if (!IsLowShown()) {
			FrmAlert(alertID_lowFunds);
			SetLowShown();
		} else {
			return;
		}
	}
}

/* memory handlers */
Int16
InitWorld(void)
{
	worldHandle = MemHandleNew(10);
	worldFlagsHandle = MemHandleNew(10);
	WriteLog("Allocation initial 20 bytes\n");

	if (worldHandle == 0 || worldFlagsHandle == 0) {
		UIDisplayError(enOutOfMemory);
		WriteLog("InitWorld FAILED alloc!\n");
		return (0);
	}
	return (1);
}


Int16
ResizeWorld(UInt32 size)
{
	WriteLog("Allocating bytes: %li\n", (unsigned long)size);

	if (MemHandleResize(worldHandle, size) != 0 ||
	    MemHandleResize(worldFlagsHandle, size) != 0) {
		UIDisplayError(enOutOfMemory);
		/* QuitGameError(); */
		WriteLog("resizeWorld FAILED alloc!\n");
		return (0);
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

UInt8
LockedWorld()
{
	return (worldPtr != NULL);
}

void
LockWorld()
{
#ifdef DEBUG
	if (worldPtr != NULL)
		DbgBreak();
#endif
	worldPtr = MemHandleLock(worldHandle);
}

void
UnlockWorld()
{
#ifdef DEBUG
	if (worldPtr == NULL)
		DbgBreak();
#endif
	MemHandleUnlock(worldHandle);
	worldPtr = NULL;
}

UInt8
LockedWorldFlags()
{
	return (worldFlagsPtr != NULL);
}

void
LockWorldFlags()
{
#ifdef DEBUG
	if (worldFlagsPtr != NULL)
		DbgBreak();
#endif
	worldFlagsPtr = MemHandleLock(worldFlagsHandle);
}

void
UnlockWorldFlags()
{
#ifdef DEBUG
	if (worldFlagsPtr == NULL)
		DbgBreak();
#endif
	MemHandleUnlock(worldFlagsHandle);
	worldFlagsPtr = NULL;
}

UInt8
GetWorldFlags(UInt32 pos)
{
	/* NOTE: LockWorld() MUST have been called before this is used!!! */
	if (pos >= GetMapMul()) {
		WriteLog("Get Flags out of range\n");
		return (0);
	}
	return (((UInt8 *)worldFlagsPtr)[pos]);
}

void
SetWorldFlags(UInt32 pos, UInt8 value)
{
	if (pos >= GetMapMul()) {
		WriteLog("Write to flags out of range\n");
		return;
	}
	((UInt8 *)worldFlagsPtr)[pos] = value;
}

void
OrWorldFlags(UInt32 pos, UInt8 value)
{
	if (pos > GetMapMul())
		return;
	((UInt8 *)worldFlagsPtr)[pos] |= value;
}

void
AndWorldFlags(UInt32 pos, UInt8 value)
{
	if (pos > GetMapMul())
		return;
	((UInt8 *)worldFlagsPtr)[pos] &= value;
}

UInt8
GetWorld(UInt32 pos)
{
	/* NOTE: LockWorld() MUST have been called before this is used!!! */
	if (pos >= GetMapMul()) {
		WriteLog("Get World out of range\n");
		return (0);
	}
	return (((UInt8 *)worldPtr)[pos]);
}

void
SetWorld(UInt32 pos, UInt8 value)
{
	if (pos > GetMapMul())
		return;
	((UInt8 *)worldPtr)[pos] = value;
}

static Err
RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	/* See if we're on in minimum required version of the ROM or later. */
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

	WriteLog("Rom Version: 0x%lx\n", (unsigned long)romVersion);

	if (romVersion < requiredVersion) {
		if ((launchFlags &
		    (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
		    (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {
			if (romVersion > sysMakeROMVersion(3, 1, 0, 0, 0) &&
			    FrmAlert(alertID_RomIncompatible) == 1) {
				ClearNewROM();
				return (0);
			}

			/*
			 * Pilot 1.0 will continuously relaunch this app
			 * unless we switch to another safe one.
			 */
			if (romVersion < sysMakeROMVersion(2, 0, 0, 0, 0)) {
				AppLaunchWithCommand(sysFileCDefaultApp,
				    sysAppLaunchCmdNormalLaunch, NULL);
			}
		}
		return (sysErrRomIncompatible);
	}
	SetNewROM();
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

/* Do the command against the key passed */
static Int16
doButtonEvent(ButtonEvent event)
{
	switch (event) {
	case BeIgnore:
		break;
	case BeUp:
		ScrollMap(dtUp);
		break;
	case BeDown:
		ScrollMap(dtDown);
		break;
	case BeLeft:
		ScrollMap(dtLeft);
		break;
	case BeRight:
		ScrollMap(dtRight);
		break;
	case BePopup:
		UIDoQuickList();
		break;
	case BeMap:
		FrmGotoForm(formID_map);
		break;
	case BeBudget:
		FrmGotoForm(formID_budget);
		break;
	case BePopulation:
		/* No-Op at moment */
		break;
	case BePassthrough:
		return (0);
#ifdef SONY_CLIE
	case BeJogUp:
		if (!IsDrawWindowMostOfScreen())
			return (1);
		if (jog_lr)
			ScrollMap(dtLeft);
		else
			ScrollMap(dtUp);
		break;
	case BeJogDown:
		if (!IsDrawWindowMostOfScreen())
			return (1);
		if (jog_lr)
			ScrollMap(dtRight);
		else
			ScrollMap(dtDown);
		break;
	case BeJogRelease:
		if (!IsDrawWindowMostOfScreen())
			return (1);
		jog_lr = 1 - jog_lr;
		UIDrawLoc();
		break;
#endif
	default:
		break;
	}
	return (1);
}

static Int16 HardButtonEvent(ButtonKey key)
{
	return (doButtonEvent(gameConfig.pc.keyOptions[key]));
}

static struct _silkKeys {
	UInt16 vChar;
	ButtonKey event;
} silky[] = {
	{ pageUpChr, BkHardUp },
	{ pageDownChr, BkHardDown },
	{ vchrHard1, BkCalendar },
	{ vchrHard2, BkAddress },
	{ vchrHard3, BkToDo },
	{ vchrHard4, BkMemo },
	{ vchrFind, BkFind },
	{ 1, BkCalc },
#ifdef SONY_CLIE
	{ vchrJogUp, BkJogUp },
	{ vchrJogDown, BkJogDown },
	{ vchrJogRelease, BkJogRelease },
#endif
	{ 0, 0 }
};

static void
buildSilkList()
{
	UInt16 btncount = 0;
	UInt16 atsilk = 0;
	UInt16 atbtn = 0;

	const PenBtnInfoType *silkinfo = EvtGetPenBtnList(&btncount);
	/* favorites / find */
	while (silky[atsilk].vChar != 1) atsilk++;
	while (atbtn < btncount) {
#if defined(DEBUG)
		WriteLog(log, "btn: %ld char: %lx\n", (long)atbtn,
		    (long)silkinfo[atbtn].asciiCode);
#endif
		if (silkinfo[atbtn].asciiCode == vchrFind) {
			if (atbtn > 0) {
				silky[atsilk].vChar =
				    silkinfo[atbtn-1].asciiCode;
				break;
			}
		}
		atbtn++;
	}
	if (silky[atsilk].vChar == 1) silky[atsilk].vChar = vchrCalc;
}

static Int16
vkDoEvent(UInt16 key)
{
	struct _silkKeys *atsilk = &(silky[0]);
	while (atsilk->vChar != 0) {
		if (key == atsilk->vChar)
			return (HardButtonEvent(atsilk->event));
		atsilk++;
	}
	return (0);

}

#ifdef SONY_CLIE
static BitmapType *pToolbarBitmap = NULL;

static void
clearToolbarBitmap(void)
{
	if (pToolbarBitmap != NULL) {
		BmpDelete(pToolbarBitmap);
		pToolbarBitmap = NULL;
	}
}

static void
drawToolBitmaps(Coord startx, Coord starty)
{
	MemHandle hBitmap;
	MemPtr pBitmap;
	UInt32 id;

	for (id = bitmapID_iconBulldoze; id <= bitmapID_iconExtra; id++) {
		hBitmap = DmGetResource('Tbmp', id);
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

#define	TBWIDTH ((Coord)(14 *(bitmapID_iconExtra+1 - bitmapID_iconBulldoze)))

static void
UIDrawToolBar(void)
{
	WinHandle wh = NULL;
	WinHandle owh = NULL;

	if (pToolbarBitmap == NULL) {
		UInt16 err;
		pToolbarBitmap = _BmpCreate(TBWIDTH, 12, getDepth(),
		    NULL, &err);
		if (pToolbarBitmap != NULL) {
			wh = _WinCreateBitmapWindow(pToolbarBitmap, &err);
			if (wh != NULL) {
				owh = WinSetDrawWindow(wh);
				drawToolBitmaps(0, 0);
				WinSetDrawWindow(owh);
				WinDeleteWindow(wh, false);
			}
		} else {
			drawToolBitmaps((sWidth - TBWIDTH) >> 1, 2);
			return;
		}
	}
	if (pToolbarBitmap != NULL) {
		_WinDrawBitmap(pToolbarBitmap, (sWidth - TBWIDTH) >> 1, 2);
	}
}

static void
toolBarCheck(Coord xpos)
{
	int id;
	/* We've already confirmed the y-axis. */
	if (xpos < ((sWidth - TBWIDTH) >> 1) ||
	    xpos > ((sWidth + TBWIDTH) >> 1)) return;

	id = (xpos - ((sWidth - TBWIDTH) >> 1)) / 14;
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

#include <unix_stdarg.h>

void
WriteLog(char *s, ...)
{
	va_list args;
	HostFILE * hf = NULL;
	Char text[0x100];

	va_start(args, s);

	hf = HostFOpen("\\pcity.log", "a");
	if (hf) {
		StrVPrintF(text, s, args);

		HostFPrintF(hf, text);
		HostFClose(hf);
	}
	va_end(args);
}
#endif
