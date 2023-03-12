/*
	File:        main.c
	
	Description: This example is based on the SGDataProcSamsple code which can be found at the following URL:
                 http://developer.apple.com/samplecode/SGDataProcSample/SGDataProcSample.html
                
                 While the code in SGDataProcSample used an offscreen GWorld and some QuickDraw
                 calls to draw text on top of video, this sample removes the need for the extra
                 GWorld by using Overlay windows and Core Graphics to draw some text and graphics
                 on top of the video image being captured.
                 
                 The graphs in the upper left of the window display the following stats:
                    Number of frames in the queue.
                    Number of skipped frames.
                    Difference between the time base time and the dataProc frame time.
                    Difference between the last frame time and the current frame time.
                
                For each of the above stats, shorter is better.
                
                The status string prints out the time, a min:seconds:frames representation of time, frames per second
                and average frames per second.
                
                The dataProc in this case will also drop frames under certain situations. Namely if there's more than
                one frame in the queue, the capture rate drops below 15fps and we haven't skipped 15 frames in a row
                trying to catch up. There's many different strategies one could use to drop less important frames or
                always display the latest frame captured and so on. These are left as an exercise for the developer.
                
	Author:      QuickTime Engineering, QuickTime DTS

	Copyright: 	 © Copyright 2000 - 2005 Apple Computer, Inc. All rights reserved.
    
	Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
                consideration of your agreement to the following terms, and your use, installation, modification 
                or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
                not agree with these terms, please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject to these terms, 
                Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
                original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
                Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
                redistribute the Apple Software in its entirety and without modifications, you must retain this 
                notice and the following text and disclaimers in all such redistributions of the Apple Software. 
                Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
                endorse or promote products derived from the Apple Software without specific prior written 
                permission from Apple.  Except as expressly stated in this notice, no other rights or 
                licenses, express or implied, are granted by Apple herein, including but not limited to any 
                patent rights that may be infringed by your derivative works or by other works in which the 
                Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
                IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
                AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
                OR IN COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
                DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
                OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
                REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
                UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
                IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
                
	Change History (most recent first): 04/01/05 fixed sgchanSound failure check
                                        01/25/05 initial release
*/

// build for carbon
#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

// defines
#define DisplayAndBail(x, y) {pMungData->err = x; if(x != noErr) { DisplayError(pMungData->pWindow, y, x); goto bail;}}

// constants
const EventTime kTimerInterval = kEventDurationSecond / 60;	// idle timer interval

const HIViewID kBoxID = {'GBOX', 100};

// mung data struct
typedef struct {
    WindowRef        	pWindow;	// windows
    WindowRef			pOverlay;
    CGContextRef		context;
    CGRect				textRect, barRect, offsetRect;
    CGPoint				textPos;
    CGRect				qfRect[25][25];
    CGRect				sfRect[25][25];
    Rect 				bounds;		// bounds rect
    SeqGrabComponent 	seqGrab;	// sequence grabber
    SGChannel			sgchanVideo;
    ImageSequence 	 	drawSeq;	// unique identifier for our draw sequence
    TimeScale 		 	timeScale;
    TimeBase			timeBase;
    UInt32				duration;
    UInt8 				queuedFrameCount;
    UInt8				skipFrameCount;
    TimeValue 		 	lastTime;
    long 			 	frameCount;
    Boolean			 	isGrabbing;
    Boolean				isAccelerated;
    EventLoopTimerRef	timerRef;
    OSErr			 	err;
} MungDataRecord, *MungDataPtr;

// globals
static BitMap gScreenbits;

#pragma mark-

OSErr SetupOverlayContext(MungDataPtr inMungData);
void UpdateStaticText(MungDataPtr inMungData);
OSErr MakeWindows(MungDataPtr inMungData, IBNibRef inNibRef);
OSErr MakeSequenceGrabber(MungDataPtr pMungData);
OSErr MakeSequenceGrabChannel(SeqGrabComponent seqGrab, SGChannel *sgchanVideo, SGChannel *sgchanSound);
ComponentResult SetVideoChannelBounds(SGChannel videoChannel, const Rect *scaledSourceBounds, const Rect *scaledVideoBounds);
void DisposeMungData(MungDataPtr inMungData);

#pragma mark-

/*****************************************************
*
* DisplayError(WindowRef inWindow, char inStr[], OSErr inError)
*
* Purpose:   display an error message as the window title
*
* Notes:     
*
*/
static void DisplayError(WindowRef inWindow, char inStr[], OSErr inError)
{
	// set the window title to display the error
	char errMsg[64];
	
	sprintf(errMsg, "%s: %d", inStr, inError);
	CopyCStringToPascal(errMsg, (unsigned char *)&errMsg);
	SetWTitle(inWindow, (unsigned char *)errMsg);
}

#pragma mark-

/*****************************************************
*
* MungGrabCompressCompleteBottleProc(SGChannel c, UInt8 *queuedFrameCount, SGCompressInfo *ci, TimeRecord *t, long refCon)
*
* Purpose:   used to allow us to figure out how many frames are queued by the vDig
*
* Notes:     the UInt8 *queuedFrameCount replaces Boolean *done. (0 (==false) still means no frames, and 1 (==true) one,
*            but if more than one are available, the number should be returned here - The value 2 previously meant more than
*            one frame, so some VDIGs may return 2 even if more than 2 are available, and some will still return 1 as they are
*            using the original definition.
*
*/
static pascal ComponentResult MungGrabCompressCompleteBottleProc(SGChannel c, UInt8 *queuedFrameCount, SGCompressInfo *ci, TimeRecord *t, long refCon)
{
	OSErr err;
	
	MungDataPtr pMungData = (MungDataPtr)refCon;
    if (NULL == pMungData) return -1;
    
    // call the original proc; you must do this
    err = SGGrabCompressComplete(c, queuedFrameCount, ci, t);
    
    // save the queued frame count so we have it
    pMungData->queuedFrameCount = *queuedFrameCount;
    
    return err;
}

/*****************************************************
*
* MungGrabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
*
* Purpose:   sequence grabber data procedure - this is where the work is done
*
* Notes:

   the sequence grabber calls the data function whenever
   any of the grabber’s channels write digitized data to the destination movie file.
   
   NOTE: We really mean any, if you have an audio and video channel then the DataProc will
   		 be called for either channel whenever data has been captured. Be sure to check which
   		 channel is being passed in. In this example we never create an audio channel so we know
   		 we're always dealing with video.
   
   This data function does two things, it first decompresses captured video
   data into an offscreen GWorld, draws some status information onto the frame then
   transfers the frame to an onscreen window.
   
   For more information refer to Inside Macintosh: QuickTime Components, page 5-120
   c - the channel component that is writing the digitized data.
   p - a pointer to the digitized data.
   len - the number of bytes of digitized data.
   offset - a pointer to a field that may specify where you are to write the digitized data,
   			and that is to receive a value indicating where you wrote the data.
   chRefCon - per channel reference constant specified using SGSetChannelRefCon.
   time	- the starting time of the data, in the channel’s time scale.
   writeType - the type of write operation being performed.
   		seqGrabWriteAppend - Append new data.
   		seqGrabWriteReserve - Do not write data. Instead, reserve space for the amount of data
   							  specified in the len parameter.
   		seqGrabWriteFill - Write data into the location specified by offset. Used to fill the space
   						   previously reserved with seqGrabWriteReserve. The Sequence Grabber may
   						   call the DataProc several times to fill a single reserved location.
   refCon - the reference constant you specified when you assigned your data function to the sequence grabber.
*/
static pascal OSErr MungGrabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
#pragma unused(offset,chRefCon,writeType)
    
    ComponentResult err = noErr;
    
    MungDataPtr pMungData = (MungDataPtr)refCon;
    if (NULL == pMungData) return -1;
    
	// reset frame and time counters after a stop/start
	if (time < pMungData->lastTime) {	
		pMungData->lastTime = 0;
		pMungData->frameCount = 0;
	}
    
    // we only care about the video	
    if (c == pMungData->sgchanVideo) {
    
    	TimeValue timeBaseTime, timeBaseDelta, frameTimeDelta;
    
	    pMungData->frameCount++;
	        
	    if (pMungData->timeScale == 0) {
	    	Fixed framesPerSecond;
	    	long  milliSecPerFrameIgnore, bytesPerSecondIgnore;
	    	
	    	// first time here so get the time scale & timebase
	    	err = SGGetChannelTimeScale(c, &pMungData->timeScale);
	    	DisplayAndBail(err, "SGGetChannelTimeScale");
	    	
	    	err = SGGetTimeBase(pMungData->seqGrab, &pMungData->timeBase);
	    	DisplayAndBail(err, "SGGetTimeBase");
	    	
	    	err = VDGetDataRate(SGGetVideoDigitizerComponent(c), &milliSecPerFrameIgnore, &framesPerSecond, &bytesPerSecondIgnore);
	    	DisplayAndBail(err, "VDGetDataRate");
	    	
	    	pMungData->duration = pMungData->timeScale / (framesPerSecond >> 16);
	    }
	    
   		if (pMungData->drawSeq == 0) {
   			
   			// set up decompression sequence	
    		Rect				   sourceRect = { 0, 0 };
            Rect                   scaleRect = pMungData->bounds;
			MatrixRecord		   scaleMatrix;	
			CodecFlags			   cFlags = codecNormalQuality;
			ImageDescriptionHandle imageDesc = (ImageDescriptionHandle)NewHandle(0);
            
            // retrieve a channel’s current sample description, the channel returns a
            // sample description that is appropriate to the type of data being captured
            err = SGGetChannelSampleDescription(c, (Handle)imageDesc);
            DisplayAndBail(err, "SGGetChannelSampleDescription");
            
            // make a scaling matrix for the sequence
			sourceRect.right = (**imageDesc).width;
			sourceRect.bottom = (**imageDesc).height;
			
			if (pMungData->isAccelerated) {
				// if accelerated and DV do high quality
				// 720x480 both fields
				if (kDVCNTSCCodecType == (**imageDesc).cType) {
					cFlags = codecHighQuality;
				}
                UnionRect(&sourceRect, &pMungData->bounds, &pMungData->bounds);
                UnionRect(&sourceRect, &pMungData->bounds, &scaleRect);
                OffsetRect(&scaleRect, short(pMungData->offsetRect.origin.x + 5), short(pMungData->offsetRect.origin.y + 5));
                pMungData->bounds.right += short(pMungData->offsetRect.size.width + 10);
                pMungData->bounds.bottom += short(pMungData->offsetRect.size.height + 10);
			} else {
                MacSetRect(&scaleRect, 0, 0, 320, 240);
                OffsetRect(&scaleRect, short(pMungData->offsetRect.origin.x + 5), short(pMungData->offsetRect.origin.y + 5));
            }
            
            RectMatrix(&scaleMatrix, &sourceRect, &scaleRect);
            			
			SetupOverlayContext(pMungData);
			
            // begin the process of decompressing a sequence of frames
            // this is a set-up call and is only called once for the sequence - the ICM will interrogate different codecs
            // and construct a suitable decompression chain, as this is a time consuming process we don't want to do this
            // once per frame (eg. by using DecompressImage)
            // for more information see Ice Floe #8 http://developer.apple.com/quicktime/icefloe/dispatch008.html
            // the destination is specified as the GWorld
			err = DecompressSequenceBeginS(&pMungData->drawSeq,					// pointer to field to receive unique ID for sequence
										  imageDesc,							// handle to image description structure
										  p,									// points to the compressed image data
										  len,                             		// size of the data buffer
										  GetWindowPort(pMungData->pWindow),	// port for the DESTINATION image
										  NULL,									// graphics device handle, if port is set, set to NULL
										  NULL,									// decompress the entire source image - no source extraction
                                          &scaleMatrix,							// transformation matrix
                                          srcCopy,								// transfer mode specifier
                                          (RgnHandle)NULL,						// clipping region in dest. coordinate system to use as a mask
                                          0,									// flags
                                          cFlags, 								// accuracy in decompression
                                          bestSpeedCodec);						// compressor identifier or special identifiers ie. bestSpeedCodec
    
            DisplayAndBail(err, "DSeqBegin");
            
            DisposeHandle((Handle)imageDesc);
        }
        
        // get the TimeBase time and figure out the delta between that time and this frame time
        timeBaseTime = GetTimeBaseTime(pMungData->timeBase, pMungData->timeScale, NULL);
        timeBaseDelta = timeBaseTime - time;
        frameTimeDelta = time - pMungData->lastTime;
        
        if (timeBaseDelta < 0) goto bail; // probably don't need this
        
        // if we have more than one queued frame and our capture rate drops below 10 frames, skip the frame to try and catch up
		if ((pMungData->queuedFrameCount > 1) &&  ((pMungData->timeScale / frameTimeDelta) < 10) && (pMungData->skipFrameCount < 15)) {
			++pMungData->skipFrameCount;
			--pMungData->frameCount;
        } else {
        	CodecFlags ignore;
        	
	        // decompress a frame into the window - can queue a frame for async decompression when passed in a completion proc
	        err = DecompressSequenceFrameS(pMungData->drawSeq,	// sequence ID returned by DecompressSequenceBegin
	        							   p,					// pointer to compressed image data
	        							   len,					// size of the buffer
	        							   0,					// in flags
	        							   &ignore,				// out flags
	        							   NULL);				// async completion proc
	        					   
	        DisplayAndBail(err, "DSeqFrameS");
	        
	       	pMungData->skipFrameCount = 0;
	       	pMungData->lastTime = time;
	    } 

		// status information -  use CG to draw
       	char 	status[64];
  		float	fps, averagefps;
  		UInt8   minutes, seconds, frames, index;
  		CGPoint	thePos;
  		CGRect  theRect;
  		
  		fps = (float)pMungData->timeScale / (float)frameTimeDelta;
       	averagefps = ((float)pMungData->frameCount * (float)pMungData->timeScale) / (float)time;
       	minutes = (time / pMungData->timeScale) / 60;
       	seconds = (time / pMungData->timeScale) % 60;
       	frames = (time % pMungData->timeScale) / frameTimeDelta; //pMungData->duration;
       	sprintf(status, "t: %ld, %02d:%02d.%02d, fps:%5.1f av:%5.1f", time, minutes, seconds, frames, fps, averagefps);

      	CGContextClearRect(pMungData->context, pMungData->barRect);
      	CGContextClearRect(pMungData->context, pMungData->textRect);
		CGContextBeginPath(pMungData->context);
		
		// number of frames in the queue
		index = (pMungData->queuedFrameCount <= 25 ? pMungData->queuedFrameCount : 25);
		CGContextAddRects(pMungData->context, pMungData->qfRect[index-1], index);
		
		// number of skipped frames
		index = (pMungData->skipFrameCount <= 25 ? pMungData->skipFrameCount : 25);
		CGContextAddRects(pMungData->context, pMungData->sfRect[index-1], index);
		
		// difference between the timeBaseTime and the this frame time
		theRect = CGRectMake(1, 15, 6 * timeBaseDelta / pMungData->duration, 3);
		CGContextAddRect(pMungData->context, theRect);
		
		// difference between the last drawn frame and this frame time
		theRect = CGRectMake(1, 20, 6 * frameTimeDelta / pMungData->duration, 3);
		CGContextAddRect(pMungData->context, theRect);

		// draw the graph
		pMungData->barRect = CGContextGetPathBoundingBox(pMungData->context);
		CGContextClosePath(pMungData->context);
        CGContextSetRGBFillColor(pMungData->context, 1.0, 0.0, 0.0, 1.0 );
		CGContextFillPath(pMungData->context);
		
		// text setup
       	CGContextSetTextDrawingMode(pMungData->context, kCGTextInvisible);
		CGContextShowTextAtPoint(pMungData->context, pMungData->textPos.x, pMungData->textPos.y, status, strlen(status));
		thePos = CGContextGetTextPosition(pMungData->context);
		pMungData->textRect.size.width = ((int)(thePos.x + .5) + 2.0) - pMungData->textRect.origin.x;
		
		// draw the background rect
		CGContextSetRGBFillColor(pMungData->context, 1.0, 0.0, 0.0, 0.2 ); 
		CGContextFillRect(pMungData->context, pMungData->textRect);
		
		// draw the status string
		CGContextSetTextDrawingMode(pMungData->context, kCGTextFill);
	    CGContextSetRGBFillColor(pMungData->context, 0.0, 1.0, 0.0, 1.0);
	    CGContextShowTextAtPoint(pMungData->context, pMungData->textPos.x, pMungData->textPos.y, status, strlen(status));
		
        // mark the context for update
	    CGContextSynchronize(pMungData->context);
	}
            
bail:
	return err;
}

#pragma mark-
/*****************************************************
*
* MGWindowEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void *inUserData)
*
* Purpose:   idle timer to idle the sequence grabber
*
* Notes:     timer should call this at least as much as the desired frame rate more if
*            capturing audio as well, we use 1/60th of a second
*
*/
static pascal void MGIdleTimer(EventLoopTimerRef inTimer, void *inUserData)
{
#pragma unused(inTimer)
	
	OSErr err = noErr;
	
	MungDataPtr pMungData = MungDataPtr(inUserData);
	if (NULL == pMungData) return;
	
	if (pMungData->isGrabbing) err = SGIdle(pMungData->seqGrab);
	if (err && err != pMungData->err) {
			// some error specific to SGIdle occurred - any errors returned from the
			// data proc will also show up here and we don't want to write over them
			
			// in QT 4 you would always encounter a cDepthErr error after a user drags
			// the window, this failure condition has been greatly relaxed in QT 5
			// it may still occur but should only apply to vDigs that really control
			// the screen
			
			// you don't always know where these errors originate from, some may come
			// from the VDig...
			
			DisplayError(pMungData->pWindow, "SGIdle", err);
			
			// ...to fix this we simply call SGStop and SGStartRecord again
			// calling stop allows the SG to release and re-prepare for grabbing
			// hopefully fixing any problems, this is obviously a very relaxed
			// approach
			SGStop(pMungData->seqGrab);
			SGStartRecord(pMungData->seqGrab);
	}
}

/*****************************************************
*
* MGWindowEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void *inUserData)
*
* Purpose:  window event handler
*
* Notes:
*
*/
static pascal OSStatus MGWindowEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void *inUserData)
{
	WindowRef theWindow;
    OSErr err;
	
	OSStatus status = eventNotHandledErr;
	
	MungDataPtr pMungData = MungDataPtr(inUserData);
	if (NULL == pMungData) goto Done;
	
	UInt32 eventKind;
	eventKind = GetEventKind(theEvent);
	
	// we need the window ref or bail
	err = GetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(theWindow), NULL, &theWindow);
    require_noerr(err, Done);
		
	switch (eventKind) {
	case kEventWindowHiding:
		// fall through, same action as kEventWindowCollapsed
        
	case kEventWindowCollapsed:
		// checking this here avoids passible codecNothingToBlitErr later
      	if (pMungData->isGrabbing) {
        	SGStop(pMungData->seqGrab);
        	SetEventLoopTimerNextFireTime(pMungData->timerRef, kEventDurationForever);
        	pMungData->isGrabbing = false;
        }
        
        break;
	case kEventWindowShown:
        // fall through
        
	case kEventWindowExpanded:
        CallNextEventHandler(nextHandler, theEvent);
        
        if (kEventWindowShown == eventKind) {
            ActivateWindow(theWindow, true);
            UpdateStaticText(pMungData);
        }
        
		// we stopped grabbing in the collapsed event so start grabbing on expanded
		if (!pMungData->isGrabbing) {
        	SGStartRecord(pMungData->seqGrab);
        	SetEventLoopTimerNextFireTime(pMungData->timerRef, kEventDurationNoWait);
        	pMungData->isGrabbing = true;
       	}
        
        status = noErr; // we handled the event
		
        break;
	case kEventWindowClickDragRgn:
		Point where;
		ICMAlignmentProcRecord apr;
        
        // we need the 'where' param from the Event for DragAlignedWindow
        err = GetEventParameter(theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(where), NULL, &where);
        require_noerr(err, Done);
        
        SGGetAlignmentProc(pMungData->seqGrab, &apr);

        DragAlignedWindow(theWindow, where, &gScreenbits.bounds, NULL, &apr);
        
        status = noErr; // we handled the event
        
		break;
	case kEventWindowClose:
    {
        // we're done
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        AppleEvent quitEvent;
        AEDesc target;
        
        CallNextEventHandler(nextHandler, theEvent);
        
        HideWindow(pMungData->pOverlay);
        
        // send a Quit apple event to ourselves
        AECreateDesc(typeProcessSerialNumber, (Ptr)&psn, sizeof(ProcessSerialNumber), &target);
        AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &target, kAutoGenerateReturnID, kAnyTransactionID, &quitEvent);
        AESend(&quitEvent, NULL, kAENoReply, kAENormalPriority, kNoTimeOut, NULL, NULL);
        AEDisposeDesc(&quitEvent);
        AEDisposeDesc(&target);
        
        status = noErr; // we handled the event
    }
		break;
	default:
		break;
	}

Done:
	return status;
}

/*****************************************************
*
* HandleQuitAE(const AppleEvent *theAppleEvent, AppleEvent *reply, long inRefcon)
*
* Purpose:  handle the Quit AppleEvent
*
* Notes:
*
*/
static pascal OSErr HandleQuitAE(const AppleEvent *theAppleEvent, AppleEvent *reply, long inRefcon)
{
#pragma unused (theAppleEvent, reply)

	MungDataPtr pMungData = MungDataPtr(inRefcon);
	
	if (pMungData) {
        // we're done
		SGStop(pMungData->seqGrab);
		pMungData->isGrabbing = false;

		RemoveEventLoopTimer(pMungData->timerRef);
        
        SGRelease(pMungData->seqGrab);
    }
	
	QuitApplicationEventLoop();
	
	return noErr;
}

/*****************************************************
*
* InstallEvenHandlers(MungDataPtr inMungData)
*
* Purpose:  intalls event handlers and carbon timer
*
* Notes:
*
*/
static OSErr InstallEvenHandlers(MungDataPtr inMungData)
{
	const EventTypeSpec windowEventList[] = { kEventClassWindow, kEventWindowCollapsed,
	 										  kEventClassWindow, kEventWindowExpanded,
	 										  kEventClassWindow, kEventWindowClickDragRgn,
	 										  kEventClassWindow, kEventWindowClose,
	 										  kEventClassWindow, kEventWindowShown,
	 										  kEventClassWindow, kEventWindowHiding };

	OSStatus err;
    
    err = InstallEventLoopTimer(GetMainEventLoop(), kEventDurationNoWait, kTimerInterval, MGIdleTimer, inMungData, &inMungData->timerRef);
    require_noerr(err, CantInstallEventLoopTimer);
    	
	err = InstallWindowEventHandler(inMungData->pWindow, NewEventHandlerUPP(MGWindowEventHandler), 7, windowEventList, inMungData, NULL);
    require_noerr(err, CantInstallWindowEventHandler);
	
	err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(HandleQuitAE), (long)inMungData, false);

CantInstallEventLoopTimer:
CantInstallWindowEventHandler:
	return err;	
}

#pragma mark-
/*****************************************************
*
* SetupOverlayContext(MungDataPtr inMungData)
*
* Purpose:  creates and sets up the CGContext for our overlay window
*           this is where the text is rendered by the DataProc
*
* Notes:
*
*/
OSErr SetupOverlayContext(MungDataPtr inMungData)
{
	Rect  portRect;
	OSErr err = noErr;
	
    SizeWindow(inMungData->pWindow, inMungData->bounds.right, inMungData->bounds.bottom, false);
    SizeWindow(inMungData->pOverlay, inMungData->bounds.right, inMungData->bounds.bottom, false);
    
    GetPortBounds(GetWindowPort(inMungData->pWindow), &inMungData->bounds);
    
    // create a context for the overlay window and transform to QD coordinates
	err = CreateCGContextForPort(GetWindowPort(inMungData->pOverlay), &inMungData->context);
    require_noerr(err, CantCreateContext);
    
	GetPortBounds(GetWindowPort(inMungData->pOverlay), &portRect);
	SyncCGContextOriginWithPort(inMungData->context, GetWindowPort(inMungData->pOverlay));    
    CGContextTranslateCTM(inMungData->context, 0, (float)(portRect.bottom - portRect.top));
    CGContextScaleCTM(inMungData->context, 1.0, -1.0);

    CGContextClearRect(inMungData->context, CGRectMake(0,
                                                       0,
                                                       (float)(portRect.right - portRect.left),
                                                       (float)(portRect.bottom - portRect.top)));
	
	CGContextSetTextMatrix(inMungData->context, CGContextGetCTM(inMungData->context));
	CGContextSelectFont(inMungData->context, "Helvetica", 10.0, kCGEncodingMacRoman);
	CGContextSetLineWidth(inMungData->context, 1.0);

	// size of the semi-transparent text background rect we draw
    inMungData->textRect = CGRectMake(inMungData->offsetRect.origin.x + 8,
									  ((float)(portRect.bottom) - inMungData->offsetRect.size.height),
									  (float)portRect.right, // this is adjusted during draw							
									  (float)18);
    // position of text we draw
	inMungData->textPos = CGPointMake(inMungData->offsetRect.origin.x + 10,
                                      ((float)(portRect.bottom) - inMungData->offsetRect.size.height + 13));

	UpdateStaticText(inMungData);

CantCreateContext:
	return err;
}

/*****************************************************
*
* UpdateStaticText(MungDataPtr inMungData)
*
* Purpose:  update the static text when needed
*
* Notes:
*
*/
void UpdateStaticText(MungDataPtr inMungData)
{
	char *yes = "Quartz: Acclerated";
	char *nope = "Quartz: Not Accelerated";
		
	CGContextSetTextDrawingMode(inMungData->context, kCGTextFill);
	CGContextSetRGBFillColor(inMungData->context, 1.0, 0.0, 0.0, 1.0);
	if (inMungData->isAccelerated) {
		CGContextShowTextAtPoint(inMungData->context, inMungData->textPos.x, inMungData->textPos.y - 15, yes, strlen(yes));
	} else {
		CGContextShowTextAtPoint(inMungData->context, inMungData->textPos.x, inMungData->textPos.y - 15, nope, strlen(nope));
	}
}

#pragma mark-
/*****************************************************
*
* MakeWindows(MungDataPtr inMungData, IBNibRef inNibRef)
*
* Purpose:  create main and overlay windows
*
* Notes:
*
*/
OSErr MakeWindows(MungDataPtr inMungData, IBNibRef inNibRef)
{
    Rect windowRect = {0, 0, 240, 320};
    Rect bestRect;
    HIViewRef control;
    WindowGroupRef windowGroup;
    WindowAttributes oAttributes = kWindowNoShadowAttribute |
    							   kWindowIgnoreClicksAttribute | kWindowNoActivatesAttribute;
                                   
	OSErr err = noErr;
    
    // Create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(inNibRef, CFSTR("MainWindow"), &inMungData->pWindow);
    require_noerr( err, CantCreateWindow);
    
    GetWindowBounds(inMungData->pWindow, kWindowContentRgn, &windowRect);
    
    // create the overlay window
	err = CreateNewWindow(kOverlayWindowClass, oAttributes, &windowRect, &inMungData->pOverlay);
    require_noerr(err, CantCreateWindow);
	
    // create the window group and group the two windows together
	CreateWindowGroup(kWindowGroupAttrMoveTogether | 
	                  kWindowGroupAttrLayerTogether |
	                  kWindowGroupAttrSharedActivation |
	                  kWindowGroupAttrHideOnCollapse, &windowGroup);
	                  
	SetWindowGroupParent(windowGroup, GetWindowGroup(inMungData->pWindow));
	SetWindowGroup(inMungData->pWindow, windowGroup);
	SetWindowGroup(inMungData->pOverlay, windowGroup);

    // set the port to the new window
    SetPortWindowPort(inMungData->pWindow);
    
    GetPortBounds(GetWindowPort(inMungData->pWindow), &inMungData->bounds);
   	AlignWindow(inMungData->pWindow, true, NULL, NULL);
    
    // get the frame size of the group box, we draw the video at this offset
    HIViewFindByID(HIViewGetRoot(inMungData->pWindow), kBoxID, &control);
    HIViewGetFrame(control, &inMungData->offsetRect);
    inMungData->offsetRect.size.width = inMungData->bounds.right - inMungData->offsetRect.size.width;
    inMungData->offsetRect.size.height = inMungData->bounds.bottom - inMungData->offsetRect.size.height;
    
	ShowWindow(inMungData->pWindow);
	ShowWindow(inMungData->pOverlay);
   	
   	inMungData->isAccelerated = CGDisplayUsesOpenGLAcceleration(kCGDirectMainDisplay);
    
CantCreateWindow:
    return err;
}

/*****************************************************
*
* MakeSequenceGrabber(MungDataPtr inMungData)
*
* Purpose:  open and configure the sequence grabber
*
* Notes:
*
*/
OSErr MakeSequenceGrabber(MungDataPtr inMungData)
{
	OSErr err = couldntGetRequiredComponent;

    // open the default sequence grabber
    inMungData->seqGrab = OpenDefaultComponent(SeqGrabComponentType, 0);
    require(inMungData->seqGrab != NULL, CantGetSG);
    
    // initialize the default sequence grabber component
    err = SGInitialize(inMungData->seqGrab);
    require_noerr(err, Bail);

    // set its graphics world to the specified window
    err = SGSetGWorld(inMungData->seqGrab, GetWindowPort(inMungData->pWindow), NULL);
    require_noerr(err, Bail);
    	
    // specify the destination data reference for a record operation
    // tell it we're not making a movie
    // if the flag seqGrabDontMakeMovie is used, the sequence grabber still calls
    // your data function, but does not write any data to the movie file
    // writeType will always be set to seqGrabWriteAppend
    err = SGSetDataRef(inMungData->seqGrab,
                       0,
                       0,
                       seqGrabDontMakeMovie | seqGrabDataProcIsInterruptSafe);

Bail:
    if (err && (inMungData->seqGrab != NULL)) { // clean up on failure
    	CloseComponent(inMungData->seqGrab);
        inMungData->seqGrab = NULL;
    }
    
CantGetSG:
	return err;
}

/*****************************************************
*
* MakeSequenceGrabChannel(SeqGrabComponent seqGrab, SGChannel *sgchanVideo, SGChannel *sgchanSound)
*
* Purpose:  create the channels
*
* Notes:
*
*/
OSErr MakeSequenceGrabChannel(SeqGrabComponent seqGrab, SGChannel *sgchanVideo, SGChannel *sgchanSound)
{
	Rect  srcBounds;
	
    OSErr err = noErr;
    
    err = SGNewChannel(seqGrab, VideoMediaType, sgchanVideo);
    if (err == noErr) {
        err = SGNewChannel(seqGrab, SoundMediaType, sgchanSound);
        if (err) {
            // don't care if we couldn't get a sound channel
            *sgchanSound = NULL;
            err = noErr;
        }
    }
        
    if (err == noErr) {
    	// get the active rectangle 
    	err = SGGetSrcVideoBounds(*sgchanVideo, &srcBounds);
    	if (err == noErr) {
    		// we always want all the source
            err = SetVideoChannelBounds(*sgchanVideo, &srcBounds, &srcBounds);
		    if (err == noErr) {
		    	// set usage for new video channel to avoid playthrough
		   		// note we don't set seqGrabPlayDuringRecord
		    	err = SGSetChannelUsage(*sgchanVideo, seqGrabRecord |
		    										  seqGrabLowLatencyCapture |
		    										  seqGrabAlwaysUseTimeBase);
                if (err == noErr && NULL != *sgchanSound) {
                    err = SGSetChannelUsage(*sgchanSound, seqGrabRecord |
                                                          //seqGrabPlayDuringRecord |
                                                          seqGrabLowLatencyCapture |
                                                          seqGrabAlwaysUseTimeBase);
                }
            }
		}
        
        if (err != noErr) {
            // clean up on failure
            SGDisposeChannel(seqGrab, *sgchanVideo);
            if (NULL != *sgchanSound) {
                SGDisposeChannel(seqGrab, *sgchanSound);
            }
        }
    }

	return err;
}

/*****************************************************
*
* SetVideoChannelBounds(SGChannel videoChannel, const Rect *scaledSourceBounds, const Rect *scaledVideoBounds)
*
* Purpose:  sets the desired video rect and video channel bounds
*
* Notes:    see Q&A 1250
*
*/
ComponentResult SetVideoChannelBounds(SGChannel videoChannel, const Rect *scaledSourceBounds, const Rect *scaledVideoBounds)
{
    Rect sourceBounds;
    Rect videoBounds;
    Rect channelBounds;
    MatrixRecord scaledSourceBoundsToSourceBounds;
    ComponentResult err;
    
    // calculate the matrix to transform the
    // scaledSourceBounds to the source bounds
    SGGetSrcVideoBounds(videoChannel, &sourceBounds);   
    RectMatrix(&scaledSourceBoundsToSourceBounds,
               scaledSourceBounds, &sourceBounds);
    
    // apply the same transform to the
    // scaledVideoBounds to get the video bounds
    videoBounds = *scaledVideoBounds;
    TransformRect(&scaledSourceBoundsToSourceBounds, &videoBounds, 0);
    
    err = SGSetVideoRect(videoChannel, &videoBounds);
    if (err) {
    	// some video digitizers may only be able to capture full frame
    	// and will return qtParamErr or possibly digiUnimpErr
    	// if they can't handle working with less than full frame
    	SGSetVideoRect(videoChannel, &sourceBounds);
    }

    // the channel bounds is scaledVideoBounds offset to (0, 0)
    channelBounds = *scaledVideoBounds;
    OffsetRect(&channelBounds, -channelBounds.left, -channelBounds.top);
   
    // Note: SGSetChannelBounds merely allows the client to
    // specify it's preferred bounds. The actual bounds returned
    // by the vDig in the image description may be different
    err = SGSetChannelBounds(videoChannel, &channelBounds);

    return err;
}

/*****************************************************
*
* DisposeMungData(MungDataPtr inMungData)
*
* Purpose:  end the decompression sequence, close the sequence grabber and
*           dispose of our data pointer
*
* Notes:   
*
*/
void DisposeMungData(MungDataPtr inMungData)
{
    // clean up the bits
    if(inMungData) {
        if (inMungData->drawSeq)
        	CDSequenceEnd(inMungData->drawSeq);
        if (inMungData->seqGrab)
    		CloseComponent(inMungData->seqGrab);
        DisposePtr((Ptr)inMungData);
    }
}

#pragma mark-
/*****************************************************
*
* main(argc, argv) 
*
* Purpose:  main program entry point
*
* Notes:    Penguins make up the scientific order Sphenisciformes and the family  Spheniscidae.
*
*/
int main(int argc, char* argv[])
{
    IBNibRef 	 nibRef;
    MungDataPtr  pMungData = NULL;
    SGChannel 	 sgchanSound = NULL;
    VideoBottles vb = { 0 };
    
    OSStatus err;

	InitCursor();
    EnterMovies();
    
    GetQDGlobalsScreenBits(&gScreenbits);
    
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr(err, CantGetNibRef);
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr(err, CantSetMenuBar);
    
    // initialize the data that's going to be passed around
    pMungData = (MungDataPtr)NewPtrClear(sizeof(MungDataRecord));
    err = MemError();
    require_noerr(err, CantCreateMungData);
    
    // create 25 rectangles which will represent our queued frames and skipped frames display
    for (UInt8 i=0; i < 25; ++i) {
    	for (UInt8 x=0; x <= i; ++x) {
	  		pMungData->qfRect[i][x] = CGRectMake(8*x+1, 5, 6, 3);
	  		pMungData->sfRect[i][x] = CGRectMake(8*x+1, 10, 6, 3);
		}
	}
    
    // create the main and overlay windows and group them together
    err = MakeWindows(pMungData, nibRef);
    require_noerr(err, CantCreateWindow);
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);    
    
    // create and initialize the sequence grabber
    err = MakeSequenceGrabber(pMungData);
    require_noerr(err, CantCreateSequenceGrabber);

    // create the channels
    err = MakeSequenceGrabChannel(pMungData->seqGrab, &pMungData->sgchanVideo, &sgchanSound);
    require_noerr(err, CantCreateVideoChannel);
    
    // specify a sequence grabber data function
	err = SGSetDataProc(pMungData->seqGrab, NewSGDataUPP(MungGrabDataProc), (long)pMungData);
    require_noerr(err, CantSetDataProc);
    
	SGSetChannelRefCon(pMungData->sgchanVideo, (long)pMungData);
	
    // set up the video bottlenecks so we can get our queued frame count
	err = SGGetVideoBottlenecks(pMungData->sgchanVideo, &vb);
    require_noerr(err, CantGetBottlenecks);
    
    vb.procCount = 9; // there are 9 bottleneck procs; this must be filled in
    vb.grabCompressCompleteProc = NewSGGrabCompressCompleteBottleUPP(MungGrabCompressCompleteBottleProc);
	
    err = SGSetVideoBottlenecks(pMungData->sgchanVideo, &vb);
    require_noerr(err, CantSetBottlenecks);
	
    if (pMungData->isAccelerated) {
        SGSetFrameRate(pMungData->sgchanVideo, FixRatio(30, 1));
    } else {
        SGSetFrameRate(pMungData->sgchanVideo, FixRatio(15, 1));
    }
		
	// install carbon event handlers
	err = InstallEvenHandlers(pMungData);
	require_noerr(err, CantInstallEventHandlers);
	
	// lights...camera...
	err = SGPrepare(pMungData->seqGrab, false, true);
    require_noerr(err, PrepareFailed);
    
    // make sure the timebase used by the video channel is being driven by
    // the sound clock if there is a sound channel, this has to be done
    // after calling SGPrepare - see Q&A 1314
    if (NULL != sgchanSound) {
		TimeBase soundTimeBase = NULL, sgTimeBase = NULL;
		err = SGGetTimeBase(pMungData->seqGrab, &sgTimeBase);
		if(noErr == err)
		     err = SGGetChannelTimeBase(sgchanSound, &soundTimeBase);
		if (noErr == err && NULL != soundTimeBase)
		    SetTimeBaseMasterClock(sgTimeBase, (Component)GetTimeBaseMasterClock(soundTimeBase), NULL);
	}
	
    // ...action
	err = SGStartRecord(pMungData->seqGrab);
    require_noerr(err, StartRecordFailed);
	
	pMungData->isGrabbing = true;
	
	// run the application
	RunApplicationEventLoop();

CantCreateMungData:
CantCreateSequenceGrabber:
CantCreateVideoChannel:
CantSetDataProc:
CantGetBottlenecks:
CantSetBottlenecks:
CantInstallEventHandlers:
PrepareFailed:
StartRecordFailed:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:

    // clean up 
    if (pMungData->context)
        CGContextRelease(pMungData->context);  

    if (pMungData->pOverlay)
        DisposeWindow(pMungData->pOverlay);
        
    if (pMungData->pWindow)
        DisposeWindow(pMungData->pWindow);

    DisposeMungData(pMungData);

    return err;
}

