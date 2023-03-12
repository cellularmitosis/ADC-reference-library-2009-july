/*
	File:		Utils.m
	
	Description: Implementation file for the Utils.m class. Contains various utility routines
				 for acquiring documents, converting between similar Carbon & Cocoa data types
				 and creation/allocation of data structures.

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

#import "Utils.h"


@implementation Utils


//////////
//
// AllocateGWorld:
// create a 32-bit gworld of the given size.
//
//////////

+(OSErr) AllocateGWorld: (Rect*)gworldRect theGWorldPtr:(GWorldPtr *)theGWorldPtr
{
    OSErr		err;
    CGrafPtr	savePort;
    GDHandle	saveGDev;
    
    *theGWorldPtr = nil;
    
    err = NewGWorld (theGWorldPtr, 32, gworldRect, nil, nil, pixPurge|useTempMem);
    if (*theGWorldPtr == nil && !err)
    {
        err = memFullErr;
    }
    
    if (err != noErr)
    {
        return err;
    }
    
    GetGWorld(&savePort, &saveGDev);
    SetGWorld(*theGWorldPtr, nil);
    EraseRect(gworldRect);
    SetGWorld(savePort, saveGDev);
    
    return err;
}

//////////
//
// copyGWorldToPort:
// copy the source gworld to the destination port.
//
//////////

+(OSErr)copyGWorldToPort:(GWorldPtr)srcGWorld destPort:(CGrafPtr)destinationQDPort device:(GDHandle)aDevice
{
    CGrafPtr	savePort;
    GDHandle	saveGDev;
	OSErr		err = noErr;

    GetGWorld(&savePort, &saveGDev);

    if (srcGWorld && destinationQDPort)
    {
        PixMapHandle destPixMapHndl, srcPixMapHndl;
        
        srcPixMapHndl = GetGWorldPixMap(srcGWorld);

        destPixMapHndl = GetPortPixMap(destinationQDPort);
                
        if (LockPixels(srcPixMapHndl))
        {
            if (LockPixels(destPixMapHndl))
            {
                Rect srcBounds, destBounds;
                
                GetPortBounds(srcGWorld, &srcBounds);
                GetPortBounds(destinationQDPort, &destBounds);

                {
                    ImageDescriptionHandle imageDesc;

                    SetGWorld(destinationQDPort, aDevice);
                    
                    err = MakeImageDescriptionForPixMap(srcPixMapHndl, &imageDesc);
                    if (err == noErr)
					{
						err = DecompressImage(GetPixBaseAddr(srcPixMapHndl), imageDesc,
									destPixMapHndl, nil, &srcBounds, srcCopy, nil);
					}
                    
                    DisposeHandle((Handle)imageDesc);
                }
                
            }
            UnlockPixels (destPixMapHndl);
        }
        UnlockPixels (srcPixMapHndl);
    }
    
    SetGWorld(savePort, saveGDev);

	return err;
}

//////////
//
// gworldToNSImage
//
// convert contents of a gworld to an NSImage 
//
//////////

+(NSImage *)gworldToNSImage:(GWorldPtr) gWorldPtr
{
    PixMapHandle 		pixMapHandle = NULL;
    Ptr 				pixBaseAddr = nil;
    NSBitmapImageRep 	*imageRep = nil;
    NSImage 			*image = nil;
    
    NSAssert(gWorldPtr != nil, @"nil gWorldPtr");

    // Lock the pixels
    pixMapHandle = GetGWorldPixMap(gWorldPtr);
    if (pixMapHandle)
    {
        Rect 		portRect;
        unsigned 	portWidth, portHeight;
        int 		bitsPerSample, samplesPerPixel;
        BOOL 		hasAlpha, isPlanar;
        int 		destRowBytes;

        NSAssert(LockPixels(pixMapHandle) != false, @"LockPixels returns false");
    
        GetPortBounds(gWorldPtr, &portRect);
        portWidth = (portRect.right - portRect.left);
        portHeight = (portRect.bottom - portRect.top);
    
        bitsPerSample 	= 8;
        samplesPerPixel = 4;
        hasAlpha		= YES;
        isPlanar 		= NO;
        destRowBytes 	= portWidth * samplesPerPixel;
        imageRep 		= [[NSBitmapImageRep alloc]	initWithBitmapDataPlanes:NULL 
                                                                pixelsWide:portWidth 
                                                                pixelsHigh:portHeight 
                                                            bitsPerSample:bitsPerSample 
                                                        samplesPerPixel:samplesPerPixel 
                                                                hasAlpha:hasAlpha 
                                                                isPlanar:NO
                                                          colorSpaceName:NSDeviceRGBColorSpace 
                                                             bytesPerRow:destRowBytes 
                                                            bitsPerPixel:0];
        if (imageRep)
        {
            char 	*theData;
            int 	pixmapRowBytes;
            int 	rowByte,rowIndex;

            theData = [imageRep bitmapData];
        
            pixBaseAddr = GetPixBaseAddr(pixMapHandle);
            if (pixBaseAddr)
            {
                pixmapRowBytes = GetPixRowBytes(pixMapHandle);
            
                for (rowIndex=0; rowIndex< portHeight; rowIndex++)
                {
                    unsigned char *dst = theData + rowIndex * destRowBytes;
                    unsigned char *src = pixBaseAddr + rowIndex * pixmapRowBytes;
                    unsigned char a,r,g,b;
                    
                    for (rowByte = 0; rowByte < portWidth; rowByte++)
                    {
                        a = *src++;		// get source Alpha component
                        r = *src++;		// get source Red component
                        g = *src++;		// get source Green component
                        b = *src++;		// get source Blue component  
            
                        *dst++ = r;		// set dest. Alpha component
                        *dst++ = g;		// set dest. Red component
                        *dst++ = b;		// set dest. Green component
                        *dst++ = a;		// set dest. Blue component  
                    }
                }
            
                image = [[NSImage alloc]initWithSize:NSMakeSize(portWidth, portHeight)];
                if (image)
                {
                    [image addRepresentation:imageRep];
                    [imageRep release];
                }
            }
        }
    }

    NSAssert(pixMapHandle != NULL, @"null pixMapHandle");
    NSAssert(imageRep != nil, @"nil imageRep");
    NSAssert(pixBaseAddr != nil, @"nil pixBaseAddr");
    NSAssert(image != nil, @"nil image");

    if (pixMapHandle)
    {
        UnlockPixels(pixMapHandle);
    }

    return image;
}

//////////
//
// MacOSPantherOrBetter:
// returns true if Mac OS Panther or better is available.
//
//////////

+(Boolean)MacOSPantherOrBetter
{
    UInt32 response;
    
     return( Gestalt(gestaltSystemVersion, 
                    (SInt32 *) &response) == noErr)
                && (response >= 0x01030 );
}

//////////
//
// MacRectToNSRect:
// converts a Macintosh Rect to a Cocoa NSRect.
//
//////////

+(NSRect)MacRectToNSRect:(Rect)aRect
{
	return (NSMakeRect(0,0,(aRect.right - aRect.left),(aRect.bottom - aRect.top)));
}


//////////
//
// pathnameToFSSpec:
// returns an FSSpec for the specified NSString file path.
//
//////////

+(OSErr)pathnameToFSSpec:(NSString *)theFilePath fsSpec:(FSSpec *)theFSSpecPtr 
{
    ComponentResult 	err = noErr;
    FSRef			theFSRef;
    Boolean			isDirectory = false;

    NSAssert(theFilePath != nil, @"nil file path");

    // make FSRef
    if (theFilePath)
    {
        err = FSPathMakeRef([theFilePath cString], &theFSRef, &isDirectory);

        // get FSSpec
        if ((err == noErr) && !isDirectory)
        {
            err = FSGetCatalogInfo(&theFSRef, kFSCatInfoNone, nil, nil, theFSSpecPtr, nil);
        }
    }
    
    return err;
}

#pragma mark-

//////////
//
// cocoaStringFromCFStringRef:
// returns a Cocoa NSString for the specified CFString.
//
//////////

+(NSString *)cocoaStringFromCFStringRef:(CFStringRef)cfStr
{
    const char *cStringPtr = NULL;
    
    cStringPtr = CFStringGetCStringPtr(cfStr, kCFStringEncodingMacRoman);
    if (cStringPtr)
    {
        return ([NSString stringWithCString:cStringPtr]);
    }
	else
	{
		// try CFStringGetCString if CFStringGetCStringPtr fails

		char buf[255];
		Boolean success = CFStringGetCString(cfStr, buf, 255, kCFStringEncodingMacRoman);
		if (success)
		{
			return ([NSString stringWithCString:buf]);
		}
	}
    
    return nil;

}

//////////
//
// getCurrentDocument:
// queries the NSDocumentController to get the current NSDocument.
//
//////////

+(id)getCurrentDocument
{
    return([[NSDocumentController sharedDocumentController] currentDocument]);
}



//////////
//
// filenameFromFullPath:
// returns the filename specified by the full path.
//
//////////

+ (NSString *)filenameFromFullPath:(NSString *)fullPath
{
	NSArray *pathComponents = [fullPath componentsSeparatedByString:@"/"];

	return ([pathComponents objectAtIndex:([pathComponents count] - 1)]);
}

//////////
//
// showErrorDialog:
// display an error dialog showing the specified error.
//
//////////

+(void)showErrorDialog:(long)err
{
    // report any errors which occurred
    if (err != noErr)
    {
        NSString *errorStr = [[NSString alloc] initWithFormat:@"%d" , err];
        int choice;
            
        /* now display error dialog and quit */
        choice = NSRunAlertPanel(@"Error", errorStr, @"OK", nil, nil);
        [errorStr release];
    }
}


//////////
//
// resizeRectToFitMainScreen:
// resizes the given rectangle to fit within the main screen bounds.
//
//////////

+(void)resizeRectToFitMainScreen:(Rect *)aRect
{
    NSRect screenFrame;
    NSSize screenSize;
    NSScreen *mainScreen = [NSScreen mainScreen];

    screenFrame = [mainScreen frame];
    screenSize = screenFrame.size;

	if (((aRect)->right - (aRect)->left) > screenSize.width ||
			((aRect)->bottom - (aRect)->top) > screenSize.height)
    {
        short widthDifference = ((aRect)->right - (aRect)->left) - (screenSize.width - 100);
        short heightDifference = ((aRect)->bottom - (aRect)->top) - (screenSize.height - 100);
        (aRect)->right = (aRect)->right - widthDifference;
        (aRect)->bottom = (aRect)->bottom - heightDifference;
    }
}


//////////
//
// setWindowFrameDimensions:
// set the NSWindow dimensions.
//
//////////

+(void)setWindowFrameDimensions:(NSWindow *)aWindow size:(NSSize)aSize
{
	NSRect windowFrame = [aWindow frame];
	
	windowFrame.size.width = aSize.width;
	windowFrame.size.height = aSize.height;

	[aWindow setFrame:windowFrame display:YES];
}


//////////
//
// promptUserForMovieFile
//
// Prompt the user for a file to open
//
//////////

+(NSString *)promptForProfileFile
{
    NSOpenPanel *oPanel = nil;
	NSArray *fileExtensions = [NSArray arrayWithObjects: @"icc", @"pf", @"icm", @"'prof'", 0];

    oPanel = [NSOpenPanel openPanel];
    NSAssert(oPanel != nil, @"openPanel failed");
    
    if ([oPanel runModalForDirectory:nil file:nil types:fileExtensions] == NSOKButton)
    {
        return [oPanel filename];            
    }
    else
    {
        return nil;
    }
}


@end
