/*
	File:		MiniMung.c
	
	Description: MiniMung shows how to run the Sequence Grabber in record mode and use
				 a DataProc to get at the captured data. This technique provides optimal
				 performance, far better than using preview mode with SG bottlenecks.
				 This code will help a lot when capturing from DV and should allow
				 30fps playthrough using DV capture on a G3.
                 
                 More specifically, our data proc will decompress the data to an offscreen,
                 after which a custom decompressor component will "decompress" the data
                 to the screen, after first applying an overlay image or peforming
                 color-clamping.

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

*/

#include "MiniMung.h"
#include "MungData.h"
#include "Utilities.h"
#include "QTUtilities.h"
#include "main.h"

extern mungDataPtr myMungData;

//////////
//
// constants
//
//////////

#define kMinimumIdleDurationInMillis	kEventDurationMillisecond
#define BailErr(x) {if (x != noErr) goto bail;}

//////////
//
// module variables
//
//////////

static SeqGrabComponent		mSeqGrab		= NULL;
static SGChannel 			mSGChanVideo 	= NULL;
static SGDataUPP 			mMyDataProcPtr 	= NULL;
static EventLoopTimerRef 	mSGTimerRef 	= 0;
static ImageSequence 		mDecomSeq 		= 0;
static Boolean				mUseOverlay;
static Boolean				mUseEffect;
static EventLoopTimerUPP	mSGTimerUPP		= nil;
static Rect					mMungRect 		= {0, 0, 240, 320};

//////////
//
// prototypes
//
//////////

static pascal OSErr MiniMungDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon);
static pascal void SGIdlingTimer(EventLoopTimerRef inTimer, void *inUserData);
static void DetectLobster(GWorldPtr mungDataOffscreen);


//////////
//
// DetectLobster
// Locate the lobster (defined by a certain
// amount of red color and brightness) in
// the current sequence grabber frame (offscreen).
//
//////////

static void DetectLobster(GWorldPtr mungDataOffscreen)
{
    CGrafPtr	oldPort;
    GDHandle	oldDevice;
    int			x, y;
    Rect		bounds;
    PixMapHandle	pix = GetGWorldPixMap(mungDataOffscreen);
    UInt32		* baseAddr;
    UInt32		reds = 0;
    Str255		tempString;
    int			minX = 10000, maxX = -10000;
    int			minY = 10000, maxY = -10000;
    Rect		tempRect;
    float		percent;
    OSErr		err;
	CodecFlags 	ignore;
    
    GetPortBounds(mungDataOffscreen, &bounds);
    OffsetRect(&bounds, -bounds.left, -bounds.top);
    for (y = 0; y < bounds.bottom; ++y)
    {
        baseAddr = (UInt32*)(GetPixBaseAddr(pix) + y * GetPixRowBytes(pix));
        for (x = 0; x < bounds.right; x++)
        {
            UInt32			color;
            long			Y,U,V;
            long			R;
            long			G;
            long			B;
            #define			kRange		(8)
            #define			kDark		(32)
            #define			kBright		(64)
            #define			RANGE_DEBUG	(0)
            
            // get the RGB of the color
            color = *baseAddr;
            R = (color & 0x00FF0000) >> 16;
            G = (color & 0x0000FF00) >> 8;
            B = (color & 0x000000FF) >> 0;
    
            // convert to YUV for comparison
            // 5 * R + 9 * G + 2 * B
            Y = (R + (R<<2) + G + (G<<3) + (B+B))>>4;
            U = B - Y;
            V = R - Y;
    
            #if RANGE_DEBUG
                if ((U < -kRange) && (V > kRange))
                    *baseAddr = 0x00FF0000;
                
                if ((Y > 1) && (Y < kBright))
                    *baseAddr = 0x00FFFFFF;
            #endif // RANGE_DEBUG
                    
            // bright enough, and red enough
            if ((Y > kDark) && (Y < kBright) && (U < -kRange) && (V > kRange))
                {
                // record the count of number of "lobster" pixels
                reds++;
    
                // and the maximum bounding box of same
                if (x < minX)
                    minX = x;
                if (x > maxX)
                    maxX = x;
                if (y < minY)
                    minY = y;
                if (y > maxY)
                    maxY = y;
                #if RANGE_DEBUG
                    *baseAddr = 0x0000FF00;
                #endif // RANGE_DEBUG
                }
            baseAddr++;
        }
    }
    
    percent = (float)reds / (bounds.right * bounds.bottom);
    
    GetGWorld(&oldPort, &oldDevice);
    SetGWorld(mungDataOffscreen, nil);
    NumToString(percent * 100, tempString);
    MoveTo(5, 10);
    ForeColor(whiteColor);
    DrawString(tempString);
    MoveTo(5, 10);
    Move(1, 1);
    ForeColor(blackColor);
    DrawString(tempString);
    
    // more than a certain percentage red?  A lobster!
    if (percent > .02)
    {
        PicHandle	aPicture = GetPicture(128);
        PenSize(2, 2);
        tempRect.left = minX;
        tempRect.right = maxX;
        tempRect.top = minY;
        tempRect.bottom = maxY;
        ForeColor(whiteColor);
        FrameRect(&tempRect);
        OffsetRect(&tempRect, 1, 1);
        ForeColor(blackColor);
        FrameRect(&tempRect);
        PenSize(1, 1);
                        
        SetGWorld(GetMungDataWindowPort(), nil);
    
        // draw an amount of butter proportional to the area the lobster is covering
        percent = (float)((tempRect.right - tempRect.left) * (tempRect.bottom - tempRect.top)) / (float)((bounds.right - bounds.left) * (bounds.bottom - bounds.top));
        if (aPicture)
        {
            tempRect.left = bounds.right;
            tempRect.top = bounds.top;
            tempRect.right = tempRect.left + ((**aPicture).picFrame.right - (**aPicture).picFrame.left);
            tempRect.bottom = tempRect.top + ((**aPicture).picFrame.bottom - (**aPicture).picFrame.top);
            for (y = 0; y < 5; ++y)
            {
                for (x = 0; x < 5; ++x)
                {
                    if ((x+1) +  5*(y) < percent*25)
                        DrawPicture(aPicture, &tempRect);
                    else
                        EraseRect(&tempRect);
                    OffsetRect(&tempRect, tempRect.right - tempRect.left, 0);
                }
            OffsetRect(&tempRect, -(tempRect.right-tempRect.left)*5, tempRect.bottom - tempRect.top);
            }
            ReleaseResource((Handle)aPicture);
        }
    
    }
    else
    {
        SetGWorld(GetMungDataWindowPort(), nil);
        tempRect.left = bounds.right;
        tempRect.top = bounds.top;
        tempRect.right = tempRect.left + 48*10;
        tempRect.bottom = tempRect.top + 48*10;
        EraseRect(&tempRect);
    }
    SetGWorld(oldPort, oldDevice);
        
    //this will draw it back to the window if you want to
        err = DecompressSequenceFrame(GetMungDataDrawSeq(),GetPixBaseAddr(GetGWorldPixMap(mungDataOffscreen)),0,&ignore,nil);
            
}

//////////
//
// MiniMungDataProc
// Sequence Grabber Data Procedure  - the sequence grabber calls 
// this data function whenever any of the grabber’s channels are
// about to write digitized data.
//
// This data function decompresses the captured video data into 
// our offscreen gworld. Then we call our custom decompressor
// component to draw this image to the screen.
// Our custom decompressor component has the ability to overlay
// an image on top of our frame, and can also perform "color
// clamping" of our frame data.
//
//////////

static pascal OSErr MiniMungDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
#pragma unused(offset,chRefCon,time,writeType,refCon)
	ComponentResult err = noErr;
	CodecFlags 		ignore;
    GWorldPtr 		gWorld;
    
	if (!myMungData) goto bail;
    
    gWorld = GetMungDataOffscreen();
	if(gWorld)
	{
		if (mDecomSeq == 0)	// init a decompression sequence
		{
			Rect bounds;
			
			GetMungDataBoundsRect(&bounds);
                    
            BailErr( CreateDecompSeqForSGChannelData(c, &bounds, gWorld, &mDecomSeq));
    
            if ((!mUseOverlay) && (GetCurrentClamp() == -1) && (!mUseEffect))
            {
				ImageSequence drawSeq;
				
                err = CreateDecompSeqForGWorldData(	gWorld, 
                                                    &bounds, 
                                                    nil, 
                                                    GetMungDataWindowPort(),
                                                    &drawSeq);
				SetMungDataDrawSeq(drawSeq);
            }
		}
        
        // decompress data to our offscreen gworld
		BailErr(DecompressSequenceFrameS(mDecomSeq,p,len,0,&ignore,nil));
		
		// image is now in the GWorld - manipulate it at will!
		if ((mUseOverlay) || (GetCurrentClamp() != -1) || (mUseEffect))
        {
			// use our custom decompressor to "decompress" the data
			// to the screen with overlays or color clamping
            BlitOneMungData(myMungData);
        }
		else
        {
			// we are doing a motion detect grab, so
			// search for lobsters in our image data
            DetectLobster(gWorld);
        }
		
	}
	
bail:
	return err;
}


//////////
//
// SGIdlingTimer
// Event loop timer which idles our Sequence Grabber
// channel.
//
//////////

static pascal void SGIdlingTimer(EventLoopTimerRef inTimer, void *inUserData)
{
#pragma unused(inUserData)

    if (mSeqGrab)
    {
        SGIdle(mSeqGrab);
    }
    
    // Reschedule the event loop timer
    SetEventLoopTimerNextFireTime(inTimer, kMinimumIdleDurationInMillis);
}


//////////
//
// MiniMung
// Begin the whole process of capturing frames.
// First, install our custom decompressor component. Then,
// create a Sequence Grabber channel for capturing. We'll 
// specify a Sequence Grabber Data Procedure, which will give
// us access to the compressed frames as they are made available.
// These frames are first decompressed to an offscreen gworld,
// then our custom decompressor is used to "decompress" the image
// to the screen. Our custom decompressor can overlay an image
// on top of the frame, or perform color clamping when it performs
// the "decompress".
//
//////////

OSErr MiniMung(WindowPtr window, Boolean withOverlay, Boolean withClamp, Boolean withEffect)
{
	OSStatus error;
	OSErr err = noErr;

	mUseOverlay = withOverlay;
	mUseEffect = withEffect;
	
	BailErr((err = InitializeMungData(mMungRect, window, withOverlay, withClamp, withEffect)));

    mMyDataProcPtr 	= NewSGDataUPP(MiniMungDataProc);
	mSeqGrab 		= OpenDefaultComponent(SeqGrabComponentType, 0);
    BailErr((err = CreateNewSGChannelForRecording(	mSeqGrab, 
                                    mMyDataProcPtr, 
                                    GetMungDataOffscreen(), // drawing destination
                                    &mMungRect, 
                                    &mSGChanVideo, 
                                    NULL)));

    mSGTimerUPP = NewEventLoopTimerUPP(SGIdlingTimer);
	error = InstallEventLoopTimer(  GetMainEventLoop(),
                                    0, // firedelay
                                    kEventDurationMillisecond * kMinimumIdleDurationInMillis,
                                    // interval
                                    mSGTimerUPP,
                                    0,
                                    &mSGTimerRef);

bail:
    return err;
}

//////////
//
// KillMiniMungGrab
// Stop grabbing frames - this involves closing any
// image sequences we've setup for decompressing
// frames and killing any timers used for idling our
// sequence grabber channel.
//
//////////

void KillMiniMungGrab()
{
    if(mDecomSeq) 
    {
        CDSequenceEnd(mDecomSeq); 
        mDecomSeq = 0;
    }

	if(mSGTimerRef)
    {
		RemoveEventLoopTimer(mSGTimerRef);
        mSGTimerRef = nil;
        DisposeEventLoopTimerUPP(mSGTimerUPP);
    }

    DoCloseSG(mSeqGrab, mSGChanVideo, mMyDataProcPtr);
    
    mSeqGrab = NULL;
    mSGChanVideo = NULL;
    mMyDataProcPtr = nil;    
    
    DisposeMungData();
}
