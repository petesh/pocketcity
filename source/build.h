#define BUILD_BULLDOZER             0
#define BUILD_ZONE_RESIDENTIAL      1
#define BUILD_ZONE_COMMERCIAL       2
#define BUILD_ZONE_INDUSTRIAL       3
#define BUILD_ROAD                  4
#define BUILD_POWER_PLANT           5
#define BUILD_NUCLEAR_PLANT         6
#define BUILD_POWER_LINE            7
#define BUILD_TREE                  8
#define BUILD_WATER                 9


#define BUILD_COST_BULLDOZER        5
#define BUILD_COST_ZONE             50
#define BUILD_COST_ROAD             20
#define BUILD_COST_POWER_PLANT      3000
#define BUILD_COST_NUCLEAR_PLANT    10000
#define BUILD_COST_POWER_LINE       5
#define BUILD_COST_TREE             10
#define BUILD_COST_WATER            200


extern void BuildSomething(int xpos, int ypos);
extern void Build_Bulldoze(int xpos, int ypos);
extern void Build_Destroy(int xpos, int ypos);
extern void CreateFullRiver(void);
extern void CreateForests(void);
