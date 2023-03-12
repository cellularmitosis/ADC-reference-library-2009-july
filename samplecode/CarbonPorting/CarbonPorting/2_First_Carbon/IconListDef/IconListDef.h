/*

File: IconListDef.h

Abstract: Defines a custom List DEFinition function for a list of icons with text 
          labels

Version: 2.0

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

#if TARGET_API_MAC_CARBON
#include <Carbon.h>
#else
#include <Icons.h>
#include <Lists.h>
#include <MacTypes.h>
#endif


/*
	IconListCellDataRec
	
	Summary:
	  This is the cell data structure required by IconListDef.  It consists of an icon 
	  and a label string.
*/
struct IconListCellDataRec
{
	IconRef icon;
	Str255 name;
};
typedef struct IconListCellDataRec IconListCellDataRec;
typedef IconListCellDataRec *  IconListCellDataPtr;


/*
	IconListDef()
	
	Summary:
	  A list definition function following the ListDefProcPtr specification in Lists.h.  
	  It draws an icon and a text label below it in each cell which needs to be at least 
	  32 pixels wide by 48 pixels high.  Since this function is a callback you don't 
	  need to worry about its parameters.
	
	Discussion:
	  This function requires Mac OS 8.5 or higher.  It draws the icon and text in a 
	  variable width (but at least 32 pixels) 48 pixel-high rectangle which is 
	  vertically centered in the cell and takes the full width of the cell.  The icon is 
	  plotted in a 32x32 pixel rectangle horizontally centered at the top of the draw 
	  rectangle.  The text is drawn centered in a text box at the botton of the draw 
	  rectangle in the Views/List Theme Font with a 2-pixel space between the bottom of 
	  the icon and the top of the text box.  You should handle a change in the list font 
	  (which you can detect by registering a handler for the kAppearanceEventClass 
	  kAEViewsFontChanged Apple Event) by invalidating the list's view rectangle.
	  
	  You need to use the IconListCellDataRec data type defined above for individual 
	  cells' data.
*/
pascal void IconListDef(short message, Boolean selected, Rect *cellRect, Cell theCell, 
						short dataOffset, short dataLen, ListHandle theList);