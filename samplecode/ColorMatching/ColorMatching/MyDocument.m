/*
	File:		MyDocument.m
	
	Description: MyDocument.m class implementation file. A subclass on NSDocument. Contains
				 code to handle display & color-matching for various image files.

	Author:		Apple Developer Technical Support
	
	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):
	   <1>	 	11/2/03 	SRK		first file
*/

#import "MyDocument.h"
#import "MyDocumentController.h"
#import "ImageProfiles.h"


@implementation MyDocument


//////////
//
// windowNibName:
// Override returning the nib file name of the document
//
//////////

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.

    return @"MyDocument";
}

//////////
//
// drawDocumentImage:
// signals our NSQuickDraw view to draw.
//
//////////

-(void)drawDocumentImage
{
    [drawingView setNeedsDisplay:YES];
}


//////////
//
// windowControllerDidLoadNib:
// code here is executed once the windowController has loaded the document's window.
// we'll perform window initialization, and update the image profiles in our image
// profile popup window
//
//////////

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    
    // Add any code here that need to be executed once the windowController has loaded the document's window.
    if ([self fileName])
    {
		NSRect imageRect;
		OSErr err;
		Rect imageBoundsRect;
        NSWindow* wind;

		// disable scrollers, as we don't currently allow scrolling
		[scrollView setHasHorizontalScroller:NO];
		[scrollView setHasVerticalScroller:NO];

		err = [mGraphicsImporterImage boundsRect:&imageBoundsRect];
        if (err != noErr) goto bail;
		imageRect = [Utils MacRectToNSRect:imageBoundsRect];
	
		[Utils setWindowFrameDimensions:[scrollView window] size:imageRect.size];
	
		[drawingView setFrame:imageRect];
		[drawingView setNeedsDisplay:YES];

		[[ImageProfiles sharedImageProfiles] updatePopupProfilesForDocument:self];

        wind = [aController window];
        if (wind == nil) goto bail;
        
        // when image window becomes main, we need to update our profile popups
		[[NSNotificationCenter defaultCenter]
			addObserver:self selector:@selector(windowBecomesMainNotification:)
			name:NSWindowDidBecomeMainNotification
			object:wind];

        // when image window is deminiaturized, we need to redraw
		[[NSNotificationCenter defaultCenter]
			addObserver:self selector:@selector(windowDeminiaturizeNotification:)
			name:NSWindowDidDeminiaturizeNotification
			object:wind];

        [self drawDocumentImage];

bail:
        return;

    }
}


//////////
//
// dataRepresentationOfType:
// Insert code here to write your document from the given data.
//
//////////

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // Insert code here to write your document from the given data.  You can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
    return nil;
}

//////////
//
// loadDataRepresentation:
// Insert code here to read your document from the given data.
//
//////////

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    // Insert code here to read your document from the given data.  You can also choose to override -loadFileWrapperRepresentation:ofType: or -readFromFile:ofType: instead.
    
    return YES;
}

//////////
//
// readFromFile:
// Reads and loads document data of type docType from the file 
// fileName, returning whether the operation was successful
//
//////////

-(BOOL)readFromFile:(NSString *)filename ofType:(NSString *)docType
{
	OSErr			err;
	CMError			cmErr;
	CMProfileRef	embeddedProfileRef = 0;
	Rect			imageBoundsRect;
	OSType 			srcProfMode;
	
	mImageColorWorldRef = nil;
	mImageDestGWorld	= nil;
	mImageSourceGWorld  = nil;
	mImageNSImage = nil;

	mGraphicsImporterImage = [[GraphicsImporterImage alloc] initGraphicsImporterImage:filename];
	err = [ColorSyncUtils setPantherQTColorMatchingOffForComponent:[mGraphicsImporterImage graphicsImportComponent]];

	cmErr = [mGraphicsImporterImage imageEmbeddedProfile:&embeddedProfileRef];
    
		// embed default rgb profile if there is no embedded
		// profile, and if we are requested to do so...
	if ((NULL == embeddedProfileRef) && autoEmbedProfileIfNone)
	{
		cmErr = [mGraphicsImporterImage imageEmbedDefaultRGBProfile];
	}	

	err = [mGraphicsImporterImage imageMacRect:&imageBoundsRect];
    if (err != noErr) goto bail;
	
	// if image is > screensize, change the image bounds rect
	[Utils resizeRectToFitMainScreen:&imageBoundsRect];
	[mGraphicsImporterImage setBoundsRect:&imageBoundsRect];

	err = [Utils AllocateGWorld: &imageBoundsRect theGWorldPtr:&mImageSourceGWorld];    
    if (err != noErr) goto bail;
	err = [Utils AllocateGWorld: &imageBoundsRect theGWorldPtr:&mImageDestGWorld];
    if (err != noErr) goto bail;

	mConcatProfileSet = [[ConcatProfileSet alloc] init];
    if (mConcatProfileSet == nil) goto bail;
	
    // set default modes
	srcProfMode = (embeddedProfileRef == nil) ? kRGB : kEmbedded;
	cmErr = [mConcatProfileSet setSrcModeAndProfile:srcProfMode aProfile:embeddedProfileRef];

	err = [mGraphicsImporterImage drawImageToGWorld:mImageSourceGWorld];
    if (err != noErr) goto bail;
	[self colorMatchImage];

    return YES;

bail:

    return NO;
}

//////////
//
// windowBecomesMainNotification:
// called when our window becomes main - we'll update our profile popups
// for the image here.
//
//////////

- (void) windowBecomesMainNotification:(NSNotification*)n
{
	[[ImageProfiles sharedImageProfiles] updatePopupProfilesForDocument:self];
}


//////////
//
// windowDeminiaturizeNotification:
// called when our window is displayed again after being miniaturized - we'll 
// redraw the image in this case.
//
//////////

- (void) windowDeminiaturizeNotification:(NSNotification*)n
{
	[self drawDocumentImage];
}

#pragma mark-

//////////
//
// concatProfileSetObj:
// get the ConcatProfileSet object for the image.
//
//////////

-(ConcatProfileSet *)concatProfileSetObj
{
    return (mConcatProfileSet);
}

//////////
//
// graphicsImporterObj:
// get the GraphicsImporterImage object for the image.
//
//////////

-(GraphicsImporterImage *)graphicsImporterObj
{
    return (mGraphicsImporterImage);
}


//////////
//
// imageNSRect:
// get the image natural bounds rect as a Cocoa NSRect.
//
//////////

-(NSRect)imageNSRect
{
	NSRect rect;
	
	[mGraphicsImporterImage imageNSRect:&rect];
	
	return rect;
}

//////////
//
// drawImageToGWorld:
// use the QuickTime Graphics Importer to draw the image to the specified port.
//
//////////

-(NSImage *)nsimage
{
	return (mImageNSImage);
}

//////////
//
// colorMatchImage:
// perform color matching for our image using the specified source,
// abstract, proofer and destination profiles. Uses either QuickTime Graphics
// Importer automatic ColorSync matching or regular ColorSync matching.
//
//////////

-(void)colorMatchImage
{
	if ([ColorSyncUtils isPantherQTColorMatchingTurnedOnForComponent:[mGraphicsImporterImage graphicsImportComponent]])
	{
		[mGraphicsImporterImage drawImageToGWorld:mImageDestGWorld];
	}
	else
	{
		[self colorMatchImageUsingColorSync];
	}
	
	// first get rid of any existing image
	if (mImageNSImage)
	{
		[mImageNSImage release];
	}
	
	// now create a new NSImage from the contents of our color-matched gworld
	mImageNSImage = [Utils gworldToNSImage:mImageDestGWorld];
}

//////////
//
// colorMatchImageUsingColorSync:
// color match the image using regular ColorSync matching.
//
//////////

-(CMError)colorMatchImageUsingColorSync
{
	CMError cmErr;	
	
		// first we dispose of the existing color world
	if (mImageColorWorldRef)
	{
		CWDisposeColorWorld(mImageColorWorldRef);
	}
	
	cmErr = [mConcatProfileSet buildColorWorld:&mImageColorWorldRef];
	if (cmErr != noErr) goto bail;
    
    cmErr = [ColorSyncUtils colorMatchSrcOffscreenToDestOffscreen:mImageSourceGWorld destGWorld:mImageDestGWorld colorWorld:mImageColorWorldRef];

bail:
    return cmErr;
}


//////////
//
// dealloc:
//
//////////

-(void)dealloc
{
	[[ImageProfiles sharedImageProfiles] updatePopupStates];

	if (mConcatProfileSet)
	{
		[mConcatProfileSet release];
	}

	if (mGraphicsImporterImage)
	{
		[mGraphicsImporterImage release];
	}

	if (mImageColorWorldRef)
	{
		CWDisposeColorWorld(mImageColorWorldRef);
	}
	
	if (mImageSourceGWorld)
	{
		DisposeGWorld(mImageSourceGWorld);
	}

	if (mImageDestGWorld)
	{
		DisposeGWorld(mImageDestGWorld);
	}
	
	if (mImageNSImage)
	{
		[mImageNSImage release];
	}

}

@end
