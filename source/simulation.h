#if !defined(_SIMULATION_H_)
#define _SIMULATION_H_

typedef enum {
    bnResidential = 0,
    bnCommercial,
    bnIndustrial,
    bnTraffic,
    bnPower,
    bnDefence,
    bnCurrentBalance,
    bnChange,
    bnNextMonth
} BudgetNumber;

#define POWEREDBIT      ((unsigned char)0x01)
#define WATEREDBIT      ((unsigned char)0x02)
#define SCRATCHBIT      ((unsigned char)0x80)

#ifdef __cplusplus
extern "C" {
#endif

void Sim_Distribute(void);
void Sim_Distribute_Specific(Int16 type);
Int16 Sim_DoPhase(Int16 nPhase);
Int32 BudgetGetNumber(BudgetNumber type);
void UpdateVolatiles(void);

#ifdef __cplusplus
}
#endif

#define GetScratch(i) (GetWorldFlags(i) & SCRATCHBIT)
#define SetScratch(i) OrWorldFlags((i), SCRATCHBIT)
#define UnsetScratch(i) AndWorldFlags((i), (unsigned char)~SCRATCHBIT)
#define ClearScratch() { \
	long i = 0; \
	for (; i < GetMapMul(); i++ ) UnsetScratch(i); \
}

#endif /* _SIMULATION_H_ */
