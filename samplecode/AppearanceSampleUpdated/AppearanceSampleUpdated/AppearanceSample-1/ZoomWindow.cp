/*
    File:		ZoomWindow.cp
    
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

	Copyright © 2000-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "ZoomWindow.h"

#define topLeft(r)	(((Point *) &(r))[0])
#define botRight(r)	(((Point *) &(r))[1])

const Rect		kBounds = { 40, 40, 240, 280 };
const UInt32 	kAttributes = kWindowCloseBoxAttribute
								| kWindowCollapseBoxAttribute
								| kWindowInWindowMenuAttribute;

#define CalcAttributes( s )	( (s) == kVertical ) ? (kAttributes | kWindowVerticalZoomAttribute) : \
							(kAttributes | kWindowHorizontalZoomAttribute)
							
ConstrainedZoomWindow::ConstrainedZoomWindow( ZoomStyle style ) :
	TWindow( kDocumentWindowClass, CalcAttributes( style ), kBounds )
{
	PicHandle thePicture;
	
	fStyle = style;
	fSmall = false;
	
	if ( fStyle == kVertical )
	{
		thePicture = GetPicture( 133 );
		// Need to detach it so multiple zoom windows open at once will work..
		// otherwise we'd release the picture on close while another window was still using it.
		DetachResource( (Handle) thePicture );
		SetWindowPic( GetWindowRef(), thePicture );
		SetTitle( CFSTR( "Vertical Zoom" ) );
	}
	else
	{
		thePicture = GetPicture( 134 );
		// Need to detach it so multiple zoom windows open at once will work..
		// otherwise we'd release the picture on close while another window was still using it.
		DetachResource( (Handle) thePicture );
		SetWindowPic( GetWindowRef(), thePicture );
		SetTitle( CFSTR( "Horizontal Zoom" ) );
	}
	
	RepositionWindow( GetWindowRef(), NULL, kWindowCascadeOnMainScreen );
	ShowWindow( GetWindowRef() );
}

ConstrainedZoomWindow::~ConstrainedZoomWindow()
{
	PicHandle thePicture = GetWindowPic( GetWindowRef() );
	
	//If your application uses a picture stored as a resource, you must release the memory it occupies by calling the 
	//Resource Manager’s ReleaseResource function and set the WindowPic field to null before you close the window.
	if (thePicture)
		ReleaseResource( (Handle) thePicture );
	
	SetWindowPic( GetWindowRef(), nil );	
}
		
Point
ConstrainedZoomWindow::GetIdealSize()
{
	Rect		bounds;
	Point		size;

    ::SetPort( GetPort() );
    ::GetPortBounds( GetPort(), &bounds );

	size.h = bounds.right - bounds.left;
	size.v = bounds.bottom - bounds.top;
	
	if ( fStyle == kHorizontal )
	{
		if ( size.h < (kBounds.right - kBounds.left) )
			size.h = kBounds.right - kBounds.left;
		else
			size.h = 120;
	}
	else
	{
		if ( size.v < (kBounds.bottom - kBounds.top) )
			size.v = kBounds.bottom - kBounds.top;
		else
			size.v = 120;
	}

	return size;
}

