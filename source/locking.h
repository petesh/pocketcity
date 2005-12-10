/*!
 * \file
 * \brief the user interface routines that need defining in any implementation
 *
 * These are all the functions that need implementing if you want to
 * make the game work.
 */
#if !defined(_LOCKING_H)
#define	_LOCKING_H

#if defined(__cplusplus)
extern "C" {
#endif

/*! \brief the zones to lock/unlock */
typedef enum {
	lz_world = 0, /*!< lock the world zone */
	lz_flags, /*!< lock the world flags */
	lz_pollution, /*!< lock the pollution memory zone */
	lz_crime, /*!< lock the crime memory zone */
	lz_end /*!< the end/guard entry for the memory zones */
} lockZone;

/*!
 * \brief lock the zone specified for writing
 * \param zone the zone to lock
 */
void LockZone(lockZone zone);

/*!
 * \brief unlock the zone specified. don't write to it anymore
 * \param zone the zone to unlock
 */
void UnlockZone(lockZone zone);

/*!
 * \brief release the memory allocated to this zone
 * \param zone the zone to release
 */
void ReleaseZone(lockZone zone);

#if defined(__cplusplus)
}
#endif

#endif /* _LOCKING_H */
