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

Int32 sWidth;
Int32 sHeight;
static Boolean hdfs;

Boolean
highDensityFeatureSet(void)
{
	static Boolean tried = false;
	UInt32 vat;

	if (tried == true)
		return (hdfs);

	tried = true;
	hdfs = false;

	if (0 != FtrGet(sysFtrCreator, sysFtrNumWinVersion, &vat))
		goto out;
	if (vat < 4)
		goto out;
	if (0 != WinScreenGetAttribute(winScreenDensity, &vat))
		goto out;
	switch(vat) {
	case kDensityQuadruple:
		WriteLog("Quadruple Density\n");
	case kDensityTriple:
		WriteLog("Triple Density\n");
	case kDensityDouble:
		hdfs = true;
		WriteLog("Double Density\n");
		SETWIDTH(BASEWIDTH * 2);
		SETHEIGHT(BASEHEIGHT * 2);
		break;
	default:
		WriteLog("Single Density\n");
		SETWIDTH(BASEWIDTH);
		SETHEIGHT(BASEHEIGHT);
		break;
	}
out:
	return (hdfs);
}

void
StartHiresDraw(void)
{
	if (hdfs) WinSetCoordinateSystem(kCoordinatesDouble);
}

void
EndHiresDraw(void)
{
	if (hdfs) WinSetCoordinateSystem(kCoordinatesStandard);
}

void
StartHiresFontDraw(void)
{
	if (hdfs) {
		BmpSetDensity(WinGetBitmap(WinGetDrawWindow()),
		    kDensityLow);
	}
}

void
EndHiresFontDraw(void)
{
	if (hdfs) {
		BmpSetDensity(WinGetBitmap(WinGetDrawWindow()),
		    kDensityDouble);
	}
}

Boolean
canHires(void)
{
	return (highDensityFeatureSet() || sonyCanHires());
}

Boolean
isHires(void)
{
	return (highDensityFeatureSet() || sonyHires());
}

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
