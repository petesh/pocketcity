#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <main.h>
#include <savegame.h>
#include <budget.h>
#include <ui.h>
#include <handler.h>
#include <drawing.h>
#include <globals.h>
#include <build.h>
#include <simulation.h>
#include <disaster.h>


GtkWidget *drawingarea;
GtkWidget *window;
GtkWidget *creditslabel;
GtkWidget *poplabel;
GtkWidget *timelabel;
GtkObject *playscrollerh, *playscrollerv;
void * worldPtr;
void * worldFlagsPtr;
GdkPixmap *zones_bitmap, *monsters, *units;
GdkBitmap *zones_mask, *monsters_mask, *units_mask;
UInt8 selectedBuildItem = 0;
void SetUpMainWindow(void);
static gint mainloop_callback(gpointer data);
void UIQuitGame(GtkWidget *w, gpointer data) { gtk_main_quit(); }
void UISetSpeed(GtkWidget *w, gpointer data);

GtkItemFactoryEntry menu_items[] = {
	{ "/_File", NULL, NULL, 0, "<Branch>" },
	{ "/File/_New", "<control>N", UINewGame, 0, NULL },
	{ "/File/_Open", "<control>O", UIOpenGame, 0, NULL },
	{ "/File/_Save", "<control>S", UISaveGame, 0, NULL },
	{ "/File/Save _As", NULL, UISaveGameAs, 0, NULL },
	{ "/File/sep1",	NULL, NULL, 0, "<Separator>" },
	{ "/File/_Quit", NULL, UIQuitGame, 0, NULL },
	{ "/_View", NULL, NULL, 	0, "<Branch>" },
	{ "/View/_Budget", NULL, UIViewBudget, 0, NULL },
	{ "/_Speed", NULL, NULL, 0, "<Branch>" },
	{ "/Speed/_Pause", NULL, UISetSpeed, SPEED_PAUSED, NULL	},
	{ "/Speed/sep1", NULL, NULL, 0, "<Separator>" },
	{ "/Speed/_Slow", NULL, UISetSpeed, SPEED_SLOW, NULL },
	{ "/Speed/_Medium", NULL, UISetSpeed, SPEED_MEDIUM, NULL },
	{ "/Speed/_Fast", NULL, UISetSpeed, SPEED_FAST, NULL },
	{ "/Speed/_Turbo", NULL, UISetSpeed, SPEED_TURBO, NULL	},
};


int
main(int argc, char *argv[])
{
	gint timerID;

	gtk_init(&argc, &argv);
	srand(time(0));

	SetUpMainWindow();
	/* start the timer */
	timerID = gtk_timeout_add(1000, (mainloop_callback), 0);

	gtk_main();
	WriteLog("Cleaning up\n");
	gtk_timeout_remove(timerID);
	free(worldPtr);
	free(worldFlagsPtr);

	return (0);
}

unsigned int timekeeper = 0;
unsigned int timekeeperdisaster = 0;

void
UISetSpeed(GtkWidget *w, gpointer data)
{
	WriteLog("Setting speed to %i\n", GPOINTER_TO_INT(data));
	game.gameLoopSeconds = GPOINTER_TO_INT(data);
}

static gint
mainloop_callback(gpointer data)
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

	if (timekeeper >= game.gameLoopSeconds &&
	    game.gameLoopSeconds != SPEED_PAUSED) {
		WriteLog("A month has gone by - total months: %lu\n",
		    (unsigned long)game.TimeElapsed);
		timekeeper = 0;
		do {
			phase = Sim_DoPhase(phase);
		} while (phase != 0);
		gtk_widget_queue_draw(drawingarea);
	}

	UIUpdateBudget();
	return (TRUE); /* yes, call us again in a sec */
}

static gint
toolbox_callback(GtkWidget *widget, gpointer data)
{
	selectedBuildItem = GPOINTER_TO_INT(data);
	return (FALSE);
}

void
scrollbar(GtkAdjustment *adj)
{
	Goto(GTK_ADJUSTMENT(playscrollerh)->value,
	    GTK_ADJUSTMENT(playscrollerv)->value);
}

static gint
delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_main_quit();
	return (FALSE);
}

static gint
drawing_exposed_callback(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	RedrawAllFields();

	return (FALSE);
}

static gint
drawing_realized_callback(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	PCityMain();
	UINewGame(NULL, 0);
	return (FALSE);
}


static gint
button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	if (event->button == 1) {
		BuildSomething(
		    (int)(event->x / vgame.tileSize) + game.map_xpos,
		    (int)(event->y / vgame.tileSize) + game.map_ypos);
	} else if (event->button == 3) {
		Build_Bulldoze(
		    (int)(event->x / vgame.tileSize) + game.map_xpos,
		    (int)(event->y / vgame.tileSize) + game.map_ypos, 0);
	}

	return (TRUE);
}

static gint
motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
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
		    (int)(x / vgame.tileSize) + game.map_xpos,
		    (int)(y / vgame.tileSize) + game.map_ypos);
	} else if (state & GDK_BUTTON3_MASK &&
	    x > 0 && x < vgame.visible_x*vgame.tileSize &&
	    y > 0 && y < vgame.visible_y*vgame.tileSize) {
		Build_Bulldoze(
		    (int)(x / vgame.tileSize) + game.map_xpos,
		    (int)(y / vgame.tileSize) + game.map_ypos, 1);
	}

	return (TRUE);
}

GtkWidget *
setupToolBox(void)
{
	GtkWidget *button_image;
	GtkTooltips *tips;
	GtkWidget *toolbox;
	GtkWidget *button;
	int i;
	char image_path[40];
	/* If you change the order here you need to change the xpm... */
	/* TODO: make the file names related to the items */
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
		sprintf(image_path, "graphic/icons/interface_%02i.png", i);
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
	return (toolbox);
}

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

void
SetUpMainWindow(void)
{
	GtkWidget *fieldbox, *box, *toolbox, *headerbox, *playingbox, *main_box;
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
	gtk_drawing_area_size((GtkDrawingArea*)drawingarea, 320, 240);
	/* and some scrollbars for the drawingarea */
	playscrollerh = gtk_adjustment_new(60, 10, 110, 1, 10, 20);
	playscrollerv = gtk_adjustment_new(57, 10, 107, 1, 10, 15);
	hscroll = gtk_hscrollbar_new(GTK_ADJUSTMENT(playscrollerh));
	vscroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(playscrollerv));
	g_signal_connect(G_OBJECT(playscrollerh), "value_changed",
	    G_CALLBACK(scrollbar), NULL);
	g_signal_connect(G_OBJECT(playscrollerv), "value_changed",
	    G_CALLBACK(scrollbar), NULL);

	gtk_table_attach_defaults(GTK_TABLE(playingbox), drawingarea,
	    0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(playingbox), hscroll, 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(playingbox), vscroll, 1, 2, 0, 1);

	/* arange in boxes  */
	toolbox = setupToolBox();
	gtk_box_pack_end(GTK_BOX(main_box), box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), toolbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), fieldbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(fieldbox), headerbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(fieldbox), playingbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(fieldbox), poplabel, TRUE, TRUE, 0);
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

void
UISetUpGraphic(void)
{
	zones_bitmap = gdk_pixmap_create_from_xpm(drawingarea->window,
	    &zones_mask, NULL, "graphic/zones_16x16-color.png");
	monsters = gdk_pixmap_create_from_xpm(drawingarea->window,
	    &monsters_mask, NULL, "graphic/monsters_16x16-color.png");
	units = gdk_pixmap_create_from_xpm(drawingarea->window,
	    &units_mask, NULL, "graphic/units_16x16-color.png");
}


Int16
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
	return (0);
}

Int16
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
	return (0);
}

void
UIInitDrawing(void)
{
}

void
UIFinishDrawing(void)
{
}

void
UIUnlockScreen(void)
{
	/* not used for this platform */
}

void
UILockScreen(void)
{
	/* not used for this platform */
}

void
UIDrawBorder(void)
{
	WriteLog("UIDrawBorder\n");
}

void
UIDrawCredits(void)
{
	char temp[23];

	sprintf(temp, "$: %ld", (long)game.credits);
	gtk_label_set_text((GtkLabel *)creditslabel, temp);
	GetDate((char *)temp);
	gtk_label_set_text((GtkLabel *)timelabel, temp);
}

void
UIUpdateBuildIcon(void)
{
	WriteLog("UIUpdateBuildIcon\n");
}

void
UIGotoForm(Int16 n)
{
	WriteLog("UIGotoForm\n");
}

void
UICheckMoney(void)
{
	WriteLog("UICheckMoney\n");
}

void
UIScrollMap(dirType direction)
{
	/* TODO: Optimize this as in the Palm port? */
	/*	   (if nessasary) */
	RedrawAllFields();
}

void
_UIDrawRect(Int16 nTop, Int16 nLeft, Int16 nHeight, Int16 nWidth)
{
	WriteLog("_UIDrawRect\n");
}

void
UIDrawField(Int16 xpos, Int16 ypos, UInt8 nGraphic)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    zones_bitmap,
	    (nGraphic % 64) * vgame.tileSize,
	    (nGraphic / 64) * vgame.tileSize,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	gdk_gc_destroy(gc);
}

void
UIDrawSpecialObject(Int16 i, Int16 xpos, Int16 ypos)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_gc_set_clip_mask(gc, monsters_mask);
	gdk_gc_set_clip_origin(gc,
	    xpos * vgame.tileSize - (game.objects[i].dir * vgame.tileSize),
	    ypos * vgame.tileSize - (i * vgame.tileSize));

	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    monsters,
	    game.objects[i].dir * vgame.tileSize,
	    i * vgame.tileSize,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	gdk_gc_destroy(gc);
}

void
UIDrawSpecialUnit(Int16 i, Int16 xpos, Int16 ypos)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_gc_set_clip_mask(gc, units_mask);
	gdk_gc_set_clip_origin(gc,
	    xpos * vgame.tileSize - (game.units[i].type * vgame.tileSize),
	    ypos * vgame.tileSize);

	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    units,
	    game.units[i].type * vgame.tileSize,
	    0,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	gdk_gc_destroy(gc);
}

void
UIDrawCursor(Int16 xpos, Int16 ypos)
{
	/* not used on this platform */
}

void
UIDrawPowerLoss(Int16 xpos, Int16 ypos)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_gc_set_clip_mask(gc, zones_mask);
	gdk_gc_set_clip_origin(gc,
	    xpos * vgame.tileSize - 128,
	    ypos * vgame.tileSize);

	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    zones_bitmap,
	    128,
	    0,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	gdk_gc_destroy(gc);
}

void
UIDrawWaterLoss(Int16 xpos, Int16 ypos)
{
	GdkGC *gc;

	gc = gdk_gc_new(drawingarea->window);
	gdk_gc_set_clip_mask(gc, zones_mask);
	gdk_gc_set_clip_origin(gc,
	    xpos * vgame.tileSize-64,
	    ypos * vgame.tileSize);

	gdk_draw_drawable(
	    drawingarea->window,
	    gc,
	    zones_bitmap,
	    64,
	    0,
	    xpos * vgame.tileSize,
	    ypos * vgame.tileSize,
	    vgame.tileSize,
	    vgame.tileSize);
	gdk_gc_destroy(gc);
}

UInt8
UIGetSelectedBuildItem(void)
{
	return (selectedBuildItem);
}

Int16
InitWorld(void)
{
	worldPtr = malloc(10);
	worldFlagsPtr = malloc(10);

	if (worldPtr == NULL || worldFlagsPtr == NULL) {
		UIDisplayError(0);
		WriteLog("malloc failed - initworld\n");
		return (0);
	}
	return (1);
}

Int16
ResizeWorld(UInt32 size)
{
	worldPtr = realloc(worldPtr, size);
	worldFlagsPtr = realloc(worldFlagsPtr, size);

	if (worldPtr == NULL || worldFlagsPtr == NULL) {
		UIDisplayError(0);
		WriteLog("realloc failed - resizeworld\n");
		return (0);
	}
	memset(worldPtr, 0, size);
	memset(worldFlagsPtr, 0, size);

	return (1);
}

void
LockWorld(void)
{
	/* not used on this platform */
}

void
UnlockWorld(void)
{
	/* not used on this platform */
}

UInt8
GetWorld(UInt32 pos)
{
	if (pos > GetMapMul())
		return (0);
	return (((UInt8 *)worldPtr)[pos]);
}

void
SetWorld(UInt32 pos, UInt8 value)
{
	if (pos > GetMapMul())
		return;
	((UInt8 *)worldPtr)[pos] = value;
}

void
LockWorldFlags(void)
{
	/* not used on this platform */
}

void
UnlockWorldFlags(void)
{
	/* not used on this platform */
}

UInt8
GetWorldFlags(UInt32 pos)
{
	if (pos > GetMapMul())
		return (0);
	return (((UInt8 *)worldFlagsPtr)[pos]);
}

void
SetWorldFlags(UInt32 pos, UInt8 value)
{
	if (pos > GetMapMul())
		return;
	((UInt8 *)worldFlagsPtr)[pos] = value;
}

void
AndWorldFlags(UInt32 pos, UInt8 value)
{
	if (pos > GetMapMul())
		return;
	((UInt8 *)worldFlagsPtr)[pos] &= value;
}

void
OrWorldFlags(UInt32 pos, UInt8 value)
{
	if (pos > GetMapMul())
		return;
	((UInt8 *)worldFlagsPtr)[pos] |= value;
}

UInt32
GetRandomNumber(UInt32 max)
{
	/* se `man 3 rand` why I'm not using: return (rand() % max) */
	return ((UInt32)((float)max*rand()/(RAND_MAX+1.0)));
}

void
MapHasJumped(void)
{
	((GtkAdjustment *)playscrollerh)->value = game.map_xpos+10;
	((GtkAdjustment *)playscrollerv)->value = game.map_ypos+7;
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(playscrollerh));
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(playscrollerv));

}

void
UISetTileSize(Int16 size)
{
	WriteLog("UISetTileSize\n");
}

void
UIDrawPop(void)
{
	char temp[50];
	sprintf(temp, "(%02u, %02u) Population: %-9li",
	    game.map_xpos, game.map_ypos,
	    vgame.BuildCount[COUNT_RESIDENTIAL]*150);

	gtk_label_set_text((GtkLabel*)poplabel, temp);
}

#ifdef DEBUG

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
#endif
