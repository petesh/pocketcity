// the BUILD_* fits the defines in simcity.h (palm)
typedef enum {
    Build_Bulldozer = 0,
    Build_Zone_Residential,
    Build_Zone_Commercial,
    Build_Zone_Industrial,
    Build_Road,
    Build_Power_Plant,
    Build_Nuclear_Plant,
    Build_Power_Line,
    Build_Water_Pump,
    Build_Water_Pipe,
    Build_Tree,
    Build_Water,
    Build_Fire_Station,
    Build_Police_Station,
    Build_Military_Base,
    // Defense items are bigger.
    Build_Defence_Fire,
    Build_Defence_Police,
    Build_Defence_Military,
    Build_Extra
} BuildCodes;

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
extern void Build_Destroy(int xpos, int ypos);
extern void CreateFullRiver(void);
extern void CreateForests(void);
extern void RemoveAllDefence(void);
