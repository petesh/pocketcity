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

gchar *savegamename;

void
UIResetViewable(void)
{
	vgame.tileSize = 16;
	/* XXX: set based on size of window */
	vgame.visible_x = 320 / vgame.tileSize;
	vgame.visible_y = 240 / vgame.tileSize;
}

static void
doOpen(GtkFileSelection *sel, gpointer data, int palm)
{
	savegamename = (gchar*)gtk_file_selection_get_filename(
	    GTK_FILE_SELECTION(data));
	if (0 == open_filename(savegamename, palm)) {
		UIResetViewable();
		PostLoadGame();
		DrawGame(1);
		MapHasJumped();
	}
}

void
open_palmfilename(GtkFileSelection *sel, gpointer data)
{
	doOpen(sel, data, 1);
}

void
open_platfilename(GtkFileSelection *sel, gpointer data)
{
	doOpen(sel, data, 0);
}

void
UIOpenGame(GtkWidget *w, gpointer data)
{
	GtkWidget *fileSel;

	fileSel = gtk_file_selection_new("Select saved game to open");
	g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
	    "clicked", G_CALLBACK(open_palmfilename), (gpointer)fileSel);

	g_signal_connect_swapped(
	    GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
	    "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);
	g_signal_connect_swapped(
	    GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->cancel_button),
	    "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);

	gtk_widget_show(GTK_WIDGET(fileSel));
}

void
UINewGame(GtkWidget *w, gpointer data)
{
	SetupNewGame();
	UIResetViewable();
	game.gameLoopSeconds = SPEED_FAST;
}


void
store_filename(GtkFileSelection *sel, gpointer data)
{
	savegamename = (gchar*)gtk_file_selection_get_filename(
	    GTK_FILE_SELECTION(data));
	WriteLog("This game will be saved as %s from now on\n", savegamename);
	UISaveGame(NULL, 0);
}

void
UISaveGameAs(GtkWidget *w, gpointer data)
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
UISaveGame(GtkWidget *w, gpointer data)
{
	if (savegamename == NULL) {
		UISaveGameAs(NULL, 0);
		return;
	}

	save_filename(savegamename, 0);
}
