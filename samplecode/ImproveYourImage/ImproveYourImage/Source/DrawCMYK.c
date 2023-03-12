/*
	File:		DrawCMYK.c
	
	Description: This example demonstrates QuickTimes support for rendering using the CMYK colorspace.
                 Test Image: Sample 10-AlisaCMYK.jpg
                 Note: Nothing will draw if the image does not contain a kImageDescriptionColorSpace
                       extention.

	Author:		QuickTime engineering

	Copyright: 	© Copyright 1999 - 2005 Apple Computer, Inc. All rights reserved.
	
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

void DrawCMYK( void )
{
	OSErr err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	GraphicsImportComponent importer = 0;
	Rect naturalBounds, windowBounds;
	ImageDescriptionHandle desc = NULL, offscreenDesc = NULL;
	Handle h = NULL;
	OSType pixelFormat;
	GWorldPtr gworld = NULL;
	PixMapHandle pixmap;
	MatrixRecord matrix;
	ImageSequence imageSequence = 0;

	BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes );
	HLock( hOpenTypeList );

	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	DisposeHandle( hOpenTypeList );
	if ( err ) return;
    
    // locate and open a graphics importer component
	err = GetGraphicsImporterForFile( &theFSSpec, &importer );
    if ( err ) return;

	// find out the real colorspace.
	err = GraphicsImportGetImageDescription( importer, &desc );
    if ( err ) return;
    
	err = GetImageDescriptionExtension( desc, &h, kImageDescriptionColorSpace, 1 );
	if( !h || !*h ) return;
	pixelFormat = *(OSType*)*h;
	pixelFormat = EndianU32_BtoN(pixelFormat);
    printf( "Pixel Format: %c%c%c%c\n", (char)(pixelFormat>>24), (char)(pixelFormat>>16), 
							(char)(pixelFormat>>8), (char)pixelFormat);

	
	// Draw the image into an offscreen gworld of that colorspace.
	err = GraphicsImportGetNaturalBounds( importer, &naturalBounds );
	err = QTNewGWorld( &gworld, pixelFormat, &naturalBounds, NULL, NULL, 0 );
    if ( noErr == err ) {
    
		pixmap = GetGWorldPixMap( gworld );
		LockPixels( pixmap );
		err = GraphicsImportSetGWorld( importer, gworld, nil );
		err = GraphicsImportDraw( importer );
		
		windowBounds = naturalBounds;
		OffsetRect( &windowBounds, 10, 45 );
		window = NewCWindow( NULL, &windowBounds, "\pDraw CMYK", true, documentProc, 
				(WindowPtr)-1, true, 0);
		
		// Copy the image from the offscreen to the window using an image decompression sequence.
		err = MakeImageDescriptionForPixMap( pixmap, &offscreenDesc );

		SetIdentityMatrix( &matrix );
		
		err = DecompressSequenceBegin( &imageSequence, offscreenDesc, GetWindowPort(window), NULL, NULL, 
				&matrix, ditherCopy, NULL, 0, codecNormalQuality, nil );

		err = DecompressSequenceFrameS( imageSequence, GetPixBaseAddr( pixmap ),
				(**offscreenDesc).dataSize, 0, NULL, NULL );
		
		CDSequenceEnd( imageSequence );
		DisposeHandle( (Handle)offscreenDesc );
	}
	
	DisposeHandle( (Handle)desc );
	DisposeHandle( h );
	if ( gworld ) DisposeGWorld( gworld );
	CloseComponent( importer );
}
