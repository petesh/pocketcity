/*!
 * \file
 * \brief Platform independent pieces of the locking code
 * This covers the LockZone code and how it interacts with the rest of the
 * world.
 */

#include <locking.h>
#include <globals.h>

/*!
 * These are the lockzones corresponding to map related items
 */
static lockmem_t lockzones[lz_end] = {
	{ NULL, NULL, &worldPtr, 0, 0 },
	{ NULL, NULL, &flagPtr, 0, 0 },
	{ NULL, NULL, &pollutionPtr, 0, 0 },
	{ NULL, NULL, &crimePtr, 0, 0 },
	{ NULL, NULL, &transportPtr, 0, 0 }
};

/*!
 * These represent the sizes of the various elements in lockzones. These
 * are needed to ensure that the correct memory is allocated to the
 * structures when they are resized.
 */
static Int8 zonesize[lz_end] = {
	sizeof (welem_t), sizeof (selem_t), sizeof (Int8),
	sizeof (Int8), sizeof (Int8)
};

void
zone_lock(lockZone zone)
{
	if (zone == lz_end) {
		zone = lz_world;
		while (zone < lz_end)
			mem_lock(lockzones + zone++);
	} else {
		mem_lock(lockzones + zone);
	}
}

void
zone_unlock(lockZone zone)
{
	if (zone == lz_end) {
		zone = lz_world;
		while (zone < lz_end)
			mem_lock(lockzones + zone++);
	} else {
		mem_unlock(lockzones + zone);
	}
}

int
zone_resize(lockZone zone, UInt32 size)
{
	lockmem_t *mem = lockzones + zone;
	int rv = 0;
	
	if (zone == lz_end) {
		zone = lz_world;
		while (zone < lz_end) {
			mem = lockzones + zone;
			rv += mem_resize(mem, size * zonesize[zone]);
			if (rv == 0)
				return (0);
			zone++;
		}
	} else {
		rv = mem_resize(mem, size);
	}
	return (rv);
}

int
zone_alloc(lockZone zone, UInt32 size)
{
	lockmem_t *mem = lockzones + zone;
	int rv = 0;

	if (zone == lz_end) {
		zone = lz_world;
		while (zone < lz_end) {
			mem = lockzones + zone;
			rv += mem_alloc(mem, size * zonesize[zone]);
			if (rv == 0)
				return (0);
			zone++;
		}
	} else {
		rv = mem_alloc(mem, size);
	}
	return (rv);
}

void
zone_release(lockZone zone)
{
	lockmem_t *mem;
	if (zone == lz_end) {
		zone = lz_world;
		while (zone < lz_end) {
			mem_release(lockzones + zone);
			zone++;
		}
	} else {
		mem = lockzones + zone;
		mem_release(mem);
	}
}

UInt32
zone_size(lockZone zone)
{
	lockmem_t *mem = lockzones + zone;
	return (mem_size(mem));
}

UInt32
mem_size(lockmem_t *mem)
{
	return (mem->size);
}
