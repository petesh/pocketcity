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

void Sim_Distribute(void);
void Sim_Distribute_Specific(int type);
int Sim_DoPhase(int nPhase);
long int BudgetGetNumber(BudgetNumber type);
void UpdateVolatiles(void);

#define GetScratch(i) (GetWorldFlags(i) & SCRATCHBIT)
#define SetScratch(i) OrWorldFlags((i), SCRATCHBIT)
#define UnsetScratch(i) AndWorldFlags((i), (unsigned char)~SCRATCHBIT)
#define ClearScratch() { \
	long i = 0; \
	for (; i < GetMapMul(); i++ ) UnsetScratch(i); \
}

#endif /* _SIMULATION_H_ */
