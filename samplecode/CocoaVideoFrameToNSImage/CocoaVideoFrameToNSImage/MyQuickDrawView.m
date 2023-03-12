//////////
//
//	File:		MyQuickDrawView.m
//
//	Contains:	Implementation file for the MyQuickDrawView class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	11/18/02	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
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

#import "MyQuickDrawView.h"


@implementation MyQuickDrawView


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
    NSString *filePath = nil;
    NSURL *fileURL = nil;

	decompressSeqID		= 0;		// unique identifier for our draw sequence
    firstDrawRect 		= TRUE;		// true for first call to the view's drawRect method
    movie 				= NULL;		// the QuickTime movie we are using
    srcGWorld 			= NULL;		// GWorld we are drawing into
    movieTime 			= 0;		// set current time value to begining of the Movie
    movieFrameCount 	= 0;		// movie frame count
    movieFrameNumber 	= 1;		// current movie frame number
    imageSize			= 0;		// how big is image
    backgroundImage		= nil;		// our background image for drawing
    
	EraseRect(&movieBox);

    filePath = [[NSBundle mainBundle] pathForResource:@"MovieFile" ofType:@"mov"];
    if (filePath)
    {
        movie = [self createQTMovieFromMovieFilePath:filePath];
        if (movie != NULL)
        {
    
            /* we must initialize the movie toolbox before calling
                any of it's functions */
            EnterMovies();
        
            movieFrameCount = [self countMovieFrames];
        
            // normalize the movie rect
            [self normalizeMovieRect];
            
            srcGWorld = [self makeGWorldForMovie];
            if (srcGWorld != NULL)
            {
                    /* set the graphics world for displaying the movie */
                SetMovieGWorld(movie, srcGWorld, GetGWorldDevice(srcGWorld));
            }
        
            /* retrieve our movie file from our bundle resource */
            filePath = [[NSBundle mainBundle] pathForResource:@"background" ofType:@"pct"];
            fileURL = [NSURL fileURLWithPath:filePath];
            /* create an NSMovie object from our QuickTime movie */
            backgroundImage = [[NSImage alloc] initWithContentsOfURL:fileURL];
            if (backgroundImage)
            {
                [self drawBackgroundImage];
            }
            
            /* draw the first frame */
            [self drawFrame];
        }
    }
    
    NSAssert(filePath != nil, @"movie not found in bundle");
    NSAssert(fileURL != nil, @"fileURLWithPath failed");
    NSAssert(backgroundImage != nil, @"initWithContentsOfURL failed");

}


//////////
//
// createQTMovieFromMovieFilePath
//
// Given a path to a movie file, create a
// QuickTime movie
//
//////////

-(Movie)createQTMovieFromMovieFilePath:(NSString *)movieFilePath
{
    NSURL 	*movieFileURL 	= nil;
    NSMovie *theNSMovie 	= nil;
    Movie 	qtMovie 		= NULL;

    NSAssert(movieFilePath, @"nil movie file path");
    
    movieFileURL = [NSURL fileURLWithPath:movieFilePath];
    if (movieFileURL != nil)
    {
        theNSMovie = [[NSMovie alloc] initWithURL:movieFileURL byReference:NO];
        if (theNSMovie != nil)
        {
            qtMovie = [theNSMovie QTMovie];
            if (qtMovie != NULL)
            {
                SetMovieActive(qtMovie,true);
            }
        }
    }
    
    NSAssert(movieFileURL != NULL, @"nil movieFileURL");
    NSAssert(theNSMovie != NULL, @"nil theNSMovie");
    NSAssert(qtMovie != NULL, @"null qtmovie");
    
    return (qtMovie);
    
}

//////////
//
// normalizeMovieRect
//
// Set the movie rect to be 0,0 based
//
//////////

-(void)normalizeMovieRect
{
    NSAssert(movie != NULL, @"NULL movie in normalizeMovieRect");

    GetMovieBox(movie, &movieBox);
    OffsetRect(&movieBox, -movieBox.left, -movieBox.top);
    SetMovieBox(movie, &movieBox);
}

//////////
//
// normalizedMovieRectWidth
//
// Return the normalized movie rect width
//
//////////

-(float)normalizedMovieRectWidth
{
    return((float)(movieBox.right - movieBox.left));
}

//////////
//
// normalizedMovieRectHeight
//
// Return the normalized movie rect height
//
//////////

-(float)normalizedMovieRectHeight
{
    return((float)(movieBox.bottom - movieBox.top));
}


//////////
//
// makeGWorldForMovie
//
//	Get the bounding rectangle of the Movie the create a 32-bit GWorld
//  with those dimensions.
//	This GWorld will be used for rendering Movie frames into.
//
//////////

-(GWorldPtr) makeGWorldForMovie
{
	Rect 		srcRect;
	GWorldPtr 	newGWorld = NULL;
	CGrafPtr	savedPort;
 	GDHandle    savedDevice;
    
	OSErr err = noErr;
    GetGWorld(&savedPort, &savedDevice);

	GetMovieBox(movie,&srcRect);
	
	err = NewGWorld(&newGWorld,
					k32ARGBPixelFormat,
					&srcRect,
					NULL,
					NULL,
					0);
	if (err == noErr)
    {
        if (LockPixels(GetGWorldPixMap(newGWorld)))
        {
            Rect 		portRect;
            RGBColor 	theBlackColor 	= { 0, 0, 0 };
            RGBColor 	theWhiteColor 	= { 65535, 65535, 65535 };

            SetGWorld(newGWorld, NULL);
            GetPortBounds(newGWorld, &portRect);
            RGBBackColor(&theBlackColor);
            RGBForeColor(&theWhiteColor);
            EraseRect(&portRect);
            
            UnlockPixels(GetGWorldPixMap(newGWorld));
        }
    }
    
    SetGWorld(savedPort, savedDevice);

    NSAssert(newGWorld != NULL, @"NULL gworld");
    
    return newGWorld;

}


//////////
//
// countMovieFrames
//
//	Count the number of video "frames" in the Movie by stepping through
//  all of the video "interesting times", or in other words, the places where the
//	movie displays a new video sample. The time between these interesting times
//  is not necessarily constant.
//
//////////

-(UInt32)countMovieFrames
{
	OSType		whichMediaType = VIDEO_TYPE;
	short		flags = nextTimeMediaSample + nextTimeEdgeOK;
	TimeValue	duration;
	TimeValue	theTime = 0;
    UInt32		frameCnt = 0;
	
    NSAssert(movie != NULL, @"NULL movie in countMovieFrames");
    
	while (theTime >= 0) 
    {
		frameCnt++;
		GetMovieNextInterestingTime(movie,
									flags,
									1,
									&whichMediaType,
									theTime,
									0,
									&theTime,
									&duration);
        NSAssert(GetMoviesError() == noErr, @"GetMovieNextInterestingTime error");
		//	after the first interesting time, don't include the time we
		//  are currently at.
        
		flags = nextTimeMediaSample;
	} // while
    
    return frameCnt;
}

//////////
//
// drawFrame
//
//  Task the movie to draw the frame to the GWorld
//
//////////

-(void)drawFrame
{
    CGrafPtr	savedPort;
    GDHandle   	savedDevice;

    GetGWorld(&savedPort, &savedDevice);
    SetGWorld(srcGWorld, NULL);

    // *** this does the actual drawing into the GWorld ***
    MoviesTask(movie,0);

    SetGWorld(savedPort, savedDevice);
}

//////////
//
// nextFrame
//
//	Get the next frame of the movie, set the movie time for that frame
//  then task the movie which will draw the frame to the GWorld
//  finally draw the frame number on top of the image and inval the window rect
//
//////////

-(void)nextFrame
{
    NSAssert(srcGWorld != NULL, @"null gworld");
    NSAssert(movie != NULL, @"null movie");

    if (movieFrameNumber >= movieFrameCount)
    {
        // reset the movie time back to the beginning
        // then do it all over again
        movieTime = 0;
        movieFrameNumber = 1;
        GoToBeginningOfMovie(movie);
    }
    else
    {
        short 		flags;
        OSType		whichMediaType;
        TimeValue	duration;

		// get the next frame of the source movie
		flags = nextTimeMediaSample;
		whichMediaType = VIDEO_TYPE;

		// if this is the first frame, include the frame we are currently on
		if (movieFrameNumber == 1)
        {
			flags |= nextTimeEdgeOK;
        }

		// skip to the next interesting time and get the duration for that frame
		GetMovieNextInterestingTime(movie,
									flags,
									1,
									&whichMediaType,
									movieTime,
									0,
									&movieTime,
									&duration);

		// set the time for the frame and give time to the movie toolbox
		SetMovieTimeValue(movie,movieTime);

        // You now have pixels you can play with in the GWorld!
        // This sample simply blits the contents of the GWorld back to
        // a window with the frame number drawn on top of the image
		
		movieFrameNumber++;		
	} 

}



//////////
//
// initDecompressSeq
//
// setup our decompression sequence for drawing
// into the gworld
//
//////////

-(ComponentResult)initDecompressSeq:(GWorldPtr)gworld destPort:(CGrafPtr)dstPort
{
	ComponentResult			err = noErr;
    Rect					bounds;
    ImageDescriptionHandle	imageDesc = (ImageDescriptionHandle)NewHandle(0);
    PixMapHandle 			hPixMap = NULL;
        
    NSAssert(imageDesc != NULL, @"null ImageDescriptionHandle");
    NSAssert(gworld != NULL, @"srcGWorld parameter NULL");
    NSAssert(dstPort != NULL, @"dstPort parameter NULL");
    
    /* Set up getting grabbed data into the Window */
    hPixMap = GetGWorldPixMap(gworld);
    NSAssert(hPixMap != NULL, @"null PixMapHandle");
    GetPixBounds(hPixMap,&bounds);
    decompressSeqID = 0;
    
    // returns an image description for the GWorlds PixMap
    // on entry the imageDesc is NULL, on return it is correctly filled out
    // you are responsible for disposing it
    err = MakeImageDescriptionForPixMap(hPixMap, &imageDesc);
    if (err == noErr)
    {
        imageSize = (GetPixRowBytes(hPixMap) * (*imageDesc)->height);
        
        // begin the process of decompressing a sequence of frames - see above notes on this call.
        // destination is specified as the QuickDraw port for our NSView
        err = DecompressSequenceBeginS( &decompressSeqID,
                                        imageDesc,
                                        GetPixBaseAddr(hPixMap),
                                        imageSize,
                                        dstPort,
                                        NULL,
                                        &bounds,
                                        NULL,
                                        ditherCopy,
                                        NULL,
                                        0,
                                        codecHighQuality,
                                        anyCodec);
    }

    NSAssert(err == noErr, @"DecompressSequenceBeginS error");

    if (imageDesc)
    {
        DisposeHandle((Handle)imageDesc);
    }
    
    return (err);
}

//////////
//
// decompress
//
// Use our decompression sequence to decompress into the gworld
//
//////////

-(OSErr)decompress
{
	ComponentResult err = noErr;
    CodecFlags 		ignore;

    err = DecompressSequenceFrameS( decompressSeqID,
                                    GetPixBaseAddr(GetGWorldPixMap(srcGWorld)),
                                    imageSize,
                                    0,
                                    &ignore,
                                    NULL);

    NSAssert(err == noErr, @"DecompressSequenceFrameS error");

    return err;

}

//////////
//
// endDecompress
//
// terminate our decompress
//
//////////

-(OSErr)endDecompress
{
    NSAssert(decompressSeqID != NULL, @"Null draw seq");
    
    return (CDSequenceEnd(decompressSeqID));
}


//////////
//
// freeAllocatedObjects
//
//////////

-(void)freeAllocatedObjects
{
    [backgroundImage release];
}

//////////
//
// drawRect
//
// Overridden by subclasses of NSView to draw the receiver's 
// image within aRect. It's here we decompress our frames
// to the window for display
//
//////////

-(void)drawRect:(NSRect)rect
{
    NSImage *image = nil;

    if (firstDrawRect)
    {
        [self initDecompressSeq:srcGWorld destPort:[self qdPort]];
        firstDrawRect = FALSE;
    }
    
    /* decompress frame to gworld */
    [self decompress];
    
    image = [self gworldToNSImage:srcGWorld];
    if (image)
    {
        [image setFlipped:YES];
        [image drawInRect:rect fromRect:rect operation:NSCompositeCopy fraction:0.4];
    }

    /* draw the frame counter */
	[self drawFrameCounter];

    /* draw background image */
    [self drawBackgroundImage];
}

//////////
//
// gworldToNSImage
//
// convert contents of a gworld to an NSImage 
//
//////////

-(NSImage *)gworldToNSImage:(GWorldPtr) gWorldPtr
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
// drawFrameCounter
//
// draws the current frame count 
//
//////////

-(void)drawFrameCounter
{
    char frame[32];
    Str255 theString;
    
    TextSize(12);
    TextMode(srcCopy);
    MoveTo(0,10);
    sprintf(frame,"Frame #%ld",movieFrameNumber);
    CopyCStringToPascal(frame, theString);
    DrawString(theString);
}


//////////
//
// drawBackgroundImage
//
// draw background image
//
//////////

-(void)drawBackgroundImage
{
    NSRect srcRect,dstRect;
    
    srcRect = NSMakeRect(0, 0, [self normalizedMovieRectWidth], [self normalizedMovieRectHeight]);
    dstRect = NSMakeRect(0, 0, [self normalizedMovieRectWidth], [self normalizedMovieRectHeight]);
    [backgroundImage drawInRect:dstRect fromRect:srcRect operation:NSCompositeSourceOver fraction:0.2];
}


@end
