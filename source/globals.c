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
#include <mem_compat.h>
#include <ui.h>

/*! \brief this is the central game struct */
GameStruct game;
/*! \brief  This is the volatile game structure (memoizing to reduce op/s) */
vGameStruct vgame;

/*! \brief the world pointer */
void *worldPtr;

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
	char month[10];

	sprintf(temp, "%s %ld", getMonthString(game.TimeElapsed % 12, month, 9),
	    (long)(game.TimeElapsed / 12) + 2000);
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

/*!
 * \brief make the world of a certain size
 * 
 * This, naturally erases the current world map.
 * 
 * \param size the new size of the world (x*y)
 * \return 0 if it all went pear shaped.
 */
Int16
ResizeWorld(UInt32 size)
{
	
	LockWorld();
	worldPtr = gRealloc(worldPtr, size);

	if (worldPtr == NULL) {
		UIDisplayError(enOutOfMemory);
		WriteLog("realloc failed - resizeworld\n");
		return (0);
	}
	gMemSet(worldPtr, size, 0);
	UnlockWorld();

	return (1);
}

/*!
 * \brief initialize the world variables
 * \return 1 if the allocation succeeded, 0 otherwise
 */
Int16
InitWorld(void)
{
	return (ResizeWorld(10));
}

/*!
 * \brief get the item on the surface of the world
 * \param pos the position in the world map to obtain
 * \return the item at that position.
 */
UInt8
GetWorld(UInt32 pos)
{
	UInt16 val;
	
	if (pos > MapMul())
		return (0);
		
	val = ((UInt16 *)worldPtr)[pos];
	/* mask against the inverse of my own flags */
	return (val & FLAGS_ANDMASK);
}

/*!
 * \brief set the object at the position to the value in question
 * \param pos the position of the item
 * \param value the value of the item
 */
void
SetWorld(UInt32 pos, UInt8 value)
{
	UInt16 *ptr;
	
	if (pos > MapMul())
		return;

	ptr = (UInt16 *)worldPtr + pos;
	/* Clear the world value */
	*ptr &= WORLD_ANDMASK;
	*ptr |= value;
}

/*!
 * \brief get the flag corresponding to the game location
 * \param pos the position of the location
 * \return the value at that position
 */
UInt8
GetWorldFlags(UInt32 pos)
{
	UInt16 val;
	
	if (pos > MapMul())
		return (0);
	val = ((UInt16 *)worldPtr)[pos];
	/* Use WORLD_ANDMASK for getting clean value */
	return ((val & WORLD_ANDMASK) >> FLAGS_SHIFTVALUE);
}

/*!
 * \brief set the flag corresponding to the game location
 * \param pos the position of the location
 * \param value the value at that position
 */
void
SetWorldFlags(UInt32 pos, UInt8 value)
{
	UInt16 *ptr;
	
	if (pos > MapMul())
		return;
	ptr = ((UInt16 *)worldPtr) + pos;
	*ptr &= FLAGS_ANDMASK;
	*ptr |= (value << FLAGS_SHIFTVALUE);
}

/*!
 * \brief and the value with the current location of the map world flags
 * \param pos the position on the map
 * \param value the value to and the current value with.
 */
void
AndWorldFlags(UInt32 pos, UInt8 value)
{
	UInt16 *ptr;
	
	if (pos > MapMul())
		return;
	ptr = ((UInt16 *)worldPtr) + pos;
	
	*ptr &= (WORLD_ANDMASK & (value << FLAGS_SHIFTVALUE)) | FLAGS_ANDMASK;
}

/*!
 * \brief or the value with the current location of the map world flags
 * \param pos the position on the map
 * \param value the value to or the current value with.
 */
void
OrWorldFlags(UInt32 pos, UInt8 value)
{
	UInt16 *ptr;
	
	if (pos > MapMul())
		return;
	
	ptr = ((UInt16 *)worldPtr) + pos;
	/* use WORLD_ANDMASK as it is the inversion of FLAGS_ANDMASK */
	*ptr |= (WORLD_ANDMASK & (value << FLAGS_SHIFTVALUE));
}

/*!
 * \brief free all the game related structures
 */
void
PurgeWorld()
{
	LockWorld();
	if (worldPtr != NULL) gFree(worldPtr);
}
