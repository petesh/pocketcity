#include <PalmTypes.h>
#include <FeatureMgr.h>
#include <ErrorBase.h>
#include <ErrorMgr.h>
#include <SystemMgr.h>

#define _HIRESSOURCE_
#include <resCompat.h>
#include <palmutils.h>

/* The optimizer will remove any if (hires) { } clauses */
#ifndef SONY_CLIE

#define hires   0

#else /* SONY_CLIE */
/* Change Resolution */
UInt32 sWidth;
UInt32 sHeight;

static UInt16 hires = 0;
static void (*holdCB)(UInt32 held);

static Err
PrvHoldNotificationHandler(SysNotifyParamType *npp)
{
    UInt32 held;
    if (npp->broadcaster != sonySysNotifyBroadcasterCode) return (errNone);
    held = ((SonySysNotifyHoldStatusChangeDetailsP)
      (npp->notifyDetailsP))->holdOn;
    ErrFatalDisplayIf(holdCB == NULL, "Received hold call w/o valid handler");
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
            if ((error = SysLibFind(sonySysLibNameHR, &refNum))){
                if (error == sysErrLibNotFound) {
                /* couldn't find lib */
                error = SysLibLoad( 'libr', sonySysFileCHRLib, &refNum );
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
    return (error);
}

Boolean
canHires(void)
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
isHires(void)
{
    return (hires != 0);
}

void
scaleEvent(EventPtr event)
{
    if (hires) {
        event->screenX *= sWidth / 160;
        event->screenY *= sHeight / 160;
    }
}

UInt32
xScale()
{
    if (hires)
        return (sWidth / 160);
    else
        return (1);
}

UInt32
yScale()
{
    if (hires)
        return (sHeight / 160);
    else
        return (1);
}

Err
unloadHiRes(void)
{
    Err rv = 0;
    if (hires != 0) {
        rv = HRClose(hires);
        hires = 0;
    }
    return (rv);
}

Err
_WinScreenMode(WinScreenModeOperation op, UInt32 *width,
  UInt32 *height, UInt32 *depth, Boolean *enableColor)
{
    if (hires)
        return HRWinScreenMode(hires, op, width, height, depth, enableColor);
    else
        return WinScreenMode(op, width, height, depth, enableColor);
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

WinHandle _WinCreateOffscreenWindow (Coord width, Coord height,
  WindowFormatType format, UInt16 *error )
{
    if (hires)
        return HRWinCreateOffscreenWindow(hires, width, height, format, error);
    else
        return WinCreateOffscreenWindow(width, height, format, error);
}

void _WinCopyRectangle(WinHandle srcWin, WinHandle dstWin,
  RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode)
{
    if (hires)
        HRWinCopyRectangle(hires, srcWin, dstWin, srcRect, destX, destY, mode);
    else
        WinCopyRectangle(srcWin, dstWin, srcRect, destX, destY, mode);
}

void _WinDrawChars(const Char *chars, Int16 len, Coord x, Coord y)
{
    if (hires)
        HRWinDrawChars(hires, chars, len, x, y);
    else
        WinDrawChars(chars, len, x, y);
}

void _WinDrawRectangleFrame(FrameType frame, RectangleType *rP)
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

/* check if the draw-window occupies most of the screen */
int
IsDrawWindowMostOfScreen()
{
    RectangleType rt;
    _WinGetDrawWindowBounds(&rt);
    return (((UInt32)rt.extent.x * rt.extent.y * 12) >= ((UInt32)sWidth * sHeight * 10));
}

#endif /* SONY_CLIE */
