/*!
 * \file
 * \brief beaming support for sending and receiving cities
 *
 * This encapsulates all the code for sending and receiving cities.
 * The various elements are:
 * packing and unpacking structures
 * establishing communications
 * sending and receiving the data.
 */

#include <StringMgr.h>
#include <ErrorMgr.h>
#include <ExgMgr.h>
#include <palmutils.h>
#include <mem_compat.h>
#include <simcity.h>
#include <appconfig.h>
#include <beam.h>
#include <savegame_be.h>
#include <logging.h>

int
BeamSend(UInt8 *map_ptr)
{
	ExgSocketType exs;
	Err rv;
	UInt32 sent;
	UInt32 toSend;
	GameStruct *gs = (GameStruct *)map_ptr;

	char beamName[50]; /* city name length + .cty + "?: " */

	gMemSet((void *)&exs, (Int32)sizeof (exs), 0);
	StrPrintF(beamName, "?_send:%s.cty", gs->cityname);
	exs.target = GetCreatorID();
	exs.count = 1;
	exs.length = saveGameSize(gs);
	exs.description = "A Pocket City";
	exs.name = beamName;
	exs.length += StrLen(exs.description);

	WriteLog("pre-exgput\n");
	rv = ExgPut(&exs);
	if (rv != errNone) {
		return (-1);
	}
	WriteLog("post-exgput\n");
	toSend = saveGameSize(gs);
	WriteLog("pre all send\n");
	while (toSend != 0) {
		sent = ExgSend(&exs, map_ptr, toSend, &rv);
		WriteLog("send bits\n");
		toSend -= sent;
		map_ptr += sent;
		if (rv != errNone) {
			ExgDisconnect(&exs, rv);
			return (-1);
		}
	}
	WriteLog("Post all send (%d)\n", rv);
	ExgDisconnect(&exs, rv);
	WriteLog("Post Disconnect\n");
	return (0);
}

Err
BeamReceive(ExgSocketType *ptr)
{
	UInt8 *bof;
	Char *tl;
	UInt32 left;
	UInt32 read;
	GameStruct *gs;
	MemHandle mh;
	MemPtr mp;
	Err err = errNone;
	UInt16 reci;
	DmOpenRef dbP = OpenMyDB();

	if (dbP == NULL)
		return (-1);

	gs = gMalloc(sizeof (GameStruct));
	if (gs == NULL)
		return (-1);

	err = ExgAccept(ptr);
	/* read the gamestruct - gives size for other elements */
	left = sizeof (GameStruct);
	bof = (UInt8 *)gs;
	while (left != 0) {
		read = ExgReceive(ptr, bof, left, &err);
		if (err != errNone)
			goto recv_done;
		left -= read;
		bof += read;
	}
	left = saveGameSize(gs);
	gs = gRealloc(gs, left);
	if (gs == NULL)
		goto recv_done;
	bof = (UInt8 *)gs + sizeof (GameStruct);
	
	left -= sizeof (gs);
	while (left != 0) {
		read = ExgReceive(ptr, bof, left, &err);
		if (err != errNone) {
			/* XXX: notify error */
			goto recv_done;
		}
		left -= read;
		bof += read;
	}
	if (MemCmp(SAVEGAMEVERSION, gs->version, 4) != 0) {
		/* XXX: Invalid savegame - warning */
		goto recv_done;
	}
	/* At this point we've been successful at reading all the structure */
	left = StrLen(gs->cityname);
	if (left < 18) {
		tl = gs->cityname + left;
	} else {
		tl = gs->cityname + 18;
	}
	tl[1] = '\0';
	if (FindGameByName(gs->cityname) != LASTGAME) {
		*tl = '0';
		do {
			if (*tl == '9')
				*tl = 'A';
			*tl = *tl + 1;
		} while (FindGameByName(gs->cityname) != LASTGAME);
	}
	reci = dmMaxRecordIndex;
	mh = DmNewRecord(dbP, &reci, saveGameSize(gs));
	if (mh == NULL) {
		/* XXX: notify memory error */
		goto recv_done;
	}
	mp = MemHandleLock(mh);
	if (mp == NULL) {
		DmReleaseRecord(dbP, reci, false);
		DmRemoveRecord(dbP, reci);
	}
	MemMove(mp, gs, saveGameSize(gs));
	SetAutoSave(gs->cityname);
	MemHandleUnlock(mp);
	DmReleaseRecord(dbP, reci, true);
recv_done:
	err = ExgDisconnect(ptr, err);
	if (gs) gFree(gs);
	return (err);
}

void
BeamRegister(void)
{
	ExgRegisterData(GetCreatorID(), exgRegTypeID, SGSTRING);
}

/*
void
BeamUnregister(void)
{
	ExgRegisterData(GetCreatorID(), exgRegTypeID, NULL);
}
*/
