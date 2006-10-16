
/*!
 * \file
 * \brief draw the initial map details
 */

#include <config.h>

#include <simcity.h>

#include <zakdef.h>
#include <globals.h>
#include <locking.h>
#include <build.h>
#include <ui.h>
#include <logging.h>
#include <initial_paint.h>

static void CreateForest(UInt32 pos, UInt16 size) BUILD_SECTION;

/*! \brief Create a river on the map */
static void CreateFullRiver(void) BUILD_SECTION;
/*! \brief Create the forests on the map. */
static void CreateForests(void) BUILD_SECTION;

void PaintTheWorld(void)
{
#if defined(LOGGING)
	UInt32 wpos;
	int val = 0;
#endif

	WriteLog("Painting the world\n");
#if defined(LOGGING)
	WriteLog("Validating that the map is clear [%ld]\n", 
	    (long)MapMul());
	zone_lock(lz_world);
	for (wpos = 0; wpos < MapMul(); wpos++) {
		val += getWorld(wpos);
	}
	assert(val == 0);
	zone_unlock(lz_world);
#endif
	CreateFullRiver();
	CreateForests();
}

/*!
 * \todo Make the river more interesting
 */
static void
CreateFullRiver(void)
{
	UInt16 i, j, k, width;
	int axis;
	UInt16 kmax;

	width = (UInt16)(GetRandomNumber(5) + 5);
	axis = (int)GetRandomNumber(1);
	kmax = axis ? getMapWidth() : getMapHeight();

	/* This is the start position of the center of the river */
	j = (UInt16)GetRandomNumber(kmax);

	zone_lock(lz_world);
	zone_lock(lz_flags);

	for (i = 0; i < kmax; i++) {
		for (k = j; k < (width + j); k++) {
			if (k < kmax) {
				if (axis)
					setWorldAndFlag(WORLDPOS(i, k),
					    Z_REALWATER, 0);
				else
					setWorldAndFlag(WORLDPOS(k, i),
					    Z_REALWATER, 0);
			}
		}

		switch (GetRandomNumber(3)) {
		case 0:
			if (width >  5)
				width--;
			break;
		case 1:
			if (width < 15)
				width++;
			break;
		default:
			break;
		}
		switch (GetRandomNumber(4)) {
		case 0:
			if (j > 0)
				j--;
			break;
		case 1:
			if (j < kmax)
				j++;
			break;
		default:
			break;
		}
	}
	zone_unlock(lz_flags);
	zone_unlock(lz_world);
}

/*!
 * Creates some "spraypainted" (someone called them that)
 * forests throughout the `wilderness`
 */
static void
CreateForests(void)
{
	UInt16 i, j, k;
	UInt32 pos;

	j = (UInt16)(GetRandomNumber(6) + 7);
	for (i = 0; i < j; i++) {
		k = (UInt16)(GetRandomNumber(6) + 8);
		pos = (UInt16)GetRandomNumber(MapMul());
		CreateForest(pos, k);
	}
}

/*!
 * \brief create a single forest at the point specified
 * \param pos Position on the map to start from
 * \param size Radius of the forest to paint.
 */
static void
CreateForest(UInt32 pos, UInt16 size)
{
	UInt16 x, y, i, j, s;

	x = (UInt16)(pos % getMapWidth());
	y = (UInt16)(pos / getMapWidth());
	zone_lock(lz_world);
	zone_lock(lz_flags);
	i = x - size > 0 ? x - size : 0;
	j = y - size > 0 ? y - size : 0;

	for (; i <= x + size; i++) {
		for (j = y - size; j <= y + size; j++) {
			if (i >= getMapWidth() ||
			    j >= getMapHeight())
				continue;
			if (getWorld(WORLDPOS(i, j)) != Z_DIRT)
				continue;
			s = ((y > j) ? (y - j) : (j - y)) +
			    ((x > i) ? (x - i) : (i - x));
			if (GetRandomNumber(s) < 2) {
				/*!
				 * \todo count_trees or count_real_trees
				 */
				setWorldAndFlag(WORLDPOS(i, j), Z_REALTREE, 0);
				vgame.BuildCount[bc_count_trees]++;
			}
		}
		j = y - size > 0 ? y - size : 0;
	}
	zone_unlock(lz_flags);
	zone_unlock(lz_world);
}
