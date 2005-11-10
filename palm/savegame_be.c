/*!
 * \file
 * \brief This file handles savegame back end
 */
#include <PalmOS.h>
#include <StringMgr.h>
#include <Form.h>

#include <stddef.h>
#include <zakdef.h>
#include <simcity.h>
#include <globals.h>
#include <handler.h>
#include <locking.h>
#include <logging.h>
#include <palmutils.h>
#include <simcity_resconsts.h>
#include <resCompat.h>
#include <savegame_be.h>
#include <mem_compat.h>
#include <pack.h>
#include <beam.h>
#include <ui.h>

#define	MAXSAVEGAMECOUNT	50
#define	DEAD	"PCNO"
#define	MAGIC_GUARD		0xFFFFFFFF

static int ReadCityRecord(MemHandle rec, GameStruct *gs,
    MemPtr *wp, MemPtr *fp) SAVE_SECTION;
static void WriteCityRecord(MemHandle rec, GameStruct *gs,
    MemPtr wp, MemPtr fp) SAVE_SECTION;
static int SaveGameByIndex(UInt16 index) SAVE_SECTION;
static int LoadGameByIndex(UInt16 index) SAVE_SECTION;
static void getAutoSaveName(char *name) SAVE_SECTION;
static void DeleteGameByIndex(UInt16 index) SAVE_SECTION;
static Int16 comparator(void *p1, void *p2, Int32 other) SAVE_SECTION;

static DmOpenRef sgref = 0;

DmOpenRef
OpenMyDB(void)
{
	Err err = 0;

	if (sgref != NULL)
		return (sgref);
	sgref = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(),
	    dmModeReadWrite);
	if (!sgref) {
		err = DmCreateDatabase(0, SGNAME, GetCreatorID(), SGTYP, false);
		if (err)
			return (NULL); /* couldn't create */
		sgref = DmOpenDatabaseByTypeCreator(SGTYP, GetCreatorID(),
		    dmModeReadWrite);
	}
	if (!sgref) {
		/* Could not open savegame database */
	}
	if (DmNumRecords(sgref) == 0) {
		/*
		 * create a record in slot 0 if it's not there.
		 * This is used for autosaves. It takes the name
		 * of the city
		 */
		UInt16 index = dmMaxRecordIndex;
		Char name[CITYNAMELEN];
		MemHandle rec = DmNewRecord(sgref, &index, CITYNAMELEN);
		MemSet(name, CITYNAMELEN, 0);
		if (rec) {
			MemPtr mp = MemHandleLock(rec);
			DmWrite(mp, 0, name, CITYNAMELEN);
			MemHandleUnlock(rec);
			DmReleaseRecord(sgref, index, true);
		}
	}
	return (sgref);
}

void
CloseMyDB(void)
{
	if (sgref) {
		DmCloseDatabase(sgref);
		sgref = 0;
	}
}

UInt16
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

	if (gameindex >= nRec)
		return (LASTGAME);
	else
		return (gameindex);
}

/*!
 * We set the tile size (16 pels).
 * We reserve 2 tiles (one at the top, one at the bottom) for use
 * with status information.
 */
void
ResetViewable(void)
{
	WriteLog("Reset viewable(%d, %d)\n", (UInt16)(sWidth / gameTileSize()),
	    (UInt16)((sHeight / gameTileSize()) - 2));

	setVisibleX((UInt16)(sWidth / gameTileSize()));
	setVisibleY((UInt16)((sHeight / gameTileSize()) - 2));

	/* Make sure we're not not of bounds on the screen */
	if (getMapXPos() + getVisibleX() > getMapWidth())
	    setMapXPos(getMapWidth() - getVisibleX());
	if (getMapYPos() + getVisibleY() > getMapHeight())
	    setMapYPos(getMapHeight() - getVisibleY());
}

UInt32
saveGameSize(GameStruct *gs)
{

	UInt32 size = (8 + sizeof (GameStruct) +
	    gs->mapx * gs->mapy +
	    ((gs->mapx * gs->mapy + ((8 / 2) - 1)) / (8 / 2)));
	return (size);
}

/*!
 * \brief Read the city record.
 * \param rec the record to read
 * \param gs the game structure
 * \param wp the pointer to the world (resized in call)
 * \param fp the world field pointer to be replaced on write
 * \return 0 if it all went well, -1 otherwise.
 */
static int
ReadCityRecord(MemHandle rec, GameStruct *gs, MemPtr *wp, MemPtr *fp)
{
	char *ptemp;
	int rv = -1;
	UInt32 foo;

	ptemp = (char *)MemHandleLock(rec);
	if (ptemp == NULL)
		return (-1);
	if (MemCmp(DEAD, (char *)ptemp, 4) == 0)
		goto leave_me;
	if (MemCmp(SAVEGAMEVERSION, (char *)ptemp, 4) == 0) {
		UInt32 size;
		MemMove((void *)gs, ptemp, sizeof (GameStruct));
		size = gs->mapx * gs->mapy;
		ptemp += sizeof (GameStruct);
		MemMove(&foo, ptemp, 4);
		if (foo != MAGIC_GUARD) {
			UISystemErrorNotify(seInvalidSaveGame);
			goto leave_me;
		}
		ptemp += 4;
		*wp = gRealloc(*wp, size);
		if (*wp == NULL) {
			UISystemErrorNotify(seOutOfMemory);
			goto leave_me;
		}
		MemMove(*wp, ptemp, (Int32)size);
		*fp = gRealloc(*fp, size);
		if (*fp == NULL) {
			UISystemErrorNotify(seOutOfMemory);
			goto leave_me;
		}
		ptemp += size;
		MemMove(&foo, ptemp, 4);
		if (foo != MAGIC_GUARD) {
			UISystemErrorNotify(seInvalidSaveGame);
			goto leave_me;
		}
		ptemp += 4;
		UnpackBits(ptemp, *fp, 2, (Int32)size);
		MemMove(*fp, ptemp, (Int32)size);
		rv = 0;
	} else {
		UISystemErrorNotify(seInvalidSaveGame);
	}

leave_me:
	MemHandleUnlock(rec);
	return (rv);
}

/*!
 * \brief Write the city record
 * \param rec the record to write to
 * \param gs the game structure
 * \param wp the world pointer
 * \param fp the flags pointer
 */
static void
WriteCityRecord(MemHandle rec, GameStruct *gs, MemPtr wp, MemPtr fp)
{
	void *pRec;
	void *pRec2;
	UInt32 size;
	UInt32 foo = MAGIC_GUARD;
	UInt32 offset = 0;

	pRec = MemHandleLock(rec);
	/* write the header and some globals */
	DmWrite(pRec, offset, gs, sizeof (GameStruct));
	offset += sizeof (GameStruct);
	DmWrite(pRec, offset, (void *)&foo, 4);
	offset += 4;
	DmWrite(pRec, offset, (void *)wp,
	    gs->mapx * gs->mapy);
	offset += gs->mapx * gs->mapy;
	DmWrite(pRec, offset, (void *)&foo, 4);
	offset += 4;
	size = (gs->mapx * gs->mapy + (((sizeof (selem_t) * 8) / 2) - 1)) /
	    ((sizeof (selem_t) * 8) / 2);
	pRec2 = gMalloc(size);
	PackBits(fp, pRec2, 2, gs->mapx * gs->mapy);
	DmWrite(pRec, offset, (void *)pRec2, size);
	offset += size;
	gFree(pRec2);
	MemHandleUnlock(rec);
}

/*!
 * \brief Save a game into the database of savegames.
 * \param index the index to save game into
 * \return 0 if saved OK, -1 otherwise
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
		rec = DmResizeRecord(db, index, saveGameSize(&game));
		rec = DmGetRecord(db, index);
	} else {
		index = DmNumRecords(db) + 1;
		rec = DmNewRecord(db, &index, saveGameSize(&game));
	}
	if (rec) {
		LockZone(lz_world);
		LockZone(lz_flags);
		WriteCityRecord(rec, &game, worldPtr, flagPtr);
		UnlockZone(lz_flags);
		UnlockZone(lz_world);
		DmReleaseRecord(db, index, true);
	}

	/* tag DB so it gets saved */
	DmSetDatabaseInfo(0, DmFindDatabase(0, SGNAME), NULL, &attr, NULL,
	    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	return ((int)index);
}

int
GameExists(char *name)
{
	return (FindGameByName(name) != LASTGAME);
}

void
SaveGameByName(char *name)
{
	UInt16 index;
	index = FindGameByName(name);

	SaveGameByIndex(index);
}

/*!
 * \brief Load a game by index from the savegames.
 * \param index index of game to load
 * \return 0 if loaded, -1 otherwise
 */
static int
LoadGameByIndex(UInt16 index)
{
	DmOpenRef db;
	MemHandle rec;
	short int loaded = -1;

	db = OpenMyDB();

	if (!db)
		return (-1); /* no database */

	if (index == LASTGAME)
		index = DmNumRecords(db) - 1;
	rec = DmQueryRecord(db, index);
	if (rec) {
		LockZone(lz_world);
		LockZone(lz_flags);
		loaded = ReadCityRecord(rec, &game, (MemPtr *)&worldPtr,
		    (MemPtr *)&flagPtr);
		UnlockZone(lz_flags);
		UnlockZone(lz_world);
	}
	if (loaded != -1) {
		PostLoadGame();
		game.units[NUM_OF_UNITS - 1].type = 0xbb;
		game.objects[0].x = 0x2222;
		game.objects[NUM_OF_OBJECTS - 1].dir = 0x3333;
		ResetViewable();
	}

	return (loaded);
}


int
LoadGameByName(char *name)
{
	UInt16 gameindex = FindGameByName(name);

	if (gameindex != LASTGAME)
		return (LoadGameByIndex(gameindex));
	else
		return (-1);
}

Int32
BeamCityByName(Char *cityName)
{
	UInt16 gameindex = FindGameByName(cityName);
	MemHandle rec = NULL;
	MemPtr rp = NULL;
	DmOpenRef db = 0;
	Int32 rv = -1;

	if (gameindex == LASTGAME)
		return (rv);

	db = OpenMyDB();
	if (!db)
		goto exit_me;
	rec = DmQueryRecord(db, gameindex);
	if (rec == NULL)
		goto exit_me;
	rp = MemHandleLock(rec);
	if (rp == NULL)
		goto exit_me;
	rv = BeamSend((UInt8 *)rp);
exit_me:
	if (rp)
		MemHandleUnlock(rec);
	return (rv);
}

/*!
 * \brief Get the name of the autosave city
 * \param name the name of the city (fills out string, must be big enough)
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
}

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

void
SetAutoSave(char *name)
{
	DmOpenRef db = OpenMyDB();
	MemHandle mh;
	MemPtr mp;

	if (db == NULL)
		return;

	if (DmNumRecords(db) < 1)
		return;

	mh = DmGetRecord(db, 0);
	if (mh == NULL) {
		return;
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
}

void
DeleteAutoSave(void)
{
	char buffer[CITYNAMELEN];

	*buffer = '\0';
	SetAutoSave(buffer);
}

/*!
 * \brief Delete the savegame from the list of savegames.
 * \param index the index into the savegames
 *
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

	if (DmNumRecords(db) < index)
		return;

	if (index > 0)
		DmRemoveRecord(db, index);
}

void
DeleteGameByName(char *name)
{
	UInt16 gameindex = FindGameByName(name);

	if (gameindex != LASTGAME)
		DeleteGameByIndex(gameindex);
}

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
			StrNCat(newCity, (char *)pRec->cityname, CITYNAMELEN);
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
	return (dirty);
}

/*!
 * \brief comparison function for CityNames list
 * \param p1 left side
 * \param p2 right side
 * \param other a useless variable
 * \return -1 (before), 0 (equals), 1 (greater)
 */
static Int16
comparator(void *p1, void *p2, Int32 other)
{
	return (StrNCaselessCompare(*(char **)p1, *(char **)p2, other));
}

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

	db = OpenMyDB();
	if (db == NULL) {
		return (NULL); /* no database */
	}
	nRec = DmNumRecords(db);

	if (nRec < 2)
		return (NULL);

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
	*count = (int)nsIndex;
	if (nsIndex > 10)
		SysQSort(cities, nsIndex, sizeof (*cities),
		    comparator, CITYNAMELEN);
	else if (nsIndex >= 2)
		SysInsertionSort(cities, nsIndex, sizeof (*cities),
		    comparator, CITYNAMELEN);
	return (cities);
}

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
