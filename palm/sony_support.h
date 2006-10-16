/*!
 * \file
 * \brief routines to interface with the sony high-resolution routines
 *
 * This contains all the interface functionality to the sony routines
 */
#if !defined(_SONY_SUPPORT_H_)
#define	_SONY_SUPPORT_H_

#ifdef SONY_CLIE

#include <Font.h>
#include <FontSelect.h>
#include <PalmOS.h>
#include <SonyCLIE.h>

#include <sections.h>

/*!
 * \brief test if this handheld can perform sony high resolution functions
 * \return true if it can perform sony high resolution functions
 */
Boolean sonyCanHires(void) SONY_SECTION;

/*!
 * \brief is this a sony high resolution implementation
 * \return true if it's a sony high resolution implementation.
 */
Boolean sonyHires(void) SONY_SECTION;
/*!
 * \brief Sony specific code to load high resolution library
 * \return ErrNone if it all went OK.
 */
Err loadHiRes(void) SONY_SECTION;
/*!
 * \brief code to unload the high resolution library if we loaded it
 * \return ErrNone -- it always succeeds
 */
Err unloadHiRes(void) SONY_SECTION;
/*! \brief unhook the hold switch */
void unhookHoldSwitch(void) SONY_SECTION;
/*!
 * \brief hook into the hold switch
 * \param CallBack the callback routine
 */
void hookHoldSwitch(void (*CallBack)(UInt32)) SONY_SECTION;

/*!
 * \brief Check if this device is a Sony. For Jog Navigation Support
 * \return true if device is a sony.
 */
Boolean IsSony(void) SONY_SECTION;

/*!
 * \brief Check if sony silk available.
 *
 * Loads library as well.
 * \return true if sony silk library is available
 */
Boolean SonySilk(void) SONY_SECTION;

/*!
 * \brief finish up with the sony silk library.
 *
 * Does not unload the silk library. It seems as though if we unload the silk
 * library then the handheld is unable to use it correctly until after the
 * next reset, which is not nice!
 */
void SonyEndSilk(void) SONY_SECTION;

/*!
 * \brief set silk area to be resizable or not
 * \param state the choice - non-zero implies resizable
 */
void SonySetSilkResizable(UInt8 resizeable) SONY_SECTION;

/*!
 * \brief redraw routine for post-collapse event
 * \pram form the form in question
 *
 * Needed because the first incarnation of the sony silk library needs
 * to have the form erased manually after a collapse/expand event
 */
void SonyCollapsePreRedraw(FormPtr form) SONY_SECTION;

/*!
 * \brief WinScreenMode call for sony handheld
 * \param op operation to perform
 * \param width <inout> screen width
 * \param height <inout> screen height
 * \param depth <inout> screen depth
 * \param enableColor <inout> enable colors.
 * \return error code from call (underlying WinScreenMode)
 *
 * Deals with the high-resolution displays also.
 */
Err _WinScreenMode(WinScreenModeOperation op, UInt32 *width, UInt32 *height,
    UInt32 *depth, Boolean *enableColor) SONY_SECTION;

/*!
 * \brief Erase a rectangle
 * \param r the rectangle
 * \param cornerDiam diameter of corner
 */
void _WinEraseRectangle(RectangleType *r, UInt16 cornerDiam) SONY_SECTION;

/*!
 * \brief Fill a rectangle
 * \param r the rectangle
 * \param cornerDiam the diameter of the corner
 */
void _WinDrawRectangle(RectangleType *rect, UInt16 cornerDiam) SONY_SECTION;

/*!
 * \brief create a bitmap
 * \param width width of bitmap
 * \param height height of bitmap
 * \param depth depth of bitmap
 * \param clut color lookup table
 * \param error <inout> errorcode from call
 * \return pointer to bitmap, or NULL, reason is in error
 */
BitmapType *_BmpCreate(Coord width, Coord height, UInt8 depth,
    ColorTableType *clut, UInt16 *error) SONY_SECTION;

/*!
 * \brief Draw a bitmap
 * \param bmp the bitmap to draw
 * \param x the starting x coordinate
 * \param y the starting y coordinate
 */
void _WinDrawBitmap(BitmapPtr bmp, Coord x, Coord y) SONY_SECTION;

/*!
 * \brief create a bitmap window
 * \param pBitmap the bitmap
 * \param err <out> the reaon for a failure
 * \return the handle to the window, or NULL on error
 */
WinHandle _WinCreateBitmapWindow(BitmapType *pBitmap, UInt16 *err) SONY_SECTION;

/*!
 * \brief create an offscreen bitmap
 * \param width the width
 * \param height the height
 * \param format the format of the bitmap (screen format, native format)
 * \param error <out> the error code if NULL is returned
 * \return the handle to the bitmap, or NULL on error
 */
WinHandle _WinCreateOffscreenWindow(Coord width, Coord height,
    WindowFormatType format, UInt16 *error) SONY_SECTION;

/*!
 * \brief copy a rectangle from one window to another
 * \param srcWin origin window
 * \param dstWin destination window
 * \param srcRect origin rectangle
 * \param destX location on destination window's x axis to place item
 * \param destY locaiton on destination window's y axis to place item
 * \param mode painting mode (copy &c)
 */
void _WinCopyRectangle(WinHandle srcWin, WinHandle dstWin,
    RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode)
    SONY_SECTION;

/*!
 * \brief draw some characters on the screen
 * \param chars the charaters to draw
 * \param len number of characters
 * \param x the x position
 * \param y the y position
 */
void _WinDrawChars(const Char *chars, Int16 len, Coord x, Coord y) SONY_SECTION;

/*!
 * \brief draw a rectangle frame
 * \param frame the type of frame to draw
 * \param rP the rectangle to draw
 */
void _WinDrawRectangleFrame(FrameType frame, RectangleType *rP) SONY_SECTION;

/*!
 * \brief set the current font
 * \param font the font to use
 */
void _FntSetFont(FontID font) SONY_SECTION;

/*!
 * \brief get the bounds of the current draw window
 * \param the rectangle to set to the size of it
 */
void _WinGetDrawWindowBounds(RectangleType *rP) SONY_SECTION;
 

#else

/* turn all the calls into collapsed constants */
#define	sonyCanHires()	(false)
#define	sonyHires()	(false)
#define	loadHiRes() (0)
#define	unloadHiRes() (0)
#define	unhookHoldSwitch()
#define	hookHoldSwitch(x)
#define	IsSony()	(false)
#define	SonySilk()	(0)
#define	SonyEndSilk()
#define	SonySetSilkResizable(X)
#define	SonyCollapsePreRedraw(X)

#define	_WinScreenMode WinScreenMode
#define	_WinEraseRectangle WinEraseRectangle
#define	_WinDrawRectangle WinDrawRectangle
#define	_WinDrawBitmap WinDrawBitmap
#define	_BmpCreate BmpCreate
#define	_WinCopyRectangle WinCopyRectangle
#define	_WinDrawChars WinDrawChars
#define	_WinDrawRectangleFrame WinDrawRectangleFrame
#define	_FntSetFont FntSetFont
#define	_WinCreateOffscreenWindow WinCreateOffscreenWindow
#define	_WinCreateBitmapWindow WinCreateBitmapWindow
#define _WinGetDrawWindowBounds WinGetDrawWindowBounds

#endif /* SONY_CLIE */

#endif /* _SONY_SUPPORT_H_ */
