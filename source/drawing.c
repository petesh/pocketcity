#include "ui.h"
#include "zakdef.h"
#include "globals.h"
#include "drawing.h"

unsigned char GetGraphicNumber(long unsigned int pos);

int CarryPower(unsigned char x);
int IsRoad(unsigned char x);
int IsPowerLine(unsigned char x);


void SetUpGraphic(void)
{
    UISetUpGraphic();
}


extern void RedrawAllFields(void)
{
    int i,j;

    LockWorld();
    LockWorldFlags();
    UIInitDrawing();
    UILockScreen();
    for (i=map_xpos; i<visible_x+map_xpos; i++)
    {
        for (j=map_ypos; j<visible_y+map_ypos; j++)
        {
            DrawFieldWithoutInit(i,j);
        }
    }

    UIDrawCursor(cursor_xpos-map_xpos, cursor_ypos-map_ypos);
    UIDrawCredits();
    UIDrawPop();

    UIUnlockScreen();
    UIFinishDrawing();
    UnlockWorldFlags();
    UnlockWorld();
}

extern void ScrollMap(int direction)
{
    switch (direction)
    {
        case 0: // up
            if (map_ypos > 0) { map_ypos-=1; } else {  map_ypos=0; return; }
            break;
        case 1: // right
            if (map_xpos <= (mapsize-1-visible_x)) { map_xpos+=1; } else { map_xpos = mapsize-visible_x; return; }
            break;
        case 2: // down
            if (map_ypos <= (mapsize-1-visible_y)) { map_ypos+=1; } else { map_ypos = mapsize-visible_y; return; }
            break;
        case 3: // left
            if (map_xpos > 0) { map_xpos-=1; } else { map_xpos=0; return; }
            break;
        default:
            return;
    }

    UIScrollMap(direction);

}


extern void MoveCursor(int direction)
{
    int old_x = cursor_xpos, old_y = cursor_ypos;
    LockWorld();

    switch (direction)
    {
        case 0: // up
            if (cursor_ypos > 0) { cursor_ypos--; }
            if ((cursor_ypos < map_ypos)) { ScrollMap(0); }
            break;
        case 1: // right
            if (cursor_xpos < mapsize-1) { cursor_xpos++; }
            if ((cursor_xpos > map_xpos+visible_x-1) && cursor_xpos < mapsize) { ScrollMap(1); }
            break;
        case 2: // down
            if (cursor_ypos < mapsize-1) { cursor_ypos++; }
            if ((cursor_ypos > map_ypos+visible_y-1) && cursor_ypos < mapsize) { ScrollMap(2); }
            break;
        case 3: // left
            if (cursor_xpos > 0) { cursor_xpos--; }
            if ((cursor_xpos < map_xpos)) { ScrollMap(3); }
            break;
    }

    DrawField(old_x, old_y);
    DrawField(cursor_xpos, cursor_ypos);

    UnlockWorld();
}


extern void DrawField(int xpos, int ypos)
{
    UIInitDrawing();
    LockWorld();
    LockWorldFlags();

    DrawFieldWithoutInit(xpos, ypos);
    
    UnlockWorldFlags();
    UnlockWorld();
    UIFinishDrawing();
}



extern void DrawCross(int xpos, int ypos)
{
    UIInitDrawing();
    LockWorld();
    LockWorldFlags();
    if (xpos > 0) { DrawFieldWithoutInit(xpos-1, ypos);}
    if (ypos > 0) { DrawFieldWithoutInit(xpos, ypos-1);}
    if (xpos+1 < mapsize) { DrawFieldWithoutInit(xpos+1, ypos);}
    if (ypos+1 < mapsize) { DrawFieldWithoutInit(xpos, ypos+1);}
    DrawFieldWithoutInit(xpos, ypos);
    UnlockWorld();
    UnlockWorldFlags();
    UIFinishDrawing();
}


// ONLY call this function if you make sure to call
// UIInitDrawing and UIFinishDrawing in the caller
// Also remember to call (Un)lockWorld(flags) (4 functions)
extern void DrawFieldWithoutInit(int xpos, int ypos)
{
    int i;
    if (xpos < map_xpos ||
            xpos >= map_xpos+visible_x ||
            ypos < map_ypos ||
            ypos >= map_ypos+visible_y)
    {
        return;
    }

    UIDrawField(xpos-map_xpos, ypos-map_ypos,GetGraphicNumber(WORLDPOS(xpos,ypos)));

    if ((GetWorldFlags(WORLDPOS(xpos,ypos)) & 0x01) == 0 && CarryPower(GetWorld(WORLDPOS(xpos, ypos))))
    {
        UIDrawPowerLoss(xpos-map_xpos, ypos-map_ypos);
    }

    if (xpos == cursor_xpos && ypos == cursor_ypos)
    {
        UIDrawCursor(cursor_xpos-map_xpos, cursor_ypos-map_ypos);
    }

    // draw monster
    for (i=0; i<NUM_OF_OBJECTS; i++) {
        if (xpos == objects[i].x &&
            ypos == objects[i].y &&
            objects[i].active != 0) {
            UIDrawSpecialObject(i, xpos - map_xpos, ypos - map_ypos);
        }
    }
    // draw extra units
    for (i=0; i<NUM_OF_UNITS; i++) {
        if (xpos == units[i].x &&
            ypos == units[i].y &&
            units[i].active != 0) {
            UIDrawSpecialUnit(i, xpos - map_xpos, ypos - map_ypos);
        }
    }

}

unsigned char GetGraphicNumber(long unsigned int pos)
{
    unsigned char retval = 0;
    LockWorld();
    retval = GetWorld(pos);
    switch (retval)
    {
        case 4:    // special case: roads
            retval = GetSpecialGraphicNumber(pos, 0);
            break;
        case 5:     // special case: power line
            retval = GetSpecialGraphicNumber(pos,1);
            break;
        case 81:     // special case: bridge
            retval = GetSpecialGraphicNumber(pos,2);
            break;
        default:
            break;
    }
    UnlockWorld();
    return retval;

}



extern unsigned char GetSpecialGraphicNumber(long unsigned int pos, int nType)
{
    int a=0,b=0,c=0,d=0;
    int nAddMe = 0;

    LockWorld();

    switch (nType)
    {
        case 0: // roads
        case 2: // bridge
            if (pos >= mapsize)                        { a = IsRoad(GetWorld(pos-mapsize));    }
            if (pos < (mapsize*mapsize)-mapsize)    { c = IsRoad(GetWorld(pos+mapsize));    }
            if (pos % mapsize < mapsize-1)            { b = IsRoad(GetWorld(pos+1));            }
            if (pos % mapsize > 0)                    { d = IsRoad(GetWorld(pos-1));            }
            nAddMe = nType==2?81:10; // 81 for bridge, 0 for normal road
            break;
        case 1:    // power lines
            if (pos >= mapsize)                        { a = CarryPower(GetWorld(pos-mapsize));    }
            if (pos < (mapsize*mapsize)-mapsize)    { c = CarryPower(GetWorld(pos+mapsize));    }
            if (pos % mapsize < mapsize-1)            { b = CarryPower(GetWorld(pos+1));            }
            if (pos % mapsize > 0)                    { d = CarryPower(GetWorld(pos-1));            }
            nAddMe = 70;
            break;
        default:
            return 0;
    }

    UnlockWorld(); 

    if ((a && b && c && d) == 1)    { return 10+nAddMe; }
    if ((a && b && d) == 1)            { return 9+nAddMe; }
    if ((b && c && d) == 1)            { return 8+nAddMe; }
    if ((a && b && c) == 1)            { return 7+nAddMe; }
    if ((a && c && d) == 1)            { return 6+nAddMe; }
    if ((a && b) == 1)                { return 5+nAddMe; }
    if ((a && d) == 1)                { return 4+nAddMe; }
    if ((c && d) == 1)                { return 3+nAddMe; }
    if ((c && b) == 1)                { return 2+nAddMe; }
    if ((a || c) == 1)                { return 1+nAddMe; }

    return nAddMe;

}


extern int CarryPower(unsigned char x)            { return ((x)>=1&&(x)<=7&&(x)!=4) || ((x)>=23&&(x)<=61)  ?1:0; }
extern int IsPowerLine(unsigned char x)        { return ((x)>=5 && (x)<=7)  ?1:0; }

extern int IsRoad(unsigned char x)        { return ((x)==4 || (x)==6 || (x)==7 || (x)==81)  ?1:0; }

extern int IsZone(unsigned char x, int nType)
{
    if (x == nType) { return 1; }
    else if (x >= (nType*10+20) && x <= (nType*10+29)) { return (x%10)+1; }
    return 0;
}
