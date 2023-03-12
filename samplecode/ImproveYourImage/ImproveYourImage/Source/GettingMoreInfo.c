/*
	File:		GetingMoreInfo.c
	
	Description: This example demonstrates how to extract image information using Graphics Importers.
	             Note: This application uses standard output (and SIOUX on the Mac with CodeWarriror)

	Author:		QuickTime Engineering, DTS

	Copyright: 	© Copyright 1999 - 2002 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <2> 4/22/02 released as dts sample code
										<1> 4/1/99 initial release

*/

#include "MacShell.h"

#ifndef __APPLE_CC__
    #include <SIOUX.h>
    #include <Console.h>
    #include <stdio.h>
#endif

void GetMoreInfo( void )
{  	
	OSErr  err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	GraphicsImportComponent importer = 0;
	Str31 name;
	OSType udType;
	short count, i;
	Handle h = NULL;
	Ptr p;
	char nul = 0;
	ImageDescriptionHandle desc = NULL;
	CTabHandle colorTable = NULL;
	UserData userData = 0;
	MatrixRecord defaultMatrix;
	RGBColor defaultOpColor;
	Rect defaultSourceRect;
	Handle colorSyncProfile = NULL;
	RgnHandle defaultClip = NULL;
	long defaultGraphicsMode;
	short drawsAllPixels;
	
#ifndef __MACH__
	SIOUXSettings.standalone = false;
   	SIOUXSettings.initializeTB = false;
   	SIOUXSettings.setupmenus = false;
   	SIOUXSettings.autocloseonquit = true;
   	SIOUXSettings.asktosaveonclose = false;
   	SIOUXSetTitle( "\pGet More Info" );
#endif
   	
	printf( "Choose an image file...\n\n" );

	BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes );
	HLock( hOpenTypeList );
	
	// prompt for an image file.
	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	DisposeHandle( hOpenTypeList );
	if (err) return;
	
	// locate and open a graphics importer component
	err = GetGraphicsImporterForFile( &theFSSpec, &importer );

	// get the file's image description.
	err = GraphicsImportGetImageDescription( importer,	// importer instance
											 &desc );	// ptr to ImageDescriptionHandle
	if( desc && *desc ) {
		// print basic statistics from the image description.
		printf( "Image width:   %d\n", (*desc)->width );
		printf( "Image height:  %d\n", (*desc)->height );
		printf( "Depth:         %d\n", (*desc)->depth );
		BlockMoveData( (*desc)->name, name, sizeof(name) );
		CopyPascalStringToC(name, (char *)name);
		printf( "Format:        %s\n", name);
		printf( "Resolution:    %.1f x %.1f dpi\n", Fix2X((*desc)->hRes), Fix2X((*desc)->vRes) );
		
		// an image description may contain a CLUT (Color Look Up Table)
		if( ((*desc)->depth < 16) || ((*desc)->depth > 32) ) {
			// get the CTable from the image description
			err = GetImageDescriptionCTable( desc,			// importer instance
											 &colorTable );	// ptr to CTabHandle
			if( colorTable ) {
				printf( "\nImage has a color table.\n" );
				DisposeCTable( colorTable );
			}
		}
	}
		
	// NOTE: GraphicsImportGetNaturalBounds() is just shorthand that builds a 
	// rectangle from the image description's width and height.
	
	// get and print out the file's metadata
	
	// create a new user data structure  
	err = NewUserData( &userData );
	
	// extract metadata from an image and add it to an already alocated user data structure
	err = GraphicsImportGetMetaData( importer,		// importer instance
									 userData );	// user data structure
	
	h = NewHandle(0);
	
	//  retrieve the first user data type from the user data list
	udType = GetNextUserDataType( userData,	// user data list
								  0 );		// user data type, 0 to retrieve first user data type
	if( 0 != udType ) {
		printf( "\nMeta-data:\n" );
		do {			
			// determine the number of items of a given type in a user data list
			count = CountUserDataType( userData, udType );
			for( i = 1; i <= count; i++ ) {
				// if the first letter of udType is 0xA9, the copyright symbol, 
				// then use GetUserDataText instead of GetUserData
				// there's a list of interesting user data types in <Movies.h>
				if( (udType>>24) == 0xA9 ) {
					// retrieve language-tagged text from an item
					err = GetUserDataText( userData,		// user data list
										   h,				// handle to recieve the data
										   udType,			// user item's type value
										   i,				// item's index value
										   langEnglish );	// language code of text to be retrieved	
					// nul-terminate the string in the handle.
					PtrAndHand( &nul, h, 1 );
					// turn any CRs into spaces (to work around SIOUX behavior).
					p = *h; while( *p ) { if( *p == 13 ) *p = ' '; p++; };
					HLock( h );
					printf( "  %c%c%c%c: %s\n", (char)(udType>>24), (char)(udType>>16), 
							(char)(udType>>8), (char)udType, *h );
					HUnlock( h );
				}
				else {
					// get a specified user data item
					err = GetUserData( userData,	// user data list
									   h,			// handle to recieve the data
									   udType,		// user item's type value
									   i );			// item's index value
					printf( "  %c%c%c%c: [%ld bytes]\n", (char)(udType>>24), (char)(udType>>16), 
							(char)(udType>>8), (char)udType, GetHandleSize(h) );
				}
			}
			//  retrieve the next user data type from the user data list
			udType = GetNextUserDataType( userData, udType );
		} while( 0 != udType );
	}
	DisposeUserData( userData );
	DisposeHandle( h );
	printf( "\n" );
	
	// print out some more esoteric properties
	err = GraphicsImportGetDefaultMatrix( importer, &defaultMatrix );
	if( noErr == err )
		printf( "Image has default matrix (matrix type %d).\n", GetMatrixType( &defaultMatrix ) );

	err = GraphicsImportGetDefaultClip( importer, &defaultClip );
	if( noErr == err )
		printf( "Image has default clip.\n" );
	
	err = GraphicsImportGetDefaultGraphicsMode( importer, &defaultGraphicsMode, &defaultOpColor );
	if( noErr == err )
		printf( "Image has default graphics mode %ld.\n", defaultGraphicsMode );

	err = GraphicsImportGetDefaultSourceRect( importer, &defaultSourceRect );
	if( noErr == err )
		printf( "Image has default rectangle (%d,%d,%d,%d).\n", 
			     defaultSourceRect.left, defaultSourceRect.top, 
			     defaultSourceRect.right, defaultSourceRect.bottom );

    // If we call GraphicsImportGetColorSyncProfile for an image that has a ColorSync profile,a generic profile
    // will be returned to avoid incorrect matching (this is because the base graphics importer will perform ColorSync
    // matching). So, to opt out of automatic ColorSync matching and access actual embedded profiles, we need to set the
    // kGraphicsImporterDontUseColorMatching flag.
    GraphicsImportSetFlags(importer, kGraphicsImporterDontUseColorMatching);
	err = GraphicsImportGetColorSyncProfile( importer, &colorSyncProfile );
	if( ( noErr == err ) && ( NULL != colorSyncProfile ) )
		printf( "Image has a ColorSync profile (%ld bytes).\n", GetHandleSize( colorSyncProfile ) );
	
	// might this image have holes?  
	drawsAllPixels = graphicsImporterDrawsAllPixels;
	
	// find out if the graphics importer expects to draw every pixel
	// as some image file formats permit non-rectangular images or images
	// with transparent regions when such an image is drawn, not every
	// pixel in the boundary rectangle will be changed
	// ignore any error
	GraphicsImportDoesDrawAllPixels( importer,			// importer instance
									 &drawsAllPixels );	// ptr to value describing predicted drawing behaviour
	switch( drawsAllPixels ) {
		case graphicsImporterDrawsAllPixels:
			printf( "Image will overwrite every pixel in its DestRect.\n" );
			break;
		case graphicsImporterDoesntDrawAllPixels:
			printf( "Image will not overwrite every pixel in its DestRect.\n" );
			break;
		case graphicsImporterDontKnowIfDrawAllPixels:
			printf( "Image may or may not overwrite every pixel in its DestRect.\n" );
			break;
	}

	// Note: In a multiple-image file, the image description, metadata, 
	// default settings, etc. can be different for each image.
	
	CloseComponent( importer );
	DisposeHandle( (Handle)desc );
	if( defaultClip ) DisposeRgn( defaultClip );
	DisposeHandle( colorSyncProfile );
	
    printf( "\nPausing, press any key to continue.\n\n" );
	pause();
}