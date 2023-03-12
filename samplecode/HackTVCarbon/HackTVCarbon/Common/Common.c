/*
	File:		Common.c
	
	Description: HackTV cross-platform common code
				 Refer to develop Issue 13, "Video Digitizing Under QuickTime",
				 for details on this code.

	Author:		QuickTime Engineering, dts

	Copyright: 	© Copyright 1992-2004 Apple Computer, Inc. All rights reserved.
	
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

	Revision 1.2	02/03/2004	dts added ability to turn on seqGrabLowLatencyCapture and seqGrabAlwaysUseTimeBase flags
								    also added the ability to use the sound clock to drive the SG timebase if desired
	Revision 1.11	04/10/2001	dts updated for UI3.4 and X
					2001/02/13	Change the window title to 'preallocating' while calling
								SGStartRecord so the user will know that the allocation is going on
	Revision 1.10  	2001/02/13	will now adjust the current bounds when resizing the window --
								now DV previews in full screen
	Revision 1.9  	2000/11/20	Fix recording wihtout sitting in a Button() loop with Nav services
								enabled; add more error reporting via the Monitor window title.
	Revision 1.7	2000/11/20	Fix the recording state in async record case
	Revision 1.6  	2000/03/06	Added #defines to facilitate testing SGOutput APIs
	Revision 1.5  	2000/03/01 	add more sized, recording on idle
	Revision 1.4  	2000/02/25 	menu stuff
	Revision 1.3  	1999/12/15	carbonized
	Revision 1.2  	Original	QTE
*/
	
#if (TARGET_OS_MAC && TARGET_API_MAC_CARBON)
	#if __APPLE_CC__
		#include <Carbon/Carbon.h>
	#elif __MWERKS__
		#include <Carbon.h>
	#else
		#error "Sorry Dave, I can't do that."
	#endif	
#else
	#include <QTML.h>
	#include <Endian.h>
	#include <Menus.h>
	#include <Printing.h>
	#include <Script.h>
	#include <Scrap.h>
	#include <QuickTimeComponents.h>
	#include <NumberFormatting.h>
#endif

#include <stdio.h>
#include <string.h>

#include "Globals.h"
#include "Common.h"

#if TARGET_OS_MAC
	#define USE_NAV_SERVICES 1
#else
	#define USE_NAV_SERVICES 0
#endif

#if (USE_NAV_SERVICES && !TARGET_API_MAC_CARBON)
	#include <Navigation.h>
#endif

#define SET_DATA_OUTPUT_MAX_OFFSET 0	// Set to 1 to call SGSetOutputMaximumOffset()
#define LINK_MULTIPLE_OUTPUTS 0			// Set to 1 to link multiple data outputs for split track files
#define CREATE_DATA_OUTPUTS 0			// Set to 1 to create data output for non-split movie.
#define SHOW_SPACE_REMAINING 0			// Set to 1 to call SGGetDataOutputStorageSpaceRemaining64()

// Disable warnings associated with "\p" strings
#if TARGET_OS_WIN32
	#pragma warning(disable: 4129)
#endif

static OSErr XorRectToRgn(Rect *srcRectA, Rect *srcRectB, RgnHandle *destRgn);
/* ---------------------------------------------------------------------- */

void InitializeSequenceGrabber(void)
{
	ComponentDescription	theDesc;
	ComponentResult			result = noErr;
	GrafPtr					savedPort;
	Component				sgCompID;
	
	gQuitFlag = false;
	gSeqGrabber = 0L;
	gVideoChannel = 0L;
	gSoundChannel = 0L;
	gMonitorPICT = NULL;
#if __MWERKS__
	gPrintRec = (THPrint) NewHandleClear (sizeof (TPrint));
#endif
	gCurrentlyRecording = false;
	
	// Find and open a sequence grabber
	theDesc.componentType = SeqGrabComponentType;
	theDesc.componentSubType = 0L;
	theDesc.componentManufacturer = 'appl';
	theDesc.componentFlags = 0L;
	theDesc.componentFlagsMask = 0L;	
	sgCompID = FindNextComponent (NULL, &theDesc);
	if (sgCompID != 0L)
		gSeqGrabber = OpenComponent (sgCompID);
	
	// If we got a sequence grabber, set it up
	if (gSeqGrabber != 0L)
	{
		// Get the monitor
		CreateMonitorWindow();
		if (gMonitor != NULL)
		{
			// Display the monitor window
			GetPort (&savedPort);
			MacSetPort ((GrafPtr)GetWindowPort(gMonitor));
			MacMoveWindow(gMonitor, 10, 30 + GetMBarHeight(), 0);
			MacShowWindow (gMonitor);		

			// Initialize the sequence grabber
			result = SGInitialize (gSeqGrabber);
			CheckError(result,"SGInitialize");
			if (result == noErr)
			{
				result = SGSetGWorld (gSeqGrabber, GetWindowPort(gMonitor), NULL);
				CheckError(result,"SGSetGWorld");
				
				// Get a video channel
				result = SGNewChannel (gSeqGrabber, VideoMediaType, &gVideoChannel);
				CheckError(result,"SGNewChannel for video");
				if ((gVideoChannel != NULL) && (result == noErr))
				{
					short	width;
					short	height;
					
					gQuarterSize = false;
					gHalfSize = true;
					gFullSize = false;
					
					result = SGGetSrcVideoBounds (gVideoChannel, &gActiveVideoRect);
					CheckError(result,"SGGetSrcVideoBounds");
					width = (gActiveVideoRect.right - gActiveVideoRect.left) / 2;
					height = (gActiveVideoRect.bottom - gActiveVideoRect.top) / 2;
					SizeWindow (gMonitor, width, height, false);
					
					result = SGSetChannelUsage (gVideoChannel, seqGrabPreview | seqGrabRecord | seqGrabPlayDuringRecord);
					CheckError(result,"SGSetChannelUsage for vid");
				#if TARGET_API_MAC_CARBON
					{
						Rect portRect;
					
						GetPortBounds(GetWindowPort(gMonitor), &portRect);
					
						result = SGSetChannelBounds (gVideoChannel, &portRect);
					}
				#else	
					result = SGSetChannelBounds (gVideoChannel, &(gMonitor->portRect));
				#endif
					CheckError(result,"SGSetChannelBounds");
				}
				
				// Get a sound channel
				result = SGNewChannel (gSeqGrabber, SoundMediaType, &gSoundChannel);
				CheckError(result,"SGNewChannel for sound");

				if ((gSoundChannel != NULL) && (result == noErr))
				{
					if (gSoundChannel != NULL)
					{
						Handle sampleRates = NULL;
						result = SGSetChannelUsage (gSoundChannel, seqGrabPreview | seqGrabRecord);
						CheckError(result,"SGSetChannelUsage for sound");
						
						// Set the volume low to prevent feedback when we start the preview,
						// in case the mic is anywhere near the speaker.
						result = SGSetChannelVolume (gSoundChannel, 0x0010);
						CheckError(result,"SGSetChannelVolume ");
						//result = SGSetSoundRecordChunkSize(gSoundChannel, -((Fixed)(.25*65536.0)));
						sampleRates = NewHandleClear(5*sizeof(Fixed));
						if(sampleRates) {
							OSErr tempErr;
							*(long*)(*sampleRates) = 8000<<16; // add 8kHz rate
							*((long*)(*sampleRates)+1) = 11025<<16; // add 11kHz rate
							*((long*)(*sampleRates)+2) = 16000<<16; // add 16kHz rate
							*((long*)(*sampleRates)+3) = 22050<<16; // add 22kHz rate
							*((long*)(*sampleRates)+4) = 32000<<16; // add 32kHz rate
							tempErr = SGSetAdditionalSoundRates(gSoundChannel,sampleRates);
							CheckError(result,"SGSetAdditionalSoundRates ");
							DisposeHandle(sampleRates);
						}	
					}
				}
				
				// Get the alignment proc (for use when dragging the monitor)
				result = SGGetAlignmentProc (gSeqGrabber, &gSeqGrabberAlignProc);
				CheckError(result,"SGGetAlignmentProc ");
			}
			
			// Go!
			if (result == noErr) {
				result = SGStartPreview (gSeqGrabber);
				CheckError(result,"SGStartPreview ");
			}
			MacSetPort (savedPort);
		}
	}
}

/* ---------------------------------------------------------------------- */

// Specify and setup a file to contain this track's data
static ComponentResult SetTrackFile(SGChannel theChannel, StringPtr prompt, StringPtr defaultName)
{
	FSSpec theFile;
#if USE_NAV_SERVICES
#pragma unused(prompt, defaultName)

	NavReplyRecord reply;
	NavDialogOptions myOptions;
	AEKeyword myKeyword;
	DescType myActualType;
	Size myActualSize;
#else
	StandardFileReply		reply;
#endif
	ComponentResult		err;
	SGOutput	theOutput;
	AliasHandle	alias = 0;

	// Get the destination filename
#if USE_NAV_SERVICES
	err = NavGetDefaultDialogOptions(&myOptions);
	err = NavPutFile(NULL, &reply, &myOptions, NULL, 'MooV', 'TVOD', NULL);
	
	err = AEGetNthPtr(&(reply.selection), 1, typeFSS, &myKeyword, &myActualType, &theFile,
							sizeof(theFile), &myActualSize);
							
	if (err != noErr || !reply.validRecord)
		goto bail;
#else
	StandardPutFile(prompt, defaultName, &reply);
	if (!reply.sfGood)
	{
		err = fnfErr;
		goto bail;
	} else
		theFile = reply.sfFile;
#endif

	// Make an alias from the filename
	if (err = QTNewAlias(&theFile, &alias, true)) goto bail;
	
	// Create an output from this file
	if (err = SGNewOutput(gSeqGrabber, (Handle)alias, rAliasType, seqGrabToDisk, &theOutput)) goto bail;

	// Associate this output with the specified channel
	if (err = SGSetChannelOutput(gSeqGrabber, theChannel, theOutput)) goto bail;

#if SET_DATA_OUTPUT_MAX_OFFSET
	// Set data output maximums
	{
		wide	maxOffset;
		maxOffset.lo = 1000 * 1024;
		maxOffset.hi = 0;
		SGSetOutputMaximumOffset(gSeqGrabber, theOutput, &maxOffset);
	}
#endif

#if LINK_MULTIPLE_OUTPUTS
	// Link multiple outputs
	{
		SGOutput	output2;
		StandardPutFile("\pLink this file", defaultName, &reply);
		if (reply.sfGood) {
			theFile = reply.sfFile;
			if (err = QTNewAlias(&theFile, &alias, true)) goto bail;
			
			// Create an output from this file
			if (err = SGNewOutput(gSeqGrabber, (Handle)alias, rAliasType, seqGrabToDisk, &output2)) goto bail;

#if SET_DATA_OUTPUT_MAX_OFFSET
			// Set data output maximums
			{
				wide	maxOffset;
				maxOffset.lo = 1000 * 1024;
				maxOffset.hi = 0;
				SGSetOutputMaximumOffset(gSeqGrabber, output2, &maxOffset);
			}
#endif
			SGSetOutputNextOutput(gSeqGrabber, theOutput, output2);
		}
	}
#endif

bail:
	if (alias) DisposeHandle((Handle)alias);
#if USE_NAV_SERVICES
	NavDisposeReply(&reply);
#endif
	return err;
}

/* ---------------------------------------------------------------------- */
#if USE_NAV_SERVICES
static	NavReplyRecord reply;
#endif
// Record a movie
void DoRecord(Boolean useButtonLoop)
{
	long	err;
	FSSpec theFile;
#if USE_NAV_SERVICES
	NavDialogOptions myOptions;
	AEKeyword myKeyword;
	DescType myActualType;
	Size myActualSize;
#else
	StandardFileReply	reply;
#endif
	if (!gCurrentlyRecording)
		{
		// Stop everything while the dialogs are up
		SGStop(gSeqGrabber);

		// Get the destination filename
#if USE_NAV_SERVICES
		reply.validRecord = false;
		err = NavGetDefaultDialogOptions(&myOptions);
		err = NavPutFile(NULL, &reply, &myOptions, NULL, 'MooV', 'TVOD', NULL);
		
		err = AEGetNthPtr(&(reply.selection), 1, typeFSS, &myKeyword, &myActualType, &theFile,
								sizeof(theFile), &myActualSize);
								
		if (err == noErr && reply.validRecord)
			if ((err = SGSetDataOutput(gSeqGrabber, &theFile, seqGrabToDisk)))
				goto bail;
#else
		StandardPutFile("\pSave new movie file as:", "\pHack.mov", &reply);
		if (!reply.sfGood)
		{
			err = fnfErr;
			goto bail;
		} else
			theFile = reply.sfFile;
			
		if ((err = SGSetDataOutput(gSeqGrabber, &theFile, seqGrabToDisk)))
			goto bail;
#endif

#if CREATE_DATA_OUTPUTS
		{
		AliasHandle	alias;
		SGOutput	theOutput;

		StandardPutFile("\pPlace data in:", "\p1.trk", &reply);
		if (!reply.sfGood)
		{
			err = fnfErr;
			goto bail;
		} else
			theFile = reply.sfFile;

		// Make an alias from the filename
		if (err = QTNewAlias(&theFile, &alias, true)) goto bail;
		
		// Create an output from this file
		if (err = SGNewOutput(gSeqGrabber, (Handle)alias, rAliasType, seqGrabToDisk, &theOutput)) {
			CheckError(err,"SGNewOutput(gSeqGrabber, ");
			goto bail;
		}

		// Associate this output with the specified channel
		if (gVideoChannel)
			if (err = SGSetChannelOutput(gSeqGrabber, gVideoChannel, theOutput)) {
				CheckError(err,"SGSetChannelOutput vid");
				goto bail;
			}
		if (gSoundChannel)
			if (err = SGSetChannelOutput(gSeqGrabber, gSoundChannel, theOutput)) {
				CheckError(err,"SGSetChannelOutput sound");
				goto bail;
			}
		gOutput = theOutput;

	#if SET_DATA_OUTPUT_MAX_OFFSET
		// Set data output maximums
		{
			wide	maxOffset;
			maxOffset.lo = 1000 * 1024;
			maxOffset.hi = 0;
			SGSetOutputMaximumOffset(gSeqGrabber, theOutput, &maxOffset);
		}
	#endif
		}
#endif
		// Ask use for separate video and sound track files if requested
		if (gSoundChannel && gRecordSound && gVideoChannel && gRecordVideo && gSplitTracks)
		{
			if ((err = SetTrackFile(gVideoChannel, "\pSave video track file as:", "\pHackVideo.trk")))
				goto bail;
			if ((err = SetTrackFile(gSoundChannel, "\pSave sound track file as:", "\pHackSound.trk")))
				goto bail;
		}
		
		// Asked for special capture flags
		if (gVideoChannel && gRecordVideo) {
			TimeBase sgTimeBase = NULL;
			long channelUsage;
			
			SGGetChannelUsage(gVideoChannel, &channelUsage);
			if (gLowLatency) {
				channelUsage |= seqGrabLowLatencyCapture;
            } else {
                channelUsage &= ~seqGrabLowLatencyCapture;
            }
                
			if (gUseTimeBase) {
				channelUsage |= seqGrabAlwaysUseTimeBase;
            } else {
                channelUsage &= ~seqGrabAlwaysUseTimeBase;
            }
			SGSetChannelUsage(gVideoChannel, channelUsage);
			
 			// If recording sound and we want to use the sound clock to drive the SG TimeBase go ahead and set that up
			if (gSoundChannel && gRecordSound && gUseSoundClock) {
				// use the Clock that is tied to the audio TimeBase as the Clock for the TimeBase
				// that is being used by a Sequence Grabber Component to cut down jitter and sync
				// issues -- this is not necessarily done by default
				TimeBase soundTimeBase = NULL;
				
				err = SGGetTimeBase(gSeqGrabber, &sgTimeBase);
				CheckError(err,"SGGetTimeBase ");
				err = GetComponentInfo((Component)GetTimeBaseMasterClock(sgTimeBase), &gSGClockComponentDescription, NULL, NULL, NULL);
				CheckError(err,"GetComponentInfo ");
				if(!err) {
				     err = SGGetChannelTimeBase(gSoundChannel, &soundTimeBase);
				     CheckError(err,"SGGetChannelTimeBase ");
				}
				if (!err && soundTimeBase)
				    SetTimeBaseMasterClock(sgTimeBase, (Component)GetTimeBaseMasterClock(soundTimeBase), NULL);
				    CheckError(err,"SetTimeBaseMasterClock ");
			} else {
				err = SGGetTimeBase(gSeqGrabber, &sgTimeBase);
				CheckError(err,"SGGetTimeBase ");
				if (gSGClockComponentDescription.componentType != 0) {
					Component c;
					
					c = FindNextComponent(0, &gSGClockComponentDescription);
					SetTimeBaseMasterClock(sgTimeBase, c, NULL);
				}
			}
		}
			
		// If not recording sound or video, then "disable" those channels
		if (gSoundChannel && !gRecordSound)
			SGSetChannelUsage(gSoundChannel, 0);
		if (gVideoChannel && !gRecordVideo)
			SGSetChannelUsage(gVideoChannel, 0);

		// Attempt to recover the preview area obscured by dialogs
	#if TARGET_OS_WIN32
		UpdatePort(gMonitor);
	#endif
		SGUpdate(gSeqGrabber, 0);

		// Startup the grab
		SGPause(gSeqGrabber, false);

		// Make the movie file
		DeleteMovieFile(&theFile);
		if (err = CreateMovieFile(&theFile, 'TVOD', smSystemScript,
			createMovieFileDontOpenFile | createMovieFileDontCreateMovie | createMovieFileDontCreateResFile,
			NULL, NULL)) goto bail;

		// Record!
		// Set the window title so that the user knows that we are recording.
		// The major delay when calling SGStartRecord is the preallocation, so
		// show that.
		SetWTitle(gMonitor, "\pStarting Record... preallocating file...");

		err = SGStartRecord(gSeqGrabber);
		CheckError(err,"SGStartRecord");
		if (err) goto bail;
		
		// Reset the window title to the default.
		SetWTitle(gMonitor, "\pMonitor");
		
		gCurrentlyRecording = true;
		SGUpdate(gSeqGrabber, 0);

		if(useButtonLoop)
			{
			while (!Button() && (err == noErr))
				{
				err = SGIdle(gSeqGrabber);
				CheckError(err,"SGIdle");
#if SHOW_SPACE_REMAINING
				{
				GWorldPtr	wPtr;
				wide		space;
				Str255		str;

				// Note this only works if we are grabbing to the window!
				SGGetGWorld(gSeqGrabber, &wPtr, 0);
				SGGetDataOutputStorageSpaceRemaining64(gSeqGrabber, gOutput, &space);
				NumToString(space.lo, str);
				SetWTitle((WindowPtr)wPtr, str);
				}
#endif
				}
			}
		else 
			{
			return;
			}
		}

	if (gCurrentlyRecording== true)
		{

		// If we recorded until we ran out of space, then allow SGStop to be
		// called to write the movie resource.  The assumption here is that the
		// data output filled up but the disk has enough free space left to
		// write the movie resource.
		if (!((err == dskFulErr) || (err != eofErr)))
			goto bail;
		err = SGStop(gSeqGrabber);
		CheckError(err,"SGStop");

#if USE_NAV_SERVICES
		if ( err == noErr && reply.validRecord) {	
			err = NavCompleteSave(&reply, kNavTranslateInPlace);	//••• Is this needed???
			CheckError(err,"NavCompleteSave");
		}
#endif

		gCurrentlyRecording = false;
		err = SGStartPreview(gSeqGrabber);
		CheckError(err,"SGStartPreview");

		NoteAlert(kMovieHasBeenRecordedAlertID, 0);

#if USE_NAV_SERVICES
		NavDisposeReply(&reply);
		reply.validRecord = false;
#endif	
		}
	return;

bail:
	gCurrentlyRecording = false;
	SGPause(gSeqGrabber, false);
	SGStartPreview(gSeqGrabber);
#if USE_NAV_SERVICES
	NavDisposeReply(&reply);
	reply.validRecord = false;
#endif
}

/* ---------------------------------------------------------------------- */

void DoAboutDialog(void)
{
	short		itemHit;
	DialogPtr	aboutDialog;

	aboutDialog = GetNewDialog(kAboutDLOGID, NULL, (WindowPtr)-1L);

	// Do the boring about dialog
	SetDialogDefaultItem(aboutDialog, 1);
	MacShowWindow(GetDialogWindow(aboutDialog));
	do
	{
		ModalDialog(NULL, &itemHit);
	}
	while (itemHit != 1);
	DisposeDialog(aboutDialog);
}

/* ---------------------------------------------------------------------- */

void DoPageSetup(void)
{
#if TARGET_API_MAC_CARBON
	DebugStr("\pYou can't call DoPageSetup today");
#else
	PrOpen();
	PrStlDialog(gPrintRec);
	PrClose();
#endif
}

/* ---------------------------------------------------------------------- */

void DoPrint(void)
{
#if TARGET_API_MAC_CARBON
	DebugStr("\pYou can't call DoPrint today");
#else
	TPPrPort	printPort;
	TPrStatus	printStatus;
	ComponentResult		err;
	Rect		tempRect;

	// Copy a frame from the monitor
	if (gMonitorPICT != NULL)
		KillPicture (gMonitorPICT);
	gMonitorPICT = NULL;
	err = SGGrabPict(gSeqGrabber, &gMonitorPICT, NULL, 0, grabPictOffScreen);
	if ((err == noErr) && (gMonitorPICT != NULL))
	{
		// Print it
		HLock((Handle) gMonitorPICT);
		PrOpen();
		if (PrJobDialog (gPrintRec))
		{
			printPort = PrOpenDoc (gPrintRec, NULL, NULL);
			err = PrError();
			PrOpenPage (printPort, 0);
			err = PrError();

			tempRect  = (**gMonitorPICT).picFrame;
			tempRect.left = EndianS16_BtoN(tempRect.left);
			tempRect.top = EndianS16_BtoN(tempRect.top);
			tempRect.right = EndianS16_BtoN(tempRect.right);
			tempRect.bottom = EndianS16_BtoN(tempRect.bottom);
			DrawPicture(gMonitorPICT, &tempRect);

			PrClosePage (printPort);
			err = PrError();
			PrCloseDoc (printPort);
			err = PrError();
			if ((**gPrintRec).prJob.bJDocLoop == bSpoolLoop)
			{
				PrPicFile (gPrintRec, 0, 0, 0, &printStatus);
				err = PrError();
			}
		}
		PrClose();
		err = PrError();
		HUnlock((Handle) gMonitorPICT);
	}
#endif
}

/* ---------------------------------------------------------------------- */

#if TARGET_API_MAC_CARBON
static OSStatus ShimPutScrap (		SInt32 				sourceBufferByteCount,
						ScrapFlavorType 	flavorType,
						const void *		sourceBuffer			)
{
	OSStatus err = noErr;

	do
	{
		ScrapRef scrap;
		err = GetCurrentScrap (&scrap);
		if (err) break;
		err = PutScrapFlavor (scrap,flavorType,kScrapFlavorMaskNone,sourceBufferByteCount,sourceBuffer);
		if (err) break;
	}
	while (false);

	return err;
}
#else
	#define ShimPutScrap(a, b, c)	PutScrap(a, b, c)
#endif

void DoCopyToClipboard(void)
{
	ComponentResult	err;

	// Copy a frame from the monitor
	if (gMonitorPICT != NULL)
		KillPicture (gMonitorPICT);
	gMonitorPICT = NULL;
	err = SGGrabPict (gSeqGrabber, &gMonitorPICT, NULL, 0, grabPictOffScreen);
	if ((err == noErr) && (gMonitorPICT != NULL))
	{
		err = ClearCurrentScrap();
		HLock ((Handle) gMonitorPICT);
		err = ShimPutScrap (GetHandleSize ((Handle) gMonitorPICT), 'PICT', *(Handle)gMonitorPICT);
		HUnlock ((Handle) gMonitorPICT);
	}
}

/* ---------------------------------------------------------------------- */

static pascal Boolean
SeqGrabberModalFilterProc (DialogPtr theDialog, const EventRecord *theEvent,
	short *itemHit, long refCon)
{
#pragma unused(theDialog, itemHit)
	// Ordinarily, if we had multiple windows we cared about, we'd handle
	// updating them in here, but since we don't, we'll just clear out
	// any update events meant for us
	
	Boolean	handled = false;
	
	if ((theEvent->what == updateEvt) && 
		((WindowPtr) theEvent->message == (WindowPtr) refCon))
	{
		BeginUpdate ((WindowPtr) refCon);
		EndUpdate ((WindowPtr) refCon);
		handled = true;
	}
	return (handled);
}

/* ---------------------------------------------------------------------- */

void DoVideoSettings(void)
{
	Rect	newActiveVideoRect;
	Rect	adjustedActiveVideoRect;
	Rect	curBounds, curVideoRect, newVideoRect, newBounds;
	short	width, height;
	ComponentResult	err;
	GrafPtr	savedPort;
	RgnHandle	deadRgn;
	SGModalFilterUPP	seqGragModalFilterUPP;
	Rect	portRect;			
	
	// Get our current state
	err = SGGetChannelBounds (gVideoChannel, &curBounds);
	err = SGGetVideoRect (gVideoChannel, &curVideoRect);
	
	// Pause
	err = SGPause (gSeqGrabber, true);
	
	// Do the dialog thang
	seqGragModalFilterUPP = (SGModalFilterUPP)NewSGModalFilterUPP(SeqGrabberModalFilterProc);
	err = SGSettingsDialog(gSeqGrabber, gVideoChannel, 0, 
		NULL, 0L, seqGragModalFilterUPP, (long)gMonitor);
	DisposeSGModalFilterUPP(seqGragModalFilterUPP);
	
	// What happened?
	err = SGGetVideoRect (gVideoChannel, &newVideoRect);
	err = SGGetSrcVideoBounds (gVideoChannel, &newActiveVideoRect);

	// Set up our port
	GetPort (&savedPort);
	MacSetPort ((GrafPtr)GetWindowPort(gMonitor));
	
	// Has our active rect changed?
	// If so, it's because our video standard changed (e.g., NTSC to PAL),
	// and we need to adjust our monitor window
	if (!MacEqualRect (&gActiveVideoRect, &newActiveVideoRect))
	{
		if (gFullSize)
		{
			
			width = newActiveVideoRect.right - newActiveVideoRect.left;
			height = newActiveVideoRect.bottom - newActiveVideoRect.top;
			
			gActiveVideoRect = newActiveVideoRect;
			SizeWindow (gMonitor, width, height, false);
			GetPortBounds(GetWindowPort(gMonitor), &portRect);
			
			err = SGSetChannelBounds (gVideoChannel, &portRect);
		}
		else if (gHalfSize)
		{
			width = (newActiveVideoRect.right - newActiveVideoRect.left) / 2;
			height = (newActiveVideoRect.bottom - newActiveVideoRect.top) / 2;
			
			gActiveVideoRect = newActiveVideoRect;
			SizeWindow (gMonitor, width, height, false);
			GetPortBounds(GetWindowPort(gMonitor), &portRect);
			
			err = SGSetChannelBounds (gVideoChannel, &portRect);
		}
		else if (gQuarterSize)
		{
			width = (newActiveVideoRect.right - newActiveVideoRect.left) / 4;
			height = (newActiveVideoRect.bottom - newActiveVideoRect.top) / 4;
			
			gActiveVideoRect = newActiveVideoRect;
			SizeWindow (gMonitor, width, height, false);
			GetPortBounds(GetWindowPort(gMonitor), &portRect);

			err = SGSetChannelBounds (gVideoChannel, &portRect);
		}
	}
	
	// Has our crop changed?
	// This code shows how to be crop video panel friendly
	// Two important things - 
	// 1) Be aware that you might have been cropped and adjust your
	//    video window appropriately
	// 2) Be aware that you might have been adjusted and attempt to
	//    account for this.  Adjusting refers to using the digitizer
	//    rect to "adjust" the active source rect within the maximum
	//    source rect.  This is useful if you're getting those nasty
	//    black bands on the sides of your video display - you can use
	//    the control-arrow key sequence to shift the active source 
	//    rect around when you're in the crop video panel
	
	adjustedActiveVideoRect = gActiveVideoRect;
	if (!MacEqualRect (&curVideoRect, &newVideoRect))
	{
		GetPortBounds(GetWindowPort(gMonitor), &portRect);

		if ((newVideoRect.left < gActiveVideoRect.left) ||
			(newVideoRect.right > gActiveVideoRect.right) ||
			(newVideoRect.top < gActiveVideoRect.top) ||
			(newVideoRect.bottom > gActiveVideoRect.bottom))
		{
			if (newVideoRect.left < gActiveVideoRect.left)
			{
				adjustedActiveVideoRect.left = newVideoRect.left;
				adjustedActiveVideoRect.right -= (gActiveVideoRect.left - newVideoRect.left);
			}
			if (newVideoRect.right > gActiveVideoRect.right)
			{
				adjustedActiveVideoRect.right = newVideoRect.right;
				adjustedActiveVideoRect.left += (newVideoRect.right - gActiveVideoRect.right);
			}
			if (newVideoRect.top < gActiveVideoRect.top)
			{
				adjustedActiveVideoRect.top = newVideoRect.top;
				adjustedActiveVideoRect.bottom -= (gActiveVideoRect.top - newVideoRect.top);
			}
			if (newVideoRect.bottom > gActiveVideoRect.bottom)
			{
				adjustedActiveVideoRect.bottom = newVideoRect.bottom;
				adjustedActiveVideoRect.top += (newVideoRect.bottom - gActiveVideoRect.bottom);
			}
			newBounds = newVideoRect;
			MapRect (&newBounds, &adjustedActiveVideoRect, &portRect);
		}
		else	// Can't tell if we've been adjusted (digitizer rect is smaller on all sides
				// than the active source rect)
		{
			newBounds = newVideoRect;
			MapRect (&newBounds, &gActiveVideoRect, &portRect);
		}
		width = newBounds.right - newBounds.left;
		height = newBounds.bottom - newBounds.top;
		err = SGSetChannelBounds (gVideoChannel, &newBounds);
	}

	// Clean out the part of the port that isn't being drawn in
	deadRgn = NewRgn();
	if (deadRgn != NULL)
	{
		Rect	boundsRect;

		GetPortBounds(GetWindowPort(gMonitor), &portRect);

		err = SGGetChannelBounds (gVideoChannel, &boundsRect);
		err = XorRectToRgn (&boundsRect, &portRect, &deadRgn);
		EraseRgn (deadRgn);
		DisposeRgn (deadRgn);
	}

	MacSetPort (savedPort);
	
#if !TARGET_OS_MAC
	// This is necessary, for now, to get the grab to start again afer the
	// dialog goes away.  For some reason the video destRect never gets reset to point
	// back to the monitor window.
	SGSetChannelBounds (gVideoChannel, &(gMonitor->portRect));
#endif

	// The pause that refreshes
	err = SGPause (gSeqGrabber, false);
}

/* ---------------------------------------------------------------------- */

void DoSoundSettings(void)
{
	SGModalFilterUPP	seqGragModalFilterUPP;
	ComponentResult		err;

	seqGragModalFilterUPP = (SGModalFilterUPP)NewSGModalFilterUPP(SeqGrabberModalFilterProc);
	err = SGSettingsDialog (gSeqGrabber, gSoundChannel, 0,
		NULL, 0L, seqGragModalFilterUPP, (long)gMonitor);
	DisposeSGModalFilterUPP(seqGragModalFilterUPP);
}

/* ---------------------------------------------------------------------- */

void DoResize(short divisor)
{
	short		width, height;

	// New width and height
	width = (gActiveVideoRect.right - gActiveVideoRect.left) / divisor;
	height = (gActiveVideoRect.bottom - gActiveVideoRect.top) / divisor;
	
	gQuarterSize = (divisor == 4 ? true : false);
	gHalfSize = (divisor == 2 ? true : false);
	gFullSize = (divisor == 1 ? true : false);
	
	SetNewSize(width,height);

}

void SetNewSize(short width, short height)
{
	GrafPtr		savedPort;
	Rect		curBounds, maxBoundsRect;
	RgnHandle	deadRgn;
	ComponentResult	err;
	Rect portRect;
		
	// Resize the monitor
	GetPort (&savedPort);
	MacSetPort ((GrafPtr)GetWindowPort(gMonitor));
	err = SGPause (gSeqGrabber, true);
	err = SGGetChannelBounds (gVideoChannel, &curBounds);

	GetPortBounds(GetWindowPort(gMonitor), &portRect);

	maxBoundsRect = portRect;
	SizeWindow (gMonitor, width, height, false);
	GetPortBounds(GetWindowPort(gMonitor), &portRect);

	MapRect (&curBounds, &maxBoundsRect, &portRect);
	err = SGSetChannelBounds (gVideoChannel, &curBounds);

	// Clean out part of port we're not drawing in
	deadRgn = NewRgn();
	if (deadRgn != NULL)
	{
		Rect	boundsRect;

		err = SGGetChannelBounds (gVideoChannel, &boundsRect);
		GetPortBounds(GetWindowPort(gMonitor), &portRect);
		err = XorRectToRgn (&boundsRect, &portRect, &deadRgn);
		EraseRgn (deadRgn);
		DisposeRgn (deadRgn);
	}
		
	MacSetPort (savedPort);
	err = SGPause (gSeqGrabber, false);
}

/* ---------------------------------------------------------------------- */

static OSErr XorRectToRgn (Rect *srcRectA, Rect *srcRectB, RgnHandle *destRgn)
{
	RgnHandle	srcRgnA = NewRgn();
	RgnHandle	srcRgnB = NewRgn();
	OSErr		result = noErr;
	
	if ((destRgn != NULL) && (*destRgn != NULL))
	{
		if ((srcRgnA != NULL) &&
			(srcRgnB != NULL))
		{
			RectRgn (srcRgnA, srcRectA);
			RectRgn (srcRgnB, srcRectB);
			MacXorRgn (srcRgnA, srcRgnB, *destRgn);
			DisposeRgn (srcRgnA);
			DisposeRgn (srcRgnB);
		}
		else
		{
			result = memFullErr;
		}
	}
	else
	{
		result = nilHandleErr;
	}
	return (result);
}

/* ---------------------------------------------------------------------- */

void CheckError(OSErr err,char * message) 
{
	Str255 msg;
	if (!err) return;
	sprintf((char*)&msg[1],"%d:%s",err,message);
	msg[0] = strlen((char*)&msg[1]);
	if (msg[0] && gMonitor) {
		SetWTitle(gMonitor,msg);
	}	
}