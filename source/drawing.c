#include "ui.h"
#include "zakdef.h"
#include "globals.h"
#include "drawing.h"

unsigned char GetGraphicNumber(long unsigned int pos);

void SetUpGraphic(void)
{
    UISetUpGraphic();
}

extern void Goto(int x, int y)
{
    game.map_xpos = x-(game.visible_x/2);
    game.map_ypos = y-(game.visible_y/2);
    if (game.map_ypos < 0) { game.map_ypos = 0; }
    if (game.map_ypos > (GetMapSize() - game.visible_y))
        game.map_ypos = GetMapSize() - game.visible_y;
    if (game.map_xpos < 0) { game.map_xpos = 0; }
    if (game.map_xpos > (GetMapSize() - game.visible_x))
      game.map_xpos = GetMapSize() - game.visible_x;
    RedrawAllFields();
}

extern void RedrawAllFields(void)
{
    int i,j;

    LockWorld();
    LockWorldFlags();
    UIInitDrawing();
    UILockScreen();
    for (i=game.map_xpos; i<game.visible_x+game.map_xpos; i++) {
        for (j=game.map_ypos; j<game.visible_y+game.map_ypos; j++) {
            DrawFieldWithoutInit(i,j);
        }
    }

    UIDrawCursor(game.cursor_xpos-game.map_xpos, game.cursor_ypos-game.map_ypos);
    UIDrawCredits();
    UIDrawPop();

    UIUnlockScreen();
    UIFinishDrawing();
    UnlockWorldFlags();
    UnlockWorld();
}

extern void ScrollMap(int direction)
{
    switch (direction) {
        case 0: // up
            if (game.map_ypos > 0) {
                game.map_ypos -= 1;
            } else {
                game.map_ypos = 0;
                return;
            }
            break;
        case 1: // right
            if (game.map_xpos <= (GetMapSize() - 1 - game.visible_x)) {
                game.map_xpos += 1;
            } else {
                game.map_xpos = GetMapSize() - game.visible_x;
                return;
            }
            break;
        case 2: // down
            if (game.map_ypos <= (GetMapSize() - 1 - game.visible_y)) {
                game.map_ypos += 1;
            } else {
                game.map_ypos = GetMapSize() - game.visible_y;
                return;
            }
            break;
        case 3: // left
            if (game.map_xpos > 0) {
                game.map_xpos-=1; 
            } else {
                game.map_xpos=0; 
                return; 
            }
            break;
        default:
            return;
    }

    UIScrollMap(direction);
}


extern void MoveCursor(int direction)
{
    int old_x = game.cursor_xpos, old_y = game.cursor_ypos;
    LockWorld();

    switch (direction) {
    case 0: // up
        if (game.cursor_ypos > 0) { game.cursor_ypos--; }
        if ((game.cursor_ypos < game.map_ypos)) { ScrollMap(0); }
        break;
    case 1: // right
        if (game.cursor_xpos < (GetMapSize() - 1)) { game.cursor_xpos++; }
        if ((game.cursor_xpos > game.map_xpos+game.visible_x-1) && game.cursor_xpos < GetMapSize()) { ScrollMap(1); }
        break;
    case 2: // down
        if (game.cursor_ypos < (GetMapSize() - 1)) { game.cursor_ypos++; }
        if ((game.cursor_ypos > game.map_ypos+game.visible_y-1) && game.cursor_ypos < GetMapSize()) { ScrollMap(2); }
        break;
    case 3: // left
        if (game.cursor_xpos > 0) { game.cursor_xpos--; }
        if ((game.cursor_xpos < game.map_xpos)) { ScrollMap(3); }
        break;
    }

    DrawField(old_x, old_y);
    DrawField(game.cursor_xpos, game.cursor_ypos);

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
    if (xpos+1 < GetMapSize()) { DrawFieldWithoutInit(xpos+1, ypos);}
    if (ypos+1 < GetMapSize()) { DrawFieldWithoutInit(xpos, ypos+1);}
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
    if (xpos < game.map_xpos ||
            xpos >= game.map_xpos+game.visible_x ||
            ypos < game.map_ypos ||
            ypos >= game.map_ypos+game.visible_y)
    {
        return;
    }

    UIDrawField(xpos-game.map_xpos, ypos-game.map_ypos, GetGraphicNumber(WORLDPOS(xpos,ypos)));

    if ((GetWorldFlags(WORLDPOS(xpos,ypos)) & 0x01) == 0 && CarryPower(GetWorld(WORLDPOS(xpos, ypos)))) {
        UIDrawPowerLoss(xpos-game.map_xpos, ypos-game.map_ypos);
    }
    
    if ((GetWorldFlags(WORLDPOS(xpos,ypos)) & 0x04) == 0 && CarryWater(GetWorld(WORLDPOS(xpos, ypos)))) {
        UIDrawWaterLoss(xpos-game.map_xpos, ypos-game.map_ypos);
    }

    if (xpos == game.cursor_xpos && ypos == game.cursor_ypos) {
        UIDrawCursor(game.cursor_xpos-game.map_xpos, game.cursor_ypos-game.map_ypos);
    }

    // draw monster
    for (i=0; i<NUM_OF_OBJECTS; i++) {
        if (xpos == game.objects[i].x &&
            ypos == game.objects[i].y &&
            game.objects[i].active != 0) {
            UIDrawSpecialObject(i, xpos - game.map_xpos, ypos - game.map_ypos);
        }
    }
    // draw extra units
    for (i=0; i<NUM_OF_UNITS; i++) {
        if (xpos == game.units[i].x &&
            ypos == game.units[i].y &&
            game.units[i].active != 0) {
            UIDrawSpecialUnit(i, xpos - game.map_xpos, ypos - game.map_ypos);
        }
    }
}

unsigned char GetGraphicNumber(long unsigned int pos)
{
    unsigned char retval = 0;
    LockWorld();
    retval = GetWorld(pos);
    switch (retval) {
        case TYPE_ROAD:    // special case: roads
            retval = GetSpecialGraphicNumber(pos, 0);
            break;
        case TYPE_POWER_LINE:     // special case: power line
            retval = GetSpecialGraphicNumber(pos,1);
            break;
        case TYPE_WATER_PIPE:
            retval = GetSpecialGraphicNumber(pos,3);
            break;            
        case TYPE_BRIDGE:     // special case: bridge
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
    /* type: 0 = road
     *       1 = power line
     *       2 = bridge
     *       3 = water pipe
     */
    int a=0,b=0,c=0,d=0;
    int nAddMe = 0;

    LockWorld();

    switch (nType) {
        case 0: // roads
        case 2: // bridge
            if (pos >= GetMapSize()) a = IsRoad(GetWorld(pos - GetMapSize()));
            if (pos < (GetMapMul()) - GetMapSize())
                c = IsRoad(GetWorld(pos + GetMapSize()));
            if (pos % GetMapSize() < (GetMapSize() - 1))
                b = IsRoad(GetWorld(pos+1));
            if (pos % GetMapSize() > 0) d = IsRoad(GetWorld(pos-1));
            // 81 for bridge, 0 for normal road
            nAddMe = nType== 2 ? TYPE_BRIDGE : 10;
            break;
        case 1:    // power lines
            if (pos >= GetMapSize())
                a = CarryPower(GetWorld(pos - GetMapSize()));
            if (pos < GetMapMul() - GetMapSize())
                c = CarryPower(GetWorld(pos + GetMapSize()));
            if (pos % GetMapSize() < GetMapSize() - 1)
                b = CarryPower(GetWorld(pos+1));
            if (pos % GetMapSize() > 0) d = CarryPower(GetWorld(pos-1));
            nAddMe = 70;
            break;
        case 3: // water pipe
            if (pos >= GetMapSize())
                a = CarryWater(GetWorld(pos - GetMapSize()));
            if (pos < GetMapMul() - GetMapSize())
                c = CarryWater(GetWorld(pos + GetMapSize()));
            if (pos % GetMapSize() < GetMapSize() - 1)
                b = CarryWater(GetWorld(pos+1));
            if (pos % GetMapSize() > 0) d = CarryWater(GetWorld(pos-1));
            nAddMe = 92;
            break;
        default:
            return 0;
    }

    UnlockWorld(); 

    if ((a && b && c && d) == 1)      { return 10+nAddMe; }
    if ((a && b && d) == 1)           { return 9+nAddMe; }
    if ((b && c && d) == 1)           { return 8+nAddMe; }
    if ((a && b && c) == 1)           { return 7+nAddMe; }
    if ((a && c && d) == 1)           { return 6+nAddMe; }
    if ((a && b) == 1)                { return 5+nAddMe; }
    if ((a && d) == 1)                { return 4+nAddMe; }
    if ((c && d) == 1)                { return 3+nAddMe; }
    if ((c && b) == 1)                { return 2+nAddMe; }
    if ((a || c) == 1)                { return 1+nAddMe; }

    return nAddMe;

}

/* some small usefull functions */
extern int CarryPower(unsigned char x)         { return ((x)>=1&&(x)<=7&&(x)!=4) || ((x)>=23&&(x)<=61)  ?1:0; }
extern int CarryWater(unsigned char x)         { return ((x)>=1&&(x)<=3)||((x)>=23&&(x)<=61)||(x)==68||(x)==69||(x)==8?1:0; }
extern int IsPowerLine(unsigned char x)        { return ((x)>=5 && (x)<=7)  ?1:0; }
extern int IsRoad(unsigned char x)             { return ((x)==4||(x)==6||(x)==7||(x)==68||(x)==69||(x)==81)  ?1:0; }

extern int IsZone(unsigned char x, int nType)
{
    if (x == nType) { return 1; }
    else if (x >= (nType*10+20) && x <= (nType*10+29)) { return (x%10)+1; }
    return 0;
}
