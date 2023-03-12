/*
	File:		Quickdraw.c
	
	Contains:	Source demonstrating how to set dock tiles and badges

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

	Copyright © 1998-2000 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>
#include "Tiler.h"

/*******************************************************************************
**	TileAnimationQuickdraw
*******************************************************************************/
void  TileAnimationQuickdraw(
		Boolean		inEraseBeforeDrawing )
{
	CGrafPtr 	theTilePort;
	Rect		theRect;

	// ***
	//
	// Start accessing the app's dock tile
	//
	// ***
	theTilePort = BeginQDContextForApplicationDockTile();

	if ( theTilePort != NULL )
	{
		// Are we supposed to erase first?
		if ( inEraseBeforeDrawing == true )
		{
			// ***
			//
			// Erase the port
			//
			// ***
			GetPortBounds( theTilePort, &theRect );
			EraseRect( &theRect );
		}

		// Phat lines, baby!
		PenSize( 3, 3 );

		// Face
		ForeColor( yellowColor );
		SetRect( &theRect, 2, 2, 126, 126 );	// L T R B
		PaintArc( &theRect, 0, 360 );
		ForeColor( blackColor );
		FrameArc( &theRect, 0, 360 );

		// Eyes
		SetRect( &theRect, 28, 30, 56, 58 );
		PaintArc( &theRect, 0, 360 );
		SetRect( &theRect, 72, 30, 100, 58 );
		PaintArc( &theRect, 0, 360 );

		// Mouth
		SetRect( &theRect, 15, 5, 113, 103 );
		FrameArc( &theRect, 105, 150 );

		// ***
		//
		// Don't forget to flush!
		//
		// * If you don't flush, the tile does not get redrawn until
		//   the next time the dock needs to update it.  That would suck.
		//
		// ***
		QDFlushPortBuffer( theTilePort, NULL );

		// ***
		//
		// Finished accessing the app's dock tile
		//
		// ***
		EndQDContextForApplicationDockTile( theTilePort );
	}
}
