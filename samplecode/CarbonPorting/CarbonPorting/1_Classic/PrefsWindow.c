/*

File: PrefsWindow.c

Abstract: Handles all Window and List related functions for the Demo.

Version: 1.0

Change History:
	
	<1.0>	first release version
			this draws a list directly into a window and mimics the 
			behavior of a list box control
			it turns out this was actually quite difficult, there is 
			a LOT of background color/pattern changing you have to 
			do to achieve the desired effect

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include "ExamplePrefs.h"
#include "Help.h"
#include "IconListUtilities.h"
#include "MissingConstants.h"	// for the virtual key codes used in HandleKeyDown
#include "PrefsWindow.h"

#include <Appearance.h>
#include <ControlDefinitions.h>
#include <Controls.h>
#include <Processes.h>
#include <Quickdraw.h>

static short gPanelNumber;

static ListHandle createIconList(WindowRef window, Rect listRect);
static void drawFrameAndFocus(ListHandle list, Boolean active, WindowRef window);
static void changePanel(WindowRef window, short newPanel);


// --------------------------------------------------------------------------------------
void OpenPrefsWindow(void)
{	
	OSStatus error;
	SInt16 pixelDepth;
	WindowRef window;
	ControlRef containerControl, embeddedControl;
	short iconListBottom;
	Rect iconListRect;
	Boolean isColorDevice;
	ListHandle iconList;
	
	error = CreateWindowFromResource(rPrefsWindowPlatinum, &window);
	if (error != noErr)
		ExitToShell();
	RepositionWindow(window, NULL, kWindowCascadeOnMainScreen);
			// CreateWindowFromResource doesn't call this for you like GetNewCWindow does
	
	SetPortWindowPort(window);
	
	SetThemeWindowBackground(window, kThemeBrushModelessDialogBackgroundActive, true);
	
	CreateRootControl(window, &containerControl);
	
		/* Get each user pane and embed each preference panel's controls (for the 
		   demonstration there is only some static text identifying each panel number).  
		   We could just as easily have used AutoEmbedControl but why make the system 
		   figure out which control to use as the embedder when we already know? */
	containerControl = GetNewControl(cPlatinumWindowUserPaneVisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 1");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 2");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 3");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 4");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 5");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 6");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 7");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 8");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 9");
	
	containerControl = GetNewControl(cPlatinumWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 8, 
					"Panel 10");
	
	gPanelNumber = 1;
	
	GetWindowDeviceDepthAndColor(window, &pixelDepth, &isColorDevice);	// draw the list with 
	SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);	// a white background
															// get the bottom-most coordinate 
	iconListBottom = kPrefsWindowPlatinumHeight - kPlatinumWindowEdgeSpacing;	// we can use
	iconListBottom -= (kPrefsWindowPlatinumHeight - kPlatinumWindowEdgeSpacing - 
						kPlatinumWindowEdgeSpacing) % 
						kCellHeight;	// then subtract out the partial cell height that 
							// would be drawn on the bottom so that it's not actually drawn
	SetRect(&iconListRect, kPlatinumWindowEdgeSpacing, kPlatinumWindowEdgeSpacing, 
			kPlatinumWindowEdgeSpacing + kListWidth, iconListBottom);
	iconList = createIconList(window, iconListRect);
	SetWindowProperty(window, kAppSignature, kIconListTag, sizeof(ListHandle), &iconList);
	
	CalculateBalloonHelpRects(window);
	
	DisableMenuItem(GetMenuRef(mDemonstration), iPrefsWindow);
	EnableMenuItem(GetMenuRef(mFile), iClose);
	
	ShowWindow(window);
} // OpenPrefsWindow

// --------------------------------------------------------------------------------------
void ClosePrefsWindow(WindowRef prefsWindow)
{
	ListHandle iconList;
	
	HideWindow(prefsWindow);
	DisableMenuItem(GetMenuRef(mFile), iClose);
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	ReleaseIconListIcons(iconList);
	LDispose(iconList);
	DisposeWindow(prefsWindow);
	
	EnableMenuItem(GetMenuRef(mDemonstration), iPrefsWindow);
}

// --------------------------------------------------------------------------------------
void AdjustControls(WindowRef prefsWindow)
{
	Rect contentRect, listViewRect;
	ControlRef rootControl, userPane;
	SInt16 userPaneWidth, userPaneHeight;
	UInt16 panelIndex;
	ListHandle iconList;
	short oldListHeight, newListHeight;
	
	GetWindowBounds(prefsWindow, kWindowContentRgn, &contentRect);
	
	userPaneWidth = (contentRect.right - contentRect.left) - 
					(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + 
					kControlSpacing) - 
					(kSizeBoxWidth + kPlatinumMinimumSpacing);
	userPaneHeight = (contentRect.bottom - contentRect.top) - kPlatinumWindowEdgeSpacing - 
						(kSizeBoxWidth + kPlatinumMinimumSpacing);
	
	GetRootControl(prefsWindow, &rootControl);
	for (panelIndex = 1; panelIndex <= kNumberOfRows; panelIndex++)
	{
		GetIndexedSubControl(rootControl, panelIndex, &userPane);
		SizeControl(userPane, userPaneWidth, userPaneHeight);
	}
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	
	SetRect(&listViewRect, (*iconList)->rView.left, (*iconList)->rView.top, 
			(*iconList)->rView.right, (*iconList)->rView.bottom);
	oldListHeight = listViewRect.bottom - listViewRect.top;
	
	newListHeight = (contentRect.bottom - contentRect.top) - kPlatinumWindowEdgeSpacing - 
					kPlatinumWindowEdgeSpacing;
	newListHeight -= newListHeight % kCellHeight;	// make the list height a multiple 
	if (newListHeight > kMaxListHeight)				// of the cell height and don't make 
		newListHeight = kMaxListHeight;				// it bigger than we have cells for
	
	if (newListHeight != oldListHeight)
	{
		Rect invalRect;
		
		listViewRect.right += kScrollBarWidth;	// we will need to redraw the scroll bar as well
			// we need to invalidate the area where a cell will be drawn or erased
		if (newListHeight > oldListHeight)
			SetRect(&invalRect, listViewRect.left - 5, listViewRect.bottom - 5, 
					listViewRect.right + 5, contentRect.bottom);	// the extra 5 pixels are 
									// to cause the bottom of the list box frame to get erased
		else
			SetRect(&invalRect, listViewRect.left - 5, 
					kPlatinumWindowEdgeSpacing + newListHeight - 5, listViewRect.right + 5, 
					contentRect.bottom);
		
			// the drawing section is as far down as possible to avoid any screen flickering
		DrawThemeFocusRect(&listViewRect, false);			// erase the focus rectangle
		LSize(kListWidth, newListHeight, iconList);			// resize the List
		drawFrameAndFocus(iconList, true, prefsWindow);		// draw the focus rectangle back
		InvalWindowRect(prefsWindow, &invalRect);
	}

	CalculateBalloonHelpRects(prefsWindow);
} // AdjustControls

// --------------------------------------------------------------------------------------
void RedrawPrefsWindowList(WindowRef prefsWindow)
{
	ListHandle iconList;
	ListBounds visibleCells;
	SInt16 pixelDepth;
	Boolean isColorDevice;
	short row;
	Cell theCell;
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	SetRect(&visibleCells, (*iconList)->visible.left, (*iconList)->visible.top, 
			(*iconList)->visible.right, (*iconList)->visible.bottom);
	
	GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
	SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
	
	for (row = visibleCells.top; row < visibleCells.bottom; row++)	// redraw just the 
	{																// visible cells
		SetPt(&theCell, 0, row);
		LDraw(theCell, iconList);
	}
}

#pragma mark -
// --------------------------------------------------------------------------------------
void HandleDrawContent(WindowRef prefsWindow)
{
	CGrafPtr windowPort;
	RgnHandle visibleRegion;
	ListHandle iconList;
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	
	windowPort = GetWindowPort(prefsWindow), 
	visibleRegion = windowPort->visRgn;
	
	if (visibleRegion != NULL)
	{
		Boolean active = IsWindowHilited(prefsWindow);
		SInt16 pixelDepth;
		Boolean isColorDevice;
		
		if (active)
			SetThemeWindowBackground(prefsWindow, kThemeBrushModelessDialogBackgroundActive, 
										false);
		else
			SetThemeWindowBackground(prefsWindow, 
										kThemeBrushModelessDialogBackgroundInactive, false);
		
		EraseRgn(visibleRegion);
		UpdateControls(prefsWindow, visibleRegion);
		
		GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
		LUpdate(visibleRegion, iconList);
		drawFrameAndFocus(iconList, active, prefsWindow);
	}
}

// --------------------------------------------------------------------------------------
void  HandleActivate(WindowRef window, Boolean activate)
{
	ControlRef rootControl;
	ListHandle iconList;
	SInt16 pixelDepth;
	Boolean isColorDevice;
	
	GetRootControl(window, &rootControl);
	GetWindowProperty(window, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	
	SetPortWindowPort(window);
	GetWindowDeviceDepthAndColor(window, &pixelDepth, &isColorDevice);
		
	if (activate)
	{
		SetThemeTextColor(kThemeTextColorModelessDialogActive, pixelDepth, isColorDevice);
		ActivateControl(rootControl);
		
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
		LActivate(true, iconList);
		RedrawPrefsWindowList(window);	// redraw the list with the active appearance
		drawFrameAndFocus(iconList, true, window);
		
		EnableMenuItem(GetMenuRef(mFile), iClose);
	}
	else	// deactivate
	{
		SetThemeTextColor(kThemeTextColorModelessDialogInactive, pixelDepth, isColorDevice);
		DeactivateControl(rootControl);
		
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
		LActivate(false, iconList);
		RedrawPrefsWindowList(window);	// redraw the  list with the inactive appearance
		drawFrameAndFocus(iconList, false, window);
		
		DisableMenuItem(GetMenuRef(mFile), iClose);
	}
}

// --------------------------------------------------------------------------------------
void HandleContentClick(WindowRef window, Point mouseLocation, EventModifiers modifiers)
{
	ListHandle iconList;
	Rect iconListRect;
	Boolean isDoubleClick;
	Cell theCell;
	
	GetWindowProperty(window, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	
	SetRect(&iconListRect, (*iconList)->rView.left, (*iconList)->rView.top, 
			(*iconList)->rView.right, (*iconList)->rView.bottom);
	
	iconListRect.right += kScrollBarWidth;	
	
	SetPortWindowPort(window);
	GlobalToLocal(&mouseLocation);
	
	if (PtInRect(mouseLocation, &iconListRect))
	{
		SInt16 pixelDepth;
		Boolean isColorDevice;
		
		GetWindowDeviceDepthAndColor(window, &pixelDepth, &isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
								// if LClick causes the list selection to change, or the 
		isDoubleClick = LClick(mouseLocation, modifiers, iconList);		// scroll bar 
		SetPt(&theCell, 0, 0);					// to change, the affected cells are 
		LGetSelect(true, &theCell, iconList);	// immediately drawn (no update event)
		
		if ((theCell.v + 1) != gPanelNumber)
			changePanel(window, theCell.v + 1);
	}
}

// --------------------------------------------------------------------------------------
void HandleKeyDown(char keyCode, WindowRef prefsWindow)
{	
		/* Why use the virtual key code instead of the character code?  When the control 
		   key is held down it often masks out bit 7 of the character code, thus making 
		   it impossible to distinguish between some key presses such as page down 
		   (0x0C) and control-L (0x4C & ~bit7 = 0x0C).  The virtual key codes, on the 
		   other hand, are unaffected by modifier keys. */
	if ( (keyCode == kUpArrowKeyCode) || (keyCode == kDownArrowKeyCode) )
	{
		ListHandle iconList;
		short lastRow;
		Cell selectedCell;
		
		GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
							&iconList);
		lastRow = (*iconList)->dataBounds.bottom - (*iconList)->dataBounds.top - 1;
		
		SetPt(&selectedCell, 0, 0);
		LGetSelect(true, &selectedCell, iconList);
		
		if ( (keyCode == kUpArrowKeyCode) && (selectedCell.v > 0) )
		{
			SInt16 pixelDepth;
			Boolean isColorDevice;
			
			GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
			SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
			
			LSetSelect(false, selectedCell, iconList);	// LSetSelect causes the indicated 
			selectedCell.v--;	// cell to be highlighted immediately (no update event)
			LSetSelect(true, selectedCell, iconList);
			
			LAutoScroll(iconList);	// scroll the list in case the selected cell isn't in view
			changePanel(prefsWindow, selectedCell.v + 1);
		}
		else if ( (keyCode == kDownArrowKeyCode) && (selectedCell.v < lastRow) )
		{
			SInt16 pixelDepth;
			Boolean isColorDevice;
			
			GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
			SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
			
			LSetSelect(false, selectedCell, iconList);
			selectedCell.v++;
			LSetSelect(true, selectedCell, iconList);
			
			LAutoScroll(iconList);	// scroll the list in case the selected cell isn't in view
			changePanel(prefsWindow, selectedCell.v + 1);
		}
	}
	else if ( (keyCode == kPageUpKeyCode) || (keyCode == kPageDownKeyCode) )
	{
		ListHandle iconList;
		ListBounds visibleCells;
		SInt16 pixelDepth;
		Boolean isColorDevice;
		
		GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
							&iconList);
		SetRect(&visibleCells, (*iconList)->visible.left, (*iconList)->visible.top, 
				(*iconList)->visible.right, (*iconList)->visible.bottom);
		
		GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
							// LScroll causes the affected cells to be drawn immediately 
		if (keyCode == kPageUpKeyCode)								// (no update event)
			LScroll(0, -(visibleCells.bottom - 1 - visibleCells.top), iconList);
		else	// keyCode == kPageDownKeyCode
			LScroll(0, (visibleCells.bottom - 1 - visibleCells.top), iconList);
	}
} // HandleKeyDown

#pragma mark -
// --------------------------------------------------------------------------------------
static ListHandle createIconList(WindowRef window, Rect listRect)
{
	ListBounds dataBounds;
	Point cellSize;
	ListHandle iconList;
	Cell theCell;
	
	SetRect(&dataBounds, 0, 0, 1, 0);		// initially there are no rows
	SetPt(&cellSize, kListWidth, kCellHeight);
	
	iconList = LNew(&listRect, &dataBounds, cellSize, kIconListLDEF, window, true, false, 
					false, true);
	
	if (iconList != NULL)
	{
		(*iconList)->selFlags = lOnlyOne;
		
		AddRowsAndDataToIconList(iconList, rIconListIconBaseID);
		
		SetPt(&theCell, 0, 0);
		LSetSelect(true, theCell, iconList);	// select the first Cell
		drawFrameAndFocus(iconList, true, window);
	}
	
	return iconList;
}

// --------------------------------------------------------------------------------------
static void drawFrameAndFocus(ListHandle list, Boolean active, WindowRef window)
{
	Rect borderRect;
	
	SetRect(&borderRect, (*list)->rView.left, (*list)->rView.top, (*list)->rView.right, 
			(*list)->rView.bottom);
	borderRect.right += kScrollBarWidth;
	
	if (active)
	{
		SetThemeWindowBackground(window, kThemeBrushModelessDialogBackgroundActive, false);
		DrawThemeListBoxFrame(&borderRect, kThemeStateActive);
		DrawThemeFocusRect(&borderRect, true);
	}
	else
	{
		SetThemeWindowBackground(window, kThemeBrushModelessDialogBackgroundInactive, false);
		DrawThemeFocusRect(&borderRect, false);
		DrawThemeListBoxFrame(&borderRect, kThemeStateInactive);
	}
}

// --------------------------------------------------------------------------------------
static void changePanel(WindowRef window, short newPanel)
{
	ControlRef rootControl, userPane;
	
	GetRootControl(window, &rootControl);
	
	GetIndexedSubControl(rootControl, gPanelNumber, &userPane);
	SetControlVisibility(userPane, false, false);	// hide the currently active panel
	
	GetIndexedSubControl(rootControl, newPanel, &userPane);
	SetControlVisibility(userPane, true, true);		// and show the newly selected panel
	gPanelNumber = newPanel;
}