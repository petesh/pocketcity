#if !defined (_SAVEGAME_BE_H)
#define _SAVEGAME_BE_H

#include <palmutils.h>
#include <sections.h>

int LoadAutoSave(void) MAP_SECTION;
void DeleteAutoSave(void) MAP_SECTION;
void SetAutoSave(char *name) MAP_SECTION;
void SaveGameByName(char *name) MAP_SECTION;
void DeleteGameByName(char *name) MAP_SECTION;
int  LoadGameByName(char *name) MAP_SECTION;
void CreateNewSaveGame(char *name) MAP_SECTION;
int GameExists(char *name) MAP_SECTION;

char **CityNames(int *count) MAP_SECTION;
void FreeCityNames(char **names) MAP_SECTION;

#endif /* _SAVEGAME_BE_H */
