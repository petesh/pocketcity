#include <gtk/gtk.h>

#if !defined(_SAVEGAME_FE_H)
#define _SAVEGAME_FE_H

#ifdef __cplusplus
extern "C" {
#endif

void newgame_handler(GtkWidget *w, gpointer data);
void savegameas_handler(GtkWidget *w, gpointer data);
void savegame_handler(GtkWidget *w, gpointer data);
void opengame_handler(GtkWidget *w, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /* _SAVEGAME_FE_H */
