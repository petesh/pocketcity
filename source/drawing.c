/*!
 * \file
 * \brief routines to deal with drawing
 *
 * This module contains all the routines that are used to
 * paint and place items on the display
 */
#include <locking.h>
#include <ui.h>
#include <zakdef.h>
#include <globals.h>
#include <drawing.h>
#include <simulation.h>
#include <distribution.h>

int
InitializeGraphics(void)
{
	return (UIInitializeGraphics());
}

void
CleanupGraphics(void)
{
	UICleanupGraphics();
}

/*!
 * \todo should only do visuals and location.
 */
void
Goto(UInt16 x, UInt16 y, goto_code center)
{
	Int16 nx = (Int16)x;
	Int16 ny = (Int16)y;

	if (center) {
		nx -= (Int16)(getVisibleX() / 2);
		ny -= (Int16)(getVisibleY() / 2);
	}

	if (nx < 0)
		nx = 0;
	if (nx > (Int16)(getMapWidth() - getVisibleX()))
		nx = (Int16)(getMapWidth() - getVisibleX());
	if (ny < 0)
		ny = 0;
	if (ny > (Int16)(getMapHeight() - getVisibleY()))
		ny = (Int16)(getMapHeight() - getVisibleY());
	setMapXPos((Int8)nx);
	setMapYPos((Int8)ny);
	RedrawAllFields();
}

void
RedrawAllFields(void)
{
	addGraphicUpdate(gu_all);
}

void
ScrollDisplay(dirType direction)
{
	int moved = 1;

	switch (direction) {
	case dtUp:
		if (getMapYPos() > 0)
			setMapYPos((Int8)(getMapYPos() - 1));
		else
			moved = 0;
		break;
	case dtRight:
		if (getMapXPos() <=
		    (Int16)(getMapWidth() - 1 - getVisibleX()))
			setMapXPos((Int8)(getMapXPos() + 1));
		else
			moved = 0;
		break;
	case dtDown:
		if (getMapYPos() <=
		    (Int16)(getMapHeight() - 1 - getVisibleY()))
			setMapYPos((Int8)(getMapYPos() + 1));
		else
			moved = 0;
		break;
	case dtLeft:
		if (getMapXPos() > 0)
			setMapXPos((Int8)(getMapXPos() - 1));
		else
			moved = 0;
		break;
	default:
		return;
	}
	if (moved)
		UIScrollDisplay(direction);
}

void
MoveCursor(dirType direction)
{
	UInt16 old_x = getCursorX();
	UInt16 old_y = getCursorY();

	zone_lock(lz_world);
	zone_lock(lz_flags);

	switch (direction) {
	case dtUp:
		if (getCursorY() > 0)
			getCursorY()--;
		if ((Int16)getCursorY() < getMapYPos())
			ScrollDisplay(direction);
		break;
	case dtRight:
		if ((Int16)getCursorX() < (getMapWidth() - 1))
			getCursorX()++;
		if (((Int16)getCursorX() >
		    (Int16)(getMapXPos() + getVisibleX()-1)) &&
			getCursorX() < getMapWidth())
			ScrollDisplay(direction);
		break;
	case dtDown:
		if ((Int16)getCursorY() < (Int16)(getMapHeight() - 1))
			getCursorY()++;
		if ((getCursorY() > getMapYPos() + getVisibleY()-1) &&
			getCursorY() < getMapHeight())
			ScrollDisplay(direction);
		break;
	case dtLeft:
		if (getCursorX() > 0)
			getCursorX()--;
		if ((Int16)getCursorX() < getMapXPos())
			ScrollDisplay(direction);
		break;
	}

	DrawField(old_x, old_y);
	DrawField(getCursorX(), getCursorY());

	zone_unlock(lz_flags);
	zone_unlock(lz_world);
}

void
DrawField(UInt16 xpos, UInt16 ypos)
{
	UIInitDrawing();

	DrawFieldWithoutInit(xpos, ypos);

	UIFinishDrawing();
}

void
DrawCross(UInt16 xpos, UInt16 ypos, UInt16 xsize, UInt16 ysize)
{
	UInt16 tx, ty;
	tx = xpos;
	ty = ypos;
	xpos -= 1;
	UIInitDrawing();

	while (xpos <= tx + xsize) {
		ypos = ty - 1;
		while (ypos <= ty + ysize) {
			if (((ypos == ty - 1) && (xpos == tx - 1)) ||
			    ((ypos == ty - 1) && (xpos == tx + xsize)) ||
			    ((ypos == ty + ysize) && (xpos == tx - 1)) ||
			    ((ypos == ty + ysize) && (xpos == tx + xsize)))
				goto next;
			if ((xpos < getMapWidth()) &&
			    (ypos < getMapHeight())) {
				DrawFieldWithoutInit(xpos, ypos);
			}
next:
			ypos += 1;
		}
		xpos += 1;
	}
	UIFinishDrawing();
}

void
DrawFieldWithoutInit(UInt16 xpos, UInt16 ypos)
{
	UInt16 i;
	selem_t flag;
	welem_t content, special;
	UInt32 worldpos;

	if (xpos >= getMapWidth() ||
	    ypos >= getMapHeight() || UIClipped(xpos, ypos))
		return;

	worldpos = WORLDPOS(xpos, ypos);
	getWorldAndFlag(worldpos, &content, &flag);
	special = GetGraphicNumber(worldpos);

	orWorldFlags(worldpos, PAINTEDBIT);

	UIPaintField(xpos, ypos, special);
	UIPaintMapField(xpos, ypos, special);
	UIPaintMapStatus(xpos, ypos, special, flag);

	if ((flag & POWEREDBIT) == 0 && CarryPower(content)) {
		UIPaintPowerLoss(xpos, ypos);
	}

	if ((flag & WATEREDBIT) == 0 && CarryWater(content)) {
		UIPaintWaterLoss(xpos, ypos);
	}

	if (xpos == getCursorX() && ypos == getCursorY()) {
		UIPaintCursor(getCursorX(), getCursorY());
	}

	/* draw monster */
	for (i = 0; i < NUM_OF_OBJECTS; i++) {
		if ((UInt16)xpos == game.objects[i].x &&
			(UInt16)ypos == game.objects[i].y &&
			game.objects[i].active != 0) {
			UIPaintSpecialObject(xpos, ypos, (Int8)i);
		}
	}
	/* draw extra units */
	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (xpos == game.units[i].x &&
			ypos == game.units[i].y &&
			game.units[i].active != 0) {
			UIPaintSpecialUnit(xpos, ypos, (Int8)i);
		}
	}
}

void
UnpaintWorld(void)
{
	UInt32 i;

	zone_lock(lz_flags);
	for (i = 0; i < MapMul(); i++)
		clearWorldFlags(i, PAINTEDBIT);
	zone_unlock(lz_flags);
}

welem_t
GetGraphicNumber(UInt32 pos)
{
	welem_t retval = 0;

	retval = getWorld(pos);
	if (IsRoad(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	} else if (IsPowerLine(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	} else if (IsWaterPipe(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	} else if (IsRail(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	}
	return (retval);
}

welem_t
GetSpecialGraphicNumber(UInt32 pos)
{
	int a = 0; /* Above me */
	int b = 0; /* To the left of me */
	int c = 0; /* Below me */
	int d = 0; /* to the right of me */
	welem_t nAddMe = 0;
	welem_t elt = 0;
	welem_t wpe = getWorld(pos);

	if (IsRoad(wpe)) {
		if (pos >= getMapWidth()) {
			elt = getWorld(pos - getMapWidth());
			a = IsRoad(elt) || IsRoadBridge(elt) ||
			    IsRoadPower(elt) || IsRoadPipe(elt);
		}
		if (pos < (unsigned long)(MapMul() - getMapWidth()))
			elt = getWorld(pos + getMapWidth());
			c = IsRoad(elt) || IsRoadBridge(elt) ||
			    IsRoadPower(elt) || IsRoadPipe(elt);
		if ((unsigned long)(pos % getMapWidth()) <
		    (unsigned long)(getMapWidth() - 1)) {
			elt = getWorld(pos + 1);
			b = IsRoad(elt) || IsRoadBridge(elt) ||
			    IsRoadPower(elt) || IsRoadPipe(elt);
		}
		if (pos % getMapWidth() > 0) {
			elt = getWorld(pos - 1);
			d = IsRoad(elt) || IsRoadBridge(elt) ||
			    IsRoadPower(elt) || IsRoadPipe(elt);
		}
		nAddMe = Z_ROAD_START;
	} else if (IsWaterPipe(wpe)) {
		if (pos >= getMapWidth())
			a = CarryWater(getWorld(pos - getMapWidth()));
		if (pos < (unsigned long)(MapMul() - getMapWidth()))
			c = CarryWater(getWorld(pos + getMapWidth()));
		if (pos % getMapWidth() < (unsigned long)(getMapWidth() - 1))
			b = CarryWater(getWorld(pos + 1));
		if (pos % getMapWidth() > 0) d = CarryWater(getWorld(pos - 1));
		nAddMe = Z_PIPE_START;
	} else if (IsPowerLine(wpe)) {
		if (pos >= getMapWidth())
			a = CarryPower(getWorld(pos - getMapWidth()));
		if (pos < (unsigned long)(MapMul() - getMapWidth()))
			c = CarryPower(getWorld(pos + getMapWidth()));
		if (pos % getMapWidth() < (unsigned long)(getMapWidth() - 1))
			b = CarryPower(getWorld(pos+1));
		if (pos % getMapWidth() > 0) d = CarryPower(getWorld(pos - 1));
		nAddMe = Z_POWERLINE;
	} else if (IsRail(wpe)) {
		if (pos >= getMapWidth()) {
			elt = getWorld(pos - getMapWidth());
			a = IsRail(elt) || IsRailTunnel(elt) ||
			    IsRailPower(elt) || IsRailPipe(elt);
		}
		if (pos < (unsigned long)(MapMul() - getMapWidth()))
			elt = getWorld(pos + getMapWidth());
			c = IsRail(elt) || IsRailTunnel(elt) ||
			    IsRailPower(elt) || IsRailPipe(elt);
		if ((unsigned long)(pos % getMapWidth()) <
		    (unsigned long)(getMapWidth() - 1)) {
			elt = getWorld(pos + 1);
			b = IsRail(elt) || IsRailTunnel(elt) ||
			    IsRailPower(elt) || IsRailPipe(elt);
		}
		if (pos % getMapWidth() > 0) {
			elt = getWorld(pos - 1);
			d = IsRail(elt) || IsRailTunnel(elt) ||
			    IsRailPower(elt) || IsRailPipe(elt);
		}
		nAddMe = Z_RAIL_START;
	}

	if ((a && b && c && d) == 1)
		return ((UInt8)(10 + nAddMe));
	if ((a && b && d) == 1)
		return ((UInt8)(9 + nAddMe));
	if ((b && c && d) == 1)
		return ((UInt8)(8 + nAddMe));
	if ((a && b && c) == 1)
		return ((UInt8)(7 + nAddMe));
	if ((a && c && d) == 1)
		return ((UInt8)(6 + nAddMe));
	if ((a && b) == 1)
		return ((UInt8)(5 + nAddMe));
	if ((a && d) == 1)
		return ((UInt8)(4 + nAddMe));
	if ((c && d) == 1)
		return ((UInt8)(3 + nAddMe));
	if ((c && b) == 1)
		return ((UInt8)(2 + nAddMe));
	if ((a || c) == 1)
		return ((UInt8)(1 + nAddMe));

	return (nAddMe);
}
