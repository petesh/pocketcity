
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

#define LASTGAME        ((UInt16)~0)
#define	MAXSAVEGAMECOUNT    50
#define DEAD            "PCNO"

static void NewGame(void);

/*
 * Open up the savegame database.
 * If it does not exist, then it tries to create it.
 * It will return a reference to the database if successful, NULL otherwise.
 */
static DmOpenRef
OpenMyDB(void)
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
static int
FindGameByName(char *name)
{
    DmOpenRef db = NULL;
    MemHandle rec;
    GameStruct *pRec;
    unsigned short int nRec;
    int gameindex = -1;

    db = OpenMyDB();
    if (!db) {
	FrmCustomAlert(alertID_majorbad,
	    "Can't Open/Create the savegame database", NULL, NULL);
	return (gameindex);
    }
    nRec = DmNumRecords(db);
    for (gameindex = 1; gameindex < nRec; gameindex++) {
	rec = DmQueryRecord(db, gameindex);
	if (rec) {
	    pRec = (GameStruct *)MemHandleLock(rec);
	    if (StrCompare(pRec->cityname, name) == 0) {
		MemHandleUnlock(rec);
		break;
	    }
	    MemHandleUnlock(rec);
	}
    }
    if (db) DmCloseDatabase(db);

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
static void
ResetViewable(void)
{
    vgame.tileSize = 16;
    vgame.visible_x = sWidth / vgame.tileSize;
    vgame.visible_y = (sHeight / vgame.tileSize) - 2;
}

/*
 * Read the city record.
 */
static int
ReadCityRecord(MemHandle rec)
{
    char *ptemp;
    int rv = -1;

    ptemp = (unsigned char *)MemHandleLock(rec);
    if (ptemp == NULL)
	return (-1);
    if (StrNCompare(DEAD, (char *)ptemp, 4) == 0)
	goto leave_me;
    if (StrNCompare(SAVEGAMEVERSION, (char*)ptemp, 4) == 0) {
	LockWorld();
	MemMove((void*)&game, ptemp, sizeof(GameStruct));
	MemMove(worldPtr, ptemp + sizeof(GameStruct), GetMapMul());
	UnlockWorld();
	PostLoadGame();
	ResetViewable();
	rv = 0;
    } else {
	FrmAlert(alertID_invalidSaveVersion);
    }

leave_me:
    MemHandleUnlock(rec);
    return (rv);
}

/*
 * Write the city record
 */
static void
WriteCityRecord(MemHandle rec)
{
    void * pRec;

    pRec = MemHandleLock(rec);
    LockWorld();
    /* write the header and some globals */
    DmWrite(pRec, 0, (void*)&game, sizeof (GameStruct));
    DmWrite(pRec, sizeof (GameStruct), (void*)(unsigned char*)worldPtr,
	GetMapMul());
    UnlockWorld();
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
    if (index <= DmNumRecords(db)) {
        rec = DmResizeRecord(db, index, GetMapMul() + sizeof (GameStruct));
    } else {
        index = DmNumRecords(db) + 1;
	rec = DmNewRecord(db, &index, GetMapMul() + sizeof (GameStruct));
    }
    if (rec) {
	WriteCityRecord(rec);
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
    int index = FindGameByName(name);

    SaveGameByIndex(index);
}

/*
 * Set up a new game.
 * Hands off everything to other routines
 */
static void
NewGame(void)
{
    SetupNewGame();
    ResetViewable();
    ResumeGame();
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
    if (db) {
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
                    DmReleaseRecord(db, index, true);
                }
            }
            index = dmMaxRecordIndex;
            rec = DmNewRecord(db, &index, GetMapMul() + sizeof (game));
            if (rec) {
		NewGame();
		WriteCityRecord(rec);
                DmReleaseRecord(db, index, true);
            }
        }
        DmCloseDatabase(db);
    }
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
    if (rec)
	loaded = ReadCityRecord(rec);
    DmCloseDatabase(db);

    return (loaded);
}


/*
 * Load a game by name
 */
int
LoadGameByName(char *name)
{
    int gameindex = FindGameByName(name);

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

    return LoadGameByName(cityname);
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

    if (!db)
        return;

    if (DmNumRecords(db) < 1)
        goto close_me;

    mh = DmQueryRecord(db, 0);
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
    if (!db) {
        return;
    }

    if (DmNumRecords(db) < index) {
        return; /* index doesn't exist */
    }

    if (index > 0)
    	DmRemoveRecord(db, index);

    DmCloseDatabase(db);
}

/*
 * Delete a city from the DB by name
 */
void
DeleteGameByName(char *name)
{
    int gameindex = FindGameByName(name);

    if (gameindex != LASTGAME)
	DeleteGameByIndex(gameindex);
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
    unsigned char * pTemp;
    unsigned short int nRec, nsIndex=0;
    char **cities;
    char *citystring;

    db = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(), dmModeReadOnly);
    if (!db) {
        return (NULL); /* no database */
    }
    nRec = DmNumRecords(db);

    if (nRec < 2)
	return (NULL);

    cities = MemPtrNew(nRec * sizeof (*cities));
    citystring = MemPtrNew(nRec * CITYNAMELEN);

    for (index = 1; index < nRec; index++) {
        rec = DmQueryRecord(db, index);
        if (rec) {
            cities[nsIndex] = citystring;
	    citystring += CITYNAMELEN;
            pTemp = (unsigned char*)MemHandleLock(rec);
            StrNCopy(cities[nsIndex], (char*)pTemp +
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
    char *gnat = *at;

    if (names == NULL)
        return;

    while(*at != NULL) {
	if (gnat > *at)
	    gnat = *at;
	at++;
    }
    MemPtrFree(gnat);
    MemPtrFree(names);
}
