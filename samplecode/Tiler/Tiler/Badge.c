/*
	File:		Badge.c
	
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
**	TileBadge
*******************************************************************************/
void TileBadge( Boolean inRestoreTileBeforeBadging )
{
	OSStatus	theErr;
	PicHandle	theBadge;
	PicHandle	theBadgeMask;
	GWorldPtr	theBadgeWorld;
	GWorldPtr	theBadgeMaskWorld;
	Rect		theRect = { 0, 0, 128, 128 };
	CGImageRef	theBadgeImage;
	GDHandle	theSavedDevice;
	GrafPtr		theSavedPort;

	// ***
	//
	// PITFALL!
	//
	// Rebadging the tile will composite over the old one!
	// You might want to restore the tile before badging.
	//
	// ***
	if ( inRestoreTileBeforeBadging )
		RestoreApplicationDockTileImage();

	// Load the pictures
	theBadge = GetPicture( kBadge );
	require( theBadge != NULL, CantLoadBadge );
	theBadgeMask = GetPicture( kBadgeMask );
	require( theBadgeMask != NULL, CantLoadBadgeMask );

	// Make some GWorlds
	theErr = NewGWorld( &theBadgeWorld, 32, &theRect, NULL, NULL, 0 );
	require_noerr( theErr, CantMakeBadgeWorld );
	theErr = NewGWorld( &theBadgeMaskWorld, 32, &theRect, NULL, NULL, 0 );
	require_noerr( theErr, CantMakeBadgeMaskWorld );

	// Draw the pictures into the GWorlds
	GetGWorld( &theSavedPort, &theSavedDevice );
	SetGWorld( theBadgeWorld, NULL );
	DrawPicture( theBadge, &theRect );
	SetGWorld( theBadgeMaskWorld, NULL );
	DrawPicture( theBadgeMask, &theRect );
	SetGWorld( theSavedPort, theSavedDevice );

	// ***
	//
	// Make a CGImage from the GWorlds' pixmaps
	//
	// ***
	theErr = CreateCGImageFromPixMaps( GetGWorldPixMap( theBadgeWorld ),
		GetGWorldPixMap( theBadgeMaskWorld ), &theBadgeImage );
	if ( theErr != noErr )
		SysBeep( 0 );
	require_noerr( theErr, CantMakeBadgeImage );

	// ***
	//
	// Badge the tile
	//
	// ***
	theErr = OverlayApplicationDockTileImage( theBadgeImage );

	if ( theBadgeImage != NULL )
		CGImageRelease( theBadgeImage );

CantMakeBadgeImage:
	DisposeGWorld( theBadgeMaskWorld );

CantMakeBadgeMaskWorld:
	DisposeGWorld( theBadgeWorld );

CantMakeBadgeWorld:
	ReleaseResource( (Handle) theBadgeMask );

CantLoadBadgeMask:
	ReleaseResource( (Handle) theBadge );

CantLoadBadge:
	;
}
