#include <gtk/gtk.h>

#if !defined(_SAVEGAME_FE_H)
#define _SAVEGAME_FE_H

#ifdef __cplusplus
extern "C" {
#endif

void UINewGame(GtkWidget *w, gpointer data);
void UISaveGameAs(GtkWidget *w, gpointer data);
void UISaveGame(GtkWidget *w, gpointer data);
void UIOpenGame(GtkWidget *w, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /* _SAVEGAME_FE_H */
