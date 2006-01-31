/*!
 * \file
 * \brief the main simulation code routines
 *
 * This file has all the routines to deal with the startup and general
 * main loop-executionness of the program.
 */

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

#include <ui.h>
#include <drawing.h>
#include <query.h>
#include <handler.h>
#include <globals.h>
#include <simulation.h>
#include <disaster.h>
#include <resCompat.h>
#include <palmutils.h>
#include <simcity_resconsts.h>
#include <mem_compat.h>
#include <minimap.h>
#include <beam.h>
#include <logging.h>
#include <locking.h>

#if defined(PALM_FIVE)
/* 5-way navigator support needs Palm (Specific) SDK */
#include <PalmNavigator.h>
#endif

#if defined(LOGGING)
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
Coord XOFFSET = 0;
Coord YOFFSET = 15;
#if defined(SONY_CLIE)
UInt16 jog_lr = 0;
#endif

static Boolean hPocketCity(EventPtr event);
static Boolean hQuickList(EventPtr event);
static Boolean hExtraList(EventPtr event);

static Int16 _PalmInit(void) LARD_SECTION;
static void _PalmFini(void) LARD_SECTION;
static void doPocketCityOpen(FormPtr form) LARD_SECTION;

static void buildSilkList(void);
static Int16 vkDoEvent(UInt16 key);
static void UIDoQuickList(void);
static void UIPopUpExtraBuildList(void);
static void CleanUpExtraBuildForm(void);
static FieldType * UpdateDescription(Int16 sel);

void _UIGetFieldToBuildOn(Int16 x, Int16 y);
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);
static void EventLoop(void);
static void cycleSpeed(void);
static void DoAbout(void) LARD_SECTION;
static Boolean CheckTextClick(Coord x, Coord y);
static Int16 doKeyEvent(keyEvent key);
static Int16 speedOffset(void);
static void UISetSelectedBuildItem(BuildCode item);
static void ClearScrolling(void);
static void SetScrolling(void);
static UInt16 IsScrolling(void);

#if defined(SONY_CLIE)
static void HoldHook(UInt32);
#endif

#if defined(HRSUPPORT)
static void toolBarCheck(Coord);
static void UIPaintToolBar(void);
/*! \brief free Tool Bar bitmap */
static void freeToolbarBitmap(void);
static void pcResizeDisplay(FormPtr form, Int16 hOff, Int16 vOff,
  Boolean draw);
#else
/*! \brief null defined freeToolBar - we don't suport toolbars in lowres */
#define	freeToolbarBitmap()
/*! \brief null defined resize - we don't suport resize in lowres */
#define	pcResizeDisplay(X, Y, A, B)
#endif

/* Collects what would otherwise be several variables */
static UInt16 IsBuilding(void);
static void SetBuilding(void);
static void SetNotBuilding(void);
static void SetDeferDrawing(void);
static void SetDrawing(void);
static UInt16 IsDeferDrawing(void);

static void performPaintDisplay(void);

/*!
 * \brief get a resource string
 *
 * gets a resource string into the buffer passed. Assumes that the buffer
 * can hold the string in question.
 * \param index the index into the resstrings_base string list that we are
 * obtaining
 * \param buffer the buffer to fill with the string
 * \param length the length of the buffer to fill
 * \todo error check the result - we need a string
 */
void
ResGetString(UInt16 index, char *buffer, UInt16 length)
{
	if (NULL == SysStringByIndex(resstrings_base, index, buffer, length)) {
		/* XXX: Error here ! */
	}
}

/*
 * The PilotMain routine.
 * Needed by all palm applications.
 * It checks the rom version and throws out to the user of it's too old.
 * Handles the auto-saving of the application at the end.
 */
UInt32
PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	UInt16 error;
	Int16 pir;

	switch (cmd) {
	case sysAppLaunchCmdNormalLaunch:
		break;
	case sysAppLaunchCmdGoTo:
		break;
	case sysAppLaunchCmdSystemReset:
		if (((SysAppLaunchCmdSystemResetType *)cmdPBP)->createDefaultDB)
			BeamRegister();
		return (0);
	case sysAppLaunchCmdExgAskUser:
		if (launchFlags & sysAppLaunchFlagSubCall) {
			((ExgAskParamPtr)cmdPBP)->result = exgAskOk;
		}
		return (0);
		break;
	case sysAppLaunchCmdExgReceiveData:
		if (errNone == BeamReceive((ExgSocketPtr)cmdPBP)) {
			if (launchFlags & sysAppLaunchFlagSubCall) {
				if (getGameInProgress())
					SaveGameByName(game.cityname);
				UILoadAutoGame();
				FrmGotoForm(formID_pocketCity);
			} else {
				((ExgSocketPtr)cmdPBP)->goToCreator =
				    GetCreatorID();
			}
		}
		return (0);
		break;
	default:
		return (0);
		break;
	}

	error = RomVersionCompatible(
	    sysMakeROMVersion(3, 5, 0, sysROMStageRelease, 0), launchFlags);

	BeamRegister();

	if (error)
		return (error);

	WriteLog("Starting Pocket City\n");
	if (0 != (pir = PCityStartup())) {
		WriteLog("Init Didn't Happen right [%d]\n", (int)pir);
		PCityShutdown();
		return (1);
	}

	if (-1 != UILoadAutoGame()) {
		FrmGotoForm(formID_pocketCity);
	} else {
		FrmGotoForm(formID_files);
	}

	EventLoop();

	if (getGameInProgress()) {
		UISaveAutoGame();
	}

	PCityShutdown();

	return (0);
}

static
FormEventHandlerType *id2handler[] = {
	hPocketCity, /* formID_pocketCity */
	hBudget, /* formID_budget */
	hMap, /* formID_map */
	hFiles, /* formID_files */
	hFilesNew, /* formID_filesNew */
	hQuickList, /* formID_quickList */
	hExtraList, /* formID_extraBuild */
	hOptions, /* formID_options */
	hButtonConfig, /* formID_ButtonConfig */
	hQuery /* formID_Query */
};
#define	IDARLEN	(sizeof (id2handler) / sizeof (id2handler[0]))

static FormEventHandlerType *
gitForm(UInt16 formID)
{
	if (formID < 1000)
		return (NULL);
	formID -= 1000;
	if (formID > (IDARLEN - 1))
		return (NULL);
	return (id2handler[formID]);
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
	FormEventHandlerType *fh;

	if (event->eType == frmLoadEvent) {
		formID = event->data.frmLoad.formID;
		WriteLog("Main::frmLoadEvent: %d -> %d\n", (int)oldFormID,
		    (int)formID);
		oldFormID = formID;
		form = FrmInitForm(formID);
		FrmSetActiveForm(form);
		fh = gitForm(formID);
		if (fh != NULL)
			FrmSetEventHandler(form, fh);
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

#if defined(LOGGING)
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
		EvtGetEvent(&event, (Int32)10);
		if (event.eType == appStopEvent)
			break;

		if (event.eType == keyDownEvent)
			if (FrmDispatchEvent(&event))
				continue;

		if (SysHandleEvent(&event))
			continue;

		if (MenuHandleEvent((MenuBarType *)0, &event, &err))
			continue;

		if (AppEvent(&event))
			continue;

		if (FrmDispatchEvent(&event))
			continue;

		/*
		 * if we're faffing around in the savegame dialogs then
		 * we're not actually playing the game.
		 */
		if (!getGameInProgress())
			continue;

		/* Game is fully formally paused ? */
		if (!getGamePlaying())
			continue;

		if (IsBuilding())
			continue;

		if (IsScrolling()) {
			if ((event.eType != keyDownEvent)) {
				if (KeyCurrentState() == 0) {
					ClearScrolling();
				} else {
					continue;
				}
			}
		}

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
#if defined(LOGGING)
			Int16 q, pc = 0;
#endif
			MoveAllObjects();
#if defined(LOGGING)
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

/*!
 * \brief Handles and resourceID's of the bitmaps
 */
static const struct _bmphandles {
	WinHandle *handle; /*!< Window Handle of bitmap */
	const DmResID resourceID; /*!< Resource ID of bitmap obtained */
} handles[] = {
	{ &winZones, (DmResID)bitmapID_zones },
	{ &winMonsters, (DmResID)bitmapID_monsters },
	{ &winUnits, (DmResID)bitmapID_units },
/* XXX:	{ &winButtons, (DmResID)bitmapID_buttons }, */
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

	/* section (2) */

	_refTiles = DmOpenDatabaseByTypeCreator(TILEDBTYPE, GetCreatorID(),
	    dmModeReadOnly);

	if (_refTiles == 0) {
		FrmAlert(alertID_tilesMissing);
		return (2);
	}

	/* section (3) */
	StartSilk();

	/* set screen mode to colors if supported */
	if (Is35ROM()) {  /* must be v3.5+ for some functions in here */
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
	rPlayGround.topLeft.x = XOFFSET;
	rPlayGround.topLeft.y = YOFFSET; /* Padding for the menubar */
	rPlayGround.extent.x = normalizeCoord(GETWIDTH());
	/* Space on the bottom */
	rPlayGround.extent.y = normalizeCoord(GETHEIGHT() - 3*16);

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
#if defined(HRSUPPORT)
		if (IsScaleModes()) StartHiresDraw();
#endif
		_WinDrawBitmap(bitmap, 0, 0);
#if defined(HRSUPPORT)
		if (IsScaleModes()) EndHiresDraw();
#endif
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
_PalmFini(void)
{
	UInt16 i;

	unhookHoldSwitch();
	freeToolbarBitmap();

	/* clean up handles */
	for (i = 0; i < (sizeof (handles) / sizeof (handles[0])); i++) {
		if (*(handles[i].handle) != NULL) {
			WinDeleteWindow(*(handles[i].handle), 0);
			*(handles[i].handle) = NULL;
		}
	}
	restoreDepthRes();
	if (_refTiles != 0) DmCloseDatabase(_refTiles);
	_refTiles = 0;
	PrefSetAppPreferences(GetCreatorID(), 0, CONFIG_VERSION,
	    &gameConfig, sizeof (AppConfig_t), true);
	/* Close the forms */
	FrmCloseAllForms();
	CloseMyDB();

	EndSilk();
}

static void
DoLoadAlert(void)
{
	switch (FrmAlert(alertID_loadGame)) {
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
}

static void
SaveCheck(void)
{
	if (FrmAlert(alertID_saveGame) == 0) {
		UISaveMyCity();
	}
}

static void
SaveBeam(void)
{
	UISaveMyCity();
	BeamCityByName(game.cityname);
}

static Boolean
DoPCityMenuProcessing(UInt16 itemID)
{
	Boolean handled = true;

	switch (itemID) {
		/* First menu ... game */
	case menuitemID_loadGame:
		DoLoadAlert();
		break;
	case menuitemID_saveGame:
		SaveCheck();
		break;
	case menuitemID_Budget:
		FrmGotoForm(formID_budget);
		break;
	case menuitemID_Map:
		FrmGotoForm(formID_map);
		break;
	case menuitemID_Configuration:
		FrmGotoForm(formID_options);
		break;

	case menuitemID_Buttons:
		FrmGotoForm(formID_ButtonConfig);
		break;

	case menuitemID_ForceResupply:
		AddGridUpdate(GRID_ALL);
		break;

		/* next menu ... build */

	case mi_removeDefence:
		RemoveAllDefence();
		break;

		/* for a reason ... */
	case mi_buildExtra:
		UIPopUpExtraBuildList();
		break;

	case mi_CauseFire:
	case mi_CauseMeltDown:
	case mi_CauseMonster:
	case mi_CauseDragon:
	case mi_CauseMeteor:
		DoSpecificDisaster((disaster_t)(itemID - mi_CauseFire +
			    diFireOutbreak));
		break;

		/* next menu ... speed */

	case menuID_SlowSpeed:
		setLoopSeconds(SPEED_SLOW);
		addGraphicUpdate(gu_speed);
		break;
	case menuID_MediumSpeed:
		setLoopSeconds(SPEED_MEDIUM);
		addGraphicUpdate(gu_speed);
		break;
	case menuID_FastSpeed:
		setLoopSeconds(SPEED_FAST);
		addGraphicUpdate(gu_speed);
		break;
	case menuID_TurboSpeed:
		setLoopSeconds(SPEED_TURBO);
		addGraphicUpdate(gu_speed);
		break;
	case menuID_PauseSpeed:
		setLoopSeconds(SPEED_PAUSED);
		addGraphicUpdate(gu_speed);
		break;

		/* next menu ... help */

	case menuitemID_about:
		DoAbout();
		break;
	case menuitemID_tips:
		FrmHelp(StrID_tips);
		break;
	case menuitemID_Beam:
		SaveBeam();
		break;
#if defined(CHEAT) || defined(DEBUG)
	case menuitemID_Funny:
		/*
		 * change this to whatever testing you're doing.
		 * just handy with a 'trigger' button for testing
		 * ie. disaters... this item is erased if you've
		 * not compiled with CHEAT or DEBUG
		 */
#if defined(CHEAT)
		game.credits += 10000;
#endif
#if defined(DEBUG)
		MeteorDisaster(20, 20);
#endif
		break;
#endif
	default:
		handled = false;
		break;
	}
	return (handled);
}

static void
doPocketCityOpen(FormPtr form)
{
#if defined(HRSUPPORT)
	Int16 hOff = 0, vOff = 0;
#endif
	RectangleType rect;

	FrmDrawForm(form);
	SetSilkResizable(form, true);

	rect.topLeft.x = normalizeCoord(GETWIDTH()) - 2 * gameTileSize();
	rect.topLeft.y = normalizeCoord(GETHEIGHT()) - 2 * gameTileSize();
	rect.extent.x = 2 * gameTileSize();
	rect.extent.y = rect.extent.x;

	minimapPlace(&rect);
	minimapSetShowing(GETMINIMAPVISIBLE());

#if defined(HRSUPPORT)
	if (collapseMove(form, CM_DEFAULT, &hOff, &vOff)) {
		pcResizeDisplay(form, hOff, vOff, false);
	}
#endif
	setGameInProgress(1);
	ResumeGame();
	FrmDrawForm(form);
	SetDrawing();
	DrawGame(1);
}

/*
 * Handler for the main pocketCity form.
 * This form performs all the updates to the main game screen.
 */
static Boolean
hPocketCity(EventPtr evp)
{
	FormPtr form;
	Boolean handled = false;
	PointType location;
	PointType newlocation;
#if defined(HRSUPPORT)
	Boolean redraw;
	Int16 hOff = 0, vOff = 0;
#endif

	switch (evp->eType) {
	case frmOpenEvent:
		form = FrmGetActiveForm();
		doPocketCityOpen(form);
		handled = true;
		break;
	case frmCloseEvent:
		SetSilkResizable(NULL, false);
		break;
	case penDownEvent:
		location.x = evp->screenX;
		location.y = evp->screenY;
		if (GETMINIMAPVISIBLE()) {
			if (minimapIsTapped(&location, &newlocation)) {
				Goto((UInt16)newlocation.x,
				    (UInt16)newlocation.y, 1);
				handled = true;
				break;
			}
		}
		if (RctPtInRectangle(location.x, location.y, &rPlayGround)) {
			scalePoint(&location);
			/* click was on the playground */
			_UIGetFieldToBuildOn(location.x, location.y);
			handled = true;
			break;
		}
		scalePoint(&location);
		if (location.y < 12) {
			handled = true;
			if (location.x >= (GETWIDTH() - 12)) {
				/* click was on change speed */
				cycleSpeed();
				addGraphicUpdate(gu_speed);
				break;
			}
			if (location.x < 12) {
				/* click was on toggle production */
				if (nSelectedBuildItem == Be_Bulldozer) {
					UISetSelectedBuildItem(
					    nPreviousBuildItem);
				} else {
					nPreviousBuildItem = nSelectedBuildItem;
					UISetSelectedBuildItem(Be_Bulldozer);
				}
				addGraphicUpdate(gu_buildicon);
				break;
			}
#if defined(HRSUPPORT)
			if (isHires())
				toolBarCheck(location.x);
#endif
			/* check for other 'penclicks' here */
		}

		handled = CheckTextClick(location.x, location.y);
		break;
	case penMoveEvent:
		location.x = evp->screenX;
		location.y = evp->screenY;
		if (RctPtInRectangle(location.x, location.y,
		    &rPlayGround)) {
			scalePoint(&location);
			_UIGetFieldToBuildOn(location.x, location.y);
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
		WriteLog("Menu Item: %d\n", (int)evp->data.menu.itemID);
		handled = DoPCityMenuProcessing(evp->data.menu.itemID);

	case keyDownEvent:
#if defined(PALM_FIVE)
		if (EvtKeydownIsVirtual(evp) && IsFiveWayNavEvent(evp)) {
			if (NavDirectionPressed(evp, Left))
				doKeyEvent(keLeft);
			else if (NavDirectionPressed(evp, Right))
				doKeyEvent(keRight);
			else if (NavDirectionPressed(evp, Up))
				doKeyEvent(keUp);
			else if (NavDirectionPressed(evp, Down))
				doKeyEvent(keDown);
			handled = true;
		} else // Note the fallthrough
#endif
		handled = (Boolean)vkDoEvent(evp->data.keyDown.chr);
		break;
	case keyUpEvent:
		handled = true;
		break;
#if defined(HRSUPPORT)
	case winDisplayChangedEvent:
#if defined(SONY_CLIE)
	case vchrSilkResize:
#endif
		redraw = collapseMove(FrmGetActiveForm(), CM_DEFAULT,
		    &hOff, &vOff);
		if (redraw)
			pcResizeDisplay(FrmGetActiveForm(), hOff, vOff, redraw);
		handled = true;
		break;
#endif
	default:
		break;
	}
	performPaintDisplay();

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

void
UIPostLoadGame(void)
{
	clearProblemFlags();
}

/*
 * Go to one of the Budget and map forms.
 * Only if we're not already at that form to begin with.
 */
void
GotoForm(Int16 n)
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
	    lp, (Int16)poplen);

	UpdateDescription(0);
	sfe = FrmDoDialog(form);

	FreeStringList(lp);

	switch (sfe) {
	case buttonID_extraBuildSelect:
		/* List entries must match entries in BuildCode 0 .. */
		UISetSelectedBuildItem((BuildCode)LstGetSelection(
		    (ListPtr)GetObjectPtr(form, listID_extraBuildList)));
		break;
	case buttonID_extraBuildFireMen:
		UISetSelectedBuildItem(Be_Defence_Fire);
		break;
	case buttonID_extraBuildPolice:
		UISetSelectedBuildItem(Be_Defence_Police);
		break;
	case buttonID_extraBuildMilitary:
		UISetSelectedBuildItem(Be_Defence_Military);
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

	SysStringByIndex(strID_Descriptions, (UInt16)sel, temp, (UInt16)256);
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

	if (Is35ROM()) {
		ftList = FrmInitForm(formID_quickList);
		FrmSetEventHandler(ftList, hQuickList);
		UISetSelectedBuildItem((BuildCode)(FrmDoDialog(ftList) -
		    gi_buildBulldoze));

		if (nSelectedBuildItem >= OFFSET_EXTRA)
			UIPopUpExtraBuildList();
		addGraphicUpdate(gu_buildicon);
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
		// XXX: Deal with any of the pop-up keys */
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
 * initialize graphics
 */
int
UIInitializeGraphics(void)
{
	setGameTileSize(16);
	setMapTileSize(4);
	return (_PalmInit());
}

/*
 * cleanup the graphics
 */
void
UICleanupGraphics(void)
{
	_PalmFini();
}

/*
 * Save the selected build item.
 */
static void
UISetSelectedBuildItem(BuildCode item)
{
	nSelectedBuildItem = item;
}

/* is Building logic and data */
static UInt16 __state;

/*
 * First the Macro.
 * You Define the Bit, Clearer, Setter and Tester
 * of the Bit field you care about.
 * If you need more than 16 bits (0..15) then change the return type here and
 * in any of the entries shared in the header file.
 */
#define	BUILD_STATEBITACCESSOR(BIT, CLEARER, SETTER, TESTER, VISIBILITY) \
VISIBILITY void \
CLEARER(void) \
{ \
	__state &= ~((UInt16)1<<(BIT)); \
} \
VISIBILITY void \
SETTER(void) \
{ \
	__state |= ((UInt16)1<<(BIT)); \
} \
VISIBILITY UInt16 \
TESTER(void) \
{ \
	return (__state & ((UInt16)1<<(BIT))); \
}

#define	GLOBAL

BUILD_STATEBITACCESSOR(1, SetNotBuilding, SetBuilding, IsBuilding, static)
BUILD_STATEBITACCESSOR(2, SetLowMoneyNotShown, SetLowMoneyShown, \
    IsLowMoneyShown, static)
BUILD_STATEBITACCESSOR(3, SetOutMoneyNotShown, SetOutMoneyShown, \
    IsOutMoneyShown, static)
BUILD_STATEBITACCESSOR(4, SetLowPowerNotShown, SetLowPowerShown, \
    IsLowPowerShown, static)
BUILD_STATEBITACCESSOR(5, SetOutPowerNotShown, SetOutPowerShown, \
    IsOutPowerShown, static)
BUILD_STATEBITACCESSOR(6, SetLowWaterNotShown, SetLowWaterShown, \
    IsLowWaterShown, static)
BUILD_STATEBITACCESSOR(7, SetOutWaterNotShown, SetOutWaterShown, \
    IsOutWaterShown, static)
BUILD_STATEBITACCESSOR(8, Clear35ROM, Set35ROM, Is35ROM, GLOBAL)
BUILD_STATEBITACCESSOR(9, SetDrawing, SetDeferDrawing, IsDeferDrawing, static)
BUILD_STATEBITACCESSOR(10, Clear40ROM, Set40ROM, Is40ROM, GLOBAL)
BUILD_STATEBITACCESSOR(11, ClearScaleModes, SetScaleModes, IsScaleModes, GLOBAL)
BUILD_STATEBITACCESSOR(12, ClearScrolling, SetScrolling, IsScrolling, static)

/*!
 * \brief clear the low and out of power flags
 */
static void
ClearLowOutPowerFlags(void)
{
	SetLowPowerNotShown();
	SetOutPowerNotShown();
}

/*!
 * \brief clear the low and out water flags
 */
static void
ClearLowOutWaterFlags(void)
{
	SetLowWaterNotShown();
	SetOutWaterNotShown();
}

/*!
 * \brief clear the low and out of money flags
 */
static void
ClearLowOutMoneyFlags(void)
{
	SetLowMoneyNotShown();
	SetOutMoneyNotShown();
}

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
static void
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
	rect.extent.x = scaleCoord((Coord)(getVisibleX() * gameTileSize()));
	rect.extent.y = scaleCoord((Coord)(getVisibleY() * gameTileSize()));
	rect.topLeft.x = XOFFSET;
	rect.topLeft.y = YOFFSET;

	if (RctPtInRectangle(x, y, &rect)) {
		Coord xpos = (x - XOFFSET) / gameTileSize() + getMapXPos();
		Coord ypos = (y - YOFFSET) / gameTileSize() + getMapYPos();
		zone_lock(lz_world); // OK
		SetPositionClicked(WORLDPOS(xpos, ypos));
		zone_unlock(lz_world); // OK
		if (UIGetSelectedBuildItem() != Be_Query)
			BuildSomething((UInt16)xpos, (UInt16)ypos);
		else
			FrmGotoForm(formID_Query);
	}
}

void
UIDisasterNotify(disaster_t disaster)
{
	char string[512];
	SysStringByIndex(st_disasters,
	    (UInt16)(disaster - diFireOutbreak), string, (UInt16)511);
	if (*string == '\0') StrPrintF(string, "generic disaster??");

	FrmCustomAlert(alertID_generic_disaster, string, 0, 0);
}

/*! \brief table containing the problem_table input/output function */
static struct problemtable {
	problem_t entry; /*!< the problem entry */
	UInt16 (*test)(void); /*!< the testing function */
	void (*set)(void); /*!< the setting function */
	UInt16 alert; /*!< the alert to display */
} problem_table[] = {
	{ peFineOnMoney, NULL, ClearLowOutMoneyFlags, 0 },
	{ peLowOnMoney, IsLowMoneyShown, SetLowMoneyShown, alertID_lowFunds },
	{ peOutOfMoney, IsOutMoneyShown, SetOutMoneyShown, alertID_outMoney },
	{ peFineOnPower, NULL, ClearLowOutPowerFlags, 0 },
	{ peLowOnPower, IsLowPowerShown, SetLowPowerShown, alertID_lowPower },
	{ peOutOfPower, IsOutPowerShown, SetOutPowerShown, alertID_outPower },
	{ peFineOnWater, NULL, ClearLowOutWaterFlags, 0 },
	{ peLowOnWater, IsLowWaterShown, SetLowWaterShown, alertID_lowWater },
	{ peOutOfWater, IsOutWaterShown, SetOutWaterShown, alertID_outWater }
};

#define	PROBLEMTABLE_SIZE (sizeof (problem_table) / sizeof (problem_table[0]))

void
UIProblemNotify(problem_t problem)
{
	UInt16 alert = 0;
	UInt16 i;

	for (i = 0; i < PROBLEMTABLE_SIZE; i++) {
		if (problem_table[i].entry == problem) {
			if (problem_table[i].test != NULL &&
			    problem_table[i].test()) return;
			problem_table[i].set();
			alert = problem_table[i].alert;
			break;
		}
	}
	if (alert != 0)
		FrmAlert(alert);
}

/*
 * Display an specific system error to the user
 */
void
UISystemErrorNotify(syserror_t error)
{
	switch (error) {
	case seOutOfMemory:
		FrmAlert(alertID_errorOutOfMemory);
		break;
	case seInvalidSaveGame:
		FrmAlert(alertID_invalidSaveVersion);
		break;
	case seUnknownBuildItem:
		FrmAlert(alertID_unknownBuildItem);
		break;
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
 * \brief Try to lock the screen from updates.
 *
 * This allows bulk updates to the screen without repainting until unlocked.
 */
void
UILockScreen(void)
{
	if (!Is35ROM())
		return;
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
	if (!Is35ROM())
		return;
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
UIPaintCursor(UInt16 xpos __attribute__((unused)),
    UInt16 ypos __attribute__((unused)))
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
 * \param elem the element to use for the loss icon
 */
static void
PaintLossIcon(UInt16 xpos, UInt16 ypos, welem_t elem)
{
	RectangleType rect;

	if (IsDeferDrawing() || UIClipped(xpos, ypos))
		return;

	xpos -= getMapXPos();
	ypos -= getMapYPos();

	rect.topLeft.x = (Coord)(elem % HORIZONTAL_TILESIZE) * gameTileSize();
	rect.topLeft.y = (Coord)(elem / HORIZONTAL_TILESIZE) * gameTileSize();
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	/* copy/paste the graphic from the offscreen image */
	StartHiresDraw();
	/* first draw the overlay */
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    (Coord)(xpos * gameTileSize() + XOFFSET),
	    (Coord)(ypos * gameTileSize() + YOFFSET),
	    winErase);
	/* now draw the powerloss icon */
	rect.topLeft.x -= gameTileSize();
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    (Coord)(xpos * gameTileSize() + XOFFSET),
	    (Coord)(ypos * gameTileSize() + YOFFSET),
	    winOverlay);
	EndHiresDraw();
}

/*!
 * \brief Draw the water loss icon on a field.
 *
 * The Field has already been determined to not have water.
 */
void
UIPaintWaterLoss(UInt16 xpos, UInt16 ypos)
{
	PaintLossIcon(xpos, ypos, Z_WATER_OUT_MASK);
}

/*!
 * \brief Draw a power loss icon for the field.
 *
 * The field has already been determined to not have power
 */
void
UIPaintPowerLoss(UInt16 xpos, UInt16 ypos)
{
	PaintLossIcon(xpos, ypos, Z_POWER_OUT_MASK);
}

/*!
 * \brief Draw a special unit at the location chosen
 * \param i the unit number being painted (index)
 * \param xpos the x position of the unit in tiles
 * \param ypos the y position of the unit in tiles
 */
void
UIPaintSpecialUnit(UInt16 xpos, UInt16 ypos, Int8 i)
{
	RectangleType rect;

	if (IsDeferDrawing() || UIClipped(xpos, ypos))
		return;

	xpos -= getMapXPos();
	ypos -= getMapYPos();

	rect.topLeft.x = game.units[i].type * gameTileSize();
	rect.topLeft.y = gameTileSize();
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	StartHiresDraw();
	_WinCopyRectangle(winUnits, WinGetActiveWindow(), &rect,
	    (Coord)(xpos * gameTileSize() + XOFFSET),
	    (Coord)(ypos * gameTileSize() + YOFFSET),
	    winErase);
	rect.topLeft.y = 0;
	_WinCopyRectangle(winUnits, WinGetActiveWindow(), &rect,
	    (Coord)(xpos * gameTileSize() + XOFFSET),
	    (Coord)(ypos * gameTileSize() + YOFFSET),
	    winOverlay);
	EndHiresDraw();
}

/*
 * Draw a special object a the location chosen.
 * Mostly monsters, but it coud be a train, palin, chopper
 */
void
UIPaintSpecialObject(UInt16 xpos, UInt16 ypos, Int8 i)
{
	RectangleType rect;

	if (IsDeferDrawing() || UIClipped(xpos, ypos))
		return;

	xpos -= getMapXPos();
	ypos -= getMapYPos();

	rect.topLeft.x = (Coord)(game.objects[i].dir * gameTileSize());
	rect.topLeft.y = (Coord)(((i * 2) + 1) * gameTileSize());
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	StartHiresDraw();
	_WinCopyRectangle(winMonsters, WinGetActiveWindow(), &rect,
	    (Coord)(xpos * gameTileSize() + XOFFSET),
	    (Coord)(ypos * gameTileSize() + YOFFSET),
	    winErase);
	rect.topLeft.y -= 16;
	_WinCopyRectangle(winMonsters, WinGetActiveWindow(), &rect,
	    (Coord)(xpos * gameTileSize() + XOFFSET),
	    (Coord)(ypos * gameTileSize() + YOFFSET),
	    winOverlay);
	EndHiresDraw();
}

/*
 * Paint the location on screen with the field.
 */
void
UIPaintField(UInt16 xpos, UInt16 ypos, welem_t nGraphic)
{
	RectangleType rect;

	if (IsDeferDrawing() || UIClipped(xpos, ypos))
		return;

	xpos -= getMapXPos();
	ypos -= getMapYPos();

	rect.topLeft.x = (nGraphic % HORIZONTAL_TILESIZE) * gameTileSize();
	rect.topLeft.y = (nGraphic / HORIZONTAL_TILESIZE) * gameTileSize();
	rect.extent.x = gameTileSize();
	rect.extent.y = gameTileSize();

	/* copy/paste the graphic from the offscreen image */
	StartHiresDraw();
	_WinCopyRectangle(winZones, WinGetActiveWindow(), &rect,
	    (Coord)(xpos * gameTileSize() + XOFFSET),
	    (Coord)(ypos * gameTileSize() + YOFFSET),
	    winPaint);
	EndHiresDraw();
}

/*!
 * \brief actually perform all the painting
 * This will perform all the painting that has been asked for on the display.
 * It allows us to pool all the painting in one point; which allows for more
 * consistent painting of the display; rather than mandating the painting
 * at a point that would make it inefficient.
 */
static void
performPaintDisplay(void)
{
	UIInitDrawing();
	if (checkGraphicUpdate(gu_playarea))
		UIPaintPlayArea();
	if (checkGraphicUpdate(gu_credits))
		UIPaintCredits();
	if (checkGraphicUpdate(gu_population))
		UIPaintPopulation();
	if (checkGraphicUpdate(gu_date))
		UIPaintDate();
	if (checkGraphicUpdate(gu_location))
		if (!GETMINIMAPVISIBLE())
			UIPaintLocation();
	if (checkGraphicUpdate(gu_buildicon))
		UIPaintBuildIcon();
	if (checkGraphicUpdate(gu_speed))
		UIPaintSpeed();
	if (checkGraphicUpdate(gu_desires))
		UIPaintDesires();
	clearGraphicUpdate();
	UIFinishDrawing();
}

/*!
 * \brief Draw the play area
 */
void
UIPaintPlayArea(void)
{
	UInt16 x = getMapXPos();
	UInt16 y = getMapYPos();

	UInt16 maxx = (x + getVisibleX()) < getMapWidth() ?
	    x + getVisibleX() : getMapWidth();
	UInt16 maxy = (y + getVisibleY()) < getMapHeight() ?
	    y + getVisibleY() : getMapHeight();

	WriteLog("Visible: (%d,%d)\nFullSize: (%d,%d)\n", getVisibleX(),
	  getVisibleY(), getMapWidth(), getMapHeight());
	WriteLog("Drawing World From (%d,%d)->(%d,%d)\n", x, y, maxx, maxy);
	zone_lock(lz_world);
	zone_lock(lz_flags);
	for (; x < maxx; x++)
		for (y = getMapYPos(); y < maxy; y++)
			DrawFieldWithoutInit(x, y);

	if (GETMINIMAPVISIBLE())
		minimapPaint();
	zone_unlock(lz_flags);
	zone_unlock(lz_world);
}

/*!
 * \brief Paint the desires onto the screen
 */
void
UIPaintDesires(void)
{
	/* return; */
}

/*
 * Scroll the map in the direction specified
 */
void
UIScrollDisplay(dirType direction)
{
	WinHandle screen;
	RectangleType rect;
	RectangleType overlap;

	Int16 to_x, to_y;
	UInt16 mapx, mapy;
	UInt16 x, y;
	UInt16 s_x, s_y, l_x, l_y, gs_m1;

	if (IsDeferDrawing())
		return;

	mapx = getMapXPos();
	mapy = getMapYPos();

	SetScrolling();

	zone_lock(lz_world);
	zone_lock(lz_flags);
	UIInitDrawing();

	if (GETMINIMAPVISIBLE()) {
		/* Repaint fhe area occluded by the minimap */
		minimapIntersect(&rPlayGround, &overlap);
		gs_m1 = gameTileSize() - 1;
		s_x = mapx + scaleCoord(
		    inGameTiles(overlap.topLeft.x - XOFFSET));
		s_y = mapy + scaleCoord(
		    inGameTiles(overlap.topLeft.y - YOFFSET));
		l_x = s_x + scaleCoord(inGameTiles(overlap.extent.x + gs_m1));
		l_y = s_y + scaleCoord(inGameTiles(overlap.extent.y + gs_m1));
		WriteLog("Minimap(%d,%d)->(%d,%d)\n", s_x, s_y, l_x, l_y);

		for (x = s_x; x <= l_x; x++) {
			for (y = s_y; y <= l_y; y++) {
				DrawFieldWithoutInit(x, y);
			}
		}
	}

	rect.topLeft.x = XOFFSET + gameTileSize() * (direction == dtRight);
	rect.topLeft.y = YOFFSET + gameTileSize() * (direction == dtDown);
	rect.extent.x = (Coord)((getVisibleX() -
	    ((direction == dtRight || direction == dtLeft) ? 1 : 0)) *
	    gameTileSize());
	rect.extent.y = (Coord)((getVisibleY() -
	    ((direction == dtUp || direction == dtDown) ? 1 : 0)) *
	    gameTileSize());
	to_x = XOFFSET + gameTileSize() * (direction == dtLeft);
	to_y = YOFFSET + gameTileSize() * (direction == dtUp);

	screen = WinGetActiveWindow();
	StartHiresDraw();
	_WinCopyRectangle(screen, screen, &rect, to_x, to_y, winPaint);
	EndHiresDraw();

	/* and lastly, fill the gap */

	if (direction == dtRight || direction == dtLeft) {
		for (y = mapy; y < getVisibleY() + mapy; y++) {
			DrawFieldWithoutInit((UInt16)(mapx +
			    (getVisibleX() - 1) * (direction == dtRight)), y);
		}
	} else {
		for (x = mapx; x < getVisibleX() + mapx; x++) {
			DrawFieldWithoutInit(x, (UInt16)(mapy +
			    (getVisibleY() - 1) * (direction == dtDown)));
		}
	}

	UIPaintCursor(getCursorX(), getCursorY());
	if (GETMINIMAPVISIBLE()) {
		minimapPaint();
	} else {
		UIPaintLocation();
	}
	UIFinishDrawing();

	zone_unlock(lz_flags);
	zone_unlock(lz_world);
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

typedef enum {
	loc_date = 0,
	loc_credits,
	loc_population,
	loc_position
} loc_screen;

#define	MIDX	1
#define	MIDY	2
#define	ENDX	4
#define	ENDY	8

/*! \brief the status positions */
struct StatusPositions {
	PointType point; /*!< point where item is located */
	PointType offset; /*!< offset of item from topleft */
	UInt32 extents; /*!< how far does this item reach */
};

/*! \brief the default shape/size of the various locations */
static RectangleType shapes[] = {
	{ {0, 0}, {0, 10} },  /* loc_date */
	{ {0, 0}, {0, 10} }, /* loc_credits */
	{ {0, 0}, {0, 10} }, /* loc_population */
	{ {0, 0}, {0, 10} } /* loc_position */
};

/*! \brief the positions of the items on screen - low resolution */
static const struct StatusPositions lrpositions[] = {
	{ {0, 0}, {0, 1}, MIDX },  /* loc_date */
	{ {0, 0}, {0, 1}, ENDY }, /* loc_credits */
	{ {0, 0}, {0, 1}, MIDX | ENDY }, /* loc_population */
	{ {0, 0}, {0, 1}, ENDX | ENDY } /* loc_position */
};

#if defined(HRSUPPORT)

/*! \brief the positions of the items on screen - high resolution */
static const struct StatusPositions hrpositions[] = {
	{ {1, 0}, {0, 1}, ENDY },  /* loc_date */
	{ {40, 0}, {0, 1}, ENDY }, /* loc_credits */
	{ {80, 0}, {0, 1}, ENDY }, /* loc_population */
	{ {140, 0}, {0, 1}, ENDY } /* loc_position */
};

/*!
 * \brief Get the valid array of status positions
 *
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
/*! \brief get low resolution positions */
#define	posAt(x)	&(lrpositions[(x)])
#endif
/*! \brief maximum size of positions array */
#define	MAXLOC		(sizeof (lrpositions) / sizeof (lrpositions[0]))

/*!
 * \brief Draw a status item at the position requested
 * \param location the item to print
 * \param text the text to display
 */
static void
DrawItem(loc_screen location, char *text)
{
	const struct StatusPositions *pos;
	Int16 sl;
	Coord tx;
	RectangleType *rt;

	pos = posAt(location);
	rt = shapes + location;
	if (rt->extent.x && rt->extent.y) {
		StartHiresDraw();
		_WinEraseRectangle(rt, 0);
		EndHiresDraw();
	}

	if (isDoubleOrMoreResolution())
		_FntSetFont(boldFont);
	sl = (Int16)StrLen(text);
	tx = FntCharsWidth(text, sl);
	switch (pos->extents & (MIDX | ENDX)) {
	case MIDX:
		rt->topLeft.x = (GETWIDTH() - tx) / 2 + pos->offset.x;
		break;
	case ENDX:
		rt->topLeft.x = GETWIDTH() - (tx + pos->offset.x);
		break;
	default:
		rt->topLeft.x = (Coord)((Int32)GETWIDTH() * pos->point.x /
		    BASEWIDTH);
	}
	if (pos->extents & ENDY) {
		rt->topLeft.y = GETHEIGHT() - (rt->extent.y + pos->offset.y);
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

/*!
 * \brief Check a click in the display.
 * \param x the x location
 * \param y the y location
 * \return the item, or -1 if not one of the items.
 *
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
			return ((Int16)i);
	}
	return (-1);
}

/*!
 * \brief Perform an action based on the location on screen that was clicked.
 * \param x the x location
 * \param y the y location
 */
static Boolean
CheckTextClick(Coord x, Coord y)
{
	int t = UICheckOnClick(x, y);
	if (t == -1)
		return (0);
	switch (t) {
	case loc_date:
		break;
	case loc_credits:
		doKeyEvent(keBudget);
		break;
	case loc_population:
		doKeyEvent(kePopulation);
		break;
	case loc_position:
		if (!GETMINIMAPVISIBLE())
			doKeyEvent(keMap);
		break;
	default:
		break;
	}
	return (1);
}

void
UIPaintDate(void)
{
	char temp[20];

	if (IsDeferDrawing())
		return;

	getDate((char *)temp);
	DrawItem(loc_date, temp);
}

void
UIPaintCredits(void)
{
	char temp[20];
	char scale;
	Int32 credits;
#if defined(HRSUPPORT)
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	if (IsDeferDrawing())
		return;

	credits = scaleNumber32((Int32)getCredits(), &scale);
	StrPrintF(temp, "$: %li%c", credits, scale);
	DrawItem(loc_credits, temp);
#if defined(HRSUPPORT)
	if (isHires()) {
		bitmapHandle = DmGetResource(TBMP, bitmapID_coin);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		StartHiresDraw();
		_WinDrawBitmap(bitmap, shapes[loc_credits].topLeft.x - 11,
		    shapes[loc_credits].topLeft.y);
		EndHiresDraw();
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	}
#endif
}

void
UIPaintLocation(void)
{
	char temp[20];
#if defined(HRSUPPORT)
	MemHandle bitmapHandle;
	BitmapPtr bitmap;
#endif

	if (IsDeferDrawing())
		return;

	StrPrintF(temp, "%02u,%02u", getMapXPos(), getMapYPos());

	DrawItem(loc_position, temp);
#if defined(HRSUPPORT)
	if (isHires()) {
		bitmapHandle = DmGetResource(TBMP, bitmapID_loca);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		StartHiresDraw();
		_WinDrawBitmap(bitmap, shapes[loc_position].topLeft.x - 11,
		    shapes[loc_position].topLeft.y);
		EndHiresDraw();
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	}
#endif
}

void
UIPaintBuildIcon(void)
{
	MemHandle bitmaphandle;
	BitmapPtr bitmap;

	if (IsDeferDrawing())
		return;

	bitmaphandle = DmGetResource(TBMP,
	    (UInt16)(bitmapID_iconBulldoze +
		(((nSelectedBuildItem <= Be_Extra)) ?
		nSelectedBuildItem : OFFSET_EXTRA)));

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
		UIPaintToolBar();
	}
#endif
}

Int8
UIClipped(UInt16 xpos, UInt16 ypos)
{
	return ((xpos < (UInt16)getMapXPos()) ||
	    (xpos >= (UInt16)(getMapXPos() + getVisibleX())) ||
	    (ypos < (UInt16)getMapYPos()) ||
	    (ypos >= (UInt16)(getMapYPos() + getVisibleY())));
}

void
UIPaintSpeed(void)
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
	    GETWIDTH() - 12, 2, winPaint);
	EndHiresDraw();

#if defined(SONY_CLIE)
	if (IsSony()) {
		bitmapHandle = DmGetResource(TBMP, bitmapID_updn + jog_lr);
		/* place at rt - (12 + 8), 1 */
		if (bitmapHandle) {
			bitmap = MemHandleLock(bitmapHandle);
			if (bitmap) {
				StartHiresDraw();
				_WinDrawBitmap(bitmap,
				    GETWIDTH() - (12 + 8), 1);
				EndHiresDraw();
				MemPtrUnlock(bitmap);
			}
			DmReleaseResource(bitmapHandle);
		}
	}

#endif
}

void
UIPaintPopulation(void)
{
	char temp[20];
	Char scale;
	Int32 popul;

	if (IsDeferDrawing())
		return;

	popul = (Int32)getPopulation();
	popul = scaleNumber32(popul, &scale);

	StrPrintF(temp, "Pop: %li%c", popul, scale);
	DrawItem(loc_population, temp);
#if defined(HRSUPPORT)
	if (isHires()) {
		MemHandle bitmapHandle;
		BitmapPtr bitmap;

		bitmapHandle = DmGetResource(TBMP, bitmapID_popu);
		if (bitmapHandle == NULL)
			return;
		bitmap = MemHandleLock(bitmapHandle);
		StartHiresDraw();
		_WinDrawBitmap(bitmap, shapes[loc_population].topLeft.x - 11,
		    shapes[loc_population].topLeft.y);
		EndHiresDraw();
		MemPtrUnlock(bitmap);
		DmReleaseResource(bitmapHandle);
	}
#endif
}

void
MapHasJumped(void)
{
}

/*!
 * \brief is the ROM on this machine compatible
 * \param requiredVersion the asked for version (3.5)
 * \param launchFlags flags for launching
 * \return true if OK, false otherwise
 *
 * Checks if the rom is compatible so we can pay the game. It may be
 * partially compatible (3.1+) in which caase flag that fact
 * Also permit/deny direct structure access depending on the version.
 */
static Err
RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 version;

	/* See if we're on in minimum required version of the ROM or later. */
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);

	WriteLog("Rom Version: 0x%lx\n", (unsigned long)version);
	Clear35ROM();

	if (version < requiredVersion) {
		if ((launchFlags &
		    (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
		    (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {
			if (version > sysMakeROMVersion(3, 1, 0, 0, 0) &&
			    FrmAlert(alertID_RomIncompatible) == 1) {
				return (0);
			}

			/*
			 * Pilot 1.0 will continuously relaunch this app
			 * unless we switch to another safe one.
			 */
			if (version < sysMakeROMVersion(2, 0, 0, 0, 0)) {
				AppLaunchWithCommand(sysFileCDefaultApp,
				    sysAppLaunchCmdNormalLaunch, NULL);
			}
		}
		return (sysErrRomIncompatible);
	}
	Set35ROM();

	if (version >= sysMakeROMVersion(4, 0, 0, sysROMStageRelease, 0))
		Set40ROM();
	else
		Clear40ROM();

	ClearScaleModes();
	if (errNone == FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version)) {
		if (version >= 5)
			SetScaleModes();
	}

	return (0);
}

/*! \brief list of speeds in game */
static UInt8 speedslist[] = {
	SPEED_PAUSED, SPEED_SLOW, SPEED_MEDIUM, SPEED_FAST, SPEED_TURBO
};
/*! \brief maximum speed */
#define	MAX_SPEED	(sizeof (speedslist) / sizeof (speedslist[0]))

/*!
 * \brief get the offset of the speed into the speedslist array
 *
 * This is used to calculate the index into the speed bitmap for display
 * purposes
 * \return the index into the array, or 0 as a default
 */
static Int16
speedOffset(void)
{
	UInt16 i;

	for (i = 0; i < MAX_SPEED; i++)
		if (getLoopSeconds() == speedslist[i])
			return ((Int16)i);
	return (0);
}

/*!
 * \brief cycle up to the next speed
 *
 * This will loop back around to paused when you exceed turbo
 */
static void
cycleSpeed(void)
{
	UInt16 i = 0;

	while (i < MAX_SPEED) {
		if (speedslist[i++] == getLoopSeconds())
			break;
	}
	setLoopSeconds(speedslist[i == MAX_SPEED ? 0 : i]);
}

/*!
 * \brief Do the command against the key passed
 * \param event the event that is being triggered
 * \return 1 if the event was dealt with, 0 otherwise.
 */
static Int16
doKeyEvent(keyEvent event)
{
	switch (event) {
	case keIgnore:
		break;
	case keUp:
		ScrollDisplay(dtUp);
		break;
	case keDown:
		ScrollDisplay(dtDown);
		break;
	case keLeft:
		ScrollDisplay(dtLeft);
		break;
	case keRight:
		ScrollDisplay(dtRight);
		break;
	case kePopup:
		UIDoQuickList();
		break;
	case keMap:
		FrmGotoForm(formID_map);
		break;
	case keBudget:
		FrmGotoForm(formID_budget);
		break;
	case kePopulation:
		/* No-Op at moment */
		break;
	case kePassthrough:
		return (0);
	case keToolBulldozer:
	case keToolResidential:
	case keToolCommercial:
	case keToolIndustrial:
	case keToolRoad:
	case keToolRail:
	case keToolCoalPlant:
	case keToolNuclearPlant:
	case keToolPowerLine:
	case keToolWaterPump:
	case keToolWaterPipe:
	case keToolTree:
	case keToolLake:
	case keToolFireStation:
	case keToolPoliceStation:
	case keToolArmyBase:
	case keToolQuery:
	case keUnitFire:
	case keUnitPolice:
	case keUnitArmy:
		UISetSelectedBuildItem(event - keToolBulldozer);
		addGraphicUpdate(gu_buildicon);
		break;

#if defined(SONY_CLIE)
	/*
	 * if the draw window doesn't occupy most of the screen ...
	 *  - for example if it's a menu
	 * allow jog assist to work.
	 */
	case keJogUp:
		if (!IsDrawWindowMostOfScreen())
			return (0);
		if (jog_lr)
			ScrollDisplay(dtLeft);
		else
			ScrollDisplay(dtUp);
		break;
	case keJogDown:
		if (!IsDrawWindowMostOfScreen())
			return (0);
		if (jog_lr)
			ScrollDisplay(dtRight);
		else
			ScrollDisplay(dtDown);
		break;
	case keJogRelease:
		if (!IsDrawWindowMostOfScreen())
			return (0);
		jog_lr = 1 - jog_lr;
		addGraphicUpdate(gu_location);
		break;
#endif /* SONY_CLIE */
	default:
		break;
	}
	return (1);
}

/*!
 * \brief process a hard button being pressed
 * \param key the key what was received
 * \return the code from performing the button event
 */
static Int16
HardButtonEvent(ButtonKey key)
{
	return (doKeyEvent(gameConfig.pc.keyOptions[key]));
}

void
clearProblemFlags(void)
{
	ClearLowOutMoneyFlags();
	ClearLowOutPowerFlags();
	ClearLowOutWaterFlags();
}

/*!
 * \brief structure for mapping the silk keys to 'button keys'
 *
 * The button keys are known 'event types'.
 */
static struct _silkKeys {
	UInt16 vChar;	/*!< the character that was received */
	ButtonKey event; /*!< the button key event that was sent. */
} silky[] = {
	{ pageUpChr, BkHardUp },
	{ pageDownChr, BkHardDown },
	{ vchrHard1, BkCalendar },
	{ vchrHard2, BkAddress },
	{ vchrHard3, BkToDo },
	{ vchrHard4, BkMemo },
	{ vchrFind, BkFind },
	{ 1, BkCalc },
#if defined(HRSUPPORT)
	{ vchrRockerUp, BkHardUp },
	{ vchrRockerDown, BkHardDown },
	{ vchrRockerLeft, BkHardLeft },
	{ vchrRockerRight, BkHardRight },
	{ vchrRockerCenter, BkRockerCenter },
#if defined(SONY_CLIE)
	{ vchrJogUp, BkJogUp },
	{ vchrJogDown, BkJogDown },
	{ vchrJogRelease, BkJogRelease },
#endif
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

/*!
 * \brief process a virtual key press
 * \param key the key that was pressed
 * \return a processed hardbutton event or 0 for not mapped
 */
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

#if defined(HRSUPPORT)
/*! \brief the tool bar bitmap */
static BitmapType *pToolbarBitmap;
/*! \brief the low-resolution bitmap (v2) for palmOS5 compatibility */
static BitmapType *pOldBitmap;

/*! \brief release the toolbar bitmaps */
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

/*!
 * \brief paint the toolbar bitmaps onto the screen
 *
 * Paints onto the cache bitmap (to speed future painting)
 * \param startx the starting x location
 * \param starty the starting y location
 * \param spacing the space between each item
 */
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

/*!
 * \brief get the dimensions of a bitmap
 * \param resID the ID of the resource
 * \param width the width
 * \param height the height
 * \return 0 if it read the dimensions, 0 otherwise
 */
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

/*! \brief the width of the toolbar */
static Coord tbWidth;
/*! \brief the width of a bitmap in the toolbar */
static Coord bWidth;

/*! \brief draw the toolbar on the screen */
static void
UIPaintToolBar(void)
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
		if (highDensityFeatureSet() && pToolbarBitmap != NULL) {
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
			drawToolBitmaps(((GETWIDTH() - tbWidth) >> 1) + 2,
			    2, bWidth);
			EndHiresDraw();
			return;
		}
	}
	if (pToolbarBitmap != NULL) {
		StartHiresDraw();
		WriteLog("Toolbar at: %ld (width=%ld) [ swidth=%ld ]\n",
		    (long)((GETWIDTH() - tbWidth) >> 1), (long)tbWidth,
		    (long)GETWIDTH());
		_WinDrawBitmap(pToolbarBitmap, (GETWIDTH() - tbWidth) >> 1, 2);
		EndHiresDraw();
	}
}

/*!
 * \brief check if a location on the toolbar was clicked.
 *
 * We already know that the y axis is correct for the toolbar, we're just
 * checking the X axis.
 * This has effect of either setting the current build item, or popping up
 * an extra build list.
 * \param xpos the location on the x axis to check for clicking
 * \todo Make the toolbar customizable
 */
static void
toolBarCheck(Coord xpos)
{
	int id;

	/* We've already confirmed the y-axis. */
	if ((xpos < ((GETWIDTH() - tbWidth) >> 1)) ||
	    (xpos > ((GETWIDTH() + tbWidth) >> 1))) return;

	id = (xpos - ((GETWIDTH() - tbWidth) >> 1)) / bWidth;
	WriteLog("Xpos: %ld [ %ld / %ld ] %d %d \n", (long)xpos,
	    (long)tbWidth, (long)GETWIDTH(), id, (int)bWidth);
	if (id == (bitmapID_iconExtra - bitmapID_iconBulldoze)) {
		UIPopUpExtraBuildList();
	} else {
		UISetSelectedBuildItem(id);
	}
	addGraphicUpdate(gu_buildicon);
}

/*!
 * \brief react to the drawing area being resized.
 *
 * This can happen when the axis is reoriented, or the softsilk area is
 * removed.
 * \param form the form
 * \param hOff the horizontal change on the screen
 * \param vOff the vertical offset of the screen
 * \param draw do I redraw the screen after the resize event.
 */
static void
pcResizeDisplay(FormPtr form, Int16 hOff, Int16 vOff, Boolean draw)
{
	RectangleType disRect;
	Int16 loc;
	Int16 nWidth;
	Int16 nHeight;

	if (hOff == 0 && vOff == 0)
		return;

	WinGetDrawWindowBounds(&disRect);

	nWidth = GETWIDTH() + scaleCoord(hOff);
	nHeight = GETHEIGHT() + scaleCoord(vOff);

	SETWIDTH(nWidth);
	SETHEIGHT(nHeight);

	ResetViewable();

	for (loc = 0; loc < (Int16)MAXLOC; loc++)
		shapes[loc].extent.x = 0;

	/* XXX: Resize the gadgets - coordinates are natural */
	rPlayGround.extent.x = normalizeCoord(GETWIDTH());
	rPlayGround.extent.y = normalizeCoord(GETHEIGHT()) - 2 * 10;

	/* Resize the minimap */
	disRect.topLeft.x = normalizeCoord(GETWIDTH()) - 32;
	disRect.topLeft.y = normalizeCoord(GETHEIGHT()) - 32;
	disRect.extent.x = 32;
	disRect.extent.y = 32;
	minimapPlace(&disRect);

	if (draw) {
		collapsePreRedraw(form);
		FrmDrawForm(form);
		DrawGame(1);
	}
}

#endif /* HRSUPPORT */

#if defined(SONY_CLIE)

/*! \brief  Pauses the game if you flick the 'hold' switch */
static void
HoldHook(UInt32 held)
{
	if (held) game.gameLoopSeconds = SPEED_PAUSED;
	addGraphicUpdate(gu_speed);
}

#endif

#if defined(LOGGING)

#include <unix_stdarg.h>

void
WriteLog(char *s, ...)
{
	va_list args;
	HostFILE * hf = NULL;
	Char text[0x100];

	hf = HostFOpen("\\tmp\\pcity.log", "a");
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

	hf = HostFOpen("\\tmp\\pcity-buildcount.log", "a");
	if (hf) {
		va_start(args, s);
		StrVPrintF(text, s, args);

		HostFPrintF(hf, text);
		HostFClose(hf);
		va_end(args);
	}
}

#endif
