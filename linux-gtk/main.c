/*! \file
 * \brief the main GTK routines for the simulation
 *
 * This file contains all the major routines to perform the simulation
 * in the GTK environment.
 */
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <main.h>
#include <savegame_fe.h>
#include <budget.h>
#include <ui.h>
#include <handler.h>
#include <drawing.h>
#include <globals.h>
#include <build.h>
#include <simulation.h>
#include <disaster.h>
#include <compilerpragmas.h>

/*! \brief handle to area on which the game is drawn */
static GtkWidget *drawingarea;
/*! \brief the handle of the window within which most of the widgets reside */
static GtkWidget *window;
/*! \brief the handle to the label containing the amount of money you have */
static GtkWidget *creditslabel;
/*! \brief the handle to the label containing the population in the city */
static GtkWidget *poplabel;
/*! \brief the handle to the label containing the date in the city */
static GtkWidget *timelabel;
/*! \brief handle to the horizontal scroll bar for moving the map around */
static GtkObject *playscrollerh;
/*! \brief handle to the vertical scroll bar for moving the map around */
static GtkObject *playscrollerv;
/*! \brief handle to the bitmaps for rendering the zones */
static GdkPixmap *pzones;
/*! \brief handle to the bitmaps for rendering the monsters */
static GdkPixmap *pmonsters;
/*! \brief handle to the bitmaps for rendering the units */
static GdkPixmap *punits;
/*! \brief handle to the bitmap masks for rendering the units */
static GdkPixmap *zones_mask;
/*! \brief handle to the bitmap masks for rendering the monsters */
static GdkPixmap *monsters_mask;
/*! \brief handle to the bitmap masks for rendering the units */
static GdkPixmap *units_mask;

static char *exec_dir;

static void SetUpMainWindow(void);
static gint mainloop_callback(gpointer data);
static void QuitGame(GtkWidget *w, gpointer data);
static void SetSpeed(GtkWidget *w, gpointer data);

/*! \brief the menu items for the main application */
GtkItemFactoryEntry menu_items[] = {
	{ "/_File", NULL, NULL, 0, "<Branch>", 0 },
	{ "/File/_New", "<control>N", NewGame, 0, NULL, 0 },
	{ "/File/_Open", "<control>O", OpenGame, 0, NULL, 0 },
	{ "/File/_Open Palm Game", NULL, OpenGame, 1, NULL, 0 },
	{ "/File/_Save", "<control>S", SaveGame, 0, NULL, 0 },
	{ "/File/Save _As", NULL, SaveGameAs, 0, NULL, 0 },
	{ "/File/sep1",	NULL, NULL, 0, "<Separator>", 0 },
	{ "/File/_Quit", NULL, QuitGame, 0, NULL, 0 },
	{ "/_View", NULL, NULL,	0, "<Branch>", 0 },
	{ "/View/_Budget", NULL, ViewBudget, 0, NULL, 0 },
	{ "/_Speed", NULL, NULL, 0, "<Branch>", 0 },
	{ "/Speed/_Pause", NULL, SetSpeed, SPEED_PAUSED, NULL, 0 },
	{ "/Speed/sep1", NULL, NULL, 0, "<Separator>", 0 },
	{ "/Speed/_Slow", NULL, SetSpeed, SPEED_SLOW, NULL, 0 },
	{ "/Speed/_Medium", NULL, SetSpeed, SPEED_MEDIUM, NULL, 0 },
	{ "/Speed/_Fast", NULL, SetSpeed, SPEED_FAST, NULL, 0 },
	{ "/Speed/_Turbo", NULL, SetSpeed, SPEED_TURBO, NULL, 0 },
};

/*!
 * \brief the main routine
 *
 * Sets up the windows and starts the simulation
 * \param argc count of arguments on the command line
 * \param argv array of the arguments
 */
int
main(int argc, char *argv[])
{
	gint timerID;
	char *px;
	
	exec_dir = strdup(argv[0]);
	px = strrchr(exec_dir, '/');
	if (px != NULL) {
		*px = '\0';
	} else {
		/* Trouble at't mine, is it windows? */
		g_print("Windows??\n");
	}

	gtk_init(&argc, &argv);
	srand(time(0));

	SetUpMainWindow();
	/* start the timer */
	timerID = g_timeout_add(1000, (mainloop_callback), 0);

	gtk_main();
	WriteLog("Cleaning up\n");
	g_source_remove(timerID);
	PurgeWorld();
	free(exec_dir);
	
	return (0);
}

/* \brief the ticker */
unsigned int timekeeper = 0;
/* \brief the disaster time clock */
unsigned int timekeeperdisaster = 0;

/*!
 * \brief set the game speed
 * \param w the widget that originated the speed message
 * \param data the speed number.
 */
static void
SetSpeed(GtkWidget *w __attribute__((unused)), gpointer data)
{
	int result = GPOINTER_TO_INT(data);

	WriteLog("Setting speed to %i\n", result);
	setLoopSeconds(result);
}

/*!
 * \brief time ticker that loops every second
 * \param data unused in this context
 * \return true, call me again in a second
 */
static gint
mainloop_callback(gpointer data __attribute__((unused)))
{
	/* this will be called every second */
	unsigned int phase = 1;

	timekeeper++;
	timekeeperdisaster++;

	if (timekeeperdisaster >= SIM_GAME_LOOP_DISASTER) {
		MoveAllObjects();
		if (UpdateDisasters()) {
			gtk_widget_queue_draw(drawingarea);
		}
	}

	if (timekeeper >= getLoopSeconds() &&
	    getLoopSeconds() != SPEED_PAUSED) {
		WriteLog("A month has gone by - total months: %lu\n",
		    (unsigned long)getMonthsElapsed());
		timekeeper = 0;
		do {
			phase = Sim_DoPhase(phase);
		} while (phase != 0);
		gtk_widget_queue_draw(drawingarea);
	}

	UIUpdateBudget();
	return (TRUE); /* yes, call us again in a sec */
}

/*! \brief handle to the bitmap masks for rendering the units */
static UInt8 selectedBuildItem = 0;

/*!
 * \brief set the item that will be build upon clicking on the play surface
 * \param widget unused
 * \param data the identifier of the toolbar item
 * \return false, handled
 */
static gint
toolbox_callback(GtkWidget *widget __attribute__((unused)), gpointer data)
{
	selectedBuildItem = GPOINTER_TO_INT(data);
	return (FALSE);
}

/*!
 * \brief slide the scrollbar
 *
 * Moves the display to the appropriate location.
 * \param adj the adjustment; unused
 */
void
scrollbar(GtkAdjustment *adj __attribute__((unused)))
{
	Goto(GTK_ADJUSTMENT(playscrollerh)->value,
	    GTK_ADJUSTMENT(playscrollerv)->value);
}

/*!
 * \brief check the size of the screen.
 *
 * intended to allow the play area to expand and contract; this code is
 * used by the initial sizing algorithm, as well as the resize handler.
 * \todo fix this to constrain the width/height so as to not be too big
 * \param width the new width of the area
 * \param height the new height of the area
 */
void
ResizeCheck(int width, int height)
{
	vgame.tileSize = 16;
	vgame.visible_x = width / vgame.tileSize;
	vgame.visible_y = height / vgame.tileSize;
}

/*!
 * \brief the configure event handler for the play area.
 *
 * Resizes the playing area elements to ensure that the play area is
 * displayed with the correct size on screen.
 * \param widget the widget that caused the configure event.
 * \param event te event that isssued this configure event
 * \param data extra data for the event.
 */
void
check_configure(GtkContainer *widget __attribute__((unused)), GdkEvent *event,
    gpointer data __attribute__((unused)))
{
	GdkEventConfigure *gek = (GdkEventConfigure *)event;
	ResizeCheck(gek->width, gek->height);
}

/*!
 * \brief when the close button is clicked, this event is called
 *
 * MAkes the main event loop terminate.
 * \param widget unused
 * \param event unused
 * \param data unused
 * \return FALSE, to stop the event being processed again
 * \todo Change the code to perform an auto save / ask to save.
 */
static gint
delete_event(GtkWidget *widget __attribute__((unused)),
    GdkEvent *event __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	gtk_main_quit();
	return (FALSE);
}

/*!
 * \brief the drawing area is exposed, and needs to be painted.
 *
 * This is an expose event for the drawing area to cause the screen to
 * be repainted. Redraws all the fields that are on the screen.
 * \param widget unused
 * \param event unused
 * \param data unused
 * \return FALSE, to make sure tnothing else tries to handle the signal
 * \todo repaint only the section of the screen that need painting.
 */
static gint
drawing_exposed_callback(GtkWidget *widget __attribute__((unused)),
    GdkEvent *event __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	RedrawAllFields();

	return (FALSE);
}

/*!
 * \brief Realize the drawing area once it is created
 *
 * This call is issued once the drawing area is initially created.
 * It allows us to create a new game and make sure the screen is the
 * correct size.
 * \param widget unused
 * \param event unused
 * \param data unused
 * \return FALSE, make sure that no one else reacts to the signal.
 */
static gint
drawing_realized_callback(GtkWidget *widget __attribute__((unused)),
    GdkEvent *event __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	PCityMain();
	NewGame(NULL, 0);
	ResizeCheck(320, 240);
	return (FALSE);
}

/*!
 * \brief someone clicked on the play area.
 *
 * The play area has a button pressed upon it. As a consequence we need to
 * either build something or bulldoze the area.
 * \param widget unused
 * \param event contains the buttons that were pressed.
 */
static gint
button_press_event(GtkWidget *widget __attribute__((unused)),
    GdkEventButton *event)
{
	if (event->button == 1) {
		BuildSomething(
		    (int)(event->x / vgame.tileSize) + getMapXPos(),
		    (int)(event->y / vgame.tileSize) + getMapYPos());
	} else if (event->button == 3) {
		Build_Bulldoze(
		    (int)(event->x / vgame.tileSize) + getMapXPos(),
		    (int)(event->y / vgame.tileSize) + getMapYPos(), 0);
	}

	return (TRUE);
}

/*!
 * \brief somethings moved
 *
 * Ooooooooo.... I have no clue
 */
static gint
motion_notify_event(GtkWidget *widget __attribute__((unused)),
    GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;

	if (event->is_hint) {
		gdk_window_get_pointer(event->window, &x, &y, &state);
	} else {
		x = event->x;
		y = event->y;
		state = event->state;
	}

	if (state & GDK_BUTTON1_MASK &&
	    x > 0 && x < vgame.visible_x*vgame.tileSize &&
	    y > 0 && y < vgame.visible_y*vgame.tileSize) {
		BuildSomething(
		    (int)(x / vgame.tileSize) + getMapXPos(),
		    (int)(y / vgame.tileSize) + getMapYPos());
	} else if (state & GDK_BUTTON3_MASK &&
	    x > 0 && x < vgame.visible_x*vgame.tileSize &&
	    y > 0 && y < vgame.visible_y*vgame.tileSize) {
		Build_Bulldoze(
		    (int)(x / vgame.tileSize) + getMapXPos(),
		    (int)(y / vgame.tileSize) + getMapYPos(), 1);
	}

	return (TRUE);
}

/*!
 * \brief set up the toolbox
 * \return the widget containing the toolbox
 */
GtkWidget *
setupToolBox(void)
{
	GtkWidget *button_image;
	GtkTooltips *tips;
	GtkWidget *toolbox;
	GtkWidget *button;
	GtkWidget *handle;
	int i;
	char image_path[MAXPATHLEN];
	/* If you change the order here you need to change the xpm... */
	/*! \todo make the file names related to the items */
	struct gaa {
		gint entry; const char *text;
	} actions[] = {
		{ Be_Bulldozer, "Bulldozer" },
		{ Be_Road, "Road" },
		{ Be_Power_Line, "Power Line" },
		{ Be_Zone_Residential, "Residential" },
		{ Be_Zone_Commercial, "Commercial" },
		{ Be_Zone_Industrial, "Industrial" },
		{ Be_Tree, "Tree" },
		{ Be_Water, "Water" },
		{ Be_Water_Pipe, "Water Pipe" },
		{ Be_Power_Plant, "Power Plant" },
		{ Be_Nuclear_Plant, "Nuclear Power Plant" },
		{ Be_Water_Pump, "Water Pump" },
		{ Be_Fire_Station, "Fire Station" },
		{ Be_Police_Station, "Police Station" },
		{ Be_Military_Base, "Military Base" },
		{ -1, NULL }, { -1, NULL }, { -1, NULL },
		{ Be_Defence_Fire, "Fire Brigade" },
		{ Be_Defence_Police, "Police Car" },
		{ Be_Defence_Military, "Tank" },
		{ -1, NULL }, { -1, NULL }, { -1, NULL },
		{ -1, NULL }, { -1, NULL }, { -1, NULL },
		{ -1, NULL }, { -1, NULL }, { -1, NULL }
	};

	tips = gtk_tooltips_new();

	toolbox = gtk_table_new(9, 3, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(toolbox), 3);

	for (i = 0; i < 30; i++) {
		if (actions[i].entry == -1)
			continue;

		button = gtk_button_new();
		sprintf(image_path, "%s/graphic/icons/interface_%02i.png",
			exec_dir, i);
		button_image = gtk_image_new_from_file(image_path);
		gtk_container_add(GTK_CONTAINER(button), button_image);
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button,
		    actions[i].text, "test2");
		g_signal_connect(G_OBJECT(button), "clicked",
		    G_CALLBACK(toolbox_callback),
		GINT_TO_POINTER(actions[i].entry));
		gtk_table_attach_defaults(GTK_TABLE(toolbox), button,
		    (i%3), (i%3)+1, (i/3), (i/3)+1);
	}

	handle = gtk_handle_box_new();
	gtk_handle_box_set_handle_position(
	    (GtkHandleBox *)handle, GTK_POS_TOP);
	gtk_container_add(GTK_CONTAINER(handle), toolbox);
	return (handle);
}

/*!
 * \brief create the keyboard and menu accelerator groups
 * \return the accelerator group.
 */
GtkAccelGroup *
createMenu(GtkWidget *main_box)
{
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

	accel_group = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
	    accel_group);
	gtk_item_factory_create_items(item_factory, nmenu_items,
	    menu_items, NULL);

	gtk_box_pack_start(GTK_BOX(main_box),
	    gtk_item_factory_get_widget(item_factory, "<main>"),
	    FALSE, TRUE, 0);

	return (accel_group);
}

/*!
 * \brief set up and configure the main window.
 *
 * Creates the main window. Creates all the elements of the main window
 * including the toolbar (!grrrrr)
 */
void
SetUpMainWindow(void)
{
	GtkWidget *fieldbox, *box, *toolbox, *headerbox;
	GtkWidget *playingbox, *main_box;
	GtkAccelGroup *accel_group;
	GtkWidget *hscroll, *vscroll;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Pocket City");
	g_signal_connect(G_OBJECT(window), "delete_event",
	    G_CALLBACK(delete_event), NULL);

	main_box = gtk_vbox_new(FALSE, 0);
	box = gtk_hbox_new(FALSE, 0);
	fieldbox = gtk_vbox_new(FALSE, 0);
	headerbox = gtk_hbox_new(FALSE, 0);
	playingbox = gtk_table_new(2, 2, FALSE);

	gtk_container_add(GTK_CONTAINER(window), main_box);

	creditslabel = gtk_label_new("Credits");
	poplabel = gtk_label_new("Population");
	timelabel = gtk_label_new("Game Time");

	/* the actual playfield is a GtkDrawingArea */
	drawingarea = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawingarea, 320, 240);
	/* and some scrollbars for the drawingarea */
	playscrollerh = gtk_adjustment_new(60, 10, 110, 1, 10, 20);
	playscrollerv = gtk_adjustment_new(57, 10, 107, 1, 10, 15);
	hscroll = gtk_hscrollbar_new(GTK_ADJUSTMENT(playscrollerh));
	vscroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(playscrollerv));
	g_signal_connect(G_OBJECT(playscrollerh), "value_changed",
	    G_CALLBACK(scrollbar), NULL);
	g_signal_connect(G_OBJECT(playscrollerv), "value_changed",
	    G_CALLBACK(scrollbar), NULL);

	gtk_table_attach(GTK_TABLE(playingbox), drawingarea,
	    0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
	gtk_table_attach(GTK_TABLE(playingbox), hscroll, 0, 1, 1, 2,
	    GTK_FILL, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(playingbox), vscroll, 1, 2, 0, 1,
	    0, GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(drawingarea), "configure_event",
	    G_CALLBACK(check_configure), NULL);

	/* arange in boxes  */
	toolbox = setupToolBox();
	gtk_box_pack_end(GTK_BOX(main_box), box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), toolbox, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), fieldbox, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(fieldbox), headerbox, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(fieldbox), playingbox, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(fieldbox), poplabel, FALSE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(headerbox), creditslabel, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(headerbox), timelabel, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(drawingarea), "expose_event",
	    G_CALLBACK(drawing_exposed_callback), NULL);

	g_signal_connect_after(G_OBJECT(drawingarea), "realize",
	    G_CALLBACK(drawing_realized_callback), NULL);

	/* set up some mouse events */
	gtk_signal_connect(GTK_OBJECT(drawingarea), "motion_notify_event",
	    (GtkSignalFunc) motion_notify_event, NULL);
	gtk_signal_connect(GTK_OBJECT(drawingarea), "button_press_event",
	    (GtkSignalFunc) button_press_event, NULL);

	gtk_widget_set_events(drawingarea, GDK_EXPOSURE_MASK |
	    GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
	    GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

	accel_group = createMenu(main_box);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	/* show all the widgets */
	gtk_widget_show_all(main_box);

	/* finally, show the main window */
	gtk_widget_show(window);
}

/*!
 * \brief load and configure the pixmaps
 */
void
UISetUpGraphic(void)
{
	char image_path[MAXPATHLEN];
	struct stat sbuf;
	
	sprintf(image_path, "%s/graphic/tile-16x16-color.png", exec_dir);
	if (stat(image_path, &sbuf) == -1) {
		perror(image_path);
		exit(1);
	}
	pzones = gdk_pixmap_create_from_xpm(drawingarea->window,
	    &zones_mask, NULL, image_path);
	sprintf(image_path, "%s/graphic/monsters_16x16-color.png", exec_dir);
	if (stat(image_path, &sbuf) == -1) {
		perror(image_path);
		exit(1);
	}
	pmonsters = gdk_pixmap_create_from_xpm(drawingarea->window,
	    &monsters_mask, NULL, image_path);
	sprintf(image_path, "%s/graphic/units_16x16-color.png", exec_dir);
	if (stat(image_path, &sbuf) == -1) {
		perror(image_path);
		exit(1);
	}
	punits = gdk_pixmap_create_from_xpm(drawingarea->window,
	    &units_mask, NULL, image_path);
}

/*!
 * \brief Display and error dialog with one parameter.
 * \param error the message to use in the dialog
 */
void
UIDisplayError1(char *error)
{
	GtkWidget * dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_ERROR,
	    GTK_BUTTONS_OK,
	    "%s",
	    error);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

/*!
 * \brief display and error involving a specific error type
 * \param nError the error number to invoke.
 */
void
UIDisplayError(erdiType nError)
{
	char temp[100];

	switch (nError) {
	case enOutOfMemory:
		strcpy(temp, "Out of memory");
		break;
	case enOutOfMoney:
		strcpy(temp, "Out of money");
		break;
	case diFireOutbreak:
		strcpy(temp, "An Australian fire has broken out somewhere!");
		break;
	case diPlantExplosion:
		strcpy(temp, "A power plant just exploded!");
		break;
	case diMonster:
		strcpy(temp, "Godzilla just came to town!");
		break;
	case diDragon:
		strcpy(temp, "A fire dragon wants to use your city as "
		    "it's lair!");
		break;
	case diMeteor:
		strcpy(temp, "A gigantic meteor has hit your city!");
		break;
	default:
		strcpy(temp, "An unknown error/disaster?");
		break;
	}
	UIDisplayError1(temp);
}

/*! unused */
void
UIInitDrawing(void)
{
}

/*! unused */
void
UIFinishDrawing(void)
{
}

/*! unused */
void
UIUnlockScreen(void)
{
	/* not used for this platform */
}

/*! unused */
void
UILockScreen(void)
{
	/* not used for this platform */
}

/*! unused */
void
UIDrawBorder(void)
{
	/* */
}

/*!
 * \brief set the labels for the credits and time
 */
void
UIDrawCredits(void)
{
	char temp[23];

	sprintf(temp, "$: %ld", (long)getCredits());
	gtk_label_set_text((GtkLabel *)creditslabel, temp);
	GetDate((char *)temp);
	gtk_label_set_text((GtkLabel *)timelabel, temp);
}

/*! unused */
void
UIUpdateBuildIcon(void)
{
	/* */
}

/*! unused */
void
UIGotoForm(Int16 n __attribute__((unused)))
{
	/* */
}

/*! unused */
void
UICheckMoney(void)
{
	/* */
}

/*!
 * \brief scroll the map in the approprite direction
 *
 * \todo double buffer to improve the rendering of the screen!
 */
void
UIScrollMap(dirType direction __attribute__((unused)))
{
	/* TODO: Optimize this as in the Palm port? */
	/*	   (if nessasary) */
	RedrawAllFields();
}

/*!
 * \brief in theory draw a rectangle.
 */
void
_UIDrawRect(Int16 nTop __attribute__((unused)),
    Int16 nLeft __attribute__((unused)), Int16 nHeight __attribute__((unused)),
    Int16 nWidth __attribute__((unused)))
{
	WriteLog("_UIDrawRect\n");
}

/*!
 * \brief draw a field
 * \param xpos the horizontal position
 * \param ypos the vertical position
 * \param nGraphic the item to paint
 */
void
UIDrawField(Int16 xpos, Int16 ypos, welem_t nGraphic)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    pzones,
	    (nGraphic % HORIZONTAL_TILESIZE) * vgame.tileSize,
	    (nGraphic / HORIZONTAL_TILESIZE) * vgame.tileSize,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	g_object_unref(gc);
}

/*!
 * \param draw a special object
 * \param i the item to draw
 * \param xpos the horizontal position
 * \param ypos the vertical position
 */
void
UIDrawSpecialObject(Int16 i, Int16 xpos, Int16 ypos)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_gc_set_clip_mask(gc, monsters_mask);
	gdk_gc_set_clip_origin(gc,
	    xpos * vgame.tileSize - (GG.objects[i].dir * vgame.tileSize),
	    ypos * vgame.tileSize - (i * vgame.tileSize));

	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    pmonsters,
	    GG.objects[i].dir * vgame.tileSize,
	    i * vgame.tileSize,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	g_object_unref(gc);
}

/*!
 * \brief draw a special unit on the screen
 * \param i the unit do draw
 * \param xpos the horizontal location on screen
 * \param ypos the vertical location on screen
 */
void
UIDrawSpecialUnit(Int16 i, Int16 xpos, Int16 ypos)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_gc_set_clip_mask(gc, units_mask);
	gdk_gc_set_clip_origin(gc,
	    xpos * vgame.tileSize - (GG.units[i].type * vgame.tileSize),
	    ypos * vgame.tileSize);

	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    punits,
	    GG.units[i].type * vgame.tileSize,
	    0,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	g_object_unref(gc);
}

/*! unused */
void
UIDrawCursor(Int16 xpos __attribute__((unused)),
    Int16 ypos __attribute__((unused)))
{
	/* not used on this platform */
}

/*!
 * \brief draw an overlay icon on the screen
 * \param xpos the horizontal location
 * \param ypos the vertical location
 * \param offset the offst of the item to paint.
 */
void
UIDrawOverlay(Int16 xpos, Int16 ypos, welem_t offset)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_gc_set_clip_mask(gc, zones_mask);
	gdk_gc_set_clip_origin(gc,
	    (xpos - (offset % HORIZONTAL_TILESIZE)) * vgame.tileSize,
	    (ypos - (offset / HORIZONTAL_TILESIZE)) * vgame.tileSize);

	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    pzones,
	    (offset % HORIZONTAL_TILESIZE) * vgame.tileSize,
	    (offset / HORIZONTAL_TILESIZE) * vgame.tileSize,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	g_object_unref(gc);
}

/*!
 * \brief Draw a power loss overlay on the screen at the specified location
 */
void
UIDrawPowerLoss(Int16 xpos, Int16 ypos)
{
	UIDrawOverlay(xpos, ypos, Z_POWER_OUT);
}

/*!
 * \brief Draw a water loss overlay on the screen at the specified location
 */
void
UIDrawWaterLoss(Int16 xpos, Int16 ypos)
{
	UIDrawOverlay(xpos, ypos, Z_WATER_OUT);
}

/*!
 * \brief get the selected item to build
 * \return the item that is selected.
 */
BuildCode
UIGetSelectedBuildItem(void)
{
	return (selectedBuildItem);
}

/*! unused */
void
LockWorld(void)
{
	/* not used on this platform */
}

/*! unused */
void
UnlockWorld(void)
{
	/* not used on this platform */
}

/*!
 * \brief get a random number between 0 and max
 * \param max the maximum limit of the value to obtain
 * \return the random number
 */
UInt32
GetRandomNumber(UInt32 max)
{
	/* se `man 3 rand` why I'm not using: return (rand() % max) */
	return ((UInt32)((float)max*rand()/(RAND_MAX+1.0)));
}

/*!
 * \brief the map has jumped to another location
 *
 * Ensure that the scroll bars are oriented correctly.
 */
void
MapHasJumped(void)
{
	((GtkAdjustment *)playscrollerh)->value = getMapXPos()+10;
	((GtkAdjustment *)playscrollerv)->value = getMapYPos()+7;
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(playscrollerh));
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(playscrollerv));
}

/*!
 * \brief unused
 */
void
UISetTileSize(Int16 size __attribute__((unused)))
{
	WriteLog("UISetTileSize\n");
}

/*!
 * \brief draw the population
 *
 * Actually simply set the population label to the appropriate value.
 */
void
UIDrawPop(void)
{
	char temp[50];
	sprintf(temp, "(%02u, %02u) Population: %-9li",
	    getMapXPos(), getMapYPos(), (long)getPopulation());

	gtk_label_set_text((GtkLabel*)poplabel, temp);
}

/*!
 * \brief terminate the game.
 *
 * Invoked when the quit option is selected from teh menu
 * \param w unused
 * \param data unused
 */
void
QuitGame(GtkWidget *w __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	gtk_main_quit();
}

#ifdef DEBUG

#include <stdarg.h>

/*!
 * \brief write output to the console
 * \param s the string for formatting
 */
void
WriteLog(char *s, ...)
{
	va_list args;
	char mbuf[2048];

	va_start(args, s);
	vsprintf(mbuf, s, args);
	g_print(mbuf);
	va_end(args);
}
#endif
