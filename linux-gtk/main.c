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
#include <unistd.h>
#include <sys/stat.h>

#include <main.h>
#include <savegame_fe.h>
#include <budget.h>
#include <logging.h>
#include <locking.h>
#include <ui.h>
#include <handler.h>
#include <drawing.h>
#include <globals.h>
#include <build.h>
#include <simulation.h>
#include <disaster.h>
#include <compilerpragmas.h>
#include <nix_utils.h>
#include <simulation-ui.h>
#include <zonemon.h>

/*! \brief path to search for graphics */
#define PATHSEARCH	".:./graphic:./graphic/icons:../graphic"

#define	MILLISECS	1000
#define	TICKPERSEC	10

/*! \brief the main window's contents */
static struct main_window {
	GtkWidget *window; /*!< \brief handle of the window */
	GtkWidget *drawing; /*!< \brief handle to game play area */
	GtkWidget *l_credits; /*!< \brief label of the amount of money */
	GtkWidget *l_location; /*!< \brief location on map */
	GtkWidget *l_pop; /*!< \brief population label */
	GtkWidget *l_time; /*!< \brief simulation date label */

	GtkObject *sc_hor; /*!< \brief horizontal scroll adjustment */
	GtkObject *sc_vert; /*!< \brief vertical scroll adjustment */

	GtkWidget *hscroll; /*!< \brief horizontal scroll bar */
	GtkWidget *vscroll; /*!< \brief vertical scroll bar */

	GdkPixmap *p_zones; /*!< \brief zone pixmap */
	GdkPixmap *p_monsters; /*!< \brief monsters pixmap */
	GdkPixmap *p_units; /*!< \brief units pixmap */

	GdkPixmap *p_zones_m; /*!< \brief mask for units */
	GdkPixmap *p_monsters_m; /*!< \brief mask for the monsters */
	GdkPixmap *p_units_m; /*!< \brief mask for the units */

	GdkPixmap *p_mapzones; /*!< \brief pixmap of the map zones */
	GdkPixmap *p_mapspecials; /*!< \brief pixmap of specials for map */
	GdkPixmap *p_mapunits; /*!< \brief pixmap for the units on map */

	GdkPixmap *p_play; /*!< \brief the play area (full) */
} mw;

/*! \brief the execution directory of the program */
static char *exec_dir;

static void SetUpMainWindow(void);
static gint mainloop_callback(gpointer data);
static void QuitGame(void);
static void SetSpeed(gpointer data, guint action, GtkWidget *w);
static void cleanupPixmaps(void);
static void ResetViewable(void);
static void ShowMainWindow(void);
static void forceRedistribute(void);
static void doRepaintDisplay(void);

/*! \brief the menu items for the main application */
const GtkItemFactoryEntry menu_items[] = {
	{ "/_File", NULL, NULL, 0, "<Branch>", 0 },
	{ "/File/_New", "<control>N", newgame_handler, 0, NULL, 0 },
	{ "/File/_Open", "<control>O", opengame_handler, 0, NULL, NULL },
	{ "/File/_Save", "<control>S", savegame_handler, 0, NULL, 0 },
	{ "/File/Save _As", NULL, savegameas_handler, 0, NULL, 0 },
	{ "/File/sep1",	NULL, NULL, 0, "<Separator>", 0 },
	{ "/File/E_xit", "<alt>F4", QuitGame, 0, NULL, 0 },
	{ "/_View", NULL, NULL,	0, "<Branch>", 0 },
	{ "/View/_Budget", "<control>B", ViewBudget, 0, NULL, 0 },
	{ "/View/_Map", "<control>M", showMap, 0, NULL, 0 },
	{ "/View/_Hover", NULL, hoverShow, 0, NULL, 0 },
	{ "/_Speed", NULL, NULL, 0, "<Branch>", 0 },
	{ "/Speed/_Pause", "<control>0", SetSpeed, 1 + SPEED_PAUSED, NULL,
		NULL },
	{ "/Speed/sep1", NULL, NULL, 0, "<Separator>", 0 },
	{ "/Speed/_Slow", "<control>1", SetSpeed, 1 + SPEED_SLOW, NULL, NULL },
	{ "/Speed/_Medium", "<control>2", SetSpeed, 1 + SPEED_MEDIUM, NULL,
		NULL },
	{ "/Speed/_Fast", "<control>3", SetSpeed, 1 + SPEED_FAST, NULL, NULL },
	{ "/Speed/_Turbo", "<control>4", SetSpeed, 1 + SPEED_TURBO, NULL,
		NULL },
	{ "/S_imulation", NULL, NULL, 0, "<Branch>", 0 },
	{ "/Simulation/_Redistribute", NULL, forceRedistribute, 0, NULL, NULL }

};
#define NMENU_ITEMS	(sizeof (menu_items) / sizeof (menu_items[0]))

/*!
 * \brief the main routine
 *
 * Sets up the windows and starts the simulation
 * \param argc count of arguments on the command line
 * \param argv array of the arguments
 */
int
main(int argc, char **argv)
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
	srand(time(NULL));

	ResetViewable();
	SetUpMainWindow();

	PCityMain();
	InitGameStruct();
	ConfigureNewGame();

	ShowMainWindow();

	/* start the timer */
	timerID = g_timeout_add(MILLISECS / TICKPERSEC, (mainloop_callback), 0);

	gtk_main();
	WriteLog("Cleaning up\n");
	g_source_remove(timerID);
	PurgeWorld();
	free(exec_dir);
	PCityShutdown();
	cleanupMap();
	cleanupPixmaps();

	return (0);
}

GtkWidget *
window_main_get(void)
{
	return (mw.window);
}

GdkDrawable *
drawable_main_get(void)
{
	return (mw.window->window);
}

/* \brief the ticker */
unsigned int timekeeper = 0;
/* \brief the disaster time clock */
unsigned int timekeeperdisaster = 0;

/*!
 * \brief set the game speed
 * \param w the widget that originated the speed message
 * \param speed the speed
 * \param data the speed number.
 */
static void
SetSpeed(gpointer data __attribute__((unused)), guint speed,
    GtkWidget *w __attribute__((unused)))
{
	speed -= 1;
	WriteLog("Setting speed to %i\n", speed);
	setLoopSeconds(speed);
}

/*!
 * \brief time ticker that loops every second
 * \param data unused in this context
 * \return true, call me again in a second
 */
static gint
mainloop_callback(gpointer data __attribute__((unused)))
{
	/* this will be called 10 times every second */
	unsigned int phase = 1;

	timekeeper++;
	timekeeperdisaster++;

	if ((timekeeperdisaster / TICKPERSEC) >= SIM_GAME_LOOP_DISASTER) {
		MoveAllObjects();
		if (UpdateDisasters()) {
			gtk_widget_queue_draw(mw.drawing);
		}
	}

	doRepaintDisplay();

	if ((timekeeper / TICKPERSEC) >= getLoopSeconds() &&
	    getLoopSeconds() != SPEED_PAUSED) {
		WriteLog("A month has gone by - total months: %lu\n",
		    (unsigned long)getMonthsElapsed());
		timekeeper = 0;
		do {
			phase = Sim_DoPhase(phase);
		} while (phase != 0);
		gtk_widget_queue_draw(mw.drawing);
	}

	UIUpdateBudget();
	return (TRUE); /* yes, call us again */
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

/*! \brief set the tile size */
static void
ResetViewable(void)
{
	/*! \todo set based on size of window/tiles */
	setGameTileSize(16);
	setMapTileSize(4);
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
	Goto(GTK_ADJUSTMENT(mw.sc_hor)->value,
	    GTK_ADJUSTMENT(mw.sc_vert)->value, goto_center);
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
	GtkAdjustment *adjh = GTK_ADJUSTMENT(mw.sc_hor);
	GtkAdjustment *adjv = GTK_ADJUSTMENT(mw.sc_vert);
	setVisibleX(width / gameTileSize());
	setVisibleY(height / gameTileSize());
	adjh->lower = getVisibleX() / 2;
	adjh->upper = getMapWidth() + adjh->lower;
	adjv->lower = getVisibleY() / 2;
	adjv->upper = getMapHeight() + adjh->lower;
	if (adjh->value > adjh->upper) adjh->value = adjh->upper;
	if (adjv->value > adjv->upper) adjv->value = adjh->upper;

	WriteLog("visx = %d, visy = %d\n", getVisibleX(), getVisibleY());
	WriteLog("hor: lower = %d, upper = %d\n", (int)adjh->lower,
	    (int)adjh->upper);
	WriteLog("ver: lower = %d, upper = %d\n", (int)adjv->lower,
	    (int)adjv->upper);
	gtk_adjustment_changed(adjh);
	gtk_adjustment_changed(adjv);
	gtk_adjustment_value_changed(adjh);
	gtk_adjustment_value_changed(adjv);
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
check_configure(GtkContainer *widget __attribute__((unused)),
    GdkEventConfigure *event,
    gpointer data __attribute__((unused)))
{
	ResizeCheck(event->width, event->height);
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
 * \return FALSE, to make sure nothing else tries to handle the signal
 */
static gint
drawing_exposed_callback(GtkWidget *widget,
    GdkEventExpose *event,
    gpointer data __attribute__((unused)))
{
	GdkGC *gc = gdk_gc_new(widget->window);

	//WriteLog("drawing: (%d,%d)\n", event->area.width, event->area.height);

	gdk_draw_drawable(
	    widget->window,
	    gc,
	    mw.p_play,
	    event->area.x + (getMapXPos() * gameTileSize()),
	    event->area.y + (getMapYPos() * gameTileSize()),
	    event->area.x,
	    event->area.y,
	    event->area.width,
	    event->area.height);
	g_object_unref(gc);
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
		    (int)(event->x / gameTileSize()) + getMapXPos(),
		    (int)(event->y / gameTileSize()) + getMapYPos());
	} else if (event->button == 3) {
		Build_Bulldoze(
		    (int)(event->x / gameTileSize()) + getMapXPos(),
		    (int)(event->y / gameTileSize()) + getMapYPos(), 0);
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
	    x > 0 && x < getVisibleX() * gameTileSize() &&
	    y > 0 && y < getVisibleY() * gameTileSize()) {
		BuildSomething(
		    (int)(x / gameTileSize()) + getMapXPos(),
		    (int)(y / gameTileSize()) + getMapYPos());
	} else if (state & GDK_BUTTON3_MASK &&
	    x > 0 && x < getVisibleX() * gameTileSize() &&
	    y > 0 && y < getVisibleY() * gameTileSize()) {
		Build_Bulldoze(
		    (int)(x / gameTileSize()) + getMapXPos(),
		    (int)(y / gameTileSize()) + getMapYPos(), 1);
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
	//GtkTooltips *tips;
	GtkWidget *toolbox;
	//GtkWidget *button;
	//GtkWidget *handle;
	unsigned int i;
	char *image_path;
	size_t max_path = (size_t)pathconf("/", _PC_PATH_MAX) + 1;
	/* If you change the order here you need to change the xpm... */
	/*! \todo make the file names related to the items */
	const struct gaa {
		gint entry;
		const char *text;
		const char *file;
	} actions[] = {
		{ Be_Bulldozer, "Bulldozer", "interface_00.png" },
		{ Be_Road, "Road", "interface_01.png" },
		{ Be_Power_Line, "Power Line", "interface_02.png" },
		{ Be_Zone_Residential, "Residential", "interface_03.png" },
		{ Be_Zone_Commercial, "Commercial", "interface_04.png" },
		{ Be_Zone_Industrial, "Industrial", "interface_05.png" },
		{ Be_Tree, "Tree", "interface_06.png" },
		{ Be_Water, "Water", "interface_07.png" },
		{ Be_Water_Pipe, "Water Pipe", "interface_08.png" },
		{ Be_Power_Plant, "Power Plant", "interface_09.png" },
		{ Be_Nuclear_Plant, "Nuclear Power Plant", "interface_10.png" },
		{ Be_Water_Pump, "Water Pump", "interface_11.png" },
		{ Be_Fire_Station, "Fire Station", "interface_12.png" },
		{ Be_Police_Station, "Police Station", "interface_13.png" },
		{ Be_Military_Base, "Military Base", "interface_14.png" },
		{ -1, NULL, NULL },
		{ Be_Defence_Fire, "Fire Brigade", "interface_18.png" },
		{ Be_Defence_Police, "Police Car", "interface_19.png" },
		{ Be_Defence_Military, "Tank", "interface_20.png" }
	};

#define SIZE_ACTIONS	(sizeof (actions) / sizeof (actions[0]))

	image_path = malloc(max_path);

	//tips = gtk_tooltips_new();

	//toolbox = gtk_table_new(9, 3, TRUE);
	toolbox = gtk_toolbar_new();
	gtk_container_set_border_width(GTK_CONTAINER(toolbox), 0);

	for (i = 0; i < SIZE_ACTIONS; i++) {
		if (actions[i].entry == -1) {
			gtk_toolbar_append_space(GTK_TOOLBAR(toolbox));
			continue;
		}

		//button = gtk_button_new();
		strcpy(image_path, actions[i].file);
		if (searchForFile(image_path, max_path, PATHSEARCH))
			button_image = gtk_image_new_from_file(image_path);
		else {
			perror(image_path);
			exit(1);
		}
		gtk_toolbar_append_item(GTK_TOOLBAR(toolbox),
		    NULL, actions[i].text, NULL, button_image,
		    G_CALLBACK(toolbox_callback),
		    GINT_TO_POINTER(actions[i].entry));
		/*gtk_container_add(GTK_CONTAINER(button), button_image);
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button,
		    actions[i].text, NULL);
		g_signal_connect(G_OBJECT(button), "clicked",
		    G_CALLBACK(toolbox_callback),
		GINT_TO_POINTER(actions[i].entry));*/
		//gtk_table_attach_defaults(GTK_TABLE(toolbox), button,
		//    (i%3), (i%3)+1, (i/3), (i/3)+1);
	}

	/*handle = gtk_handle_box_new();
	gtk_handle_box_set_handle_position(
	    (GtkHandleBox *)handle, GTK_POS_TOP);
	gtk_container_add(GTK_CONTAINER(handle), toolbox);*/

	free(image_path);
	return (toolbox);
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

	accel_group = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
	    accel_group);
	gtk_item_factory_create_items(item_factory, NMENU_ITEMS,
	    (GtkItemFactoryEntry *)menu_items, NULL);

	gtk_box_pack_start(GTK_BOX(main_box),
	    gtk_item_factory_get_widget(item_factory, "<main>"),
	    FALSE, TRUE, 0);

	return (accel_group);
}

/*!
 * \brief hovering over the drawing area
 * \param widget unused
 * \param event unused
 * \param data unused
 */
static gboolean
hoveringDrawing(GtkWidget *widget __attribute__((unused)),
    GdkEventMotion *event, gpointer data __attribute__((unused)))
{
	hoverUpdate((event->x / gameTileSize()) + getMapXPos(),
	    (event->y / gameTileSize()) + getMapYPos(), 0);
	return (0);
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

	mw.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mw.window), "Pocket City");
	g_signal_connect(G_OBJECT(mw.window), "delete_event",
	    G_CALLBACK(delete_event), NULL);

	main_box = gtk_vbox_new(FALSE, 0);
	box = gtk_vbox_new(FALSE, 0);
	fieldbox = gtk_vbox_new(FALSE, 0);
	headerbox = gtk_hbox_new(FALSE, 0);
	playingbox = gtk_table_new(2, 2, FALSE);

	gtk_container_add(GTK_CONTAINER(mw.window), main_box);

	mw.l_credits = gtk_label_new("Credits");
	mw.l_location = gtk_label_new("Location");
	mw.l_pop = gtk_label_new("Population");
	mw.l_time = gtk_label_new("Game Time");

	/* the actual playfield is a Gtkmw.drawing */
	mw.drawing = gtk_drawing_area_new();
	gtk_widget_set_size_request(mw.drawing, 320, 320);
	/* and some scrollbars for the mw.drawing */
	mw.sc_hor = gtk_adjustment_new(50, 0, 100, 1, 10, 20);
	mw.sc_vert = gtk_adjustment_new(50, 0, 100, 1, 10, 15);
	mw.hscroll = gtk_hscrollbar_new(GTK_ADJUSTMENT(mw.sc_hor));
	mw.vscroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(mw.sc_vert));
	g_signal_connect(G_OBJECT(mw.sc_hor), "value_changed",
	    G_CALLBACK(scrollbar), NULL);
	g_signal_connect(G_OBJECT(mw.sc_vert), "value_changed",
	    G_CALLBACK(scrollbar), NULL);

	gtk_table_attach(GTK_TABLE(playingbox), mw.drawing,
	    0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
	gtk_table_attach(GTK_TABLE(playingbox), mw.hscroll, 0, 1, 1, 2,
	    GTK_FILL, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(playingbox), mw.vscroll, 1, 2, 0, 1,
	    0, GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(mw.drawing), "configure_event",
	    G_CALLBACK(check_configure), NULL);

	/* arange in boxes  */
	toolbox = setupToolBox();
	gtk_box_pack_end(GTK_BOX(main_box), box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), toolbox, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), fieldbox, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(fieldbox), headerbox, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(fieldbox), playingbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(fieldbox), mw.l_location, FALSE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(fieldbox), mw.l_pop, FALSE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(headerbox), mw.l_credits, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(headerbox), mw.l_time, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(mw.drawing), "expose_event",
	    G_CALLBACK(drawing_exposed_callback), NULL);

	g_signal_connect_after(G_OBJECT(mw.drawing), "realize",
	    G_CALLBACK(drawing_realized_callback), NULL);

	g_signal_connect(G_OBJECT(mw.drawing), "motion_notify_event",
	    G_CALLBACK(hoveringDrawing), NULL);

	/* set up some mouse events */
	gtk_signal_connect(GTK_OBJECT(mw.drawing), "motion_notify_event",
	    G_CALLBACK(motion_notify_event), NULL);
	gtk_signal_connect(GTK_OBJECT(mw.drawing), "button_press_event",
	    G_CALLBACK(button_press_event), NULL);

	gtk_widget_set_events(mw.drawing, GDK_EXPOSURE_MASK |
	    GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
	    GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

	accel_group = createMenu(main_box);
	gtk_window_add_accel_group(GTK_WINDOW(mw.window), accel_group);

	/* show all the widgets */
	gtk_widget_show_all(main_box);

	gtk_widget_realize(mw.window);
}

static void
ShowMainWindow(void)
{
	/* finally, show the main window */
	gtk_widget_show(mw.window);
}

/*! \brief image pixmaps */
static struct image_pms {
	char *filename; /*!< Filename of the image file */
	GdkPixmap **pm; /*!< Pixmap to place the image into */
	GdkPixmap **mask; /*!< Mask for transparency of the image */
} image_pixmaps[] = {
	{ "tile-16x16-color.png", &mw.p_zones, &mw.p_zones_m },
	{ "monsters_16x16-color.png", &mw.p_monsters, &mw.p_monsters_m },
	{ "units_16x16-color.png", &mw.p_units, &mw.p_units_m },
	{ "tile-4x4-color.png", &mw.p_mapzones, NULL },
	{ "monsters_16x16-color.png", &mw.p_mapspecials, NULL },
	{ "units_16x16-color.png", &mw.p_mapunits, NULL },
	{ NULL, NULL, NULL }
};

/*!
 * \brief clean up the pixmaps
 */
static void
cleanupPixmaps(void)
{
	int elt;

	if (mw.p_play != NULL) g_object_unref(G_OBJECT(mw.p_play));
	for (elt = 0; image_pixmaps[elt].filename != NULL; elt++) {
		if (image_pixmaps[elt].pm != NULL &&
			*image_pixmaps[elt].pm != NULL)
			g_object_unref(G_OBJECT(*image_pixmaps[elt].pm));
		if (image_pixmaps[elt].mask != NULL &&
			*image_pixmaps[elt].mask != NULL)
			g_object_unref(G_OBJECT(*image_pixmaps[elt].mask));

	}
}

/*!
 * \brief load and configure the pixmaps
 */
void
UIInitGraphic(void)
{
	char *image_path;
	int i;
	struct image_pms *ipm;
	size_t max_path = (size_t)pathconf("/", _PC_PATH_MAX) + 1;

	image_path = malloc(max_path);

	for (i = 0; image_pixmaps[i].filename != NULL; i++) {
		ipm = image_pixmaps + i;
		strncpy(image_path, ipm->filename, max_path - 1);
		if (searchForFile(image_path, max_path, PATHSEARCH)) {
			*ipm->pm = gdk_pixmap_create_from_xpm(
			    mw.window->window, ipm->mask, NULL, image_path);
			if (*ipm->pm == NULL) {
				WriteLog("Could not create pixmap from file %s",
				    ipm->filename);
				free(image_path);
				exit(1);
			}
		} else {
			perror(image_path);
			free(image_path);
			exit(1);
		}
	}
	/* load the icon */
	strncpy(image_path, "pcityicon.png", max_path - 1);
	if (searchForFile(image_path, max_path, PATHSEARCH)) {
		gtk_window_set_icon_from_file(GTK_WINDOW(mw.window),
		    image_path, NULL);
	}
	free(image_path);
}

/*!
 * \brief Display and error dialog with one parameter.
 * \param error the message to use in the dialog
 */
void
UIDisplayError1(char *error)
{
	GtkWidget * dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(mw.window),
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_ERROR,
	    GTK_BUTTONS_OK,
	    "%s",
	    error);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
UIDisasterNotify(disaster_t disaster)
{
	char temp[100];

	switch (disaster) {
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
		return;
	}
	UIDisplayError1(temp);
}

/*! 
 * \brief notify that theres a problem in the city.
 * \param problem the problem to notify.
 */
void
UIProblemNotify(problem_t problem)
{
	char temp[100];

	switch (problem) {
	case peFineOnMoney:
		return;
	case peLowOnMoney:
		strcpy(temp, "Low on money");
		break;
	case peOutOfMoney:
		strcpy(temp, "Out of money");
		break;
	case peFineOnPower:
		return;
	case peLowOnPower:
		strcpy(temp, "Low On Power");
		break;
	case peOutOfPower:
		strcpy(temp, "Out of Power");
		break;
	case peFineOnWater:
		return;
	case peLowOnWater:
		strcpy(temp, "Low on Water");
		break;
	case peOutOfWater:
		strcpy(temp, "Out of Water");
		break;
	default:
		return;
	}

	UIDisplayError1(temp);
}

void
UISystemErrorNotify(syserror_t error)
{
	char temp[100];

	if (error == seOutOfMemory) {
		strcpy(temp, "Out of memory. Save and exit now!!!!");
	} else {
		return;
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
UIPostLoadGame(void)
{
}

void
UIUnlockScreen(void)
{
	gtk_widget_queue_draw(mw.drawing);
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
	char temp[24];

	sprintf(temp, "$: %ld", (long)getCredits());
	gtk_label_set_text((GtkLabel *)mw.l_credits, temp);
}

/*!
 * \brief update/draw the date on screen
 */
void
UIDrawDate(void)
{
	char temp[24];

	getDate((char *)temp);
	gtk_label_set_text((GtkLabel *)mw.l_time, temp);
}

/*!
 * \brief Draw the Location on screen
 */
void
UIDrawLocation(void)
{
	char temp[50];

	sprintf(temp, "(%02u, %02u)", (int)getMapXPos(), (int)getMapYPos());

	gtk_label_set_text((GtkLabel*)mw.l_location, temp);
}

/*!
 * \brief draw the population
 *
 * Actually simply set the population label to the appropriate value.
 */
void
UIDrawPopulation(void)
{
	char temp[50];
	sprintf(temp, "Population: %-9li", (long)getPopulation());

	gtk_label_set_text((GtkLabel*)mw.l_pop, temp);
}

/*!
 * \brief Draw the speed/update the seed icon on screen
 * \todo Implement this for the platform
 */
void
UIDrawSpeed(void)
{
}

/*!
 * \brief Draw the build icon on the display
 * \todo Impement this for this platform
 */
void
UIDrawBuildIcon(void)
{

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
 * \brief scroll the map in the appropriate direction
 *
 * \todo double buffer to improve the rendering of the screen!
 */
void
UIScrollDisplay(dirType direction __attribute__((unused)))
{
	RedrawAllFields();
	gtk_widget_queue_draw(mw.drawing);
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

void
UIPaintField(UInt16 xpos, UInt16 ypos, welem_t nGraphic)
{
	GdkGC *gc = gdk_gc_new(mw.p_play);

	gdk_draw_drawable(
	    GDK_DRAWABLE(mw.p_play),
	    gc,
	    mw.p_zones,
	    (nGraphic % HORIZONTAL_TILESIZE) * gameTileSize(),
	    (nGraphic / HORIZONTAL_TILESIZE) * gameTileSize(),
	    xpos * gameTileSize(),
	    ypos * gameTileSize(),
	    gameTileSize(),
	    gameTileSize());
	g_object_unref(gc);
	gtk_widget_queue_draw(mw.drawing);
}

void
UIPaintSpecialObject(UInt16 xpos, UInt16 ypos, Int8 i)
{
	GdkGC *gc;

	gc = gdk_gc_new(mw.p_play);
	gdk_gc_set_clip_mask(gc, mw.p_monsters_m);
	gdk_gc_set_clip_origin(gc,
	    xpos * gameTileSize() - (GG.objects[i].dir * gameTileSize()),
	    ypos * gameTileSize() - (i * gameTileSize()));

	gdk_draw_drawable(
	    mw.p_play,
	    gc,
	    mw.p_monsters,
	    GG.objects[i].dir * gameTileSize(),
	    i * gameTileSize(),
	    xpos * gameTileSize(),
	    ypos * gameTileSize(),
	    gameTileSize(),
	    gameTileSize());
	g_object_unref(gc);
	gtk_widget_queue_draw(mw.drawing);
}

void
UIPaintSpecialUnit(UInt16 xpos, UInt16 ypos, Int8 i)
{
	GdkGC *gc;

	gc = gdk_gc_new(mw.p_play);
	gdk_gc_set_clip_mask(gc, mw.p_units_m);
	gdk_gc_set_clip_origin(gc,
	    xpos * gameTileSize() - (GG.units[i].type * gameTileSize()),
	    ypos * gameTileSize());

	gdk_draw_drawable(
	    mw.p_play,
	    gc,
	    mw.p_units,
	    GG.units[i].type * gameTileSize(),
	    0,
	    xpos * gameTileSize(),
	    ypos * gameTileSize(),
	    gameTileSize(),
	    gameTileSize());
	g_object_unref(gc);
	gtk_widget_queue_draw(mw.drawing);
}

void
UIDrawMapZone(Int16 xpos, Int16 ypos, welem_t nGraphic, GdkDrawable *drawable)
{
	GdkGC *gc = gdk_gc_new(drawable);
	gdk_draw_drawable(
	    drawable,
	    gc,
	    mw.p_mapzones,
	    (nGraphic % HORIZONTAL_TILESIZE) * mapTileSize(),
	    (nGraphic / HORIZONTAL_TILESIZE) * mapTileSize(),
	    xpos * mapTileSize(),
	    ypos * mapTileSize(),
	    mapTileSize(),
	    mapTileSize());
	g_object_unref(gc);
}

void
UIDrawMapSpecialObject(Int16 xpos, Int16 ypos, Int16 i, GdkDrawable *drawable)
{
	GdkGC *gc = gdk_gc_new(drawable);

	gdk_draw_drawable(
	    drawable,
	    gc,
	    mw.p_mapspecials,
	    i * mapTileSize(),
	    0,
	    xpos * mapTileSize(),
	    ypos * mapTileSize(),
	    mapTileSize(),
	    mapTileSize());
	g_object_unref(gc);
}

void
UIDrawMapSpecialUnit(Int16 xpos, Int16 ypos, Int16 i, GdkDrawable *drawable)
{
	GdkGC *gc = gdk_gc_new(drawable);

	gdk_draw_drawable(
	    drawable,
	    gc,
	    mw.p_units,
	    i * mapTileSize(),
	    0,
	    xpos * mapTileSize(),
	    ypos * mapTileSize(),
	    mapTileSize(),
	    mapTileSize());
	g_object_unref(gc);
}

void
UIPaintCursor(UInt16 xpos __attribute__((unused)),
    UInt16 ypos __attribute__((unused)))
{
	/* not used on this platform */
}

/*!
 * \brief draw an overlay icon on the screen
 * \param xpos the horizontal location
 * \param ypos the vertical location
 * \param offset the offst of the item to paint.
 */
static void
DrawOverlay(UInt16 xpos, UInt16 ypos, welem_t offset)
{
	GdkGC *gc;

	gc = gdk_gc_new(mw.p_play);
	gdk_gc_set_clip_mask(gc, mw.p_zones_m);
	gdk_gc_set_clip_origin(gc,
	    (xpos - (offset % HORIZONTAL_TILESIZE)) * gameTileSize(),
	    (ypos - (offset / HORIZONTAL_TILESIZE)) * gameTileSize());

	gdk_draw_drawable(
	    mw.p_play,
	    gc,
	    mw.p_zones,
	    (offset % HORIZONTAL_TILESIZE) * gameTileSize(),
	    (offset / HORIZONTAL_TILESIZE) * gameTileSize(),
	    xpos * gameTileSize(),
	    ypos * gameTileSize(),
	    gameTileSize(),
	    gameTileSize());
	g_object_unref(gc);
}

void
UIPaintPowerLoss(UInt16 xpos, UInt16 ypos)
{
	DrawOverlay(xpos, ypos, Z_POWER_OUT);
}

void
UIPaintWaterLoss(UInt16 xpos, UInt16 ypos)
{
	DrawOverlay(xpos, ypos, Z_WATER_OUT);
}

BuildCode
UIGetSelectedBuildItem(void)
{
	return (selectedBuildItem);
}

/*! \brief painted flag - for play area back-buffer */
static int painted_flag;

/*! \brief clear the painted flag */
void
clearPaintedFlag(void)
{
	painted_flag = 0;
}

/*! \brief set the painted flag */
void
setPaintedFlag(void)
{
	painted_flag = 1;
}

/*!
 * \brief test painted flag
 * \return the painted flag state
 */
int
getPaintedFlag(void)
{
	return (painted_flag);
}

/*!
 * \todo render the entire play area into an offscreen pixmap, making this call
 * a non-op (the expose code for the drawing area will take care of it).
 */
void
UIDrawPlayArea(void)
{
	Int16 x;
	Int16 y;
	Int16 maxx = getMapXPos() + getVisibleX() >= getMapWidth() ?
	    getMapWidth() : getMapXPos() + getVisibleX();
	Int16 maxy = getMapYPos() + getVisibleY() >= getMapHeight() ?
	    getMapHeight() : getMapYPos() + getVisibleY();

	for (x = getMapXPos(); x < maxx; x++) {
		for (y = getMapYPos(); y < maxy; y++) {
			if (!(getWorldFlags(WORLDPOS(x, y)) & PAINTEDBIT))
				DrawFieldWithoutInit(x, y);
		}
	}

	gtk_widget_queue_draw(mw.drawing);
}

/*!
 * The resizing of the map corresponds to recreating the backing map area
 * as well as the backing play area.
 */
void
UIMapResize(void)
{
	if (mw.p_play != NULL) g_object_unref(G_OBJECT(mw.p_play));
	mw.p_play = gdk_pixmap_new(drawable_main_get(),
	    getMapWidth() * gameTileSize(),
	    getMapHeight() * gameTileSize(), -1);
	clearPaintedFlag();
	resizeMap();
}

Int8
UIClipped(UInt16 xpos __attribute__((unused)),
    UInt16 ypos __attribute__((unused)))
{
	return (FALSE);
}

void
LockZone(lockZone zone __attribute__((unused)))
{
	/* not used on this platform */
}

void
UnlockZone(lockZone zone __attribute__((unused)))
{
	/* not used on this platform */
}

void
ReleaseZone(lockZone zone)
{
	if (zone == lz_world) {
		free(worldPtr);
		worldPtr = NULL;
	}
}

UInt32
GetRandomNumber(UInt32 max)
{
	/* se `man 3 rand` why I'm not using: return (rand() % max) */
	return ((UInt32)((float)max*rand()/(RAND_MAX+1.0)));
}

/*!
 * Ensure that the scroll bars are moved correctly.
 */
void
MapHasJumped(void)
{
	GTK_ADJUSTMENT(mw.sc_hor)->value = getMapXPos();
	GTK_ADJUSTMENT(mw.sc_vert)->value = getMapYPos();
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(mw.sc_hor));
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(mw.sc_vert));
	gtk_widget_queue_draw(mw.drawing);
}

/*!
 * refresh all the entities on the screen that need repainting
 */
static void
doRepaintDisplay(void)
{
	UIInitDrawing();
	if (checkGraphicUpdate(gu_playarea))
		UIDrawPlayArea();
	if (checkGraphicUpdate(gu_credits))
		UIDrawCredits();
	if (checkGraphicUpdate(gu_population))
		UIDrawPopulation();
	if (checkGraphicUpdate(gu_date))
		UIDrawDate();
	if (checkGraphicUpdate(gu_location))
		if (!GETMINIMAPVISIBLE())
			UIDrawLocation();
	if (checkGraphicUpdate(gu_buildicon))
		UIDrawBuildIcon();
	if (checkGraphicUpdate(gu_speed))
		UIDrawSpeed();
	/*if (checkGraphicUpdate(gu_desires))
		UIPaintDesires();*/
	clearGraphicUpdate();
	UIFinishDrawing();
}

/*!
 * \brief terminate the game.
 *
 * Invoked when the quit option is selected from the menu
 */
static void
QuitGame(void)
{
	gtk_main_quit();
}

/*!
 * \brief force a redistribution next iteration
 */
static void
forceRedistribute(void)
{
	AddGridUpdate(GRID_ALL);
}

#ifdef LOGGING

#include <stdarg.h>

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
#endif /* LOGGING */
