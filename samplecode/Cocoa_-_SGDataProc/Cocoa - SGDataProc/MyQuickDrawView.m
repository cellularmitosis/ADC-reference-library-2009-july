//////////
//
//	File:		MyQuickDrawView.m
//
//	Contains:	Implementation file for the MyQuickDrawView class
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	� 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	5/20/02	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple�s copyrights in 
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

#import <QuickTime/QuickTime.h>
#import <Carbon/Carbon.h>

#import "MyQuickDrawView.h"
#import "MyObject.h"

static MyQuickDrawView 		*myQDViewObject;	// our MyQuickDrawView object
static TimeScale			gTimeScale;			// time scale for our grabbed video
static TimeValue			gLastTime;			// time value when a frame was last given to us


@implementation MyQuickDrawView

#define BailErr(x) {err = x; if(err != noErr) goto bail;}
#define BailIfNull(x) {gSeqGrab = x; if(gSeqGrab == nil) goto bail;}



//////////
//
// setupDecomp
//
// Code to setup our decompresion sequences. We make
// two, one to decompress to a gworld, and the other
// to decompress to the window 
//
//////////

-(ComponentResult)setupDecomp
{
	ComponentResult			err = noErr;
    Rect					sourceRect = { 0, 0 }, bounds;
    MatrixRecord			scaleMatrix;	
    ImageDescriptionHandle	imageDesc = (ImageDescriptionHandle)NewHandle(0);
    PixMapHandle 			hPixMap;
    
    /* Set up getting grabbed data into the GWorld */
    
    // retrieve a channel�s current sample description, the channel returns a sample description that is
    // appropriate to the type of data being captured
    err = SGGetChannelSampleDescription(gSGChanVideo,(Handle)imageDesc);
    BailErr(err);

    /***** IMPORTANT NOTE *****
    
        Previous versions of this sample code made an incorrect decompression
        request.  Intending to draw the DV frame at quarter-size into a quarter-size
        offscreen GWorld, it made the call

        err = DecompressSequenceBegin(..., &rect, nil, ...);

        passing a quarter-size rectangle as the source rectangle.  The correct
        interpretation of this request is to draw the top-left corner of the DV
        frame cropped at normal size.  Unfortunately, a DV-specific bug in QuickTime
        5 caused it to misinterpret this request and scale the frame to fit.

        This bug will be fixed in QuickTime 6.  If your code behaves as intended
        because of the bug, you should fix your code to pass a matrix scaling the
        frame to fit the offscreen gworld:

        RectMatrix( & scaleMatrix, &dvFrameRect, &gworldBounds );
        err = DecompressSequenceBegin(..., nil, &scaleMatrix, ...);
    
        This approach will work in all versions of QuickTime.
                
    **************************/
    
    // make a scaling matrix for the sequence
    sourceRect.right = (**imageDesc).width;
    sourceRect.bottom = (**imageDesc).height;
    RectMatrix(&scaleMatrix, &sourceRect, &gBoundsRect);
            
    // begin the process of decompressing a sequence of frames
    // this is a set-up call and is only called once for the sequence - the ICM will interrogate different codecs
    // and construct a suitable decompression chain, as this is a time consuming process we don't want to do this
    // once per frame (eg. by using DecompressImage)
    // for more information see Ice Floe #8 http://developer.apple.com/quicktime/icefloe/dispatch008.html
    // the destination is specified as the GWorld
    err = DecompressSequenceBegin(&gDecomSeq,	// pointer to field to receive unique ID for sequence
                                    imageDesc,		// handle to image description structure
                                    gPGWorld,		// port for the DESTINATION image
                                    NULL,			// graphics device handle, if port is set, set to NULL
                                    NULL,			// source rectangle defining the portion of the image to decompress
                                    &scaleMatrix,	// transformation matrix
                                    srcCopy,		// transfer mode specifier
                                    NULL,			// clipping region in dest. coordinate system to use as a mask
                                    0,						// flags
                                    codecNormalQuality,		// accuracy in decompression
                                    bestSpeedCodec);		// compressor identifier or special identifiers ie. bestSpeedCodec
    BailErr(err);
    
    DisposeHandle((Handle)imageDesc);
    imageDesc = NULL;
    
    /* Set up getting grabbed data into the Window */
    hPixMap = GetGWorldPixMap(gPGWorld);
    GetPixBounds(hPixMap,&bounds);
    gDrawSeq = 0;
    
    // returns an image description for the GWorlds PixMap
    // on entry the imageDesc is NULL, on return it is correctly filled out
    // you are responsible for disposing it
    err = MakeImageDescriptionForPixMap(hPixMap, &imageDesc);
    BailErr(err);
    
    gImageSize = (GetPixRowBytes(hPixMap) * (*imageDesc)->height); // ((**hPixMap).rowBytes & 0x3fff) * (*desc)->height;
    
    // begin the process of decompressing a sequence of frames - see above notes on this call.
    // destination is specified as the QuickDraw port for our NSView
    err = DecompressSequenceBegin(&gDrawSeq,
                                    imageDesc,
                                    [self qdPort],	// Use the QuickDraw port for our NSView as destination!
                                    NULL,
                                    &bounds,
                                    NULL,
                                    ditherCopy,
                                    NULL,
                                    0,
                                    codecNormalQuality,
                                    anyCodec);
    BailErr(err);

bail:

    if (imageDesc)
        DisposeHandle((Handle)imageDesc);
    
    return (err);
}


//////////
//
// decompToWindow
//
// Decompress an image to our window (the QuickDraw port for
// our NSView)
//
//////////

-(ComponentResult)decompToWindow
{
	ComponentResult err = noErr;
    CodecFlags 		ignore;

    err = DecompressSequenceFrameS(gDrawSeq,							// sequence ID returned by DecompressSequenceBegin
                                GetPixBaseAddr(GetGWorldPixMap(gPGWorld)),	// pointer to compressed image data
                                gImageSize,									// size of the buffer
                                0,											// in flags
                                &ignore,									// out flags
                                NULL);										// async completion proc
    return err;
}

//////////
//
// doDecomp
//
// Setup and run our decompression sequence, plus display
// frames-per-second data to our window
//
//////////

-(void)doDecomp:(NSRect)rect
{    
    if(gPGWorld) 
    {
        if (gDecomSeq == 0) 
        {
            [self setupDecomp];
        }
        else
        {
            [self decompToWindow];
        }
    }
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
    [self doDecomp:rect];
}


//////////
//
// sgIdleTimer
//
// A timer whose purpose is to call the SGIdle function
// to provide processing time for our sequence grabber
// component. 
//
//////////

-(void) sgIdleTimer:(id)sender
{
    OSErr err;

    err = SGIdle(gSeqGrab);
    /* put up an error dialog to display any errors */
    if (err != noErr)
    {
        NSString *errorStr = [[NSString alloc] initWithFormat:@"%d" , err];
        int choice;

        // some error specific to SGIdle occurred - any errors returned from the
        // data proc will also show up here and we don't want to write over them
        
        // in QT 4 you would always encounter a cDepthErr error after a user drags
        // the window, this failure condition has been greatly relaxed in QT 5
        // it may still occur but should only apply to vDigs that really control
        // the screen
        
        // you don't always know where these errors originate from, some may come
        // from the VDig...
			
        /* now display error dialog and quit */
        choice = NSRunAlertPanel(@"Error", errorStr, @"OK", nil, nil);
        [errorStr release];
			
        // ...to fix this we simply call SGStop and SGStartRecord again
        // calling stop allows the SG to release and re-prepare for grabbing
        // hopefully fixing any problems, this is obviously a very relaxed
        // approach

        SGStop(gSeqGrab);
        SGStartRecord(gSeqGrab);
    }

}


//////////
//
// doSeqGrab
//
// Initialize the Sequence Grabber, create a new
// sequence grabber channel, create an offscreen
// GWorld for use with our decompression sequence,
// then begin recording. We also setup a timer to
// idle the sequence grabber
//
//////////

-(OSErr) doSeqGrab:(NSRect)grabRect
{
    OSErr	err = noErr;
    
    gTimeScale 	= 0;
    gLastTime 	= 0;

    /* initialize the movie toolbox */
    err = EnterMovies();
    BailErr(err);
    
    // open the sequence grabber component and initialize it
    gSeqGrab = OpenDefaultComponent(SeqGrabComponentType, 0);
    BailIfNull(gSeqGrab);
    
    err = SGInitialize(gSeqGrab);
    BailErr(err);
    
	// specify the destination data reference for a record operation
	// tell it we're not making a movie
	// if the flag seqGrabDontMakeMovie is used, the sequence grabber still calls
	// your data function, but does not write any data to the movie file
	// writeType will always be set to seqGrabWriteAppend
    err = SGSetDataRef(gSeqGrab, 0, 0, seqGrabDontMakeMovie);
    BailErr(err);

    // create a new sequence grabber video channel
    err = SGNewChannel(gSeqGrab, VideoMediaType, &gSGChanVideo);
    BailErr(err);

    gBoundsRect.top 	= grabRect.origin.y;
    gBoundsRect.left 	= grabRect.origin.x;
    gBoundsRect.bottom 	= grabRect.size.height;
    gBoundsRect.right 	= grabRect.size.width;
    err = SGSetChannelBounds(gSeqGrab, &gBoundsRect);
    
    // create the GWorld
    err = QTNewGWorld(&gPGWorld,	// returned GWorld
    				  k32ARGBPixelFormat,		// pixel format
    				  &gBoundsRect,				// bounding rectangle
    				  0,						// color table
    				  NULL,						// graphic device handle
    				  0);						// flags
    BailErr(err);
   
    // lock the pixmap and make sure it's locked because
    // we can't decompress into an unlocked PixMap
    if(!LockPixels(GetPortPixMap(gPGWorld)))
    {
    	BailErr(-1);
    }

    err = SGSetGWorld(gSeqGrab, gPGWorld, GetMainDevice());
    BailErr(err);

    // set the bounds for the channel
    err = SGSetChannelBounds(gSGChanVideo, &gBoundsRect);
    BailErr(err);
    
    // set the usage for our new video channel to avoid playthrough
    // note: we do not set seqGrabPlayDuringRecord because if you set this flag
    // the data from the channel may be played during the record operation,
    // if the destination buffer is onscreen. However, playing the
    // data may affect the quality of the recorded sequence by causing frames 
    // to be dropped...something we definitely want to avoid
    err = SGSetChannelUsage(gSGChanVideo, seqGrabRecord);
    BailErr(err);
    
    // specify a data function for use by the sequence grabber
    // whenever any channel assigned to the sequence grabber writes data,
    // this data function is called and may then write the data to another destination
    err = SGSetDataProc(gSeqGrab,NewSGDataUPP(&mySGDataProc),NULL);
    BailErr(err);

    /* lights...camera... */
    err = SGPrepare(gSeqGrab,false,true);
    BailErr(err);
    
    // start recording!!
    err = SGStartRecord(gSeqGrab);
    BailErr(err);

    /* setup a timer to idle the sequence grabber */
    gMyTimer = [[NSTimer scheduledTimerWithTimeInterval:0.1		// interval, 0.1 seconds
                        target:self
                        selector:@selector(sgIdleTimer:)		// call this method
                        userInfo:nil
                        repeats:YES] retain];					// repeat until we cancel it
    
bail:

	return err;
}

//////////
//
// gworld
//
// Accessor method for the gPGWorld class variable
//
//////////

-(GWorldPtr)gworld
{
    return gPGWorld;
}

//////////
//
// decomSeq
//
// Accessor method for the gDecomSeq class variable
//
//////////

-(ImageSequence)decomSeq
{
    return gDecomSeq;
}

//////////
//
// drawSeq
//
// Accessor method for the gDrawSeq class variable
//
//////////

-(ImageSequence)drawSeq
{
    return gDrawSeq;
}

//////////
//
// sgChanVideo
//
// Accessor method for the gSGChanVideo class variable
//
//////////

-(SGChannel)sgChanVideo
{
    return gSGChanVideo;
}

//////////
//
// boundsRect
//
// Accessor method for the boundsRect class variable
//
//////////

-(Rect)boundsRect
{
    return gBoundsRect;
}

//////////
//
// endGrab
//
// Perform clean-up when we are finished recording
//
//////////

-(void)endGrab
{
    ComponentResult result;
    OSErr 			err;

    // kill our sequence grabber idle timer first
    [gMyTimer invalidate];
    [gMyTimer release];
    
    // stop recording
    SGStop(gSeqGrab);

    // end our decompression sequences
    err = CDSequenceEnd(gDecomSeq);
    err = CDSequenceEnd(gDrawSeq);

    // finally, close our sequence grabber component
    result = CloseComponent(gSeqGrab);
    
    // get rid of our gworld
    DisposeGWorld(gPGWorld);
}

@end


/* ---------------------------------------------------------------------- */
/* sequence grabber data procedure - this is where the work is done
/* ---------------------------------------------------------------------- */
/* mySGDataProc - the sequence grabber calls the data function whenever
   any of the grabber�s channels write digitized data to the destination movie file.
   
   NOTE: We really mean any, if you have an audio and video channel then the DataProc will
   		 be called for either channel whenever data has been captured. Be sure to check which
   		 channel is being passed in. In this example we never create an audio channel so we know
   		 we're always dealing with video.
   
   This data function decompresses captured video data into an offscreen GWorld,
   then transfers the frame to an onscreen window.
   
   For more information refer to Inside Macintosh: QuickTime Components, page 5-120
   c - the channel component that is writing the digitized data.
   p - a pointer to the digitized data.
   len - the number of bytes of digitized data.
   offset - a pointer to a field that may specify where you are to write the digitized data,
   			and that is to receive a value indicating where you wrote the data.
   chRefCon - per channel reference constant specified using SGSetChannelRefCon.
   time	- the starting time of the data, in the channel�s time scale.
   writeType - the type of write operation being performed.
   		seqGrabWriteAppend - Append new data.
   		seqGrabWriteReserve - Do not write data. Instead, reserve space for the amount of data
   							  specified in the len parameter.
   		seqGrabWriteFill - Write data into the location specified by offset. Used to fill the space
   						   previously reserved with seqGrabWriteReserve. The Sequence Grabber may
   						   call the DataProc several times to fill a single reserved location.
   refCon - the reference constant you specified when you assigned your data function to the sequence grabber.
*/


pascal OSErr mySGDataProc(SGChannel c, 
                            Ptr p,
                            long len,
                            long *offset,
                            long chRefCon,
                            TimeValue time,
                            short writeType, 
                            long refCon)
{
#pragma unused(offset,chRefCon,time,writeType)
    
    CodecFlags 		ignore;
	ComponentResult err = noErr;
    CGrafPtr		theSavedPort;
    GDHandle    	theSavedDevice;
    char 			status[64];
    Str255			theString;
    Rect 			bounds;
    float 			fps;
    
    /* grab the time scale for use with our fps calculations - but this 
        needs to be done only once */
    if (gTimeScale == 0)
    {        
        err = SGGetChannelTimeScale([myQDViewObject sgChanVideo], &gTimeScale);
        BailErr(err);
    }
    
    if([myQDViewObject gworld]) 
    {
        // decompress a frame into the GWorld - can queue a frame for async decompression when passed in a completion proc
        // once the image is in the GWorld it can be manipulated at will
        err = DecompressSequenceFrameS([myQDViewObject decomSeq],	// sequence ID returned by DecompressSequenceBegin
        								p,						// pointer to compressed image data
        								len,					// size of the buffer
        								0,						// in flags
        								&ignore,				// out flags
        								NULL);					// async completion proc
        BailErr(err);

        // ******  IMAGE IS NOW IN THE GWORLD ****** //

	}
    
    /* compute and display frames-per-second */
    GetGWorld(&theSavedPort, &theSavedDevice);
    SetGWorld([myQDViewObject gworld], NULL);
    TextSize(12);
    TextMode(srcCopy);
    bounds = [myQDViewObject boundsRect];
    MoveTo(bounds.left, bounds.bottom-3);
    fps = (float)((float)gTimeScale / (float)(time - gLastTime));
    sprintf(status, "fps:%5.1f", fps);
    CopyCStringToPascal(status, theString);
    DrawString(theString);
    SetGWorld(theSavedPort, theSavedDevice);
    
    /* remember current time, so next time this routine is called
        we can compute the frames-per-second */
    gLastTime = time;

    /* calling the display method will invoke this NSView's lockFocus, drawRect and unlockFocus methods as necessary.
        Our drawRect method (above) is used to decompress one of a sequence of frames. This method draws the image
        back to the window from the GWorld and could be used as a "preview" */
    [myQDViewObject display];

bail:
    
	return err;
}

//////////
//
// saveQDViewObjectForCallback
//
// This routine stores a reference to our MyQuickDrawView object. We'll
// need this so we can call into methods in this class from outside the
// implementation of the class methods (specifically, from our SGDataProc
// C routine above).
//
//////////

void saveQDViewObjectForCallback(void *theObject)
{
   myQDViewObject = (MyQuickDrawView *)theObject;
}


