#include "handler.h"
#include "drawing.h"
#include "zakdef.h"
#include "ui.h"
#include "globals.h"
#include "handler.h"

int powerleft=0;


void PowerMoveOnFromThisPoint(unsigned long pos);
unsigned long PowerMoveOn(unsigned long pos, int direction);
int PowerNumberOfSquaresAround(unsigned long pos);
int PowerFieldCanCarry(unsigned long pos);
void UpgradeZones(void);
void UpgradeZone(long unsigned pos);
void DowngradeZone(long unsigned pos);
int DoTheRoadTrip(long unsigned int startPos);
signed long GetZoneScore(long unsigned int pos);
signed int GetScoreFor(unsigned char iamthis, unsigned char what);
long unsigned int GetRandomZone(void);

extern void Sim_DistributePower(void)
{
	unsigned long i,j;

	// reset powergrid
	LockWorldFlags();
	for (j=0; j<mapsize*mapsize; j++) { SetWorldFlags(j, GetWorldFlags(j) & 0xfc); }
	

	// Find all the powerplants and move out from there
	LockWorld(); // this lock locks for ALL power subs
	for (i=0; i<mapsize*mapsize; i++)
	{
		if (GetWorld(i) == 60 || GetWorld(i) == 61)
		{
			if ((GetWorldFlags(i) & 0x01) == 0) // have we already processed this powerplant?
			{
				
				powerleft=0;
				PowerMoveOnFromThisPoint(i);
				for (j=0; j<mapsize*mapsize; j++) { SetWorldFlags(j, GetWorldFlags(j) & 0xfd); }
			}

		}

	}
	UnlockWorld();
	UnlockWorldFlags();

}

void PowerMoveOnFromThisPoint(unsigned long pos)
{
	// a recursive function
	int cross = 0;
	int direction = 0;


	do
	{

		if ((GetWorldFlags(pos) & 0x01) == 0)
		{
			powerleft--;
			if (GetWorld(pos) == 60) { powerleft += 50; }
			if (GetWorld(pos) == 61) { powerleft +=150; }
		}

		SetWorldFlags(pos, GetWorldFlags(pos) | 0x03);

		if (powerleft < 1) { powerleft = 0; return; }
	
		cross = PowerNumberOfSquaresAround(pos);
		
		if ((cross & 0x0f) == 0)
		{
			return;
		}

		
		if ((cross & 0x0f) == 1)
		{
			// just a single way out
			if ((cross & 0x10) == 0x10)	     { direction = 0; }
			else if ((cross & 0x20) == 0x20) { direction = 1; }
			else if ((cross & 0x40) == 0x40) { direction = 2; }
			else if ((cross & 0x80) == 0x80) { direction = 3; }
		}
		
		if ((cross & 0x0f) != 1)
		{
			// we are at a power section
			if ((cross & 0x10) == 0x10) { PowerMoveOnFromThisPoint(pos-mapsize); }
			if ((cross & 0x20) == 0x20) { PowerMoveOnFromThisPoint(pos+1); }
			if ((cross & 0x40) == 0x40) { PowerMoveOnFromThisPoint(pos+mapsize); }
			if ((cross & 0x80) == 0x80) { PowerMoveOnFromThisPoint(pos-1); }
			return;
		}

		pos = PowerMoveOn(pos, direction);



	} while (PowerFieldCanCarry(pos));
	
}

int PowerFieldCanCarry(unsigned long pos)
{
	if ((GetWorldFlags(pos) & 0x02) == 0x02) { return 0; } // allready been here with this plant
	return CarryPower(GetWorld(pos));
}

int PowerNumberOfSquaresAround(unsigned long pos)
{
	// return:
	// 0001 00xx if up
	// 0010 00xx if right
	// 0100 00xx if down
	// 1000 00xx if left
	// xx = number of directions

	// count fields that carry power
	// do not look behind map border

	int retval=0;
	int number=0;


	if (PowerFieldCanCarry(PowerMoveOn(pos, 0))) { retval |= 0x10; number++; }
	if (PowerFieldCanCarry(PowerMoveOn(pos, 1))) { retval |= 0x20; number++; }
	if (PowerFieldCanCarry(PowerMoveOn(pos, 2))) { retval |= 0x40; number++; }
	if (PowerFieldCanCarry(PowerMoveOn(pos, 3))) { retval |= 0x80; number++; }

	retval |= number;

	return retval;
}



unsigned long PowerMoveOn(unsigned long pos, int direction)
{

	switch (direction)
	{
	case 0: // up
		if (pos < mapsize) { return pos; }
		pos -= mapsize;
		break;

	case 1: // right
		if ((pos+1) >= mapsize*mapsize) { return pos; }
		pos++;
		break;

	case 2: // down
		if ((pos+mapsize) >= mapsize*mapsize) { return pos; }
		pos += mapsize;
		break;

	case 3: //left
		if (pos == 0) { return pos; }
		pos--;
		break;
	}

	return pos;

}

void UpgradeZones()
{
	long unsigned int Zones[256];
	signed long ZonePoints[256];
	int i,j,zoneCount=0,topscorer;
	long signed topscore;
	long signed int randomZone;

	int downCount = 11*tax+30;
	int upCount = (0-8)*tax+250;

	int max = mapsize*3;
	if (max > 256) { max = 256; }


	// find some random zones and their scores
	for (i=0; i<max; i++)
	{
		randomZone = GetRandomZone();
		if (randomZone != -1) // -1 means we didn't find a zone
		{
			topscore = GetZoneScore(randomZone);
			if (topscore != -1)	// -1 means we can't do the road trip from that zone
			{
				Zones[zoneCount] = randomZone;
				ZonePoints[zoneCount] = topscore;
				zoneCount++;
			}
			else
			{
				// this zone should be downgrade right away
				DowngradeZone(randomZone);
			}
		}
	}


	// upgrade the bests
	for (i=0; i<zoneCount && i<upCount; i++)
	{
		topscore = 0;
		topscorer = -1;

		// find the one with max points
		for (j=0; j<zoneCount; j++)
		{
			if (ZonePoints[j] > topscore)
			{
				topscore = ZonePoints[j];
				topscorer = j;
			}
		}

		// upgrade him/her/it/whatever
		if (topscorer != -1)
		{
			if (ZonePoints[topscorer] != -1)
			{
				ZonePoints[topscorer] = -1;
				UpgradeZone(Zones[topscorer]);
			}
		}
	}

	// downgrade the worst
	for (i=0; i<zoneCount && i<downCount; i++)
	{
		topscore = -2; // sat to this to avoid downgrading recently upgraded zones
		topscorer = -1;

		// find the one with min points
		for (j=0; j<zoneCount; j++)
		{
			if (ZonePoints[j] < topscore)
			{
				topscore = ZonePoints[j];
				topscorer = j;
			}
		}

		// upgrade him/her/it/whatever
		if (topscorer != -1)
		{
			ZonePoints[topscorer] = 0;
			DowngradeZone(Zones[topscorer]);
		}
	}
	


}

void DowngradeZone(long unsigned pos)
{
	int type;
	LockWorld();

	type = GetWorld(pos);
	if (type >= 30 && type <= 39)
	{
		SetWorld(pos, (type == 30) ? 1 : type-1);
		BuildCount[COUNT_COMMERCIAL]--;
	}
	else if (type >= 40 && type <= 49)
	{
		SetWorld(pos, (type == 40) ? 2 : type-1);
		BuildCount[COUNT_RESIDENTIAL]--;
	}
	else if (type >= 50 && type <= 59)
	{
		SetWorld(pos, (type == 50) ? 3 : type-1);
		BuildCount[COUNT_INDUSTRIAL]--;
	}

	UnlockWorld();

}

void UpgradeZone(long unsigned pos)
{
	int type;

	LockWorld();
	
	type = GetWorld(pos);

	if (type == 1 || (type >= 30 && type <= 38))
	{
		SetWorld(pos, (type == 1) ? 30 : type+1);
		BuildCount[COUNT_COMMERCIAL]++;
	}
	else if (type == 2 || (type >= 40 && type <= 48))
	{
		SetWorld(pos, (type == 2) ? 40 : type+1);
		BuildCount[COUNT_RESIDENTIAL]++;
	}
	else if (type == 3 || (type >= 50 && type <= 58))
	{
		SetWorld(pos, (type == 3) ? 50 : type+1);
		BuildCount[COUNT_INDUSTRIAL]++;
	}
	UnlockWorld();
}




int DoTheRoadTrip(long unsigned int startPos)
{
	return 1; // for now
}


signed long GetZoneScore(long unsigned int pos)
{
	signed long score = 0;
	int x = pos%mapsize;
	int y = pos/mapsize;
	signed int i,j;
	int bRoad = 0;
	int type = 0;

	LockWorld();
	type = GetWorld(pos); //temporary holder
	type = ((IsZone(type, 1) != 0) ? 1 : ((IsZone(type,2) != 0) ? 2 : 3));

	LockWorldFlags();
	if ((GetWorldFlags(pos) & 0x01) == 0) { UnlockWorldFlags(); UnlockWorld(); return -1; } // whoops, no power
	UnlockWorldFlags();
	// take a look around at the enviroment ;)
	for (i=x-3; i<4+x; i++)
	{
		for (j=y-3; j<4+y; j++)
		{
			if (!(i<0 || i>=mapsize || j<0 || j>=mapsize))
			{
				
				score += GetScoreFor(type, GetWorld(WORLDPOS(i,j)));
				if (IsRoad(GetWorld(WORLDPOS(i,j))) && bRoad == 0)
				{
					// can we reach all kinds of zones from here?
					bRoad = DoTheRoadTrip(WORLDPOS(i,j));
				}

			}
		}
	}

	UnlockWorld();
	if (!bRoad) { return -1; }
	return score;
}


signed int GetScoreFor(unsigned char iamthis, unsigned char what)
{
	if (IsZone(what,1)) { return iamthis==1?0:(iamthis==2?50:(iamthis==3?50:50)); } // com
	if (IsZone(what,2)) { return iamthis==1?50:(iamthis==2?0:(iamthis==3?50:50)); } // res
	if (IsZone(what,3)) { return iamthis==1?(0-25):(iamthis==2?(0-75):(iamthis==3?25:(0-50))); } // ind
	if (IsRoad(what)  ) { return iamthis==1?75:(iamthis==2?50:(iamthis==3?75:66)); } // roads
	if (what == 60    ) { return iamthis==1?(0-75):(iamthis==2?(0-100):(iamthis==3?30:(0-75))); } // powerplant
	if (what == 61    ) { return iamthis==1?(0-150):(iamthis==2?(0-200):(iamthis==3?15:(0-175))); } // nuclearplant
	if (what == 21    ) { return iamthis==1?50:(iamthis==2?85:(iamthis==3?25:50)); } // tree
	if (what == 22    ) { return iamthis==1?150:(iamthis==2?500:(iamthis==3?85:200)); } // water
	return 0;
}


long unsigned int GetRandomZone()
{
	long unsigned int pos = 0;
	int i;
	unsigned char type;

	LockWorld();
	for (i=0; i<10; i++)
	{
		pos = GetRandomNumber(mapsize*mapsize);
		type = GetWorld(pos);
		if ((type >= 1 && type <= 3) || (type >= 30 && type <= 59))
		{
			UnlockWorld();
			return pos;
		}
	}

	UnlockWorld();
	return -1;

}


extern int Sim_DoPhase(int nPhase)
{
	switch (nPhase)
	{
	case 1:
		Sim_DistributePower();
		nPhase++;
		break;
	case 2:
		UpgradeZones();
		nPhase++;
		break;
	case 3:
	//	DowngradeZones();
		nPhase++;
		break;
	case 4:
		TimeElapsed++;
		nPhase++;
		UIInitDrawing();
		UIDrawCredits();
		UIFinishDrawing();
		break;
	}

	if (nPhase == 5) { return 0; }
	return nPhase;
}