/*
	File:		Quartz.c
	
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
**	TileAnimationQuartz
*******************************************************************************/
void  TileAnimationQuartz( Boolean inEraseBeforeDrawing )
{
	CGContextRef 	theContext;
	CGRect			theRect;

	// ***
	//
	// Start accessing the app's dock tile
	//
	// ***
	theContext = BeginCGContextForApplicationDockTile();

	if ( theContext != NULL )
	{
		if ( inEraseBeforeDrawing )
		{
			// ***
			//
			// Clear the context
			//
			// ***
			theRect = CGRectMake( 0, 0, 128, 128 );
			CGContextClearRect( theContext, theRect );
		}

		// Phat lines, baby!
		CGContextSetLineWidth( theContext, 3 );

		// Face
		CGContextBeginPath( theContext );
		CGContextSetRGBFillColor( theContext, 1.0, 0.83, 0.0, 0.60 );	// RGBA
		CGContextAddArc( theContext, 64, 64, 61, 0, 360 * kDegree, true );
		CGContextClosePath( theContext );
		CGContextDrawPath( theContext, kCGPathFillStroke );

		// Eyes
		CGContextBeginPath( theContext );
		CGContextSetRGBFillColor( theContext, 0, 0, 0, 1 );	// RGBA
		CGContextAddArc( theContext, 42, 84, 14, 0, 360 * kDegree, true );
		CGContextAddArc( theContext, 86, 84, 14, 0, 360 * kDegree, true );
		CGContextClosePath( theContext );
		CGContextDrawPath( theContext, kCGPathFill );

		// Mouth
		CGContextBeginPath( theContext );
		CGContextAddArc( theContext, 64, 74, 47, 345 * kDegree, 195 * kDegree, true );
		CGContextDrawPath( theContext, kCGPathStroke );
		CGContextClosePath( theContext );

		// ***
		//
		// Don't forget to flush!
		//
		// * If you don't flush, the tile does not get redrawn until
		//   the next time the dock needs to update it.  That would suck.
		//
		// ***
		CGContextFlush( theContext );

		// ***
		//
		// Finished accessing the app's dock tile
		//
		// ***
		EndCGContextForApplicationDockTile( theContext );
	}
}
