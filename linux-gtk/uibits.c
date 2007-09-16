/*!
 * \file
 * \brief implementation of the ui misc routines
 *
 * Bits and bobs for the ui. does things like create aligned labels
 * and such. Anything that needed to be placed in a simple routine to
 * avoid repetition of UI code should go here.
 *
 * In theory I should be using glade to do the UI, but as most of my
 * programming takes place on the windows platform, glade is not present.
 * it would probably not be that much trouble to get it up and running
 * though. Maybe I should try?
 */

#include <uibits.h>

GtkWidget *
create_left_label(char *s)
{
	GtkWidget *l;

	l = gtk_label_new(s);
	gtk_misc_set_alignment(GTK_MISC(l), 0, 0.5);
	return (l);
}

GtkWidget *
create_right_label(char *string)
{
	GtkWidget * l;

	l = gtk_label_new(string);
	gtk_misc_set_alignment(GTK_MISC(l), 1, 0.5);
	return (l);
}
