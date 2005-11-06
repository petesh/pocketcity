/*!
 * \file
 * \brief This file handles savegame front end
 */

#include <PalmOS.h>
#include <StringMgr.h>
#include <unix_string.h>
#include <unix_stdlib.h>
#include <StdIOPalm.h>
#include <stddef.h>
#include <simcity.h>
#include <simcity_resconsts.h>
#include <savegame.h>
#include <savegame_be.h>
#include <logging.h>
#include <globals.h>
#include <zakdef.h>
#include <drawing.h>
#include <build.h>
#include <handler.h>
#include <simulation.h>
#include <resCompat.h>
#include <palmutils.h>
#include <repeathandler.h>

static void UpdateSaveGameList(void) SAVE_SECTION;
static void CleanSaveGameList(void) SAVE_SECTION;
static void DeleteFromList(void) SAVE_SECTION;
static int  LoadFromList(void) SAVE_SECTION;

static FormPtr filesSetup(FormPtr form) SAVE_SECTION;

static void cnCancelButtonPressed(void) SAVE_SECTION;
static void cnCreateButtonPressed(void) SAVE_SECTION;
static FormPtr cityNewSetup(FormPtr form) SAVE_SECTION;
static void cnFieldContentChanged(UInt16 fieldID) SAVE_SECTION;
static void cnRepeatPressed(EventPtr event) SAVE_SECTION;

#if defined(HRSUPPORT)
static Boolean resizeSavegameForm(FormPtr form, Int16 hOff,
    Int16 vOff) SAVE_SECTION;
#else
#define	resizeSavegameForm(F, H, V)
#endif

/*! \brief constant for 'add new game' */
#define	LASTGAME		((UInt16)~0)
/*! \brief maximum number of savegames in the Palm OS */
#define	MAXSAVEGAMECOUNT	50

/*! \brief list of cities n the savegame */
static char **citylist;

int
UILoadAutoGame(void)
{
	return (LoadAutoSave());
}

void
UIClearAutoSaveSlot(void)
{
	DeleteAutoSave();
}

/*!
 * Needs to find the city in the set of saved cities
 */
void
UISaveMyCity(void)
{
	SaveGameByName((char *)game.cityname);
}

/*!
 * The autosave slot is simply the name of the current autosave city.
 */
void
UISaveAutoGame(void)
{
	SetAutoSave((char *)game.cityname);
	UISaveMyCity();
}

/*!
 * Makes sure that the text field is given focus.
 */
Boolean
hFilesNew(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	UInt16 fieldID;
	WChar chr;

	switch (event->eType) {
	case frmOpenEvent:
		SetGameNotInProgress();
		form = FrmGetActiveForm();
		SetSilkResizable(form, true);
		collapseMove(form, CM_DEFAULT, NULL, NULL);
		FrmDrawForm(cityNewSetup(form));
		handled = true;
		break;
	case frmCloseEvent:
		break;
	case keyDownEvent:
		form = FrmGetActiveForm();
		fieldID = FrmGetObjectId(form, FrmGetFocus(form));
		if (fieldID < fieldID_width || fieldID > fieldID_width)
			break;

		chr = event->data.keyDown.chr;
		if (((chr >= chrDigitZero) && (chr <= chrDigitNine)) ||
			(chr == chrBackspace)) {
			EvtEnqueueKey(chrTilde, 0, 0);
			break;
		}
		if (chr == chrTilde)
			cnFieldContentChanged(fieldID);
		break;

	case ctlRepeatEvent:
		cnRepeatPressed(event);
		break;
	case ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case buttonID_FilesNewCreate:
			cnCreateButtonPressed();
			handled = true;
			break;
		case buttonID_FilesNewCancel:
			cnCancelButtonPressed();
			handled = true;
			break;
		}
		break;
#if defined(HRSUPPORT)
	case winDisplayChangedEvent:
#if defined(SONY_CLIE)
	case vchrSilkResize:
#endif
		form = FrmGetActiveForm();
		if (collapseMove(form, CM_DEFAULT, NULL, NULL)) {
			FrmEraseForm(form);
			FrmDrawForm(form);
		}
		handled = true;
		break;
#endif
	default:
		break;
	}

	return (handled);
}

/*!
 * \brief structure for the map size manipulation
 */
static buttonmapping_t mapsize_map[] = {
	{ rbutton_widthdown, rbutton_widthup, fieldID_width, 50, 120, 0, 0 },
	{ rbutton_heightdown, rbutton_heightup, fieldID_height, 50, 120, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 }
};

/*!
 * \brief deal with the repeat buttons being pressed
 * \param event the event that came from the repeater
 */
static void
cnRepeatPressed(EventPtr event)
{
	UInt16 control = event->data.ctlRepeat.controlID;
	(void) processRepeater(mapsize_map, control, true, NULL);
}

/*!
 * \brief deal with the width or height being changed directly
 * \param fieldID the field that changed
 */
static void
cnFieldContentChanged(UInt16 fieldID)
{
	(void) processRepeater(mapsize_map, fieldID, false, NULL);
}

/*
 * \brief react to cancel button being pressed on newcity form.
 *
 * The cancel button is pressed in the new city form
 * Note the use of the returntoform api, this is means the last
 * form is reopened, and the frmcloseevent isn't sent to the
 * form that's being closed.
 */
static void
cnCancelButtonPressed(void)
{
	game.cityname[0] = '\0';
	FrmReturnToForm(0);
}

/*!
 * \brief the create button is pressed in the new city form
 */
static void
cnCreateButtonPressed(void)
{
	char *pGameName;
	FormPtr form;
	UInt8 level = 0;
	UInt8 width;
	UInt8 height;
	FieldPtr fp;
	MemHandle mh;
	MemPtr mp;

	InitGameStruct();

	form = FrmGetActiveForm();
	if (FrmGetControlValue(form, FrmGetObjectIndex(form, buttonID_Easy)))
		level = 0;
	if (FrmGetControlValue(form, FrmGetObjectIndex(form, buttonID_Medium)))
		level = 1;
	if (FrmGetControlValue(form, FrmGetObjectIndex(form, buttonID_Hard)))
		level = 2;
	setDifficultyLevel(level);

	if (FrmGetControlValue(form, FrmGetObjectIndex(form,
	    buttonID_dis_off)))
		level = 0;
	if (FrmGetControlValue(form, FrmGetObjectIndex(form,
	    buttonID_dis_one)))
		level = 1;
	if (FrmGetControlValue(form, FrmGetObjectIndex(form,
	    buttonID_dis_two)))
		level = 2;
	if (FrmGetControlValue(form, FrmGetObjectIndex(form,
	    buttonID_dis_three)))
		level = 3;
	setDisasterLevel(level);

	fp = (FieldPtr)GetObjectPtr(form, fieldID_width);
	mh = FldGetTextHandle(fp);
	mp = MemHandleLock(mh);
	width = (UInt8)StrAToI(mp);
	MemHandleUnlock(mh);
	fp = (FieldPtr)GetObjectPtr(form, fieldID_height);
	mh = FldGetTextHandle(fp);
	mp = MemHandleLock(mh);
	height = (UInt8)StrAToI(mp);
	MemHandleUnlock(mh);
	setMapSize(width, height);
	ConfigureNewGame();

	pGameName = FldGetTextPtr((FieldPtr)GetObjectPtr(form,
	    fieldID_newGameName));
	if (pGameName != NULL) {
		MemSet(game.cityname, CITYNAMELEN, '\0');
		StrNCopy((char *)game.cityname, pGameName, CITYNAMELEN);
		while (GameExists((char *)game.cityname)) {
			UInt16 slen = StrLen((char *)game.cityname);
			if (slen < CITYNAMELEN-1) {
				game.cityname[slen-1] = '0' - 1;
				game.cityname[slen] = '\0';
				slen++;
			}
			game.cityname[slen - 1]++;
		}
		SaveGameByName((char *)game.cityname);
		CleanSaveGameList();
		if (LoadGameByName((char *)game.cityname) != -1) {
			FrmEraseForm(form);
			form = FrmGetFormPtr(formID_files);
			if (form != NULL) {
				FrmEraseForm(form);
				FrmDeleteForm(form);
			}
			FrmGotoForm(formID_pocketCity);
		} else {
			UpdateSaveGameList();
		}
	} else {
		game.cityname[0] = '\0';
		WriteLog("No name specified\n");
	}
}

/*!
 * \brief Set up the new file form.
 * \param form pointer to the form.
 * \return the form pointer.
 */
static FormPtr
cityNewSetup(FormPtr form)
{
	int i;
	MemHandle hText;
	MemPtr pText;

	FrmSetFocus(form, FrmGetObjectIndex(form, fieldID_newGameName));
	FrmSetControlValue(form, FrmGetObjectIndex(form, buttonID_Easy), 1);
	FrmSetControlValue(form, FrmGetObjectIndex(form, buttonID_dis_one), 1);

	for (i = 0; i < 2; i++) {
		hText = MemHandleNew(5);
		pText = MemHandleLock(hText);
		StrPrintF((char *)pText, "%u", 100);
		MemHandleUnlock(hText);
		FldSetTextHandle((FieldPtr)GetObjectPtr(form,
		    (UInt16)(fieldID_width + i)), hText);
	}

	return (form);
}

/*!
 * Ensures that the list is populated with all the cities in the save game
 * slots.
 */
Boolean
hFiles(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
#if defined(HRSUPPORT)
	Int16 hOff = 0, vOff = 0;
#endif

	switch (event->eType) {
	case frmOpenEvent:
		form = FrmGetActiveForm();
		SetSilkResizable(form, true);
		collapseMove(form, CM_DEFAULT, &hOff, &vOff);
		resizeSavegameForm(form, hOff, vOff);
		filesSetup(form);
		FrmDrawForm(form);
		handled = true;
		break;
	case frmCloseEvent:
		CleanSaveGameList();
		SetSilkResizable(NULL, false);
		break;
	case ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case buttonID_FilesNew:
			/* create new game and add it to the list */
			FrmPopupForm(formID_filesNew);
			handled = true;
			break;
		case buttonID_FilesLoad:
			/* create new game */
			if (LoadFromList() != -1)
				FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		case buttonID_FilesDelete:
			DeleteFromList();
			CleanSaveGameList();
			UpdateSaveGameList();
			handled = true;
			break;
		}
		break;
#if defined(HRSUPPORT)
	case winDisplayChangedEvent:
#if defined(SONY_CLIE)
	case vchrSilkResize:
#endif
		form = FrmGetActiveForm();
		if (collapseMove(form, CM_DEFAULT, &hOff, &vOff)) {
			resizeSavegameForm(form, hOff, vOff);
			FrmDrawForm(form);
		}
		handled = true;
		break;
#endif
	default:
		break;
	}

	return (handled);
}

/*!
 * \brief set up the files form
 * \param form the form to be dealt with
 * \return the form passed in
 */
static FormPtr
filesSetup(FormPtr form)
{
	SetGameNotInProgress();
	UpdateSaveGameList();

	return (form);
}

#if defined(HRSUPPORT)
/*!
 * \brief Resize the savegame form.
 *
 * Deal with the change of the dimensions of the old form
 * \param form the pointer to the form that is to be resized
 * \param hOff the offset of the form horizontally
 * \param vOff the offset of the form vertically
 * \return true if the form was moved/resized
 */
static Boolean
resizeSavegameForm(FormPtr form, Int16 hOff, Int16 vOff)
{
	if (!(hOff || vOff))
		return (false);

	RearrangeObjectOnly(form, listID_FilesList, 0, 0, hOff, vOff);
	RearrangeObjectOnly(form, buttonID_FilesNew, 0, vOff, 0, 0);
	RearrangeObjectOnly(form, buttonID_FilesLoad, 0, vOff, 0, 0);
	RearrangeObjectOnly(form, buttonID_FilesDelete, 0, vOff, 0, 0);
	return (true);
}

#endif /* HRSUPPORT */

/*!
 * \brief Update the list of save games.
 *
 * Can be called from 2 contexts ... from the delete items dialog
 * or from the main save game dialog. That's the reason for the
 * check agains the active form and the formID_files pointer
 */
static void
UpdateSaveGameList(void)
{
	int count;
	FormPtr files_form;
	ListPtr list;

	if (citylist != NULL)
		FreeCityNames(citylist);

	citylist = CityNames(&count);

	files_form = FrmGetFormPtr((UInt16)formID_files);

	if (files_form == NULL) {
		WriteLog("could not get the files form pointer\n");
		return;
	}
	list = (ListPtr)GetObjectPtr(files_form, listID_FilesList);

	if (list == NULL) {
		WriteLog("Could not get the pointer for list\n");
		return;
	}

	if (NULL == citylist) {
		LstSetListChoices(list, NULL, 0);
		if (files_form == FrmGetActiveForm() && FrmVisible(files_form))
			LstDrawList(list);
		return; /* no cities */
	}

	/* update list */
	LstSetListChoices(list, citylist, count);
	if (files_form == FrmGetActiveForm() && FrmVisible(files_form))
		LstDrawList(list);
}

/*!
 * \brief clean up the savegame list.
 *
 * free the memory that has been allocated in the files form.
 * Makes sure that all the objects allocated have been released.
 */
static void
CleanSaveGameList(void)
{
	ListPtr listp;
	FormPtr form = FrmGetFormPtr(formID_files);

	if (form == NULL)
		return;
	listp = (ListPtr)GetObjectPtr(form, listID_FilesList);
	if (listp == NULL)
		return;

	LstSetListChoices(listp, NULL, 0);

	FreeCityNames(citylist);
	citylist = NULL;
}

/*!
 * \brief Load a game from the list of save games
 *
 * Uses the index into the array, as the array is
 * unsorted, and maps 1:1 with the underlying savegames
 */
static int
LoadFromList(void)
{
	FormPtr form = FrmGetFormPtr(formID_files);
	ListPtr listp;
	int index;

	listp = (ListPtr)GetObjectPtr(form, listID_FilesList);
	index = LstGetSelection(listp);
	if (index >= 0) {
		char *text = LstGetSelectionText(listp, index);
		return (LoadGameByName(text));
	} else {
		return (-1);
	}
}

/*!
 * \brief Delete a game from the list of savegames.
 *
 * uses the index into the list to choose the city to delete
 * This is agian because the array is unsorted
 */
static void
DeleteFromList(void)
{
	FormPtr form = FrmGetFormPtr(formID_files);
	ListPtr listp;
	Int16 index;

	listp = (ListPtr)GetObjectPtr(form, listID_FilesList);
	index = LstGetSelection(listp);
	if (index != noListSelection) {
		char *name;
		name = LstGetSelectionText(listp, index);
		DeleteGameByName(name);
	}
}
