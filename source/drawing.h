/*! \file
 * \brief interface to routines that are used for drawing
 */
#if !defined(_DRAWING_H_)
#define	_DRAWING_H_

#include <zakdef.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include <appconfig.h>
#include <sections.h>
#include <compilerpragmas.h>

EXPORT void SetUpGraphic(void);
EXPORT void RedrawAllFields(void);
EXPORT void ScrollDisplay(dirType direction);
EXPORT void MoveCursor(dirType direction);
EXPORT void DrawField(Int16 xpos, Int16 ypos);
EXPORT void DrawCross(Int16 xpos, Int16 ypos, Int16 xsize, Int16 ysize);
EXPORT welem_t GetSpecialGraphicNumber(UInt32 pos);
EXPORT welem_t GetGraphicNumber(UInt32 pos);
EXPORT void DrawFieldWithoutInit(Int16 xpos, Int16 ypos);

EXPORT void Goto(Int16 x, Int16 y);

#if defined(__cplusplus)
}
#endif

#endif /* _DRAWING_H_ */
