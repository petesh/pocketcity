extern void Sim_Distribute(void);
extern void Sim_Distribute_Specific(int type);
extern int Sim_DoPhase(int nPhase);
extern signed long int BudgetGetNumber(int type);
extern void UpdateVolatiles(void);

void NewScratch(void);
void FreeScratch(void);
int GetScratch(long i);
void SetScratch(long i);
void ClearScratch(void);

enum BudgetNumbers {
    BUDGET_RESIDENTIAL,
    BUDGET_COMMERCIAL,
    BUDGET_INDUSTRIAL,
    BUDGET_TRAFFIC,
    BUDGET_POWER,
    BUDGET_DEFENCE,
    BUDGET_CURRENT_BALANCE,
    BUDGET_CHANGE,
    BUDGET_NEXT_MONTH
};

#define POWEREDBIT      ((unsigned char)0x01)
#define WATEREDBIT      ((unsigned char)0x02)
#define SCRATCHBIT      ((unsigned char)0x80)
