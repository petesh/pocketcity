/*!
 * \file
 * \brief externally exposed symbols from the main form
 */

#include <zakdef.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#if !defined(_MAIN_H_)
#define _MAIN_H_

#if defined(__cplusplus)
extern "C" {
#endif

void UIDrawMapField(Int16 xpos, Int16 ypos, welem_t nGraphic,
    GdkDrawable *drawable);
void UIDrawMapSpecialObject(Int16 xpos, Int16 ypos, Int16 i,
    GdkDrawable *drawable);
void UIDrawMapSpecialUnit(Int16 xpos, Int16 ypos, Int16 i,
    GdkDrawable *drawable);

GtkWidget *mainwindow_get(void);

#if defined(__cplusplus)
}
#endif

#endif /* _MAIN_H_ */
