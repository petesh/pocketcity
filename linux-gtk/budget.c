#include <gtk/gtk.h>

GtkWidget * create_left_label(char * s)
{
    GtkWidget * l;
    l = gtk_label_new(s);
    gtk_misc_set_alignment(GTK_MISC(l),0,0);
    return l;
}

extern void UIViewBudget(GtkWidget *w, gpointer data)
{
    GtkWidget * dlg;
    GtkWidget *table, *mainbox;
    GtkWidget *button;

    dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(dlg), "Pocket City Budget");

    mainbox = gtk_vbox_new(FALSE,10);
    table = gtk_table_new(12,3,TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(mainbox),3);

    button = gtk_button_new_with_label("OK");

    gtk_container_add(GTK_CONTAINER(dlg), mainbox);
    gtk_box_pack_start(GTK_BOX(mainbox), table,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(mainbox), button,TRUE,TRUE,0);
    // layout the labels
    gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new("Income"),0,3,0,1);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Residential"),0,1,1,2);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Commercial"),0,1,2,3);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Industrial"),0,1,3,4);

    gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new("Expenses"),0,3,5,6);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Traffic"),0,3,6,7);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Power"),0,3,7,8);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Defence"),0,3,8,9);
    
    gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new("Total"),0,3,10,11);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Current balance"),0,2,11,12);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Change"),0,2,12,13);
    gtk_table_attach_defaults(GTK_TABLE(table),create_left_label("Next month's balance"),0,2,13,14);
    

    gtk_widget_show_all(dlg);
}

