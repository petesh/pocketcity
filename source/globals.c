/*! \file
 * \brief convenience routines
 *
 * This contains routines for working on the difficulty, disaster level
 * of the simulation, as well as displaying the date.
 */
#include <zakdef.h>
#if defined(PALM)
#include <StringMgr.h>
#include <unix_stdio.h>
#else
#include <stdio.h>
#endif

/*! \brief this is the central game struct */
GameStruct game;
/*! \brief  This is the volatile game structure (memoizing to reduce op/s) */
vGameStruct vgame;

/*! \brief This is the game configuration */
AppConfig_t gameConfig = {
	CONFIG_VERSION,
	DEFAULT_APPCONFIG
};

/*!
 * \brief Get the date in the game
 * \todo deal with phenomenally big dates
 * \param temp buffer to put date string into
 * \return the temp buffer (silli!)
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

/*!
 * \brief find a key in an array.
 *
 * Array is an array of possibly structs with key as the first element
 * \param ary the arry to look into
 * \param addit amount ot add to get to next element in array
 * \param key key to check against
 * \return array item, or NULL
 *
 */
void *
getIndexOf(char *ary, Int16 addit, Int16 key)
{
	while (*(Int16 *)ary) {
		if (key == *(Int16 *)ary)
			return (ary);
		ary += addit;
	}
	return (NULL);
}

/*!
 * \brief Get the disaster level setting of the game
 * \return disaster level
 */
UInt8
GetDisasterLevel(void)
{
	return (game.diff_disaster & 0xF);
}

/*!
 * \brief Set The disaster Level of the game
 * \param value the new value of disaster level
 */
void
SetDisasterLevel(UInt8 value)
{
	game.diff_disaster &= 0xf0;
	game.diff_disaster |= (value & 0x0f);
}

/*!
 * \brief Get the difficulty level of the game
 * \return the difficulty level
 */
UInt8
GetDifficultyLevel(void)
{
	return ((game.diff_disaster >> 4) & 0x0f);
}

/*!
 * \brief Set the difficulty level of the game
 * \param value the new difficulty level
 */
void
SetDifficultyLevel(UInt8 value)
{
	game.diff_disaster &= 0x0f;
	game.diff_disaster |= ((value & 0x0f) << 4);
}
