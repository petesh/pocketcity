#include <PalmTypes.h>
#include <FeatureMgr.h>
#include <ErrorBase.h>
#include <ErrorMgr.h>
#include <SystemMgr.h>
#include <ui.h>

#define	_HIRESSOURCE_
#include <resCompat.h>
#include <palmutils.h>

#if defined(HRSUPPORT)

/*! \brief screen width in native coordinates */
Int32 sWidth;
/*! \brief screen height in native coordinates */
Int32 sHeight;
/*! \brief high density feature check value; holds density value */
static UInt32 hdfs = ~0UL;

/*!
 * \brief set the screen resolution
 * 
 * This sets the screen resolution in terms of width and height based on
 * the high density feature set or the sony library.
 */
void
setScreenRes(void)
{
	switch(highDensityFeatureSet()) {
	case kDensityQuadruple:
		WriteLog("Quadruple Density\n");
	case kDensityTriple:
		WriteLog("Triple Density\n");
	case kDensityDouble:
		WriteLog("Double Density\n");
		SETWIDTH(BASEWIDTH * 2);
		SETHEIGHT(BASEHEIGHT * 2);
		break;
	case kDensityOneAndAHalf:
		WriteLog("One And A Half Density\n");
		SETWIDTH(BASEWIDTH * 1.5);
		SETHEIGHT(BASEHEIGHT * 1.5);
		break;
	default:
		if (sonyHires()) {
			WriteLog("Sony High Density\n");
			SETWIDTH(320);
			SETHEIGHT(320);
		} else {
			WriteLog("Single Density\n");
			SETWIDTH(BASEWIDTH);
			SETHEIGHT(BASEHEIGHT);
		}
		break;
	}
}
/*!
 * \brief Check for the High Density Feature set
 *
 * This routine is used to permit choosing an appropriate screen
 * resolution for all the painted screen.
 *
 * \return 0 if there is no Hi Density Available
 */
UInt32
highDensityFeatureSet(void)
{
	UInt32 tval;

	if (hdfs != ~0UL)
		return (hdfs);

	hdfs = 0;

	if (0 != FtrGet(sysFtrCreator, sysFtrNumWinVersion, &tval))
		goto out;
	if (tval < 4)
		goto out;
	if (0 != WinScreenGetAttribute(winScreenDensity, &tval))
		goto out;
	hdfs = tval;
	if (hdfs == kDensityLow)
		hdfs = 0;
out:
	return (hdfs);
}

/*!
 * \brief start high resolution drawing
 * 
 * Do not call this without having checked that the high density feature
 * set is available.
 * It is important to call the Start/End functions while on the same
 * active window, otherwise chaos can ensue.
 */
void
StartHiresDraw(void)
{
	if (hdfs) WinSetCoordinateSystem(hdfs);
	StartHiresFontDraw();
}

/*!
 * \brief End the High Resolution Drawing
 *
 * Restores the active window's coordinate system to the standard coordinate
 * system.
 */
void
EndHiresDraw(void)
{
	EndHiresFontDraw();
	if (hdfs) WinSetCoordinateSystem(kCoordinatesStandard);
}

/*!
 * \brief Make the fonts paint in high resolution i.e. smaller
 *
 * This is accomplished by telling the current screen that it is in
 * low resolution mode.
 */
void
StartHiresFontDraw(void)
{
	if (hdfs) {
		BmpSetDensity(WinGetBitmap(WinGetDrawWindow()),
		    kDensityLow);
	}
}

/*!
 * \brief Make the fonts paint at normal resolution
 *
 * This is accomplished by telling the active drawing window that it is
 * operating at it's native resolution.
 */
void
EndHiresFontDraw(void)
{
	if (hdfs) {
		BmpSetDensity(WinGetBitmap(WinGetDrawWindow()), hdfs);
	}
}

/*!
 * \brief Check we can do resolutions other than normal.
 *
 * checks against both the Palm5 High Resolution API as well as the
 * sony high resolution API.
 * \return If the machine can perform High Resolution drawing
 */
Boolean
canHires(void)
{
	return (highDensityFeatureSet() || sonyCanHires());
}

/*!
 * \brief check that the screen is in high resolution mode
 *
 * Because this call verifies the maximum density of the display we
 * are guaranteed it will function correctly on the Palm 5
 */
Boolean
isHires(void)
{
	return (highDensityFeatureSet() || sonyHires());
}

/*!
 * \brief scale the event pointer's location on screen
 *
 * This allows us to check the location on screen in terms of the high
 * density coordinates set.
 */
void
scaleEvent(EventPtr event)
{
	if (isHires()) {
		event->screenX *= sWidth / BASEWIDTH;
		event->screenY *= sHeight / BASEHEIGHT;
	}
}

/*
UInt32
xScale(void)
{
	if (isHires())
		return (sWidth / BASEWIDTH);
	else
		return (1);
}

UInt32
yScale()
{
	if (isHires())
		return (sHeight / BASEHEIGHT);
	else
		return (1);
}
*/

#endif /* HRSUPPORT */
