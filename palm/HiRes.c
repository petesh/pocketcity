#include <PalmTypes.h>
#include <FeatureMgr.h>
#include <ErrorBase.h>
#include <ErrorMgr.h>
#include <SystemMgr.h>
#include <ui.h>
#include <PenInputMgr.h>

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
 *
 * \todo software silk can resize display
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
			SETWIDTH(BASEWIDTH * 2);
			SETHEIGHT(BASEHEIGHT * 2);
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
 * \brief is the screen running at double or greater resolution?
 *
 * This is to allow us to use a smaller font on the 1 and 1.5 resolution
 * displays.
 * \return is the screen double or greater resolution
 */
Boolean
isDoubleOrMoreResolution()
{
	if (hdfs >= kDensityDouble || sonyHires())
		return (1);
	return (0);
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
	UInt32 mul;
	if (isHires()) {
		if (sonyHires())
			mul = kDensityDouble;
		else
			mul = hdfs;
		event->screenX  = (Coord)((UInt32)event->screenX * mul /
		    kDensityLow);
		event->screenY = (Coord)((UInt32)event->screenY * mul /
		    kDensityLow);
	}
}

/*!
 * \brief scale an X coordinate
 * \param x the coordinate to scale
 * \return the newly scaled coordinate
 */
Coord
scaleCoord(Coord x)
{
	UInt32 mul = sonyHires() ? kDensityDouble : hdfs;
	return ((Coord)((UInt32)x * mul / kDensityLow));
}

/*!
 * \brief normalize a scaled value back to the 'original' resolution
 * \param x the parameter to scale
 * \return the normalized value
 */
Coord
normalizeCoord(Coord x)
{
	UInt32 mul = sonyHires() ? kDensityDouble : hdfs;
	return ((Coord)((UInt32)x * kDensityLow / mul));
}

/*!
 * \brief check for the virtual silk screen
 * \return true if the Virtual Silkscreen is available
 */
Int16
hasVirtualSilk(void)
{
	static Int16 has_silk = -1;
	Err err;
	UInt32 version;

	if (has_silk != -1)
		return (has_silk);

	err = FtrGet(pinCreator, pinFtrAPIVersion, &version);
	if (!err && version) {
		if (pinAPIVersion1_0 == version)
			has_silk = 1;
		else
			has_silk = 2;
		return (has_silk);
	}

	if (SonySilk()) {
		has_silk = 1;
	}

	has_silk = 0;

	return (has_silk);
}

/*!
 * \brief end the use of the silk routines
 */
void
EndSilk(void)
{
	SonyEndSilk();
}

/*!
 * \brief Set silk is resizable or not
 */
void
SetSilkResizable(FormPtr form, UInt8 resizeable)
{
	if (hasVirtualSilk()) {
		UInt16 state;

		if (resizeable)
			state = pinInputTriggerEnabled;
		else
			state = pinInputTriggerDisabled;
		if (resizeable) {
			FrmSetDIAPolicyAttr(form, frmDIAPolicyCustom);
		}
		PINSetInputTriggerState(state);
	} else if (SonySilk()) {
		SonySetSilkResizable(resizeable);
	}
}

#endif /* HRSUPPORT */
