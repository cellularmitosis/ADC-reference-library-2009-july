/*
	File:		DoubleMung.c
	
	Description: DoubleMung is similar to MiniMung. It shows how to run the Sequence 
                Grabber in record mode and use a DataProc to get at the captured data. 
                However, it captures from two different cameras, then "blends" the data 
                from these two captures using a QT effect.

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

#include "MungData.h"
#include "DoubleMung.h"
#include "QTUtilities.h"

//////////
//
// constants
//
//////////

#define BailErr(x) {if (x != noErr) goto bail;}

//////////
//
// globals
//
//////////

Rect	gMungRect = {0, 0, 240, 320};

//////////
//
// module variables
//
//////////

static SeqGrabComponent			mSeqGrab 					= NULL;
static SGChannel				mSGChanVideo 				= NULL;
static SGDataUPP 				mMyDataProcPtr 				= nil;

static SeqGrabComponent			mSeqGrab2 					= NULL;
static SGChannel				mSGChanVideo2 				= NULL;
static SGDataUPP 				mMyData2ProcPtr 			= nil;

static ControlActionUPP 		mMySliderControlTrackProc 	= nil;

static EventLoopTimerUPP		mSGTimerUPP 				= nil;
static EventLoopTimerRef		mSGTimerRef 				= NULL;

static ControlHandle			mMixerSliderControl 		= NULL;

static GWorldPtr 				mGWorld2 					= nil;
static GWorldPtr 				mGWorld1 					= nil;

static ImageSequence 			mDecomSeq 					= NULL;
static ImageSequence 			mDecomSeq2 					= NULL;

static ImageSequence 			mDrawSeq 					= NULL;
static ImageSequence 			mDrawSeq2 					= NULL;
static ImageSequence 			mDrawSeq3 					= NULL;

static WindowPtr				mMungWindow 				= nil;

static OSType					mEffect 					= NULL;

static TimeBase					mEffectTimeBase 			= NULL;
static QTAtomContainer			mEffectParams 				= NULL;
static ImageDescriptionHandle 	mEffectDesc 				= NULL;
static ImageDescriptionHandle	mEffectDesc2 				= NULL;

//////////
//
// prototypes
//
//////////

static void CreateOurSliderControl(WindowPtr window);
static pascal void SliderControlActionProc(ControlRef	theControl, ControlPartCode	partCode);
static pascal void EventTimerProc(EventLoopTimerRef inTimer, void *inUserData);
static pascal OSErr Mung1DataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon);
static pascal OSErr Mung2DataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon);
static void CreateEffect();
static void StopImageSequence(ImageSequence *seq);


//////////
//
// EventTimerProc
// Event loop timer which idles our sequence grabber channels
//
//////////

static pascal void EventTimerProc(EventLoopTimerRef inTimer, void *inUserData)
{
#pragma unused (inTimer, inUserData)

	SGIdle(mSeqGrab);
	SGIdle(mSeqGrab2);
} 

//////////
//
// SliderControlActionProc
// Control Action Procedure for our slider control.
// We idle our Sequence Grabber channels here.
//
//////////

static pascal void SliderControlActionProc(ControlRef	theControl, ControlPartCode	partCode)
{
#pragma unused (theControl, partCode)

	SGIdle(mSeqGrab);
	SGIdle(mSeqGrab2);
} 

//////////
//
// CreateEffect
// Prompt the user for, and create an atom container
// with the appropriate atoms for the selected effect
//
//////////

static void CreateEffect()
{
    QTParameterDialog		dialog = nil;
    OSErr					dialogResult;
    Boolean					done = false;
    OSType					sourceType = 'srca';
    OSType					sourceType2 = 'srcb';

    BailErr(QTNewAtomContainer(&mEffectParams));

    mEffect = 130;
    BailErr(QTInsertChild(mEffectParams, kParentAtomIsContainer, 'wpID', 1, 0, 
                    sizeof(mEffect), &mEffect, nil));

    mEffect = 'smp2';
    BailErr(QTInsertChild(mEffectParams, kParentAtomIsContainer, kParameterWhatName, kParameterWhatID, 0, sizeof(mEffect), &mEffect, nil));

    QTCreateStandardParameterDialog(nil, mEffectParams, 0, &dialog);
    while (!done) 
    {
        EventRecord theEvent;

        WaitNextEvent(everyEvent, &theEvent, nil, 0);
        dialogResult = QTIsStandardParameterDialogEvent (&theEvent, dialog);
        if ((dialogResult == codecParameterDialogConfirm) || 
                (dialogResult == userCanceledErr)) 
        {
            done = true;
        }
    }
    QTDismissStandardParameterDialog(dialog);

    BailErr(QTInsertChild(mEffectParams, kParentAtomIsContainer, kEffectSourceName, 1, 0, sizeof(sourceType), &sourceType, nil));

    BailErr(QTInsertChild(mEffectParams, kParentAtomIsContainer, kEffectSourceName, 2, 0, sizeof(sourceType2), &sourceType2, nil));

    QTCopyAtomDataToPtr(mEffectParams, 
        QTFindChildByID(mEffectParams, kParentAtomIsContainer, kParameterWhatName, kParameterWhatID, nil),
        true, sizeof(mEffect), &mEffect, nil);

    HLockHi(mEffectParams);
bail:
;
}


//////////
//
// Mung1DataProc
// Sequence Grabber Data Procedure  - the sequence grabber calls 
// this data function whenever any of the grabber’s channels are
// about to write digitized data.
//
// This data function decompresses the captured video data for
// the first camera into an offscreen gworld. Once there, it 
// is copied to the screen.
//
//////////

static pascal OSErr Mung1DataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
#pragma unused(offset,chRefCon,time,writeType,refCon)
	ComponentResult err = noErr;
	CodecFlags ignore;

	if(mGWorld1)
	{
		if (mDecomSeq == 0)
		{			
			BailErr( CreateDecompSeqForSGChannelData(c, &gMungRect, mGWorld1, &mDecomSeq));

            err = CreateDecompSeqForGWorldData(	mGWorld1, 
                                                &gMungRect, 
                                                nil, 
                                                GetWindowPort(mMungWindow), 
                                                &mDrawSeq);
		}
        
        // decompress to offscreen
		BailErr(DecompressSequenceFrameS(mDecomSeq,p,len,0,&ignore,nil));
		
		// data is now in the offscreen, draw it onscreen
    BailErr(DecompressSequenceFrameS(mDrawSeq,
                                    GetPixBaseAddr(GetGWorldPixMap(mGWorld1)),
                                    0,
                                    0,
                                    &ignore,
                                    nil));
	}
	
bail:
	return err;
}

//////////
//
// BlendOffscreenGWorldsUsingQTEffects
// Start a decompression sequence for the effect, then
// create 2 new data sources for the effect. The first
// data source will reference the offscreen which contains
// the captured frames from our first camera. The second 
// data source will reference the offscreen which contains
// the captured frames from our second camera. Finally, we
// setup a time base for this decompression sequence. Our 
// slider control is then used to "run" the effect - as you
// move the slider, the effect is executed.
//
//////////

static void BlendOffscreenGWorldsUsingQTEffects()
{
    ICMFrameTimeRecord 	frameTime = {{0}};
    long				frameNum = 0, steps = 255;
	CodecFlags 			ignore;
    
    if (mDrawSeq3 == 0)
    {
        ImageDescriptionHandle imageDesc = nil;
        OSType					sourceType = 'srca';
        OSType					sourceType2 = 'srcb';
        ImageSequenceDataSource	effectSource;
        Rect					bounds;
        MatrixRecord			matrix;
        
        // create definition of our effect
        BailErr(MakeImageDescriptionForEffect (mEffect, &imageDesc));
        
        (**imageDesc).width = gMungRect.right - gMungRect.left;
        (**imageDesc).height = gMungRect.bottom - gMungRect.top;
        
        // start the decompression sequence for the effect
        bounds = gMungRect;
        SetIdentityMatrix(&matrix);
        TranslateMatrix(&matrix, ((bounds.right - bounds.left) / 2) << 16, (bounds.bottom - bounds.top + 52) << 16);
        BailErr(DecompressSequenceBeginS(&mDrawSeq3, imageDesc, 
                                        *(mEffectParams), GetHandleSize(mEffectParams),
                                        (GWorldPtr) GetWindowPort(mMungWindow), nil, &bounds, 
                                        &matrix,srcCopy,nil,0, codecNormalQuality, bestSpeedCodec));
        DisposeHandle((Handle)imageDesc);
        imageDesc =nil;
    
        // make a data source that references our first offscreen (camera 1)
        BailErr(MakeImageDescriptionForPixMap(GetPortPixMap(mGWorld1), &mEffectDesc));
        BailErr(CDSequenceNewDataSource(mDrawSeq3, &effectSource, sourceType, 1,
                        (Handle)mEffectDesc, nil, 0));
        BailErr(CDSequenceSetSourceData(effectSource, GetPixBaseAddr(GetPortPixMap(mGWorld1)), (**mEffectDesc).dataSize));
    
        // make a data source that references our second offscreen (camera 2)
        BailErr(MakeImageDescriptionForPixMap(GetPortPixMap(mGWorld2), &mEffectDesc2));
        BailErr(CDSequenceNewDataSource(mDrawSeq3, &effectSource, sourceType2, 1,
                        (Handle)mEffectDesc2, nil, 0));
        BailErr(CDSequenceSetSourceData(effectSource, GetPixBaseAddr(GetPortPixMap(mGWorld2)), (**mEffectDesc2).dataSize));
    
        // make the time base to run the effect
        mEffectTimeBase = NewTimeBase();
        SetTimeBaseRate(mEffectTimeBase, 0);		
        BailErr(CDSequenceSetTimeBase(mDrawSeq3, mEffectTimeBase));
    }


    frameNum = GetControlValue(mMixerSliderControl);

    SetTimeBaseValue(mEffectTimeBase, frameNum, steps);
                
    frameTime.value.lo = frameNum;
    frameTime.value.hi = 0;
    frameTime.scale = steps;
    frameTime.base = 0;
    frameTime.duration = steps;
    frameTime.rate = 0;
    frameTime.recordSize = sizeof(frameTime);
    frameTime.frameNumber = 1; // note: always 1, this is the SAMPLE number, not the frame within the effect!
    frameTime.flags = icmFrameTimeHasVirtualStartTimeAndDuration;
    frameTime.virtualStartTime.lo = 0;
    frameTime.virtualStartTime.hi = 0;
    frameTime.virtualDuration = steps;
            
    BailErr(DecompressSequenceFrameWhen(mDrawSeq3, 
                            *(mEffectParams), GetHandleSize(mEffectParams), 
                            0, &ignore, nil, &frameTime));

bail:
;
}

//////////
//
// Mung2DataProc
// Sequence Grabber Data Procedure  - the sequence grabber calls 
// this data function whenever any of the grabber’s channels are
// about to write digitized data.
//
// This data function decompresses the captured video data for
// the second camera into an offscreen gworld. Once there, it 
// is copied to the screen.
//
// Next, an effect is used to "blend" the frames from the two
// offscreen gworlds.
//
//////////

static pascal OSErr Mung2DataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
#pragma unused(offset,chRefCon,time,writeType,refCon)
	ComponentResult err = noErr;
	CodecFlags ignore;

	if(mGWorld2)
    {
		if (mDecomSeq2 == 0)
        {
            Rect			offsetRect;
            MatrixRecord 	mr;

            BailErr( CreateDecompSeqForSGChannelData(c, &gMungRect, mGWorld2, &mDecomSeq2));

            // offset the display of this decompress so it will display
            // in the window adjacent to (not on top of) the first capture
            offsetRect = gMungRect;
            OffsetRect(&offsetRect, (offsetRect.right - offsetRect.left) + 10, 0);
            RectMatrix(&mr, &gMungRect, &offsetRect);
            
            BailErr( CreateDecompSeqForGWorldData(	mGWorld2, 
													&gMungRect, 
													&mr, 
													GetWindowPort(mMungWindow),
													&mDrawSeq2));
        }
        
        // decompress to offscreen
		BailErr(DecompressSequenceFrameS(mDecomSeq2,p,len,0,&ignore,nil));
		
		// data is now in the offscreen, draw it onscreen
		BailErr(DecompressSequenceFrameS(	mDrawSeq2,
											GetPixBaseAddr(GetGWorldPixMap(mGWorld2)),
											0,
											0,
											&ignore,
											nil));

        // blend the two frames	into a third offscreen using QT Effects
        BlendOffscreenGWorldsUsingQTEffects();
    }
	
bail:
	return err;
}

//////////
//
// CreateOurSliderControl
// Create a slider control, which will be used to control
// execution of our QT Effect (to blend frames)
//
//////////

static void CreateOurSliderControl(WindowPtr window)
{
	Rect	controlBounds;

	controlBounds.left = 10;
	controlBounds.right = 320*2 - 10;
	controlBounds.top = gMungRect.bottom + 20;
	controlBounds.bottom = controlBounds.top + 32;
	
	mMySliderControlTrackProc = NewControlActionUPP(SliderControlActionProc);
    CreateSliderControl(window, &controlBounds, 0, 0, 255, kControlSliderPointsDownOrRight, 0, true, mMySliderControlTrackProc, &mMixerSliderControl);
	
	Draw1Control(mMixerSliderControl);
}


//////////
//
// DoubleMung
// Setup two Sequence Grabber channels to capture frames from
// two different cameras. Each channel will have a Sequence
// Grabber data procedure associated with it. The data procedure
// for the first channel will decompress the frames to an offscreen,
// then draw these frames to the screen. The data procedure for
// the second channel will decompress the frames to an offscreen, 
// then draw these frames to the screen, but it will also create an
// Image Sequence for a QT Effect. The QT Effect will have two data
// sources, one for each of the offscreen gworlds associated with each
// channel. As the QT Effect is run, it will blend the two offscreens
// together. A slider control is used to control execution of the effect.
//
//////////

OSErr DoubleMung(WindowPtr window)
{
	OSErr err = noErr;
	
    // offscreen for camera 1 frames
    BailErr((err = QTNewGWorld(&(mGWorld2),32,&gMungRect,0,0,0)));
	LockPixels(GetGWorldPixMap(mGWorld2));

    // offscreen for camera 2 frames
    BailErr((err = QTNewGWorld(&(mGWorld1),32,&gMungRect,0,0,0)));
	LockPixels(GetGWorldPixMap(mGWorld1));

    mMungWindow = window;
    mDrawSeq = 0;
    
    // prompt the user for an effect
    CreateEffect();
    
	// create Sequence Grabber channel for camera 1
	mSeqGrab 		= OpenDefaultComponent(SeqGrabComponentType, 0);
	mMyDataProcPtr	= NewSGDataUPP(Mung1DataProc);
    BailErr((err = CreateNewSGChannelForRecording(mSeqGrab, mMyDataProcPtr, mGWorld1, &gMungRect, &mSGChanVideo, NULL)));

	// create Sequence Grabber channel for camera 2
	mSeqGrab2 		= OpenDefaultComponent(SeqGrabComponentType, 0);
	mMyData2ProcPtr	= NewSGDataUPP(Mung2DataProc);
    BailErr((err = CreateNewSGChannelForRecording(mSeqGrab2, mMyData2ProcPtr, mGWorld2, &gMungRect, &mSGChanVideo2, NULL)));

    // create an event loop timer to idle our channels
	mSGTimerUPP	= NewEventLoopTimerUPP(EventTimerProc);
	mSGTimerRef	= nil;
	InstallEventLoopTimer(GetMainEventLoop(), kEventDurationNoWait, (kEventDurationSecond) / 30.0, mSGTimerUPP, NULL, &mSGTimerRef);

    // create a slider control, which will be used to control
    // execution of our QuickTime Effect
    CreateOurSliderControl(window);

    SetPortWindowPort(mMungWindow);

bail:
	return err;
}

//////////
//
// StopImageSequence
// End processing for our Image Sequence
//
//////////

static void StopImageSequence(ImageSequence *seq)
{
    if (seq)
    {
        if(*seq) 
        {
            CDSequenceEnd(*seq);
            *seq = NULL;
        }
    }
}

//////////
//
// KillDoubleMungGrab
// Stop grabbing frames. Dispose/cleanup all data
// structures allocated for our grab operations.
//
//////////

void KillDoubleMungGrab()
{

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

    DoCloseSG(mSeqGrab2, mSGChanVideo2, mMyData2ProcPtr);
    mSeqGrab2 = NULL;
    mSGChanVideo2 = NULL;
    mMyData2ProcPtr = nil;    

    if (mMixerSliderControl)
    {
        DisposeControl(mMixerSliderControl);
        mMixerSliderControl = NULL;
    }

    if(mGWorld1) 
    {
        DisposeGWorld(mGWorld1);
        mGWorld1 = nil;
    }
    
    if(mGWorld2) 
    {
        DisposeGWorld(mGWorld2);
        mGWorld2 = nil;
    }
    
    StopImageSequence(&mDecomSeq);
    StopImageSequence(&mDecomSeq2);
    StopImageSequence(&mDrawSeq2);
    StopImageSequence(&mDrawSeq);
    StopImageSequence(&mDrawSeq3);

    if (mEffectParams) 
    {
        QTDisposeAtomContainer(mEffectParams);
        mEffectParams = NULL;
    }
    
    if (mEffectDesc) 
    {
        DisposeHandle((Handle)mEffectDesc);
        mEffectDesc = NULL;
    }
    
    if (mEffectDesc2) 
    {
        DisposeHandle((Handle)mEffectDesc2);
        mEffectDesc2 = NULL;
    }
    
    if (mEffectTimeBase) 
    {
        DisposeTimeBase(mEffectTimeBase);
        mEffectTimeBase = NULL;
    }

    mEffect = 0;
}
