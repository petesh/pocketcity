#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <main.h>
#include <savegame.h>
#include <globals.h>
#include <handler.h>
#include <ui.h>
#include <simulation.h>

gchar * savegamename;

void
UIResetViewable(void)
{
    vgame.tileSize = 16;
    vgame.visible_x = 320/ vgame.tileSize;
    vgame.visible_y = 240/ vgame.tileSize;
}

void open_filename(GtkFileSelection *sel, gpointer data)
{
    int fd, ret;
    char tempversion[4];
    
    savegamename = (gchar*)gtk_file_selection_get_filename(
      GTK_FILE_SELECTION(data));
    g_print("Opening save game from %s\n", savegamename);
    
    fd = open(savegamename, O_RDONLY);
    if (fd == -1) {
        perror("open"); /* TODO: make this nicer */
        return;
    }
    /* first of all, check the savegame version */
    ret = read(fd, (void*)tempversion, 4);
    if (ret == -1) {
        perror("read version"); /* TODO: make this nicer */
        return;
    }
    if (strncmp(tempversion, SAVEGAMEVERSION, 4) != 0) {
        g_print("Wrong save game format - aborting\n");
        return;
    }
    /* version was ok, rewind the file */
    lseek(fd, 0, SEEK_SET);
    
    /* God, I love to read all my vars at one time */
    /* using a struct :D ... unfortunately horribly unportable */
    ret = read(fd, (void *)&game, sizeof(GameStruct));
    if (ret == -1) {
        perror("read game"); /* TODO: make this nicer */
        return;
    } else if (ret != sizeof(GameStruct)) {
        g_print("Whoops, couldn't read full length of game\n");
        return;
    }

    /* and now the great worldPtr :D */
    ret = read(fd, (void *)worldPtr, GetMapMul());
    if (ret == -1) {
        perror("read world"); /* TODO: make this nicer */
        return;
    } else if (ret != GetMapMul()) {
        g_print("Whoops, couldn't read full lenght of world\n");
        return;
    }
    
    if (close(fd) == -1) {
        perror("close"); /* TODO: make this nicer */
        return;
    }

    /* update the screen with the new game */
    UIResetViewable();
    PostLoadGame();
    DrawGame(1);
    MapHasJumped();
}

extern void UIOpenGame(GtkWidget *w, gpointer data)
{
    GtkWidget *fileSel;

    fileSel = gtk_file_selection_new("Select saved game to open");
    g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
            "clicked", G_CALLBACK(open_filename), (gpointer)fileSel);

    g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
            "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);
    g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->cancel_button),
            "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);
            
    gtk_widget_show(GTK_WIDGET(fileSel));
}

extern void UINewGame(GtkWidget *w, gpointer data)
{
    SetupNewGame();
    UIResetViewable();
    game.gameLoopSeconds = SPEED_FAST;
}


void store_filename(GtkFileSelection *sel, gpointer data)
{
    savegamename = (gchar*)gtk_file_selection_get_filename(GTK_FILE_SELECTION(data));
    g_print("This game will be saved as %s from now on\n", savegamename);
    UISaveGame(NULL,0);
}

extern void UISaveGameAs(GtkWidget *w, gpointer data)
{
    GtkWidget *fileSel;

    fileSel = gtk_file_selection_new("Save game as...");
    g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
            "clicked", G_CALLBACK(store_filename), (gpointer)fileSel);

    g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->ok_button),
            "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);
    g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fileSel)->cancel_button),
            "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fileSel);
            
    gtk_widget_show(GTK_WIDGET(fileSel));
}

extern void UISaveGame(GtkWidget *w, gpointer data)
{
    int fd, ret;
    
    if (savegamename == NULL) {
        UISaveGameAs(NULL, 0);
        return;
    }
    g_print("Saving game as %s...\n", savegamename);

    fd = open(savegamename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open"); /* TODO: make this nicer */
        return;
    }
    /* God, I love to write all my vars at one time */
    /* using a struct :D */
    ret = write(fd, (void*)&game, sizeof(GameStruct));
    if (ret == -1) {
        perror("write game"); /* TODO: make this nicer */
        return;
    } else if (ret != sizeof(GameStruct)) {
        g_print("Whoops, couldn't write full length of game\n");
        return;
    }

    /* and now the great worldPtr :D */
    ret = write(fd, (void*)worldPtr, GetMapMul());
    if (ret == -1) {
        perror("write world"); /* TODO: make this nicer */
        return;
    } else if (ret != GetMapMul()) {
        g_print("Whoops, couldn't write full length of world\n");
        return;
    }
    
    if (close(fd) == -1) {
        perror("close"); /* TODO: make this nicer */
        return;
    }
}
