#ifndef _RESCOMPAT_H_
#define _RESCOMPAT_H_

#include <PalmTypes.h>
#include <Window.h>
Boolean canColor(UInt16 nbits);
Err changeDepthRes(UInt32 ncolors);
Err restoreDepthRes(void);
UInt32 getDepth(void);
UInt32 GetCreatorID(void);

#ifdef SONY_CLIE
#include <Font.h>
#include <FontSelect.h>
#include <SonyCLIE.h>

extern UInt32 sWidth;
extern UInt32 sHeight;
#define SETWIDTH(x)     sWidth=(x)
#define SETHEIGHT(y)    sHeight=(y)
Err loadHiRes(void);
Err unloadHiRes(void);
Boolean isHires(void);
Boolean canHires(void);
void scaleEvent(EventPtr event);
void unhookHoldSwitch(void);
void hookHoldSwitch(void (*CallBack)(UInt32));
int IsDrawWindowMostOfScreen(void);

Err _WinScreenMode(WinScreenModeOperation op, UInt32 *width, UInt32 *height,
  UInt32 *depth, Boolean *enableColor);
void _WinEraseRectangle(RectangleType *r, UInt16 cornerDiam);
void _WinDrawBitmap(BitmapPtr bmp, Coord x, Coord y);
WinHandle _WinCreateOffscreenWindow (Coord width, Coord height,
  WindowFormatType format, UInt16 *error);
void _WinCopyRectangle(WinHandle srcWin, WinHandle dstWin,
  RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode);
void _WinDrawChars (const Char *chars, Int16 len, Coord x, Coord y);
void _WinDrawRectangleFrame(FrameType frame, RectangleType *rP);
void _FntSetFont(FontID font);

#else

/* turn all the calls into collapsed constants */
#define sWidth 160
#define sHeight 160
#define SETWIDTH(x)
#define SETHEIGHT(y)
#define loadHiRes() (0)
#define unloadHiRes() (0)
#define isHires() false
#define canHires() false
#define scaleEvent(t)
#define unhookHoldSwitch()
#define hookHoldSwitch(x)
#define IsDrawWindowMostOfScreen() true

#define _WinScreenMode WinScreenMode
#define _WinEraseRectangle WinEraseRectangle
#define _WinDrawBitmap WinDrawBitmap
#define _WinCopyRectangle WinCopyRectangle
#define _WinDrawChars WinDrawChars
#define _WinDrawRectangleFrame WinDrawRectangleFrame
#define _FntSetFont FntSetFont
#define _WinCreateOffscreenWindow WinCreateOffscreenWindow

#endif /* SONY_CLIE */

#endif /* _RESCOMPAT_H_ */

