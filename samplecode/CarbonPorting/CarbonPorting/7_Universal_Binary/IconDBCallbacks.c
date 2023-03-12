/*

File: IconDBCallbacks.c

Abstract: Data browser control callback functions for a list of icons with text 
          labels.

Version: 7.0

Change History:
	
	<7.0>	removed all code specific to PEF
			removed all code specific to Mac OS X prior to 10.3
	<6.0>	added support for Quartz 2D/Core Graphics
			split the callbacks into different versions based on 
			what's available in each system version
			added calculateCGDrawingBounds and getClipCGRect for the 
			Mach-O version
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

static void calculateDrawingBounds(CGRect inItemRect, CGRect *outIconRect, 
										CGRect *outTextRect);
static CGRect getClipCGRect(const Rect *portBounds);


// --------------------------------------------------------------------------------------
pascal void DrawIconDataBrowserItem103CB(ControlRef browser, DataBrowserItemID item, 
											DataBrowserPropertyID property, 
											DataBrowserItemState itemState, 
											const Rect *theRect, SInt16 gdDepth, 
											Boolean colorDevice)
{
#pragma unused (theRect)
	Rect enclosingRect, portBounds;
	GrafPtr port;
	CGRect enclosingCGRect, iconCGRect, textCGRect;
	Boolean active;
	ThemeDrawingState savedState = NULL;
	CGContextRef context;
	IconDBItemDataRec *itemData;
	RGBColor labelColor;
	HIThemeTextInfo textInfo;
	
		/* The data browser currently gives us the content part bounds in the theRect 
		   parameter but we want the enclosing part bounds to draw in so that we can 
		   draw a fill style highlight. */
	GetDataBrowserItemPartBounds(browser, item, property, kDataBrowserPropertyEnclosingPart, 
									&enclosingRect);
	
		/* In Mac OS X we're going to use Quartz 2D/Core Graphics for the drawing, so we 
		   need to convert the enclosing part bounds to a CGRect */
	GetPort(&port);		// the data browser sets the port up for us so we just need to get it
	GetPortBounds(port, &portBounds);
	enclosingCGRect = CGRectMake(enclosingRect.left, 
									portBounds.bottom - portBounds.top - 
									enclosingRect.bottom, 
									enclosingRect.right - enclosingRect.left, 
									enclosingRect.bottom - enclosingRect.top);
	calculateDrawingBounds(enclosingCGRect, &iconCGRect, &textCGRect);
	
	active = IsControlActive(browser);
	
	if ((itemState & kDataBrowserItemIsSelected) != 0)
	{
		CGRect clipRect;
		RGBColor foregroundColor;
		
		GetThemeDrawingState(&savedState);
		
		SetThemePen(active ? kThemeBrushPrimaryHighlightColor : kThemeBrushSecondaryHighlightColor, 
					gdDepth, colorDevice);

		clipRect = getClipCGRect(&portBounds);	// call these before beginning the context
		GetForeColor(&foregroundColor);
		
		QDBeginCGContext(port, &context);
		CGContextClipToRect(context, clipRect);
		CGContextSaveGState(context);
		
		CGContextSetRGBFillColor(context, (float)foregroundColor.red / (float)USHRT_MAX, 
									(float)foregroundColor.green / (float)USHRT_MAX, 
									(float)foregroundColor.blue / (float)USHRT_MAX, 1.0);
		CGContextFillRect(context, enclosingCGRect);
		
		CGContextRestoreGState(context);
	}
	else
	{
			/* The data browser will redraw items on the edge of its content view.  
			   Because HIThemeDrawTextBox does not erase the drawing rectangle before 
			   it draws, the text becomes thicker with every draw due to anti-aliasing.  
			   As a workaround, this section erases the text rectangle (if the item was 
			   selected then the enclosing rectangle was erased above). */
		CGRect clipRect;
		RGBColor backgroundColor;
		
		clipRect = getClipCGRect(&portBounds);	// call these before beginning the context
		GetBackColor(&backgroundColor);
		
		QDBeginCGContext(port, &context);
		CGContextClipToRect(context, clipRect);
		CGContextSaveGState(context);
		
		CGContextSetRGBFillColor(context, (float)backgroundColor.red / (float)USHRT_MAX, 
									(float)backgroundColor.green / (float)USHRT_MAX, 
									(float)backgroundColor.blue / (float)USHRT_MAX, 1.0);
		CGContextFillRect(context, textCGRect);
		
		CGContextRestoreGState(context);
	}
	
	itemData = (IconDBItemDataRec *)item;
	
	labelColor.red = 0;
	labelColor.green = 0;
	labelColor.blue = 0;
	PlotIconRefInContext(context, &iconCGRect, kAlignNone, 
							active ? kTransformNone : kTransformDisabled, &labelColor, 
							kPlotIconRefNormalFlags, itemData->icon);
	
	textInfo.version = kHIThemeTextInfoVersionZero;
	textInfo.state = active ? kThemeStateActive : kThemeStateInactive;
	textInfo.fontID = kThemeViewsFont;
	textInfo.horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
	textInfo.verticalFlushness = kHIThemeTextVerticalFlushTop;
	textInfo.options = kHIThemeTextBoxOptionNone;
	textInfo.truncationPosition = kHIThemeTextTruncationNone;
	HIThemeDrawTextBox(itemData->name, &textCGRect, &textInfo, context, 
						kHIThemeOrientationInverted);
	
	QDEndCGContext(port, &context);
	if (savedState != NULL)
		SetThemeDrawingState(savedState, true);
} // DrawIconDataBrowserItem103CB

// --------------------------------------------------------------------------------------
pascal void DrawIconDataBrowserItem104CB(ControlRef browser, DataBrowserItemID item, 
											DataBrowserPropertyID property, 
											DataBrowserItemState itemState, 
											const Rect *theRect, SInt16 gdDepth, 
											Boolean colorDevice)
{
#pragma unused (theRect, gdDepth, colorDevice)
	Rect enclosingRect, portBounds;
	GrafPtr port;
	CGRect enclosingCGRect, iconCGRect, textCGRect;
	Boolean active;
	CGContextRef context;
	IconDBItemDataRec *itemData;
	RGBColor labelColor;
	HIThemeTextInfo textInfo;
	
		/* The data browser currently gives us the content part bounds in the theRect 
		   parameter but we want the enclosing part bounds to draw in so that we can 
		   draw a fill style highlight. */
	GetDataBrowserItemPartBounds(browser, item, property, kDataBrowserPropertyEnclosingPart, 
									&enclosingRect);
	
		/* In Mac OS X we're going to use Quartz 2D/Core Graphics for the drawing, so we 
		   need to convert the enclosing part bounds to a CGRect */
	GetPort(&port);		// the data browser sets the port up for us so we just need to get it
	GetPortBounds(port, &portBounds);
	enclosingCGRect = CGRectMake(enclosingRect.left, 
									portBounds.bottom - portBounds.top - 
									enclosingRect.bottom, 
									enclosingRect.right - enclosingRect.left, 
									enclosingRect.bottom - enclosingRect.top);
	calculateDrawingBounds(enclosingCGRect, &iconCGRect, &textCGRect);
	
	active = IsControlActive(browser);
	
	if ((itemState & kDataBrowserItemIsSelected) != 0)
	{
		CGRect clipRect;
		
		clipRect = getClipCGRect(&portBounds);	// call this before beginning the context
		
		QDBeginCGContext(port, &context);
		CGContextClipToRect(context, clipRect);
		CGContextSaveGState(context);
		
		HIThemeSetFill(active ? kThemeBrushPrimaryHighlightColor : kThemeBrushSecondaryHighlightColor, 
						NULL, context, kHIThemeOrientationInverted);
		CGContextFillRect(context, enclosingCGRect);
		
		CGContextRestoreGState(context);
	}
	else
	{
			/* The data browser will redraw items on the edge of its content view.  
			   Because HIThemeDrawTextBox does not erase the drawing rectangle before 
			   it draws, the text becomes thicker with every draw due to anti-aliasing.  
			   As a workaround, this section erases the text rectangle (if the item was 
			   selected then the enclosing rectangle was erased above). */
		CGRect clipRect;
		RGBColor backgroundColor;
		
		clipRect = getClipCGRect(&portBounds);	// call these before beginning the context
		GetBackColor(&backgroundColor);
		
		QDBeginCGContext(port, &context);
		CGContextClipToRect(context, clipRect);
		CGContextSaveGState(context);
		
		CGContextSetRGBFillColor(context, (float)backgroundColor.red / (float)USHRT_MAX, 
									(float)backgroundColor.green / (float)USHRT_MAX, 
									(float)backgroundColor.blue / (float)USHRT_MAX, 1.0);
		CGContextFillRect(context, textCGRect);
		
		CGContextRestoreGState(context);
	}
	
	itemData = (IconDBItemDataRec *)item;
	
	labelColor.red = 0;
	labelColor.green = 0;
	labelColor.blue = 0;
	PlotIconRefInContext(context, &iconCGRect, kAlignNone, 
							active ? kTransformNone : kTransformDisabled, &labelColor, 
							kPlotIconRefNormalFlags, itemData->icon);
	
	textInfo.version = kHIThemeTextInfoVersionZero;
	textInfo.state = active ? kThemeStateActive : kThemeStateInactive;
	textInfo.fontID = kThemeViewsFont;
	textInfo.horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
	textInfo.verticalFlushness = kHIThemeTextVerticalFlushTop;
	textInfo.options = kHIThemeTextBoxOptionNone;
	textInfo.truncationPosition = kHIThemeTextTruncationNone;
	HIThemeDrawTextBox(itemData->name, &textCGRect, &textInfo, context, 
						kHIThemeOrientationInverted);
	
	QDEndCGContext(port, &context);
} // DrawIconDataBrowserItem104CB

// --------------------------------------------------------------------------------------
static void calculateDrawingBounds(CGRect inItemRect, CGRect *outIconRect, 
										CGRect *outTextRect)
{
	float textBottom, itemCenterH;
	
	textBottom = inItemRect.origin.y + ((inItemRect.size.height - kContentHeight) / 2.0);
	itemCenterH = inItemRect.origin.x + (inItemRect.size.width / 2.0);
	
	*outIconRect = CGRectMake(itemCenterH - (kIconWidth / 2.0), 
								textBottom + kTextBoxHeight + kIconTextSpacingV, kIconWidth, 
								kIconHeight);
	
	*outTextRect = CGRectMake(inItemRect.origin.x, textBottom, inItemRect.size.width, 
								kTextBoxHeight);
}

// --------------------------------------------------------------------------------------
static CGRect getClipCGRect(const Rect *portBounds)
{
	RgnHandle clipRegion;
	Rect clipBounds;
	
	clipRegion = NewRgn();
	GetClip(clipRegion);
	GetRegionBounds(clipRegion, &clipBounds);
	DisposeRgn(clipRegion);
	
	return CGRectMake(clipBounds.left, 
						portBounds->bottom - portBounds->top - clipBounds.bottom, 
						clipBounds.right - clipBounds.left, clipBounds.bottom - clipBounds.top);
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
			HIViewSetVisible(itemData->userPane, true);
			break;
		
		case kDataBrowserItemDeselected:
			HIViewSetVisible(itemData->userPane, false);
			break;
	}
}