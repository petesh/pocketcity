/*!
 * \file
 * \brief Contains the implementation of various palm functions
 */
#define ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS
#include <PalmTypes.h>
#include <FeatureMgr.h>
#include <ErrorBase.h>
#include <ErrorMgr.h>
#include <SystemMgr.h>
#include <SysUtils.h>
#include <StringMgr.h>
#include <Form.h>

#if defined(PALM_FIVE)
#include <PalmNavigator.h>
#endif

#include <resCompat.h>
#include <simcity_resconsts.h>
#include <simcity.h>
#include <palmutils.h>
#include <logging.h>
#include <mem_compat.h>

/* included the TRG magic numbers :( */
#define	TRGSysFtrID	'TRG '
#define	TRGVgaFtrNum	2

/* included Palm Zire (old) magic numbers */
#define PalmOEMCompanyID	'Palm'
#define ZireOriginalDeviceID	'Cubs'

void
RearrangeObjectOnly(FormPtr form, UInt16 oID, Int16 offsetX, Int16 offsetY,
    Int16 resizeX, Int16 resizeY)
{
	RectangleType objrect;

	WriteLog("Move/resize: %d -> delta:%d,%d grow:%d,%d\n", (int)oID,
	    (int)offsetX, (int)offsetY, (int)resizeX, (int)resizeY);
	FrmGetObjectBounds(form, FrmGetObjectIndex(form, oID), &objrect);

	objrect.topLeft.x += offsetX;
	objrect.topLeft.y += offsetY;
	objrect.extent.x += resizeX;
	objrect.extent.y += resizeY;

	FrmSetObjectBounds(form, FrmGetObjectIndex(form, oID), &objrect);
}

void
RearrangeBitmap(FormPtr form, UInt16 oID, Int16 offsetX, Int16 offsetY)
{
	Coord x, y;

	FrmGetObjectPosition(form, FrmGetObjectIndex(form, oID), &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition(form, FrmGetObjectIndex(form, oID), x, y);
}

Boolean
isHandEra(void)
{
	UInt32 version;
	static UInt16 rv = 3;
	
	if (rv != 3)
		return ((Boolean)(rv == 1));
	rv = 0;
	if (FtrGet(TRGSysFtrID, TRGVgaFtrNum, &version) == 0)
		if (sysGetROMVerMajor(version) >= 1)
			rv = 1;
	return ((Boolean)(rv == 1));
}

Boolean
isZireOld(void)
{
	static UInt16 rv = 3;
	UInt32 vcl;

	if (rv != 3)
		return ((Boolean)(rv == 1));
	rv = 0;
	if ((FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &vcl) == 0) &&
	    (vcl == PalmOEMCompanyID) &&
	    (FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &vcl) == 0) &&
	    (vcl == ZireOriginalDeviceID))
		rv = 1;
	return ((Boolean)(rv == 1));
}

#if defined(PALM_FIVE)
Boolean
hasFiveWayNav(void)
{
	static UInt16 rv = 3;
	UInt32 vcl;
	
	if (rv != 3)
		return ((Boolean)(rv == 1));
	rv = 0;
	if ((FtrGet(navFtrCreator, navFtrVersion, &vcl) == 0))
		rv = 1;
	return ((Boolean)(rv == 1));
}
#endif
 
UInt32
getDepth(void)
{
	static UInt32 avd = 0;
	
	if (avd != 0) {
		WriteLog("Depth: saved == %ld\n", (long)avd);
		return (avd);
	}
	if (Is35ROM()) {
		(void) _WinScreenMode(winScreenModeGet, NULL, NULL, &avd, NULL);
	} else {
		avd = 1;
	}
	WriteLog("Depth: == %ld\n", (long)avd);
	return (avd);
}

/*!
 * \brief get the highest numbered bit that is set in the 32 bit value passed
 * \param x the value to find the highest bit set from
 * \return the highest bit set.
 */
static UInt32
hibit(UInt32 x)
{
	int r = 0;
	
	if (x & 0xffff0000)  { x >>= 16; r += 16; }
	if (x & 0x0000ff00)  { x >>=  8; r +=  8; }
	if (x & 0x000000f0)  { x >>=  4; r +=  4; }
	if (x & 0x0000000c)  { x >>=  2; r +=  2; }
	if (x & 0x00000002)  { r +=  1; }
	return (r);
}

Err
changeDepthRes(UInt32 ndepth, Boolean tryHigh)
{
	UInt32 depth = ndepth;
	Boolean enablecol = 1;
	UInt32 dep = 0;
	UInt32 cdep;
	UInt32 width;
	UInt32 height;
	Err result;

	if (tryHigh) {
		(void) loadHiRes();
		setScreenRes();
	} else {
		SETWIDTH(BASEWIDTH);
		SETWIDTH(BASEHEIGHT);
	}
	
	(void) _WinScreenMode(winScreenModeGetSupportsColor, NULL, NULL, NULL,
	    &enablecol);
	(void) _WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL,
	    &dep, NULL);
	/* in theory there's 16color _as well as_ 16grays */
	cdep = 1 + hibit(dep);
	if ((cdep >= ndepth) && (ndepth > 1)) {
		do {
			cdep = hibit(dep);
			if ((cdep+1) & ndepth) {
				depth = cdep + 1;
				break;
			}
			dep = dep & ~(1 << cdep);
		} while (dep);

		/* Could not match... */
		if (!dep) {
			depth = 1;
			enablecol = 0;
		}
	} else {
		enablecol = 0;
		depth = 1;
	}

	width = sWidth;
	height = sHeight;

	if (isHandEra())
		result = _WinScreenMode(winScreenModeSet, NULL, NULL,
		    &depth, &enablecol);
	else
		result = _WinScreenMode(winScreenModeSet, &width, &height,
		    &depth, &enablecol);

#if defined(LOGGING)
	if (result != errNone)
		WriteLog("Could not set resolution to (%d,%d)\n", (int)width,
		    (int)height);
	else
		WriteLog("Resolution set to (%d,%d)\n", (int)width,
		    (int)height);
#endif
	return (result);
}

Err
restoreDepthRes(void)
{
	Err rv;

	if (0 != (rv = WinScreenMode(winScreenModeSetToDefaults, NULL, NULL,
		    NULL, NULL)))
		return (rv);
	if (0 != (rv = unloadHiRes()))
		return (rv);
	return (0);
}

Boolean
canColor(UInt16 nbits)
{
	UInt32 de;
	UInt32 wi;
	UInt32 he;
	Boolean ec;

	WinScreenMode(winScreenModeGetSupportedDepths,
	    &wi, &he, &de, &ec);
	if (de & (1<<(nbits-1)))
		return (true);
	return (false);
}

/*
UInt32
GetCreatorID(void)
{
	static UInt32 nCreatorID = 0;

	if (nCreatorID == 0) {
		UInt16 nCard;
		LocalID LocalDB;
		Err err;
		err = SysCurAppDatabase(&nCard, &LocalDB);
		ErrFatalDisplayIf(err, "Could not get current app database.");
		err = DmDatabaseInfo(nCard, LocalDB, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, &nCreatorID);
		ErrFatalDisplayIf(err,
		    "Could not get app database info, looking for creator ID");
	}

	return (nCreatorID);
}
*/
UInt32
GetCreatorID(void)
{
	return (PROGTOKEN);
}

#if defined(LOGGING)

void
DangerWillRobinson(char *information, char *file, int line)
{
	char buffer[80];
	StrPrintF(buffer, "%s(%d)", file, line);
	FrmCustomAlert(alertID_programmingNiggle, information, buffer, NULL);
}

#endif

Char **
FillStringList(UInt16 resID, UInt16 *length)
{
	UInt16 max = 0;
	UInt16 atitem = 0;
	Char *foo = NULL;
	Char **rv = NULL;
	Char *lom;
	Char item[201];
	UInt32 maxlen = 0;

	do {
		foo = SysStringByIndex(resID, max, item, 200);
		if (*foo != '\0') {
			maxlen += 1 + StrLen(item);
			max++;
		} else break;
	} while (foo);
	rv = (Char **)MemPtrNew(sizeof (*rv) * (max + 1));
	MemSet(rv, sizeof (*rv) * (max + 1), 0);
	lom = (Char *)MemPtrNew(sizeof (*lom) * maxlen);
	MemSet(lom, sizeof (*lom) * maxlen, 0);
	rv[0] = lom;
	while (atitem < max) {
		UInt16 sli;
		SysStringByIndex(resID, atitem, item, 200);
		sli = StrLen(item);
		StrNCopy(rv[atitem], item, (Int16)sli);
		rv[atitem + 1] = rv[atitem] + (sli + 1);
		atitem++;
	}
	*length = max;
	return (rv);
}

void
FreeStringList(Char **list)
{
	MemPtrFree(list[0]);
	MemPtrFree(list);
}

void *
GetObjectPtr(FormType *form, UInt16 index)
{
	return (FrmGetObjectPtr(form,
	    FrmGetObjectIndex(form, index)));
}

void
compatBmpGetDimensions(BitmapPtr pBmp, Coord *pWidth, Coord *pHeight,
    UInt16 *pRowBytes)
{
	if (!Is40ROM()) {
		if (pWidth != NULL) *pWidth = (Coord)pBmp->width;
		if (pHeight != NULL) *pHeight = (Coord)pBmp->height;
		if (pRowBytes != NULL) *pRowBytes = pBmp->rowBytes;
	} else
		BmpGetDimensions(pBmp, pWidth, pHeight, pRowBytes);
}

