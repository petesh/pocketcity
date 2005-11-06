/*!
 * \file
 * \brief convenience routines
 *
 * This contains routines for working on the difficulty, disaster level
 * of the simulation, as well as displaying the date.
 */

#include <globals.h>
#include <config.h>
#include <mem_compat.h>
#include <logging.h>
#include <locking.h>
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

vGameVisuals visuals;

char *worldPtr;
char *flagPtr;
char *powerMap;
char *waterMap;
char *pollutionMap;
char *crimeMap;

/*! \brief This is the game configuration */
AppConfig_t gameConfig = {
	CONFIG_VERSION,
	DEFAULT_APPCONFIG
};

/*! \brief the varaible used to record what elements of the world need change */
static UInt16 needchange;

/*!
 * \brief add an item that needs updating in the display
 * \param entity the entity to be updated.
 */
void
addGraphicUpdate(graphicupdate_t entity)
{
	if (entity != gu_all)
		needchange |= (UInt16)entity;
	else
		needchange = ~(UInt16)0;
}

/*!
 * \brief remove and item that needs updating in the display
 * \param entity the entity that needs removal
 */
void
removeGraphicUpdate(graphicupdate_t entity)
{
	needchange &= ~((UInt16)entity);
}

/*!
 * \brief check that a graphic entity needs changing
 * \param entity the entity to check
 * \return whether the entity needs changing
 */
UInt8
checkGraphicUpdate(graphicupdate_t entity)
{
	return ((UInt8)(needchange & entity ? 1 : 0));
}

/*!
 * \brief check that any graphic item needs repainting
 * \return true if any of the entities need repainting
 */
UInt8
checkAnyGraphicUpdate(void)
{
	return ((UInt8)(needchange ? 1 : 0));
}

/*!
 * \brief clear the need update fields in their entirety
 */
void
clearGraphicUpdate(void)
{
	needchange = 0;
}

/*!
 * \brief Get the date in the game
 * \todo deal with phenomenally big dates
 * \param temp buffer to put date string into
 * \return the temp buffer (silli!)
 */
char *
getDate(char *temp)
{
	Char month[10];

	sprintf(temp, "%s %ld", getMonthString(
	    (UInt16)(getMonthsElapsed() % 12),
	    month, (UInt16)9),
	    (long)((getMonthsElapsed() / 12) + 2000));
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
	return ((UInt8)(GG.diff_disaster & 0xF));
}

/*!
 * \brief Set The disaster Level of the game
 * \param value the new value of disaster level
 */
void
setDisasterLevel(UInt8 value)
{
	GG.diff_disaster &= 0xf0;
	GG.diff_disaster |= (UInt8)(value & 0x0f);
}

/*!
 * \brief Get the difficulty level of the game
 * \return the difficulty level
 */
UInt8
getDifficultyLevel(void)
{
	return ((UInt8)((GG.diff_disaster >> 4) & 0x0f));
}

/*!
 * \brief Set the difficulty level of the game
 * \param value the new difficulty level
 */
void
setDifficultyLevel(UInt8 value)
{
	GG.diff_disaster &= (UInt8)0x0f;
	GG.diff_disaster |= (UInt8)((value & 0x0f) << 4);
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
	LockZone(lz_flags);
	worldPtr = gRealloc(worldPtr, size);
	flagPtr = gRealloc(flagPtr, size);

	if (worldPtr == NULL) {
		UISystemErrorNotify(seOutOfMemory);
		WriteLog("realloc failed - resizeworld\n");
		return (0);
	}
	if (flagPtr == NULL) {
		UISystemErrorNotify(seOutOfMemory);
		WriteLog("realloc failed - resizeworldflags\n");
		return (0);
	}

	gMemSet(worldPtr, (Int32)size, 0);
	gMemSet(flagPtr, (Int32)size, 0);
	UnlockZone(lz_world);
	UnlockZone(lz_flags);
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
	if (pos > MapMul())
		return (0);

	return (((welem_t *)worldPtr)[pos]);
}

/*!
 * \brief set the object at the position to the value in question
 * \param pos the position of the item
 * \param value the value of the item
 */
void
setWorld(UInt32 pos, welem_t value)
{
	if (pos > MapMul())
		return;

	((welem_t *)worldPtr)[pos] = value;
}

/*!
 * \brief set the world field and field flag
 * \param pos the position
 * \param value the value
 * \param status the status of this position
 */
void
setWorldAndFlag(UInt32 pos, welem_t value, selem_t status)
{
	if (pos > MapMul())
		return;

	((welem_t *)worldPtr)[pos] = value;
	((selem_t *)flagPtr)[pos] = status;
}

/*!
 * \brief get the flag corresponding to the game location
 * \param pos the position of the location
 * \return the value at that position
 */
selem_t
getWorldFlags(UInt32 pos)
{
	if (pos > MapMul())
		return (0);
	return (((selem_t *)flagPtr)[pos]);
}

/*!
 * \brief set the flag corresponding to the game location
 * \param pos the position of the location
 * \param value the value at that position
 */
void
setWorldFlags(UInt32 pos, selem_t value)
{
	if (pos > MapMul())
		return;
	((selem_t *)flagPtr)[pos] = value;
}

/*!
 * \brief and the value with the current location of the map world flags
 * \param pos the position on the map
 * \param value the value to and the current value with.
 */
void
andWorldFlags(UInt32 pos, selem_t value)
{
	if (pos > MapMul())
		return;
	((selem_t *)flagPtr)[pos] &= value;
}

/*!
 * \brief clear the bits chosen for the world flag
 * \param pos the position on the map
 * \param value the value to nand
 */
void
clearWorldFlags(UInt32 pos, selem_t value)
{
	andWorldFlags(pos, ~value);
}

/*!
 * \brief or the value with the current location of the map world flags
 * \param pos the position on the map
 * \param value the value to or the current value with.
 */
void
orWorldFlags(UInt32 pos, selem_t value)
{
	if (pos > MapMul())
		return;
	((selem_t *)flagPtr)[pos] |= value;
}

/*!
 * \brief get the values of the field and it's flag
 * \param pos the location on the map to extract the value of
 * \param world a pointer to fill with the value of the world field
 * \param flag a pointer to fill with the value of the flag at that location
 */
void
getWorldAndFlag(UInt32 pos, welem_t *world, selem_t *flag)
{
	if (pos > MapMul())
		return;
	*world = ((welem_t *)worldPtr)[pos];
	*flag = ((selem_t *)flagPtr)[pos];
}

/*!
 * \brief free all the game related structures
 */
void
PurgeWorld(void)
{
	ReleaseZone(lz_world);
	ReleaseZone(lz_flags);
}

/*!
 * \brief scale a signed 32 bit number down using the K,M,B scale
 *
 * Don't use the value returned for any math! it's for display only.
 * \param old_value the value that needs to be scaled
 * \param scale (out) the scale i.e. none, K, M, B ...
 * \return the scaled value (could be the original value)
 */
Int32
scaleNumber32(Int32 old_value, Char *scale)
{
	const char si_scale[] = " KMBTQ";
	const char *at_scale = si_scale;
	int negative = 0;
	if (old_value < 0) {
		negative = 1;
		old_value = -old_value;
	}
	while (old_value > MILLION) {
		at_scale++;
		old_value /= 1000;
	}
	*scale = *at_scale;
	if (negative)
		old_value = -old_value;
	return (old_value);
}

/*!
 * \brief scale a signed 32 bit number down using the K,M,B scale
 *
 * Don't use the value returned for any math! it's for display only.
 * \param old_value the value that needs to be scaled
 * \param scale (out) the scale i.e. none, K, M, B ...
 * \return the scaled value (could be the original value)
 */
Int16
scaleNumber16(Int16 old_value, Char *scale)
{
	return ((Int16)scaleNumber32((Int32)old_value, scale));
}

/*!
 * \brief set one of the savegame status bits
 * \param bit the bit to set
 * \param value the value of the new status bit
 *
 * It sets the bit to 1 if value is non zero.
 */
void
setGameBit(gamestatusbit_t bit, UInt8 value)
{
	GG.gas_bits &= (~bit & 0xff);
	if (value) GG.gas_bits |= bit;
}

/*!
 * \brief get one of the savegame status bits
 * \param bit the field to get the value of
 * \returns non-zero if the bit is set, zero otherwise
 */
UInt8
getGameBit(gamestatusbit_t bit)
{
	return (GG.gas_bits & bit);
}
