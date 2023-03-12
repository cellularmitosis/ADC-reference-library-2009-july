/*

File: IconListDef.c

Abstract: Custom List DEFinition function for a list of icons with text labels.

Version: 2.0

Change History:
	
	<2.0>	Carbonized
			added accessor functions for the Classic version (see 
			the comment below)
			for the Carbon version we're using DrawThemeTextBox 
			instead of TETextBox
	<1.0>	first release version

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

#include "IconListDef.h"

#if TARGET_API_MAC_OS8		// additional includes for this file under Classic
#if OPAQUE_TOOLBOX_STRUCTS || !ACCESSOR_CALLS_ARE_FUNCTIONS
#error This file needs to access toolbox structures and for their accessor functions to be declared
#endif
#include <Appearance.h>
#include <CFBase.h>		// just so that our CFStringRef doesn't cause a compile error
#include <Quickdraw.h>
#include <QuickdrawText.h>
#include <Script.h>
#include <TextEdit.h>
#endif

enum
{
	kIconWidth = 32,
	kIconHeight = 32,
	kTextBoxHeight = 14,
	kIconTextSpacingV = 2,
	kContentHeight = kIconHeight + kTextBoxHeight + kIconTextSpacingV
};

static void calculateDrawingBounds(const Rect *inCellRect, Rect *outIconRect, 
									Rect *outTextRect);
static void drawIconListCell(ListHandle theList, const Rect *cellRect, 
								IconListCellDataPtr theCellData, Boolean selected);


// --------------------------------------------------------------------------------------
pascal void IconListDef(short message, Boolean selected, Rect *cellRect, Cell theCell, 
						short dataOffset, short dataLen, ListHandle theList)
{
#pragma unused (dataOffset)

	switch(message)
	{
		case lDrawMsg:
		case lHiliteMsg:
				/* Drawing and highlighting are put together because of text smoothing 
				   (anti-aliasing).  Simply using the QuickDraw hilitetransfermode to 
				   paint the cell doesn't work because the text will be smoothed 
				   according to the background color which would still be white.  So 
				   instead we'll need to set the background color according to whether 
				   or not the cell is highlighted, erase the cell, then draw its 
				   contents for both highlight and draw messages.  An alternative would 
				   be, for lHilite messages, to set the background color, erase only the 
				   region around the icon, then draw only the text but messing with all 
				   the Regions would probably end up taking more time so we're just 
				   going to go ahead and erase the entire cell. */
			if (dataLen == sizeof(IconListCellDataRec))		// this should always be true 
			{												// but check it just in case
				IconListCellDataRec cellData;
				
				LGetCell(&cellData, &dataLen, theCell, theList);
				drawIconListCell(theList, cellRect, &cellData, selected);
			}
			break;
	}
}

// --------------------------------------------------------------------------------------
static void calculateDrawingBounds(const Rect *inCellRect, Rect *outIconRect, Rect *outTextRect)
{
	short iconTop, cellCenterH;
	
	iconTop = inCellRect->top + ((inCellRect->bottom - inCellRect->top - kContentHeight) / 2);
	cellCenterH = (inCellRect->left + inCellRect->right) / 2;
	
	SetRect(outIconRect, cellCenterH - (kIconWidth / 2), iconTop, 
			cellCenterH + (kIconWidth / 2), iconTop + kIconHeight);
	
	SetRect(outTextRect, inCellRect->left, iconTop + kIconHeight + kIconTextSpacingV, 
			inCellRect->right, iconTop + kIconHeight + kIconTextSpacingV + kTextBoxHeight);
}

// --------------------------------------------------------------------------------------
static void drawIconListCell(ListHandle theList, const Rect *cellRect, 
								IconListCellDataRec *theCellData, Boolean selected)
{
	GrafPtr savedPort;
	CGrafPtr listPort;
	ThemeDrawingState savedState;
	Boolean active;
	Rect iconRect, textRect;
	short savedFont, savedSize;
	Style savedFace;
	CFStringRef cellName;
	
	GetPort(&savedPort);
	listPort = GetListPort(theList);
	SetPort((GrafPtr)listPort);
	
	GetThemeDrawingState(&savedState);
	
	if (selected)						// we don't need to change the background 
	{									// color if this Cell isn't highlighted
		Pattern whitePattern;
		RGBColor highlightColor;
		
		GetQDGlobalsWhite(&whitePattern);	// set the background pattern so that 
		BackPat(&whitePattern);				// the color is properly set as a solid color
		
		LMGetHiliteRGB(&highlightColor);
		RGBBackColor(&highlightColor);		// set the background to the highlight color
	}
	
	EraseRect(cellRect);
	
	calculateDrawingBounds(cellRect, &iconRect, &textRect);	// get the drawing Rects
	active = GetListActive(theList);
	
		// draw the IconRef using Icon Services
	PlotIconRef(&iconRect, kAlignNone, active ? kTransformNone : kTransformDisabled, 
				kIconServicesNormalUsageFlag, theCellData->icon);
	
#if TARGET_API_MAC_OS8		// draw TextEdit text in Classic
#pragma unused (cellName)
	savedFont = GetPortTextFont(listPort);	// Get/SetThemeDrawingState doesn't save or 
	savedFace = GetPortTextFace(listPort);	// restore these
	savedSize = GetPortTextSize(listPort);
	
	UseThemeFont(kThemeViewsFont, smCurrentScript);
	TETextBox(&theCellData->name[1], theCellData->name[0], &textRect, teCenter);
	
	TextFont(savedFont);
	TextFace(savedFace);
	TextSize(savedSize);
#else						// draw Appearance text in Carbon
#pragma unused (savedFont, savedSize, savedFace)
	cellName = CFStringCreateWithPascalString(kCFAllocatorDefault, theCellData->name, 
												GetApplicationTextEncoding());
	DrawThemeTextBox(cellName, kThemeViewsFont, 
						active ? kThemeStateActive : kThemeStateInactive, true, 
						&textRect, teCenter, NULL);
	CFRelease(cellName);
#endif
	
	SetThemeDrawingState(savedState, true);
	SetPort(savedPort);
} // drawIconListCell


#pragma mark -
/* Glue functions for Classic - we could simply link with CarbonAccessors.o but this 
   would make the 'LDEF' resource needlessly large */

#if TARGET_API_MAC_OS8

// List Accessor Glue
// --------------------------------------------------------------------------------------
pascal CGrafPtr GetListPort(ListHandle list)
{
	CGrafPtr listPort = NULL;
	
	if (list != NULL)
		listPort = (CGrafPtr)((*list)->port);
	
	return listPort;
}

// --------------------------------------------------------------------------------------
pascal Boolean GetListActive(ListHandle list)
{
	Boolean listActive = false;
	
	if (list != NULL)
		listActive = ((*list)->lActive);
	
	return listActive;
}

// Quickdraw Accessor Glue
// --------------------------------------------------------------------------------------
pascal Pattern * GetQDGlobalsWhite(Pattern *white)
{
	if (white != NULL)
	{
			/* We could try to come up with some clever way to get qd and actually use 
			   its white Pattern but that's a lot more trouble than it's worth.  We 
			   could also manually generate a Pattern with all 0's but the easiest thing 
			   to do is get the white pattern out of the system file. */
		GetIndPattern(white, 0, 20);	// 0 is the ID of the 'PAT#' resource in the system 
	}									// file, 20 is the index of the all-white pattern
	
	return white;
}

// --------------------------------------------------------------------------------------
pascal short GetPortTextFont(CGrafPtr port)
{
	short portFont = 0;
	
	if (port != NULL)
		portFont = port->txFont;
	
	return portFont;
}

// --------------------------------------------------------------------------------------
pascal Style GetPortTextFace(CGrafPtr port)
{
	Style portFace = normal;
	
	if (port != NULL)
		portFace = port->txFace;
	
	return portFace;
}

// --------------------------------------------------------------------------------------
pascal short GetPortTextSize(CGrafPtr port)
{
	short portSize = 0;
	
	if (port != NULL)
		portSize = port->txSize;
	
	return portSize;
}

#endif // TARGET_API_MAC_OS8