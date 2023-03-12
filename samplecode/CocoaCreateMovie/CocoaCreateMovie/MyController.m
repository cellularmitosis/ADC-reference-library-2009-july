//////////
//
//	File:		MyController.m
//
//	Contains:	Implementation file for the MyController class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	7/18/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


//////////
//
// header files
//
//////////

#import "MyController.h"
#import <QuickTime/QuickTime.h>
#import "QuickTimeCode.h"



@implementation MyController

//////////
//
// getNumberOfImages
//
// Return the number of images in our
// image array - we'll use these images
// to construct the video track in our
// movie
//
//////////

- (int)getNumberOfImages
{
    return [imageArray count];
}

//////////
//
// getImageAtArrayIndex
//
// Return the NSImage from the specified
// index in our image array - we'll use
// these images to construct the video
// track in our movie
//
//////////

- (id)getImageAtArrayIndex:(int)index
{
    return [imageArray objectAtIndex:index];
}

//////////
//
// init
//
// Our controller's initialization method
//
//////////

- (id)init
{
    if (self = [super init]) 
    {
        // this will hold our NSImages which we will
        // use to construct our movie video track
        imageArray = [[NSMutableArray alloc] init];
        
        // we'll want to be called when the application
        // quits so we can do any cleanup
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(applicationWillTerminate:)
            name:@"NSApplicationWillTerminateNotification" object:NSApp];
    }
    return self;
}

//////////
//
// applicationWillTerminate
//
// We'll release any objects we initialized
// in our init method.
//
//////////

- (void)applicationWillTerminate:(NSNotification *)notification
{
    [imageArray dealloc];    
}


//////////
//
// awakeFromNib
//
// Called after all our objects are unarchived and
// connected but just before the interface is made visible
// to the user. We will do custom initialization of our
// objects here.
//
//////////

- (void)awakeFromNib
{
    // look for: <main bundle path>/Resources/bundlePath/name.extension>
    NSArray 	*imageBundlePathArray 	= [[NSBundle mainBundle] pathsForResourcesOfType:@"jpg" inDirectory:nil];    
    NSMovie	*myNSMovie 		= [[[NSMovie alloc] init] autorelease];
    Rect	movieRect 		= {0,0,0,0};
    int 	i;
    Movie 	theMovie;
    NSSize 	imageSize;
    NSPoint 	point;
    NSRect 	boundsRect;

    // iterate over all the jpeg images in our bundle and store
    // them in an NSImage array for later use
    for (i=0; i<[imageBundlePathArray count]; ++i)
    {
        NSImage *anImage = [[NSImage alloc] init];    
        NSURL *fileUrl = [NSURL fileURLWithPath:[imageBundlePathArray objectAtIndex:i]];        

        [imageArray addObject:[anImage initWithContentsOfURL:fileUrl]];

        [anImage release];
    }


    // pass our controller object instance to our C code so it may
    // be used to call methods in this class
    setObjCObject(self);

    // we'll size our movie view to that of the
    // first image we find in our bundle
    imageSize = [[imageArray objectAtIndex:0] size];
    [movieView setFrameSize:imageSize];

    point.x = 0; point.y = 0;
    [movieView setFrameOrigin:point];
    
    // size our window frame to that of the
    // first image we find in our bundle
    boundsRect.size.width = imageSize.width;
    boundsRect.size.height = imageSize.height+20;
    boundsRect.origin.x = 0;
    boundsRect.origin.y = 0;
    [window setFrame:boundsRect display:YES];
    point.x = 10; point.y = -20;
    [window setFrameTopLeftPoint:point];

    // QuickTime initialization
    EnterMovies();

    // our movie will be the same size as
    // the first image we find in our bundle
    movieRect.bottom = imageSize.height;
    movieRect.right = imageSize.width;

    // Create our movie!
    theMovie = CreateMovie(&movieRect);
    
    // Initialize our NSMovie with our newly created QuickTime movie
    [myNSMovie initWithMovie:theMovie];
    
    [movieView setMovie: myNSMovie];
    [movieView gotoBeginning:nil];
}

@end

//////////
//
// CopyNSImageToGWorld
//
// Copy the raw bitmap data from an NSImage
// to a QuickDraw GWorld
//
//////////

void CopyNSImageToGWorld(NSImage *image, GWorldPtr gWorldPtr)
{
    NSArray 		*repArray;
    PixMapHandle 	pixMapHandle;
    Ptr 		pixBaseAddr;
    int			imgRepresentationIndex;

    // Lock the pixels
    pixMapHandle = GetGWorldPixMap(gWorldPtr);
    LockPixels (pixMapHandle);
    pixBaseAddr = GetPixBaseAddr(pixMapHandle);

    repArray = [image representations];
    for (imgRepresentationIndex = 0; imgRepresentationIndex < [repArray count]; ++imgRepresentationIndex)
    {
        NSObject *imageRepresentation = [repArray objectAtIndex:imgRepresentationIndex];
        
        if ([imageRepresentation isKindOfClass:[NSBitmapImageRep class]])
        {
            Ptr bitMapDataPtr = [(NSBitmapImageRep *)imageRepresentation bitmapData];

            if ((bitMapDataPtr != nil) && (pixBaseAddr != nil))
            {
                int i,j;
                int pixmapRowBytes = GetPixRowBytes(pixMapHandle);
                NSSize imageSize = [(NSBitmapImageRep *)imageRepresentation size];
                for (i=0; i< imageSize.height; i++)
                {
                    unsigned char *src = bitMapDataPtr + i * [(NSBitmapImageRep *)imageRepresentation bytesPerRow];
                    unsigned char *dst = pixBaseAddr + i * pixmapRowBytes;
                    for (j = 0; j < imageSize.width; j++)
                    {
                        *dst++ = 0;		// X - our src is 24-bit only
                        *dst++ = *src++;	// Red component
                        *dst++ = *src++;	// Green component
                        *dst++ = *src++;	// Blue component           
                    }
                }
            }
        }
    }
    UnlockPixels(pixMapHandle);
}

