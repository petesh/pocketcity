/*!
 * \file
 * \brief resource constants for the game
 *
 * Contains all the definitions for the resources from the application.
 * They are made use of by the various forms to create and check elements
 * that are selected. Some of the fields have multiple interpretations
 */
/*!
 * These items are referenced in an array in the simcity.c program -
 * they need to be in the same order here as the handler entries in the
 * id2handler array in simcity.c
 */
#define	formID_pocketCity 1000
#define	formID_budget 1001
#define	formID_map 1002
#define	formID_files 1003
#define	formID_filesNew 1004
#define	formID_quickList 1005
#define	formID_extraBuild 1006
#define	formID_options 1007
#define	formID_ButtonConfig		 1008
#define	formID_Query	1009
/* Not Used yet */
#define	formID_DistribDetails 1010

#define	menu_pocketCity 	1000
#define	menu_budget 	1001
#define	menu_map 	1002
#define fpc_gadgetUI	1003

#define	menuitemID_loadGame 1000
#define	menuitemID_saveGame 1001
#define	menuitemID_Budget 1002
#define	menuitemID_Map 1003

#define	menuitemID_Configuration 1004
#define	menuitemID_Buttons 1005
#define menuitemID_ForceResupply	1006

#define	mi_removeDefence		1007
#define	mi_buildExtra 1008 /* pops up an extra list */

#define	mi_CauseFire			1009
#define	mi_CauseMeltDown		1010
#define	mi_CauseMonster			1011
#define	mi_CauseDragon			1012
#define	mi_CauseMeteor			1013

#define	menuID_SlowSpeed 1014
#define	menuID_MediumSpeed 1015
#define	menuID_FastSpeed 1016
#define	menuID_TurboSpeed 1017
#define	menuID_PauseSpeed 1018

#define	menuitemID_about 1019
#define	menuitemID_tips 1020

#define menuitemID_Beam 1021

#define	menuitemID_Funny 1022

#define	gi_buildBulldoze 1100
#define	gi_buildResidential 1101
#define	gi_buildCommercial 1102
#define	gi_buildIndustrial 1103
#define	gi_buildRoad 1104
#define	gi_buildRail 1105

#define	gi_buildPowerPlant 1106
#define	gi_buildNuclearPlant 1107
#define	gi_buildPowerLine 1108

#define	gi_buildWaterPlant 1109
#define	gi_buildWaterPipe 1110

#define	gi_buildTree 1111
#define	gi_buildWater 1112
#define	gi_defenceFire 1113
#define	gi_defencePolice 1114
#define	gi_defenceMilitary 1115
#define	gi_queryItem 1116
#define	gi_buildExtra 1117

#define	listID_shifter_popup 5000
#define	listID_shifter 5001

/* notice how the IDs here are the same as the menuIDs (uptil the "extra") */
/*
 * Button IDs are the 'index' of the button within the button matrix. The
 * defined matrix is 10x(whatever).
 */
#define	bitmapID_iconBulldoze 1100
#define	bitmapID_iconResidential 1101
#define	bitmapID_iconCommercial 1102
#define	bitmapID_iconIndustrial 1103
#define	bitmapID_iconRoad 1104
#define	bitmapID_iconRail 1105
#define	bitmapID_iconPowerPlant 1106
#define	bitmapID_iconNuclear 1107
#define	bitmapID_iconPowerline 1108

#define	bitmapID_iconWaterPlant 1109
#define	bitmapID_iconWaterPipe 1110

#define	bitmapID_iconTree 1111
#define	bitmapID_iconWater 1112

#define	bitmapID_iconDefFire 1113
#define	bitmapID_iconDefPolice 1114
#define	bitmapID_iconDefMilitary 1115

#define	bitmapID_iconQuery 1116

/* anything after this isn't on the toolbar */
#define	bitmapID_iconExtra 1117

/* Make sure this keeps in sync with the iconExtra */
#define	OFFSET_EXTRA (bitmapID_iconExtra-bitmapID_iconBulldoze)

#define	bitmapID_iconPushed 1400

#define	alertID_errorOutOfMemory 1000
#define	alertID_RomIncompatible 1001
#define	alertID_outMoney 1002
#define	alertID_lowFunds 1003
#define	alertID_about 1004
#define	alertID_loadGame 1006
#define	alertID_saveGame 1007
#define	alertID_invalidSaveVersion 1008
#define	alertID_generic_disaster 1009
#define	alertID_majorbad 1010
#define	alertID_programmingNiggle 1011
#define	alertID_tilesMissing		 1012
#define alertID_lowPower	1013
#define alertID_outPower	1014
#define alertID_lowWater	1015
#define alertID_outWater	1016

#define	st_disasters			2000

#define	bitmapID_Speed 3000

/* menu for budget */
#define	menuitemID_BudgetBack 1300

#define	fieldID_taxrate		1300
#define	fieldID_budget_tra	1301
#define	fieldID_budget_pow	1302
#define	fieldID_budget_def	1303

#define rbutton_taxdown		1304
#define rbutton_taxup		1305
#define rbutton_trafdown	1306
#define rbutton_trafup		1307
#define rbutton_powdown		1308
#define rbutton_powup		1309
#define rbutton_defdown		1310
#define rbutton_defup		1311

#define	labelID_budget_inc 1312

#define	labelID_budget_tra 1313
#define	labelID_budget_pow 1314
#define	labelID_budget_def 1315

#define	labelID_budget_now 1316
#define	labelID_budget_tot 1317
#define	labelID_budget_bal 1318

/* menu for map */
#define	menuitemID_MapBack 1340

/* controls for files */
#define	listID_FilesList 1360
#define	buttonID_FilesNew 1361
#define	buttonID_FilesLoad 1362
#define	buttonID_FilesDelete 1363

/* controls for files=>new */
#define	fieldID_newGameName 1010
#define	buttonID_FilesNewCreate 1011
#define	buttonID_FilesNewCancel 1012
#define	fieldID_width	1013
#define	fieldID_height	1014
#define rbutton_widthdown 1015
#define rbutton_widthup 1016
#define rbutton_heightdown 1017
#define rbutton_heightup 1018

/* controls for options form  - double up on create game form */
#define	buttonID_dis_off 1000
#define	buttonID_dis_one 1001
#define	buttonID_dis_two 1002
#define	buttonID_dis_three 1003

#define	buttonID_Easy 1004
#define	buttonID_Medium 1005
#define	buttonID_Hard 1006

#define	checkboxID_autobulldoze 1010
#define	checkboxID_minimapvisible 1011
#define	checkboxID_minimapdetailed 1012

/* Controls for the DistribDetails Form */
#define	pb_dd_overview 1000
#define	pb_dd_power 1001
#define	pb_dd_water 1002

#define	la_dd_sitespowered		1003
#define	la_dd_powersupplied		1004
#define	la_dd_bubbles			1005

/* ok and cancel */
#define	buttonID_OK 1100
#define	buttonID_Cancel 1101

/* controls for extra Build list */
#define	listID_extraBuildList 1013

#define	buttonID_extraBuildSelect 1014
#define	buttonID_extraBuildCancel 1015

#define	buttonID_extraBuildFireMen 1016
#define	buttonID_extraBuildPolice 1017
#define	buttonID_extraBuildMilitary 1018

#define	labelID_extraBuildPrice 1019
#define	labelID_extraBuildDescription 1020
#define	strID_Descriptions 1021
#define	strID_Items 1022
#define	wdlID_Costs 1023
#define strID_Months 1024
#define strID_densities 1025
#define strID_values 1026
#define strID_crime 1027
#define strID_pollution 1028

#define	bitmapID_zones 1600
#define	bitmapID_monsters 1601
#define	bitmapID_units 1602

#define	bitmapID_coin 1700
#define	bitmapID_popu 1701
#define	bitmapID_loca 1702

#define	bitmapID_updn 1703
#define	bitmapID_ltrt 1704

#define	StrID_tips 1021
#define	StrID_build 1022
#define	StrID_Popups 1023

#define	List_Cal 2500
#define	List_Cal_Popup 2501
#define	List_Addr 2502
#define	List_Addr_Popup 2503
#define	List_HrUp 2504
#define	List_HrUp_Popup 2505
#define	List_HrDn 2506
#define	List_HrDn_Popup 2507
#define	List_ToDo 2508
#define	List_ToDo_Popup 2509
#define	List_Memo 2510
#define	List_Memo_Popup 2511

#define	List_Calc	2512
#define	List_Calc_Popup	2513
#define	List_Find	2514
#define	List_Find_Popup	2515

#define	List_JogUp 2516
#define	List_JogUp_Popup 2517
#define	List_JogDn 2518
#define	List_JogDn_Popup 2519
#define	List_JogOut 2520
#define	List_JogOut_Popup 2521

#define List_RockerLeft 2522
#define List_RockerLeft_Popup 2523
#define List_RockerRight 2524
#define List_RockerRight_Popup 2525
#define List_RockerCenter 2526
#define List_RockerCenter_Popup 2527

/* Controls for the Query Form */
#define	labelID_zonetype 1001
#define labelID_zonevalue 1002
#define labelID_zonedensity 1003
#define	labelID_pollution 1004
#define labelID_crimelevel 1005
#define labelID_ispowered 1006
#define labelID_iswatered 1007
#define labelID_carrypower 1008
#define labelID_carrywater 1009
#define labelID_zonelocation 1010

/* strings */
#define pcity_description	3390

/* Resource Strings */
#define	resstrings_base	4096
#define	si_cash_scale	0
#define	si_empty_land	1
#define	si_power_line	2
#define	si_road		3
#define	si_rail		4
#define	si_realwater	5
#define	si_forest	6
#define	si_maprender	7
#define	si_fakewater	8
#define	si_faketree	9
#define	si_pump		10
#define	si_waste	11
#define	si_fire		12
#define	si_crater	13
#define	si_pipe		14
#define	si_powerline	15
#define	si_powerwater	16
#define	si_commercialslum	17
#define	si_residentialslum	18
#define	si_industrialslum	19
#define	si_coalplant	20
#define	si_nuclearplant	21
#define	si_firestation	22
#define	si_policedept	23
#define	si_armybase	24
#define	si_commercial	25
#define	si_residential	26
#define	si_industrial	27
#define	si_powerroad	28
#define	si_piperoad	29
#define si_bridge	30
#define	si_railpower	31
#define	si_railpipe	32
#define	si_railovroad	33
#define si_railtunnel	34
#define si_carries	35
#define si_needs	36
