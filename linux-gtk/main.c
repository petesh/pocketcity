#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "../source/ui.h"
#include "../source/handler.h"
#include "../source/drawing.h"
#include "../source/globals.h"
#include "../source/build.h"
#include "../source/simulation.h"
#include "../source/disaster.h"


GtkWidget *drawingarea;
GtkWidget *window;
GtkWidget *creditslabel;
GtkWidget *poplabel;
GtkWidget *timelabel;

void * worldPtr;
void * worldFlagsPtr;
GdkPixmap *zones,*monsters,*units;
GdkBitmap *zones_mask,*monsters_mask,*units_mask;
unsigned char selectedBuildItem = 0;
unsigned short drawing = 0;


void SetUpMainWindow(void);
gint mainloop_callback(gpointer data);

int main(int argc, char *argv[])
{
    gint timerID;
    gtk_init (&argc, &argv);
    srand(time(0));

    SetUpMainWindow();
    // start the timer
    timerID = gtk_timeout_add(1000, (mainloop_callback),0);

    gtk_main();
    g_print("Cleaning up\n");
    gtk_timeout_remove(timerID);
    free(worldPtr);
    free(worldFlagsPtr);
    return 0;
}

unsigned int timekeeper = 0;
unsigned int timekeeperdisaster = 0;

gint mainloop_callback(gpointer data)
{
    // this will be called every second
    unsigned int phase = 1;
    timekeeper++;
    timekeeperdisaster++;

    if (timekeeperdisaster >= SIM_GAME_LOOP_DISASTER) {
        MoveAllObjects();
        if (UpdateDisasters()) {
            gtk_widget_queue_draw(drawingarea);
        }
    }
    
    if (timekeeper >= game.gameLoopSeconds) {
        g_print("A month has gone by - total months: %lu\n", game.TimeElapsed);
        timekeeper = 0;
        do {
            phase = Sim_DoPhase(phase);
        } while (phase != 0);
        gtk_widget_queue_draw(drawingarea);
    }

    return TRUE; // yes, call us again in a sec
}

gint toolbox_callback(GtkWidget *widget, gpointer data)
{
    gint action = GPOINTER_TO_INT(data);
    if (action == 270) {
        CreateDragon(55,55);
        return FALSE;
    }
    if (action >= 260) {
        ScrollMap(action-260);
    } else {
        selectedBuildItem = action;
    }

    return FALSE;
}

gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gtk_main_quit();
    return FALSE;
}

static gint drawing_exposed_callback(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    drawing = 1;
    RedrawAllFields();
    drawing = 0;

    return FALSE;
}

static gint drawing_realized_callback(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    PCityMain();
    SetupNewGame();
    game.visible_x = 320/16;
    game.visible_y = 240/16;
    game.gameLoopSeconds = SPEED_FAST;
    return FALSE;
}


static gint button_press_event(GtkWidget *widget, GdkEventButton *event)
{
    if (event->button == 1) {
        BuildSomething(
                (int)(event->x / game.tileSize) + game.map_xpos,
                (int)(event->y / game.tileSize) + game.map_ypos);
    } else if (event->button == 3) {
        Build_Bulldoze(
                (int)(event->x / game.tileSize) + game.map_xpos,
                (int)(event->y / game.tileSize) + game.map_ypos);
    }

    return TRUE;
}

static gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
    int x, y;
    GdkModifierType state;

    if (event->is_hint) {
        gdk_window_get_pointer (event->window, &x, &y, &state);
    } else {
        x = event->x;
        y = event->y;
        state = event->state;
    }

    if (state & GDK_BUTTON1_MASK 
            && x > 0 && x < game.visible_x*game.tileSize
            && y > 0 && y < game.visible_y*game.tileSize) {
        BuildSomething(
                (int)(x / game.tileSize) + game.map_xpos,
                (int)(y / game.tileSize) + game.map_ypos);
    } else if (state & GDK_BUTTON3_MASK
            && x > 0 && x < game.visible_x*game.tileSize
            && y > 0 && y < game.visible_y*game.tileSize) {
        Build_Bulldoze(
                (int)(x / game.tileSize) + game.map_xpos,
                (int)(y / game.tileSize) + game.map_ypos);
    }

    return TRUE;
}


void SetUpMainWindow(void)
{
    GtkWidget *fieldbox,*box, *toolbox, *headerbox;
    GtkWidget *button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pocket City");
    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);
    
    box = gtk_hbox_new(FALSE,0);
    fieldbox = gtk_vbox_new(FALSE,0);
    toolbox = gtk_table_new(10,3,TRUE);
    headerbox = gtk_hbox_new(FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_container_set_border_width(GTK_CONTAINER(toolbox), 3);

    creditslabel = gtk_label_new("Credits");
    poplabel = gtk_label_new("Population");
    timelabel = gtk_label_new("Game Time");

    // the actual playfield is a GtkDrawingArea
    drawingarea = gtk_drawing_area_new();
    gtk_drawing_area_size((GtkDrawingArea*)drawingarea,320,240);
    // arange in boxes 
    gtk_box_pack_start(GTK_BOX(box), toolbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), fieldbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(fieldbox), headerbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(fieldbox), drawingarea, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(fieldbox), poplabel, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(headerbox), creditslabel, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(headerbox), timelabel, TRUE, TRUE, 0);

    g_signal_connect(G_OBJECT(drawingarea),"expose_event",
            G_CALLBACK(drawing_exposed_callback), NULL);

    g_signal_connect_after(G_OBJECT(drawingarea),"realize",
            G_CALLBACK(drawing_realized_callback), NULL);

    // set up some mouse events
    gtk_signal_connect(GTK_OBJECT(drawingarea),"motion_notify_event",
            (GtkSignalFunc) motion_notify_event, NULL);
    gtk_signal_connect(GTK_OBJECT(drawingarea),"button_press_event",
            (GtkSignalFunc) button_press_event, NULL);

    gtk_widget_set_events(drawingarea, GDK_EXPOSURE_MASK
             | GDK_LEAVE_NOTIFY_MASK
             | GDK_BUTTON_PRESS_MASK
             | GDK_POINTER_MOTION_MASK
             | GDK_POINTER_MOTION_HINT_MASK
             );

    // setup the toolbox
    {
        char labels[] = "Bu\0Ro\0Po\0"
                        "Re\0Co\0In\0"
                        "Tr\0Wa\0  \0"
                        "PP\0NP\0  \0"
                        "FS\0PS\0MB\0"
                        "  \0  \0  \0"
                        "DF\0DP\0DM\0"
                        "  \0/\\\0  \0"
                        "<-\0  \0->\0"
                        "  \0\\/\0Te\0";
        int i;
        gint actions[] = { BUILD_BULLDOZER,BUILD_ROAD,BUILD_POWER_LINE,
                            BUILD_ZONE_RESIDENTIAL,BUILD_ZONE_COMMERCIAL,BUILD_ZONE_INDUSTRIAL,
                            BUILD_TREE,BUILD_WATER,-1,
                            BUILD_POWER_PLANT,BUILD_NUCLEAR_PLANT,-1,
                            BUILD_FIRE_STATION,BUILD_POLICE_STATION,BUILD_MILITARY_BASE,
                            -1,-1,-1,
                            BUILD_DEFENCE_FIRE,BUILD_DEFENCE_POLICE,BUILD_DEFENCE_MILITARY,
                            -1,260,-1,
                            263,-1,261,
                            -1,262,270 };
                            
        for (i=0; i<30; i++) {
            if (*(labels+i*3) == ' ') { continue; }
            button = gtk_button_new_with_label(labels+i*3);
            g_signal_connect(G_OBJECT(button),"clicked",
                G_CALLBACK(toolbox_callback), GINT_TO_POINTER(actions[i]));
            gtk_table_attach_defaults(GTK_TABLE(toolbox), button, (i%3), (i%3)+1, (i/3), (i/3)+1);
            gtk_widget_show(button);
        }
    }


    
    // show all the widgets
    gtk_widget_show(timelabel);
    gtk_widget_show(poplabel);
    gtk_widget_show(creditslabel);
    gtk_widget_show(headerbox);
    gtk_widget_show(drawingarea);
    gtk_widget_show(fieldbox);
    gtk_widget_show(toolbox);
    gtk_widget_show(box);
    
    // finally, show the main window    
    gtk_widget_show(window);
}

extern void UISetUpGraphic(void)
{
    zones = gdk_pixmap_create_from_xpm(drawingarea->window,
                                       &zones_mask,
                                       NULL,
                                       "graphic/zones_16x16-color.xpm");
    monsters = gdk_pixmap_create_from_xpm(drawingarea->window,
                                       &monsters_mask,
                                       NULL,
                                       "graphic/monsters_16x16-color.xpm");
    units = gdk_pixmap_create_from_xpm(drawingarea->window,
                                       &units_mask,
                                       NULL,
                                       "graphic/units_16x16-color.xpm");
}


extern int UIDisplayError(int nError) 
{
    GtkWidget * dialog;
    char temp[100];

    switch (nError)
    {
        case ERROR_OUT_OF_MEMORY: strcpy(temp,"Out of memory"); break;
        case ERROR_OUT_OF_MONEY: strcpy(temp,"Out of money"); break;
        case ERROR_FIRE_OUTBREAK: strcpy(temp,"An Australian fire has broken out somewhere!"); break;
        case ERROR_PLANT_EXPLOSION: strcpy(temp,"A power plant just exploded!"); break;
        case ERROR_MONSTER: strcpy(temp,"Godzilla just came to town!"); break;
        case ERROR_DRAGON: strcpy(temp,"A fire dragon wants to use your city as it's lair!"); break;
        case ERROR_METEOR: strcpy(temp,"A gigantic meteor has hit your city!"); break;
    }
    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_OK,
                                  "%s",
                                  temp);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));
    return 0;
}

extern void UIInitDrawing(void)
{
}

extern void UIFinishDrawing(void)
{
}

extern void UIUnlockScreen(void)
{
    // not used for this platform
}

extern void UILockScreen(void)
{
    // not used for this platform
}

extern void UIDrawBorder(void)
{
    g_print("UIDrawBorder\n");
}

extern void UIDrawCredits(void)
{
    char temp[23];
    sprintf(temp, "$: %ld", game.credits);
    gtk_label_set_text((GtkLabel*)creditslabel, temp);
    GetDate((char*)temp);
    gtk_label_set_text((GtkLabel*)timelabel, temp);
}

extern void UIUpdateBuildIcon(void)
{
    g_print("UIUpdateBuildIcon\n");
}

extern void UIGotoForm(int n)
{
    g_print("UIGotoForm\n");
}

extern void UICheckMoney(void)
{
    g_print("UICheckMoney\n");
}

extern void UIScrollMap(int direction)
{
    // TODO: Optimize this as in the Palm port?
    //       (if nessasary)
    RedrawAllFields();
}

extern void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth)
{
    g_print("_UIDrawRect\n");
}

extern void UIDrawField(int xpos, int ypos, unsigned char nGraphic)
{
    if (drawing == 0) {
        gtk_widget_queue_draw(drawingarea);
    } else {
        GdkGC *gc;
    
        gc = gdk_gc_new(drawingarea->window);
        gdk_draw_drawable(
            drawingarea->window,
            gc,
            zones,
            (nGraphic%64)*game.tileSize,
            (nGraphic/64)*game.tileSize,
            xpos*game.tileSize,
            ypos*game.tileSize,
            game.tileSize,
            game.tileSize);
    }
}

extern void UIDrawSpecialObject(int i, int xpos, int ypos)
{
    if (drawing == 0) {
        gtk_widget_queue_draw(drawingarea);
    } else {
        GdkGC *gc;
        gc = gdk_gc_new(drawingarea->window);
        gdk_gc_set_clip_mask(gc,monsters_mask);
        gdk_gc_set_clip_origin(gc,
            xpos*game.tileSize-(game.objects[i].dir*game.tileSize),
            ypos*game.tileSize-(i*game.tileSize));

        gdk_draw_drawable(
            drawingarea->window,
            gc,
            monsters,
            game.objects[i].dir*game.tileSize,
            i*game.tileSize,
            xpos*game.tileSize,
            ypos*game.tileSize,
            game.tileSize,
            game.tileSize);
    }
}

extern void UIDrawSpecialUnit(int i, int xpos, int ypos)
{
    if (drawing == 0) {
        gtk_widget_queue_draw(drawingarea);
    } else {
        GdkGC *gc;
        gc = gdk_gc_new(drawingarea->window);
        gdk_gc_set_clip_mask(gc,units_mask);
        gdk_gc_set_clip_origin(gc,
            xpos*game.tileSize-(game.units[i].type*game.tileSize),
            ypos*game.tileSize);

        gdk_draw_drawable(
            drawingarea->window,
            gc,
            units,
            game.units[i].type*game.tileSize,
            0,
            xpos*game.tileSize,
            ypos*game.tileSize,
            game.tileSize,
            game.tileSize);
    }
}

extern void UIDrawCursor(int xpos, int ypos)
{
    // not used on this platform
}

extern void UIDrawPowerLoss(int xpos, int ypos)
{
    if (drawing == 0) {
        gtk_widget_queue_draw(drawingarea);
    } else {
        GdkGC *gc;
        gc = gdk_gc_new(drawingarea->window);
        gdk_gc_set_clip_mask(gc,zones_mask);
        gdk_gc_set_clip_origin(gc,
            xpos*game.tileSize-128,
            ypos*game.tileSize);

        gdk_draw_drawable(
            drawingarea->window,
            gc,
            zones,
            128,
            0,
            xpos*game.tileSize,
            ypos*game.tileSize,
            game.tileSize,
            game.tileSize);
    }
}

extern unsigned char UIGetSelectedBuildItem(void)
{
    return selectedBuildItem;
}

extern int InitWorld(void)
{
    worldPtr = malloc(10);
    worldFlagsPtr = malloc(10);

    if (worldPtr == NULL || worldFlagsPtr == NULL) {
        UIDisplayError(0);
        UIWriteLog("malloc failed - initworld\n");
        return 0;
    }
    return 1;
}

extern int ResizeWorld(long unsigned size)
{
    worldPtr = realloc(worldPtr, size);
    worldFlagsPtr = realloc(worldFlagsPtr, size);

    if (worldPtr == NULL || worldFlagsPtr == NULL) {
        UIDisplayError(0);
        UIWriteLog("realloc failed - resizeworld\n");
        return 0;
    }
    memset(worldPtr,0,size);
    memset(worldFlagsPtr,0,size);
    return 1;
}

extern void LockWorld(void)
{
    // not used on this platform
}

extern void UnlockWorld(void)
{
    // not used on this platform
}

extern unsigned char GetWorld(long unsigned int pos)
{
    if (pos > (game.mapsize*game.mapsize)) { return 0; }
    return ((char*)worldPtr)[pos];
}

extern void SetWorld(long unsigned int pos, unsigned char value)
{
    if (pos > (game.mapsize*game.mapsize)) { return; }
    ((char*)worldPtr)[pos] = value;
}

extern void LockWorldFlags(void)
{
    // not used on this platform
}

extern void UnlockWorldFlags(void)
{
    // not used on this platform
}

extern unsigned char GetWorldFlags(long unsigned int pos)
{
    if (pos > (game.mapsize*game.mapsize)) { return 0; }
    return ((char*)worldFlagsPtr)[pos];
}

extern void SetWorldFlags(long unsigned int pos, unsigned char value)
{
    if (pos > (game.mapsize*game.mapsize)) { return; }
    ((char*)worldFlagsPtr)[pos] = value;
}

extern unsigned long GetRandomNumber(unsigned long max)
{
    // se `man 3 rand` why I'm not using:
    // return (rand() % max)
    return (unsigned long)((float)max*rand()/(RAND_MAX+1.0));
}

extern void UISetTileSize(int size)
{
    g_print("UISetTileSize\n");
}

extern void UIDrawPop(void) 
{
    char temp[50];
    sprintf(temp, "(%02u,%02u) Population: %-9li",
            game.map_xpos,
            game.map_ypos,
            game.BuildCount[COUNT_RESIDENTIAL]*150);

    gtk_label_set_text((GtkLabel*)poplabel, temp);
}


extern void UIWriteLog(char* s)
{
    g_print(s); 
}


