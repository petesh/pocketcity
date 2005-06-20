/*! \file
 * \brief interface routines to the simulation
 */
#if !defined(_SIMULATION_H_)
#define	_SIMULATION_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <zakdef.h>
#include <compilerpragmas.h>

/*! \brief budget numbers that are obtained for reporting */
typedef enum {
	bnResidential = 0,
	bnCommercial,
	bnIndustrial,
	bnIncome,
	bnTraffic,
	bnPower,
	bnDefence,
	bnCurrentBalance,
	bnChange,
	bnNextMonth
} BudgetNumber;

/*! \brief the bit associated with power in the worldflags */
#define	POWEREDBIT	((unsigned char)0x01)
/*! \brief the bit associated with water in the worldflags */
#define	WATEREDBIT	((unsigned char)0x02)
/*! \brief the number of fits saved into the savegame */
#define SAVEDBITS	(2)

/*! \brief the bit associated with scratch/unvisited in the worldflags */
#define	SCRATCHBIT	((unsigned char)0x80)
/*! \brief the bit associated with knowing if the field has been painted */
#define PAINTEDBIT	((unsigned char)0x40)

/*!
 * \brief Perform a phase of the simulation
 * \param nPhase the phase number to do
 * \return the next phase to go to
 */
EXPORT Int16 Sim_DoPhase(Int16 nPhase);

/*!
 * \brief Get a number for the budget.
 * \param type the item that we want to extract
 * \return the value of that item from the budget
 */
EXPORT Int32 BudgetGetNumber(BudgetNumber type);

/*!
 * \brief update the vgame entries
 *
 * Updates the BuildCount array after a load game.
 */
EXPORT void UpdateVolatiles(void);

/*!
 * \brief Update the various counter entities for the statistics
 * 
 * This updates the various fields that are not recorded directly in the system
 */
EXPORT void UpdateCounters(void);

/*!
 * \brief record all the statistics for the month.
 */
void RecordStatistics(void);

/*!
 * \brief get the population of the simulation
 * \return the population
 */
EXPORT UInt32 getPopulation(void);

/*!
 * \brief Is this node a road
 * \param x the node entry to query
 * \return true if it's road
 */
EXPORT Int16 IsRoad(welem_t x);

/*!
 * \brief is the node a road overlapping with power
 * \param x the node to query
 * \return true if the node is a water overlapping with power
 */
EXPORT Int16 IsRoadPower(welem_t x);

/*!
 * \brief is the node a road overlapping with power
 * \param x the node to query
 * \return true if the node is a road overlapping power line
 */
EXPORT Int16 IsRoadPipe(welem_t x);

/*!
 * \brief is this node a bridge
 * \param x the node to query
 * \return true if we're a bridge (this is not the same as a road)
 */
EXPORT Int16 IsRoadBridge(welem_t x);

/*!
 * \brief is the zone either a road or bridge
 * \param x the zone to test
 * \return true if the zone is either a bridge or road
 */
EXPORT Int16 IsRoadOrBridge(welem_t x);

/*!
 * \brief is the node Rail
 * \param x the zone to test
 * \return true if the zone is rail
 */
EXPORT Int16 IsRail(welem_t x);

/*!
 * \brief is the node rail overlapping power
 * \param x the zone to test
 * \return true of the zone is a rail overlapping with power
 */
EXPORT Int16 IsRailPower(welem_t x);

/*!
 * \brief is the node rail overlapping with a pipe
 * \param x the node to test
 * \return true if the zone is a rail line overlapping with a pipe
 */
EXPORT Int16 IsRailPipe(welem_t x);

/*!
 * \brief is the node a rail tunnel
 * \param x the node to test
 * \return true if the zone is a rail tunnel
 */
EXPORT Int16 IsRailTunnel(welem_t x);

/*!
 * \brief is the node a rail line or a tunnel
 * \param x the node to test
 * \return true if the node is a rail line or a tunnel
 */
EXPORT Int16 IsRailOrTunnel(welem_t x);

/*!
 * \brief is the node a rail track overlapping with a road
 * \param x the node to test
 * \return true if the case is true
 */
EXPORT Int16 IsRailOvRoad(welem_t x);

/*!
 * \brief is the node a power overlapping with water pipe
 * \param x the node to query
 * \return true if the node is a water pipe overlapping with power
 */
EXPORT Int16 IsPowerWater(welem_t x);

/*!
 * \brief get the value of the zone
 * \param x the node to get the value of
 * \return the value of the node
 */
EXPORT Int16 ZoneValue(welem_t x);

/*!
 * \brief Is this node a power line
 * \param x the node entry to query
 * \return true if it's a power line
 */
EXPORT Int16 IsPowerLine(welem_t x);

/*!
 * \brief is this node a water pipe
 * \param x the node to query
 * \return true if this is a pipe
 */
EXPORT Int16 IsWaterPipe(welem_t x);

/*!
 * \brief Is this node of the zone type passed in.
 * \param x node item to query
 * \param nType type of node.
 * \return zone value, or zero.
 */
EXPORT Int16 IsZone(welem_t x, zoneType nType);

/*!
 * \brief is this zone occupied
 * \param x the zone to test
 */
EXPORT Int16 IsOccupied(welem_t x);

/*!
 * \brief Is this node a transportation node?
 * \param x the noe to check
 * \return if this is the case
 *
 * say if the node is road & rail or any overlapping combination thereof
 */
EXPORT Int16 IsTransport(welem_t x);

/*!
 * \brief is this zone real water
 * \param X the zone to check
 * \return true if it is real water
 */
#define IsRealWater(X)	((X) == Z_REALWATER)

/*!
 * \brief Is the node a pump
 * \param X the node to test
 * \return true if it's a pump
 */
#define IsPump(X) ((X) == Z_PUMP)

/*!
 * \brief is the node a coal power plant
 * \param X the node to test
 * \return true if it is a coal plant
 */
#define IsCoalPlant(X)	(((X) >= Z_COALPLANT_START) && ((X) <= Z_COALPLANT_END))

/*!
 * \brief is the node a nuclear power pland
 * \param X the node to test
 * \return true if it is a nuclear plant
 */
#define IsNukePlant(X)	(((X) >= Z_NUCLEARPLANT_START) && \
    ((X) <= Z_NUCLEARPLANT_END))

/*
 * \brief is the node a real tree
 * \param X the node to test
 * \return true if it is a real tree
 */
#define	IsRealTree(X)	((X) == Z_REALTREE)

/*!
 * \brief is the node a fake tree
 * \param X the node to test
 * \return true if it is a fake tree
 */
#define	IsFakeTree(X)	((X) == Z_FAKETREE)

/*!
 * \brief is the node a tree (fake or real)
 * \param X the node to test
 * \return true if the node's a tree
 */
#define	IsTree(X)	(IsFakeTree(X) || IsRealTree(X))

/*!
 * \brief is the node real water
 * \param X the node to test
 * \return true if the node is real water
 */
#define IsRealWater(X)	((X) == Z_REALWATER)

/*!
 * \brief is the node fake water
 * \param X the node to test
 * \return true if the node is fake water
 */
#define IsFakeWater(X)	((X) == Z_FAKEWATER)

/*!
 * \brief is the node water
 * \param X the node to test
 * \return true if the node is real water
 */
#define IsWater(X)	(IsFakeWater(X) || IsRealWater(X))

/*!
 * \brief is the node a slum
 * \param X the node to test
 * \return true if the node is a slum
 */
#define IsSlum(X) (((X) >= Z_COMMERCIAL_SLUM) && ((X) <= Z_INDUSTRIAL_SLUM))

/*!
 * \brief Is a zone 'growable'.
 * \param X the zone to test
 * \return true if the zone is growable (which implies score)
 */
#define	IsGrowable(X) (IsSlum(X) || \
    (((X) >= Z_COMMERCIAL_MIN) && ((X) <= Z_INDUSTRIAL_MAX)))

/*!
 * \brief is the zone a Commercial one
 * \param X the node to test
 * \return true if it is
 */
#define	IsCommercial(X)	(((X) == Z_COMMERCIAL_SLUM) || \
    (((X) >= Z_COMMERCIAL_MIN) && ((X) <= Z_COMMERCIAL_MAX)))

/*!
 * \brief is the zone a Residential one
 * \param X the node to test
 * \return true if it is
 */
#define	IsResidential(X)	(((X) == Z_RESIDENTIAL_SLUM) || \
    (((X) >= Z_RESIDENTIAL_MIN) && ((X) <= Z_RESIDENTIAL_MAX)))

/*!
 * \brief is the zone an Industrial one
 * \param X the node to test
 * \return true if it is
 */
#define	IsIndustrial(X)	(((X) == Z_INDUSTRIAL_SLUM) || \
    (((X) >= Z_INDUSTRIAL_MIN) && ((X) <= Z_INDUSTRIAL_MAX)))

/*!
 * \brief is the node wasteland
 * \param X the node to test
 * \return true if it is wasteland
 */
#define IsWaste(X)	((X) == Z_WASTE)

/*!
 * \brief is the zone any of the 'fire station' nodes
 * \param X the node to test
 * \return true if it is a fire station
 */
#define IsFireStation(X)	(((X) >= Z_FIRESTATION_START) && \
    ((X) <= Z_FIRESTATION_END))

/*!
 * \brief is the zone any of the 'police department' nodes
 * \param X the node to test
 * \return true if it is a police department
 */
#define IsPoliceDept(X)	(((X) >= Z_POLICEDEPT_START) && \
    ((X) <= Z_POLICEDEPT_END))

/*!
 * \brief is the zone any of the 'army base' nodes
 * \param X the node to test
 * \return true if it is an army base
 */
#define	IsArmyBase(X)	(((X) >= Z_ARMYBASE_START) && \
    ((X) <= Z_ARMYBASE_END))

#define DIR_UP		(1<<0)
#define	DIR_DOWN	(1<<1)
#define DIR_VER		((DIR_UP) | (DIR_DOWN))
#define	DIR_LEFT	(1<<2)
#define	DIR_RIGHT	(1<<3)
#define DIR_HOR		((DIR_LEFT) | (DIR_RIGHT))
#define	DIR_ALL		((DIR_HOR) | (DIR_VER))

typedef Int16 (*carryfn_t)(welem_t);
typedef Int16 (*carryfnarg_t)(welem_t, void *);

/*!
 * \brief Is one of the zones in the direction passed of the type passed
 * \param pos position to start from
 * \param checkfn function to assert or deny the test
 * \param dirs the directions to check
 * \return true if the direction passed is of the correct type
 */
EXPORT UInt8 CheckNextTo(UInt32 pos, UInt8 dirs, carryfn_t checkfn);

/*!
 * \brief check if one of the zones around it is of a certain type.
 * \param pos starting position
 * \param dirs the directions to check
 * \param checkfn function to check with
 * \param cfarg argument for the check function
 * \return the directions that match using the check function.
 */
EXPORT UInt8 CheckNextTo1(UInt32 pos, UInt8 dirs, carryfnarg_t checkfn,
    void *cfarg);

/*!
 * \brief Can the node carry power.
 * \param x the node entry
 * \return true if this node carries power
 */
EXPORT Int16 CarryPower(welem_t x);

/*!
 * \brief can the node carry water.
 * \param x the node entry
 * \return true if this node carries water
 */
EXPORT Int16 CarryWater(welem_t x);

/*
 * \brief End the simulation, free any memory that's in use
 */
void endSimulation(void);

#define	getScratch(i) (getWorldFlags(i) & SCRATCHBIT)
#define	setScratch(i) orWorldFlags((i), SCRATCHBIT)
#define	unsetScratch(i) andWorldFlags((i), (selem_t)~SCRATCHBIT)
#define	clearScratch() { \
	UInt32 XXX = 0; \
	for (; XXX < MapMul(); XXX++) unsetScratch(XXX); \
}

#if defined(__cplusplus)
}
#endif

#endif /* _SIMULATION_H_ */
