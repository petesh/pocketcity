/*!
 * \file
 * interface to the minimap
 */

#include <PalmTypes.h>
#include <Rect.h>

/*!
 * \brief is the minimap showing
 * \return true if the minimap is showing
 */
Int8 minimapGetShowing(void);

/*!
 * \brief set whether the minimap is showing
 * \param show set whether it is showing or not
 */
void minimapSetShowing(Int8 show);

/*!
 * \brief get the detailed flag (fast devices can render details)
 * \return whether detailed minimaps are shown
 */
Int8 minimapGetDetailed(void);

/*!
 * \brief set detailed flag for minimap
 * \param detailed the new setting for the detailed map
 */
void minimapSetDetailed(Int8 detailed);

/*!
 * \brief set the location of the minimap on the display
 * \param pos the position and size of the minimap
 */
void minimapPlace(RectangleType *pos);

/*!
 * \brief paint the minimap if it is visible on the screen
 */
void minimapPaint(void);

/*!
 * \brief check if the minimap is tapped, returning the %age on each axis
 * \param point the point to test
 * \param percentage the percentages on the x and y axis
 * \return non-zero if the map has been tapped, zero otherwise.
 */
Int8
minimapIsTapped(PointType *point, PointType *percentage);

/*!
 * \brief get the intersection of the other rectangle and the dest rectangle
 * \param other the other rectangle (usually the play area)
 * \param dest the overlap rectangle
 */
void
minimapIntersect(const RectangleType *other, RectangleType *dest);
