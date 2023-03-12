/*

File: IconDBCallbacks.h

Abstract: Defines data browser control callback functions for a list of icons with 
          text labels

Version: 7.0

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

#include <Carbon/Carbon.h>


/*
	IconDBItemDataRec
	
	Summary:
	  This is the item data structure required by the Icon Data Browser.  Set the Data 
	  Browser item ID to a pointer to this structure.  The icon and name fields are used 
	  by the draw item callback and the userPane field is used by the item notification 
	  callback.
*/
struct IconDBItemDataRec
{
	IconRef icon;
	CFStringRef name;
	HIViewRef userPane;
};
typedef struct IconDBItemDataRec IconDBItemDataRec;


/*
	DrawIconDataBrowserItem103CB()
	DrawIconDataBrowserItem104CB()
	
	Summary:
	  Data browser draw item callback functions following the DataBrowserDrawItemProcPtr 
	  specification in ControlDefinitions.h.  They draw an icon and a text label below 
	  it in each item which needs to be at least 32 pixels wide by 48 pixels high.  
	  Since these functions are callbacks you don't need to worry about their parameters.
	
	Discussion:
	  The number in the function name indicates the minimum OS version required: 10.3 
	  and 10.4.  Both functions are forward compatible (though not optimized) with later 
	  versions of Mac OS.  These functions draw the icon and text in a variable width 
	  (but at least 32 pixels) 48 pixel-high rectangle which is vertically centered in 
	  the item and takes the full width of the item.  The icon is plotted in a 32x32 
	  pixel rectangle horizontally centered at the top of the draw rectangle.  The text 
	  is drawn centered in a text box at the botton of the draw rectangle in the 
	  Views/List Theme Font with a 2-pixel space between the bottom of the icon and the 
	  top of the text box.  You should handle a change in the list font (which you can 
	  detect by registering a handler for the kAppearanceEventClass kAEViewsFontChanged 
	  Apple Event) by invalidating the data browser control's bounding rectangle.
	  
	  The data browser control should use the fill highlight style (as opposed to 
	  minimal), list view, and a plain background.  You need to use a pointer to the  
	  IconDBItemDataRec data type defined above when you set the items' item IDs.
*/
pascal void DrawIconDataBrowserItem103CB(ControlRef browser, DataBrowserItemID item, 
											DataBrowserPropertyID property, 
											DataBrowserItemState itemState, 
											const Rect *theRect, SInt16 gdDepth, 
											Boolean colorDevice);
pascal void DrawIconDataBrowserItem104CB(ControlRef browser, DataBrowserItemID item, 
											DataBrowserPropertyID property, 
											DataBrowserItemState itemState, 
											const Rect *theRect, SInt16 gdDepth, 
											Boolean colorDevice);

/*
	IconDataBrowserItemSelectionCB()
	IconDataBrowserItemSelectionCompCB
	
	Summary:
	  Data browser item notification callback functions following the 
	  DataBrowserItemNotificationProcPtr specification in ControlDefinitions.h.  They 
	  hide the previously selected user pane and show the currently selected user pane.
	
	Discussion:
	  This function hides the previously selected user pane on a deselect message and 
	  shows the currently selected user pane on the subsequent select message.  The data 
	  browser control should have the select only one and and never empty selection set 
	  selection flags set.  You need to use a pointer to the IconDBItemDataRec data type 
	  defined above when you set the items' item IDs.
	  
	  If you provide your own item notification callback you should call this function 
	  from your callback when you receive a deselect or select message.
*/
pascal void IconDataBrowserItemSelectionCB(ControlRef browser, DataBrowserItemID item, 
											DataBrowserItemNotification message);