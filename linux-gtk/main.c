/*!
 * \file
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
#include <distribution.h>
#include <simulation.h>
#include <disaster.h>
#include <compilerpragmas.h>
#include <nix_utils.h>
#include <simulation-ui.h>
#include <zonemon.h>
#include <localize.h>
#include <actions.h>
#include <assert.h>

/*! \brief path to search for graphics */
Char *pathsearch = (Char *)"$:$/..:$/graphic:$/graphic/icons:$/../graphic:$/../graphic/icons:$/../../graphic:$/../../graphic/icons";

static size_t max_path;
/*! \brief number of milli seconds in a second */
#define	MILLISECS	1000
/*! \brief the number of ticks to make every second */
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

static void set_speed_handler(GtkAction *action, gpointer data);
static void toolbox_callback(GtkAction *action, gpointer data);
static void set_speed_pause(void);
static void set_speed_slow(void);
static void set_speed_medium(void);
static void set_speed_fast(void);
static void set_speed_turbo(void);

static void cleanupPixmaps(void);
static void ResetViewable(void);
static void ShowMainWindow(void);
static void doRepaintDisplay(void);

#if defined(DEBUG)
static void force_redistribute(void);
static void cash_up(void);
#endif

static void s_nothing(void) { }

const struct _stockitems {
	const char *name;
	const char *imagefile;
	const char *accelerator;
} stockitems[] = {
	{ A_SPEED_PAUSE, "speed_paused.png", "<Control>0" },
	{ A_SPEED_SLOW, "speed_slow.png", "<Control>1" },
	{ A_SPEED_MEDIUM, "speed_medium.png", "<Control>2" },
	{ A_SPEED_FAST, "speed_fast.png", "<Control>3" },
	{ A_SPEED_TURBO, "speed_turbo.png", "<Control>4" },
	{ A_BULLDOZER, "interface_00.png", NULL },
	{ A_ROAD, "interface_01.png", NULL },
	{ A_POWERLINE, "interface_02.png", NULL },
	{ A_RESIDENTIAL, "interface_03.png", NULL },
	{ A_COMMERCIAL, "interface_04.png", NULL },
	{ A_INDUSTRIAL, "interface_05.png", NULL },
	{ A_TREE, "interface_06.png", NULL },
	{ A_WATER, "interface_07.png", NULL },
	{ A_WATERPIPE, "interface_08.png", NULL },
	{ A_COALPLANT, "interface_09.png", NULL },
	{ A_NUKEPLANT, "interface_10.png", NULL },
	{ A_WATERPUMP, "interface_11.png", NULL },
	{ A_FIRESTATION, "interface_12.png", NULL },
	{ A_POLICESTATION, "interface_13.png", NULL },
	{ A_MILITARYBASE, "interface_14.png", NULL },
	{ A_FIREBRIGADE, "interface_18.png", NULL },
	{ A_POLICECAR, "interface_19.png", NULL },
	{ A_TANK, "interface_20.png", NULL },
	{ BUDGET_SHOW, NULL, "<Control>B" },
	{ MAP_SHOW, NULL, "<Control>M" },
	{ HOVER_SHOW, NULL, "<Control>H" }
};

#define NSTOCKITEMS	(sizeof (stockitems) / sizeof (stockitems[0]))

/*! \brief a hookup set of action->event mappings as default */
const struct _actionhooks {
	char *name; /*!< name of the action */
	char *label; /*!< the text for the action (base) */
	char *tooltip; /*!< the tooltip for the action (base) */
	char *icon; /*!< the name of the icon for the action */
	GCallback handler;	/*!< function handler */
	gint parameter;
} actionhooks[] = {
	{ "file-action", N_("_File"), NULL, NULL, s_nothing, 0 },
	{ "game-new", N_("_New"), N_("Create a new game"), GTK_STOCK_NEW,
	  newgame_handler, 0 },
	{ "game-open", N_("_Open"), N_("Open a save game"), GTK_STOCK_OPEN,
	  opengame_handler, 0 },
	{ "game-save", N_("_Save"), N_("Save a game in progress"),
	  GTK_STOCK_SAVE, savegame_handler, 0 },
	{ "game-save-as", N_("Save _As"),
	  N_("Save a city into a new file"), GTK_STOCK_SAVE_AS,
	  savegameas_handler, 0 },
	{ "game-exit", N_("E_xit"), N_("Leave the game"),
	  GTK_STOCK_QUIT, QuitGame, 0 },
	{ "view-action", N_("_View"), NULL, NULL, s_nothing, 0 },
	{ BUDGET_SHOW, N_("_Budget"), N_("Show the budget information"),
	  NULL, budget_show, 0 },
	{ MAP_SHOW, N_("_Map"), N_("Show the mini map"), NULL,
	  map_show, 0 },
	{ HOVER_SHOW, N_("_Hover"), N_("Query what's under the mouse"),
	  NULL, hover_show, 0 },
	{ "speed-action", N_("_Speed"), NULL, NULL, s_nothing, 0 },

	{ A_SPEED_PAUSE, N_("_Pause"), N_("Pause the game"), NULL,
	  G_CALLBACK(set_speed_handler), SPEED_PAUSED },
	{ A_SPEED_SLOW, N_("_Slow"), N_("Set speed to slowest rate"), NULL,
	  G_CALLBACK(set_speed_handler), SPEED_SLOW },
	{ A_SPEED_MEDIUM, N_("_Medium"), N_("Set the speed to medium rate"),
	  NULL, G_CALLBACK(set_speed_handler), SPEED_MEDIUM },
	{ A_SPEED_FAST, N_("_Fast"), N_("Set speed to fast rate"),
	  NULL, G_CALLBACK(set_speed_handler), SPEED_FAST },
	{ A_SPEED_TURBO, N_("_Turbo"), N_("Set speed to fastest rate"),
	  NULL, G_CALLBACK(set_speed_handler), SPEED_TURBO },

	{ "simulation-action", N_("S_imulation"), NULL, NULL, s_nothing, 0 },
#if defined(DEBUG)
	{ "mark-redistribute", N_("_Redistribute"), N_("ask to recheck the "
	  "power circuit"), NULL, force_redistribute, 0 },
	{ "cash-up", N_("_Cash me up"), N_("give user money"), NULL,
	  cash_up, 0 },
#endif

	{ A_BULLDOZER, N_("Bulldozer"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Bulldozer },
	{ A_ROAD, N_("Road"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Road },
	{ A_POWERLINE, N_("Power Line"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Power_Line },
	{ A_RESIDENTIAL, N_("Residential"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Zone_Residential },
	{ A_COMMERCIAL, N_("Commercial"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Zone_Commercial },
	{ A_INDUSTRIAL, N_("Industrial"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Zone_Industrial },
	{ A_TREE, N_("Tree"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Tree },
	{ A_WATER, N_("Water"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Water },
	{ A_WATERPIPE, N_("Water Pipe"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Water_Pipe },
	{ A_COALPLANT, N_("Coal Power Plant"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Power_Plant },
	{ A_NUKEPLANT, N_("Nuclear Power Plant"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Nuclear_Plant },
	{ A_WATERPUMP, N_("Water Pump"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Water_Pump },
	{ A_FIRESTATION, N_("Fire Station"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Fire_Station },
	{ A_POLICESTATION, N_("Police Station"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Police_Station },
	{ A_MILITARYBASE, N_("Military Base"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Military_Base },
	{ A_FIREBRIGADE, N_("Fire Brigade"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Defence_Fire },
	{ A_POLICECAR, N_("Police Car"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Defence_Police },
	{ A_TANK, N_("Tank"), NULL, NULL,
	  G_CALLBACK(toolbox_callback), Be_Defence_Military },
};

/*! \brief number of action hooks */
#define NACTIONHOOKS	(sizeof (actionhooks) / sizeof (actionhooks[0]))

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
	{ "/View/_Budget", "<control>B", budget_show, 0, NULL, 0 },
	{ "/View/_Map", "<control>M", map_show, 0, NULL, 0 },
	{ "/View/_Hover", "<control>H", hover_show, 0, NULL, 0 },
	{ "/_Speed", NULL, NULL, 0, "<Branch>", 0 },
	{ "/Speed/_Pause", "<control>0", set_speed_pause, 0, NULL, 0 },
	{ "/Speed/sep1", NULL, NULL, 0, "<Separator>", 0 },
	{ "/Speed/_Slow", "<control>1", set_speed_slow, 0, NULL, 0 },
	{ "/Speed/_Medium", "<control>2", set_speed_medium, 0, NULL, 0 },
	{ "/Speed/_Fast", "<control>3", set_speed_fast, 0, NULL, 0 },
	{ "/Speed/_Turbo", "<control>4", set_speed_turbo, 0, NULL, 0 },
#if defined(DEBUG)
	{ "/S_imulation", NULL, NULL, 0, "<Branch>", 0 },
	{ "/Simulation/_Redistribute", NULL, force_redistribute, 0, NULL, NULL },
	{ "/Simulation/_Cash Me Up", "<control>C", cash_up, 0, NULL, NULL },
#endif

};
/*! \brief number of menu items - the standard #define */
#define	NMENU_ITEMS	(sizeof (menu_items) / sizeof (menu_items[0]))

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
	char *px, *ax, *py;

	max_path = (size_t)pathconf("/", _PC_PATH_MAX) + 1;
	exec_dir = strdup(argv[0]);
	px = strrchr(exec_dir, '/');
	if (px != NULL) {
		*px = '\0';
	} else {
		/* Trouble at't mine */
		char *cwd = getcwd(NULL, 0);
		free(exec_dir);
		exec_dir = cwd;
	}

	srand(time(NULL));
	/* fill in all the $'s in the pathsearch variable */
	py = calloc(1, 1024);
	px = pathsearch;
	while (NULL != (ax = strchr(px, '$'))) {
		strncat(py, px, ax - px);
		strncat(py, exec_dir, strlen(exec_dir));
		px = ax + 1;
	}
	strncat(py, px, strlen(px));
	pathsearch = py;

	gtk_init(&argc, &argv);

	bindtextdomain(PACKAGE, "../po");
	textdomain(PACKAGE);

	ResetViewable();
	SetUpMainWindow();

	PCityMain();
	InitGameStruct();
	ConfigureNewGame();

	ShowMainWindow();

	/* start the timer */
	timerID = g_timeout_add(MILLISECS / TICKPERSEC,
	  (mainloop_callback), 0);

	gtk_main();
	WriteLog("Cleaning up\n");
	g_source_remove(timerID);
	PurgeWorld();
	free(exec_dir);
	free(pathsearch);
	PCityShutdown();
	cleanupMap();
	cleanupPixmaps();

	return (0);
}

/*!
 * \brief get the main window widget
 */
GtkWidget *
window_main_get(void)
{
	return (mw.window);
}

/*!
 * \brief get the drawable for the main window
 */
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
 * \brief set the speed
 * \param speed the speed to set it to
 */
static void
set_speed(int speed)
{
	WriteLog("Setting speed to %i\n", speed);
	setLoopSeconds(speed);
}

/*!
 * \brief handler for the event trigger
 * \param action unused
 * \param data converted to the parameter type
 */
static void
set_speed_handler(GtkAction *action __attribute__((unused)),
  gpointer data)
{
	set_speed(GPOINTER_TO_INT(data));
}

/*!
 * \brief set the game speed to paused
 */
static void
set_speed_pause(void)
{
	set_speed(SPEED_PAUSED);
}

/*!
 * \brief set the game speed to slow
 */
static void
set_speed_slow(void)
{
	set_speed(SPEED_SLOW);
}

/*!
 * \brief set the game speed to normal
 */
static void
set_speed_medium(void)
{
	set_speed(SPEED_MEDIUM);
}

/*!
 * \brief set the game speed to fast
 */
static void
set_speed_fast(void)
{
	set_speed(SPEED_FAST);
}

/*!
 * \brief set the game speed to fast
 */
static void
set_speed_turbo(void)
{
	set_speed(SPEED_TURBO);
}

#if defined(DEBUG)

/*!
 * \brief give a cash injection to the user
 * \param data unused
 * \param param unused
 * \param w source widget
 */
static void
cash_up(void)
{
	incCredits(20000);
	addGraphicUpdate(gu_credits);
	doRepaintDisplay();
}

#endif

static int
searchFile(char *origfile)
{

	return (searchForFile(origfile, max_path, pathsearch));
}

static GdkPixbuf *
load_pixbuf(const char *name)
{
	char *buffer;
	GError *error = NULL;
	GdkPixbuf *ret;

	buffer = alloca(max_path + 1);
	assert(buffer != NULL);
	strncpy(buffer, name, max_path);
	if (searchFile(buffer)) {
		ret = gdk_pixbuf_new_from_file(buffer, &error);
		if (error != NULL) {
			WriteLog("Could not open file: %s\n",
			  error->message);
			g_error_free(error);
			exit(1);
		} else {
			return (ret);
		}
	} else {
		/* XXX: Show dialog */
		WriteLog("Could not create pixbuf from file %s\n",
		  buffer);
		exit (1);
	}
}

static GdkPixmap *
load_pixmap(const char *name, GdkBitmap **mask)
{
	char *buffer;

	buffer = alloca(max_path + 1);
	assert(buffer != NULL);
	strncpy(buffer, name, max_path);
	if (searchFile(buffer)) {
		return (gdk_pixmap_create_from_xpm(mw.window->window,
		  mask, NULL, buffer));
	} else {
		/* XXX: Show dialog */
		WriteLog("Could not create pixmap from file %s\n",
		  buffer);
		exit(1);
	}
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
	
	if (!getGamePlaying() || !getGameInProgress())
		return (TRUE);

	timekeeper++;
	timekeeperdisaster++;

	if ((timekeeperdisaster / TICKPERSEC) >= SIM_GAME_LOOP_DISASTER) {
		MoveAllObjects();
		if (UpdateDisasters()) {
			gtk_widget_queue_draw(mw.drawing);
		}
	}

	addGraphicUpdate(gu_date);
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
 * \param action unused
 * \param data the identifier of the toolbar item
 */
static void
toolbox_callback(GtkAction *action __attribute__((unused)), gpointer data)
{
	selectedBuildItem = GPOINTER_TO_INT(data);
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
	    (x > 0) && x < getVisibleX() * gameTileSize() &&
	    (y > 0) && y < getVisibleY() * gameTileSize()) {
		BuildSomething(
		    (int)(x / gameTileSize()) + getMapXPos(),
		    (int)(y / gameTileSize()) + getMapYPos());
	} else if (state & GDK_BUTTON3_MASK &&
	    (x > 0) && x < getVisibleX() * gameTileSize() &&
	    (y > 0) && y < getVisibleY() * gameTileSize()) {
		Build_Bulldoze(
		    (int)(x / gameTileSize()) + getMapXPos(),
		    (int)(y / gameTileSize()) + getMapYPos(), 1);
	}

	return (TRUE);
}

#if 0
/*!
 * \brief set up the toolbox
 * \return the widget containing the toolbox
 */
GtkWidget *
setupToolBox(void)
{
	GtkWidget *button_image;
	GtkWidget *toolbox;
	unsigned int i;
	Char *image_path;
	size_t max_path = (size_t)pathconf("/", _PC_PATH_MAX) + 1;
	GSList *list = NULL;
	GtkToolItem *item;
	GtkTooltips *tips;

	image_path = malloc(max_path);

	tips = gtk_tooltips_new();

	toolbox = gtk_toolbar_new();
	gtk_container_set_border_width(GTK_CONTAINER(toolbox), 0);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbox), GTK_TOOLBAR_ICONS);

	for (i = 0; i < SIZE_ACTIONS; i++) {
		if (actions[i].entry == -1) {
			item = gtk_separator_tool_item_new();
			gtk_toolbar_insert(GTK_TOOLBAR(toolbox),
			    GTK_TOOL_ITEM(item), -1);
			continue;
		}

		strcpy((char *)image_path, actions[i].file);
		if (searchForFile(image_path, max_path, pathsearch))
			button_image = gtk_image_new_from_file(
			    (const char *)image_path);
		else {
			perror((const char *)image_path);
			exit(1);
		}

		if (list == NULL) {
			item = gtk_radio_tool_button_new(NULL);
			list = gtk_radio_tool_button_get_group(
			    GTK_RADIO_TOOL_BUTTON(item));
		} else {
			item = gtk_radio_tool_button_new(list);
			list = gtk_radio_tool_button_get_group(
			    GTK_RADIO_TOOL_BUTTON(item));
		}

		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), GTK_WIDGET(item),
		    actions[i].text, NULL);
		g_signal_connect(G_OBJECT(item), "clicked",
		    G_CALLBACK(toolbox_callback),
		    GINT_TO_POINTER(actions[i].entry));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(item),
		    button_image);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbox), GTK_TOOL_ITEM(item),
		    -1);
	}

	/*
	 * handle = gtk_handle_box_new();
	 * gtk_handle_box_set_handle_position(
	 * (GtkHandleBox *)handle, GTK_POS_TOP);
	 * gtk_container_add(GTK_CONTAINER(handle), toolbox);
	 */

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
#endif

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
	GtkWidget *footerbox, *headerbox;
	GtkWidget *toolbox;
	GtkWidget *playingbox, *main_box;
	//GtkAccelGroup *accel_group;
	GtkActionGroup *act_group;
	GtkAction *action;
	GtkStockItem stock_item;
	GtkIconFactory *icon_factory;
	GtkUIManager *gum;
	char *gumfile = alloca(max_path);
	char *nel = alloca(max_path);
	unsigned int i;

	mw.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mw.window), _("Pocket City"));
	g_signal_connect(G_OBJECT(mw.window), "delete_event",
	    G_CALLBACK(delete_event), NULL);

	/* Create the stock items */
	icon_factory = gtk_icon_factory_new();
	stock_item.translation_domain = PACKAGE;
	for (i = 0; i < NSTOCKITEMS; i++) {
		stock_item.stock_id = (char *)stockitems[i].name;
		stock_item.label = NULL;
		stock_item.modifier = 0;
		stock_item.keyval = 0;
		if (stockitems[i].accelerator != NULL) {
			gtk_accelerator_parse(stockitems[i].accelerator,
			    &stock_item.keyval,
			    &stock_item.modifier);
		}
		if (stockitems[i].imagefile != NULL) {
			GdkPixbuf *buf;
			GtkIconSet *set;
			
			buf = load_pixbuf(stockitems[i].imagefile);
			set = gtk_icon_set_new_from_pixbuf(buf);
			gtk_icon_factory_add(icon_factory,
			  stockitems[i].name, set);
		}

		gtk_stock_add(&stock_item, 1);
	}
	gtk_icon_factory_add_default(icon_factory);

	act_group = gtk_action_group_new("actions");
	for (i = 0; i < NACTIONHOOKS; i++) {
		char *label = gettext(actionhooks[i].label);
		action = gtk_action_new(actionhooks[i].name,
		  label,
		  actionhooks[i].tooltip == NULL ? label :
		  gettext(actionhooks[i].tooltip),
		  actionhooks[i].icon == NULL ? actionhooks[i].name :
		  actionhooks[i].icon);
		sprintf(nel, "<Actions>/actions/%s", actionhooks[i].name);
		gtk_action_set_accel_path(action, nel);
		//gtk_action_connect_accelerator(action);
		g_signal_connect(G_OBJECT(action), "activate",
		    actionhooks[i].handler,
		    GINT_TO_POINTER(actionhooks[i].parameter));
		gtk_action_group_add_action_with_accel(act_group, action,
		  NULL);
	}

	/* practically everything is in this */
	main_box = gtk_vbox_new(FALSE, 0);

	/* Load the remainder of the UI */
	strcpy(gumfile, "ui.xml");
	if (searchFile(gumfile)) {
		GError *err = NULL;
		gum = gtk_ui_manager_new();
		gtk_ui_manager_insert_action_group(gum, act_group, 0);
		gtk_ui_manager_add_ui_from_file(gum, gumfile, &err);
		if (err != NULL) {
			WriteLog("ui could not be loaded: %s\n",
			    err->message);
			exit(0);
		}
#if defined(DEBUG)
		strcpy(gumfile, "ui-debug.xml");
		if (searchFile(gumfile)) {
			gtk_ui_manager_add_ui_from_file(gum, gumfile, &err);
			if (err != NULL) {
				WriteLog("ui could not be loaded: %s\n",
				    err->message);
				exit(0);
			}
		} else {
			WriteLog("ui file (%s) could not be found\n",
			    gumfile);
			exit(0);
		}
#endif
		gtk_window_add_accel_group(GTK_WINDOW(mw.window),
		    gtk_ui_manager_get_accel_group(gum));
		toolbox = gtk_ui_manager_get_widget(gum, "/mainmenu");
		WriteLog("toolbox = %p\n", toolbox);
		gtk_box_pack_start(GTK_BOX(main_box), toolbox, FALSE,
		    TRUE, 0);
		toolbox = gtk_ui_manager_get_widget(gum, "/buildtoolbar");
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbox),
		  GTK_TOOLBAR_ICONS);
	} else {
		// XXX: Show error message
		WriteLog("ui file (%s) could not be found\n", gumfile);
		exit(0);
	}

	headerbox = gtk_hbox_new(FALSE, 0);
	playingbox = gtk_table_new(2, 2, FALSE);
	footerbox = gtk_hbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(mw.window), main_box);


	mw.l_credits = gtk_label_new("Credits");
	mw.l_time = gtk_label_new("Game Time");
	gtk_box_pack_start(GTK_BOX(headerbox), mw.l_credits, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(headerbox), mw.l_time, TRUE, TRUE, 0);
	mw.l_location = gtk_label_new("Location");
	mw.l_pop = gtk_label_new("Population");
	gtk_box_pack_start(GTK_BOX(footerbox), mw.l_location, TRUE, TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(mw.l_location), 0, 0);
	gtk_box_pack_end(GTK_BOX(footerbox), mw.l_pop, TRUE, TRUE, 0);

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

	gtk_box_pack_start(GTK_BOX(main_box), toolbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), headerbox, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), playingbox, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(main_box), footerbox, FALSE, FALSE, 0);

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

	/* show all the widgets */
	gtk_widget_show_all(main_box);

	gtk_widget_realize(mw.window);
}

/*!
 * \brief show the main window.
 * Simply makes sure that the window is displayed
 */
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
	int i;
	struct image_pms *ipm;
	char *image_path;

	for (i = 0; image_pixmaps[i].filename != NULL; i++) {
		ipm = image_pixmaps + i;
		*ipm->pm = load_pixmap(ipm->filename, ipm->mask);
	}
	/* load the icon */
	image_path = alloca((size_t)pathconf("/", _PC_PATH_MAX) + 1);
	strcpy((char *)image_path, "pcityicon.png");
	if (searchFile(image_path)) {
		gtk_window_set_icon_from_file(GTK_WINDOW(mw.window),
		    (char const *)image_path, NULL);
	}
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
		    "its lair!");
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
	/* if (checkGraphicUpdate(gu_desires)) UIPaintDesires(); */
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

#if defined(DEBUG)

/*!
 * \brief force a redistribution next iteration
 */
static void
force_redistribute(void)
{
	AddGridUpdate(GRID_ALL);
}

#endif

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
