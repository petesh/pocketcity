#include <PalmTypes.h>
#include <FeatureMgr.h>
#include <ErrorBase.h>
#include <ErrorMgr.h>
#include <SystemMgr.h>

#define _HIRESSOURCE_
#include "resCompat.h"

static UInt32 oWidth = 0;
static UInt32 oHeight = 0;
static UInt32 oDepth = 0;
static Boolean oUseColor = 0;

// The optimizer will remove any if (hires) { } clauses
#ifndef SONY_CLIE

#define hires   0

#else /* SONY_CLIE */
// Change Resolution
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
        // Not CLIE: maybe not available
    } else {
        if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR) {
            // HR available
            if ((error = SysLibFind(sonySysLibNameHR, &refNum))){
                if (error == sysErrLibNotFound) {
                // couldn't find lib
                error = SysLibLoad( 'libr', sonySysFileCHRLib, &refNum );
                }
            }
            if (!error) {
                hires = refNum;
                // Now we can use HR lib
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
        if (rv == NULL) {
            rv = SysLibRemove(hires);
            ErrFatalDisplayIf(rv != 0, "Could not unload hires lib");
        } else ErrFatalDisplay("Could not HRClose");
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

void _FntSetFont(FontID font)
{
    if (hires)
        HRFntSetFont(hires, font);
    else
        FntSetFont(font);
}

void _WinDrawPixel(Coord x, Coord y)
{
    if (hires)
        HRWinDrawPixel(hires, x, y);
    else
        WinDrawPixel(x, y);
}

#endif /* SONY_CLIE */

UInt32
getDepth(void)
{
    static UInt32 avd = 0;
    if (avd != 0)
        return (avd);
    (void) _WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL,
                          &avd, NULL);
    avd = 1 << (avd-1);
    return (avd);
}

UInt32
hibit(UInt32 x)
{
    int r = 0;
    if ( x & 0xffff0000 )  { x >>= 16;  r += 16; }
    if ( x & 0x0000ff00 )  { x >>=  8;  r +=  8; }
    if ( x & 0x000000f0 )  { x >>=  4;  r +=  4; }
    if ( x & 0x0000000c )  { x >>=  2;  r +=  2; }
    if ( x & 0x00000002 )  {            r +=  1; }
    return r;
}

Err
changeDepthRes(UInt32 ndepth)
{
    UInt32 depth = ndepth;
    Boolean enablecol = 1;
    UInt32 dep = 0;
    UInt32 cdep;
    UInt32 width;
    UInt32 height;

    SETWIDTH(160);
    SETHEIGHT(160);

    (void)loadHiRes();
    if (hires) {
        SETWIDTH(320);
        SETHEIGHT(320);
    }
    (void) _WinScreenMode(winScreenModeGet, &oWidth, &oHeight, &oDepth,
        &oUseColor);
    (void) _WinScreenMode(winScreenModeGetSupportsColor, NULL, NULL, NULL,
			  &enablecol);
    (void) _WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL,
                          &dep, NULL);
    // in theory there's 16color _as well as_ 16grays
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

        // if we're trying for hires we _must_ use color (256 minimum)
        // if (hires && (depth < 8)) depth = 8;
        // Could not match...
        if (!dep) {
            (void) unloadHiRes();
            depth = 1;
            enablecol = 0;
            SETWIDTH(160);
            SETHEIGHT(160);
        }
    } else {
      enablecol = 0;
      depth = 1;
      SETWIDTH(160);
      SETHEIGHT(160);
      (void) unloadHiRes();
    }

    width = sWidth;
    height = sHeight;
    
    return (_WinScreenMode(winScreenModeSet, &width, &height, &depth,
          &enablecol));
}

Err
restoreDepthRes(void)
{
    
    UInt32 de;
    UInt32 wi;
    UInt32 he;
    Boolean ec;
    Err rv;
    de = oDepth;
    wi = oWidth;
    he = oHeight;
    ec = oUseColor;
    if (canHires())
    if ((rv = _WinScreenMode(winScreenModeSet, &wi, &he, &de, &ec)) != 0)
        return (rv);
    if (0 != (rv = unloadHiRes())) return (rv);
        return (0);
}

Boolean
canColor(UInt16 nbits)
{
    static Boolean rv = false;
    static Boolean inited = false;
    if (!inited) {
        UInt32 de;
        UInt32 wi = 160;
        UInt32 he = 160;
        Boolean ec;
        inited = true;
        WinScreenMode(winScreenModeGetSupportedDepths, &wi, &he, &de, &ec);
        if (de & (1<<(nbits-1)))
            rv = true;
    }
    return (rv);
}

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

    return nCreatorID;
}

