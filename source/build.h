#if !defined(_BUILD_H_)
#define _BUILD_H_

/* the BUILD_* fits the defines in simcity.h (palm) */
typedef enum {
    Be_Bulldozer = 0,
    Be_Zone_Residential,
    Be_Zone_Commercial,
    Be_Zone_Industrial,
    Be_Road,
    Be_Power_Plant,
    Be_Nuclear_Plant,
    Be_Power_Line,
    Be_Water_Pump,
    Be_Water_Pipe,
    Be_Tree,
    Be_Water,
    Be_Fire_Station,
    Be_Police_Station,
    Be_Military_Base,
    /* Defense items are bigger. */
    Be_Defence_Fire,
    Be_Defence_Police,
    Be_Defence_Military,
    Be_Extra
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

extern void Build_Bulldoze(int xpos, int ypos, unsigned int _unused);
extern void BuildSomething(int xpos, int ypos);
extern void Build_Destroy(int xpos, int ypos);
extern void CreateFullRiver(void);
extern void CreateForests(void);
extern void RemoveAllDefence(void);

#endif /* _BUILD_H_ */
