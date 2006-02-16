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
#include <localize.h>

/*!
 * mapping of zone tiles to their corresponding zone type
 */
static const struct type_zone {
	welem_t	tile_start; /*!< starting tile # */
	welem_t	tile_end; /*!< ending tile # */
	char *zonestring; /*!< string to use for display */
} type_zones[] = {
	{ Z_DIRT, Z_DIRT, N_("Empty Land") },
	{ Z_REALTREE, Z_REALTREE, N_("Forest") },
	{ Z_REALWATER, Z_REALWATER, N_("Real Water") },
	{ Z_FAKETREE, Z_FAKETREE, N_("Development Tree") },
	{ Z_FAKEWATER, Z_FAKEWATER, N_("Lake") },
	{ Z_PUMP, Z_PUMP, N_("Water Pump") },
	{ Z_WASTE, Z_WASTE, N_("Wasteland") },
	{ Z_FIRE1, Z_FIRE3, N_("Fire") },
	{ Z_CRATER, Z_CRATER, N_("Crater") },
	{ Z_PIPE_START, Z_PIPE_END, N_("Pipe") },
	{ Z_POWERLINE_START, Z_POWERLINE_END, N_("Power Line") },
	{ Z_POWERWATER_START, Z_POWERWATER_END,
		N_("Powerline and Water Pipe") },
	{ Z_COMMERCIAL_SLUM, Z_COMMERCIAL_SLUM,
		N_("Commercial Zone (new)") },
	{ Z_RESIDENTIAL_SLUM, Z_RESIDENTIAL_SLUM,
		N_("Residential Zone (new)") },
	{ Z_INDUSTRIAL_SLUM, Z_INDUSTRIAL_SLUM, N_("Industrial Zone (new)") },
	{ Z_COALPLANT_START, Z_COALPLANT_END, N_("Coal Plant") },
	{ Z_NUCLEARPLANT_START, Z_NUCLEARPLANT_END, N_("Nuclear Plant") },
	{ Z_FIRESTATION_START, Z_FIRESTATION_END, N_("Fire Station") },
	{ Z_POLICEDEPT_START, Z_POLICEDEPT_END, N_("Police Department") },
	{ Z_ARMYBASE_START, Z_ARMYBASE_END, N_("Army Base") },
	{ Z_COMMERCIAL_MIN, Z_COMMERCIAL_MAX, N_("Commercial Zone") },
	{ Z_RESIDENTIAL_MIN, Z_RESIDENTIAL_MAX, N_("Residential Zone") },
	{ Z_INDUSTRIAL_MIN, Z_INDUSTRIAL_MAX, N_("Industrial Zone") },
	{ Z_POWERROAD_START, Z_POWERROAD_END, N_("Powerline over Road") },
	{ Z_PIPEROAD_START, Z_PIPEROAD_END, N_("Pipe over Road") },
	{ Z_ROAD_START, Z_ROAD_END, N_("Road") },
	{ Z_BRIDGE_START, Z_BRIDGE_END, N_("Bridge") },
	{ Z_RAIL_START, Z_RAIL_END, N_("Rail") },
	{ Z_RAILPIPE_START, Z_RAILPIPE_END, N_("Rail under Water Pipe") },
	{ Z_RAILPOWER_START, Z_RAILPOWER_END, N_("Rail under Powerline") },
	{ Z_RAILOVROAD_START, Z_RAILOVROAD_END, N_("Rail over Road") },
	{ Z_RAILTUNNEL_START, Z_RAILTUNNEL_END, N_("Rail Tunnel") },
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
	strncpy(dest, gettext(type_zones[i].zonestring), destlen - 1);
	dest[destlen - 1] = '\0';
	return (0);
}

char *densities[] = {
	N_("Low"),
	N_("Medium"),
	N_("High"),
	N_("Very High")
};

char *values[] = {
	N_("Slum"),
	N_("Lower Class"),
	N_("Middle Class"),
	N_("Upper Class")
};

int
getFieldValue(welem_t world, char *dest, int destlen)
{
	strncpy(dest, gettext(values[ZoneValue(world) % 4]), destlen - 1);
	values[destlen - 1] = '\0';

	return (0);
}

int
getFieldDensity(welem_t world, char *dest, int destlen)
{
	strncpy(dest, gettext(values[(ZoneValue(world) / 4) % 4]), destlen - 1);
	values[destlen - 1] = '\0';

	return (0);
}
