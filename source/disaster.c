#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"
#include "build.h"
#include "disaster.h"


void FireSpread(int x, int y);
void CreateWaste(int x, int y);
unsigned short int GetDefenceValue(int xpos, int ypos);
unsigned short int ContainsDefence(int x, int y);
void MonsterCheckSurrounded(int i);
        


extern void DoNastyStuffTo(int type, unsigned int probability)
{
    // nasty stuff means: turn it into wasteland
    long unsigned int randomTile;
    int i,x,y;

    if (GetRandomNumber(probability) != 0) { return; } // nothing happened :(

    LockWorld();
    for (i=0; i<50; i++) {
        randomTile = GetRandomNumber(mapsize*mapsize);
        if (GetWorld(randomTile) == type) {
            // wee, let's destroy something
            x = randomTile % mapsize;
            y = randomTile / mapsize;
            CreateWaste(x,y);
            UnlockWorld();
            return;
        }
    }
    UnlockWorld();
    return;
}



extern void DoRandomDisaster(void)
{
    long unsigned int randomTile;
    int i,x,y, type,random;
#ifdef DEBUG
    char temp[10];
#endif

    LockWorld();

    for (i=0; i<100; i++) {
        randomTile = GetRandomNumber(mapsize*mapsize);
        type = GetWorld(randomTile);
        if (type != TYPE_DIRT &&
            type != TYPE_REAL_WATER &&
	    type != TYPE_CRATER) {
            x = randomTile % mapsize;
            y = randomTile / mapsize;
            random = GetRandomNumber(500); // TODO: shuld depend on difficulty
#ifdef DEBUG
            LongToString(random, temp);
            UIWriteLog("Random disaster: ");
            UIWriteLog(temp);
            UIWriteLog("\n");
#endif
            if (random < 10) {
                if (BurnField(x,y,0)) {
                    UIDisplayError(ERROR_FIRE_OUTBREAK);
                }
            } else if (random < 15) {
                if (CreateMonster(x,y)) {
                    UIDisplayError(ERROR_MONSTER);
                }
            } else if (random < 17) {
                if (CreateDragon(x,y)) {
                    UIDisplayError(ERROR_DRAGON);
                }
            } else if (random < 19) {
		if (MeteorDisaster()) {
		    UIDisplayError(ERROR_METEOR);
		}
	    }	
            UnlockWorld();
            return; // only one chance for disaster per turn
        }
    }
    UnlockWorld();
}


extern int UpdateDisasters(void)
{
    // return false if no disasters are found
    int i,j,type, retval=0;

    LockWorld();
    LockWorldFlags();
    for (j=0; j<mapsize*mapsize; j++) { SetWorldFlags(j, GetWorldFlags(j) & 0xfc); } // clear used flag
    for (i=0; i<mapsize; i++) {
        for (j=0; j<mapsize; j++) {
            type = GetWorld(WORLDPOS(i,j));
            if ((GetWorldFlags(WORLDPOS(i,j)) & 0x02) == 0) { // already looked at this one?
                if (type == TYPE_FIRE2) {
                    retval = 1;
                    if (GetRandomNumber(5) != 0) {
                        if (GetDefenceValue(i,j) < 3) { // hmm, are there any defence here? :)
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
    if (!(type == TYPE_REAL_WATER || type == TYPE_BRIDGE)) {
        SetWorld(WORLDPOS(x,y), TYPE_WASTE);
    }
    DrawCross(x,y);
    BuildCount[COUNT_WASTE]++;
    UnlockWorld();
    if (type == TYPE_POWER_PLANT || type == TYPE_NUCLEAR_PLANT)  {
        UIDisplayError(ERROR_PLANT_EXPLOSION);
        FireSpread(x,y);
    }
}

void FireSpread(int x, int y) 
{
    if (x > 0) { BurnField(x-1,y,0); }
    if (x < mapsize-1) { BurnField(x+1,y,0); }
    if (y > 0) { BurnField(x,y-1,0); }
    if (y < mapsize-1) { BurnField(x,y+1,0); }
}


extern int BurnField(int x, int y, int forceit)
{
    int type;
    
    LockWorld();
    LockWorldFlags();
    type = GetWorld(WORLDPOS(x,y));
    if (forceit != 0 ||
        (type != TYPE_FIRE1 &&
        type != TYPE_FIRE2 &&
        type != TYPE_FIRE3 &&
        type != TYPE_DIRT  &&
        type != TYPE_WASTE &&
        type != TYPE_WATER &&
        type != TYPE_REAL_WATER &&
	type != TYPE_CRATER &&
        ContainsDefence(x,y) == 0)) {
        Build_Destroy(x,y);
        SetWorld(WORLDPOS(x,y), TYPE_FIRE1);
        SetWorldFlags(WORLDPOS(x,y), GetWorldFlags(WORLDPOS(x,y)) | 0x02); // set used flag
        DrawCross(x,y);
        BuildCount[COUNT_FIRE]++;
        UnlockWorldFlags();
        UnlockWorld();
        return 1;
    }
    UnlockWorldFlags();
    UnlockWorld();
    return 0;
}


extern int CreateMonster(int x, int y)
{
    // return true on success, false on error
    int type;
    LockWorld();
    type = GetWorld(WORLDPOS(x,y));
    UnlockWorld();
    if (type != TYPE_REAL_WATER && type != TYPE_CRATER) {
        objects[OBJ_MONSTER].x = x;
        objects[OBJ_MONSTER].y = y;
        objects[OBJ_MONSTER].dir = GetRandomNumber(8);
        objects[OBJ_MONSTER].active = 1;
        DrawField(x,y);
        return 1;
    }
    return 0;
}

extern int CreateDragon(int x, int y)
{
    // return true on success, false on error
    int type;
    LockWorld();
    type = GetWorld(WORLDPOS(x,y));
    UnlockWorld();
    if (type != TYPE_REAL_WATER && type != TYPE_CRATER) {
        objects[OBJ_DRAGON].x = x;
        objects[OBJ_DRAGON].y = y;
        objects[OBJ_DRAGON].dir = GetRandomNumber(8);
        objects[OBJ_DRAGON].active = 1;
        DrawField(x,y);
        return 1;
    }
    return 0;
}


void MonsterCheckSurrounded(int i) 
{
    if (GetDefenceValue(objects[i].x,objects[i].y) >= 11) {
        objects[i].active = 0; // kill the sucker
        UIWriteLog("killing a monster\n");
    }
}

unsigned short int GetDefenceValue(int xpos, int ypos)
{
    // police = 2
    // firemen = 3
    // military = 6
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
        if (units[i].x == x &&
            units[i].y == y &&
            units[i].active != 0) {
            switch(units[i].type) {
                case DEFENCE_POLICE: return 2;
                case DEFENCE_FIREMEN: return 3;
                case DEFENCE_MILITARY: return 6;
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
        if (objects[i].active != 0) {

            // hmm, is this thing destructive?
            if (i == OBJ_DRAGON) {
                // sure it is :D
                if (!BurnField(objects[i].x, objects[i].y,1)) {
                    CreateWaste(objects[i].x, objects[i].y);
                }
                MonsterCheckSurrounded(i);
            } else if (i == OBJ_MONSTER) {
                // whoo-hoo, bingo again
                CreateWaste(objects[i].x, objects[i].y);
                MonsterCheckSurrounded(i);
            }

            x = objects[i].x; // save old position
            y = objects[i].y;

            switch(GetRandomNumber(OBJ_CHANCE_OF_TURNING))
            { // should we let it turn?
                case 1: // yes, clockwise
                    objects[i].dir = (objects[i].dir+1)%8;
                    break;
                case 2: // yes, counter-clockwise
                    objects[i].dir = (objects[i].dir+7)%8;
                    break;
                default: // nope
                    break; 
            }
            // now move it a nod
            switch(objects[i].dir) { // first up/down
                case 0: // up
                case 1: // up-right
                case 7: // up-left
                    if (objects[i].y > 0) {
                        objects[i].y--;
                    } else {
                        objects[i].dir = 4;
                    }
                    break;
                case 3: // down-right
                case 4: // down
                case 5: // down-left
                    if (objects[i].y < mapsize-1) { 
                        objects[i].y++; 
                    } else {
                        objects[i].dir = 0;
                    }
                    break;
                default:
                    break;
            }

            switch(objects[i].dir) { // then left/right
                case 1: // up-right
                case 2: // right
                case 3: // down-right
                    if (objects[i].x < mapsize-1) { 
                        objects[i].x++; 
                    } else {
                        objects[i].dir = 6;
                    }
                    break;
                case 5: // down-left
                case 6: // left
                case 7: // up-left
                    if (objects[i].x > 0) { 
                        objects[i].x--; 
                    } else {
                        objects[i].dir = 2;
                    }
                    break;
                default:
                    break;
            }
            DrawCross(x, y); // draw where it came from (erase it)
            DrawField(objects[i].x, objects[i].y); // draw object
        }
    }
}

int MeteorDisaster(void)
{
    int k;
    unsigned long int pos;
    k = GetRandomNumber(3)+1;
    pos = GetRandomNumber(mapsize*mapsize);
    CreateMeteor(pos, k);
    return 1;
}

void CreateMeteor(long unsigned int pos, int size)
{
    int x,y,i,j,s;
    LockWorld();
    x = pos % mapsize;
    y = pos / mapsize;
    i = x;
    j = y;
    for (i=x-size; i<=x+size; i++) {
        for (j=y-size; j<=y+size; j++) {
            if (i >= 0 && i < mapsize && j >= 0 && j < mapsize) {
                s = 5;
                if (GetRandomNumber(s) < 2) {
                    if(GetWorld(WORLDPOS(i,j)) != TYPE_REAL_WATER) {
                        SetWorld(WORLDPOS(i,j), TYPE_WASTE);
                        BuildCount[COUNT_WASTE]++;
                    }
                } else if (GetRandomNumber(s) < 4) {
                    if(GetWorld(WORLDPOS(i,j)) != TYPE_REAL_WATER && GetWorld(WORLDPOS(i,j)) != TYPE_WATER) {
                            BurnField(i,j,1);
                            BurnField(i,j,1);
                    }
                }
            }
        }
    }
    UnlockWorld();
    SetWorld(WORLDPOS(x,y), TYPE_CRATER);
    RedrawAllFields();
}

