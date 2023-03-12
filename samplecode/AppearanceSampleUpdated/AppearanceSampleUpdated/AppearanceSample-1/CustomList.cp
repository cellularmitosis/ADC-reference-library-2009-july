/*
	File:		CustomList.cp

	Contains:	Custom List class.

    Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright © 1997-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
	#include <CFString.h>
#endif

#include "CustomList.h"

#define width( r )		( (r).right - (r).left )
#define height( r )		( (r).bottom - (r).top )

CustomList::CustomList( ) 
	: TWindow( CFSTR( "AppearanceSample" ), CFSTR( "Custom List" ) )
{				
	OSStatus theError;
	ControlRef theListControl;
	Rect boundsRect = {10, 10, 200, 200};

	this->myListDefSpec.defType = kListDefUserProcType;
	this->myListDefSpec.u.userProc = NewListDefUPP( CustomListRoutine );

	GetWindowBounds( GetWindowRef(), kWindowContentRgn, &boundsRect );
	
	OffsetRect( &boundsRect, -boundsRect.left, -boundsRect.top );

	theError = CreateListBoxControl( GetWindowRef(), &boundsRect, false, 9, 9, true,
					false, 20, 50, true, &(this->myListDefSpec),
					&theListControl);
	
	if (theError == noErr)
	{
		ListHandle theList;
		OSStatus theError;
		
		// Get the ListHandle out of the list control
		theError = GetControlData(theListControl, kControlEntireControl, kControlListBoxListHandleTag, 
				sizeof(ListHandle), &theList, nil);

		this->theListControl = theListControl;
	}

	Show();
		
}

CustomList::~CustomList()
{
	if ( this->theListControl )
	{
		DisposeControl( this->theListControl );
		this->theListControl = nil;
	}

	if (this->myListDefSpec.u.userProc)
	{
		DisposeListDefUPP(this->myListDefSpec.u.userProc);
		this->myListDefSpec.u.userProc = nil;
	}
}

void DisposeList( ListHandle theList )
{
  // Just a placeholder for a routine to dispose of any memory that may have been
  // allocated in a list.
  
  // In this case, there's nothing to deallocate.  As the comment in CFString.h says,
  // we must not deallocate the strings made with CFSTR(), since it does not return
  // a new reference.
}


void InitList( ListHandle theList )
{
	ListBounds myListBounds; 
	
	GetListDataBounds( theList, &myListBounds );
	CFStringRef oldLogoCF[5];
	short i,j,whichColor=0;
	Cell whichCell;
	
	oldLogoCF[0] = CFSTR("green"); 	  
	oldLogoCF[1] = CFSTR("yellow"); 	  
	oldLogoCF[2] = CFSTR("orange"); 	  
	oldLogoCF[3] = CFSTR("red"); 	  
	oldLogoCF[4] = CFSTR("purple"); 	  
	
	for(i=myListBounds.left;i<myListBounds.right;i++)
		for(j=myListBounds.top;j<myListBounds.bottom;j++)
			{
				whichCell.h = i;
				whichCell.v = j;
				LSetCell( &oldLogoCF[whichColor % 5], sizeof(CFStringRef), whichCell, theList );
				whichColor++; // cycle through the colors
			}
}

void ListDraw( Cell lCell, ListHandle lHandle, Rect *lRect, Boolean lSelect, Boolean hiliteIt)
{
	GrafPtr savePort;
	RgnHandle saveClip;
	PenState savePenState;
	short colorIndex;
	CFStringRef cellString;
	short length = sizeof(CFStringRef);

	GetPort(&savePort);
	SetPort(GetListPort(lHandle));
	saveClip = NewRgn();
	require( saveClip != nil, ListDraw_CantCreateClip);
	GetPenState(&savePenState);
	PenNormal();
	EraseRect(lRect);

	// Get the full rect of the cell.  We're being passed in the wrong rectangle for the rightmost column.
	LRect(lRect, lCell, lHandle);
	colorIndex = (lCell.h + lCell.v)&3;
	LGetCell( &cellString, &length, lCell, lHandle );

	// if selected, draw background of cell in user selected hilite color
	if (lSelect)
		{
			PenMode(hilitetransfermode);
			PaintRect(lRect);
		}
	else
		EraseRect(lRect);

	PenNormal();
	
	if (length == sizeof(CFStringRef)) // we got the data out properly
		{
			lRect->left += 1; // Offset the text a bit from the left edge of the box.
			DrawThemeTextBox( cellString, kThemeSystemFont, kThemeStateActive, false, lRect, 
					teJustLeft, nil );
			PenNormal();
		}

	DisposeRgn( saveClip );

	// falls through
ListDraw_CantCreateClip:
	SetPort(savePort);
}



void CustomListRoutine(short lMessage, Boolean lSelect, Rect *lRect, Cell lCell, short lDataOffset, short lDataLen, ListHandle lHandle)
{

	switch( lMessage )
	{
		case lInitMsg: InitList(lHandle);
				break;
		case lCloseMsg: DisposeList(lHandle);
				break;
		case lHiliteMsg:
		case lDrawMsg: ListDraw( lCell, lHandle, lRect, lSelect, lMessage == lHiliteMsg );
				break;
	}
}
