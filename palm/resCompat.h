/*!
 * \file
 * \brief compatibility interface file
 *
 * Provides interface to allow the use of either high resolution code or
 * low resolution code depending on the setting of the SONY_CLIE or PALM_HIGH
 * macros.
 */
#if !defined(_RESCOMPAT_H_)
#define	_RESCOMPAT_H_

#include <PalmTypes.h>
#include <Window.h>
#include <Form.h>
#include <Event.h>
#include <sony_support.h>
#include <sections.h>

#define	BASEWIDTH ((Coord)160)
#define	BASEHEIGHT ((Coord)160)

#if defined(SONY_CLIE) || defined(PALM_HIGH)

#define CM_DEFAULT	(0)
#define CM_MOVEX	(1<<1)
#define CM_MOVEY	(1<<2)
#define CM_MODAL	(1<<3)

#define	HRSUPPORT

extern Coord sWidth;
extern Coord sHeight;

#define	SETWIDTH(x)	sWidth = (x)
#define	SETHEIGHT(y)	sHeight = (y)
#define GETWIDTH()	(sWidth)
#define GETHEIGHT()	(sHeight)

Boolean isDoubleOrMoreResolution(void);
void scaleEvent(EventPtr event);
UInt32 highDensityFeatureSet(void);
Coord scaleCoord(Coord x);
Coord normalizeCoord(Coord x);
Boolean canHires(void);
Boolean isHires(void);
void setScreenRes(void) HIRES_SECTION;
void StartHiresDraw(void);
void EndHiresDraw(void);
void StartHiresFontDraw(void);
void EndHiresFontDraw(void);
void SetSilkResizable(FormPtr form, UInt8 resizable) HIRES_SECTION;
Int16 hasVirtualSilk(void) HIRES_SECTION;
void StartSilk(void) HIRES_SECTION;
void EndSilk(void) HIRES_SECTION;
Boolean collapseMove(FormPtr form, UInt8 stretchy, Int16 *roffsetX,
    Int16 *roffsetY) HIRES_SECTION;
void collapsePreRedraw(FormPtr form) HIRES_SECTION;
#define	StartScaleDraw	StartHiresFontDraw
#define EndScaleDraw		EndHiresFontDraw

#else

/* turn all the calls into collapsed constants */

#define	sWidth BASEWIDTH
#define	sHeight BASEHEIGHT

#define	SETWIDTH(x)
#define	SETHEIGHT(y)
#define GETWIDTH()	(BASEWIDTH)
#define GETHEIGHT()	(BASEHEIGHT)

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
#define StartSilk()
#define collapseMove(F,M,X,Y)
#define	collapsePreRedraw(X)

#endif /* SONY_CLIE || PALM_HIGH */


#endif /* _RESCOMPAT_H_ */
