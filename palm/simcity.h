#include "../source/zakdef.h"

// global vars from simcity.c
extern short int game_in_progress;
extern MemPtr worldPtr;
extern MemPtr worldFlagsPtr;
extern void UIWriteLog(char * s);


#define formID_pocketCity                1000
#define formID_budget                    1001
#define formID_map                       1002
#define formID_files                     1003
#define formID_filesNew                  1004
#define formID_quickList                 1005

#define menuID_pocketCity                1000
#define menuID_budget                    1001
#define menuID_map                       1002
#define menuID_files                     1003

#define menuitemID_loadGame              1001
#define menuitemID_saveGame              1002
#define menuitemID_about                 1003
#define menuitemID_Budget                1004
#define menuitemID_Map                   1005
#define menuitemID_Funny                 1006
#define menuID_SlowSpeed                 1052
#define menuID_MediumSpeed               1053
#define menuID_FastSpeed                 1054
#define menuID_TurboSpeed                1055
#define menuID_PauseSpeed                1056

#define poplabel                         1055

#define menuitemID_buildBulldoze         1100
#define menuitemID_buildResidential      1101
#define menuitemID_buildCommercial       1102
#define menuitemID_buildIndustrial       1103
#define menuitemID_buildRoad             1104
#define menuitemID_buildPowerPlant       1105
#define menuitemID_buildNuclearPlant     1106
#define menuitemID_buildPowerLine        1107
#define menuitemID_buildTree             1108
#define menuitemID_buildWater            1109

// notice how the IDs here are the same as the menuIDs
#define bitmapID_iconBulldoze            1100
#define bitmapID_iconResidential         1101
#define bitmapID_iconCommercial          1102
#define bitmapID_iconIndustrial          1103
#define bitmapID_iconRoad                1104
#define bitmapID_iconPowerPlant          1105
#define bitmapID_iconNuclear             1106
#define bitmapID_iconPowerline           1107
#define bitmapID_iconTree                1108
#define bitmapID_iconWater               1109
#define bitmapID_iconPushed              1400

#define alertID_errorOutOfMemory         1000
#define alertID_RomIncompatible          1001
#define alertID_outMoney                 1002
#define alertID_lowFunds                 1003
#define alertID_about                    1004
#define alertID_loadGame                 1006
#define alertID_saveGame                 1007
#define alertID_invalidSaveVersion	 1008
#define alertID_gameSaved		 1009
#define alertID_gameLoaded		 1010
#define alertID_fireOutBreak         1011
#define alertID_plantExplosion         1012

// the jumps in these defines makes coding easier
#define bitmapID_SpeedPaused            3000
#define bitmapID_SpeedSlow              3015
#define bitmapID_SpeedNormal            3010
#define bitmapID_SpeedFast              3005
#define bitmapID_SpeedUltra             3001

// menu for budget
#define menuitemID_BudgetBack           1300   
#define labelID_budget_res              1310 
#define labelID_budget_com              1311
#define labelID_budget_ind              1312
#define labelID_budget_tra              1313
#define labelID_budget_pow              1314
#define labelID_budget_tot              1315
#define labelID_budget_bal              1316
#define labelID_budget_now              1317

// menu for map
#define menuitemID_MapBack              1340

// controls for files
#define listID_FilesList                1360
#define buttonID_FilesNew               1361
#define buttonID_FilesLoad              1362
#define buttonID_FilesDelete            1363
// menu for files
#define menuitemID_FilesNew             1360
#define menuitemID_FilesOpen            1361
#define menuitemID_FilesDelete          1362


// controls for files=>new
#define fieldID_newGameName             1010
#define buttonID_FilesNewCreate         1011
#define buttonID_FilesNewCancel        1012

#define bitmapID_zones      1600
#define bitmapID_monsters   1601

