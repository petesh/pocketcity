// the BUILD_* fits the defines in simcity.h (palm)
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
// these are "extras"
#define BUILD_FIRE_STATION          10
#define BUILD_POLICE_STATION        11
#define BUILD_MILITARY_BASE         12
#define BUILD_WATER_PIPE            13
#define BUILD_WATER_PUMP            14

#define BUILD_DEFENCE_FIRE          250
#define BUILD_DEFENCE_POLICE        251
#define BUILD_DEFENCE_MILITARY      252
///////////////////////////////////////


#define BUILD_COST_BULLDOZER        5
#define BUILD_COST_ZONE             50
#define BUILD_COST_ROAD             20
#define BUILD_COST_POWER_PLANT      3000
#define BUILD_COST_NUCLEAR_PLANT    10000
#define BUILD_COST_POWER_LINE       5
#define BUILD_COST_TREE             10
#define BUILD_COST_WATER            200
#define BUILD_COST_BRIDGE           100
#define BUILD_COST_FIRE_STATION     700
#define BUILD_COST_POLICE_STATION   500
#define BUILD_COST_MILITARY_BASE    10000
#define BUILD_COST_WATER_PIPES      20
#define BUILD_COST_WATER_PUMP       3000

extern void BuildSomething(int xpos, int ypos);
extern void Build_Bulldoze(int xpos, int ypos);
extern void Build_Destroy(int xpos, int ypos);
extern void CreateFullRiver(void);
extern void CreateForests(void);
extern void RemoveAllDefence(void);
