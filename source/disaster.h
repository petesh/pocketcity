#if !defined(_DISASTER_H_)
#define _DISASTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <appconfig.h>

#ifndef OTHER_SECTION
#define OTHER_SECTION
#endif

void DoNastyStuffTo(Int16 type, UInt16 probability) OTHER_SECTION;
void DoRandomDisaster() OTHER_SECTION;
Int16 UpdateDisasters() OTHER_SECTION;
Int16 BurnField(Int16 x, Int16 y, Int16 forceit) OTHER_SECTION;
Int16 CreateMonster(Int16 x, Int16 y) OTHER_SECTION;
Int16 CreateDragon(Int16 x, Int16 y) OTHER_SECTION;
void MoveAllObjects(void) OTHER_SECTION;
Int16 MeteorDisaster(Int16 x, Int16 y) OTHER_SECTION;
void DoSpecificDisaster(erdiType disaster) OTHER_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* _DISASTER_H_ */
