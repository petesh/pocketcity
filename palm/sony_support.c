#include <PalmTypes.h>
#include <FeatureMgr.h>
#include <ErrorBase.h>
#include <ErrorMgr.h>
#include <SystemMgr.h>

#ifndef	SONY_CLIE

#else /* SONY_CLIE */

#include <resCompat.h>
#include <sony_support.h>
#include <palmutils.h>

/*
 * Variable to indicate that high resolution is in effect & using sony
 * routines.
 */
static UInt16 hires = 0;

static void (*holdCB)(UInt32 held);

static Err
PrvHoldNotificationHandler(SysNotifyParamType *npp)
{
	UInt32 held;

	if (npp->broadcaster != sonySysNotifyBroadcasterCode)
		return (errNone);
	held = ((SonySysNotifyHoldStatusChangeDetailsP)
	    (npp->notifyDetailsP))->holdOn;
	ErrFatalDisplayIf(holdCB == NULL,
	    "Received hold call without valid handler");
	if (holdCB != NULL)
		holdCB(held);
	return (errNone);
}

void
hookHoldSwitch(void (*CallBack)(UInt32))
{
	Err err;
	UInt32 val;

	err = FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &val);
	if (!err && val) {
		UInt16 CardNo;
		LocalID dbID;
		DmSearchStateType state;
		DmGetNextDatabaseByTypeCreator(true, &state, 'appl',
		    GetCreatorID(), true, &CardNo, &dbID);
		SysNotifyRegister(CardNo, dbID,
		    sonySysNotifyHoldStatusChangeEvent,
		    PrvHoldNotificationHandler, sysNotifyNormalPriority, NULL);
		holdCB = CallBack;
	}
}

void
unhookHoldSwitch(void)
{
	Err err;
	UInt32 val;

	err = FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &val);
	if (!err && val) {
		UInt16 CardNo;
		LocalID dbID;
		DmSearchStateType state;
		DmGetNextDatabaseByTypeCreator(true, &state, 'appl',
		    GetCreatorID(), true, &CardNo, &dbID);
		SysNotifyUnregister(CardNo, dbID,
		    sonySysNotifyHoldStatusChangeEvent,
		    sysNotifyNormalPriority);
		holdCB = NULL;
	}
}

Err
goHires(void)
{
	Err result;
	UInt32 width;
	UInt32 height;

	width = sWidth;
	height = sHeight;

	result = _WinScreenMode(winScreenModeSet, &width, &height,
	    NULL, NULL);
	return (result);
}

/* Sony specific code to load high resolution library */
Err
loadHiRes(void)
{
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error = 0;
	UInt16 refNum;

	if ((error = FtrGet(sonySysFtrCreator,
		sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		/* Not CLIE: maybe not available */
	} else {
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR) {
			/* HR available */
			if ((error = SysLibFind(sonySysLibNameHR, &refNum))) {
				if (error == sysErrLibNotFound) {
				/* couldn't find lib */
				error = SysLibLoad('libr', sonySysFileCHRLib,
				    &refNum);
				}
			}
			if (!error) {
				hires = refNum;
				/* Now we can use HR lib */
				HROpen(hires);
			}
			ErrFatalDisplayIf(error, "could not load hires lib");
		}
	}
	if (!error)
		error = goHires();

	return (error);
}

Err
unloadHiRes(void)
{
	Err rv = 0;
	if (hires != 0) {
		rv = HRClose(hires);
		SysLibRemove(hires);
		hires = 0;
	}
	return (rv);
}

Boolean
sonyCanHires(void)
{
	if (hires == 0) {
		loadHiRes();
		if (hires != 0) {
			HRClose(hires);
			SysLibRemove(hires);
			hires = 0;
			return (true);
		} else
			return (false);
	} else
		return (true);
}

Boolean
sonyHires(void)
{
	return  (hires != 0);
}

Err
_WinScreenMode(WinScreenModeOperation op, UInt32 *width, UInt32 *height,
    UInt32 *depth, Boolean *enableColor)
{
	if (hires)
		return (HRWinScreenMode(hires, op, width, height, depth,
		    enableColor));
	else
		return (WinScreenMode(op, width, height, depth, enableColor));
}

void
_WinEraseRectangle(RectangleType *r, UInt16 cornerDiam)
{
	if (hires)
		HRWinEraseRectangle(hires, r, cornerDiam);
	else
		WinEraseRectangle(r, cornerDiam);
}

void
_WinDrawBitmap(BitmapPtr bmp, Coord x, Coord y)
{
	if (hires) {
		HRWinDrawBitmap(hires, bmp, x, y);
	} else {
		WinDrawBitmap(bmp, x, y);
	}
}

BitmapType *
_BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *clut,
    UInt16 *error)
{
	if (hires)
		return (HRBmpCreate(hires, width, height, depth, clut, error));
	else
		return (BmpCreate(width, height, depth, clut, error));
}

WinHandle
_WinCreateBitmapWindow(BitmapType *pBitmap, UInt16 *err)
{
	if (hires)
		return (HRWinCreateBitmapWindow(hires, pBitmap, err));
	else
		return (WinCreateBitmapWindow(pBitmap, err));
}

WinHandle
_WinCreateOffscreenWindow(Coord width, Coord height, WindowFormatType format,
    UInt16 *error)
{
	if (hires)
		return (HRWinCreateOffscreenWindow(hires, width, height,
		    format, error));
	else
		return (WinCreateOffscreenWindow(width, height, format, error));
}

void
_WinCopyRectangle(WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect,
    Coord destX, Coord destY, WinDrawOperation mode)
{
	if (hires)
		HRWinCopyRectangle(hires, srcWin, dstWin, srcRect,
		    destX, destY, mode);
	else
		WinCopyRectangle(srcWin, dstWin, srcRect, destX, destY, mode);
}

void
_WinDrawChars(const Char *chars, Int16 len, Coord x, Coord y)
{
	if (hires)
		HRWinDrawChars(hires, chars, len, x, y);
	else
		WinDrawChars(chars, len, x, y);
}

void
_WinDrawRectangleFrame(FrameType frame, RectangleType *rP)
{
	if (hires)
		HRWinDrawRectangleFrame(hires, frame, rP);
	else
		WinDrawRectangleFrame(frame, rP);
}

void
_FntSetFont(FontID font)
{
	if (hires)
		HRFntSetFont(hires, font);
	else
		FntSetFont(font);
}

void
_WinDrawPixel(Coord x, Coord y)
{
	if (hires)
		HRWinDrawPixel(hires, x, y);
	else
		WinDrawPixel(x, y);
}

void
_WinGetDrawWindowBounds(RectangleType *rP)
{
	if (hires)
		HRWinGetWindowBounds(hires, rP);
	else
		WinGetDrawWindowBounds(rP);
}

/*
 * check if the draw-window occupies most of the screen...
 * This is a jog assist support routine.
 */
int
IsDrawWindowMostOfScreen()
{
	RectangleType rt;
	_WinGetDrawWindowBounds(&rt);
	return (((UInt32)rt.extent.x * rt.extent.y * 12) >=
	    ((UInt32)sWidth * sHeight * 10));
}

/*
 * Check if this device is a Sony. For Jog Navigation Support
 */
Boolean
IsSony(void)
{
	static int tested = -1;
	UInt32 val;

	if (tested != -1)
		return (tested == 1);
	tested = 0;
	if (!FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &val)) {
		if (val == sonyHwrOEMCompanyID_Sony)
			tested = 1;
	}
	return (tested == 1);
}


#endif /* SONY_CLIE */
