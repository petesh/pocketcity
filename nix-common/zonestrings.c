/*! \file
 * This file translates a zone entry into a string for display.
 *
 * \todo Need to source the data from a configuration file. This replication
 * of information between the palm and the real platform is not going to cut
 * the mustard.
 */

#include <string.h>

#include <zonestrings.h>
#include <simulation.h>

static const struct type_zone {
	welem_t	tile_start; /*!< starting tile # */
	welem_t	tile_end; /*!< ending tile # */
	char *zonestring; /*!< string to use for display */
} type_zones[] = {
	{ Z_DIRT, Z_DIRT, "Empty Land" },
	{ Z_REALTREE, Z_REALTREE, "Forest" },
	{ Z_REALWATER, Z_REALWATER, "Real Water" },
	{ Z_FAKETREE, Z_FAKETREE, "Development Tree" },
	{ Z_FAKEWATER, Z_FAKEWATER, "Lake" },
	{ Z_PUMP, Z_PUMP, "Water Pump" },
	{ Z_WASTE, Z_WASTE, "Wasteland" },
	{ Z_FIRE1, Z_FIRE3, "Fire" },
	{ Z_CRATER, Z_CRATER, "Crater" },
	{ Z_PIPE_START, Z_PIPE_END, "Pipe" },
	{ Z_POWERLINE_START, Z_POWERLINE_END, "Power Line" },
	{ Z_POWERWATER_START, Z_POWERWATER_END, "Powerline and Water Pipe" },
	{ Z_COMMERCIAL_SLUM, Z_COMMERCIAL_SLUM, "Commercial Zone (new)" },
	{ Z_RESIDENTIAL_SLUM, Z_RESIDENTIAL_SLUM, "Residential Zone (new)" },
	{ Z_INDUSTRIAL_SLUM, Z_INDUSTRIAL_SLUM, "Industrial Zone (new)" },
	{ Z_COALPLANT_START, Z_COALPLANT_END, "Coal Plant" },
	{ Z_NUCLEARPLANT_START, Z_NUCLEARPLANT_END, "Nuclear Plant" },
	{ Z_FIRESTATION_START, Z_FIRESTATION_END, "Fire Station" },
	{ Z_POLICEDEPT_START, Z_POLICEDEPT_END, "Police Department" },
	{ Z_ARMYBASE_START, Z_ARMYBASE_END, "Army Base" },
	{ Z_COMMERCIAL_MIN, Z_COMMERCIAL_MAX, "Commercial Zone" },
	{ Z_RESIDENTIAL_MIN, Z_RESIDENTIAL_MAX, "Residential Zone" },
	{ Z_INDUSTRIAL_MIN, Z_INDUSTRIAL_MAX, "Industrial Zone" },
	{ Z_POWERROAD_START, Z_POWERROAD_END, "Powerline over Road" },
	{ Z_PIPEROAD_START, Z_PIPEROAD_END, "Pipe over Road" },
	{ Z_ROAD_START, Z_ROAD_END, "Road" },
	{ Z_BRIDGE_START, Z_BRIDGE_END, "Bridge" },
	{ Z_RAIL_START, Z_RAIL_END, "Rail" },
	{ Z_RAILPIPE_START, Z_RAILPIPE_END, "Rail under Water Pipe" },
	{ Z_RAILPOWER_START, Z_RAILPOWER_END, "Rail under Powerline" },
	{ Z_RAILOVROAD_START, Z_RAILOVROAD_END, "Rail over Road" },
	{ Z_RAILTUNNEL_START, Z_RAILTUNNEL_END, "Rail Tunnel" },
	{ 0, 0, 0 }
};

int
getFieldString(welem_t world, char *dest, int destlen)
{
	int i = 0;

	while (type_zones[i].zonestring != NULL) {
		if ((world >= type_zones[i].tile_start) &&
		    (world <= type_zones[i].tile_end)) {
			break;
		}
		i++;
	}
	if (type_zones[i].zonestring == NULL) return (-1);
	strncpy(dest, type_zones[i].zonestring, destlen - 1);
	dest[destlen - 1] = '\0';
	return (0);
}

char *densities[] = {
	"Low",
	"Medium",
	"High",
	"Very High"
};

char *values[] = {
	"Slum",
	"Lower Class",
	"Middle Class",
	"Upper Class"
};

int
getFieldValue(welem_t world, char *dest, int destlen)
{
	strncpy(dest, values[ZoneValue(world) % 4], destlen - 1);
	values[destlen - 1] = '\0';

	return (0);
}

int
getFieldDensity(welem_t world, char *dest, int destlen)
{
	strncpy(dest, values[(ZoneValue(world) / 4) % 4], destlen - 1);
	values[destlen - 1] = '\0';

	return (0);
}
