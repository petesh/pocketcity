/*!
 * \file
 * \brief Implementation of the query form
 *
 * Contains the handler code, as well as the code to translate an identified
 * zone into a string for display.
 */
#include <PalmTypes.h>
#include <Chars.h>
#include <StringMgr.h>
#include <SysUtils.h>

#include <query.h>
#include <palmutils.h>
#include <simcity.h>
#include <locking.h>
#include <logging.h>
#include <simcity_resconsts.h>
#include <zakdef.h>
#include <simulation.h>
#include <globals.h>
#include <mem_compat.h>

static FormPtr querySetup(void) QUERY_SECTION;
static void queryCleanup(void) QUERY_SECTION;
static void zonetoPtr(Char *zonemsg, welem_t tile, UInt16 maxlen) QUERY_SECTION;
static void frmShowID(FormPtr fp, UInt16 id) QUERY_SECTION;
static void frmHideID(FormPtr fp, UInt16 id) QUERY_SECTION;

/*!
 * \brief Handle the query form.
 * takes care of the set-up, cleanup app-button selection and cleanup
 */
Boolean
hQuery(EventPtr event)
{
	Boolean handled = false;

	switch (event->eType) {
	case frmOpenEvent:
		PauseGame();
		WriteLog("query opened\n");
		FrmDrawForm(querySetup());
		handled = true;
		break;
	case frmCloseEvent:
		WriteLog("Query close\n");
		queryCleanup();
		break;
	case keyDownEvent:
		switch (event->data.keyDown.chr) {
		case vchrLaunch:
			FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		}
	case ctlSelectEvent:
		switch (event->data.ctlEnter.controlID) {
		case buttonID_OK:
			FrmGotoForm(formID_pocketCity);
			handled = true;
			break;
		}
	default:
		break;
	}

	return (handled);
}

/*! \brief structure dealing with the zone and the string to print */
static const struct type_zone {
	welem_t	tile_start; /*!< starting tile # */
	welem_t	tile_end; /*!< ending tile # */
	UInt16	zonestring; /*!< string to use for display */
} type_zones[] = {
	{ Z_DIRT, Z_DIRT, si_empty_land },
	{ Z_REALTREE, Z_REALTREE, si_forest },
	{ Z_REALWATER, Z_REALWATER, si_realwater },
	{ Z_FAKETREE, Z_FAKETREE, si_faketree },
	{ Z_FAKEWATER, Z_FAKEWATER, si_fakewater },
	{ Z_PUMP, Z_PUMP, si_pump },
	{ Z_WASTE, Z_WASTE, si_waste },
	{ Z_FIRE1, Z_FIRE3, si_fire },
	{ Z_CRATER, Z_CRATER, si_crater },
	{ Z_PIPE_START, Z_PIPE_END, si_pipe },
	{ Z_POWERLINE_START, Z_POWERLINE_END, si_powerline },
	{ Z_POWERWATER_START, Z_POWERWATER_END, si_powerwater },
	{ Z_COMMERCIAL_SLUM, Z_COMMERCIAL_SLUM, si_commercialslum },
	{ Z_RESIDENTIAL_SLUM, Z_RESIDENTIAL_SLUM, si_residentialslum },
	{ Z_INDUSTRIAL_SLUM, Z_INDUSTRIAL_SLUM, si_industrialslum },
	{ Z_COALPLANT_START, Z_COALPLANT_END, si_coalplant },
	{ Z_NUCLEARPLANT_START, Z_NUCLEARPLANT_END, si_nuclearplant },
	{ Z_FIRESTATION_START, Z_FIRESTATION_END, si_firestation },
	{ Z_POLICEDEPT_START, Z_POLICEDEPT_END, si_policedept },
	{ Z_ARMYBASE_START, Z_ARMYBASE_END, si_armybase },
	{ Z_COMMERCIAL_MIN, Z_COMMERCIAL_MAX, si_commercial },
	{ Z_RESIDENTIAL_MIN, Z_RESIDENTIAL_MAX, si_residential },
	{ Z_INDUSTRIAL_MIN, Z_INDUSTRIAL_MAX, si_industrial },
	{ Z_POWERROAD_START, Z_POWERROAD_END, si_powerroad },
	{ Z_PIPEROAD_START, Z_PIPEROAD_END, si_piperoad },
	{ Z_ROAD_START, Z_ROAD_END, si_road },
	{ Z_BRIDGE_START, Z_BRIDGE_END, si_bridge },
	{ Z_RAIL_START, Z_RAIL_END, si_rail },
	{ Z_RAILPIPE_START, Z_RAILPIPE_END, si_railpipe },
	{ Z_RAILPOWER_START, Z_RAILPOWER_END, si_railpower },
	{ Z_RAILOVROAD_START, Z_RAILOVROAD_END, si_railovroad },
	{ Z_RAILTUNNEL_START, Z_RAILTUNNEL_END, si_railtunnel },
	{ 0, 0, 0 }
};

/*!
 * \brief Convert a zone type into a display string.
 * \param zonemsg message to occupy
 * \param tile the tile in question
 * \param maxlen the maximum length to fill the zone message to
 */
static void
zonetoPtr(Char *zonemsg, welem_t tile, UInt16 maxlen)
{
	struct type_zone *tzone = (struct type_zone *)&type_zones[0];

	while (tzone->zonestring != 0) {
		if ((tile >= tzone->tile_start) && (tile <= tzone->tile_end))
			break;
		tzone++;
	}

	if (tzone->zonestring == 0) {
		StrIToA(zonemsg, tile);
	} else {
		ResGetString(tzone->zonestring, zonemsg, maxlen);
	}
}

/*!
 * \brief show the form id item in question
 * \param fp the form pointer
 * \param id the id of the item to show
 */
static void
frmShowID(FormPtr fp, UInt16 id)
{
	FrmShowObject(fp, FrmGetObjectIndex(fp, id));
}

/*!
 * \brief hide the form item in question
 * \param fp the pointer to the form where the item resides
 * \param id the id of the item on the form
 */
static void
frmHideID(FormPtr fp, UInt16 id)
{
	FrmHideObject(fp, FrmGetObjectIndex(fp, id));
}

/*!
 * \brief Set up the display items for the query form.
 * \return the form pointer
 */
static FormPtr
querySetup(void)
{
	Char *temp;
	FormPtr form;
	ControlPtr ctl;
	FieldPtr fld;
	welem_t element;
	selem_t status;
	Int16 valdens;

	LockZone(lz_world);
	LockZone(lz_flags);
	getWorldAndFlag(GetPositionClicked(), &element, &status);
	UnlockZone(lz_flags);
	UnlockZone(lz_world);

	form = FrmGetActiveForm();
	temp = (Char *)MemPtrNew(255);
	StrPrintF(temp, "(%d,%d)", (int)(GetPositionClicked() % getMapWidth()),
	    (int)(GetPositionClicked() / getMapWidth()));
	ctl = (ControlPtr)GetObjectPtr(form, labelID_zonelocation);
	CtlSetLabel(ctl, temp);

	temp = (Char *)MemPtrNew(255);
	zonetoPtr(temp, element, 255);
	fld = (FieldPtr)GetObjectPtr(form, labelID_zonetype);
	FldSetTextPtr(fld, temp);
	FldRecalculateField(fld, true);

	temp = (Char *)MemPtrNew(255);
	valdens = ZoneValue(element);
	SysStringByIndex(strID_values, (UInt16)(valdens % 4), temp, 255);
	ctl = (ControlPtr)GetObjectPtr(form, labelID_zonevalue);
	CtlSetLabel(ctl, temp);

	temp = (Char *)MemPtrNew(255);
	SysStringByIndex(strID_densities, (UInt16)(valdens / 4), temp, 255);
	ctl = (ControlPtr)GetObjectPtr(form, labelID_zonedensity);
	CtlSetLabel(ctl, temp);

	/* Pollution / Crime NYI */

	frmHideID(form, labelID_ispowered);
	if (CarryPower(element)) {
		if (status & POWEREDBIT)
			frmShowID(form, labelID_ispowered);
		frmShowID(form, labelID_carrypower);
	} else {
		frmHideID(form, labelID_carrypower);
	}
	frmHideID(form, labelID_iswatered);
	if (CarryWater(element)) {
		if (status & WATEREDBIT)
			frmShowID(form, labelID_iswatered);
		frmShowID(form, labelID_carrywater);
	} else {
		frmHideID(form, labelID_carrywater);
	}

	return (form);
}

/*!
 * \brief Tidy up the memory allocated to the form.
 */
static void
queryCleanup(void)
{
	FormPtr form;
	Char *temp;

	form = FrmGetActiveForm();

	temp = (char *)CtlGetLabel((ControlPtr)GetObjectPtr(form,
		    labelID_zonelocation));
	if (temp) MemPtrFree(temp);

	temp = (char *)FldGetTextPtr((FieldPtr)GetObjectPtr(form,
	    labelID_zonetype));
	if (temp) MemPtrFree(temp);

	temp = (char *)CtlGetLabel((ControlPtr)GetObjectPtr(form,
	    labelID_zonevalue));
	if (temp) MemPtrFree(temp);

	temp = (char *)CtlGetLabel((ControlPtr)GetObjectPtr(form,
	    labelID_zonedensity));
	if (temp) MemPtrFree(temp);
}
