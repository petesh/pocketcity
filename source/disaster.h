/*!
 * \file
 * \brief interface for routines for disasters
 */
#if !defined(_DISASTER_H_)
#define	_DISASTER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <appconfig.h>
#include <sections.h>
#include <compilerpragmas.h>

/*!
 * \brief Do nasty things to a location.
 *
 * turns a zone into wasteland based on the normalized probability.
 * \param type of zone that can be affected
 * \param probability the normalized probability of something bad happens
 * \param purge remove a tile if it's found
 */
EXPORT void
DoNastyStuffTo(welem_t type, UInt16 probability, UInt8 purge) DISASTER_SECTION;

/*! \brief perform nasties to zones if they are not being completely funded */
EXPORT void DoCommitmentNasties(void) DISASTER_SECTION;

/*! \brief Do a random disaster. */
EXPORT void DoRandomDisaster(void) DISASTER_SECTION;

/*!
 * \brief Deliberately cause a disaster.
 * \param disaster the type of the disaster to cause.
 */
EXPORT void DoSpecificDisaster(disaster_t disaster) DISASTER_SECTION;

/*!
 * \brief Make sure the disasters are still happening.
 *
 * Causes all disasters to go to their next stage.
 * \return true if zone was affected.
 */
EXPORT Int16 UpdateDisasters(void) DISASTER_SECTION;

/*!
 * \brief burn the field specified.
 *
 * Can be forced to burn.
 * \param x position on horizontal of map to spread fire
 * \param y position on vertical of map to spread fire
 * \param forceit force the field to burn
 * \return true if field was burned.
 */
EXPORT Int16 BurnField(UInt16 x, UInt16 y, Int16 forceit) DISASTER_SECTION;

/*!
 * \brief Create a 'zilla at the location specified.
 * \param x position on horizontal of map to spread fire
 * \param y position on vertical of map to spread fire
 * \return true if monster was created
 */
EXPORT Int16 CreateMonster(UInt16 x, UInt16 y) DISASTER_SECTION;

/*!
 * \brief Create a dragon at the location.
 * \param x position on horizontal of map to spread fire
 * \param y position on vertical of map to spread fire
 * \return true if dragon was created
 */
EXPORT Int16 CreateDragon(UInt16 x, UInt16 y) DISASTER_SECTION;

/*! \brief Move all the moveable elements around the screen.  */
EXPORT void MoveAllObjects(void) DISASTER_SECTION;

/*!
 * \brief We've had a meteor strike on the map at that location.
 * \param x horizontal position
 * \param y vertical position
 * \return always happens (true)
 */
EXPORT Int16 MeteorDisaster(UInt16 x, UInt16 y) DISASTER_SECTION;

#if defined(__cplusplus)
}
#endif

#endif /* _DISASTER_H_ */
