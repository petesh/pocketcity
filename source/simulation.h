/*! \file
 * \brief interface routines to the simulation
 */
#if !defined(_SIMULATION_H_)
#define	_SIMULATION_H_

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
/*! \brief the bit associated with scratch/unvisited in the worldflags */
#define	SCRATCHBIT	((unsigned char)0x80)

#ifdef __cplusplus
extern "C" {
#endif

#include <zakdef.h>
#include <compilerpragmas.h>

EXPORT void Sim_Distribute(void);
EXPORT void Sim_Distribute_Specific(Int16 type);
EXPORT Int16 Sim_DoPhase(Int16 nPhase);
EXPORT Int32 BudgetGetNumber(BudgetNumber type);
EXPORT void UpdateVolatiles(void);
EXPORT UInt32 getPopulation(void);

EXPORT Int16 IsRoad(welem_t x);
EXPORT Int16 IsRoadPower(welem_t x);
EXPORT Int16 IsRoadPipe(welem_t x);
EXPORT Int16 IsRoadBridge(welem_t x);
EXPORT Int16 IsRoadOrBridge(welem_t x);

EXPORT Int16 IsRail(welem_t x);
EXPORT Int16 IsRailPower(welem_t x);
EXPORT Int16 IsRailPipe(welem_t x);
EXPORT Int16 IsRailTunnel(welem_t x);
EXPORT Int16 IsRailOrTunnel(welem_t x);

EXPORT Int16 IsRailOvRoad(welem_t x);

EXPORT Int16 IsPowerWater(welem_t x);

EXPORT Int16 ZoneValue(welem_t x);
EXPORT Int16 IsPipe(welem_t x);
EXPORT Int16 IsPowerLine(welem_t x);
EXPORT Int16 IsWaterPipe(welem_t x);
EXPORT Int16 IsZone(welem_t x, zoneType nType);

EXPORT Int16 IsOccupied(welem_t x);

#define IsRealWater(X)	((X) == Z_REALWATER)
#define IsPump(X) ((X) == Z_PUMP)
#define IsCoalPlant(X)	(((X) >= Z_COALPLANT_START) && ((X) <= Z_COALPLANT_END))
#define IsNukePlant(X)	(((X) >= Z_NUCLEARPLANT_START) && \
    ((X) <= Z_NUCLEARPLANT_END))
#define	IsRealTree(X)	((X) == Z_REALTREE)
#define	IsFakeTree(X)	((X) == Z_FAKETREE)
#define	IsTree(X)	(IsFakeTree(X) || IsRealTree(X))
#define IsRealWater(X)	((X) == Z_REALWATER)
#define IsFakeWater(X)	((X) == Z_FAKEWATER)
#define IsWater(X)	(IsFakeWater(X) || IsRealWater(X))

#define IsSlum(X) (((X) >= Z_COMMERCIAL_SLUM) && ((X) <= Z_INDUSTRIAL_SLUM))

#define	IsGrowable(X) (IsSlum(X) || \
    (((X) >= Z_COMMERCIAL_MIN) && ((X) <= Z_INDUSTRIAL_MAX)))

#define DIR_UP		(1<<0)
#define	DIR_DOWN	(1<<1)
#define DIR_VER		((DIR_UP) | (DIR_DOWN))
#define	DIR_LEFT	(1<<2)
#define	DIR_RIGHT	(1<<3)
#define DIR_HOR		((DIR_LEFT) | (DIR_RIGHT))
#define	DIR_ALL		((DIR_HOR) | (DIR_VER))

typedef Int16 (*carryfn_t)(welem_t);
typedef Int16 (*carryfnarg_t)(welem_t, void *);

EXPORT UInt8 CheckNextTo(Int32 pos, UInt8 dirs, carryfn_t checkfn);
EXPORT UInt8 CheckNextTo1(Int32 pos, UInt8 dirs, carryfnarg_t checkfn,
    void *cfarg);

EXPORT Int16 CarryPower(welem_t x);
EXPORT Int16 CarryWater(welem_t x);

#ifdef __cplusplus
}
#endif

#define	getScratch(i) (getWorldFlags(i) & SCRATCHBIT)
#define	setScratch(i) orWorldFlags((i), SCRATCHBIT)
#define	unsetScratch(i) andWorldFlags((i), (selem_t)~SCRATCHBIT)
#define	clearScratch() { \
	long i = 0; \
	for (; i < MapMul(); i++) unsetScratch(i); \
}

#endif /* _SIMULATION_H_ */
