#if !defined (_SAVEGAME_BE_H)
#define _SAVEGAME_BE_H

#include <palmutils.h>

int LoadAutoSave(void);
void DeleteAutoSave(void);
void SetAutoSave(char *name);
void SaveGameByName(char *name);
void DeleteGameByName(char *name);
int  LoadGameByName(char *name);
void CreateNewSaveGame(char *name);
int GameExists(char *name);

char **CityNames(int *count);
void FreeCityNames(char **names);

#endif /* _SAVEGAME_BE_H */
