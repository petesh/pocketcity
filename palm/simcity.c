#include <PalmOS.h>
#include <StringMgr.h>
#include <KeyMgr.h>
#include <StdIOPalm.h>
#include <simcity.h>
#include <savegame.h>
#include <savegame_be.h>
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

static RectangleType rPlayGround;

/*! \brief the on screen zones */
static WinHandle winZones;
/*! \brief the Monsters */
static WinHandle winMonsters;
/*! \brief The defensive units */
static WinHandle winUnits;

/*! \brief the offscreen window containing the speed images */
static WinHandle winSpeeds;

BuildCode nSelectedBuildItem = Be_Bulldozer;
BuildCode nPreviousBuildItem = Be_Bulldozer;

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

static void _UIGetFieldToBuildOn(Int16 x, Int16 y);
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);
static void EventLoop(void);
static void cycleSpeed(void);
static void DoAbout(void);
static void CheckTextClick(Coord x, Coord y);
static Int16 doButtonEvent(ButtonEvent key);
static Int16 speedOffset(void);

#if defined(SONY_CLIE)
static void HoldHook(UInt32);
#endif

#if defined(HRSUPPORT)
static void toolBarCheck(Coord);
static void UIDrawToolBar(void);
static void freeToolbarBitmap(void);
static void pcResizeDisplay(Boolean draw);
#else
#define	freeToolbarBitmap()
#define pcResizeDisplay(X)
#endif

/* Collects what would otherwise be several variables */
static UInt8 IsBuilding(void);
static void SetBuilding(void);
static void SetNotBuilding(void);
static void SetLowNotShown(void);
static void SetOutNotShown(void);
static void SetDeferDrawing(void);
static void SetDrawing(void);
static UInt8 IsDeferDrawing(void);

void UIDrawSpeed(void);

void
ResGetString(UInt16 index, char *buffer, UInt16 length)
{
	if (NULL == SysStringByIndex(resstrings_base, index, buffer, length)) {
		/* XXX: Error here ! */
	}
}

/*
 * The PilotMain routine.
 * Needed by all palm applications. It checks the rom version and
 * Throws out to the user of it's too old.
 * Handles the auto-saving of the application at the end.
 */
UInt32
PilotMain(UInt16 cmd, MemPtr cmdPBP __attribute__((unused)), UInt16 launchFlags)
{
	UInt16 error;
	Int16 pir;

	switch (cmd) {
	case sysAppLaunchCmdNormalLaunch:
		break;
	default:
		return (0);
		break;
	}

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
	static UInt16 oldFormID = ~((UInt16)0);
	FormPtr form;
	UInt16 formID;

	if (event->eType == frmLoadEvent) {
		formID = event->data.frmLoad.formID;
		WriteLog("Main::frmLoadEvent: %d -> %d\n", (int)oldFormID,
		    (int)formID);
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

#if defined(DEBUG)
static char *cstrings[] = {
	"bc_count_residential", /*!< count of residential areas */
	"bc_value_residential", /*!< values of residential units */
	"bc_count_commercial", /*!< count of the commercial units */
	"bc_value_commercial", /*!< value of commercial units */
	"bc_count_industrial", /*!< count of industrial units */
	"bc_value_industrial", /*!< value of industrial units */
	"bc_count_roads", /*!< count of roads */
	"bc_value_roads", /*!< value of roads */
	"bc_count_trees", /*!< count of trees/forests/parks */
	"bc_value_trees", /*!< value of trees/forests/parks (unnatural) */
	"bc_water", /*!< count of water (unnatural) */
	"bc_coalplants", /*!< count of coal power plants */
	"bc_nuclearplants", /*!< count of nuclear power plants */
	"bc_powerlines", /*!< count of power lines */
	"bc_waterpumps", /*!< count of water pumps */
	"bc_waterpipes", /*!< count of water pipes */
	"bc_waste", /*!< count of wasteland zones */
	"bc_radioactive", /*< count of radioactive areas */
	"bc_fire", /*!< count of fire elements */
	"bc_fire_stations", /*!< count of fire stations */
	"bc_police_departments", /*!< count of police stations */
	"bc_military_bases", /*!< count of military bases */
	"bc_cashflow", /*!< cashflow value */
	"bc_pollution", /*!< pollution valuation */
	"bc_crime" /*!< Criminal level */
};
#endif /* bcstrings */
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

		if (MenuHandleEvent((MenuBarType *)0, &event, &err)) continue;

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
		if (getLoopSeconds() != SPEED_PAUSED) {
			if (simState == 0) {
				timeTemp = TimGetSeconds();
				if (timeTemp >=
				    timeStamp + getLoopSeconds()) {
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

		if (getDifficultyLevel() == 0 &&
		    getLoopSeconds() == SPEED_PAUSED) {
			continue;
		}

		timeTemp = TimGetSeconds();

		if (timeTemp >= (timeStampDisaster + SIM_GAME_LOOP_DISASTER)) {
#ifdef DEBUG
			Int16 q, pc = 0;
#endif
			MoveAllObjects();
#ifdef DEBUG
			/*
			 * This will print the BuildCount array
			 * don't forget to keep an eye on it
			 * in the log - should be up-to-date
			 * AT ALL TIMES!
			 */
			for (q = 0; q < bc_tail; q++)
				if ((long)vgame.BuildCount[q] != 0) {
					WriteLogX("%s=%li ", cstrings[q], 
					    (long)vgame.BuildCount[q]);
					pc++;
					if (pc == 3) {
						pc = 0;
						WriteLogX("\n");
					}
				}
			WriteLogX("\n");
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
 * Handles and resourceID's of the bitmaps
 */
static const struct _bmphandles {
	WinHandle *handle;
	const DmResID resourceID;
} handles[] = {
	{ &winZones, (DmResID)bitmapID_zones },
	{ &winMonsters, (DmResID)bitmapID_monsters },
	{ &winUnits, (DmResID)bitmapID_units },
//XXX:	{ &winButtons, (DmResID)bitmapID_buttons },
	{ &winSpeeds, (DmResID)bitmapID_Speed }
};

static DmOpenRef _refTiles;
/*
 * Set up the game in relation to the screen mode.
 */
static Int16
_PalmInit(void)
{
	UInt32		depth;
	Int16		err;
	UInt16		i;
	MemHandle	bitmaphandle;
	BitmapPtr	bitmap;
	WinHandle	winHandle = NULL;
	WinHandle	privhandle;
	UInt16		prefSize;
	Int16		rv = 0;
	Coord		width;
	Coord		height;

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
			changeDepthRes(8, true);
		} else if ((depth & (1 << (4-1))) != 0) {
			/* 4bpp (greyscale) is supported */
			changeDepthRes(4, true);
		}
		/* falls through if you've no color */
	}

	/* The 'playground'... built by the size of the screen */
	rPlayGround.topLeft.x = 0;
	rPlayGround.topLeft.y = 15; /* Padding for the menubar */
	rPlayGround.extent.x = sWidth;
	rPlayGround.extent.y = sHeight - 2*16; /* Space on the bottom */

	/* section (4) */

	/* Section (5); */
	/* create an offscreen window, and copy the zones to be used later */
	for (i = 0; i < (sizeof (handles) / sizeof (handles[0])); i++) {
		bitmaphandle = DmGetResource(TBMP, handles[i].resourceID);
		if (bitmaphandle == NULL) {
			WriteLog("could not get bitmap handle[%d:%ld]\n",
			    (int)i, (long)handles[i].resourceID);
			rv = 5;
			goto returnWV;
		}
		bitmap = (BitmapPtr)MemHandleLock(bitmaphandle);
		if (bitmap == NULL) {
			WriteLog("MemHandleLock Failed handle[%d:%ld]\n",
			    (int)i, (long)handles[i].resourceID);
			DmReleaseResource(bitmaphandle);
			rv = 5;
			goto returnWV;
		}
		compatBmpGetDimensions(bitmap, &width, &height, NULL);

		privhandle = _WinCreateOffscreenWindow(width,
		    height, nativeFormat, (UInt16 *)&err);
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
	UInt16 i;

	unhookHoldSwitch();
	freeToolbarBitmap();
	EndSilk();

	switch (reached) {
	case 0:
		PurgeWorld();
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
		game.credits += 10000;
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
		DoSpecificDisaster((erdiType)(itemID - mi_CauseFire +
			    diFireOutbreak));
		handled = true;
		break;

		/* next menu ... speed */

	case menuID_SlowSpeed:
		setLoopSeconds(SPEED_SLOW);
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_MediumSpeed:
		setLoopSeconds(SPEED_MEDIUM);
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_FastSpeed:
		setLoopSeconds(SPEED_FAST);
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_TurboSpeed:
		setLoopSeconds(SPEED_TURBO);
		UIDrawSpeed();
		handled = true;
		break;
	case menuID_PauseSpeed:
		setLoopSeconds(SPEED_PAUSED);
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
		FrmDrawForm(form);
		SetSilkResizable(form, true);
		collapseMove(form, CM_DEFAULT, NULL, NULL);
		pcResizeDisplay(false);
		SetGameInProgress();
		ResumeGame();
		FrmDrawForm(form);
		SetDrawing();
		DrawGame(1);
		handled = true;
		break;
	case frmCloseEvent:
		SetSilkResizable(NULL, false);
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
				UIDrawSpeed();
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
				UIDrawBuildIcon();
				break;
			}
#if defined(HRSUPPORT)
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
		timeStamp = TimGetSeconds() - getLoopSeconds() + 2;
		/* so the simulation routine won't kick in right away */
		timeStampDisaster = timeStamp - getLoopSeconds() + 1;
		handled = true;
		break;
	case menuEvent:
		WriteLog("Menu Item: %d\n", (int)event->data.menu.itemID);
		handled = DoPCityMenuProcessing(event->data.menu.itemID);

	case keyDownEvent:
		handled = vkDoEvent(event->data.keyDown.chr);
		break;
#if defined(HRSUPPORT)
	case winDisplayChangedEvent:
#if defined(SONY_CLIE)
	case vchrSilkResize:
#endif
		pcResizeDisplay(collapseMove(FrmGetActiveForm(), CM_DEFAULT,
		    NULL, NULL));
		handled = true;
		break;
#endif
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
	const UInt8 *qq = (const UInt8 *)"??";

	vh = DmGetResource(TVER, 1);
	if (vh != NULL) vs = MemHandleLock(vh);
	if (vh == NULL) vs = (MemPtr)qq;
	bh = DmGetResource(TSTR, StrID_build);
	if (bh != NULL) bs = MemHandleLock(bh);
	if (bh == NULL) bs = (MemPtr)qq;
	FrmCustomAlert(alertID_about, (const Char *)vs, (const Char *)bs, NULL);
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
	LstSetListChoices((ListPtr)GetObjectPtr(form, listID_extraBuildList),
	    lp, poplen);

	UpdateDescription(0);
	sfe = FrmDoDialog(form);

	FreeStringList(lp);

	switch (sfe) {
	case buttonID_extraBuildSelect:
		/* List entries must match entries in BuildCode 0 .. */
		nSelectedBuildItem = (BuildCode)LstGetSelection(
		    (ListPtr)GetObjectPtr(form, listID_extraBuildList));
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

	WriteLog("sfe = %u, bi = %u\n", (unsigned int)sfe,
	    (unsigned int)nSelectedBuildItem);

	CleanUpExtraBuildForm();
	FrmDeleteForm(form);
}

/*
 * Update the description of the item in the extra build list.
 * XXX: Use MemHandles instead?
 */
FieldPtr
UpdateDescription(Int16 sel)
{
	Int16 cost;
	Char *temp = (Char *)MemPtrNew(256);
	Int16 *ch;
	MemHandle mh;
	FormType *fp;
	FieldPtr ctl;

	fp = FrmGetFormPtr(formID_extraBuild);
	ctl = (FieldPtr)GetObjectPtr(fp, labelID_extraBuildDescription);

	SysStringByIndex(strID_Descriptions, sel, temp, 256);
	FldSetTextPtr(ctl, temp);
	FldRecalculateField(ctl, true);

	mh = DmGetResource('wrdl', wdlID_Costs);
	if (mh != NULL) {
		ch = (Int16 *)MemHandleLock(mh);
		cost = ch[sel+1];
		MemHandleUnlock(mh);
		DmReleaseResource(mh);
	} else {
		cost = -1;
	}
	temp = (Char *)MemPtrNew(16);
	ctl = (FieldPtr)GetObjectPtr(fp, labelID_extraBuildPrice);
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
	    (FieldPtr)GetObjectPtr(form, labelID_extraBuildDescription));
	if (ptr != 0)
		MemPtrFree(ptr);
	ptr = (void*)FldGetTextPtr((FieldPtr)GetObjectPtr(form,
		    labelID_extraBuildPrice));
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
			ptr = (void *)FldGetTextPtr((FieldPtr)GetObjectPtr(form,
				    labelID_extraBuildDescription));
			ptr2 = (void *)FldGetTextPtr(
			    (FieldPtr)GetObjectPtr(form,
				labelID_extraBuildPrice));
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
			CtlHitControl((ControlPtr)GetObjectPtr(
			    FrmGetActiveForm(), buttonID_extraBuildCancel));
			handled = true;
			break;
		case pageUpChr:
			LstScrollList((ListPtr)GetObjectPtr(FrmGetActiveForm(),
			    listID_extraBuildList), winUp, 4);
			handled = true;
			break;
		case pageDownChr:
			LstScrollList((ListPtr)GetObjectPtr(FrmGetActiveForm(),
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
		nSelectedBuildItem = (BuildCode)(FrmDoDialog(ftList) -
		    gi_buildBulldoze);

		if (nSelectedBuildItem >= OFFSET_EXTRA)
			UIPopUpExtraBuildList();
		UIDrawBuildIcon();
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
			CtlHitControl((ControlPtr)GetObjectPtr(form,
			    gi_buildBulldoze));
			handled = true;
			break;
		}
		break;
	case penUpEvent:
		if (event->screenY < 0) {
			form = FrmGetActiveForm();
			CtlHitControl((ControlPtr)GetObjectPtr(form,
			    gi_buildBulldoze));
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
BuildCode
UIGetSelectedBuildItem(void)
{
	return (nSelectedBuildItem);
}

/*
 * Save the selected build item.
 */
void
UISetSelectedBuildItem(BuildCode item)
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
 * in any of the entries shared in the header file.
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
BUILD_STATEBITACCESSOR(7, ClearDirectBmps, SetDirectBmps, IsDirectBmps, GLOBAL)

/*
 * Memory of what was clicked under the pen
 * Helps reduce the externally visible state,
 * and allows the query to run without having too much global access.
 */
static UInt32 __clicker;

/*
 * Get the item clicked on the main form last.
 */
UInt32
GetPositionClicked()
{
	return (__clicker);
}

/*
 * Set the field value of the item clicked on the form.
 * Saves having to re-lookup the location in the WorldMap.
 */
void
SetPositionClicked(UInt32 item)
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
	rect.extent.x = vgame.visible_x * gameTileSize();
	rect.extent.y = vgame.visible_y * gameTileSize();
	rect.topLeft.x = XOFFSET;
	rect.topLeft.y = YOFFSET;

	if (RctPtInRectangle(x, y, &rect)) {
		UInt32 xpos = (x - XOFFSET) / gameTileSize() + getMapXPos();
		UInt32 ypos = (y - YOFFSET) / gameTileSize() + getMapYPos();
		LockZone(lz_world);
		SetPositionClicked(WORLDPOS(xpos, ypos));
		UnlockZone(lz_world);
		if (UIGetSelectedBuildItem() != Be_Query)
			BuildSomething(xpos, ypos);
		else
			FrmGotoForm(formID_Query);
	}
}

/*
 * Display an error to the user of a specific error / disaster type
 */
void
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
	}
	if ((nError > diSTART) && (nError < diEND)) {
		char string[512];
		SysStringByIndex(st_disasters, nError - diFireOutbreak,
		    string, 511);
		if (*string == '\0') StrPrintF(string, "generic disaster??");

		FrmCustomAlert(alertID_generic_disaster, string, 0, 0);
	}
}

/*
 * Display an error that is simply an error string
 */
void
UIDisplayError1(char *message)
{
	FrmCustomAlert(alertID_majorbad, message, 0, 0);
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

/*!
 * \brief handle for checking in the winscfeenunlock function
 * The WinScreenLock is 'optional' depending on how much memory you have.
 * the call may or may not succeed.
 * by using these two APIs we can get faster, flickerless allscreen updating
 */
static UInt8 *didLock = NULL;
/*!\ brief counter of number of calls to lock screen */
static UInt8 lockCalls = 0;

/*!
 * \Try to lock the screen from updates.
 *
 * This allows bulk updates to the screen without repainting until unlocked.
 */
void
UILockScreen(void)
{
	if (!IsNewROM()) return;
	ErrFatalDisplayIf(lockCalls > 0, "double lock on screen attempted");
	if (!didLock)
		didLock = WinScreenLock(winLockCopy);
	lockCalls++;
}

/*
 * Unlock the display.
 * Allows any pending updates to proceed.
 */
void
UIUnlockScreen(void)
{
	if (!IsNewROM()) return;
	ErrFatalDisplayIf(lockCalls == 0, "double free on screen attempted");
	if (didLock != NULL) {
		WinScreenUnlock();
		didLock = NULL;
	}
	lockCalls--;
}

/*
 * Get a month string
 */
char *
getMonthString(UInt16 month, char *string, UInt16 maxlen)
{
	SysStringByIndex(strID_Months, month, string, maxlen);
	return (string);
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

	
	StartHiresDraw();
	_WinDrawRectangleFrame(1, &rect);
	EndHiresDraw();
}

/*
 * Draw the border around the play area
 */
void
UIDrawBorder()
{
	if (IsDeferDrawing())
		return;

	_UIDrawRect(YOFFSET, XOFFSET, vgame.visible_y * gameTileSize(),
	    vgame.visible_x * gameTileSize());
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
 * Would be the tracking cursor on the screen in a bigger environment
 */
void
UIDrawCursor(Int16 xpos __attribute__ ((unused)),
    Int16 ypos __attribute__((unused)))
{
}

/*!
 * \brief Draw a generic loss icon.
 * 
 * It is obtained from the zones bitmap.
 * The location in the zones bitmap is stated by the tilex and tiley values.
 * This specifies the overlay. The icon itself is tile pixels before this.
 * \param xpos the x position on the area to paint
 * \param ypos the y position on the area to paint
 */
void
UIDrawLossIcon(Int16 xpos, Int16 ypos, welem_t elem)
{
	RectangleType rect;

	if (IsDeferDrawing())
		return;

	rect.topLeft.x = (Coord)(elem % HORIZONTAL_TILESIZE) * gameTileSize();
	rect.topLeft.y = (Coord)(elem / HORIZONTAL_TILESIZE) * gameTileSize();
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	/* copy/paste the graphic from the offscreen image */
	StartHiresDraw();
	/* first draw the overlay */
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    xpos * gameTileSize() + XOFFSET, ypos * gameTileSize() + YOFFSET,
	    winErase);
	/* now draw the powerloss icon */
	rect.topLeft.x -= gameTileSize();
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    xpos * gameTileSize() + XOFFSET, ypos * gameTileSize() + YOFFSET,
	    winOverlay);
	EndHiresDraw();
}

/*!
 * \brief Draw the water loss icon on a field.
 * 
 * The Field has already been determined to not have water.
 */
void
UIDrawWaterLoss(Int16 xpos, Int16 ypos)
{
	UIDrawLossIcon(xpos, ypos, Z_WATER_OUT_MASK);
}

/*!
 * \brief Draw a power loss icon for the field.
 * 
 * The field has already been determined to not have power
 */
void
UIDrawPowerLoss(Int16 xpos, Int16 ypos)
{
	UIDrawLossIcon(xpos, ypos, Z_POWER_OUT_MASK);
}

/*!
 * \brief Draw a special unit at the location chosen
 * \param i the unit number being painted (index)
 * \param xpos the x position of the unit in tiles
 * \param ypos the y position of the unit in tiles
 */
void
UIDrawSpecialUnit(Int16 xpos, Int16 ypos, Int8 i)
{
	RectangleType rect;
	if (IsDeferDrawing())
		return;

	rect.topLeft.x = game.units[i].type * gameTileSize();
	rect.topLeft.y = gameTileSize();
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	StartHiresDraw();
	_WinCopyRectangle(winUnits, WinGetActiveWindow(), &rect,
	    xpos * gameTileSize() + XOFFSET, ypos * gameTileSize() + YOFFSET,
	    winErase);
	rect.topLeft.y = 0;
	_WinCopyRectangle(winUnits, WinGetActiveWindow(), &rect,
	    xpos * gameTileSize() + XOFFSET, ypos * gameTileSize() + YOFFSET,
	    winOverlay);
	EndHiresDraw();
}

/*
 * Draw a special object a the location chosen.
 * Mostly monsters, but it coud be a train, palin, chopper
 */
void
UIDrawSpecialObject(Int16 xpos, Int16 ypos, Int8 i)
{
	RectangleType rect;
	if (IsDeferDrawing())
		return;

	rect.topLeft.x = (game.objects[i].dir) * gameTileSize();
	rect.topLeft.y = ((i * 2) + 1) * gameTileSize();
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	StartHiresDraw();
	_WinCopyRectangle(winMonsters, WinGetActiveWindow(), &rect,
	    xpos * gameTileSize() + XOFFSET, ypos * gameTileSize() + YOFFSET,
	    winErase);
	rect.topLeft.y -= 16;
	_WinCopyRectangle(winMonsters, WinGetActiveWindow(), &rect,
	    xpos * gameTileSize() + XOFFSET, ypos * gameTileSize() + YOFFSET,
	    winOverlay);
	EndHiresDraw();
}

/*
 * Paint the location on screen with the field.
 */
void
UIDrawField(Int16 xpos, Int16 ypos, welem_t nGraphic)
{
	RectangleType rect;

	if (IsDeferDrawing())
		return;

	rect.topLeft.x = (nGraphic % HORIZONTAL_TILESIZE) * gameTileSize();
	rect.topLeft.y = (nGraphic / HORIZONTAL_TILESIZE) * gameTileSize();
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	/* copy/paste the graphic from the offscreen image */
	StartHiresDraw();
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    xpos * gameTileSize() + XOFFSET, ypos * gameTileSize() + YOFFSET,
	    winPaint);
	EndHiresDraw();
}

/*
 * Scroll the map in the direction specified
 */
void
UIScrollDisplay(dirType direction)
{
	WinHandle screen;
	RectangleType rect;
	int to_x, to_y, i;
	Int16 mapx, mapy;

	if (IsDeferDrawing())
		return;

	rect.topLeft.x = XOFFSET + gameTileSize() * (direction == dtRight);
	rect.topLeft.y = YOFFSET + gameTileSize() * (direction == dtDown);
	rect.extent.x = (vgame.visible_x -
	    1 * (direction == dtRight || direction == dtLeft)) * gameTileSize();
	rect.extent.y = (vgame.visible_y -
	    1 * (direction == dtUp || direction == dtDown)) * gameTileSize();
	to_x = XOFFSET + gameTileSize() * (direction == dtLeft);
	to_y = YOFFSET + gameTileSize() * (direction == dtUp);


	screen = WinGetActiveWindow();
	StartHiresDraw();
	_WinCopyRectangle(screen, screen, &rect, to_x, to_y, winPaint);
	EndHiresDraw();

	/* and lastly, fill the gap */
	LockZone(lz_world);
	UIInitDrawing();

	mapy = getMapYPos();
	mapx = getMapXPos();
	if (direction == dtRight || direction == dtLeft) {
		for (i = mapy; i < vgame.visible_y + mapy; i++) {
			DrawFieldWithoutInit(mapx +
			    (vgame.visible_x - 1) * (direction == dtRight), i);
		}
	} else {
		for (i = mapx; i < vgame.visible_x + mapx; i++) {
			DrawFieldWithoutInit(i, mapy +
			    (vgame.visible_y - 1) * (direction == dtDown));
		}
	}

	UIDrawCursor(vgame.cursor_xpos - mapx, vgame.cursor_ypos - mapy);
	UIDrawLoc();

	UIFinishDrawing();
	UnlockZone(lz_world);
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
	PointType point;
	PointType offset;
	UInt32 extents;
};

static RectangleType shapes[] = {
	{ {0, 0}, {0, 13} },  /* DATELOC */
	{ {0, 0}, {0, 13} }, /* CREDITSLOC */
	{ {0, 0}, {0, 13} }, /* POPLOC */
	{ {0, 0}, {0, 13} } /* POSITIONLOC */
};

/*
 * extents.x
 */
static const struct StatusPositions lrpositions[] = {
	{ {0, 0} , {0, 1}, MIDX },  /* DATELOC */
	{ {0, 0}, {0, 1}, ENDY }, /* CREDITSLOC */
	{ {0, 0}, {0, 1}, MIDX | ENDY }, /* POPLOC */
	{ {0, 0}, {0, 1}, ENDX | ENDY } /* POSITIONLOC */
};
#ifdef HRSUPPORT

/*
 * High resolution version of the positions
 */
static const struct StatusPositions hrpositions[] = {
	{ {1, 0}, {0, 1}, ENDY },  /* DATELOC */
	{ {40, 0}, {0, 1}, ENDY }, /* CREDITSLOC */
	{ {80, 0}, {0, 1}, ENDY }, /* POPLOC */
	{ {140, 0}, {0, 1}, ENDY }, /* POSITIONLOC */
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
	if (isHires()) {
		WriteLog("Plucking HiRes Positions (%d)\n",
		    (int)highDensityFeatureSet());
		sp = (struct StatusPositions *)&(hrpositions[0]);
	} else {
		WriteLog("Plucking Low Resolution Positions\n");
		sp = (struct StatusPositions *)&(lrpositions[0]);
	}
	return (&(sp[pos]));
}

#else
#define	posAt(x)	&(lrpositions[(x)])
#endif
#define	MAXLOC		(sizeof (lrpositions) / sizeof (lrpositions[0]))

/*
 * Draw a status item at the position requested
 */
void
UIDrawItem(Int16 location, char *text)
{
	const struct StatusPositions *pos;
	Int16 sl;
	Coord tx;
	RectangleType *rt;

	if ((location < 0) || ((UInt16)location >= MAXLOC)) {
		Warning("UIDrawitem request for item out of bounds");
		return;
	}
	pos = posAt(location);
	rt = shapes + location;
	if (rt->extent.x && rt->extent.y) {
		StartHiresDraw();
		_WinEraseRectangle(rt, 0);
		EndHiresDraw();
	}

	if (isDoubleOrMoreResolution())
		_FntSetFont(boldFont);
	sl = StrLen(text);
	tx = FntCharsWidth(text, sl);
	switch (pos->extents & (MIDX | ENDX)) {
	case MIDX:
		rt->topLeft.x = (sWidth - tx) / 2 + pos->offset.x;
		break;
	case ENDX:
		rt->topLeft.x = sWidth - (tx + pos->offset.x);
		break;
	default:
		rt->topLeft.x = sWidth * pos->point.x / BASEWIDTH;
	}
	if (pos->extents & ENDY) {
		rt->topLeft.y = sHeight - (rt->extent.y + pos->offset.y);
	}
	rt->extent.x = tx;
	if (highDensityFeatureSet()) {
		StartHiresFontDraw();
		_WinDrawChars(text, sl, normalizeCoord(rt->topLeft.x),
		    normalizeCoord(rt->topLeft.y));
		EndHiresFontDraw();
	} else {
		_WinDrawChars(text, sl, rt->topLeft.x, rt->topLeft.y);
	}
	if (isDoubleOrMoreResolution())
		_FntSetFont(stdFont);
}

/*
 * Check a click in the display.
 * See if it's one of the status locations. If it is then return it.
 */
static Int16
UICheckOnClick(Coord x, Coord y)
{
	UInt16 i;
	for (i = 0; i < MAXLOC; i++) {
		RectangleType *rt = shapes + i;
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
 */
void
UIDrawDate(void)
{
	char temp[20];

	if (IsDeferDrawing())
		return;

	getDate((char *)temp);
	UIDrawItem(DATELOC, temp);
}

/*
 * Draw the amount of credits on screen.
 */
void
UIDrawCredits(void)
{
	char temp[20];
	char scale;
	UInt32 credits;
#ifdef HRSUPPORT
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	if (IsDeferDrawing())
		return;

	credits = scaleNumber(getCredits(), &scale);
	StrPrintF(temp, "$: %ld%c", credits, scale);
	UIDrawItem(CREDITSLOC, temp);
#ifdef HRSUPPORT
	if (isHires()) {
		bitmapHandle = DmGetResource(TBMP, bitmapID_coin);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		StartHiresDraw();
		_WinDrawBitmap(bitmap, shapes[CREDITSLOC].topLeft.x - 11,
		    shapes[CREDITSLOC].topLeft.y);
		EndHiresDraw();
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	}
#endif
}

/*
 * Draw the map position on screen
 */
void
UIDrawLoc(void)
{
	char temp[20];
#ifdef HRSUPPORT
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	if (IsDeferDrawing())
		return;

	StrPrintF(temp, "%02u,%02u", getMapXPos(), getMapYPos());

	UIDrawItem(POSITIONLOC, temp);
#ifdef HRSUPPORT
	if (isHires()) {
		bitmapHandle = DmGetResource(TBMP, bitmapID_loca);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		StartHiresDraw();
		_WinDrawBitmap(bitmap, shapes[POSITIONLOC].topLeft.x - 11,
		    shapes[POSITIONLOC].topLeft.y);
		EndHiresDraw();
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	}
#endif
}

/*
 * Update the build icon on screen
 */
void
UIDrawBuildIcon(void)
{
	MemHandle bitmaphandle;
	BitmapPtr bitmap;

	if (IsDeferDrawing())
		return;

	bitmaphandle = DmGetResource(TBMP,
	    bitmapID_iconBulldoze + (((nSelectedBuildItem <= Be_Extra)) ?
	    nSelectedBuildItem : OFFSET_EXTRA));

	if (bitmaphandle == NULL)
		/* TODO: onscreen error? +save? */
		return;
	bitmap = (BitmapPtr)MemHandleLock(bitmaphandle);
	StartHiresDraw();
	_WinDrawBitmap(bitmap, 2, 2);
	EndHiresDraw();
	MemPtrUnlock(bitmap);
	DmReleaseResource(bitmaphandle);
#if defined(HRSUPPORT)
	if (isHires()) {
		UIDrawToolBar();
	}
#endif
}

/*!
 * \brief Draw the speed icon on screen
 */
void
UIDrawSpeed(void)
{
	RectangleType rect;
#if defined(SONY_CLIE)
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	rect.topLeft.x = speedOffset() * 10;
	rect.topLeft.y = 0;
	rect.extent.x = 10;
	rect.extent.y = 10;
	StartHiresDraw();
	_WinCopyRectangle(winSpeeds, WinGetActiveWindow(), &rect,
	    sWidth - 12, 2, winPaint);
	EndHiresDraw();

#ifdef SONY_CLIE
	if (IsSony()) {
		bitmapHandle = DmGetResource(TBMP, bitmapID_updn + jog_lr);
		/* place at rt - (12 + 8), 1 */
		if (bitmapHandle) {
			bitmap = MemHandleLock(bitmapHandle);
			if (bitmap) {
				StartHiresDraw();
				_WinDrawBitmap(bitmap, sWidth - (12 + 8), 1);
				EndHiresDraw();
				MemPtrUnlock(bitmap);
			}
			DmReleaseResource(bitmapHandle);
		}
	}

#endif
}

/*!
 * \brief Draw the population on screen.
 */
void
UIDrawPop(void)
{
	char temp[20];
	Char scale;
	UInt32 popul;

	if (IsDeferDrawing())
		return;

	popul = getPopulation();
	popul = scaleNumber(popul, &scale);

	StrPrintF(temp, "Pop: %lu%c", popul, scale);
	UIDrawItem(POPLOC, temp);
#ifdef HRSUPPORT
	if (isHires()) {
		MemHandle bitmapHandle;
		BitmapPtr bitmap;
		
		bitmapHandle = DmGetResource(TBMP, bitmapID_popu);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		StartHiresDraw();
		_WinDrawBitmap(bitmap, shapes[POPLOC].topLeft.x - 11,
		    shapes[POPLOC].topLeft.y);
		EndHiresDraw();
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
	if (getCredits() == 0) {
		if (!IsOutShown()) {
			FrmAlert(alertID_outMoney);
			SetOutShown();
		} else {
			return;
		}
	} else if (getCredits() <= 1000) {
		if (!IsLowShown()) {
			FrmAlert(alertID_lowFunds);
			SetLowShown();
		} else {
			return;
		}
	}
}

void
MapHasJumped(void)
{
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

	if (romVersion >= sysMakeROMVersion(4, 0, 0, sysROMStageRelease, 0))
		ClearDirectBmps();
	else
		SetDirectBmps();

	return (0);
}

static struct fromto {
	UInt32 from, to;
} speedslist[] = {
	{SPEED_PAUSED, SPEED_SLOW},
	{SPEED_SLOW, SPEED_MEDIUM},
	{SPEED_MEDIUM, SPEED_FAST},
	{SPEED_FAST, SPEED_TURBO},
	{SPEED_TURBO, SPEED_PAUSED}
};

static Int16
speedOffset(void)
{
	UInt16 i;

	for (i = 0; i < (sizeof (speedslist) / sizeof (speedslist[0])); i++) {
		if (getLoopSeconds() == speedslist[i].from) {
			return (i);
		}
	}
	return (0);
}

static void
cycleSpeed(void)
{
	UInt16 i;

	for (i = 0; i < (sizeof (speedslist) / sizeof (speedslist[0])); i++) {
		if (speedslist[i].from == getLoopSeconds()) {
			setLoopSeconds(speedslist[i].to);
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
		ScrollDisplay(dtUp);
		break;
	case BeDown:
		ScrollDisplay(dtDown);
		break;
	case BeLeft:
		ScrollDisplay(dtLeft);
		break;
	case BeRight:
		ScrollDisplay(dtRight);
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
	/*
	 * if the draw window doesn't occupy most of the screen ...
	 *  - for example if it's a menu
	 * allow jog assist to work.
	 */
	case BeJogUp:
		if (!IsDrawWindowMostOfScreen())
			return (0);
		if (jog_lr)
			ScrollDisplay(dtLeft);
		else
			ScrollDisplay(dtUp);
		break;
	case BeJogDown:
		if (!IsDrawWindowMostOfScreen())
			return (0);
		if (jog_lr)
			ScrollDisplay(dtRight);
		else
			ScrollDisplay(dtDown);
		break;
	case BeJogRelease:
		if (!IsDrawWindowMostOfScreen())
			return (0);
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
	{ 0, BkEnd }
};


/*!
 * \brief find the calculator button on the screen
 * 
 * The calculator button can be a favorites button on the Zire. The problem
 * is that the Zire uses a different code on PalmOS 4 than on PalmOS 5 for
 * the key, making it not work.
 */
static void
buildSilkList()
{
	UInt16 btncount = 0;
	UInt16 atsilk = 0;
	UInt16 atbtn = 0;

	const PenBtnInfoType *silkinfo = EvtGetPenBtnList(&btncount);
	/* favorites / find */
	while (silky[atsilk].vChar != 1) atsilk++;
	/* Bail if we're not a Zire */
	if (!isZireOld()) {
		silky[atsilk].vChar = vchrCalc;
		return;
	}
	while (atbtn < btncount) {
		WriteLog("btn: %ld char: %lx\n", (long)atbtn,
		    (long)silkinfo[atbtn].asciiCode);
		/*
		 * Assumes the favourite/calculator button is one
		 * before the find button. Crap, really.
		 */
		if (silkinfo[atbtn].asciiCode == vchrFind) {
			if (atbtn > 0) { // XXX: fixme
				switch (silkinfo[atbtn-1].asciiCode) {
				case vchrCalc:
				case vchrMenu:
					break;
				default:
					silky[atsilk].vChar =
					    silkinfo[atbtn-1].asciiCode;
					break;
				}
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

#ifdef HRSUPPORT
static BitmapType *pToolbarBitmap = NULL;
static BitmapType *pOldBitmap;

static void
freeToolbarBitmap(void)
{
	if (pOldBitmap != NULL) {
		BmpDelete(pOldBitmap);
		pOldBitmap = NULL;
	}
	if (pToolbarBitmap != NULL) {
		BmpDelete(pToolbarBitmap);
		pToolbarBitmap = NULL;
	}
}

static void
drawToolBitmaps(Coord startx, Coord starty, Coord spacing)
{
	MemHandle hBitmap;
	MemPtr pBitmap;
	UInt32 id;

	for (id = bitmapID_iconBulldoze; id <= bitmapID_iconExtra; id++) {
		hBitmap = DmGetResource(TBMP, id);
		if (hBitmap == NULL) continue;
		pBitmap = MemHandleLock(hBitmap);
		if (pBitmap == NULL) {
			DmReleaseResource(hBitmap);
			continue;
		}
		_WinDrawBitmap(pBitmap, startx, starty);
		startx += spacing;
		MemPtrUnlock(pBitmap);
		DmReleaseResource(hBitmap);
	}
}

int
GetBitmapDimensions(UInt16 resID, Coord *width, Coord *height)
{
	BitmapPtr bmp;
	MemHandle mh = DmGetResource(bitmapRsc, resID);

	if (mh == NULL)
		return (1);

	bmp = MemHandleLock(mh);

	if (bmp == NULL) {
		DmReleaseResource(mh);
		return (1);
	}

	compatBmpGetDimensions(bmp, width, height, NULL);
	MemHandleUnlock(mh);
	DmReleaseResource(mh);
	return (0);
}

static Coord tbWidth;
static Coord bWidth;

static void
UIDrawToolBar(void)
{
	WinHandle wh = NULL;
	WinHandle owh = NULL;

	if (pToolbarBitmap == NULL) {
		UInt16 err;
		Coord bHeight;

		GetBitmapDimensions(bitmapID_iconBulldoze, &bWidth, &bHeight);
		bWidth += 4; 

		tbWidth = (1 + (bitmapID_iconExtra - bitmapID_iconBulldoze)) *
		    bWidth;

		pToolbarBitmap = _BmpCreate(tbWidth, bHeight, getDepth(),
		    NULL, &err);
		if (pToolbarBitmap  != NULL && highDensityFeatureSet()) {
			pOldBitmap = pToolbarBitmap;
			pToolbarBitmap = (BitmapPtr)BmpCreateBitmapV3(
			    pToolbarBitmap, kDensityLow, 
			    BmpGetBits(pToolbarBitmap), NULL);
		}
		if (pToolbarBitmap != NULL) {
			wh = _WinCreateBitmapWindow(pToolbarBitmap, &err);
			if (wh != NULL) {
				owh = WinSetDrawWindow(wh);
				drawToolBitmaps(2, 0, bWidth);
				WinSetDrawWindow(owh);
				WinDeleteWindow(wh, false);
			}
		} else {
			StartHiresDraw();
			drawToolBitmaps(((sWidth - tbWidth) >> 1) + 2,
			    2, bWidth);
			EndHiresDraw();
			return;
		}
	}
	if (pToolbarBitmap != NULL) {
		StartHiresDraw();
		WriteLog("Toolbar at: %ld (width=%ld) [ swidth=%ld ]\n",
		    (long)((sWidth - tbWidth) >> 1), (long)tbWidth,
		    (long)sWidth);
		_WinDrawBitmap(pToolbarBitmap, (sWidth - tbWidth) >> 1, 2);
		EndHiresDraw();
	}
}

static void
toolBarCheck(Coord xpos)
{
	int id;

	/* We've already confirmed the y-axis. */
	if ((xpos < ((sWidth - tbWidth) >> 1)) ||
	    (xpos > ((sWidth + tbWidth) >> 1))) return;

	id = (xpos - ((sWidth - tbWidth) >> 1)) / bWidth;
	WriteLog("Xpos: %ld [ %ld / %ld ] %d %d \n", (long)xpos,
	    (long)tbWidth, (long)sWidth, id, (int)bWidth);
	if (id == (bitmapID_iconExtra - bitmapID_iconBulldoze)) {
		UIPopUpExtraBuildList();
	} else {
		nSelectedBuildItem = id;
	}
	UIDrawBuildIcon();
}

static void
pcResizeDisplay(Boolean draw)
{
	RectangleType disRect;
	FormPtr fp = FrmGetActiveForm();
	Coord nWidth;
	Coord nHeight;
	Int16 loc;

	WinGetDrawWindowBounds(&disRect);

	WriteLog("WinBounds (%d,%d) [%d,%d]\n", (int)disRect.topLeft.x,
	    (int)disRect.topLeft.y, (int)disRect.extent.x,
	    (int)disRect.extent.y);

	nWidth = scaleCoord(disRect.extent.x);
	nHeight = scaleCoord(disRect.extent.y);

	if (nWidth == sWidth && nHeight == sHeight)
		return;

	WriteLog("old wh = (%d, %d)\n", (int)sWidth, (int)sHeight);

	SETWIDTH(nWidth);
	SETHEIGHT(nHeight);
	ResetViewable();
	for (loc = 0; loc < (Int16)MAXLOC; loc++)
		shapes[loc].extent.x = 0;

	WriteLog("new wh = (%d, %d)\n", (int)sWidth, (int)sHeight);
	rPlayGround.extent.x = sWidth;
	rPlayGround.extent.y = sHeight - 2 * 16;
	if (draw) {
		collapsePreRedraw(fp);
		FrmDrawForm(fp);
		DrawGame(1);
		
	}
}

#endif /* HRSUPPORT */

#ifdef	SONY_CLIE

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

	hf = HostFOpen("\\pcity.log", "a");
	if (hf) {
		va_start(args, s);
		StrVPrintF(text, s, args);

		HostFPrintF(hf, text);
		HostFClose(hf);
		va_end(args);
	}
}

void
WriteLogX(char *s, ...)
{
	va_list args;
	HostFILE * hf = NULL;
	Char text[0x100];

	hf = HostFOpen("\\pcity-buildcount.log", "a");
	if (hf) {
		va_start(args, s);
		StrVPrintF(text, s, args);

		HostFPrintF(hf, text);
		HostFClose(hf);
		va_end(args);
	}
}

#endif
