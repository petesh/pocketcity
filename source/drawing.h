/*! \file
 * \brief interface to routines that are used for drawing
 */
#if !defined(_DRAWING_H_)
#define	_DRAWING_H_

#include <zakdef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <appconfig.h>

#ifndef OTHER_SECTION
#define	OTHER_SECTION
#endif

void SetUpGraphic(void);
void RedrawAllFields(void);
void ScrollMap(dirType direction);
void MoveCursor(dirType direction);
void DrawField(Int16 xpos, Int16 ypos);
void DrawCross(Int16 xpos, Int16 ypos, Int16 xsize, Int16 ysize);
welem_t GetSpecialGraphicNumber(UInt32 pos);
void DrawFieldWithoutInit(Int16 xpos, Int16 ypos);
void Goto(Int16 x, Int16 y) OTHER_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* _DRAWING_H_ */
