/*! \file
 * \brief routines to deal with drawing
 *
 * This module contains all the routines that are used to
 * paint and place items on the display
 */
#include <ui.h>
#include <zakdef.h>
#include <globals.h>
#include <drawing.h>
#include <simulation.h>

void
InitGraphic(void)
{
	UIInitGraphic();
}

/*!
 * \todo should only do visuals and location.
 */
void
Goto(Int16 x, Int16 y, goto_code center)
{
	Int16 nx = x;
	Int16 ny = y;

	if (center) {
		nx -= (getVisibleX() / 2);
		ny -= (getVisibleY() / 2);
	}

	if (nx < 0)
		nx = 0;
	if (nx > (getMapWidth() - getVisibleX()))
		nx = getMapWidth() - getVisibleX();
	if (ny < 0)
		ny = 0;
	if (ny > (getMapHeight() - getVisibleY()))
		ny = getMapHeight() - getVisibleY();
	setMapXPos(nx);
	setMapYPos(ny);
	RedrawAllFields();
}

void
RedrawAllFields(void)
{
	UIInitDrawing();
	UILockScreen();

	LockZone(lz_world);
	UIDrawPlayArea();
	UnlockZone(lz_world);

	UIDrawDate();
	UIDrawCredits();
	UIDrawPop();
	UIDrawLoc();
	UIDrawBuildIcon();
	UIDrawSpeed();

	UIUnlockScreen();
	UIFinishDrawing();
}

void
ScrollDisplay(dirType direction)
{
	int moved = 1;

	switch (direction) {
	case dtUp:
		if (getMapYPos() > 0)
			setMapYPos(getMapYPos() - 1);
		else
			moved = 0;
		break;
	case dtRight:
		if (getMapXPos() <= (getMapWidth() - 1 - getVisibleX()))
			setMapXPos(getMapXPos() + 1);
		else
			moved = 0;
		break;
	case dtDown:
		if (getMapYPos() <= (getMapHeight() - 1 - getVisibleY()))
			setMapYPos(getMapYPos() + 1);
		else
			moved = 0;
		break;
	case dtLeft:
		if (getMapXPos() > 0)
			setMapXPos(getMapXPos() - 1);
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
	int old_x = getCursorX();
	int old_y = getCursorY();

	LockZone(lz_world);

	switch (direction) {
	case dtUp:
		if (getCursorY() > 0)
			getCursorY()--;
		if (getCursorY() < getMapYPos())
			ScrollDisplay(direction);
		break;
	case dtRight:
		if (getCursorX() < (getMapWidth() - 1))
			getCursorX()++;
		if ((getCursorX() > getMapXPos() + getVisibleX()-1) &&
			getCursorX() < getMapWidth())
			ScrollDisplay(direction);
		break;
	case dtDown:
		if (getCursorY() < (getMapHeight() - 1))
			getCursorY()++;
		if ((getCursorY() > getMapYPos() + getVisibleY()-1) &&
			getCursorY() < getMapHeight())
			ScrollDisplay(direction);
		break;
	case dtLeft:
		if (getCursorX() > 0)
			getCursorX()--;
		if ((getCursorX() < getMapXPos()))
			ScrollDisplay(direction);
		break;
	}

	DrawField(old_x, old_y);
	DrawField(getCursorX(), getCursorY());

	UnlockZone(lz_world);
}

void
DrawField(Int16 xpos, Int16 ypos)
{
	UIInitDrawing();

	DrawFieldWithoutInit(xpos, ypos);

	UIFinishDrawing();
}

void
DrawCross(Int16 xpos, Int16 ypos, Int16 xsize, Int16 ysize)
{
	Int16 tx, ty;
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
			if ((xpos >= 0) && (ypos >= 0) && \
			    (xpos < getMapWidth()) && (ypos < getMapHeight())) {
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
DrawFieldWithoutInit(Int16 xpos, Int16 ypos)
{
	UInt16 i;
	selem_t flag;
	welem_t content, special;
	UInt32 worldpos;

	if (xpos < 0 || ypos < 0 || xpos >= getMapWidth() ||
	    ypos >= getMapHeight() || UIClipped(xpos, ypos))
		return;

	worldpos = WORLDPOS(xpos, ypos);
	getWorldAndFlag(worldpos, &content, &flag);
	special = GetGraphicNumber(worldpos);

	UIDrawField(xpos, ypos, special);
	UIDrawMapField(xpos, ypos, special);
	UIDrawMapStatus(xpos, ypos, special, flag);

	if ((flag & POWEREDBIT) == 0 && CarryPower(content)) {
		UIDrawPowerLoss(xpos, ypos);
	}

	if ((flag & WATEREDBIT) == 0 && CarryWater(content)) {
		UIDrawWaterLoss(xpos, ypos);
	}

	if (xpos == getCursorX() && ypos == getCursorY()) {
		UIDrawCursor(getCursorX(), getCursorY());
	}

	/* draw monster */
	for (i = 0; i < NUM_OF_OBJECTS; i++) {
		if ((UInt16)xpos == game.objects[i].x &&
			(UInt16)ypos == game.objects[i].y &&
			game.objects[i].active != 0) {
			UIDrawSpecialObject(xpos, ypos, i);
		}
	}
	/* draw extra units */
	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (xpos == game.units[i].x &&
			ypos == game.units[i].y &&
			game.units[i].active != 0) {
			UIDrawSpecialUnit(xpos, ypos, i);
		}
	}
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
	int nAddMe = 0;
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
		return (10 + nAddMe);
	if ((a && b && d) == 1)
		return (9 + nAddMe);
	if ((b && c && d) == 1)
		return (8 + nAddMe);
	if ((a && b && c) == 1)
		return (7 + nAddMe);
	if ((a && c && d) == 1)
		return (6 + nAddMe);
	if ((a && b) == 1)
		return (5 + nAddMe);
	if ((a && d) == 1)
		return (4 + nAddMe);
	if ((c && d) == 1)
		return (3 + nAddMe);
	if ((c && b) == 1)
		return (2 + nAddMe);
	if ((a || c) == 1)
		return (1 + nAddMe);

	return (nAddMe);
}

