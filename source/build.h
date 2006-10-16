/*!
 * \file
 * \brief Interface to the routines for building.
 *
 * This contains the declarations for all the functions for building
 * that are used by other sections of the simulation
 */

#if !defined(_BUILD_H_)
#define	_BUILD_H_

#include <zakdef.h>
#include <sections.h>
#include <compilerpragmas.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief The codes that are mapped for building items.
 *
 * These items are aligned with the keyCodes in palm/appconfig.h
 * as well as the string list StrID_Popups in palm/game.rcp
 * ease the mapping of buttons to items to build.
 */
typedef enum BuildCode {
	Be_Bulldozer = 0,
	Be_Zone_Residential,
	Be_Zone_Commercial,
	Be_Zone_Industrial,
	Be_Road,
	Be_Rail,
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

/*! \brief Cost of building a rail line */
#define	BUILD_COST_RAIL			25
/*! \brief cost of building a rail tunnel */
#define	BUILD_COST_RAILTUNNEL		125

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

/*!
 * \brief Bulldoze a zone
 * \param xpos X position on the map
 * \param ypos Y position on the map
 * \param _unused unused.
 */
EXPORT int
Build_Bulldoze(UInt16 xpos, UInt16 ypos, welem_t _unused)
    BUILD_SECTION;
/*!
 * \brief Build something at the location specified.
 *
 * \param xpos the X position on the map
 * \param ypos the Y position on the map
 */
EXPORT int BuildSomething(UInt16 xpos, UInt16 ypos) BUILD_SECTION;
/*!
 * \brief Attempt to destroy the item at the position in question
 *
 * Performed by the bulldoze and the auto bulldoze.
 * \param xpos the X position on the map
 * \param ypos the Y position on the map
 */
EXPORT void Build_Destroy(UInt16 xpos, UInt16 ypos) BUILD_SECTION;

/*! \brief Remove all the defences on the map */
EXPORT void RemoveAllDefence(void) BUILD_SECTION;

#if defined(__cplusplus)
}
#endif

#endif /* _BUILD_H_ */
