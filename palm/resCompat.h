#if !defined(_RESCOMPAT_H_)
#define	_RESCOMPAT_H_

#include <PalmTypes.h>
#include <Window.h>
#include <Event.h>
#include <sony_support.h>

#define	BASEWIDTH ((Int32)160)
#define	BASEHEIGHT ((Int32)160)

#if defined(SONY_CLIE) || defined(PALM_HIGH)

#define	HRSUPPORT

extern Int32 sWidth;
extern Int32 sHeight;

#define	SETWIDTH(x)	sWidth = (x)
#define	SETHEIGHT(y)	sHeight = (y)

Boolean isDoubleOrMoreResolution(void);
void scaleEvent(EventPtr event);
UInt32 highDensityFeatureSet(void);
Coord scaleCoordX(Coord x);
Coord scaleCoordY(Coord y);
Boolean canHires(void);
Boolean isHires(void);
void setScreenRes(void);
void StartHiresDraw(void);
void EndHiresDraw(void);
void StartHiresFontDraw(void);
void EndHiresFontDraw(void);
#define	StartScaleDraw	StartHiresFontDraw
#define EndScaleDraw		EndHiresFontDraw
#define normalizeX(X)	((X) * BASEWIDTH / sWidth)
#define normalizeY(Y)	((Y) * BASEHEIGHT / sHeight)

#else

/* turn all the calls into collapsed constants */

#define	sWidth BASEWIDTH
#define	sHeight BASEHEIGHT

#define	SETWIDTH(x)
#define	SETHEIGHT(y)

#define	isHires() (false)
#define	canHires() (false)

#define highDensityFeatureSet()	(0)
#define isDoubleOrMoreResolution() (0)
#define scaleEvent(e)
#define scaleCoordX(x)	(x)
#define scaleCoordY(y)	(y)
#define normalizeX(X)	(X)
#define normalizeY(Y)	(Y)
#define StartHiresDraw()
#define EndHiresDraw()
#define StartHiresFontDraw()
#define EndHiresFontDraw()
#define	StartScaleDraw()
#define EndScaleDraw()
#define setScreenRes()

#endif /* SONY_CLIE || PALM_HIGH */


#endif /* _RESCOMPAT_H_ */
