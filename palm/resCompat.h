#if !defined(_RESCOMPAT_H_)
#define	_RESCOMPAT_H_

#include <PalmTypes.h>
#include <Window.h>
#include <Form.h>
#include <Event.h>
#include <sony_support.h>
#include <sections.h>

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
Coord scaleCoord(Coord x);
Coord normalizeCoord(Coord x);
Boolean canHires(void);
Boolean isHires(void);
void setScreenRes(void) OTHER_SECTION;
void StartHiresDraw(void);
void EndHiresDraw(void);
void StartHiresFontDraw(void);
void EndHiresFontDraw(void);
void SetSilkResizable(FormPtr form, UInt8 resizable) OTHER_SECTION;
Int16 hasVirtualSilk(void) OTHER_SECTION;
void EndSilk(void) OTHER_SECTION;
Boolean CollapseMove(FormPtr form, Boolean modal, Int16 *roffsetX,
    Int16 *roffsetY) OTHER_SECTION;
void CollapsePreRedraw(FormPtr form) OTHER_SECTION;
#define	StartScaleDraw	StartHiresFontDraw
#define EndScaleDraw		EndHiresFontDraw

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
#define scaleCoord(x)	(x)
#define normalizeCoord(X)	(X)
#define StartHiresDraw()
#define EndHiresDraw()
#define StartHiresFontDraw()
#define EndHiresFontDraw()
#define	StartScaleDraw()
#define EndScaleDraw()
#define setScreenRes()
#define SetSilkResizable(X,Y)
#define	hasVirtualSilk()	(0)
#define EndSilk()
#define CollapseMove(F,M,X,Y)
#define	CollapsePreRedraw(X)

#endif /* SONY_CLIE || PALM_HIGH */


#endif /* _RESCOMPAT_H_ */
