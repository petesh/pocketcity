#include <gtk/gtk.h>
#include <stdio.h>
#include "../source/simulation.h"
#include "main.h"

GtkWidget *dlg=0;
GtkWidget *lres,*lcom,*lind,*lpow,*ltra,*ldef,*lbal,*lcha,*lnex;

extern void UIUpdateBudget(void)
{
    char temp[20];
    if (!dlg) { return; }

    sprintf(temp,"%li", BudgetGetNumber(BUDGET_RESIDENTIAL));
    gtk_label_set_text((GtkLabel*)lres,temp);
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_COMMERCIAL));
    gtk_label_set_text((GtkLabel*)lcom,temp);
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_INDUSTRIAL));
    gtk_label_set_text((GtkLabel*)lind,temp);
    
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_POWER));
    gtk_label_set_text((GtkLabel*)lpow,temp);
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_TRAFFIC));
    gtk_label_set_text((GtkLabel*)ltra,temp);
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_DEFENCE));
    gtk_label_set_text((GtkLabel*)ldef,temp);
    
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_CURRENT_BALANCE));
    gtk_label_set_text((GtkLabel*)lbal,temp);
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_CHANGE));
    gtk_label_set_text((GtkLabel*)lcha,temp);
    sprintf(temp,"%li", BudgetGetNumber(BUDGET_NEXT_MONTH));
    gtk_label_set_text((GtkLabel*)lnex,temp);

}

gint budget_ok(GtkWidget *widget, gpointer data)
{
    gtk_object_destroy(GTK_OBJECT(dlg));
    dlg = 0;
    return FALSE;
}

gint close_budget_cancel(GtkWidget *widget, gpointer data)
{
    dlg = 0;
    return FALSE;
}

GtkWidget * create_left_label(char * s)
{
    GtkWidget * l;
    l = gtk_label_new(s);
    gtk_misc_set_alignment(GTK_MISC(l),0,0.5);
    return l;
}

GtkWidget * create_right_label(void)
{
    GtkWidget * l;
    l = gtk_label_new("-");
    gtk_misc_set_alignment(GTK_MISC(l),1,0.5);
    return l;
}

extern void UIViewBudget(GtkWidget *w, gpointer data)
{
    GtkWidget *table, *mainbox;
    GtkWidget *button;

    dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//    gtk_window_set_modal(GTK_WINDOW(dlg),1);
    gtk_window_set_title(GTK_WINDOW(dlg), "Pocket City Budget");

    mainbox = gtk_vbox_new(FALSE,10);
    table = gtk_table_new(12,3,TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(mainbox),3);

    button = gtk_button_new_with_label("OK");
    g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(budget_ok), 0);
    g_signal_connect(G_OBJECT(dlg),"delete_event", G_CALLBACK(close_budget_cancel), 0);

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

    // now the moneyflow
    lres = create_right_label();
    lcom = create_right_label();
    lind = create_right_label();
    lpow = create_right_label();
    ltra = create_right_label();
    ldef = create_right_label();
    lbal = create_right_label();
    lcha = create_right_label();
    lnex = create_right_label();
    gtk_table_attach_defaults(GTK_TABLE(table),lres,2,3,1,2);
    gtk_table_attach_defaults(GTK_TABLE(table),lcom,2,3,2,3);
    gtk_table_attach_defaults(GTK_TABLE(table),lind,2,3,3,4);
    gtk_table_attach_defaults(GTK_TABLE(table),ltra,2,3,6,7);
    gtk_table_attach_defaults(GTK_TABLE(table),lpow,2,3,7,8);
    gtk_table_attach_defaults(GTK_TABLE(table),ldef,2,3,8,9);
    gtk_table_attach_defaults(GTK_TABLE(table),lbal,2,3,11,12);
    gtk_table_attach_defaults(GTK_TABLE(table),lcha,2,3,12,13);
    gtk_table_attach_defaults(GTK_TABLE(table),lnex,2,3,13,14);
    
    UIUpdateBudget();

    gtk_widget_show_all(dlg);
}

