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

void scaleEvent(EventPtr event);
Boolean highDensityFeatureSet(void);
Boolean canHires(void);
Boolean isHires(void);
void scaleEvent(EventPtr event);
void StartHiresDraw(void);
void EndHiresDraw(void);
void StartHiresFontDraw(void);
void EndHiresFontDraw(void);

#else

/* turn all the calls into collapsed constants */

#define	sWidth BASEWIDTH
#define	sHeight BASEHEIGHT

#define	SETWIDTH(x)
#define	SETHEIGHT(y)

#define	isHires() (false)
#define	canHires() (false)

#define	scaleEvent(t)
#define highDensityFeatureSet()	(0)
#define scaleEvent(e)
#define StartHiresDraw()
#define EndHiresDraw()
#define StartHiresFontDraw()
#define EndHiresFontDraw()

#endif /* SONY_CLIE || PALM_HIGH */


#endif /* _RESCOMPAT_H_ */
