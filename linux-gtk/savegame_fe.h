#include <gtk/gtk.h>

#if !defined(_SAVEGAME_FE_H)
#define _SAVEGAME_FE_H

#ifdef __cplusplus
extern "C" {
#endif

void newgame_handler(void);
void savegameas_handler(void);
void savegame_handler(void);
void opengame_handler(gpointer data, guint action, GtkWidget *w);

#ifdef __cplusplus
}
#endif

#endif /* _SAVEGAME_FE_H */
