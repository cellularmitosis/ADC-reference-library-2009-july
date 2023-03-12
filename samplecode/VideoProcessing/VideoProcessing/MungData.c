/*
	File:		MungData.c
	
	Description: Code to create & initialize data structure whose contents will be used
                during decompression operations in conjunction with our custom
                decompressor component. Our data structure include an offscreen
                gworld for decompressing frames. Also included is color clamp 
                information which is passed to our decompressor component for 
                performing color clamp operations on our data, and an overlay 
                offscreen which is used to overlay an image on top of our frame data.

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
#include "AppBlit_Component.h"
#include "QTUtilities.h"
#include "Utilities.h"


//////////
//
// constants
//
//////////

#define	BailNULL(n)  if (!n) goto bail;
#define	BailError(n) if (n) goto bail;
#define	BailNil(n) if (!n) goto bail;
#define BailErr(x) {if (x != noErr) goto bail;}
#define bitdepth 32

//////////
//
// globals
//
//////////

mungDataPtr myMungData = NULL;

//////////
//
// module variables
//
//////////

long		mWorlds[20];
UInt32		mRedCount[256], mGreenCound[256], mBlueCount[256];

//////////
//
// prototypes
//
//////////

static void DecompressSequencePreflight(GWorldPtr srcGWorld, 
                                        ImageSequence *imageSeq,
                                        GWorldPtr destGWorld,
                                        Rect *srcRect);
static void DrawRGBHistogram(mungDataRecord *theMungData);
static void CreateEffectDescription(mungDataRecord *theMungData);
static void CreateEffectDecompSequence(mungDataRecord *theMungData);
static void AddGWorldDataSourceToEffectDecompSeq(mungDataRecord *theMungData);
static void MakeEffectTimeBaseForEffect(mungDataRecord *theMungData);
static void DrawUsingEffect(mungDataRecord *theMungData);

//////////
//
// InitializeMungData
// Create & initialize a data structure whose contents will be used
// during decompression operations in conjunction with our custom
// decompressor component. Our data structure include an offscreen
// gworld for decompressing frames. Also included is color clamp information
// which is passed to our decompressor component for performing color
// clamp operations on our data, and an overlay offscreen which is used
// to overlay an image on top of our frame data.
//
//////////

OSErr InitializeMungData(Rect bounds, WindowPtr	window, Boolean createOverlayGWorld, Boolean withClamp, Boolean withEffect)
{
	OSErr err = noErr;
    
	if(myMungData) 
    {
        DisposeMungData();
	}
    
	myMungData = (mungDataPtr)NewPtrClear(sizeof(mungDataRecord));
	if (myMungData == nil) 
    {
        err = MemError();
        goto bail;
    }

    if (!withEffect)
    {
        myMungData->effect = 0;
    }

	BailErr(QTNewGWorld(&(myMungData->gw),bitdepth,&bounds,0,0,0));
	LockPixels(GetGWorldPixMap(myMungData->gw));
    
    SetMungDataColorDefaults();

    if (createOverlayGWorld)
    {
        myMungData->selectedIndex = -1;

        BailErr(QTNewGWorld(&(myMungData->overlay),bitdepth,&bounds,0,0,0));
        LockPixels(GetGWorldPixMap(myMungData->overlay));

        EraseRectAndAlpha(myMungData->overlay, &bounds);
        
        // put the overlay into the GWorld, move the picture to the 
		// bottom right of the movie
        DrawLobsterPICTtoGWorld(myMungData->overlay, &bounds);
    }
    else
    {
        myMungData->selectedIndex = 0;
        myMungData->overlay = NULL;

        if (withClamp)
        {
            SetCurrentClamp(0);
        }
        else
        {
            SetCurrentClamp(-1);
            if (withEffect)
            {
                myMungData->effect = 'fmns';
            }
        }

    }
	
	myMungData->bounds = bounds;
	myMungData->window = window;

	SetRect(&bounds, 0, 0, 256*2+4, 128*3 + 20);
	BailErr(QTNewGWorld(&(myMungData->histoWorld),bitdepth,&bounds,0,0,0));
	LockPixels(GetGWorldPixMap(myMungData->histoWorld));

bail:
	return err;
}

//////////
//
// DisposeMungData
// Dispose our data structure
//
//////////

OSErr DisposeMungData(void)
{
    OSErr err = noErr;
    
    if(myMungData) 
    {
        if(myMungData->drawSeq) 
        {
            CDSequenceEnd(myMungData->drawSeq);
        }
        
        if(myMungData->gw) 
        {
            DisposeGWorld(myMungData->gw); 
            myMungData->gw = nil;
        }
        
        if(myMungData->overlay) 
        {
            DisposeGWorld(myMungData->overlay); 
            myMungData->overlay = nil;
        }
        
        if(myMungData->histoWorld) 
        {
            DisposeGWorld(myMungData->histoWorld); 
            myMungData->histoWorld = nil;
        }
    
        if (myMungData->effectTimeBase)
        {
            DisposeTimeBase(myMungData->effectTimeBase);
        }
        if (myMungData->effectParams)
        {
            QTDisposeAtomContainer(myMungData->effectParams);
        }
        if (myMungData->effectDesc)
        {
            DisposeHandle((Handle)myMungData->effectDesc);
        }

        DisposePtr((Ptr)myMungData);
        myMungData = nil;
    }

    return err;
}


//////////
//
// CreateEffectDescription
// Build an Effects Description - an atom container which
// specifies an effect and its sources 
//
//////////

static void CreateEffectDescription(mungDataRecord *theMungData)
{
    OSType sourceType = 'srca';

    BailErr(QTNewAtomContainer(&theMungData->effectParams));

    BailErr(QTInsertChild(theMungData->effectParams, kParentAtomIsContainer, kEffectSourceName, 1, 0, 
                    sizeof(sourceType), &sourceType, nil));

    BailErr(QTInsertChild(theMungData->effectParams, kParentAtomIsContainer, kParameterWhatName, kParameterWhatID, 0, sizeof(theMungData->effect), &theMungData->effect, nil));
    HLockHi(theMungData->effectParams);

bail:
;
}

//////////
//
// CreateEffectDecompSequence
// Create a decompression sequence for our effect
//
//////////

static void CreateEffectDecompSequence(mungDataRecord *theMungData)
{
    ImageDescriptionHandle 	imageDesc = nil;

    // create definition of our effect
    BailErr(MakeImageDescriptionForEffect (theMungData->effect, &imageDesc));
    
    (**imageDesc).width = theMungData->bounds.right - theMungData->bounds.left;
    (**imageDesc).height = theMungData->bounds.bottom - theMungData->bounds.top;

    // start the decompression sequence for the effect
    BailErr(DecompressSequenceBeginS(&theMungData->drawSeq, 
                                    imageDesc,
                                    *(theMungData->effectParams),
                                    GetHandleSize(theMungData->effectParams),
                                    (GWorldPtr) GetWindowPort(theMungData->window), 
                                    nil, 
                                    &theMungData->bounds, 
                                    nil,
                                    srcCopy,
                                    nil,
                                    0, 
                                    codecNormalQuality, 
                                    bestSpeedCodec));
    DisposeHandle((Handle)imageDesc);

bail:
;
}

//////////
//
// AddGWorldDataSourceToEffectDecompSeq
// Add our offscreen gworld as a data source to our effect
// decompression sequence
//
//////////

static void AddGWorldDataSourceToEffectDecompSeq(mungDataRecord *theMungData)
{
    ImageSequenceDataSource	effectSource;
    OSType sourceType = 'srca';

    // make a data source that references our offscreen
    BailErr(MakeImageDescriptionForPixMap(GetPortPixMap(theMungData->gw),
                                            &theMungData->effectDesc));

    BailErr(CDSequenceNewDataSource(theMungData->drawSeq, &effectSource, sourceType, 1,
                    (Handle)theMungData->effectDesc, nil, 0));
    BailErr(CDSequenceSetSourceData(effectSource, GetPixBaseAddr(GetPortPixMap(theMungData->gw)), (**theMungData->effectDesc).dataSize));

bail:
;
}

//////////
//
// MakeEffectTimeBaseForEffect
// Create a new timebase to run our effect
//
//////////

static void MakeEffectTimeBaseForEffect(mungDataRecord *theMungData)
{
    // make the time base to run the effect
    theMungData->effectTimeBase = NewTimeBase();
    SetTimeBaseRate(theMungData->effectTimeBase, 0);		
    BailErr(CDSequenceSetTimeBase(theMungData->drawSeq, theMungData->effectTimeBase));

bail:
;

}


//////////
//
// DrawUsingEffect
// Setup a decompression sequence for our effect, then draw.
// We use our offscreen gworld as a data source to the effect.
//
//////////

static void DrawUsingEffect(mungDataRecord *theMungData)
{
    ICMFrameTimeRecord 	frameTime = {{0}};
    long				frameNum = 0, steps = 1;
    CodecFlags 			ignore = 0;
    
    
    if (theMungData->drawSeq == 0)
    {
        // build an effect decompression sequence
        CreateEffectDescription(theMungData);
        CreateEffectDecompSequence(theMungData);
        AddGWorldDataSourceToEffectDecompSeq(theMungData);
        MakeEffectTimeBaseForEffect(theMungData);
    }
    
    SetTimeBaseValue(theMungData->effectTimeBase, frameNum, steps);
    
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
            
    DecompressSequenceFrameWhen(theMungData->drawSeq,
                                *(theMungData->effectParams),
                                GetHandleSize(theMungData->effectParams),
                                0,
                                &ignore,
                                nil,
                                &frameTime);
}

//////////
//
// DecompressSequencePreflight
// Pass a compressed sample so a codec can perform preflighting 
// before the first DecompressSequenceFrameWhen call
//
//////////

static void DecompressSequencePreflight(GWorldPtr srcGWorld, 
                                        ImageSequence *imageSeq,
                                        GWorldPtr destGWorld,
                                        Rect *srcRect)
{
    ImageDescriptionHandle imageDesc = nil;
    
    BailErr(MakeImageDescriptionForPixMap (GetGWorldPixMap(srcGWorld), &imageDesc));
    
    // use our built-in decompressor to draw
    (**imageDesc).cType = kCustomDecompressorType;
    
    // pass a compressed sample so a codec can perform preflighting before the first DecompressSequenceFrameWhen call

    BailErr(DecompressSequenceBegin(imageSeq, 
                                    imageDesc, 
                                    destGWorld,
                                    0,
                                    srcRect, 
                                    nil,
                                    srcCopy,
                                    nil,
                                    0,
                                    codecNormalQuality,
                                    bestSpeedCodec));

bail:
    if (imageDesc)
    {
        DisposeHandle((Handle)imageDesc);
    }
}


//////////
//
// DrawRGBHistogram
// Draw color histogram table (red, green and blue) for our image
//
//////////

static void DrawRGBHistogram(mungDataRecord *theMungData)
{
    Rect	histoRect, destRect, worldRect;
    int		x, pen = 2;
    float	totalPixels;

    SetGWorld(theMungData->histoWorld, nil);
    
    GetPortBounds(theMungData->histoWorld, &worldRect);
    EraseRect(&worldRect);
    
    PenSize(pen, 1);
    histoRect = theMungData->bounds;
    OffsetRect(&histoRect, histoRect.right - histoRect.left, 0);
    destRect.left = histoRect.left;
    totalPixels = -128.0 / ((histoRect.bottom - histoRect.top) * (histoRect.right - histoRect.left)) * 32.0;
    histoRect.bottom = histoRect.top + 128;
    histoRect.right = histoRect.left + 256*pen;
    OffsetRect(&histoRect, -histoRect.left, -histoRect.top);
    
    // red
    ClipRect(&histoRect);
    for (x = 1; x < 256; ++x)
        {
        MoveTo(histoRect.left + x*pen, histoRect.bottom);
        ForeColor(redColor);
        Line(0, totalPixels * mRedCount[x]);
        }
    ClipRect(&worldRect);
    if (theMungData->selectedIndex == 0)
        ForeColor(magentaColor);
    else
        ForeColor(blackColor);
    MoveTo(histoRect.left + theMungData->redMin*pen-pen, histoRect.bottom);
    LineTo(histoRect.left + theMungData->redMin*pen-pen, histoRect.top);
    if (theMungData->selectedIndex == 1)
        ForeColor(magentaColor);
    else
        ForeColor(blackColor);
    MoveTo(histoRect.left + theMungData->redMax*pen+pen, histoRect.bottom);
    LineTo(histoRect.left + theMungData->redMax*pen+pen, histoRect.top);
    OffsetRect(&histoRect, 0, histoRect.bottom - histoRect.top + 10);

    // green
    ClipRect(&histoRect);
    for (x = 1; x < 256; ++x)
        {
        MoveTo(histoRect.left + x*pen, histoRect.bottom);
        ForeColor(greenColor);
        Line(0, totalPixels * mGreenCound[x]);
        }
    ClipRect(&worldRect);
    ForeColor(blackColor);
    if (theMungData->selectedIndex == 2)
        ForeColor(magentaColor);
    else
        ForeColor(blackColor);
    MoveTo(histoRect.left + theMungData->greenMin*pen-pen, histoRect.bottom);
    LineTo(histoRect.left + theMungData->greenMin*pen-pen, histoRect.top);
    if (theMungData->selectedIndex == 3)
        ForeColor(magentaColor);
    else
        ForeColor(blackColor);
    MoveTo(histoRect.left + theMungData->greenMax*pen+pen, histoRect.bottom);
    LineTo(histoRect.left + theMungData->greenMax*pen+pen, histoRect.top);
    OffsetRect(&histoRect, 0, histoRect.bottom - histoRect.top + 10);
        
    // blue
    ClipRect(&histoRect);
    for (x = 1; x < 256; ++x)
        {
        MoveTo(histoRect.left + x*pen, histoRect.bottom);
        ForeColor(blueColor);
        Line(0, totalPixels * mBlueCount[x]);
        }
    ClipRect(&worldRect);
    ForeColor(blackColor);
    if (theMungData->selectedIndex == 4)
        ForeColor(magentaColor);
    else
        ForeColor(blackColor);
    MoveTo(histoRect.left + theMungData->blueMin*pen-pen, histoRect.bottom);
    LineTo(histoRect.left + theMungData->blueMin*pen-pen, histoRect.top);
    if (theMungData->selectedIndex == 5)
        ForeColor(magentaColor);
    else
        ForeColor(blackColor);
    MoveTo(histoRect.left + theMungData->blueMax*pen+pen, histoRect.bottom);
    LineTo(histoRect.left + theMungData->blueMax*pen+pen, histoRect.top);
    PenSize(1, 1);
    
    SetGWorld(GetWindowPort(theMungData->window), nil);
    GetPortBounds(theMungData->histoWorld, &histoRect);
    destRect.top = histoRect.top;
    destRect.bottom = histoRect.bottom;
    destRect.left = destRect.left;
    destRect.right = destRect.left + histoRect.right - histoRect.left;
    CopyBits(
            (BitMap*)*GetGWorldPixMap(theMungData->histoWorld),
            (BitMap*)*GetGWorldPixMap(GetWindowPort(theMungData->window)),
            &histoRect,
            &destRect,
            srcCopy, nil);    
}

//////////
//
// BlitOneMungData
// Our custom blitter. This will perform a "decompress"
// operation of the data in our offscreen gworld using 
// our custom decompressor component and draw the resulting
// image data to the screen.
//
// This custom decompressor component has the ability
// to overlay graphics on top of the data as well as
// perform color clamping of the data.
//
//////////

void BlitOneMungData(mungDataRecord *theMungData)
{
    static	long nextDraw = 0;

    if (theMungData->effect)
    {
		// draw using a QT Effect
        DrawUsingEffect(theMungData);
        
        return;
    }
		
    // draw using a custom built-in blitter
    
    // create a sequence if we do not already have one
    if (theMungData->drawSeq == 0)
    {
        // install our custom blitter
        InstallAppBlitComponentCodec();

        // Pass a compressed sample so a codec can perform preflighting 
        // before the first DecompressSequenceFrameWhen call
        DecompressSequencePreflight(theMungData->gw, 
                                    &theMungData->drawSeq,
                                    GetWindowPort(theMungData->window),
                                    &theMungData->bounds);
    }
        
    // draw the resulting (offscreen) image to the screen
    {
        CodecFlags 	ignore = 0;
        
        // our built-in decompressor takes two GWorlds as the
        // "compressed data", plus #count array pointers, plus the 
        // clamp values
        mWorlds[0] = (long)theMungData->gw;
        mWorlds[1] = (long)theMungData->overlay;
        mWorlds[2] = (long)mRedCount;
        mWorlds[3] = (long)mGreenCound;
        mWorlds[4] = (long)mBlueCount;
        mWorlds[5] = theMungData->redMin;
        mWorlds[6] = theMungData->redMax;
        mWorlds[7] = theMungData->greenMin;
        mWorlds[8] = theMungData->greenMax;
        mWorlds[9] = theMungData->blueMin;
        mWorlds[10] = theMungData->blueMax;

        BailErr(DecompressSequenceFrame(theMungData->drawSeq,
									(void*)&mWorlds,// "compressed data" = our custom data
									0,
									&ignore,
									nil));
        
        // if we are in color adjustment mode, draw that to the window
        if ((TickCount() > nextDraw) && (theMungData->selectedIndex >= 0))
        {
            DrawRGBHistogram(theMungData);
            
            nextDraw = TickCount() + 5;
        }

    }
    
bail:
    return;

}


//////////
//
// GetMungDataOffscreen
// Accessor method which retrieves the current offscreen
// used for drawing
//
//////////

GWorldPtr GetMungDataOffscreen()
{
    return (myMungData->gw);
}

//////////
//
// SetMungDataDrawSeq
// Accessor method to set the draw sequence
//
//////////

void SetMungDataDrawSeq(ImageSequence theDrawSeq)
{
	myMungData->drawSeq = theDrawSeq;
}

//////////
//
// GetMungDataDrawSeq
// Accessor method to get the draw sequence value
//
//////////

ImageSequence GetMungDataDrawSeq()
{
	return myMungData->drawSeq;
}


//////////
//
// SetMungDataColorDefaults
// Reset our color default values used in color 
// clamping operations
//
//////////

void SetMungDataColorDefaults()
{
	if(myMungData) 
    {
        myMungData->redMin = 2;
        myMungData->redMax = 254;
        myMungData->greenMin = 2;
        myMungData->greenMax = 254;
        myMungData->blueMin = 2;
        myMungData->blueMax = 254;
    }
}



//////////
//
// AdjustColorClampEndpoints
// Accessor method to set a color clamp endpoint value
//
//////////

void AdjustColorClampEndpoints(short hMouseCoord)
{
    short hCoord = hMouseCoord;
    
    hCoord -= 320;
    hCoord /= 2;
    if (hCoord < 0)
        hCoord = 0;
    if (hCoord > 255)
        hCoord = 255;
    
    if (myMungData)
    {
        switch (myMungData->selectedIndex)
        {
            case 0: myMungData->redMin=hCoord; break;
            case 1: myMungData->redMax=hCoord; break;
            case 2: myMungData->greenMin=hCoord; break;
            case 3: myMungData->greenMax=hCoord; break;
            case 4: myMungData->blueMin=hCoord; break;
            case 5: myMungData->blueMax=hCoord; break;
        }
    }
}


//////////
//
// GetMungDataBoundsRect
// Accessor method to get the bounds rect in use
//
//////////

void GetMungDataBoundsRect(Rect *boundsRect)
{
	MacSetRect (boundsRect, 
			myMungData->bounds.left, 
			myMungData->bounds.top, 
			myMungData->bounds.right, 
			myMungData->bounds.bottom
	);
}

//////////
//
// GetMungDataWindowPort
// Accessor method to get the window port
//
//////////

CGrafPtr GetMungDataWindowPort()
{
	return GetWindowPort(myMungData->window);
}

//////////
//
// GetCurrentClamp
// Accessor method to get the currently selected
// color clamp endpoint
//
//////////

long GetCurrentClamp()
{
    return myMungData->selectedIndex;
}

//////////
//
// SetCurrentClampEndpoint
// Accessor method to set the current color clamp
// endpoint value
//
//////////

void SetCurrentClamp(short index)
{
    myMungData->selectedIndex = index;
}


//////////
//
// IncrementCurrentClamp
// Accessor method to increment the current
// color clamp value
//
//////////

void IncrementCurrentClamp()
{
    switch (myMungData->selectedIndex)
        {
        case 0: myMungData->redMin++; break;
        case 1: myMungData->redMax++; break;
        case 2: myMungData->greenMin++; break;
        case 3: myMungData->greenMax++; break;
        case 4: myMungData->blueMin++; break;
        case 5: myMungData->blueMax++; break;
        }
}

//////////
//
// DecrementCurrentClamp
// Accessor method to decrement the current color
// clamp value
//
//////////

void DecrementCurrentClamp()
{
    switch (myMungData->selectedIndex)
        {
        case 0: myMungData->redMin--; break;
        case 1: myMungData->redMax--; break;
        case 2: myMungData->greenMin--; break;
        case 3: myMungData->greenMax--; break;
        case 4: myMungData->blueMin--; break;
        case 5: myMungData->blueMax--; break;
        }
}


