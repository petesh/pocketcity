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

void Sim_Distribute(void);
void Sim_Distribute_Specific(Int16 type);
Int16 Sim_DoPhase(Int16 nPhase);
Int32 BudgetGetNumber(BudgetNumber type);
void UpdateVolatiles(void);
UInt32 getPopulation(void);

Int16 IsRoad(welem_t x);
Int16 IsRoadPower(welem_t x);
Int16 IsRoadPipe(welem_t x);
Int16 IsPowerWater(welem_t x);

Int16 ZoneValue(welem_t x);
Int16 IsPipe(welem_t x);
Int16 IsBridge(welem_t x);
Int16 IsPowerLine(welem_t x);
Int16 IsRoadPower(welem_t x);
Int16 IsWaterPipe(welem_t x);
Int16 IsRoadWater(welem_t x);
Int16 IsBridge(welem_t x);
Int16 IsZone(welem_t x, zoneType nType);

Int16 CarryPower(welem_t x);
Int16 CarryWater(welem_t x);

#ifdef __cplusplus
}
#endif

#define	GetScratch(i) (GetWorldFlags(i) & SCRATCHBIT)
#define	SetScratch(i) OrWorldFlags((i), SCRATCHBIT)
#define	UnsetScratch(i) AndWorldFlags((i), (unsigned char)~SCRATCHBIT)
#define	ClearScratch() { \
	long i = 0; \
	for (; i < MapMul(); i++) UnsetScratch(i); \
}

#endif /* _SIMULATION_H_ */
