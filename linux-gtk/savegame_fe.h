#include <gtk/gtk.h>

#if !defined(_SAVEGAME_FE_H)
#define _SAVEGAME_FE_H

#ifdef __cplusplus
extern "C" {
#endif

void NewGame(GtkWidget *w, gpointer data);
void SaveGameAs(GtkWidget *w, gpointer data);
void SaveGame(GtkWidget *w, gpointer data);
void OpenGame(GtkWidget *w, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /* _SAVEGAME_FE_H */
