extern void Sim_Distribute(char type);
extern int Sim_DoPhase(int nPhase);
extern signed long int BudgetGetNumber(int type);

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

#define POWEREDBIT      0x01
#define SCRATCHBIT      0x02
#define WATEREDBIT      0x04
#define TYPEPOWER       0
#define TYPEWATER       1
