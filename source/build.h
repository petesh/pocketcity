/*! \file
 * \brief The routines for building.
 *
 * This contains the declarations for all the functions for building
 * that are used by other sections of the simulation
 */

#if !defined(_BUILD_H_)
#define	_BUILD_H_

#include <zakdef.h>
#include <appconfig.h>

/*!
 * \brief The codes that are mapped for building items.
 *
 * These items are aligned with the BUILD_* items in simcity.h to
 * ease the mapping of buttons to items to build.
 */
typedef enum {
	Be_Bulldozer = 0,
	Be_Zone_Residential,
	Be_Zone_Commercial,
	Be_Zone_Industrial,
	Be_Road,
	Be_Power_Plant,
	Be_Nuclear_Plant,
	Be_Power_Line,
	Be_Water_Pump,
	Be_Water_Pipe,
	Be_Tree,
	Be_Water,
	Be_Fire_Station,
	Be_Police_Station,
	Be_Military_Base,
	Be_Query,
	/* Here begins OFFSET_EXTRA */
	/* Defense items are bigger. */
	Be_Defence_Fire,
	Be_Defence_Police,
	Be_Defence_Military,
	Be_Extra,
	Be_OOB
} BuildCode;

/*! \brief The Cost of building a bulldozer */
#define	BUILD_COST_BULLDOZER		5
/*! \brief The cost of building one of the standard zones */
#define	BUILD_COST_ZONE			 50
/*! \brief The cost of building a road */
#define	BUILD_COST_ROAD			 20
/*! \brief The cost of building a coal power plant */
#define	BUILD_COST_POWER_PLANT	  3000
/*! \brief The cost of building a nuclear power plant */
#define	BUILD_COST_NUCLEAR_PLANT	10000
/*! \brief The cost of building a power line */
#define	BUILD_COST_POWER_LINE	   5
/*! \brief The cost of building a tree/forest */
#define	BUILD_COST_TREE			 10
/*! \brief The cost of building a lake/water */
#define	BUILD_COST_WATER			200
/*! \brief The cost of building a bridge (over water) */
#define	BUILD_COST_BRIDGE		   100
/*! \brief The cost of building a fire station */
#define	BUILD_COST_FIRE_STATION	 700
/*! \brief The cost of building a police station */
#define	BUILD_COST_POLICE_STATION   500
/*! \brief The cost of building a military base */
#define	BUILD_COST_MILITARY_BASE	10000
/*! \brief The cost of building a water pipe */
#define	BUILD_COST_WATER_PIPE	  20
/*! \brief The cost of building a water pump */
#define	BUILD_COST_WATER_PUMP	   3000

#ifdef __cplusplus
extern "C" {
#endif

void Build_Bulldoze(Int16 xpos, Int16 ypos, welem_t _unused) BUILD_SECTION;
void BuildSomething(Int16 xpos, Int16 ypos) BUILD_SECTION;
void Build_Destroy(Int16 xpos, Int16 ypos) BUILD_SECTION;
void CreateFullRiver(void) BUILD_SECTION;
void CreateForests(void) BUILD_SECTION;
void RemoveAllDefence(void) BUILD_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* _BUILD_H_ */
