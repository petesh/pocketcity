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

/*!
 * \brief Set up the graphics.
 */
void
SetUpGraphic(void)
{
	UISetUpGraphic();
}

/*!
 * \brief Set the map position to that location.
 *
 * this function also repaints screen.
 * \param x horizontal position
 * \param y vertical position
 * \todo should only do visuals and location.
 */
void
Goto(Int16 x, Int16 y)
{
	Int16 nx = x - (vgame.visible_x / 2);
	Int16 ny = y - (vgame.visible_y / 2);
	if (nx < 0)
		nx = 0;
	if (nx > (getMapWidth() - vgame.visible_x))
		nx = getMapWidth() - vgame.visible_x;
	if (ny < 0)
		ny = 0;
	if (ny > (getMapHeight() - vgame.visible_y))
		ny = getMapHeight() - vgame.visible_y;
	setMapXPos(nx);
	setMapYPos(ny);
	RedrawAllFields();
}

/*!
 * \brief Draw everything on the screen.
 *
 * The Game area, Credits and population.
 *
 * \todo change to painting from a bitmap of the screen?
 */
void
RedrawAllFields(void)
{
	Int16 i, j;

	LockZone(lz_world);
	UIInitDrawing();
	UILockScreen();
	for (i = getMapXPos(); i < vgame.visible_x + getMapXPos(); i++) {
		for (j = getMapYPos();
		    j < vgame.visible_y + getMapYPos();
		    j++) {
			DrawFieldWithoutInit(i, j);
		}
	}

	UIDrawDate();
	UIDrawCredits();
	UIDrawPop();
	UIDrawLoc();
	UIDrawBuildIcon();
	UIDrawSpeed();

	UIUnlockScreen();
	UIFinishDrawing();
	UnlockZone(lz_world);
}

/*!
 * \brief Scroll the map in the direction specified
 * \param direction the direction to scroll map in
 */
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
		if (getMapXPos() <= (getMapWidth() - 1 - vgame.visible_x))
			setMapXPos(getMapXPos() + 1);
		else
			moved = 0;
		break;
	case dtDown:
		if (getMapYPos() <= (getMapHeight() - 1 - vgame.visible_y))
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

/*!
 * \brief Move the cursor in the location specified.
 * \param direction the direction to scroll map in
 */
void
MoveCursor(dirType direction)
{
	int old_x = vgame.cursor_xpos;
	int old_y = vgame.cursor_ypos;

	LockZone(lz_world);

	switch (direction) {
	case dtUp:
		if (vgame.cursor_ypos > 0)
			vgame.cursor_ypos--;
		if (vgame.cursor_ypos < getMapYPos())
			ScrollDisplay(direction);
		break;
	case dtRight:
		if (vgame.cursor_xpos < (getMapWidth() - 1))
			vgame.cursor_xpos++;
		if ((vgame.cursor_xpos > getMapXPos() + vgame.visible_x-1) &&
			vgame.cursor_xpos < getMapWidth())
			ScrollDisplay(direction);
		break;
	case dtDown:
		if (vgame.cursor_ypos < (getMapHeight() - 1))
			vgame.cursor_ypos++;
		if ((vgame.cursor_ypos > getMapYPos() + vgame.visible_y-1) &&
			vgame.cursor_ypos < getMapHeight())
			ScrollDisplay(direction);
		break;
	case dtLeft:
		if (vgame.cursor_xpos > 0)
			vgame.cursor_xpos--;
		if ((vgame.cursor_xpos < getMapXPos()))
			ScrollDisplay(direction);
		break;
	}

	DrawField(old_x, old_y);
	DrawField(vgame.cursor_xpos, vgame.cursor_ypos);

	UnlockZone(lz_world);
}

/*!
 * \brief Draw the field at the specified location
 * \param xpos horizontal position
 * \param ypos vertical position
 */
void
DrawField(Int16 xpos, Int16 ypos)
{
	UIInitDrawing();

	DrawFieldWithoutInit(xpos, ypos);

	UIFinishDrawing();
}


/*!
 * \brief Draw all zones around the point that is being painted.
 * \param xpos horizontal position
 * \param ypos vertical position
 */
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


/*!
 * \brief draw a field without initializing something.
 *
 * ONLY call this function if you make sure to call
 * UIInitDrawing and UIFinishDrawing in the caller
 * Also remember to call (Un)lockWorld (2 functions)
 * \param xpos horizontal position
 * \param ypos vertical position
 */
void
DrawFieldWithoutInit(Int16 xpos, Int16 ypos)
{
	UInt16 i;
	selem_t flag;
	welem_t content, special;
	UInt32 worldpos;
	Int16 mapx = getMapXPos();
	Int16 mapy = getMapYPos();

	if (xpos < mapx ||
		xpos >= mapx + vgame.visible_x ||
		ypos < mapy ||
		ypos >= mapy + vgame.visible_y) {
		return;
	}

	worldpos = WORLDPOS(xpos, ypos);
	getWorldAndFlag(worldpos, &content, &flag);
	special = GetGraphicNumber(worldpos);

	UIDrawField(xpos - mapx, ypos - mapy, special);

	if ((flag & POWEREDBIT) == 0 && CarryPower(content)) {
		UIDrawPowerLoss(xpos - mapx, ypos - mapy);
	}

	if ((flag & WATEREDBIT) == 0 && CarryWater(content)) {
		UIDrawWaterLoss(xpos - mapx, ypos - mapy);
	}

	if (xpos == vgame.cursor_xpos && ypos == vgame.cursor_ypos) {
		UIDrawCursor(vgame.cursor_xpos - mapx,
		    vgame.cursor_ypos - mapy);
	}

	/* draw monster */
	for (i = 0; i < NUM_OF_OBJECTS; i++) {
		if ((UInt16)xpos == game.objects[i].x &&
			(UInt16)ypos == game.objects[i].y &&
			game.objects[i].active != 0) {
			UIDrawSpecialObject(xpos - mapx, ypos - mapy, i);
		}
	}
	/* draw extra units */
	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (xpos == game.units[i].x &&
			ypos == game.units[i].y &&
			game.units[i].active != 0) {
			UIDrawSpecialUnit(xpos - mapx, ypos - mapy, i);
		}
	}
}

/*!
 * \brief Get the graphic to use for the position in question.
 * \param pos index into map array
 * \return the graphic to paint at this location
 */
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

/*
 * \brief Deals with special graphics fields
 * \param pos index into map array
 * \param ntype the type of the node.
 * \return the special graphic number for this place.
 */
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
	} else if (IsPipe(wpe)) {
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

