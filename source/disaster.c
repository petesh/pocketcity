#include <handler.h>
#include <drawing.h>
#include <zakdef.h>
#include <ui.h>
#include <globals.h>
#include <handler.h>
#include <build.h>
#include <disaster.h>
#include <simulation.h>

#if defined(PALM)
#include <StringMgr.h>
#include <unix_stdio.h>
#else
#include <stdio.h>
#endif

void FireSpread(int x, int y);
void CreateWaste(int x, int y);
unsigned short int GetDefenceValue(int xpos, int ypos);
unsigned short int ContainsDefence(int x, int y);
void MonsterCheckSurrounded(int i);
void CreateMeteor(int x, int y, int size);
        
extern void DoNastyStuffTo(int type, unsigned int probability)
{
    /* nasty stuff means: turn it into wasteland */
    long unsigned int randomTile;
    int i,x,y;

    if (GetRandomNumber(probability) != 0) { return; } /* nothing happened :( */

    LockWorld();
    for (i=0; i<50; i++) {
        randomTile = GetRandomNumber(GetMapMul());
        if (GetWorld(randomTile) == type) {
            /* wee, let's destroy something */
            x = randomTile % GetMapSize();
            y = randomTile / GetMapSize();
            CreateWaste(x,y);
            UnlockWorld();
            return;
        }
    }
    UnlockWorld();
    return;
}

void
DoRandomDisaster(void)
{
    unsigned long randomTile;
    int i,x,y, type, random;
#ifdef DEBUG
    char temp[10];
#endif
    /* for those who can't handle the game (truth?) */
    if (game.disaster_level == 0) { return; }

    LockWorld();

    for (i=0; i<100; i++) { /* give em' a 100 tries to hit a useful tile */
        randomTile = GetRandomNumber(GetMapMul());
        type = GetWorld(randomTile);
        if (type != TYPE_DIRT &&
            type != TYPE_REAL_WATER &&
            type != TYPE_CRATER) {
            x = randomTile % GetMapSize();
            y = randomTile / GetMapSize();
	    /* TODO: should depend on difficulty */
            random = GetRandomNumber((4-game.disaster_level) * 1000);
#ifdef DEBUG
            sprintf(temp, "%d", random);
            UIWriteLog("Random disaster: ");
            UIWriteLog(temp);
            UIWriteLog("\n");
#endif
            if (random < 10 && vgame.BuildCount[COUNT_FIRE] == 0) {
                DoSpecificDisaster(diFireOutbreak);
            } else if (random < 15 && game.objects[OBJ_MONSTER].active == 0) {
                DoSpecificDisaster(diMonster);
            } else if (random < 17 && game.objects[OBJ_DRAGON].active == 0) {
                DoSpecificDisaster(diDragon);
            } else if (random < 19) {
                DoSpecificDisaster(diMeteor);
            }
            UnlockWorld();
            return; /* only one chance for disaster per turn */
        }
    }
    UnlockWorld();
}

void
DoSpecificDisaster(erdiType disaster)
{
    unsigned long randomTile;
    unsigned int x, y;
    int ce = 0;
    int i = 0;

    while (i++ < 400 && ce == 0) {
        randomTile = GetRandomNumber(GetMapMul());
        x = randomTile % GetMapSize();
        y = randomTile / GetMapSize();
        
        switch (disaster) {
        case diFireOutbreak:
            ce = BurnField(x, y, 0);
            break;
        case diPlantExplosion:
            ce = 0;
            break;
        case diMonster:
            ce = CreateMonster(x, y);
            break;
        case diDragon:
            ce = CreateDragon(x, y);
            break;
        case diMeteor:
            ce = MeteorDisaster(x, y);
            break;
        default: break;
        }
    }
    if (ce) {
        UIDisplayError(disaster);
        Goto(x,y);
        MapHasJumped();
    }
}

int
UpdateDisasters(void)
{
    /* return false if no disasters are found */
    int i,j,type, retval=0;

    LockWorld();
    LockWorldFlags();
    ClearScratch();
    for (i=0; i<GetMapSize(); i++) {
        for (j=0; j<GetMapSize(); j++) {
            type = GetWorld(WORLDPOS(i,j));
	    /* already looked at this one? */
            if (GetScratch(WORLDPOS(i,j)) == 0) {
                if (type == TYPE_FIRE2) {
                    retval = 1;
                    if (GetRandomNumber(5) != 0) {
			/* hmm, are there any defences here? */
                        if (GetDefenceValue(i,j) < 3) {
                            FireSpread(i,j);
                        }
                        SetWorld(WORLDPOS(i,j),TYPE_FIRE3);
                    } else {
                        CreateWaste(i,j);
                    }
                } else if (type == TYPE_FIRE1) {
                    retval = 1;
                    SetWorld(WORLDPOS(i,j),TYPE_FIRE2);
                } else if (type == TYPE_FIRE3) {
                    retval = 1;
                    CreateWaste(i,j);
                }
            }
        }
    }
    UnlockWorldFlags();
    UnlockWorld();
    return retval;
}

void CreateWaste(int x, int y)
{
    int type;
    LockWorld();
    type = GetWorld(WORLDPOS(x,y));
    Build_Destroy(x,y);
    if (type == TYPE_REAL_WATER || type == TYPE_BRIDGE) {
        UnlockWorld();
        return;
    }
    SetWorld(WORLDPOS(x,y), TYPE_WASTE);
    vgame.BuildCount[COUNT_WASTE]++;
    DrawCross(x,y);
    UnlockWorld();
    if (type == TYPE_POWER_PLANT || type == TYPE_NUCLEAR_PLANT)  {
        UIDisplayError(diPlantExplosion);
        FireSpread(x,y);
    }
}

void FireSpread(int x, int y) 
{
    if (x > 0) { BurnField(x-1,y,0); }
    if (x < GetMapSize()-1) { BurnField(x+1,y,0); }
    if (y > 0) { BurnField(x,y-1,0); }
    if (y < GetMapSize()-1) { BurnField(x,y+1,0); }
}


int
BurnField(int x, int y, int forceit)
{
    int type;
    
    LockWorld();
    LockWorldFlags();
    type = GetWorld(WORLDPOS(x,y));
    if ((forceit != 0 &&
         type != TYPE_BRIDGE &&
         type != TYPE_REAL_WATER)
        ||
        (type != TYPE_FIRE1 &&
        type != TYPE_FIRE2 &&
        type != TYPE_FIRE3 &&
        type != TYPE_DIRT  &&
        type != TYPE_WASTE &&
        type != TYPE_WATER &&
        type != TYPE_REAL_WATER &&
        type != TYPE_CRATER &&
        type != TYPE_BRIDGE &&
        ContainsDefence(x,y) == 0)) {

        Build_Destroy(x,y);
        SetWorld(WORLDPOS(x,y), TYPE_FIRE1);
        SetScratch(WORLDPOS(x,y));
        DrawCross(x,y);
        vgame.BuildCount[COUNT_FIRE]++;
        UnlockWorldFlags();
        UnlockWorld();
        return 1;
    }
    UnlockWorldFlags();
    UnlockWorld();
    return 0;
}


int
CreateMonster(int x, int y)
{
    /* return true on success, false on error */
    int type;
    LockWorld();
    type = GetWorld(WORLDPOS(x,y));
    UnlockWorld();
    if (type != TYPE_REAL_WATER && type != TYPE_CRATER) {
        game.objects[OBJ_MONSTER].x = x;
        game.objects[OBJ_MONSTER].y = y;
        game.objects[OBJ_MONSTER].dir = GetRandomNumber(8);
        game.objects[OBJ_MONSTER].active = 1;
        DrawField(x,y);
        return 1;
    }
    return 0;
}

int
CreateDragon(int x, int y)
{
    /* return true on success, false on error */
    int type;
    LockWorld();
    type = GetWorld(WORLDPOS(x,y));
    UnlockWorld();
    if (type != TYPE_REAL_WATER && type != TYPE_CRATER) {
        game.objects[OBJ_DRAGON].x = x;
        game.objects[OBJ_DRAGON].y = y;
        game.objects[OBJ_DRAGON].dir = GetRandomNumber(8);
        game.objects[OBJ_DRAGON].active = 1;
        DrawField(x,y);
        return 1;
    }
    return 0;
}


void
MonsterCheckSurrounded(int i) 
{
    if (GetDefenceValue(game.objects[i].x,game.objects[i].y) >= 11 ||
	GetRandomNumber(50) < 2) {
        game.objects[i].active = 0; /* kill the sucker */
        UIWriteLog("killing a monster\n");
    }
}

unsigned short int GetDefenceValue(int xpos, int ypos)
{
    /* police = 2 */
    /* firemen = 3 */
    /* military = 6 */
    short int def = 
    ContainsDefence(xpos+1, ypos)+
    ContainsDefence(xpos+1, ypos+1)+
    ContainsDefence(xpos+1, ypos-1)+
    ContainsDefence(xpos,ypos+1)+
    ContainsDefence(xpos,ypos-1)+
    ContainsDefence(xpos-1,ypos)+
    ContainsDefence(xpos-1,ypos+1)+
    ContainsDefence(xpos-1,ypos+1);
    return def;
}

unsigned short int ContainsDefence(int x, int y)
{
    int i;
    for (i=0; i<NUM_OF_UNITS; i++) {
        if (game.units[i].x == x &&
            game.units[i].y == y &&
            game.units[i].active != 0) {
            switch(game.units[i].type) {
                case DuPolice: return 2;
                case DuFireman: return 3;
                case DuMilitary: return 6;
                default: return 0;
            }
        }
    }
    return 0;
}

extern void MoveAllObjects(void)
{
    int i,x,y;
    for (i=0; i<NUM_OF_OBJECTS; i++) {
        if (game.objects[i].active != 0) {

            /* hmm, is this thing destructive? */
            if (i == OBJ_DRAGON) {
                /* sure it is :D */
                if (!BurnField(game.objects[i].x, game.objects[i].y,1)) {
                    CreateWaste(game.objects[i].x, game.objects[i].y);
                }
                MonsterCheckSurrounded(i);
            } else if (i == OBJ_MONSTER) {
                /* whoo-hoo, bingo again */
                CreateWaste(game.objects[i].x, game.objects[i].y);
                MonsterCheckSurrounded(i);
            }

            x = game.objects[i].x; /* save old position */
            y = game.objects[i].y;

            switch(GetRandomNumber(OBJ_CHANCE_OF_TURNING))
            { /* should we let it turn? */
                case 1: /* yes, clockwise */
                    game.objects[i].dir = (game.objects[i].dir+1)%8;
                    break;
                case 2: /* yes, counter-clockwise */
                    game.objects[i].dir = (game.objects[i].dir+7)%8;
                    break;
                default: break; 
            }
            /* now move it a nod */
            switch(game.objects[i].dir) { /* first up/down */
                case 0: /* up */
                case 1: /* up-right */
                case 7: /* up-left */
                    if (game.objects[i].y > 0) {
                        game.objects[i].y--;
                    } else {
                        game.objects[i].dir = 4;
                    }
                    break;
                case 3: /* down-right */
                case 4: /* down */
                case 5: /* down-left */
                    if (game.objects[i].y < GetMapSize()-1) { 
                        game.objects[i].y++; 
                    } else {
                        game.objects[i].dir = 0;
                    }
                    break;
                default:
                    break;
            }

            switch(game.objects[i].dir) { /* then left/right */
                case 1: /* up-right */
                case 2: /* right */
                case 3: /* down-right */
                    if (game.objects[i].x < GetMapSize()-1) { 
                        game.objects[i].x++; 
                    } else {
                        game.objects[i].dir = 6;
                    }
                    break;
                case 5: /* down-left */
                case 6: /* left */
                case 7: /* up-left */
                    if (game.objects[i].x > 0) { 
                        game.objects[i].x--; 
                    } else {
                        game.objects[i].dir = 2;
                    }
                    break;
                default:
                    break;
            }
            DrawCross(x, y); /* draw where it came from (erase it) */
            DrawField(game.objects[i].x, game.objects[i].y); /* draw object */
        }
    }
}

extern int MeteorDisaster(int x, int y)
{
    int k;
    k = GetRandomNumber(3)+1;
    CreateMeteor(x,y,k);
    return 1;
}

void CreateMeteor(int x, int y, int size)
{
    int i,j;
    LockWorld();
    UILockScreen();
    for (i=x-size; i<=x+size; i++) {
        for (j=y-size; j<=y+size; j++) {
            if (i >= 0 && i < GetMapSize() && j >= 0 && j < GetMapSize()) {
                if (GetRandomNumber(5) < 2) {
                    if (GetWorld(WORLDPOS(i,j)) != TYPE_REAL_WATER) {
                        CreateWaste(i,j);
                    }
                } else if (GetRandomNumber(5) < 4) {
                    if (GetWorld(WORLDPOS(i,j)) != TYPE_REAL_WATER && GetWorld(WORLDPOS(i,j)) != TYPE_WATER) {
                        BurnField(i,j,1);
                    }
                }
            }
        }
    }
    Build_Destroy(x,y);
    SetWorld(WORLDPOS(x,y), TYPE_CRATER);
    UIUnlockScreen();            
    UnlockWorld();
    RedrawAllFields();
}

