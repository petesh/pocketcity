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

/*! \brief initialize the graphics. */
void InitGraphic(void);

/*!
 * \brief Draw everything on the screen.
 *
 * The Game area, Credits and population.
 */
EXPORT void RedrawAllFields(void);

/*!
 * \brief Scroll the map in the direction specified
 * \param direction the direction to scroll map in
 */
EXPORT void ScrollDisplay(dirType direction);

/*!
 * \brief Move the cursor in the location specified.
 * \param direction the direction to scroll map in
 */
EXPORT void MoveCursor(dirType direction);

/*!
 * \brief Draw the field at the specified location
 * \param xpos horizontal position
 * \param ypos vertical position
 */
EXPORT void DrawField(UInt16 xpos, UInt16 ypos);

/*!
 * \brief Draw all zones around the point that is being painted.
 *
 * It is essential that you have locked the world and flags before calling this
 * routine.
 * \param xpos horizontal position
 * \param ypos vertical position
 * \param xsize the size on the x-axis
 * \param ysize the size on the y-axis
 */
EXPORT void DrawCross(UInt16 xpos, UInt16 ypos, UInt16 xsize, UInt16 ysize);

/*!
 * \brief Get the graphic to use for the position in question.
 * \param pos index into map array
 * \return the graphic to paint at this location
 */
EXPORT welem_t GetGraphicNumber(UInt32 pos);

/*
 * \brief Deals with special graphics fields
 * \param pos index into map array
 * \param ntype the type of the node.
 * \return the special graphic number for this place.
 */
EXPORT welem_t GetSpecialGraphicNumber(UInt32 pos);

/*!
 * \brief draw a field without initializing something.
 *
 * ONLY call this function if you make sure to call
 * UIInitDrawing and UIFinishDrawing in the caller
 * Also remember to call (Un)lockWorld (2 functions)
 *
 * This function will not try to draw outside the bounds of the
 * current world, or the defined 'clipping region'
 * \param xpos horizontal position
 * \param ypos vertical position
 */
EXPORT void DrawFieldWithoutInit(UInt16 xpos, UInt16 ypos);

/*! \brief codes for goto function */
typedef enum { goto_plain = 0, goto_center } goto_code;
/*!
 * \brief Set the map position to that location.
 *
 * this function also asks to repaint the screen.
 * If the center flag is set, then it makes that point be the center of the
 * screen.
 * \param x horizontal position
 * \param y vertical position
 * \param center should I center on this location?
 */
EXPORT void Goto(UInt16 x, UInt16 y, goto_code center);

#if defined(__cplusplus)
}
#endif

#endif /* _DRAWING_H_ */
