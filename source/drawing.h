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

/*! \brief Set up the graphics. */
EXPORT void SetUpGraphic(void);

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
EXPORT void DrawField(Int16 xpos, Int16 ypos);

/*!
 * \brief Draw all zones around the point that is being painted.
 * \param xpos horizontal position
 * \param ypos vertical position
 * \param xsize the size on the x-axis
 * \param ysize the size on the y-axis
 */
EXPORT void DrawCross(Int16 xpos, Int16 ypos, Int16 xsize, Int16 ysize);

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
 * current city.
 * \param xpos horizontal position
 * \param ypos vertical position
 */
EXPORT void DrawFieldWithoutInit(Int16 xpos, Int16 ypos);

/*!
 * \brief Set the map position to that location.
 *
 * this function also repaints screen.
 * \param x horizontal position
 * \param y vertical position
 */
EXPORT void Goto(Int16 x, Int16 y);

#if defined(__cplusplus)
}
#endif

#endif /* _DRAWING_H_ */
