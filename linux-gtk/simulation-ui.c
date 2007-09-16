/*!
 * \file
 * \brief simulation UI file. Contains code to debug the simulation routines.
 *
 * Displays the map/power/water distribution graphs.
 * principal is: red == need power, blue == need water
 * When supplied the node turns black, otherwise it stays the bad color
 *
 * This code is really part of the map functionality. I just need it for
 * debugging for the moment.
 */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <zakdef.h>
#include <simulation.h>
#include <distribution.h>
#include <ui.h>
#include <globals.h>
#include <main.h>
#include <drawing.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <localize.h>

/*! \brief window handle */
static GtkWidget *pw_win;

/*! \brief pointer to the drawing area */
static GtkWidget *dw_area;

/*! \brief pixmap handles for the map */
struct tag_pmh {
	GdkPixmap *allmap; /*!< pixmap for the overall map */
	GdkPixmap *power; /*!< pixmap for the power map */
	GdkPixmap *power_mask; /*!< Mask for the power map */
	GdkPixmap *water; /*!< pixmap for the water map */
	GdkPixmap *water_mask; /*!< mask for the water map */
} pmh;

/*! \brief show the power bitmap */
#define	SHOW_POWER	1
/*! \brief show the water bitmap */
#define	SHOW_WATER	(SHOW_POWER<<1)

/*! \brief govern the visibility of the water and power pixmaps */
static int shown_pixmaps;

/*! \brief handle for the power button */
GtkWidget *button_power;
/*! \brief handle for the water button */
GtkWidget *button_water;

/*! \brief the list of buttons and fields they care about */
static struct button_list {
	GtkWidget	**button; /*!< widget that caused the click */
	int		field; /*!< value to set shown_pixmaps to */
} list_buttons[] = {
	{ &button_power, SHOW_POWER },
	{ &button_water, SHOW_WATER },
	{ NULL, 0 }
};

/*!
 * \brief one of the buttons was clicked
 * \param widget the button that was clicked
 * \param data unused
 */
static void
bpwater_clicked(GtkWidget *widget, gpointer data __attribute__((unused)))
{
	int i;
	struct button_list *bl;
	int newshown_pixmaps = shown_pixmaps;

	for (i = 0; list_buttons[i].button != NULL; i++) {
		bl = list_buttons + i;
		if (*bl->button == widget) {
			if (bl->field == shown_pixmaps) {
				newshown_pixmaps = 0;
			} else {
				newshown_pixmaps = bl->field;
			}
		}
		if (bl->field == shown_pixmaps) {
			gtk_toggle_button_set_active(
			    GTK_TOGGLE_BUTTON(*bl->button), FALSE);
		}
	}
	shown_pixmaps = newshown_pixmaps;
	gtk_widget_queue_draw(dw_area);
}

/*!
 * \brief close the window
 * \param wid unused
 * \param data unused
 * \return false, it always closes.
 */
static gint
close_window(GtkWidget *wid __attribute__((unused)),
    gpointer data __attribute__((unused)))
{
	pw_win = NULL;
	return (FALSE);
}

void
cleanupMap(void)
{
	if (pmh.allmap != NULL) g_object_unref(G_OBJECT(pmh.allmap));
	if (pmh.power != NULL) g_object_unref(G_OBJECT(pmh.power));
	if (pmh.water != NULL) g_object_unref(G_OBJECT(pmh.water));
	if (pmh.power_mask != NULL) g_object_unref(G_OBJECT(pmh.power_mask));
	if (pmh.water_mask != NULL) g_object_unref(G_OBJECT(pmh.water_mask));
	bzero(&pmh, sizeof (pmh));
}

/*! \brief color need/desire table */
typedef enum { ccBlank, ccNeed, ccHas } ccr_t;

/*! \brief blank color */
static GdkColor colBlank = { 0, 0, 0, 0 };
/*! \brief need power color */
static GdkColor colNeedPower = { 1, 255, 0, 0 };
/*! \brief need water color */
static GdkColor colNeedWater = { 1, 0, 0, 255 };
/*! \brief power/ware is supplied color */
static GdkColor colFull = { 2, 255, 255, 255 };

/*!
 * \brief update one of the maps
 * \param xpos the x position
 * \param ypos the y position
 * \param col the color to use
 * \param carry doe sthe item carry the subject in question
 * \param dw_ori the drawing of the origin
 * \param dw_mask the mask for the drawing
 */
static void
updateAMap(Int16 xpos, Int16 ypos, GdkColor *col, ccr_t carry,
    GdkDrawable *dw_ori, GdkDrawable *dw_mask)
{
	GdkGC *gc;

	if (col == NULL) {
		gc = gdk_gc_new(dw_mask);
		gdk_gc_set_function(gc, GDK_OR);
		if (carry == ccBlank) {
			col = &colFull;
		} else {
			col = &colBlank;
		}
		gdk_gc_set_foreground(gc, &colFull);
		gdk_gc_set_background(gc, &colBlank);
		gdk_draw_rectangle(dw_mask, gc, TRUE, xpos * mapTileSize(),
		    ypos * mapTileSize(), mapTileSize(), mapTileSize());
		g_object_unref(gc);
	}

	gc = gdk_gc_new(dw_ori);
	gdk_gc_set_foreground(gc, col);
	gdk_draw_rectangle(dw_ori, gc, TRUE, xpos * mapTileSize(),
	    ypos * mapTileSize(), mapTileSize(), mapTileSize());
	g_object_unref(gc);
}

/*!
 * \brief check if the node carries the specific flagged item
 * \param world the value of the world
 * \param flag the value of the flag
 * \param carry test function for carrying
 * \param flagbit the flagbt to check with
 * \return the carries/doesn carry/supplied state
 */
static ccr_t
checkCommon(welem_t world, selem_t flag, carryfn_t carry, UInt8 flagbit)
{
	if (!carry(world))
		return (ccBlank);
	if (flag & flagbit)
		return (ccHas);
	return (ccNeed);
}

/*!
 * \brief update a power field
 * \param xpos the xposition
 * \param ypos the y position
 * \param has_carry does carry the item
 */
static void
updatePower(Int16 xpos, Int16 ypos, ccr_t has_carry)
{
	GdkColor *dc = (has_carry == ccNeed) ? &colNeedPower : NULL;
	updateAMap(xpos, ypos, dc, has_carry,
	    GDK_DRAWABLE(pmh.power), GDK_DRAWABLE(pmh.power_mask));
}

/*!
 * \brief update a water field
 * \param xpos the xposition
 * \param ypos the y position
 * \param has_carry does carry the item
 */
static void
updateWater(Int16 xpos, Int16 ypos, ccr_t has_carry)
{
	GdkColor *dc = (has_carry == ccNeed) ? &colNeedWater : NULL;
	updateAMap(xpos, ypos, dc, has_carry,
	    GDK_DRAWABLE(pmh.water), GDK_DRAWABLE(pmh.water_mask));
}

/*!
 * \brief initialize the map
 */
/*
 * static void
 * initMap(void)
 * {
 * 	Int16 xpos;
 *	Int16 ypos;
 *	selem_t flag;
 *	welem_t content, special;
 *	UInt32 worldpos;
 *
 *	for (xpos = 0; xpos < getMapWidth(); xpos++) {
 *		for (ypos = 0; ypos < getMapHeight(); ypos++) {
 *			worldpos = WORLDPOS(xpos, ypos);
 *			getWorldAndFlag(worldpos, &content, &flag);
 *			special = GetGraphicNumber(worldpos);
 *			UIPaintMapZone(xpos, ypos, special,
 *			    GDK_DRAWABLE(pmh.allmap));
 *			updatePower(xpos, ypos, checkCommon(content, flag,
 *				    &CarryPower, POWEREDBIT));
 *			updateWater(xpos, ypos, checkCommon(content, flag,
 *				    &CarryWater, WATEREDBIT));
 *		}
 *	}
 * }
 */

/*! \brief initialize the painting structures */
void
doPixPaint(void)
{
	GdkDrawable *dwa = drawable_main_get();

#if defined(DEBUG)
	assert(mapTileSize());
	assert(getMapWidth());
	assert(getMapHeight());
#endif

	pmh.allmap = gdk_pixmap_new(dwa,
	    getMapWidth() * mapTileSize(),
	    getMapHeight() * mapTileSize(), -1);
	pmh.power = gdk_pixmap_new(dwa,
	    getMapWidth() * mapTileSize(),
	    getMapHeight() * mapTileSize(), -1);
	pmh.power_mask = gdk_pixmap_new(dwa,
	    getMapWidth() * mapTileSize(),
	    getMapHeight() * mapTileSize(), 1);
	pmh.water = gdk_pixmap_new(dwa,
	    getMapWidth() * mapTileSize(),
	    getMapHeight() * mapTileSize(), -1);
	pmh.water_mask = gdk_pixmap_new(dwa,
	    getMapWidth() * mapTileSize(),
	    getMapHeight() * mapTileSize(), 1);
}

/*!
 * \brief handle the window being exposed
 * \param area the widget area being exposed
 * \param event the exposure event
 * \param data unused
 * \return FALSE, let system also handle the event
 */
static gint
expose_pw(GtkWidget *area,
    GdkEventExpose *event,
    gpointer data __attribute__((unused)))
{
	GdkGC *gc = gdk_gc_new(area->window);
	GdkPixmap *pm_ext = NULL;
	GdkPixmap *pm_ovl = NULL;

	gdk_draw_drawable(
	    area->window,
	    gc,
	    pmh.allmap,
	    event->area.x,
	    event->area.y,
	    event->area.x,
	    event->area.y,
	    event->area.width,
	    event->area.height);

	switch (shown_pixmaps) {
	case SHOW_POWER:
		pm_ext = pmh.power;
		pm_ovl = pmh.power_mask;
		break;
	case SHOW_WATER:
		pm_ext = pmh.water;
		pm_ovl = pmh.water_mask;
		break;
	default:
		break;
	}
	if (pm_ext != NULL) {
		gdk_gc_set_clip_mask(gc, pm_ovl);
		gdk_draw_drawable(
		    area->window,
		    gc,
		    pm_ext,
		    event->area.x,
		    event->area.y,
		    event->area.x,
		    event->area.y,
		    event->area.width,
		    event->area.height);
	}

	g_object_unref(gc);

	return (FALSE);
}

/*!
 * \brief show the map
 */
void
map_show(void)
{
	GtkWidget *table;

	if (pw_win != NULL) {
		gtk_widget_show(pw_win);
		return;
	}

	pw_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(pw_win),
	    _("Power/Water Distribution"));
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pw_win), TRUE);

	g_signal_connect(G_OBJECT(pw_win), "delete_event",
	    G_CALLBACK(close_window), NULL);

	table = gtk_table_new(2, 2, FALSE);

	button_power = gtk_toggle_button_new_with_label(
	    _("Power Grid"));
	button_water = gtk_toggle_button_new_with_label(
	    _("Water Supply"));

	g_signal_connect(G_OBJECT(button_power), "clicked",
	    G_CALLBACK(bpwater_clicked), NULL);
	g_signal_connect(G_OBJECT(button_water), "clicked",
	    G_CALLBACK(bpwater_clicked), NULL);

	gtk_table_attach(GTK_TABLE(table), button_power, 0, 1, 0, 1, GTK_FILL,
	    GTK_SHRINK, 2, 2);
	gtk_table_attach(GTK_TABLE(table), button_water, 1, 2, 0, 1, GTK_FILL,
	    GTK_SHRINK, 2, 2);

	dw_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(dw_area, getMapWidth() * mapTileSize(),
	    getMapHeight() * mapTileSize());
	gtk_table_attach(GTK_TABLE(table), dw_area, 0, 2, 1, 2, GTK_EXPAND,
	    GTK_EXPAND, 2, 2);
	gtk_container_add(GTK_CONTAINER(pw_win), table);

	gtk_window_set_policy(GTK_WINDOW(pw_win), FALSE, FALSE, TRUE);

	g_signal_connect(G_OBJECT(dw_area), "expose_event",
	    G_CALLBACK(expose_pw), NULL);

	gtk_widget_show_all(table);
	gtk_widget_show(pw_win);
}

void
UIPaintMapField(UInt16 xpos, UInt16 ypos, welem_t elem)
{
	if (pmh.allmap == NULL)
		doPixPaint();

	UIDrawMapZone(xpos, ypos, elem,
	    GDK_DRAWABLE(pmh.allmap));
}

void
UIPaintMapStatus(UInt16 xpos, UInt16 ypos, welem_t world, selem_t status)
{
	updatePower(xpos, ypos, checkCommon(world, status, &CarryPower,
		    POWEREDBIT));
	updateWater(xpos, ypos, checkCommon(world, status, &CarryWater,
		    WATEREDBIT));
}

/*!
 * \brief resize the map ... need to recreate all the elements
 */
void
resizeMap(void)
{
	cleanupMap();
	doPixPaint();
}
