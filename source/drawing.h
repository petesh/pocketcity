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

void SetUpGraphic(void) OTHER_SECTION;
void RedrawAllFields(void) OTHER_SECTION;
void ScrollMap(dirType direction) OTHER_SECTION;
void MoveCursor(dirType direction) OTHER_SECTION;
void DrawField(Int16 xpos, Int16 ypos) OTHER_SECTION;
void DrawCross(Int16 xpos, Int16 ypos) OTHER_SECTION;
UInt8 GetSpecialGraphicNumber(UInt32 pos, Int16 nType) OTHER_SECTION;
void DrawFieldWithoutInit(Int16 xpos, Int16 ypos) OTHER_SECTION;
void Goto(Int16 x, Int16 y) OTHER_SECTION;

Int16 IsRoad(UInt8 x) OTHER_SECTION;
Int16 IsZone(UInt8 x, zoneType nType) OTHER_SECTION;
Int16 CarryPower(UInt8 x) OTHER_SECTION;
Int16 CarryWater(UInt8 x) OTHER_SECTION;
Int16 IsPowerLine(UInt8 x) OTHER_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* _DRAWING_H_ */
