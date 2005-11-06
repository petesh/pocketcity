/*!
 * \file
 * \brief externally exposed symbols from the main form
 */

#include <zakdef.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#if !defined(_MAIN_H_)
#define	_MAIN_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief draw a field for the map
 * \param xpos the horizontal position
 * \param ypos the vertical position
 * \param nGraphic the item to paint
 * \param drawable the drawable surface to paint on
 */
void UIDrawMapZone(Int16 xpos, Int16 ypos, welem_t nGraphic,
    GdkDrawable *drawable);

/*!
 * \brief draw a special object for the map
 * \param i the item to draw
 * \param xpos the horizontal position
 * \param ypos the vertical position
 * \param drawable the drawable to paint on
 */
void UIDrawMapSpecialObject(Int16 xpos, Int16 ypos, Int16 i,
    GdkDrawable *drawable);

/*!
 * \brief draw a special unit on the map
 * \param i the unit do draw
 * \param xpos the horizontal location on screen
 * \param ypos the vertical location on screen
 * \param drawable the drawable to paint on
 */
void UIDrawMapSpecialUnit(Int16 xpos, Int16 ypos, Int16 i,
    GdkDrawable *drawable);

/*
 * \brief get the widget corresponding to the main window
 * \return the widget (could be null)
 */
GtkWidget *window_main_get(void);
GdkDrawable *drawable_main_get(void);

#if defined(__cplusplus)
}
#endif

#endif /* _MAIN_H_ */
