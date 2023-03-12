/*
	File:		ExportFromCGBitmapContext.c
	
	Description: The first part of this sample does what DrawUsingCGImage does, but then this same
                 CGImage is drawn to a CGBitmapContext and exported as a PNG.

	Author:		QuickTime Engineering, DTS

	Copyright: 	© Copyright 2003 - 2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <1> 08/16/05 initial release

*/

#include "MacShell.h"

void ExportFromCGBitmapContext( void )
{
	OSErr  err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	Rect   bounds = { 45, 10, 100, 100 };
	GraphicsImportComponent importer = 0;
	GraphicsExportComponent exporter = 0;
	CGImageRef imageRef = 0;
	CGContextRef context = NULL;
	CGRect rect;

	BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes );
	HLock( hOpenTypeList );

	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	DisposeHandle( hOpenTypeList );
	if ( err ) return;

	// locate and open a graphics importer component which can be used to draw the
	// selected file. If a suitable importer is not found the ComponentInstance
	// is set to NULL.
	err = GetGraphicsImporterForFile( &theFSSpec,	// specifies the file to be drawn
									  &importer );	// pointer to the returned GraphicsImporterComponent
	                                  
	window = NewCWindow( NULL, &bounds, "\pExport PNG From CGBitmapContext", false, documentProc, (WindowPtr)-1, true, 0);

	// import the image as a CGImage
	err = GraphicsImportCreateCGImage( importer, &imageRef, kGraphicsImportCreateCGImageUsingCurrentSettings );
	if (err) return;

	SizeWindow( window, CGImageGetWidth( imageRef ), CGImageGetHeight( imageRef ), false );
	ShowWindow(window);
    
    // create a Core Graphics Context from the window port
	err = QDBeginCGContext(GetWindowPort(window), &context);
	if (err) return;

	// make a rectangle designating the location and dimensions in user space of the bounding box in which to draw the image
	rect = CGRectMake( 0, 0, CGImageGetWidth( imageRef ), CGImageGetHeight( imageRef ) );

	// draw the image
	CGContextDrawImage( context, rect, imageRef );

    // end the the context we had for the port
	QDEndCGContext(GetWindowPort(window), &context);
	
	// close the importer instance
	CloseComponent(importer);
    
    // open the PNG exporter
    err = OpenADefaultComponent(GraphicsExporterComponentType, kQTFileTypePNG, &exporter);
	if (noErr == err) {
		
        // create a CGBitmapContext to draw into
		CGContextRef bitmapContextRef = NULL;
		CGColorSpaceRef colorSpace;
		
		void  *bitmapData;
		int	  bitmapBytesPerRow = (CGImageGetWidth(imageRef) * 4);
		int	  bitmapByteCount = (bitmapBytesPerRow * CGImageGetHeight(imageRef));
    	
    	colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
		bitmapData = malloc(bitmapByteCount);
    	
    	bitmapContextRef = CGBitmapContextCreate(bitmapData, CGImageGetWidth(imageRef), CGImageGetHeight(imageRef), 8, bitmapBytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst);  
		if (NULL != bitmapContextRef) {
			 UInt32 sizeWritten;
             Boolean isSelected, isReplacing;
			 
			// draw the image
			CGContextDrawImage(bitmapContextRef, rect, imageRef);

	    	err = PutFile("\pSave PNG to:", "\pExported.png", &theFSSpec, &isSelected, &isReplacing);
            if (noErr == err) {	
                
                // set the CGBitmapContext that the graphics exporter will use as its input image
                // you could have also used GraphicsExportSetInputCGImage in this example because
                // we already have a CGImage but that use is pretty obvious
                err = GraphicsExportSetInputCGBitmapContext(exporter, bitmapContextRef);
                
                // set the destination of the export
                err = GraphicsExportSetOutputFile(exporter, &theFSSpec);
                
                // do the export
                err = GraphicsExportDoExport(exporter, &sizeWritten);
            }
        	
            // release the bitmap context
        	CFRelease(bitmapContextRef);
        }
        
        // close the export component
		CloseComponent(exporter);
		
        // free the bits and the color space
		free (bitmapData);
		CGColorSpaceRelease( colorSpace );	
	}
}