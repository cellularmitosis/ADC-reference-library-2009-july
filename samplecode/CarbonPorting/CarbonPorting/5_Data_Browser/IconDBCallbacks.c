/*

File: IconDBCallbacks.c

Abstract: Data browser control callback functions for a list of icons with text 
          labels.

Version: 5.0

Change History:
	
	<5.0>	changed this file's name from IconListDef.c to 
			IconDBCallbacks.c
			converted everything from a List Definition to a Data 
			Browser draw item callback
			added a Data Browser item notification callback to 
			handle item selection and deselection
	<4.0>	removed all Classic code
	<3.0>	no changes necessary
	<2.0>	Carbonized
			added accessor functions for the Classic version (see 
			the comment below)
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

#include "IconDBCallbacks.h"

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


// --------------------------------------------------------------------------------------
pascal void DrawIconDataBrowserItemCB(ControlRef browser, DataBrowserItemID item, 
										DataBrowserPropertyID property, 
										DataBrowserItemState itemState, const Rect *theRect, 
										SInt16 gdDepth, Boolean colorDevice)
{
#pragma unused (theRect)
	Rect enclosingRect, iconRect, textRect;
	Boolean active;
	IconDBItemDataRec *itemData;
	
		/* The data browser currently gives us the content part bounds in the theRect 
		   parameter but we want the enclosing part bounds to draw in so that we can 
		   draw a fill style highlight. */
	GetDataBrowserItemPartBounds(browser, item, property, kDataBrowserPropertyEnclosingPart, 
									&enclosingRect);
	
	active = IsControlActive(browser);
	
	if ((itemState & kDataBrowserItemIsSelected) != 0)
	{
		ThemeDrawingState savedState;
		
		GetThemeDrawingState(&savedState);
		
		SetThemePen(active ? kThemeBrushPrimaryHighlightColor : kThemeBrushSecondaryHighlightColor, 
					gdDepth, colorDevice);
		PaintRect(&enclosingRect);
		
		SetThemeDrawingState(savedState, true);
	}
	
	calculateDrawingBounds(&enclosingRect, &iconRect, &textRect);	// get the drawing Rects
	itemData = (IconDBItemDataRec *)item;
	
	PlotIconRef(&iconRect, kAlignNone, active ? kTransformNone : kTransformDisabled, 
				kIconServicesNormalUsageFlag, itemData->icon);
	DrawThemeTextBox(itemData->name, kThemeViewsFont, 
						active ? kThemeStateActive : kThemeStateInactive, true, &textRect, 
						teCenter, NULL);
}

// --------------------------------------------------------------------------------------
static void calculateDrawingBounds(const Rect *inItemRect, Rect *outIconRect, Rect *outTextRect)
{
	short iconTop, itemCenterH;
	
	iconTop = inItemRect->top + ((inItemRect->bottom - inItemRect->top - kContentHeight) / 2);
	itemCenterH = (inItemRect->left + inItemRect->right) / 2;
	
	SetRect(outIconRect, itemCenterH - (kIconWidth / 2), iconTop, 
			itemCenterH + (kIconWidth / 2), iconTop + kIconHeight);
	
	SetRect(outTextRect, inItemRect->left, iconTop + kIconHeight + kIconTextSpacingV, 
			inItemRect->right, iconTop + kIconHeight + kIconTextSpacingV + kTextBoxHeight);
}

#pragma mark -
// --------------------------------------------------------------------------------------
pascal void IconDataBrowserItemSelectionCB(ControlRef browser, DataBrowserItemID item, 
											DataBrowserItemNotification message)
{
#pragma unused (browser)
	IconDBItemDataRec *itemData;
	
	itemData = (IconDBItemDataRec *)item;
	
	switch (message)
	{
		case kDataBrowserItemSelected:
			SetControlVisibility(itemData->userPane, true, true);	// this will draw over the 
			break;												// previously selected user pane
		
		case kDataBrowserItemDeselected:
			SetControlVisibility(itemData->userPane, false, false);		// we've already been 
			break;	// drawn over so there's no need to update the display (which would flicker)
	}
}