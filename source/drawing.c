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
	game.map_xpos = x - (vgame.visible_x / 2);
	game.map_ypos = y - (vgame.visible_y / 2);
	if (game.map_ypos < 0)
		game.map_ypos = 0;
	if (game.map_ypos > (GetMapSize() - vgame.visible_y))
		game.map_ypos = GetMapSize() - vgame.visible_y;
	if (game.map_xpos < 0)
		game.map_xpos = 0;
	if (game.map_xpos > (GetMapSize() - vgame.visible_x))
		game.map_xpos = GetMapSize() - vgame.visible_x;
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
	for (i = game.map_xpos; i < vgame.visible_x + game.map_xpos; i++) {
		for (j = game.map_ypos;
		    j < vgame.visible_y + game.map_ypos; j++) {
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
		if (game.map_ypos > 0) {
			game.map_ypos -= 1;
		} else {
			game.map_ypos = 0;
			return;
		}
		break;
	case dtRight:
		if (game.map_xpos <= (GetMapSize() - 1 - vgame.visible_x)) {
			game.map_xpos += 1;
		} else {
			game.map_xpos = GetMapSize() - vgame.visible_x;
			return;
		}
		break;
	case dtDown:
		if (game.map_ypos <= (GetMapSize() - 1 - vgame.visible_y)) {
			game.map_ypos += 1;
		} else {
			game.map_ypos = GetMapSize() - vgame.visible_y;
			return;
		}
		break;
	case dtLeft:
		if (game.map_xpos > 0) {
			game.map_xpos -= 1;
		} else {
			game.map_xpos = 0;
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
		if (vgame.cursor_ypos < game.map_ypos)
			ScrollMap(direction);
		break;
	case dtRight:
		if (vgame.cursor_xpos < (GetMapSize() - 1))
			vgame.cursor_xpos++;
		if ((vgame.cursor_xpos > game.map_xpos+vgame.visible_x-1) &&
			vgame.cursor_xpos < GetMapSize())
			ScrollMap(direction);
		break;
	case dtDown:
		if (vgame.cursor_ypos < (GetMapSize() - 1))
			vgame.cursor_ypos++;
		if ((vgame.cursor_ypos > game.map_ypos+vgame.visible_y-1) &&
			vgame.cursor_ypos < GetMapSize())
			ScrollMap(direction);
		break;
	case dtLeft:
		if (vgame.cursor_xpos > 0)
			vgame.cursor_xpos--;
		if ((vgame.cursor_xpos < game.map_xpos))
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
 * \brief Draw a cursor cross
 * \param xpos horizontal position
 * \param ypos vertical position
 */
void
DrawCross(Int16 xpos, Int16 ypos)
{
	UIInitDrawing();
	if (xpos > 0)
		DrawFieldWithoutInit(xpos - 1, ypos);
	if (ypos > 0)
		DrawFieldWithoutInit(xpos, ypos - 1);
	if (xpos + 1 < GetMapSize())
		DrawFieldWithoutInit(xpos + 1, ypos);
	if (ypos + 1 < GetMapSize())
		DrawFieldWithoutInit(xpos, ypos + 1);
	DrawFieldWithoutInit(xpos, ypos);
	UIFinishDrawing();
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
	UInt8 flag;
	UInt8 content;

	if (xpos < game.map_xpos ||
		xpos >= game.map_xpos + vgame.visible_x ||
		ypos < game.map_ypos ||
		ypos >= game.map_ypos + vgame.visible_y) {
		return;
	}

	UIDrawField(xpos - game.map_xpos, ypos - game.map_ypos,
		GetGraphicNumber(WORLDPOS(xpos, ypos)));

	flag = GetWorldFlags(WORLDPOS(xpos, ypos));
	content = GetWorld(WORLDPOS(xpos, ypos));

	if ((flag & POWEREDBIT) == 0 && CarryPower(content)) {
		UIDrawPowerLoss(xpos - game.map_xpos, ypos - game.map_ypos);
	}

	if ((flag & WATEREDBIT) == 0 && CarryWater(content)) {
		UIDrawWaterLoss(xpos - game.map_xpos, ypos - game.map_ypos);
	}

	if (xpos == vgame.cursor_xpos && ypos == vgame.cursor_ypos) {
		UIDrawCursor(vgame.cursor_xpos - game.map_xpos,
			vgame.cursor_ypos - game.map_ypos);
	}

	/* draw monster */
	for (i = 0; i < NUM_OF_OBJECTS; i++) {
		if ((UInt16)xpos == game.objects[i].x &&
			(UInt16)ypos == game.objects[i].y &&
			game.objects[i].active != 0) {
			UIDrawSpecialObject(i, xpos - game.map_xpos,
			    ypos - game.map_ypos);
		}
	}
	/* draw extra units */
	for (i = 0; i < NUM_OF_UNITS; i++) {
		if (xpos == game.units[i].x &&
			ypos == game.units[i].y &&
			game.units[i].active != 0) {
			UIDrawSpecialUnit(i, xpos - game.map_xpos,
			    ypos - game.map_ypos);
		}
	}
}

/*!
 * \brief Get the graphic to use for the position in question.
 * \param pos index into map array
 * \return the graphc to paint at this location
 */
static UInt8
GetGraphicNumber(UInt32 pos)
{
	UInt8 retval = 0;

	retval = GetWorld(pos);
	switch (retval) {
	case TYPE_ROAD:	/* special case: roads */
		retval = GetSpecialGraphicNumber(pos, 0);
		break;
	case TYPE_POWER_LINE:	 /* special case: power line */
		retval = GetSpecialGraphicNumber(pos, 1);
		break;
	case TYPE_WATER_PIPE:
		retval = GetSpecialGraphicNumber(pos, 3);
		break;
	case TYPE_BRIDGE:	 /* special case: bridge */
		retval = GetSpecialGraphicNumber(pos, 2);
		break;
	default:
		break;
	}
	return (retval);
}

/*
 * \brief Deals with special graphics fields
 * \param pos index into map array
 * \param ntype the type of the node.
 * \return the special graphic number for this place.
 */
UInt8
GetSpecialGraphicNumber(UInt32 pos, Int16 nType)
{
	/*
	 * type: 0 = road
	 *   1 = power line
	 *   2 = bridge
	 *   3 = water pipe
	 */
	int a = 0, b = 0, c = 0, d = 0;
	int nAddMe = 0;

	switch (nType) {
	case 0: /* roads */
	case 2: /* bridge */
		if (pos >= GetMapSize())
			a = IsRoad(GetWorld(pos - GetMapSize()));
		if (pos < (unsigned long)(MapMul() - GetMapSize()))
			c = IsRoad(GetWorld(pos + GetMapSize()));
		if ((unsigned long)(pos % GetMapSize()) <
		    (unsigned long)(GetMapSize() - 1))
			b = IsRoad(GetWorld(pos + 1));
		if (pos % GetMapSize() > 0) d = IsRoad(GetWorld(pos - 1));
		/* 81 for bridge, 0 for normal road */
		nAddMe = nType == 2 ? TYPE_BRIDGE : 10;
		break;
	case 1:	/* power lines */
		if (pos >= GetMapSize())
			a = CarryPower(GetWorld(pos - GetMapSize()));
		if (pos < (unsigned long)(MapMul() - GetMapSize()))
			c = CarryPower(GetWorld(pos + GetMapSize()));
		if (pos % GetMapSize() < (unsigned long)(GetMapSize() - 1))
			b = CarryPower(GetWorld(pos+1));
		if (pos % GetMapSize() > 0) d = CarryPower(GetWorld(pos - 1));
		nAddMe = 70;
		break;
	case 3: /* water pipe */
		if (pos >= GetMapSize())
			a = CarryWater(GetWorld(pos - GetMapSize()));
		if (pos < (unsigned long)(MapMul() - GetMapSize()))
			c = CarryWater(GetWorld(pos + GetMapSize()));
		if (pos % GetMapSize() < (unsigned long)(GetMapSize() - 1))
			b = CarryWater(GetWorld(pos + 1));
		if (pos % GetMapSize() > 0) d = CarryWater(GetWorld(pos - 1));
		nAddMe = 92;
		break;
	default:
		return (0);
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

/*!
 * \brief Can the node carry power
 * \param x the node entry
 * \return true if this node carries power
 */
Int16
CarryPower(UInt8 x)
{
	return (((x >= ZONE_COMMERCIAL) && (x <= TYPE_POWERROAD_2) &&
	    (x != TYPE_ROAD)) || ((x >= TYPE_FIRE_STATION) &&
	    (x <= TYPE_NUCLEAR_PLANT)) ? 1 : 0);
}

/*!
 * \brief can the node carry power
 * \param x the node entry
 * \return true if this node carries water
 */
Int16
CarryWater(UInt8 x)
{
	return (((x >= ZONE_COMMERCIAL) && (x <= ZONE_INDUSTRIAL)) ||
	    ((x >= TYPE_FIRE_STATION) && (x <= TYPE_NUCLEAR_PLANT)) ||
	    (x == TYPE_WATERROAD_1) || (x == TYPE_WATERROAD_2) ||
	    (x == TYPE_WATER_PIPE) ? 1 : 0);
}

/*!
 * \brief Is this node a power line
 * \param x the node entry to query
 * \return true if it's a power line
 */
Int16
IsPowerLine(UInt8 x)
{
	return (((x >= TYPE_POWER_LINE) && (x < TYPE_WATER_PIPE)) ? 1 : 0);
}

/*!
 * \brief Is this node a road
 * \param x the node entry to query
 * \return true if it's road
 */
Int16
IsRoad(UInt8 x)
{
	return (((x == TYPE_ROAD) || (x == TYPE_POWERROAD_1) ||
	    (x == TYPE_POWERROAD_2) || (x == TYPE_WATERROAD_1) ||
	    (x == TYPE_WATERROAD_2) || (x == TYPE_BRIDGE)) ? 1 : 0);
}

/*!
 * \brief Is this node of the zone type passed in.
 * \param x node item to query
 * \param nType type of node.
 * \return zone value, or zero.
 * XXX: Magic numbers.
 */
Int16
IsZone(UInt8 x, zoneType nType)
{
	if (x == nType) {
		return (1);
	} else if (x >= (nType * 10 + 20) && x <= (nType * 10 + 29))
		return ((x % 10) + 1);
	return (0);
}
