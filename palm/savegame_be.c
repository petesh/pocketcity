
/*
 * This file handles savegame back end
 */
#include <PalmOS.h>
#include <StringMgr.h>
#include <Form.h>

#include <stddef.h>
#include <zakdef.h>
#include <simcity.h>
#include <globals.h>
#include <handler.h>
#include <ui.h>
#include <palmutils.h>
#include <simcity_resconsts.h>
#include <resCompat.h>
#include <savegame_be.h>
#include <mem_compat.h>

#define	LASTGAME	((UInt16)~0)
#define	MAXSAVEGAMECOUNT	50
#define	DEAD	"PCNO"

static DmOpenRef OpenMyDB(void) SAVE_SECTION;
static UInt16 FindGameByName(char *name) SAVE_SECTION;
static int ReadCityRecord(MemHandle rec, GameStruct *gs,
    MemPtr *wp) SAVE_SECTION;
static void WriteCityRecord(MemHandle rec, GameStruct *gs,
    MemPtr wp) SAVE_SECTION;
static int SaveGameByIndex(UInt16 index) SAVE_SECTION;
static int LoadGameByIndex(UInt16 index) SAVE_SECTION;
static void getAutoSaveName(char *name) SAVE_SECTION;
static void DeleteGameByIndex(UInt16 index) SAVE_SECTION;
static Int16 comparator(void *p1, void *p2, Int32 other) SAVE_SECTION;

/*
 * Open up the savegame database.
 * If it does not exist, then it tries to create it.
 * It will return a reference to the database if successful, NULL otherwise.
 */
static DmOpenRef
OpenMyDB()
{
	Err err = 0;
	DmOpenRef db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(),
	    dmModeReadWrite);
	if (!db) {
		err = DmCreateDatabase(0, SGNAME, GetCreatorID(), SGTYP, false);
		if (err)
			return (NULL); /* couldn't create */
		db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(),
		    dmModeReadWrite);
	}
	return (db);
}

/*
 * Find a savegame by the name passed
 */
static UInt16
FindGameByName(char *name)
{
	DmOpenRef db = NULL;
	MemHandle rec;
	GameStruct *pRec;
	UInt16 nRec;
	UInt16 gameindex = LASTGAME;

	db = OpenMyDB();
	if (db == NULL) {
		FrmCustomAlert(alertID_majorbad,
		    "Can't Open/Create the savegame database", NULL, NULL);
		return (gameindex);
	}
	nRec = DmNumRecords(db);
	for (gameindex = 1; gameindex < nRec; gameindex++) {
		rec = DmQueryRecord(db, gameindex);
		if (rec == NULL)
			continue;
		pRec = (GameStruct *)MemHandleLock(rec);
		if (StrCaselessCompare((const Char *)pRec->cityname,
		    name) == 0) {
			MemHandleUnlock(rec);
			break;
		}
		MemHandleUnlock(rec);
	}
	if (db != NULL) DmCloseDatabase(db);

	if (gameindex >= nRec)
		return (LASTGAME);
	else
		return (gameindex);
}

/*
 * Reset the viewable elements of the volatile game configuration.
 * We set the tile size (16 pels).
 * We reserve 2 tiles (one at the top, one at the bottom) for use
 * with status information.
 */
void
ResetViewable(void)
{
	WriteLog("Reset viewable\n");
	vgame.tileSize = 16;
	vgame.visible_x = sWidth / vgame.tileSize;
	vgame.visible_y = (sHeight / vgame.tileSize) - 2;
}

/*
 * Read the city record.
 */
static int
ReadCityRecord(MemHandle rec, GameStruct *gs, MemPtr *wp)
{
	char *ptemp;
	int rv = -1;

	ptemp = (char *)MemHandleLock(rec);
	if (ptemp == NULL)
		return (-1);
	if (StrNCompare(DEAD, (char *)ptemp, 4) == 0)
		goto leave_me;
	if (StrNCompare(SAVEGAMEVERSION, (char *)ptemp, 4) == 0) {
		UInt32 size;
		MemMove((void *)gs, ptemp, sizeof (GameStruct));
		size = gs->mapx * gs->mapy * 2;
		*wp = gRealloc(*wp, size);
		MemMove(*wp, ptemp + sizeof (GameStruct), size);
		rv = 0;
	} else {
		FrmAlert(alertID_invalidSaveVersion);
		rv = -1;
	}

leave_me:
	MemHandleUnlock(rec);
	return (rv);
}

/*
 * Write the city record
 */
static void
WriteCityRecord(MemHandle rec, GameStruct *gs, MemPtr wp)
{
	void *pRec;

	pRec = MemHandleLock(rec);
	/* write the header and some globals */
	DmWrite(pRec, 0, gs, sizeof (GameStruct));
	DmWrite(pRec, sizeof (GameStruct), (void *)(unsigned char *)wp,
	    gs->mapx * gs->mapy * 2);
	MemHandleUnlock(rec);
}

/*
 * Save a game into the database of savegames.
 */
static int
SaveGameByIndex(UInt16 index)
{
	DmOpenRef db = NULL;
	MemHandle rec;
	UInt16 attr = dmHdrAttrBackup;

	if (index == 0)
	return (0);

	db = OpenMyDB();
	if (db == NULL)
		return (-1);
	if (index <= DmNumRecords(db)) {
		rec = DmResizeRecord(db, index,
		    WorldSize() + sizeof (GameStruct));
		rec = DmGetRecord(db, index);
	} else {
		index = DmNumRecords(db) + 1;
		rec = DmNewRecord(db, &index,
		    WorldSize() + sizeof (GameStruct));
	}
	if (rec) {
		LockWorld();
		WriteCityRecord(rec, &game, worldPtr);
		UnlockWorld();
		DmReleaseRecord(db, index, true);
	}

	/* tag DB so it gets saved */
	DmSetDatabaseInfo(0, DmFindDatabase(0, SGNAME), NULL, &attr, NULL,
	    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	DmCloseDatabase(db);
	return (index);
}

/*
 * Check if a saved city by this name exists
 */
int
GameExists(char *name)
{
	return (FindGameByName(name) != LASTGAME);
}

/*
 * save the city that is currently being used.
 * Needs to find the city in the set of saved cities
 */
void
SaveGameByName(char *name)
{
	UInt16 index;
	index = FindGameByName(name);

	SaveGameByIndex(index);
}

/*
 * Create a new save game slot.
 * Save the city into it using a special save-game mode that says it is to
 * be reconfigured
 */
void
CreateNewSaveGame(char *name)
{
	DmOpenRef db;
	MemHandle rec;

	UInt16 index = dmMaxRecordIndex;

	db = OpenMyDB();

	/* no, this should NOT be an "else if" */
	if (db == NULL)
		return;
	if (DmNumRecords(db) >= MAXSAVEGAMECOUNT) {
		/* TODO: alert user - max is 50 savegames */
	} else {
		if (DmNumRecords(db) == 0) {
			/*
			 * create a record in slot 0 if it's not there.
			 * This is used for autosaves. It takes the name
			 * of the city
			 */
			rec = DmNewRecord(db, &index, CITYNAMELEN);
			if (rec) {
				MemPtr mp = MemHandleLock(rec);
				DmWrite(mp, 0, name, CITYNAMELEN);
				MemHandleUnlock(rec);
				DmReleaseRecord(db, index, true);
			}
		}
		index = dmMaxRecordIndex;
		rec = DmNewRecord(db, &index,
		    WorldSize() + sizeof (game));
		if (rec) {
			ResetViewable();
			ResumeGame();
			LockWorld();
			WriteCityRecord(rec, &game, worldPtr);
			UnlockWorld();
			DmReleaseRecord(db, index, true);
		}
	}
	DmCloseDatabase(db);
}

/*
 * Load a game by index into the list of savegames.
 */
static int
LoadGameByIndex(UInt16 index)
{
	DmOpenRef db;
	MemHandle rec;
	short int loaded = -1;

	db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(), dmModeReadOnly);

	if (!db)
		return (-1); /* no database */

	if (index == LASTGAME)
	index = DmNumRecords(db) - 1;
	rec = DmQueryRecord(db, index);
	if (rec) {
		LockWorld();
		loaded = ReadCityRecord(rec, &game, &worldPtr);
		UnlockWorld();
	}
	if (loaded != -1) {
		PostLoadGame();
		ResetViewable();
	}
	DmCloseDatabase(db);

	return (loaded);
}


/*
 * Load a game by name
 */
int
LoadGameByName(char *name)
{
	UInt16 gameindex = FindGameByName(name);

	if (gameindex != LASTGAME)
		return (LoadGameByIndex(gameindex));
	else
		return (-1);
}

/*
 * Get the name of the autosave city
 */
static void
getAutoSaveName(char *name)
{
	DmOpenRef db = OpenMyDB();
	MemHandle rec;
	MemPtr ptr;

	*name = '\0';
	if (db == NULL)
		return;

	rec = DmQueryRecord(db, 0);
	if (rec) {
		ptr = MemHandleLock(rec);
		MemMove(name, ptr, CITYNAMELEN);
		MemHandleUnlock(rec);
	}

	DmCloseDatabase(db);
}

/*
 * Load the autosave city.
 */
int
LoadAutoSave(void)
{
	char cityname[CITYNAMELEN];

	getAutoSaveName(cityname);

	if (*cityname != '\0') {
		/* Old format ... autosave == slot0 */
		if (StrNCompare(cityname, SAVEGAMEVERSION, 4) == 0)
			return (LoadGameByIndex(0));
	} else {
		return (-1);
	}

	return (LoadGameByName(cityname));
}

/*
 * set the autosave name
 */
void
SetAutoSave(char *name)
{
	DmOpenRef db = OpenMyDB();
	MemHandle mh;
	MemPtr mp;

	if (db == NULL)
		return;

	if (DmNumRecords(db) < 1)
		goto close_me;

	mh = DmGetRecord(db, 0);
	if (mh == NULL) {
		goto close_me;
	} else {
		mh = DmResizeRecord(db, 0, CITYNAMELEN);
	}

	mp = MemHandleLock(mh);

	if (mp == NULL)
		goto release;

	DmWrite(mp, 0, name, CITYNAMELEN);

	MemHandleUnlock(mh);

release:
	DmReleaseRecord(db, 0, true);
close_me:
	DmCloseDatabase(db);
}

/*
 * Clear the autosave name
 */
void
DeleteAutoSave(void)
{
	char buffer[CITYNAMELEN];

	*buffer = '\0';
	SetAutoSave(buffer);
}

/*
 * Delete the savegame from the list of savegames.
 * This will also compact the database of savegames. i.e. when the
 * savegame is deleted it is removed completely.
 */
static void
DeleteGameByIndex(UInt16 index)
{
	DmOpenRef db;

	db = OpenMyDB();
	if (db == NULL) {
		return;
	}

	if (DmNumRecords(db) < index) {
		goto close_me; /* index doesn't exist */
	}

	if (index > 0)
		DmRemoveRecord(db, index);

close_me:
	DmCloseDatabase(db);
}

/*!
 * \brief Delete a city from the DB by name
 * \param name the name of the city to delete
 */
void
DeleteGameByName(char *name)
{
	UInt16 gameindex = FindGameByName(name);

	if (gameindex != LASTGAME)
		DeleteGameByIndex(gameindex);
}

/*!
 * \brief rename a city
 * \param oldname the old name of the city
 * \param newname the new name of the city
 */
int
RenameCity(char *oldname, char *newname)
{
	DmOpenRef db;
	MemHandle rec;
	GameStruct *pRec;
	UInt16 gameindex;
	UInt16 nRec;
	Boolean dirty = false;

	db = OpenMyDB();

	if (db == NULL)
		return (-1); /* no database */
	nRec = DmNumRecords(db);
	for (gameindex = 0; gameindex < nRec; gameindex++) {
		rec = DmGetRecord(db, gameindex);
		if (rec == NULL)
			continue;
		pRec = MemHandleLock(rec);
		if (StrCompare((const Char *)pRec->cityname, oldname) == 0) {
			DmStrCopy(pRec, offsetof(GameStruct, cityname),
			    newname);
			dirty = true;
		}
		DmReleaseRecord(db, gameindex, dirty);
		if (dirty)
			break;
	}
	
	DmCloseDatabase(db);

	return (dirty);
}

int
CopyCity(Char *name)
{
	DmOpenRef db;
	MemHandle rec;
	GameStruct *pRec;
	UInt16 gameindex;
	UInt16 nRec;
	Boolean dirty = false;

	db = OpenMyDB();

	if (db == NULL)
		return (-1); /* no database */
	nRec = DmNumRecords(db);
	for (gameindex = 0; gameindex < nRec; gameindex++) {
		rec = DmQueryRecord(db, gameindex);
		if (rec == NULL)
			continue;
		pRec = MemHandleLock(rec);
		if (StrCompare((const Char *)pRec->cityname, name) == 0) {
			UInt16 reci;
			UInt32 len;
			MemHandle mhp;
			MemPtr pmhp;
			char newCity[CITYNAMELEN];

			reci = dmMaxRecordIndex;
			len = MemHandleSize(rec);
			mhp = DmNewRecord(db, &reci, len);
			pmhp = MemHandleLock(mhp);
			*newCity = '\0';
			StrNCat(newCity, pRec->cityname, CITYNAMELEN);
			StrNCat(newCity, " Copy", CITYNAMELEN);
			DmWrite(pmhp, 0, pRec, len);
			DmStrCopy(pmhp, offsetof(GameStruct, cityname),
			    newCity);
			MemHandleUnlock(mhp);
			DmReleaseRecord(db, reci, true);
			dirty = true;
		}
		MemHandleUnlock(rec);
		DmReleaseRecord(db, gameindex, false);
		if (dirty)
			break;
	}
	
	DmCloseDatabase(db);

	return (dirty);
}

/*
 * comparison function for CityNames list
 */
static Int16
comparator(void *p1, void *p2, Int32 other)
{
	return (StrNCaselessCompare(*(char **)p1, *(char **)p2, other));
}

/*
 * Get The City Names
 * Returns a NULL terminated array
 */
char **
CityNames(int *count)
{
	DmOpenRef db;
	UInt16 index = 0;
	MemHandle rec;
	unsigned char *pTemp;
	unsigned short int nRec;
	unsigned short int nsIndex = 0;
	char **cities;
	char *citystring;

	db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(), dmModeReadOnly);
	if (db == NULL) {
		return (NULL); /* no database */
	}
	nRec = DmNumRecords(db);

	if (nRec < 2) {
		DmCloseDatabase(db);
	return (NULL);
	}

	cities = (char **)MemPtrNew(nRec * sizeof (*cities));
	citystring = (char *)MemPtrNew(nRec * CITYNAMELEN);

	for (index = 1; index < nRec; index++) {
		rec = DmQueryRecord(db, index);
		if (rec) {
			cities[nsIndex] = citystring;
			citystring += CITYNAMELEN;
			pTemp = (unsigned char *)MemHandleLock(rec);
			StrNCopy(cities[nsIndex], (char *)pTemp +
			    offsetof(GameStruct, cityname), CITYNAMELEN);
			MemHandleUnlock(rec);
			nsIndex++;
		}
	}
	cities[nsIndex] = NULL;
	*count = nsIndex;
	if (nsIndex > 10)
	SysQSort(cities, nsIndex, sizeof (*cities),
		comparator, CITYNAMELEN);
	else if (nsIndex >= 2)
	SysInsertionSort(cities, nsIndex, sizeof (*cities),
		comparator, CITYNAMELEN);

	DmCloseDatabase(db);

	return (cities);
}

/*
 * deallocate the cities list
 */
void
FreeCityNames(char **names)
{
	char **at = names;
	char *gnat;

	if (names == NULL)
		return;

	gnat = *at;

	while (*at != NULL) {
		if (gnat > *at)
			gnat = *at;
		at++;
	}
	MemPtrFree(gnat);
	MemPtrFree(names);
}

