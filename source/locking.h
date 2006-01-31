/*!
 * \file
 * \brief Interface to locking data structures.
 *
 * These are structures that can only be accessed by locking the memory
 * before accessing it. The reason is primarly due to the palm platform
 * where memory is in unlocked 'handles' that allows it to be moved around
 * in real memory until it is needed; whereupon it is locked - resulting in
 * a stock standard pointer.
 *
 * The meat of the implementation is up to the platform in question. The
 * barest minimum an implementation can do is to simply put the memory in the
 * lockzone's defined destination when allocated and zero it when freed.
 *
 * Other things that can be done are only initializing the data element when
 * the zone is locked (better security).
 * 
 */
#if !defined(_LOCKING_H)
#define	_LOCKING_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <config.h>

/*!
 * \brief The core locking structure
 *
 * This structure contains the various elements that correspond to a lockable
 * data structure.
 */
typedef struct _locking {
	MemHandle	handle; /*!< The movable handle of memory */
	MemPtr		ptr;	/*!< The locked handle of memory */
	Char		**pptr;	/*!< A variable to be set on locking */
	int		lock_count; /*!< The count of locks to memory */
	UInt32		size; /*!< memory size */
} lockmem_t;

/*!
 * Operations that we perform on the data structures
 */

/*!
 * \brief Allocate memory to the structure
 * This function produces an exception if the structure has already been
 * allocated.
 * \param str the structure to lock
 * \param size the size of the structure to allocate
 * \return true if the memory has been allocated
 *
 * An allocation of 0 has platform dependent results.
 */
int mem_alloc(lockmem_t *str, UInt32 size);

/*! 
 * \brief Lock the structure
 * \param str the structure to lock
 */
void mem_lock(lockmem_t *str);

/*!
 * \brief unlock the structure
 * \param str the structure to unlock
 * decrements the lock_count. When the lock_count reaches 0 it is completely
 * unlocked.
 */
void mem_unlock(lockmem_t *str);

/*!
 * \brief resize the memory - may not already be allocated memory
 * \param str the structure to resize
 * \param size the new size of the data
 * \return true if the memory has been resized
 * resizing to 0 has the same effect as calling mem_release
 */
int mem_resize(lockmem_t *str, UInt32 size);

/*!
 * \brief get the size of the memory structure
 *
 * resizing to 0 has the same effect as calling mem_release
 * \param str the structure to resize
 * \return true if the memory has been resized
 */
UInt32 mem_size(lockmem_t *str);

/*!
 * \brief release the memory
 * \param str the structure to release
 */
void mem_release(lockmem_t *str);


/*! \brief the zones to lock/unlock */
typedef enum {
	lz_world = 0, /*!< lock the world zone */
	lz_flags, /*!< lock the world flags */
	lz_pollution, /*!< lock the pollution memory zone */
	lz_crime, /*!< lock the crime memory zone */
	lz_transport, /*!< lock the transport memory zone */
	lz_end /*!< the end/guard entry for the memory zones */
} lockZone;

/*!
 * \brief lock the zone specified for writing
 * \param zone the zone to lock
 */
void zone_lock(lockZone zone);

/*!
 * \brief unlock the zone specified. don't write to it anymore
 * \param zone the zone to unlock
 */
void zone_unlock(lockZone zone);

/*!
 * \brief release the memory allocated to this zone
 * \param zone the zone to release
 * useing lz_end releases all zones (probably wanted)
 */
void zone_release(lockZone zone);

/*!
 * \brief resize the zone in question to the appropriate size
 * \param zone the zone to resize
 * \param size the new size of the zone
 */
int zone_resize(lockZone zone, UInt32 size);

/*!
 * \brief allocate memory to a zone
 * \param zone the zone to lock
 * \param size the size to set the zone to (bytes)
 */
int zone_alloc(lockZone zone, UInt32 size);

/*!
 * \brief get the memory allocated to the zone
 * \param zone the zone that we're interested in
 * \return the size of the zone
 */
UInt32 zone_size(lockZone zone);

#if defined(__cplusplus)
}
#endif

#endif /* _LOCKING_H */
