#define TBMP 1415736688


#define ERROR_OUT_OF_MEMORY		1



#define ZONE_COMMERCIAL			1
#define ZONE_RESIDENTIAL		2
#define ZONE_INDUSTRIAL			3


#define COUNT_RESIDENTIAL		0
#define COUNT_COMMERCIAL		1
#define COUNT_INDUSTRIAL		2
#define COUNT_ROADS				3
#define COUNT_TREES				4
#define COUNT_WATER				5


#define WORLDPOS(x,y)			((x)+(y)*mapsize)


#define SIM_GAME_LOOP_SECONDS	10	// minimum number of seconds a game frame lasts


#ifdef __palmos__
#define TILE_SIZE 24
#else // we have a nokia :)
#define TILE_SIZE 24
#endif