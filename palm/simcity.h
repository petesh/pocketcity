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


// the bitmap for the tiles
#define bitmapID_PowerLossOverlay       2001 // 32x32
#define bitmapID_PowerLossOverlay2      2002 // 16x16


// controls for files=>new
#define fieldID_newGameName             1010
#define buttonID_FilesNewCreate         1011
#define buttonID_FilesNewCancel        1012

/*
    1x1        \
    2x2         } These are specials
    4x4        /
    8x8        ID: 1000-1199
    16x16    ID: 1200-1399
    32x32    ID: 1400-1599

*/



// IDs for 32x32 tiles
#define bitmapID_DirtBmp                1400
#define bitmapID_CommZone               1401
#define bitmapID_ResZone                1402
#define bitmapID_IndZone                1403

#define bitmapID_PowerRoadUpDown        1406
#define bitmapID_PowerRoadLeftRight     1407
#define bitmapID_PowerLoss              1408
#define bitmapID_CursorBmp              1409
#define bitmapID_RoadLeftRight          1410
#define bitmapID_RoadUpDown             1411
#define bitmapID_RoadRightDown          1412
#define bitmapID_RoadLeftDown           1413
#define bitmapID_RoadLeftUp             1414
#define bitmapID_RoadRightUp            1415
#define bitmapID_RoadLeftUpDown         1416
#define bitmapID_RoadRightUpDown        1417
#define bitmapID_RoadRightLeftDown      1418
#define bitmapID_RoadRightLeftUp        1419
#define bitmapID_RoadCross              1420
#define bitmapID_TreeBmp                1421
#define bitmapID_WaterBmp               1422

#define bitmapID_CommOne                1430
#define bitmapID_CommTwo                1431
#define bitmapID_CommThree              1432
#define bitmapID_CommFour               1433
#define bitmapID_CommFive               1434
#define bitmapID_CommSix                1435
#define bitmapID_CommSeven              1436
#define bitmapID_CommEight              1437
#define bitmapID_CommNine               438
#define bitmapID_CommTen                1439
#define bitmapID_ResOne                 1440
#define bitmapID_ResTwo                 1441
#define bitmapID_ResThree               1442
#define bitmapID_ResFour                1443
#define bitmapID_ResFive                1444
#define bitmapID_ResSix                 1445
#define bitmapID_ResSeven               1446
#define bitmapID_ResEight               1447
#define bitmapID_ResNine                1448
#define bitmapID_ResTen                 1449
#define bitmapID_IndOne                 1450
#define bitmapID_IndTwo                 1451
#define bitmapID_IndThree               1452
#define bitmapID_IndFour                1453
#define bitmapID_IndFive                1454
#define bitmapID_IndSix                 1455
#define bitmapID_IndSeven               1456
#define bitmapID_IndEight               1457
#define bitmapID_IndNine                1458
#define bitmapID_IndTen                 1459
#define bitmapID_PowerPlant             1460
#define bitmapID_NulearPlant            1461

#define bitmapID_PowerLeftRight         1470
#define bitmapID_PowerUpDown            1471
#define bitmapID_PowerRightDown         1472
#define bitmapID_PowerLeftDown          1473
#define bitmapID_PowerLeftUp            1474
#define bitmapID_PowerRightUp           1475
#define bitmapID_PowerLeftUpDown        1476
#define bitmapID_PowerRightUpDown       1477
#define bitmapID_PowerLeftRightDown     1478
#define bitmapID_PowerLeftRightUp       1479
#define bitmapID_PowerCross             1480


// IDs for 16x16 tiles
#define bitmapID_DirtBmp2               1200
#define bitmapID_CommZone2              1201
#define bitmapID_ResZone2               1202
#define bitmapID_IndZone2               1203

#define bitmapID_PowerRoadUpDown2       1206
#define bitmapID_PowerRoadLeftRight2    1207
#define bitmapID_PowerLoss2             1208
#define bitmapID_CursorBmp2             1209
#define bitmapID_RoadLeftRight2         1210
#define bitmapID_RoadUpDown2            1211
#define bitmapID_RoadRightDown2         1212
#define bitmapID_RoadLeftDown2          1213
#define bitmapID_RoadLeftUp2            1214
#define bitmapID_RoadRightUp2           1215
#define bitmapID_RoadLeftUpDown2        1216
#define bitmapID_RoadRightUpDown2       1217
#define bitmapID_RoadRightLeftDown2     1218
#define bitmapID_RoadRightLeftUp2       1219
#define bitmapID_RoadCross2             1220
#define bitmapID_TreeBmp2               1221
#define bitmapID_WaterBmp2              1222

#define bitmapID_CommOne2               1230
#define bitmapID_CommTwo2               1231
#define bitmapID_CommThree2             1232
#define bitmapID_CommFour2              1233
#define bitmapID_CommFive2              1234
#define bitmapID_CommSix2               1235
#define bitmapID_CommSeven2             1236
#define bitmapID_CommEight2             1237
#define bitmapID_CommNine2              1238
#define bitmapID_CommTen2               1239
#define bitmapID_ResOne2                1240
#define bitmapID_ResTwo2                1241
#define bitmapID_ResThree2              1242
#define bitmapID_ResFour2               1243
#define bitmapID_ResFive2               1244
#define bitmapID_ResSix2                1245
#define bitmapID_ResSeven2              1246
#define bitmapID_ResEight2              1247
#define bitmapID_ResNine2               1248
#define bitmapID_ResTen2                1249
#define bitmapID_IndOne2                1250
#define bitmapID_IndTwo2                1251
#define bitmapID_IndThree2              1252
#define bitmapID_IndFour2               1253
#define bitmapID_IndFive2               1254
#define bitmapID_IndSix2                1255
#define bitmapID_IndSeven2              1256
#define bitmapID_IndEight2              1257
#define bitmapID_IndNine2               1258
#define bitmapID_IndTen2                1259
#define bitmapID_PowerPlant2            1260
#define bitmapID_NulearPlant2           1261
#define bitmapID_Waste2                 1262
#define bitmapID_Fire1_2                1263
#define bitmapID_Fire2_2                1264
#define bitmapID_Fire3_2                1265
#define bitmapID_RealWaterBmp2          1266

#define bitmapID_PowerLeftRight2        1270
#define bitmapID_PowerUpDown2           1271
#define bitmapID_PowerRightDown2        1272
#define bitmapID_PowerLeftDown2         1273
#define bitmapID_PowerLeftUp2           1274
#define bitmapID_PowerRightUp2          1275
#define bitmapID_PowerLeftUpDown2       1276
#define bitmapID_PowerRightUpDown2      1277
#define bitmapID_PowerLeftRightDown2    1278
#define bitmapID_PowerLeftRightUp2      1279
#define bitmapID_PowerCross2            1280
