/*
 * This file handles savegame front end.
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
#include <ui.h>
#include <globals.h>
#include <zakdef.h>
#include <drawing.h>
#include <build.h>
#include <handler.h>
#include <simulation.h>
#include <resCompat.h>
#include <palmutils.h>

static void UpdateSaveGameList(void) SAVE_SECTION;
static void CleanSaveGameList(void) SAVE_SECTION;
static void DeleteFromList(void) SAVE_SECTION;
static int  LoadFromList(void) SAVE_SECTION;

#define	LASTGAME		((UInt16)~0)
#define	MAXSAVEGAMECOUNT	50

static char **citylist = NULL;

/*
 * Load the game form the auto-save slot
 */
int
UILoadAutoGame(void)
{
	return (LoadAutoSave());
}

/*
 * Delete the game stored in the auto-save slot
 */
void
UIClearAutoSaveSlot(void)
{
	DeleteAutoSave();
}

/*
 * save the city that is currently being used.
 * Needs to find the city in the set of saved cities
 */
void
UISaveMyCity(void)
{
	SaveGameByName(game.cityname);
}

/*
 * Save the autosave game.
 * This is a different to 'my city'. the autosave slot is *Distinct* from
 * the mycity slot. If you don't save the game before loading a new-one then
 * you've lost all the changes since the last time you explicitly saved it,
 * and for a lot of people that is the time they first loaded the city.
 */
void
UISaveAutoGame(void)
{
	SetAutoSave(game.cityname);
	UISaveMyCity();
}

/*
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

/*
 * the create button is pressed in the new city form
 */
static void
cnCreateButtonPressed(void)
{
	char *pGameName;
	FormPtr form;

	form = FrmGetActiveForm();
	if (FrmGetControlValue(form, FrmGetObjectIndex(form, buttonID_Easy)))
		SetDifficultyLevel(0);
	if (FrmGetControlValue(form, FrmGetObjectIndex(form, buttonID_Medium)))
		SetDifficultyLevel(1);
	if (FrmGetControlValue(form, FrmGetObjectIndex(form, buttonID_Hard)))
		SetDifficultyLevel(2);

	pGameName = FldGetTextPtr((FieldPtr)GetObjectPtr(form,
	    fieldID_newGameName));
	if (pGameName != NULL) {
		MemSet(game.cityname, CITYNAMELEN, '\0');
		StrNCopy((char *)game.cityname, pGameName, CITYNAMELEN);
		while (GameExists(game.cityname)) {
			int slen = StrLen(game.cityname);
			if (slen < CITYNAMELEN-1) {
				game.cityname[slen-1] = '0' - 1;
				game.cityname[slen] = '\0';
				slen++;
			}
			game.cityname[slen - 1]++;
		}
		CreateNewSaveGame(game.cityname);
		CleanSaveGameList();
		if (LoadGameByName(game.cityname) != -1) {
			FrmEraseForm(form);
			form = FrmGetFormPtr(formID_files);
			if (form != NULL)
				FrmEraseForm(form);
			FrmGotoForm(formID_pocketCity);
		} else {
			UpdateSaveGameList();
		}
	} else {
		game.cityname[0] = '\0';
		WriteLog("No name specified\n");
	}
}

/*
 * Set up the new file form.
 */
static FormPtr
cityNewSetup(void)
{
	FormPtr form = FrmGetActiveForm();

	FrmSetFocus(form, FrmGetObjectIndex(form, fieldID_newGameName));
	FrmSetControlValue(form, FrmGetObjectIndex(form, buttonID_Easy), 1);

	return (form);
}

/*
 * Handler for the new file form.
 * Makes sure that the text field is given focus.
 */
Boolean
hFilesNew(EventPtr event)
{
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		SetGameNotInProgress();
		FrmDrawForm(cityNewSetup());
		handled = true;
		break;
	case frmCloseEvent:
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
	default:
		break;
	}

	return (handled);
}

/*
 * set up the files form
 */
static FormPtr
filesSetup(void)
{
	FormPtr form = FrmGetActiveForm();

	SetGameNotInProgress();
	UpdateSaveGameList();

	return (form);
}

/*
 * Handler for the list of cities dialog.
 * Ensures that the list is populated with all the cities in the save game
 * slots.
 */
Boolean
hFiles(EventPtr event)
{
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		FrmDrawForm(filesSetup());
		handled = true;
		break;
	case frmCloseEvent:
		CleanSaveGameList();
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
	default:
		break;
	}

	return (handled);
}

/*
 * Update the list of save games.
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

/*
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

/*
 * Load a game from the list of save games
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

/*
 * Delete a game from the list of savegames.
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
