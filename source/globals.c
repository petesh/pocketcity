#include <zakdef.h>
#if defined(PALM)
#include <StringMgr.h>
#include <unix_stdio.h>
#else
#include <stdio.h>
#endif

/* this is the central game struct */
GameStruct game;
/* This is the volatile game structure (memoizing to reduce op/s) */
vGameStruct vgame;

/* This is the game configuration */
AppConfig_t gameConfig = {
	CONFIG_VERSION,
	DEFAULT_APPCONFIG
};

/*
 * Get the date in the game
 * XXX: deal with phenomenally big dates
 */
char *
GetDate(char *temp)
{
	char year[10];
	const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

	temp[0] = months[(game.TimeElapsed % 12) * 3];
	temp[1] = months[(game.TimeElapsed % 12) * 3 + 1];
	temp[2] = months[(game.TimeElapsed % 12) * 3 + 2];
	temp[3] = ' ';

	sprintf(year, "%ld", (long)(game.TimeElapsed / 12) + 2000);
	temp[4] = year[0];
	temp[5] = year[1];
	temp[6] = year[2];
	temp[7] = year[3];
	temp[8] = (char)0;

	return ((char *)temp);
}

/*
 * find a key in an array.
 * Array is an array of possibly structs with key as the first element
 */
void *
getIndexOf(char *ary, Int16 addit, Int16 key)
{
	while (*(Int16 *)ary) {
		if (key == *(Int16 *)ary)
			return (ary);
		ary += addit;
	}
	return (0);
}

/*
 * Get the disaster level setting of the game
 */
UInt8
GetDisasterLevel(void)
{
	return (game.diff_disaster & 0xF);
}

/*
 * Set The disaster Level of the game
 */
void
SetDisasterLevel(UInt8 value)
{
	game.diff_disaster &= 0xf0;
	game.diff_disaster |= ((value & 0x0f) << 4);
}

/*
 * Get the difficulty level of the game
 */
UInt8
GetDifficultyLevel(void)
{
	return ((game.diff_disaster >> 4) & 0xF);
}

/*
 * Set the difficulty level of the game
 */
void
SetDifficultyLevel(UInt8 value)
{
	game.diff_disaster &= 0xf;
	game.diff_disaster |= ((value & 0x0f) << 4);
}
