/*!
 * \file
 * \brief support for the sony handheld
 *
 * Contains the code that implements the sony high resolution routines
 */
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
#include <ui.h>

/*!
 * \brief high resolution checked.
 * Variable to indicate that high resolution is in effect & using sony
 * routines.
 */
static UInt16 hires;
/*! \brief I did load the high resolution library */
static UInt8 didl;

/*! \brief reference to the silk library */
static Int16 silk_ref = -1;
/*! \brief version of the silk library */
static Int8 silk_ver = -1;

/*! \bref hold callback function */
static void (*holdCB)(UInt32 held);

/*!
 * \brief the hold switch has been flipped
 * \param npp the notification
 * \return ErrNone if it's all OK
 */
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
		SysCurAppDatabase(&CardNo, &dbID);
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

Err
loadHiRes(void)
{
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error = 0;
	UInt16 refNum;

	if (highDensityFeatureSet() != 0)
		return (errNone);
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
				if (!error) didl = 1;
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
		if (didl)
			SysLibRemove(hires);
		didl = 0;
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

int
IsDrawWindowMostOfScreen()
{
	RectangleType rt;
	_WinGetDrawWindowBounds(&rt);
	return (((UInt32)rt.extent.x * rt.extent.y * 12) >=
	    ((UInt32)sWidth * sHeight * 10));
}

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

/*!
 * \brief Hook function for sony silk events
 * \param notifyParamsP unused
 * \return 0 - always works.
 */
static Err
SonyNotifyHook(SysNotifyParamType *notifyParamsP __attribute__((unused)))
{
	EventType ev;

	MemSet(&ev, sizeof(ev), 0);
	ev.eType = winDisplayChangedEvent;
	EvtAddUniqueEventToQueue(&ev, 0,  true);
	return (0);
}

Boolean
SonySilk(void)
{
	Err error = errNone;
	UInt32 version;
	UInt16		cardNo;
	LocalID		dbID;

	if (silk_ref != -1)
		return (silk_ref != 0);

	if (SysLibFind(sonySysLibNameSilk, &silk_ref)) {
		error = SysLibLoad('libr', sonySysFileCSilkLib, &silk_ref);
	}
	if (error == sysErrLibNotFound || silk_ref == -1) {
		silk_ref = 0;
		return (0);
	}
	WriteLog("found sony silk library\n");

	error = FtrGet(sonySysFtrCreator, sonySysFtrNumVskVersion, &version);
	if (error)
		silk_ver = 0;
	else
		silk_ver = 1;

	SonySetSilkResizable(true);

	SysCurAppDatabase(&cardNo, &dbID);
	SysNotifyRegister(cardNo, dbID, sysNotifyDisplayChangeEvent,
	    SonyNotifyHook, sysNotifyNormalPriority, NULL);


	return (silk_ref != 0);
}

void
SonyEndSilk(void)
{
	if (silk_ref != -1 && silk_ref != 0) {
		if (silk_ver == 0) {
			SilkLibResizeDispWin(silk_ref, silkResizeNormal);
			SilkLibDisableResize(silk_ref);
			SilkLibClose(silk_ref);
		} else {
			VskSetState(silk_ref, vskStateResize, vskResizeMin);
			VskSetState(silk_ref, vskStateEnable, 0);
			VskClose(silk_ref);
		}
	}
	silk_ref = 0;
}

void
SonySetSilkResizable(UInt8 state)
{
	if (silk_ver == -1)
		return;
	if (silk_ver == 0) {
		if (state) {
			SilkLibEnableResize(silk_ref);
		} else {
			SilkLibResizeDispWin(silk_ref, silkResizeNormal);
			SilkLibDisableResize(silk_ref);
		}
	} else {
		if (!state)
			VskSetState(silk_ref, vskStateEnable, state);
		else if (VskGetAPIVersion(silk_ref) >= 0x03)
			VskSetState(silk_ref, vskStateEnable, 
			    vskResizeVertically | vskResizeHorizontally);
		else
			VskSetState(silk_ref, vskStateEnable, 1);
	}
}

void
SonyCollapsePreRedraw(FormPtr form)
{
	if (silk_ver == 0)
		FrmEraseForm(form);
}

#endif /* SONY_CLIE */
