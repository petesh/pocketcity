/*! \file
 * \brief interface routines for disasters
 */
#if !defined(_DISASTER_H_)
#define	_DISASTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <appconfig.h>
#include <sections.h>
#include <compilerpragmas.h>

EXPORT void DoNastyStuffTo(welem_t type, UInt16 probability) DISASTER_SECTION;
EXPORT void DoRandomDisaster() DISASTER_SECTION;
EXPORT Int16 UpdateDisasters() DISASTER_SECTION;
EXPORT Int16 BurnField(Int16 x, Int16 y, Int16 forceit) DISASTER_SECTION;
EXPORT Int16 CreateMonster(Int16 x, Int16 y) DISASTER_SECTION;
EXPORT Int16 CreateDragon(Int16 x, Int16 y) DISASTER_SECTION;
EXPORT void MoveAllObjects(void) DISASTER_SECTION;
EXPORT Int16 MeteorDisaster(Int16 x, Int16 y) DISASTER_SECTION;
EXPORT void DoSpecificDisaster(erdiType disaster) DISASTER_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* _DISASTER_H_ */
