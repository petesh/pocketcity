/*!
 * \file
 * \brief interface to the general ui routines
 */
#include <gtk/gtk.h>

#if !defined(_UIBITS_H)
#define _UIBITS_H

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief create a left aligned label
 * \param string the string to put into the label
 * \return the newly created label
 */
GtkWidget *create_right_label(char *string);

/*!
 * \brief create a right aligned label
 * \param s the string to set the value to
 * \return the newly created right laligned label
 */
GtkWidget *create_left_label(char *s);

#if defined(__cplusplus)
}
#endif

#endif
