/*! \file
 * This file is for displaying tile information as you hover over individual
 * zones.
 *
 * It's the equivalent of the query form, but operates on hover.
 */

#include <gtk/gtk.h>
#include <zakdef.h>
#include <stdio.h>
#include <globals.h>
#include <locking.h>
#include <zonemon.h>
#include <simulation.h>
#include <zonestrings.h>
#include <uibits.h>

/*! \brief contents of the hover window */
static struct tag_things {
	GtkWidget	*win;	/*!< window pointer */
	GtkWidget	*name;	/*!< Name of zone */
	GtkWidget	*value;	/*!< zone value */
	GtkWidget	*density;	/*!< zone density */
	GtkWidget	*pollution; /*!< zone pollution */
	GtkWidget	*crime; /*!< crime label */
	GtkWidget	*power;	/*!< need power label */
	GtkWidget	*painted;
	GtkWidget	*water;	/*!< need water label */

	UInt16 old_xpos;		/*!< old xposition */
	UInt16 old_ypos;		/*!< oly y position */
} ttg;

/*!
 * \brief update the hover location
 * \param xpos the xposition we're now over
 * \param ypos the y position we're now over
 * \param force forcibly update the output
 */
void
hoverUpdate(UInt16 xpos, UInt16 ypos, int force)
{
	welem_t	world = 0;
	selem_t	status = 0;
	char text[1024];

	if (ttg.win == NULL)
		return;

	LockZone(lz_world);
	LockZone(lz_flags);
	getWorldAndFlag(WORLDPOS(xpos, ypos), &world, &status);
	UnlockZone(lz_flags);
	UnlockZone(lz_world);

	if ((ttg.old_xpos == xpos) && (ttg.old_ypos == ypos)) 
		if (!force) return;

	ttg.old_xpos = xpos;
	ttg.old_ypos = ypos;

	sprintf(text, "Status (%d, %d)", (int)xpos, (int)ypos);
	gtk_window_set_title(GTK_WINDOW(ttg.win), text);
	if (-1 == getFieldString(world, text, 1024)) {
		sprintf(text, "type == %x(?)", (int)world);
	}
	gtk_label_set_text(GTK_LABEL(ttg.name), text);
	(void) getFieldValue(world, text, 1024);
	gtk_label_set_text(GTK_LABEL(ttg.value), text);
	(void) getFieldDensity(world, text, 1024);
	gtk_label_set_text(GTK_LABEL(ttg.density), text);
	gtk_widget_hide(ttg.power);
	if (CarryPower(world)) {
		if (status & POWEREDBIT)
			gtk_label_set_text(GTK_LABEL(ttg.power), "Pow+");
		else
			gtk_label_set_text(GTK_LABEL(ttg.power), "-Pow");
		gtk_widget_show(ttg.power);
	}
	gtk_widget_hide(ttg.water);
	if (CarryWater(world)) {
		if (status & WATEREDBIT)
			gtk_label_set_text(GTK_LABEL(ttg.water), "Wat+");
		else
			gtk_label_set_text(GTK_LABEL(ttg.water), "-Wat");
		gtk_widget_show(ttg.water);
	}
	if (status & PAINTEDBIT)
		gtk_label_set_text(GTK_LABEL(ttg.painted), "HAS");
	else
		gtk_label_set_text(GTK_LABEL(ttg.painted), "---");
}

static gint
hoverClose(GtkWidget *wid __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	ttg.win = NULL;
	return (FALSE);
}

void
hoverShow(void)
{
	GtkLabel *lab;
	GtkTable *table;

	if (ttg.win != NULL) {
		gtk_widget_show(ttg.win);
		return;
	}

	ttg.win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(ttg.win), "Status");
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(ttg.win), TRUE);
	g_signal_connect(G_OBJECT(ttg.win), "delete_event",
	    G_CALLBACK(hoverClose), NULL);

	table = GTK_TABLE(gtk_table_new(6, 3, TRUE));
	lab = GTK_LABEL(create_right_label("Type:"));
	gtk_table_attach(table, GTK_WIDGET(lab), 0, 1, 0, 1,
	    GTK_FILL, GTK_FILL, 0, 0);
	ttg.name = create_left_label("-");
	gtk_table_attach(table, ttg.name, 1, 3, 0, 1, GTK_FILL, GTK_FILL,
	    0, 0);

	lab = GTK_LABEL(create_right_label("Value:"));
	gtk_label_set_justify(lab, GTK_JUSTIFY_RIGHT);
	gtk_table_attach(table, GTK_WIDGET(lab), 0, 1, 1, 2,
    	    GTK_FILL, GTK_FILL, 0, 0);
	ttg.value = create_left_label("-");
	gtk_table_attach(table, ttg.value, 1, 3, 1, 2, GTK_FILL, GTK_FILL,
	    0, 0);

	lab = GTK_LABEL(create_right_label("Density:"));
	gtk_table_attach(table, GTK_WIDGET(lab), 0, 1, 2, 3,
	    GTK_FILL, GTK_FILL, 0, 0);
	ttg.density = create_left_label("-");
	gtk_table_attach(table, ttg.density, 1, 3, 2, 3,
	    GTK_FILL, GTK_FILL, 0, 0);

	lab = GTK_LABEL(create_right_label("Pollution:"));
	gtk_table_attach(table, GTK_WIDGET(lab), 0, 1, 3, 4,
	    GTK_FILL, GTK_FILL, 0, 0);
	ttg.pollution = create_left_label("-");
	gtk_table_attach(table, ttg.pollution, 1, 3, 3, 4,
	    GTK_FILL, GTK_FILL, 0, 0);

	lab = GTK_LABEL(create_right_label("Crime:"));
	gtk_table_attach(table, GTK_WIDGET(lab), 0, 1, 4, 5,
	    GTK_FILL, GTK_FILL, 0, 0);
	ttg.crime = create_left_label("-");
	gtk_table_attach(table, ttg.crime, 1, 3, 4, 5,
	    GTK_FILL, GTK_FILL, 0, 0);

	ttg.power = gtk_label_new("-");
	gtk_table_attach(table, ttg.power, 0, 1, 5, 6,
	    GTK_EXPAND, GTK_FILL, 0, 0);

	ttg.painted = gtk_label_new("-");
	gtk_table_attach(table, ttg.painted, 1, 2, 5, 6,
	    GTK_EXPAND, GTK_FILL, 0, 0);
	ttg.water = gtk_label_new("-");
	gtk_table_attach(table, ttg.water, 2, 3, 5, 6,
	    GTK_EXPAND, GTK_FILL, 0, 0);

	gtk_container_add(GTK_CONTAINER(ttg.win), GTK_WIDGET(table));

	gtk_window_set_policy(GTK_WINDOW(ttg.win), FALSE, TRUE, FALSE);
	gtk_widget_show_all(GTK_WIDGET(table));
	gtk_widget_show(ttg.win);
}

