/******************************************************************************
 *
 * Copyright (c) 1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DotDotTwo.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <SysEvtMgr.h>
#include <StringMgr.h>
#include "simcity.h"
#include "../source/ui.h"
#include "../source/handler.h"
#include "../source/globals.h"
#include "../source/simulation.h"


MemHandle worldHandle;
MemHandle worldFlagsHandle;
MemPtr worldPtr;
MemPtr worldFlagsPtr;
RectangleType rScrollUp, rScrollDown, rScrollLeft, rScrollRight, rPlayGround;
unsigned char nSelectedBuildItem = 0;

UInt32 timeStamp = 0;
short int simState = 0;
short int DoDrawing = 0;

static Boolean hPocketCity(EventPtr event);
void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth);
void _UICheckScrollBarClick(int x, int y);
void _PalmInit(void);
void _UIGetFieldToBuildOn(int x, int y);
Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags);

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	EventType event;
	short err;
	FormPtr form;
	int formID;
	UInt32 timeTemp;
	UInt16 error;

	error = RomVersionCompatible (0x03503000, launchFlags);
	if (error) return (error);

	if (cmd == sysAppLaunchCmdNormalLaunch)
	{
		_PalmInit();
		ZakMain();
		FrmGotoForm(formID_pocketCity);
		do {
			EvtGetEvent(&event, 1);
			if (SysHandleEvent(&event)) continue;
			if (MenuHandleEvent((void*)0, &event, &err)) continue;

			if (event.eType == frmLoadEvent)
			{
				formID = event.data.frmLoad.formID;
				form = FrmInitForm(formID);
				FrmSetActiveForm(form);
				switch (formID)
				{
				case formID_pocketCity:
					FrmSetEventHandler(form, hPocketCity);
					break;
				}
			}
			else if (event.eType == winExitEvent)
			{
				if (event.data.winExit.exitWindow == (WinHandle) FrmGetFormPtr(formID_pocketCity))
				{
					DoDrawing = 0;
				}
			}
			else if (event.eType == winEnterEvent)
			{
				if (event.data.winEnter.enterWindow == (WinHandle) FrmGetFormPtr(formID_pocketCity) && event.data.winEnter.enterWindow == (WinHandle) FrmGetFirstForm())
				{
					DoDrawing = 1;
				}
			}
			
			FrmDispatchEvent(&event);

			// the almighty homemade >>"multithreader"<<
			if (simState == 0)
			{
				timeTemp = TimGetSeconds();
				if (timeTemp >= timeStamp+SIM_GAME_LOOP_SECONDS)
				{
					simState = 1;
					timeStamp = timeTemp;
				}
			}
			else
			{
				simState = Sim_DoPhase(simState);
				RedrawAllFields();
			}
			

		} while (event.eType != appStopEvent);
	}

	return 0;

}


void _PalmInit(void)
{
	timeStamp = TimGetSeconds();
	rScrollUp.topLeft.x = 149;
	rScrollUp.topLeft.y = 136;
	rScrollUp.extent.x = 9;
	rScrollUp.extent.y = 5;

	rScrollDown.topLeft.x = 149;
	rScrollDown.topLeft.y = 142;
	rScrollDown.extent.x = 9;
	rScrollDown.extent.y = 5;

	rScrollLeft.topLeft.x = 148;
	rScrollLeft.topLeft.y = 150;
	rScrollLeft.extent.x = 5;
	rScrollLeft.extent.y = 9;

	rScrollRight.topLeft.x = 154;
	rScrollRight.topLeft.y = 150;
	rScrollRight.extent.x = 5;
	rScrollRight.extent.y = 9;

	rPlayGround.topLeft.x = 1;
	rPlayGround.topLeft.y = 15;
	rPlayGround.extent.x = 144;
	rPlayGround.extent.y = 144;
}





static Boolean hPocketCity(EventPtr event)
{
	FormPtr form;
	int handled = 0;

	switch (event->eType)
	{
	case frmOpenEvent:
		form = FrmGetActiveForm();
		FrmDrawForm(form);
		DrawGame(1);
		handled = 1;
		break;
	case penDownEvent:
		if (RctPtInRectangle(event->screenX, event->screenY, &rPlayGround))
		{
			_UIGetFieldToBuildOn(event->screenX, event->screenY);
		}
		_UICheckScrollBarClick(event->screenX, event->screenY);
		break;
	case penMoveEvent:
		if (RctPtInRectangle(event->screenX, event->screenY, &rPlayGround))
		{
			_UIGetFieldToBuildOn(event->screenX, event->screenY);
		}
		break;
	case menuEvent:
		if (event->data.menu.itemID >= menuitemID_buildBulldoze)
		{
			nSelectedBuildItem = event->data.menu.itemID - menuitemID_buildBulldoze;
		} else {
			switch (event->data.menu.itemID)
			{
			case menuID_view32:
				UISetTileSize(6);
				break;
			case menuID_view16:
				UISetTileSize(5);
				break;
			}
		}
		break;
	}

	
	return handled;
}

extern unsigned char UIGetSelectedBuildItem(void) { return nSelectedBuildItem; }


void _UICheckScrollBarClick(int x, int y)
{
	if (RctPtInRectangle(x, y, &rScrollUp))	{ ScrollMap(0); }
	else if (RctPtInRectangle(x, y, &rScrollRight))	{ ScrollMap(1); }
	else if (RctPtInRectangle(x, y, &rScrollDown))	{ ScrollMap(2); }
	else if (RctPtInRectangle(x, y, &rScrollLeft))	{ ScrollMap(3); }
}

void _UIGetFieldToBuildOn(int x, int y)
{
	RectangleType rect;
	int i,j;
	rect.extent.x = TILE_SIZE;
	rect.extent.y = TILE_SIZE;

	for (i=0; i<visible_x; i++)
	{
		for (j=0; j<visible_y; j++)
		{
			rect.topLeft.x = 1+i*TILE_SIZE;
			rect.topLeft.y = 15+j*TILE_SIZE;
			if (RctPtInRectangle(x,y,&rect))
			{
				BuildSomething(i+map_xpos,j+map_ypos);
				return;
			}
		}
	}
}





extern int UIDisplayError(int nError)
{
	switch (nError)
	{
	case MSG_ERROR_OUT_OF_MEMORY: FrmAlert(alertID_errorOutOfMemory); break;
	}
}


extern void UIInitDrawing(void) {}
extern void UIFinishDrawing(void) {}

void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth)
{
	// draws a rect on screen: note, the rect within the border will be exactly nHeight*nWidth pxls
	// the frame's left border will be at nTop-1 and so on
	
	RectangleType	rect;

	if (DoDrawing == 0) { return; }

	rect.topLeft.x = nLeft;
	rect.topLeft.y = nTop;
	rect.extent.x = nWidth;
	rect.extent.y = nHeight;

	WinDrawRectangleFrame(1, &rect);

}

extern void UIDrawBorder()
{
	MemHandle bitmaphandle;
	BitmapPtr bitmap;

	if (DoDrawing == 0) { return; }

	bitmaphandle = DmGet1Resource(TBMP, bitmapID_ScrollBars);
	bitmap = MemHandleLock(bitmaphandle);
	WinDrawBitmap(bitmap, 160-12,160-24);
	MemHandleUnlock(bitmaphandle);

	_UIDrawRect(15,1,visible_y*TILE_SIZE,visible_x*TILE_SIZE);
	
}


extern void UISetUpGraphic(void)
{
}

extern void UIDrawCursor(int xpos, int ypos)
{
}

extern void UIDrawPowerLoss(int xpos, int ypos)
{
	MemHandle  bitmaphandle;
	BitmapPtr  bitmap;
	int overlayID = bitmapID_PowerLossOverlay;
	int powerID = bitmapID_PowerLoss;

	if (DoDrawing == 0) { return; }

	WinPushDrawState();


	if (TILE_SIZE == 32) { overlayID = bitmapID_PowerLossOverlay; powerID = bitmapID_PowerLoss; };
	if (TILE_SIZE == 16) { overlayID = bitmapID_PowerLossOverlay2; powerID = bitmapID_PowerLoss2; };
	
	// first draw the overlay
	WinSetDrawMode(winErase);
	bitmaphandle = DmGet1Resource( TBMP, overlayID);
	bitmap = MemHandleLock(bitmaphandle);
	WinPaintBitmap(bitmap, xpos*TILE_SIZE+1, ypos*TILE_SIZE+15);
	MemHandleUnlock(bitmaphandle);
	
	// now draw the 's'
	WinSetDrawMode(winOverlay);
	bitmaphandle = DmGet1Resource( TBMP , powerID);
	bitmap = MemHandleLock(bitmaphandle);
	WinPaintBitmap(bitmap, xpos*TILE_SIZE+1, ypos*TILE_SIZE+15);
	MemHandleUnlock(bitmaphandle);

	
	WinPopDrawState();
}


extern void UIDrawField(int xpos, int ypos, unsigned char nGraphic)
{
	MemHandle  bitmaphandle;
	BitmapPtr  bitmap;
	int startID = 0;

	if (DoDrawing == 0) { return; }

	if (TILE_SIZE == 32) { startID = bitmapID_DirtBmp; };
	if (TILE_SIZE == 16) { startID = bitmapID_DirtBmp2; };

	bitmaphandle = DmGet1Resource( TBMP , nGraphic + startID);
	bitmap = MemHandleLock(bitmaphandle);
	WinDrawBitmap(bitmap, xpos*TILE_SIZE+1, ypos*TILE_SIZE+15);
	MemHandleUnlock(bitmaphandle);

}

extern unsigned long GetRandomNumber(unsigned long max)
{
	if (max == 0) { return 0; };
	return (UInt16)SysRandom(0) % (UInt16)max;
}



extern void UIDrawCredits(void)
{
	char temp[23];
	char c[20];
	RectangleType rect;

	if (DoDrawing == 0) { return; }

	StrCopy(temp, "$: ");
	LongToString(credits,(char*)c);
	StrCat(temp, c);


	rect.topLeft.x = 66;
	rect.topLeft.y = 1;
	rect.extent.x = 94;
	rect.extent.y = 11;
	
	WinEraseRectangle(&rect,0);

	WinDrawChars((char*)temp,StrLen(temp),66,1);
	GetDate((char*)temp);
	WinDrawChars((char*)temp,StrLen(temp),120,1);
}




extern void UISetTileSize(int size)
{
	/*
	1 = 1x1		128
	2 = 2x2		64
	3 = 4x4		32
	4 = 8x8		16
	5 = 16x16	8
	6 = 32x32	4
	*/
	if (!(size >= 1 && size <= 6)) { return; }

	visible_x = 1<<(8-size);
	visible_y = 1<<(8-size);
	TILE_SIZE = 1<<(size-1);
	RedrawAllFields();
}









/*** memory handlers ***/
extern int InitWorld(void)
{
	worldHandle = MemHandleNew(10);
	worldFlagsHandle = MemHandleNew(10);

	if (worldHandle == 0 || worldFlagsHandle == 0)
	{
		UIDisplayError(0);
		return 0;
	}
	return 1;
}


extern int ResizeWorld(long unsigned size)
{
	char * pWorld;
	int i;
	
	if (MemHandleResize(worldHandle, size) != 0 || MemHandleResize(worldFlagsHandle, size) != 0)
	{
		UIDisplayError(0);
		//QuitGameError();
		return 0;
	}

	LockWorld();
	LockWorldFlags();
	
	for (i=0; i<size; i++) { *(((unsigned char*)worldPtr)+i) = 0; }
	for (i=0; i<size; i++) { *(((unsigned char*)worldFlagsPtr)+i) = 0; }

	UnlockWorld();
	UnlockWorldFlags();

	return 1;
}


extern	void LockWorld() { worldPtr = MemHandleLock(worldHandle); }
extern	void UnlockWorld() { MemHandleUnlock(worldHandle); }
extern void LockWorldFlags() { worldFlagsPtr = MemHandleLock(worldFlagsHandle); }
extern void UnlockWorldFlags() { MemHandleUnlock(worldFlagsHandle); }



extern unsigned char GetWorldFlags(long unsigned int pos)
{
	// NOTE: LockWorld() MUST have been called before this is used!!!
	if (pos > (mapsize*mapsize)) { return 0; }
	return ((unsigned char*)worldFlagsPtr)[pos];
}

extern void SetWorldFlags(long unsigned int pos, unsigned char value)
{
	if (pos > mapsize*mapsize) { return; }
	((unsigned char*)worldFlagsPtr)[pos] = value;
}


extern unsigned char GetWorld(unsigned long pos)
{
	// NOTE: LockWorld() MUST have been called before this is used!!!
	if (pos > (mapsize*mapsize)) { return 0; }
	return ((unsigned char*)worldPtr)[pos];
}

extern void SetWorld(unsigned long pos, unsigned char value)
{
	if (pos > mapsize*mapsize) { return; }
	((unsigned char*)worldPtr)[pos] = value;
}


/*** end ***/














Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;
	char temp[20];

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	LongToString(romVersion,(char*)temp);
	WinDrawChars((char*)temp,10,0,0);

	if (romVersion < requiredVersion)
	{
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
		{
			FrmAlert (alertID_RomIncompatible);
		
			// Pilot 1.0 will continuously relaunch this app unless we switch to 
			// another safe one.
			if (romVersion < 0x02000000)
			{
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
			}
		}
		
		return (sysErrRomIncompatible);
	}

	return (0);
}




