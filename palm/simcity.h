#if !defined(_SIMCITY_H)
#define _SIMCITY_H

#include <zakdef.h>

/* global vars from simcity.c */
#define SGTYP   'DATA'
#define TILEDBTYPE 'tidb'
#define SGNAME  "PCitySave"

extern MemPtr worldPtr;
extern MemPtr worldFlagsPtr;

UInt8 GetItemClicked(void);
/* For the 'dialogs' */
void PauseGame(void);
void ResumeGame(void);
UInt8 IsGamePlaying(void);
/*
 * For the 'is there a game in progress'.
 * Faffing around with save games causes games to not be in progress.
 * Needed as separate concept from the Pause / Resume for autosave purposes.
 */
void SetGameInProgress(void);
void SetGameNotInProgress(void);
UInt8 IsGameInProgress(void);

void ClearNewROM(void);
void SetNewROM(void);
UInt8 IsNewROM(void);

#endif
