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
#include <logging.h>
#include <resCompat.h>
#include <UIColor.h>

/*! \brief structure containing information about the minimap */
static struct minimap {
	RectangleType rect; /*!< the location of the minimap on screen */
	RectangleType rout; /*!< the location of the minimap in fractions */

	Int8 showing:1; /*!< is the minimap showing */
	Int8 detailed:1; /*!< is the minimap detailed */
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
minimapIntersect(const RectangleType *other, RectangleType *dest)
{
	RctGetIntersection(&minimap.rect, other, dest);
}


void
minimapPaint(void)
{
	if (!minimap.showing) return;

	WinEraseRectangle(&minimap.rect, 0);
	minimap.rout.topLeft.x = (Coord)(minimap.rect.topLeft.x +
	    (Int32)getMapXPos() * minimap.rect.extent.x / getMapWidth());
	minimap.rout.topLeft.y = (Coord)(minimap.rect.topLeft.y +
	    (Int32)getMapYPos() * minimap.rect.extent.y / getMapHeight());
	WriteLog("Minimap rect = (%d,%d) -> (%d,%d)\n",
	    (int)minimap.rout.topLeft.x, (int)minimap.rout.topLeft.y,
	    (int)minimap.rout.extent.x, (int)minimap.rout.extent.y);
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
