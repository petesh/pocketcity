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

/*! \brief set the tile size */
void
UIResetViewable(void)
{
	vgame.TileSize = 16;
	vgame.MapTileSize = 4;
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
	setCityFileName(filename);
	if (0 == load_defaultfilename(palm)) {
		UIResetViewable();
		PostLoadGame();
		DrawGame(1);
		MapHasJumped();
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
open_afile(GtkObject *sel __attribute__((unused)), gpointer data)
{
	fsh_data *dat = (fsh_data *)data;
	doOpen((gchar *)gtk_file_selection_get_filename(
		    GTK_FILE_SELECTION(dat->file_sel)),
	    GPOINTER_TO_INT(dat->data));
}

/*!
 * \brief free the widget and the data pointer
 */
static void
free_object(GtkObject *obj __attribute__((unused)), gpointer data)
{
	fsh_data *dat = (fsh_data *)data;
	gtk_widget_destroy(dat->file_sel);
	g_free(dat);
}

/*!
 * \brief open a game file from the system
 * \param w unused
 * \param data unused
 * \todo save any game in progress
 */
void
opengame_handler(gpointer data __attribute__((unused)),
    guint action, GtkWidget *w __attribute__((unused)))
{
	fsh_data *handler = g_malloc(sizeof (fsh_data));

	handler->file_sel = gtk_file_selection_new("Select saved game to open");
	handler->data = GINT_TO_POINTER(action - 1);

	g_signal_connect(GTK_OBJECT(
		    GTK_FILE_SELECTION(handler->file_sel)->ok_button),
	    "clicked", G_CALLBACK(open_afile), (gpointer)handler);

	g_signal_connect(
	    GTK_OBJECT(GTK_FILE_SELECTION(handler->file_sel)->ok_button),
	    "clicked", G_CALLBACK(free_object), (gpointer)handler);
	g_signal_connect(
	    GTK_OBJECT(GTK_FILE_SELECTION(handler->file_sel)->cancel_button),
	    "clicked", G_CALLBACK(free_object), (gpointer)handler);

	gtk_widget_show(GTK_WIDGET(handler->file_sel));
}

static GtkWidget *ng_form;

static gint
close_newgame(GtkWidget *widget __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	ng_form = NULL;
	return (FALSE);
}

/*!
 * \brief start a new game
 * \param w unused
 * \param data unsued
 * \todo save the game before starting a new one
 */
void
newgame_handler(void)
{
	GtkWidget *table, *mainbox;

	//SetupNewGame();
	//UIResetViewable();
	//setLoopSeconds(SPEED_PAUSED);
	ng_form = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(ng_form), "Create New City");

	mainbox = gtk_vbox_new(FALSE, 10);
	table = gtk_table_new(12, 3, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(mainbox), 3);

	g_signal_connect(G_OBJECT(ng_form), "delete_event",
	    G_CALLBACK(close_newgame), 0);

	gtk_container_add(GTK_CONTAINER(ng_form), mainbox);
	gtk_box_pack_start(GTK_BOX(mainbox), table, TRUE, TRUE, 0);

	gtk_widget_show_all(ng_form);
}

/*!
 * \brief set the save game name
 * \param sel the file selection dialog that caused this
 * \param data unused
 */
void
store_filename(GtkWidget *sel __attribute__((unused)), gpointer data)
{
	gchar *name = (gchar*)gtk_file_selection_get_filename(data);

	if (name != NULL)
		setCityFileName(name);
	else
		return;
	WriteLog("This game will be saved as %s from now on\n", name);
	save_defaultfilename(0);
}

void
savegameas_handler(void)
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
savegame_handler(void)
{
	if (getCityFileName() == NULL) {
		savegameas_handler();
		return;
	}

	save_defaultfilename(0);
}
