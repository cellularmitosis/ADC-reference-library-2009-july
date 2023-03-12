//////////
//
//	File:		MyQuickDrawView.m
//
//	Contains:	Implementation file for the MyQuickDrawView class.
//
//  Overview:
//
//  Subclass of NSQuickDrawView. This class contains code for drawing into a NSQuickDrawView. More specifically, 
//  we draw video frames to the NSQuickDrawView's QuickDraw port using QuickTime/QuickDraw calls. We
//  also draw on top of these video frames (compositing) using standard Cocoa, Core Graphics and QuickDraw routines.
//
//  Specifics:
//
//	This Cocoa sample demonstrates how to step frame-by-frame through a QuickTime movie using
//	the GetNextInterestingTime function, drawing each frame of the movie into an offscreen GWorld using 
//	a decompression sequence. The offscreen GWorld contents are then drawn into a Cocoa NSQuickDrawView 
//	(using the NSQuickDrawView's QuickDraw port) for display. Other drawing is also done into the NSQuickDrawView using
//	Cocoa, Core Graphics and QuickDraw routines.
//	
//	Lastly, a compile flag is provided which allows drawing into the NSQuickDrawView's QuickDraw 
//	port directly, bypassing the offscreen GWorld (Note - if you select this option to have QuickTime 
//	draw _directly_ to the NSQuickDrawView's QuickDraw port (via SetMovieGWorld), QuickTime essentially 
//	creates a subwindow which appears inside the main window to perform it's drawing. It's contents can 
//	completely cover the contents of the window buffer where all Cocoa drawing takes place so even if 
//	you try drawing on top (compositing) of it you will only see the movie drawing).
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	11/18/02	srk		first file
//     <2>      12/9/02     srk		added compile flag to optionally draw directly to NSQuickDrawView QT port,
//									other misc. improvements/re-organization of the code
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

#import "MyQuickDrawView.h"


#define DO_OFFSCREEN_DRAWING	1		/* 1 = draw to offscreen gworld first, then blit to window
                                           0 = draw to window QD port only */

#if DO_OFFSCREEN_DRAWING
    enum { kHeightOfWindowButtonArea = 50 };
#else
    enum { kHeightOfWindowButtonArea = 70 };
#endif



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
	decompressSeqID		= 0;		// unique identifier for our draw sequence
    firstDrawRect 		= TRUE;		// true for first call to the view's drawRect method
    movie 				= NULL;		// the QuickTime movie we are using
    srcGWorld 			= NULL;		// GWorld we are drawing into
    movieTime 			= 0;		// set current time value to begining of the Movie
    movieFrameCount 	= 0;		// movie frame count
    movieFrameNumber 	= 1;		// current movie frame number
    imageSize			= 0;		// how big is image
    
	EraseRect(&movieBox);

        /* prompt the user for a movie to display */
    movie = [self createQTMovieFromMovieFilePath:[myObject movieFilePath]];
    if (movie != NULL)
    {
        /* we must initialize the movie toolbox before calling
            any of it's functions */
        EnterMovies();
    
        movieFrameCount = [self countMovieFrames];
    
        // normalize the movie rect
        [self normalizeMovieRect];
        
        #if DO_OFFSCREEN_DRAWING
            /* make offscreen gworld to draw movie frames into */
            srcGWorld = [self makeGWorldForMovie];
            if (srcGWorld != NULL)
            {
                    /* set the graphics world for displaying the movie */
                SetMovieGWorld(movie, srcGWorld, GetGWorldDevice(srcGWorld));
            }
        #endif
    
        displayString = [self makeDisplayString];
        [self setViewWindowContentSizeAndPosition];
        [self setViewFrameRectToMovieRect];
        [self setMovieFrameButtonSizeAndPosition];
        
    }

}

//////////
//
// setMovieFrameButtonSizeAndPosition
//
// Adjust our button to fit into the window appropriately
//
//////////

-(void)setMovieFrameButtonSizeAndPosition
{
    NSRect newResizedRect, buttonAreaRect;
    
    NSAssert(movieFrameButton != nil, @"nil movie frame button");
    
    [movieFrameButton setFrameOrigin:NSMakePoint(0, 0)];
    buttonAreaRect = NSMakeRect(0, 0, 
                                [self normalizedMovieRectWidth], 
                                [self normalizedMovieRectHeight] + kHeightOfWindowButtonArea);
    buttonAreaRect.size.height = kHeightOfWindowButtonArea;
    newResizedRect = [self centerRectInEnclosingRect:[movieFrameButton frame] enclosingRect:buttonAreaRect];
    [movieFrameButton setFrame:newResizedRect];

}

//////////
//
// centerRectInEnclosingRect
//
// Utility routine which will center the specified rect in
// another rect
//
//////////

-(NSRect)centerRectInEnclosingRect:(NSRect) currentRect enclosingRect:(NSRect)enclosingRect
{
    float currentWidth = NSWidth(currentRect);
    float currentHeight = NSHeight(currentRect);
    float newX,newY;
    
    /* this is the new x,y */
    newX = (NSWidth(enclosingRect) - currentWidth)/2 + enclosingRect.origin.x;
    newY = (NSHeight(enclosingRect) - currentHeight)/2 + enclosingRect.origin.y;
    
    return (NSMakeRect(newX, newY, currentWidth, currentHeight));
    
}

//////////
//
// setViewFrameRectToMovieRect
//
// set the view's frame rect to the movie rect
//
//////////

-(void)setViewFrameRectToMovieRect
{
    NSRect newFrameRect = NSMakeRect(0, 0, 
                                [self normalizedMovieRectWidth], 
                                [self normalizedMovieRectHeight] + kHeightOfWindowButtonArea);
    [self setFrame:newFrameRect];
}

//////////
//
// setViewWindowContentSizeAndPosition
//
// set the view's window content size appropriately
//
//////////

-(void)setViewWindowContentSizeAndPosition
{
    NSWindow *window = [self window];
    
    if (window != nil)
    {
        [window setContentSize: NSMakeSize([self normalizedMovieRectWidth], 
                                            [self normalizedMovieRectHeight]  + kHeightOfWindowButtonArea)];
        
        [window center];
    }
    
    NSAssert(window != nil, @"nil window");
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
// nextFrame
//
//	Get the next frame of the movie, set the movie time for that frame
//  then task the movie which will draw the frame to the GWorld
//  finally draw the frame number on top of the image and inval the window rect
//
//////////

-(void)nextFrame
{
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

-(ComponentResult)initDecompressSeq:(CGrafPtr)dstPort
{
	ComponentResult			err = noErr;
    Rect					bounds;
    ImageDescriptionHandle	imageDesc = (ImageDescriptionHandle)NewHandle(0);
    PixMapHandle 			hPixMap = NULL;
        
    NSAssert(imageDesc != NULL, @"null ImageDescriptionHandle");
    NSAssert(srcGWorld != NULL, @"srcGWorld NULL");
    
    /* Set up getting grabbed data into the Window */
    hPixMap = GetGWorldPixMap(srcGWorld);
    NSAssert(hPixMap != NULL, @"null PixMapHandle");
    NSAssert(LockPixels(hPixMap), @"LockPixels failed");
    
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
    UnlockPixels(hPixMap);
    
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
// makeDisplayString
//
// create a string for display
//
//////////

-(NSMutableAttributedString *)makeDisplayString
{
    NSFont 				*font 	= nil;
    NSMutableDictionary *attrs 	= nil;

    font = [NSFont fontWithName:@"Helvetica" size:36.0];
    NSAssert(font != nil, @"fontWithName failure");

    attrs = [NSMutableDictionary dictionary];
    NSAssert(attrs != nil, @"NSMutableDictionary failure");

    [attrs setObject:font forKey:NSFontAttributeName];
    [attrs setObject:[NSColor blueColor]forKey:NSForegroundColorAttributeName];
    
    return ([[NSMutableAttributedString alloc]
                            initWithString:@"QuickTime!" 
                                attributes:attrs]);
}

//////////
//
// freeAllocatedObjects
//
//////////

-(void)freeAllocatedObjects
{
    #if DO_OFFSCREEN_DRAWING
        [self endDecompress];
    #endif
    
    [displayString release];
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
    /* do initialization the first time our drawRect
        is called */
    if (firstDrawRect)
    {
        #if DO_OFFSCREEN_DRAWING
                /* Draw to offscreen GWorld, then later blit to
                    the window's QuickDraw port */
            [self initDecompressSeq:[self qdPort]];
        #else
                /* Draw _directly_ to the NSQuickDrawView's QuickDraw port - note
                    that if you are doing this, QuickTime creates a subwindow 
                    which appears inside the main window to perform it's drawing.
                    It's contents can completely cover the contents of the window
                    buffer where all Cocoa drawing takes place so even if you try
                    drawing on top of it you will only see the movie drawing */
            SetMovieGWorld(movie, [self qdPort], nil);
        #endif
        
        firstDrawRect = FALSE;
    }
    
    /* This causes QuickTime to draw the movie frame into the 
        movie's designated gworld. */
    MoviesTask(movie,0);

    #if DO_OFFSCREEN_DRAWING
        /* use a decompression sequence to decompress frame from
         our offscreen gworld to the screen */
        [self decompress];
        /* do some drawing with Core Graphics calls */
        [self drawCoreGraphicsFivePointStar:[self qdPort]];
        /* do some Cocoa drawing of miscellaneous shapes over the frame */
        [self drawCocoaMiscShapes];    
        /* draw some shapes using QuickDraw calls */
        [self drawQuickDrawShapes];
    #endif

    // draw frame counter using Cocoa calls
    [self drawFrameCounter];

}

//////////
//
// drawCoreGraphicsFivePointStar
//
// use Core Graphics routines to draw a shape (a five point star)
//
//////////


-(void)drawCoreGraphicsFivePointStar:(CGrafPtr)port
{
    CGContextRef context = NULL;
    const float kMediaHeight = 50.0;
    const float kMediaWidth = 50.0;
    const float yOffset = 95.0, xOffset = 113.0;
    CGRect mediaBox;
    // This color approximates a green
    const float kStarRGBColor[3] = {0.1, 0.6, 0.1};
    // Define the locations (relative to the origin of the
    // graphics context) of all five points of the star
    const float kSize = (kMediaHeight * 0.9);
    const float kHalfSize = (kSize * 0.5);
    const float kTenthSize = (kSize * 0.1);
    CGPoint location[6] =
    {
        {kTenthSize + xOffset, 0 + yOffset}, // lower left
        {kHalfSize + xOffset, (kSize - kTenthSize) + yOffset}, // top
        {kSize - kTenthSize + xOffset, 0 + yOffset}, // lower right
        {0 + xOffset, kHalfSize + yOffset}, // left
        {kSize + xOffset, kHalfSize + yOffset}, // right
        {kTenthSize + xOffset, 0 + yOffset} // return to lower left
    };

    mediaBox = CGRectMake( 0, 0, kMediaWidth, kMediaHeight );
    // Create the context
    QDBeginCGContext( port, &context );
    if (context)
    {
        // We must begin a new page before drawing to a PDF context
        CGContextBeginPage( context, &mediaBox );
        //
        // Just for fun, skew the star slightly by rotating the CTM
        // a little bit.
        //
        // Since the rotation will otherwise cause the tip of one edge of the
        // star to be clipped, we translate the CTM ten units horizontally to
        // make sure the entire star fits on the page.
        //
        CGContextTranslateCTM( context, 10.0, 0.0 );
        CGContextRotateCTM( context, 0.1 );
        // Set the color
        CGContextSetRGBFillColor( context, kStarRGBColor[0],
                                kStarRGBColor[1],
                                kStarRGBColor[2],
                                1.0 );
        // Add the star to the path
        CGContextAddLines( context, &location[0], 6 );
        CGContextEOFillPath( context );
        // FillPath and EOFillPath always clear the path,
        // so we need to add the lines again to draw the star outline
        CGContextAddLines( context, &location[0], 6 );
        CGContextStrokePath( context );
        // We’ve finished rendering the page
        CGContextEndPage( context );
    
            //	We're done drawing
        QDEndCGContext( port, &context );
    }

}


//////////
//
// drawQuickDrawShapes
//
// draws some QuickDraw shapes 
//
//////////

-(void)drawQuickDrawShapes
{
    Rect aRect = {100,150,130,180};
    Pattern aPattern; 
    
    GetQDGlobalsLightGray(&aPattern);
    PenPat(&aPattern);
    PaintOval(&aRect);
}

//////////
//
// drawFrameCounter
//
// draws the current frame count using Cocoa calls
//
//////////

-(void)drawFrameCounter
{
    float yPos;
    NSSize stringSize;
    NSMutableDictionary *descriptionAttributes;
    NSString *frameNumDisplayString = [[NSString alloc] initWithFormat:@"Frame #%4.0f", (float)movieFrameNumber, nil];
    NSAssert(frameNumDisplayString != nil, @"failure to create string");

    descriptionAttributes = [[NSMutableDictionary alloc] init];
    NSAssert(descriptionAttributes != nil, @"failure to create NSMutableDictionary");
    [descriptionAttributes setObject:[NSFont fontWithName:@"Arial" size:11] forKey:NSFontAttributeName];
//  [descriptionAttributes setObject:[NSColor whiteColor] forKey:NSBackgroundColorAttributeName];	// background color
    stringSize = [frameNumDisplayString sizeWithAttributes:descriptionAttributes];
    #if DO_OFFSCREEN_DRAWING
        /* draw on top of the movie */
        yPos = [self normalizedMovieRectHeight] - stringSize.height;
    #else
        /* don't draw on top of the movie because it will get
            wiped out by the movie drawing - instead draw below it */
        yPos = [self normalizedMovieRectHeight];
    #endif
        /* draw the frame count string at the desired location */
	[frameNumDisplayString drawAtPoint:NSMakePoint(0,yPos) withAttributes:descriptionAttributes];

    /* clean up */
    [descriptionAttributes release];
    [frameNumDisplayString release];
}

//////////
//
// drawCocoaMiscShapes
//
// Use Cocoa calls to draw some text and other miscellaneous shapes
//
//////////

-(void)drawCocoaMiscShapes
{
    NSColor *aColor;

    aColor = [NSColor colorWithDeviceRed:0.5
                                    green:0.5
                                    blue:0.1
                                    alpha:0.5 ];
    NSAssert(aColor != nil, @"colorWithDeviceRed failure");
    [aColor set];
    
    [NSBezierPath fillRect:NSMakeRect(12,75,130,57)];
    
    [self drawChristmasStar];
    
    NSAssert(displayString != nil, @"displayString nil");
    [displayString drawAtPoint:NSMakePoint(16,30)];

}

//////////
//
// drawStar
//
// draw a star shape using Cocoa calls
//
//////////

-(void)drawChristmasStar
{
    NSBezierPath *path = [NSBezierPath bezierPath];
    NSPoint center = NSMakePoint(50,120);
    
    NSAssert(path != nil, @"bezierPath method failed");
    
    [[NSColor magentaColor] set];
    [path setLineWidth:2.0];
    [path moveToPoint:NSMakePoint(50,70)];
    [path curveToPoint:NSMakePoint(100,120)
        controlPoint1:center controlPoint2:center];
    [path curveToPoint:NSMakePoint(50,170)
        controlPoint1:center controlPoint2:center];
    [path curveToPoint:NSMakePoint(0,120)
        controlPoint1:center controlPoint2:center];
    [path curveToPoint:NSMakePoint(50,70)
        controlPoint1:center controlPoint2:center];
    [path stroke];
}


@end
