/*! \file
 * \brief front end for performing savegames
 *
 * Does all the gtk-related processing for the savegames.
 * The idea is to separate the front end from the back end, allowing
 * us to use a common set of routines to read and write savegames on
 * unix platforms.
 */
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <main.h>
#include <globals.h>
#include <handler.h>
#include <ui.h>
#include <simulation.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <strings.h>
#include <savegame_fe.h>
#include <savegame_be.h>
#include <compilerpragmas.h>

/*! \brief the savegame name */
gchar *savegamename;

/*! \brief set the tile size */
void
UIResetViewable(void)
{
	vgame.tileSize = 16;
	/* XXX: set based on size of window */
}

/*!
 * \brief load a game
 *
 * Called by the open dialog handler.
 * \param data the file name
 */
static void
doOpen(gchar *filename, int palm)
{
	if (0 == open_filename(filename, palm)) {
		UIResetViewable();
		PostLoadGame();
		DrawGame(1);
		MapHasJumped();
		savegamename = g_strdup(filename);
	}
}

/*!
 * \brief structure to pass 2 parameters to the file selection handler
 * 
 * This is intended to get around the annoying fact that you can only pass
 * a data pointer to an event handler
 */
typedef struct {
	GtkWidget *file_sel; /*!< The file handler dialog */
	gpointer data; /*!< The data to go with the dialog */
} fsh_data;

/*!
 * \brief open a savegame
 * \param sel unused
 * \param data the filename from the selection dialog
 * \todo fix the code to open the specific filename based on introspection
 * into the filename
 */
static void
open_afile(GtkFileSelection *sel, gpointer data)
{
	fsh_data *dat = (fsh_data *)data;
	g_print("sel = %p; dat->file_sel = %p\n", sel, dat->file_sel);
	doOpen((gchar *)gtk_file_selection_get_filename(
		    GTK_FILE_SELECTION(dat->file_sel)),
	    GPOINTER_TO_INT(dat->data));
}

/*!
 * \brief open a game file from the system
 * \param w unused
 * \param data unused
 * \todo save any game in progress
 */
void
OpenGame(GtkWidget *w __attribute__((unused)), gpointer data)
{
	fsh_data *handler = g_malloc(sizeof (fsh_data));

	handler.file_sel = gtk_file_selection_new("Select saved game to open");
	handler.data = data;

	g_signal_connect(GTK_OBJECT(
		    GTK_FILE_SELECTION(handler.file_sel)->ok_button),
	    "clicked", G_CALLBACK(open_afile), (gpointer)&handler);

	g_signal_connect_swapped(
	    GTK_OBJECT(GTK_FILE_SELECTION(handler.file_sel)->ok_button),
	    "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)&handler);
	g_signal_connect_swapped(
	    GTK_OBJECT(GTK_FILE_SELECTION(handler.file_sel)->cancel_button),
	    "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)&handler);

	gtk_widget_show(GTK_WIDGET(handler.file_sel));
}

/*!
 * \brief start a new game
 * \param w unused
 * \param data unsued
 * \todo save the game before starting a new one
 */
void
NewGame(GtkWidget *w __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	SetupNewGame();
	UIResetViewable();
	game.gameLoopSeconds = SPEED_FAST;
}

/*!
 * \brief set the save game name
 * \param sel the file selection dialog that caused this
 * \param data unused
 */
void
store_filename(GtkFileSelection *sel, gpointer data __attribute((unused)))
{
	gchar *name = (gchar*)gtk_file_selection_get_filename(sel);

	if (name != NULL) {
		if (savegamename != NULL) {
			g_free(savegamename);
		}
		savegamename = name;
	}
	WriteLog("This game will be saved as %s from now on\n", savegamename);
	SaveGame(NULL, 0);
}

void
SaveGameAs(GtkWidget *w __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	GtkWidget *fileSel;

	fileSel = gtk_file_selection_new("Save game as...");
	g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
	    "clicked", G_CALLBACK(store_filename), (gpointer)fileSel);

	g_signal_connect_swapped(
	    GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
	    "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);
	g_signal_connect_swapped(
	    GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->cancel_button),
	    "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);

	gtk_widget_show(GTK_WIDGET(fileSel));
}

void
SaveGame(GtkWidget *w __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	if (savegamename == NULL) {
		SaveGameAs(NULL, 0);
		return;
	}

	save_filename(savegamename, 0);
}
