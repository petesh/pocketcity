/*!
 * \file
 * This file contains the code for the on-screen minimap which is used to
 * scroll around the real world
 */

#include <minimap.h>
#include <zakdef.h>
#include <globals.h>
#include <simcity.h>
#include <ui.h>
#include <resCompat.h>
#include <UIColor.h>

static struct minimap {
	RectangleType rect;
	RectangleType rout;

	Int8 showing:1;
	Int8 detailed:1;
} minimap;

Int8
minimapGetShowing(void)
{
	return (minimap.showing);
}

void
minimapSetShowing(Int8 show)
{
	minimap.showing = show ? 1 : 0;
}

Int8
minimapGetDetailed(void)
{
	return (minimap.detailed);
}

void
minimapSetDetailed(Int8 detailed)
{
	minimap.detailed = detailed ? 1 : 0;
}

void
minimapPlace(RectangleType *pos)
{
	MemMove(&minimap.rect, pos, sizeof (RectangleType));

	if (minimap.rect.extent.x != 0 && minimap.rect.extent.y != 0) {
		minimap.rout.extent.x = (Coord)((Int32)minimap.rect.extent.x *
		    getVisibleX() / getMapWidth());
		minimap.rout.extent.y = (Coord)((Int32)minimap.rect.extent.y *
		    getVisibleY() / getMapHeight());
	}
}

void
minimapPaint(void)
{
	if (!minimap.showing) return;

	WinEraseRectangle(&minimap.rect, 0);
	minimap.rout.topLeft.x = minimap.rect.topLeft.x +
	    (Int32)getMapXPos() * minimap.rect.extent.x / getMapWidth();
	minimap.rout.topLeft.y = minimap.rect.topLeft.y +
	    (Int32)getMapYPos() * minimap.rect.extent.y / getMapHeight();
	WriteLog("Minimap rect = (%d,%d) -> (%d,%d)\n", minimap.rout.topLeft.x,
	    minimap.rout.topLeft.y, minimap.rout.extent.x,
	    minimap.rout.extent.y);
	WinDrawRectangle(&minimap.rout, 0);
}

Int8
minimapIsTapped(PointType *point, PointType *percentage)
{
	if (!minimap.showing) return (0);
	if (RctPtInRectangle(point->x, point->y, &minimap.rect)) {
		percentage->x = (Coord)(((Int32)point->x -
		    minimap.rect.topLeft.x) * 100 /
		    minimap.rect.extent.x);
		percentage->y = (Coord)(((Int32)point->y -
		    minimap.rect.topLeft.y) * 100 /
		    minimap.rect.extent.y);
		return (1);
	}
	return (0);
}
