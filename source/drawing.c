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

static UInt8 GetGraphicNumber(UInt32 pos);

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
	if (nx > (GetMapWidth() - vgame.visible_x))
		nx = GetMapWidth() - vgame.visible_x;
	if (ny < 0)
		ny = 0;
	if (ny > (GetMapHeight() - vgame.visible_y))
		ny = GetMapHeight() - vgame.visible_y;
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

	LockWorld();
	UIInitDrawing();
	UILockScreen();
	for (i = getMapXPos();
	    i < vgame.visible_x + getMapXPos();
	    i++) {
		for (j = getMapYPos();
		    j < vgame.visible_y + getMapYPos();
		    j++) {
			DrawFieldWithoutInit(i, j);
		}
	}

	UIDrawCredits();
	UIDrawPop();

	UIUnlockScreen();
	UIFinishDrawing();
	UnlockWorld();
}

/*!
 * \brief Scroll the map in the direction specified
 * \param direction the direction to scroll map in
 * \todo off by one error?
 */
void
ScrollMap(dirType direction)
{
	switch (direction) {
	case dtUp:
		if (getMapYPos() > 0) {
			setMapYPos(getMapYPos() - 1);
		} else {
			setMapYPos(0);
			return;
		}
		break;
	case dtRight:
		if (getMapXPos() <= (GetMapWidth() - 1 - vgame.visible_x)) {
			setMapXPos(getMapXPos() + 1);
		} else {
			setMapXPos(GetMapWidth() - vgame.visible_x);
			return;
		}
		break;
	case dtDown:
		if (getMapYPos() <= (GetMapHeight() - 1 - vgame.visible_y)) {
			setMapYPos(getMapYPos() + 1);
		} else {
			setMapYPos(GetMapHeight() - vgame.visible_y);
			return;
		}
		break;
	case dtLeft:
		if (getMapXPos() > 0) {
			setMapXPos(getMapXPos() - 1);
		} else {
			setMapXPos(0);
			return;
		}
		break;
	default:
		return;
	}
	UIScrollMap(direction);
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

	LockWorld();

	switch (direction) {
	case dtUp:
		if (vgame.cursor_ypos > 0)
			vgame.cursor_ypos--;
		if (vgame.cursor_ypos < getMapYPos())
			ScrollMap(direction);
		break;
	case dtRight:
		if (vgame.cursor_xpos < (GetMapWidth() - 1))
			vgame.cursor_xpos++;
		if ((vgame.cursor_xpos > getMapXPos() + vgame.visible_x-1) &&
			vgame.cursor_xpos < GetMapWidth())
			ScrollMap(direction);
		break;
	case dtDown:
		if (vgame.cursor_ypos < (GetMapHeight() - 1))
			vgame.cursor_ypos++;
		if ((vgame.cursor_ypos > getMapYPos() + vgame.visible_y-1) &&
			vgame.cursor_ypos < GetMapHeight())
			ScrollMap(direction);
		break;
	case dtLeft:
		if (vgame.cursor_xpos > 0)
			vgame.cursor_xpos--;
		if ((vgame.cursor_xpos < getMapXPos()))
			ScrollMap(direction);
		break;
	}

	DrawField(old_x, old_y);
	DrawField(vgame.cursor_xpos, vgame.cursor_ypos);

	UnlockWorld();
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
	WriteLog("drawCross(%d, %d, %d, %d)\n", (int)xpos, (int)ypos,
	    (int)xsize, (int)ysize);
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
			    (xpos < GetMapWidth()) && (ypos < GetMapHeight())) {
				WriteLog("elt(%d,%d)\n", xpos, ypos);
				DrawFieldWithoutInit(xpos, ypos);
			}
next:
			ypos += 1;
		}
		xpos += 1;
	}
	UIFinishDrawing();
	WriteLog("\n");
}


/*!
 * \brief draw a field without initializing something.
 *
 * ONLY call this function if you make sure to call
 * UIInitDrawing and UIFinishDrawing in the caller
 * Also remember to call (Un)lockWorld(flags) (4 functions)
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
	flag = GetWorldFlags(worldpos);
	content = GetWorld(worldpos);
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
			UIDrawSpecialObject(i, xpos - mapx, ypos - mapy);
		}
	}
	/* draw extra units */
	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (xpos == game.units[i].x &&
			ypos == game.units[i].y &&
			game.units[i].active != 0) {
			UIDrawSpecialUnit(i, xpos - mapx, ypos - mapy);
		}
	}
}

/*!
 * \brief Get the graphic to use for the position in question.
 * \param pos index into map array
 * \return the graphic to paint at this location
 */
static welem_t
GetGraphicNumber(UInt32 pos)
{
	welem_t retval = 0;

	retval = GetWorld(pos);
	if (IsRoad(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	} else if (IsPowerLine(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	} else if (IsWaterPipe(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	} /* else if (IsBridge(retval)) {
		retval = GetSpecialGraphicNumber(pos);
	} */
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
	welem_t wpe = GetWorld(pos);

	if (IsRoad(wpe)) {
		if (pos >= GetMapWidth()) {
			elt = GetWorld(pos - GetMapWidth());
			a = IsRoad(elt) || IsBridge(elt) || IsRoadPower(elt) ||
			    IsRoadWater(elt);
		}
		if (pos < (unsigned long)(MapMul() - GetMapWidth()))
			elt = GetWorld(pos + GetMapWidth());
			c = IsRoad(elt) || IsBridge(elt) || IsRoadPower(elt) ||
			    IsRoadWater(elt);
		if ((unsigned long)(pos % GetMapWidth()) <
		    (unsigned long)(GetMapWidth() - 1)) {
		    	elt = GetWorld(pos + 1);
			b = IsRoad(elt) || IsBridge(elt) || IsRoadPower(elt) ||
			    IsRoadWater(elt);
		}
		if (pos % GetMapWidth() > 0) {
			elt = GetWorld(pos - 1);
			d = IsRoad(elt) || IsBridge(elt) || IsRoadPower(elt) ||
			    IsRoadWater(elt);
		}
		nAddMe = Z_ROAD_START;
	} else if (IsPipe(wpe)) {
		if (pos >= GetMapWidth())
			a = CarryWater(GetWorld(pos - GetMapWidth()));
		if (pos < (unsigned long)(MapMul() - GetMapWidth()))
			c = CarryWater(GetWorld(pos + GetMapWidth()));
		if (pos % GetMapWidth() < (unsigned long)(GetMapWidth() - 1))
			b = CarryWater(GetWorld(pos + 1));
		if (pos % GetMapWidth() > 0) d = CarryWater(GetWorld(pos - 1));
		nAddMe = Z_PIPE_START;
	} else if (IsPowerLine(wpe)) {
		if (pos >= GetMapWidth())
			a = CarryPower(GetWorld(pos - GetMapWidth()));
		if (pos < (unsigned long)(MapMul() - GetMapWidth()))
			c = CarryPower(GetWorld(pos + GetMapWidth()));
		if (pos % GetMapWidth() < (unsigned long)(GetMapWidth() - 1))
			b = CarryPower(GetWorld(pos+1));
		if (pos % GetMapWidth() > 0) d = CarryPower(GetWorld(pos - 1));
		nAddMe = Z_POWERLINE;
	}
	/* else if (IsBridge(wpe)) {
		if (pos >= GetMapWidth()) {
			elt = GetWorld(pos - GetMapWidth());
			a = IsRoad(elt) || IsBridge(elt);
		}
		if (pos < (unsigned long)(MapMul() - GetMapWidth())) {
			elt = GetWorld(pos + GetMapWidth());
			c = IsRoad(elt) || IsBridge(elt);
		}
		nAddMe = Z_BRIDGE_START;
	} */

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

