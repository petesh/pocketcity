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
#include <stack.h>

#define	MILLION 1000000

/*!
 * \brief contains the mapping of a field in the statistics to a counter code
 * 
 * this is used to produce the graphs.
 * As all the values in the BuildCount array are 32bit values and all the
 * statistic items are 16 bit values an amount of shifting is done to the
 * build count value once it exceeds the maximum 16bit value
 */
stat_to_value statvalues[] = {
	{ st_cashflow, bc_cashflow },
	{ st_pollution, bc_pollution },
	{ st_crime, bc_crime },
	{ st_residential, bc_value_residential },
	{ st_commercial, bc_value_commercial }, 
	{ st_industrial, bc_value_industrial },
	{ st_tail, 0 }
};

/*! \brief this is the central game struct */
GameStruct game;
/*! \brief  This is the volatile game structure (memoizing to reduce op/s) */
vGameStruct vgame;

/*! \brief the game 'visuals' - tilesize and the likes */
vGameVisuals visuals;

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
getDate(char *temp)
{
	char month[10];

	sprintf(temp, "%s %ld", getMonthString(getMonthsElapsed() % 12,
	    month, 9), (long)(getMonthsElapsed() / 12) + 2000);
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
getDisasterLevel(void)
{
	return (GG.diff_disaster & 0xF);
}

/*!
 * \brief Set The disaster Level of the game
 * \param value the new value of disaster level
 */
void
setDisasterLevel(UInt8 value)
{
	GG.diff_disaster &= 0xf0;
	GG.diff_disaster |= (value & 0x0f);
}

/*!
 * \brief Get the difficulty level of the game
 * \return the difficulty level
 */
UInt8
getDifficultyLevel(void)
{
	return ((GG.diff_disaster >> 4) & 0x0f);
}

/*!
 * \brief Set the difficulty level of the game
 * \param value the new difficulty level
 */
void
setDifficultyLevel(UInt8 value)
{
	GG.diff_disaster &= 0x0f;
	GG.diff_disaster |= ((value & 0x0f) << 4);
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
	WriteLog("Resize World = %ld\n", (long)size);
	LockZone(lz_world);
	worldPtr = gRealloc(worldPtr, size);

	if (worldPtr == NULL) {
		UIDisplayError(enOutOfMemory);
		WriteLog("realloc failed - resizeworld\n");
		return (0);
	}

	gMemSet(worldPtr, size, 0);
	UnlockZone(lz_world);
	return (1);
}

/*!
 * \brief initialize the world variables
 * \return 1 if the allocation succeeded, 0 otherwise
 */
Int16
InitWorld(void)
{
	return (1);
}

/*!
 * \brief get the item on the surface of the world
 * \param pos the position in the world map to obtain
 * \return the item at that position.
 */
welem_t
getWorld(UInt32 pos)
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
setWorld(UInt32 pos, welem_t value)
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
selem_t
getWorldFlags(UInt32 pos)
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
setWorldFlags(UInt32 pos, selem_t value)
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
andWorldFlags(UInt32 pos, selem_t value)
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
orWorldFlags(UInt32 pos, selem_t value)
{
	UInt16 *ptr;
	
	if (pos > MapMul())
		return;
	
	ptr = ((UInt16 *)worldPtr) + pos;
	/* use WORLD_ANDMASK as it is the inversion of FLAGS_ANDMASK */
	*ptr |= (WORLD_ANDMASK & (value << FLAGS_SHIFTVALUE));
}

void
getWorldAndFlag(UInt32 pos, welem_t *world, selem_t *flag)
{
	UInt16 val;

	if (pos > MapMul())
		return;

	val = ((UInt16 *)worldPtr)[pos];
	*world = val & FLAGS_ANDMASK;
	*flag = ((val & WORLD_ANDMASK) >> FLAGS_SHIFTVALUE);
}

/*!
 * \brief free all the game related structures
 */
void
PurgeWorld()
{
	ReleaseZone(lz_world);
}

/*!
 * \brief scale a number down using the K,M,B scale
 *
 * Don't use the value returned for any math! it's for display only.
 * \param old_value the value that needs to be scaled
 * \param scale (out) the scale i.e. none, K, M, B ...
 * \return the scaled value (could be the original value)
 */
Int32
scaleNumber(UInt32 old_value, Char *scale)
{
	const char si_scale[] = " KMBTQ";
	const char *at_scale = si_scale;
	while (old_value > MILLION) {
		at_scale++;
		old_value /= 1000;
	}
	*scale = *at_scale;
	return (old_value);
}

