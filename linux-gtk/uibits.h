
#include <gtk/gtk.h>

#if !defined(_UIBITS_H)
#define _UIBITS_H

#if defined(__cplusplus)
extern "C" {
#endif

GtkWidget *create_right_label(char *string);
GtkWidget *create_left_label(char *s);

#if defined(__cplusplus)
}
#endif

#endif
