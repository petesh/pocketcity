#if !defined(_DISASTER_H_)
#define _DISASTER_H_

void DoNastyStuffTo(int type, unsigned int probability);
void DoRandomDisaster();
int UpdateDisasters();
int BurnField(int x, int y,int forceit);
int CreateMonster(int x, int y);
int CreateDragon(int x, int y);
void MoveAllObjects(void);
int MeteorDisaster(int x, int y);
void DoSpecificDisaster(erdiType disaster);

#endif /* _DISASTER_H_ */
