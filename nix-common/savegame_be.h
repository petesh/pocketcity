
#if !defined(_SAVEGAME_BE_H)
#define _SAVEGAME_BE_H

#if defined(__cplusplus)
extern "C" {
#endif

int open_filename(char *name, int palm);
int save_filename(char *name, int palm);

#if defined(__cplusplus)
}
#endif

#endif /* _SAVEGAME_BE_H */
