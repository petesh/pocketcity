/*! \file
 * \brief This file corresponds to the budget form.
 * 
 * This is the gtk implementation of the budget form
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <simulation.h>
#include <zakdef.h>
#include <globals.h>
#include <main.h>
#include <compilerpragmas.h>

/*! \brief handle for the budget dialog */
static GtkWidget *dlg = 0;

/*! \brief label containing the residential income */
static GtkWidget *lres;
/*! \brief label containing the commercial income */
static GtkWidget *lcom;
/*! \brief label containing the industrial income */
static GtkWidget *lind;
/*! \brief label containing the power cost */
static GtkWidget *lpow;
/*! \brief label containing the traffic cost */
static GtkWidget *ltra;
/*! \brief label containing the defence cost */
static GtkWidget *ldef;
/*! \brief label containing the budget opening balance */
static GtkWidget *lbal;
/*! \brief label containing the budget delta (income - costs) */
static GtkWidget *lcha;
/*! \brief label containing next months values (balance - change) */
static GtkWidget *lnex;
/*! \brief traffic percentage adjustment */
static GtkObject *adjust_tra;
/*! \brief power percentage adjustment */
static GtkObject *adjust_pow;
/*! \brief defence percentage adjustment */
static GtkObject *adjust_def;

/*!
 * \brief update each of the budget fields
 */
void
UIUpdateBudget(void)
{
	char temp[20];

	if (!dlg)
		return;

	sprintf(temp, "%li", (long)BudgetGetNumber(bnResidential));
	gtk_label_set_text((GtkLabel*)lres, temp);
	sprintf(temp, "%li", (long)BudgetGetNumber(bnCommercial));
	gtk_label_set_text((GtkLabel*)lcom, temp);
	sprintf(temp, "%li", (long)BudgetGetNumber(bnIndustrial));
	gtk_label_set_text((GtkLabel*)lind, temp);

	sprintf(temp, "%li", (long)BudgetGetNumber(bnPower));
	gtk_label_set_text((GtkLabel*)lpow, temp);
	sprintf(temp, "%li", (long)BudgetGetNumber(bnTraffic));
	gtk_label_set_text((GtkLabel*)ltra, temp);
	sprintf(temp, "%li", (long)BudgetGetNumber(bnDefence));
	gtk_label_set_text((GtkLabel*)ldef, temp);

	sprintf(temp, "%li", (long)BudgetGetNumber(bnCurrentBalance));
	gtk_label_set_text((GtkLabel*)lbal, temp);
	sprintf(temp, "%li", (long)BudgetGetNumber(bnChange));
	gtk_label_set_text((GtkLabel*)lcha, temp);
	sprintf(temp, "%li", (long)BudgetGetNumber(bnNextMonth));
	gtk_label_set_text((GtkLabel*)lnex, temp);

}

/*!
 * \brief Adjust the traffic percentage
 * \param adj the adjustment widget that caused this change
 */
static void
budget_traffic(GtkAdjustment *adj)
{
	char temp[20];

	setUpkeep(ue_traffic, gtk_adjustment_get_value(GTK_ADJUSTMENT(adj)));
	sprintf(temp, "%li", (long)BudgetGetNumber(bnTraffic));
	gtk_label_set_text((GtkLabel*)ltra, temp);
}

/*!
 * \brief Adjust the power percentage
 * \param adj the adjustment widget that caused this change
 */
static void
budget_power(GtkAdjustment *adj)
{
	char temp[20];

	setUpkeep(ue_power, gtk_adjustment_get_value(GTK_ADJUSTMENT(adj)));
	sprintf(temp, "%li", (long)BudgetGetNumber(bnPower));
	gtk_label_set_text((GtkLabel*)lpow, temp);
}

/*!
 * \brief Adjust the percentage allocation of defence
 * \param adj the adjustment widget that caused this change
 */
static void
budget_defence(GtkAdjustment *adj)
{
	char temp[20];

	setUpkeep(ue_defense, gtk_adjustment_get_value(GTK_ADJUSTMENT(adj)));
	sprintf(temp, "%li", (long)BudgetGetNumber(bnDefence));
	gtk_label_set_text((GtkLabel*)ldef, temp);
}

/*!
 * \brief Handler for closing the budget dialog.
 *
 * Clears the dlg flag to prevent the accidental writing of values into the
 * form fields.
 *
 * \param widget unused
 * \param data unused
 * \return always returns false(0) to allow the form to close.
 */
static gint
close_budget(GtkWidget *widget __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	dlg = NULL;
	return (FALSE);
}

/*!
 * \brief create a left aligned label
 * \param s the string to put into the label
 * \return the newly created label
 */
static GtkWidget *
create_left_label(char *s)
{
	GtkWidget *l;

	l = gtk_label_new(s);
	gtk_misc_set_alignment(GTK_MISC(l), 0, 0.5);
	return (l);
}

/*!
 * \brief create a right aligned label with the content "-"
 * \return the newly created right laligned label
 */
static GtkWidget *
create_right_label(void)
{
	GtkWidget * l;

	l = gtk_label_new("-");
	gtk_misc_set_alignment(GTK_MISC(l), 1, 0.5);
	return (l);
}

/*!
 * \brief View the budget form
 *
 * This is the GTK handler for displaying the budget form.
 * \param w owner widget (unused)
 * \param data unused
 */
void
ViewBudget(GtkWidget *w __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	GtkWidget *table, *mainbox;
	GtkWidget *spinner;

	dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dlg), "Pocket City Budget");

	mainbox = gtk_vbox_new(FALSE, 10);
	table = gtk_table_new(12, 3, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(mainbox), 3);

	g_signal_connect(G_OBJECT(dlg), "delete_event",
	    G_CALLBACK(close_budget), 0);

	gtk_container_add(GTK_CONTAINER(dlg), mainbox);
	gtk_box_pack_start(GTK_BOX(mainbox), table, TRUE, TRUE, 0);
	/* layout the labels */
	gtk_table_attach_defaults(GTK_TABLE(table),
	    gtk_label_new("Income"), 0, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Residential"), 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Commercial"), 0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Industrial"), 0, 1, 3, 4);

	gtk_table_attach_defaults(GTK_TABLE(table),
	    gtk_label_new("Expenses"), 0, 3, 5, 6);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Traffic"), 0, 3, 6, 7);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Power"), 0, 3, 7, 8);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Defence"), 0, 3, 8, 9);

	gtk_table_attach_defaults(GTK_TABLE(table),
	    gtk_label_new("Total"), 0, 3, 10, 11);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Current balance"), 0, 2, 11, 12);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Change"), 0, 2, 12, 13);
	gtk_table_attach_defaults(GTK_TABLE(table),
	    create_left_label("Next month's balance"), 0, 2, 13, 14);

	/* now the moneyflow */
	lres = create_right_label();
	lcom = create_right_label();
	lind = create_right_label();
	lpow = create_right_label();
	ltra = create_right_label();
	ldef = create_right_label();
	lbal = create_right_label();
	lcha = create_right_label();
	lnex = create_right_label();
	gtk_table_attach_defaults(GTK_TABLE(table), lres, 2, 3, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), lcom, 2, 3, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), lind, 2, 3, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(table), ltra, 2, 3, 6, 7);
	gtk_table_attach_defaults(GTK_TABLE(table), lpow, 2, 3, 7, 8);
	gtk_table_attach_defaults(GTK_TABLE(table), ldef, 2, 3, 8, 9);
	gtk_table_attach_defaults(GTK_TABLE(table), lbal, 2, 3, 11, 12);
	gtk_table_attach_defaults(GTK_TABLE(table), lcha, 2, 3, 12, 13);
	gtk_table_attach_defaults(GTK_TABLE(table), lnex, 2, 3, 13, 14);

	/* and the adjustments */
	adjust_tra = gtk_adjustment_new(getUpkeep(ue_traffic), 0, 100,
	    1, 10, 0);
	spinner = gtk_spin_button_new(GTK_ADJUSTMENT(adjust_tra), 0.1, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), spinner, 1, 2, 6, 7);
	g_signal_connect(G_OBJECT(spinner), "value_changed",
	    G_CALLBACK(budget_traffic), NULL);

	adjust_pow = gtk_adjustment_new(getUpkeep(ue_power), 0, 100, 1,
	    10, 0);
	spinner = gtk_spin_button_new(GTK_ADJUSTMENT(adjust_pow), 0.1, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), spinner, 1, 2, 7, 8);
	g_signal_connect(G_OBJECT(spinner), "value_changed",
	    G_CALLBACK(budget_power), NULL);

	adjust_def = gtk_adjustment_new(getUpkeep(ue_defense), 0, 100,
	    1, 10, 0);
	spinner = gtk_spin_button_new(GTK_ADJUSTMENT(adjust_def), 0.1, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), spinner, 1, 2, 8, 9);
	g_signal_connect(G_OBJECT(spinner), "value_changed",
	    G_CALLBACK(budget_defence), NULL);

	UIUpdateBudget();

	gtk_widget_show_all(dlg);
}
