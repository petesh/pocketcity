/*!
 * \file
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
#include <uibits.h>

/*! \brief private city list selection object */
struct city_listselect {
	GtkListStore *store; /*!< store of list for screen */
	GtkWidget *list; /*!< list widget */
	savegame_t *sg; /*!< savegame widget */
};

/*! \brief set the tile size */
void
UIResetViewable(void)
{
	/*! \todo set based on size of window/tiles */
	vgame.TileSize = 16;
	vgame.MapTileSize = 4;
}

/*!
 * \brief free the list selection strucure
 * \param sel the city celection structure
 */
static void
free_listselect(struct city_listselect *sel)
{
	if (sel == NULL) return;
	if (sel->sg) savegame_close(sel->sg);
	if (sel->store != NULL) g_object_unref(sel->store);
	free(sel);
}
/*!
 * \brief load one of the palm games from the pdb file
 *
 * The OK button was clicked on the list of games from the list of
 * the palm cities.
 * \param widget the dialog
 * \param response the response button clicked on the dialog
 * \param data the city structure
 */
static void
ImportOneFromGame(GtkWidget *widget, gint response, gpointer data)
{
	struct city_listselect *sel = (struct city_listselect *)data;

	if (response == GTK_RESPONSE_OK) {
		GtkTreeSelection *select;
		GtkTreeModel *model;
		GtkTreeIter iter;
		int city = 0;

		select = gtk_tree_view_get_selection(GTK_TREE_VIEW(sel->list));
		if (gtk_tree_selection_get_selected(select, &model, &iter)) {
			gtk_tree_model_get(model, &iter, 0, &city, -1);
		}

		if (-1 != savegame_getcity(sel->sg, city, &game,
			    (char **)&worldPtr)) {
			UIResetViewable();
		}
		PostLoadGame();
		DrawGame(1);
		MapHasJumped();
	}
	free_listselect(sel);
	gtk_widget_destroy(widget);
	if (response == GTK_RESPONSE_OK) {
		UIResetViewable();
	}
}

/*!
 * \brief load the cities
 * \param sg the savegame
 */
static void
loadCities(savegame_t *sg)
{
	int i;
	struct city_listselect *ls = malloc(sizeof (struct city_listselect));
	GtkTreeIter iter;
	GtkWidget *dlg;

	ls->store = gtk_list_store_new(2, G_TYPE_UINT, G_TYPE_STRING);
	ls->sg = sg;

	for (i = 0; i < savegame_citycount(sg); i++) {
		gtk_list_store_append(ls->store, &iter);
		gtk_list_store_set(ls->store, &iter,
		    0, i,
		    1, savegame_getcityname(sg, i),
		    -1);
	}
	dlg = gtk_dialog_new_with_buttons("Pick A City",
	    GTK_WINDOW(mainwindow_get()), GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	    GTK_STOCK_OK, GTK_RESPONSE_OK,
	    NULL);

	ls->list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls->store));
	gtk_tree_view_append_column(GTK_TREE_VIEW(ls->list),
	    gtk_tree_view_column_new_with_attributes("Name",
	    gtk_cell_renderer_text_new(), "text", 1, NULL));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox),
	    ls->list, TRUE, TRUE, 0);
	g_signal_connect(GTK_OBJECT(dlg), "response",
	    G_CALLBACK(ImportOneFromGame), ls);
	gtk_widget_show_all(dlg);
}

/*!
 * \brief load a game
 *
 * Called by the open dialog handler.
 * \param filename the file name
 */
static void
doOpen(gchar *filename)
{
	savegame_t *sg = savegame_open(filename);

	if (sg == NULL) {
		WriteLog("no savegames in file\n");
		return;
	}
	setCityFileName(filename);

	if (savegame_citycount(sg) == 0) {
		savegame_close(sg);
		return;
	}

	if (savegame_citycount(sg) > 1) {
		loadCities(sg);
	} else {
		if (-1 != savegame_getcity(sg, 0, &game, (char **)&worldPtr)) {
			UIResetViewable();
		}
		PostLoadGame();
		DrawGame(1);
		MapHasJumped();
		savegame_close(sg);
	}
}

/*!
 * \brief open a savegame
 * \param sel unused
 * \param data the filename from the selection dialog
 */
static void
open_afile(GtkObject *sel __attribute__((unused)), gpointer data)
{
	GtkWidget *dat = (GtkWidget *)data;

	doOpen((gchar *)gtk_file_selection_get_filename(
		    GTK_FILE_SELECTION(dat)));
}

/*!
 * \brief free the widget and the data pointer
 * \param obj unused
 * \param data the object to destroy
 */
static void
free_object(GtkObject *obj __attribute__((unused)), gpointer data)
{
	GtkWidget *dat = (GtkWidget *)data;
	gtk_widget_destroy(dat);
}

/*!
 * \brief open a game file from the system
 * \todo save any game in progress
 */
void
opengame_handler(void)
{
	GtkWidget *file_sel = gtk_file_selection_new(
	    "Select saved game to open");

	g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_sel)->ok_button),
	    "clicked", G_CALLBACK(open_afile), (gpointer)file_sel);

	g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_sel)->ok_button),
	    "clicked", G_CALLBACK(free_object), (gpointer)file_sel);
	g_signal_connect(
	    GTK_OBJECT(GTK_FILE_SELECTION(file_sel)->cancel_button),
	    "clicked", G_CALLBACK(free_object), (gpointer)file_sel);

	gtk_widget_show(GTK_WIDGET(file_sel));
}

/*! \brief new game form entities */
static struct ng_form_tag {
	GtkWidget *form; /*!< form */
	GtkWidget *cityName; /*!< City's name */
	GtkWidget *foo; /*!< foo? */
} ng;

/*!
 * \brief deal with the closing of the newgame form
 * \param widget unused
 * \param data unused
 */
static gint
close_newgame(GtkWidget *widget __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	ng.form = NULL;
	return (FALSE);
}

/*!
 * \todo save the game before starting a new one
 */
void
newgame_handler(void)
{
	GtkWidget *table, *mainbox; 
	GtkWidget *label;

	/* SetupNewGame(); */
	/* UIResetViewable(); */
	/* setLoopSeconds(SPEED_PAUSED); */
	ng.form = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(ng.form), "Create New City");

	mainbox = gtk_vbox_new(FALSE, 10);
	table = gtk_table_new(12, 4, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(mainbox), 3);

	label = create_right_label("City Name:");
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);

	ng.cityName = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), ng.cityName, 1, 4, 0, 1,
	    GTK_FILL | GTK_EXPAND, 0, 0, 0);

	/* Width/height */

	/* Add all the choices */


	g_signal_connect(G_OBJECT(ng.form), "delete_event",
	    G_CALLBACK(close_newgame), 0);

	gtk_container_add(GTK_CONTAINER(ng.form), mainbox);
	gtk_box_pack_start(GTK_BOX(mainbox), table, TRUE, TRUE, 0);

	gtk_widget_show_all(ng.form);
}

/*!
 * \brief set the save game name
 * \param sel unused
 * \param data the file selection dialog that caused this
 */
void
store_filename(GtkWidget *sel __attribute__((unused)), gpointer data)
{
	gchar *name = (gchar*)gtk_file_selection_get_filename(data);

	if (name != NULL) {
		if (-1 == setCityFileName(name)) {
			return;
		}
	} else
		return;
	WriteLog("This game will be saved as %s from now on\n", name);
	save_defaultfilename();
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

	save_defaultfilename();
}
