/*
	File:		Common.c

	Contains:	HackTV cross-platform common code

	Written by:	Gary Woodcock
	Updated by: Brian Friedkin
			
	Copyright:	© 1992-1998 by Apple Computer, Inc.
*/
#include <QTML.h>
#include <Endian.h>
#include <Menus.h>
#include <Printing.h>
#include <Script.h>
#include <Scrap.h>
#include <QuickTimeComponents.h>
#include "Globals.h"
#include "Common.h"

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
	gMonitorPICT = nil;
	gPrintRec = (THPrint) NewHandleClear (sizeof (TPrint));
	
	// Find and open a sequence grabber
	theDesc.componentType = SeqGrabComponentType;
	theDesc.componentSubType = 0L;
	theDesc.componentManufacturer = 'appl';
	theDesc.componentFlags = 0L;
	theDesc.componentFlagsMask = 0L;	
	sgCompID = FindNextComponent (nil, &theDesc);
	if (sgCompID != 0L)
		gSeqGrabber = OpenComponent (sgCompID);
	
	// If we got a sequence grabber, set it up
	if (gSeqGrabber != 0L)
	{
		// Get the monitor
		CreateMonitorWindow();
		if (gMonitor != nil)
		{
			// Display the monitor window
			GetPort (&savedPort);
			MacSetPort (gMonitor);
			MacMoveWindow(gMonitor, 10, 30 + GetMBarHeight(), 0);
			MacShowWindow (gMonitor);		

			// Initialize the sequence grabber
			result = SGInitialize (gSeqGrabber);
			if (result == noErr)
			{
				result = SGSetGWorld (gSeqGrabber, (CGrafPtr) gMonitor, nil);
				
				// Get a video channel
				result = SGNewChannel (gSeqGrabber, VideoMediaType, &gVideoChannel);
				if ((gVideoChannel != nil) && (result == noErr))
				{
					short	width;
					short	height;
					
					gQuarterSize = false;
					gHalfSize = true;
					gFullSize = false;
					
					result = SGGetSrcVideoBounds (gVideoChannel, &gActiveVideoRect);
					width = (gActiveVideoRect.right - gActiveVideoRect.left) / 2;
					height = (gActiveVideoRect.bottom - gActiveVideoRect.top) / 2;
					SizeWindow (gMonitor, width, height, false);
					
					result = SGSetChannelUsage (gVideoChannel, seqGrabPreview | seqGrabRecord | seqGrabPlayDuringRecord);
					result = SGSetChannelBounds (gVideoChannel, &(gMonitor->portRect));
				}
				
				// Get a sound channel
				result = SGNewChannel (gSeqGrabber, SoundMediaType, &gSoundChannel);

				if ((gSoundChannel != nil) && (result == noErr))
				{
					if (gSoundChannel != nil)
					{
						result = SGSetChannelUsage (gSoundChannel, seqGrabPreview | seqGrabRecord);
						
						// Set the volume low to prevent feedback when we start the preview,
						// in case the mic is anywhere near the speaker.
						result = SGSetChannelVolume (gSoundChannel, 0x0010);
					}
				}
				
				// Get the alignment proc (for use when dragging the monitor)
				result = SGGetAlignmentProc (gSeqGrabber, &gSeqGrabberAlignProc);
			}
			
			// Go!
			if (result == noErr)
				result = SGStartPreview (gSeqGrabber);
			MacSetPort (savedPort);
		}
	}
}

/* ---------------------------------------------------------------------- */

// Specify and setup a file to contain this track's data
static ComponentResult SetTrackFile(SGChannel theChannel, StringPtr prompt, StringPtr defaultName)
{
	StandardFileReply		reply;
	ComponentResult		err;
	SGOutput	theOutput;
	AliasHandle	alias = 0;

	// Get the destination filename
	StandardPutFile(prompt, defaultName, &reply);
	if (!reply.sfGood)
	{
		err = fnfErr;
		goto bail;
	}

	// Make an alias from the filename
	if (err = QTNewAlias(&reply.sfFile, &alias, true)) goto bail;
	
	// Create an output from this file
	if (err = SGNewOutput(gSeqGrabber, (Handle)alias, rAliasType, seqGrabToDisk, &theOutput)) goto bail;

	// Associate this output with the specified channel
	if (err = SGSetChannelOutput(gSeqGrabber, theChannel, theOutput)) goto bail;

bail:
	if (alias) DisposeHandle((Handle)alias);
	return err;
}

/* ---------------------------------------------------------------------- */

// Record a movie
void DoRecord(void)
{
	long	err;
	StandardFileReply	reply;
	
	// Stop everything while the dialogs are up
	SGStop(gSeqGrabber);

	// Get the destination filename
	StandardPutFile("\pSave new movie file as:", "\pHack.mov", &reply);
	if (!reply.sfGood)
	{
		err = fnfErr;
		goto bail;
	}
	if ((err = SGSetDataOutput(gSeqGrabber, &reply.sfFile, seqGrabToDisk)))
		goto bail;

	// Ask use for separate video and sound track files if requested
	if (gSoundChannel && gRecordSound && gVideoChannel && gRecordVideo && gSplitTracks)
	{
		if ((err = SetTrackFile(gVideoChannel, "\pSave video track file as:", "\pHackVideo.trk")))
			goto bail;
		if ((err = SetTrackFile(gSoundChannel, "\pSave sound track file as:", "\pHackSound.trk")))
			goto bail;
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

	// Make the movie file
	DeleteMovieFile(&reply.sfFile);
	if (err = CreateMovieFile(&reply.sfFile, 'TVOD', smSystemScript,
		createMovieFileDontOpenFile | createMovieFileDontCreateMovie | createMovieFileDontCreateResFile,
		nil, nil)) goto bail;
		
	FlushEvents(mDownMask+mUpMask,0);
	
	// Record!
	if (err = SGStartRecord(gSeqGrabber))
		goto bail;

	while (!Button() && (err == noErr))
	{
		err = SGIdle(gSeqGrabber);
	}

	// If we recorded until we ran out of space, then allow SGStop to be
	// called to write the movie resource.  The assumption here is that the
	// data output filled up but the disk has enough free space left to
	// write the movie resource.
	if (!((err == dskFulErr) || (err != eofErr)))
		goto bail;
	err = SGStop(gSeqGrabber);
	NoteAlert(kMovieHasBeenRecordedAlertID, 0);
	err = SGStartPreview(gSeqGrabber);

	return;

bail:
	SGPause(gSeqGrabber, false);
	SGStartPreview(gSeqGrabber);
}

/* ---------------------------------------------------------------------- */

void DoAboutDialog(void)
{
	short		itemHit;
	DialogPtr	aboutDialog;

	aboutDialog = GetNewDialog(kAboutDLOGID, nil, (WindowPtr)-1L);

	// Do the boring about dialog
	SetDialogDefaultItem(aboutDialog, 1);
	MacShowWindow(aboutDialog);
	do
	{
		ModalDialog(nil, &itemHit);
	}
	while (itemHit != 1);
	DisposeDialog(aboutDialog);
}

/* ---------------------------------------------------------------------- */

void DoPageSetup(void)
{
	PrOpen();
	PrStlDialog(gPrintRec);
	PrClose();
}

/* ---------------------------------------------------------------------- */

void DoPrint(void)
{
	TPPrPort	printPort;
	TPrStatus	printStatus;
	ComponentResult		err;
	Rect		tempRect;

	// Copy a frame from the monitor
	if (gMonitorPICT != nil)
		KillPicture (gMonitorPICT);
	gMonitorPICT = nil;
	err = SGGrabPict(gSeqGrabber, &gMonitorPICT, nil, 0, grabPictOffScreen);
	if ((err == noErr) && (gMonitorPICT != nil))
	{
		// Print it
		HLock((Handle) gMonitorPICT);
		PrOpen();
		if (PrJobDialog (gPrintRec))
		{
			printPort = PrOpenDoc (gPrintRec, nil, nil);
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
}

/* ---------------------------------------------------------------------- */

void DoCopyToClipboard(void)
{
	ComponentResult	err;

	// Copy a frame from the monitor
	if (gMonitorPICT != nil)
		KillPicture (gMonitorPICT);
	gMonitorPICT = nil;
	err = SGGrabPict (gSeqGrabber, &gMonitorPICT, nil, 0, grabPictOffScreen);
	if ((err == noErr) && (gMonitorPICT != nil))
	{
		err = ZeroScrap();
		HLock ((Handle) gMonitorPICT);
		err = PutScrap (GetHandleSize ((Handle) gMonitorPICT), 'PICT', *(Handle)gMonitorPICT);
		HUnlock ((Handle) gMonitorPICT);
	}
}

/* ---------------------------------------------------------------------- */

static pascal Boolean
SeqGrabberModalFilterProc (DialogPtr theDialog, EventRecord *theEvent,
	short *itemHit, long refCon)
{
	// Ordinarily, if we had multiple windows we cared about, we'd handle
	// updating them in here, but since we don't, we'll just clear out
	// any update events meant for us
	
	Boolean	handled = false;
	
	if ((theEvent->what == updateEvt) && 
		((WindowPtr) theEvent->message == (WindowPtr) refCon))
	{
		WindowPtr	wPtr = (WindowPtr) refCon;
		BeginUpdate (wPtr);
		EndUpdate (wPtr);
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
	
	// Get our current state
	err = SGGetChannelBounds (gVideoChannel, &curBounds);
	err = SGGetVideoRect (gVideoChannel, &curVideoRect);
	
	// Pause
	err = SGPause (gSeqGrabber, true);
	
	// Do the dialog thang
	seqGragModalFilterUPP = (SGModalFilterUPP)NewSGModalFilterProc(SeqGrabberModalFilterProc);
	err = SGSettingsDialog(gSeqGrabber, gVideoChannel, 0, 
		nil, 0L, seqGragModalFilterUPP, (long)StripAddress ((Ptr) gMonitor));
	DisposeRoutineDescriptor(seqGragModalFilterUPP);
	
	// What happened?
	err = SGGetVideoRect (gVideoChannel, &newVideoRect);
	err = SGGetSrcVideoBounds (gVideoChannel, &newActiveVideoRect);

	// Set up our port
	GetPort (&savedPort);
	MacSetPort (gMonitor);
	
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
			err = SGSetChannelBounds (gVideoChannel, &(gMonitor->portRect));
		}
		else if (gHalfSize)
		{
			width = (newActiveVideoRect.right - newActiveVideoRect.left) / 2;
			height = (newActiveVideoRect.bottom - newActiveVideoRect.top) / 2;
			
			gActiveVideoRect = newActiveVideoRect;
			SizeWindow (gMonitor, width, height, false);
			err = SGSetChannelBounds (gVideoChannel, &(gMonitor->portRect));
		}
		else if (gQuarterSize)
		{
			width = (newActiveVideoRect.right - newActiveVideoRect.left) / 4;
			height = (newActiveVideoRect.bottom - newActiveVideoRect.top) / 4;
			
			gActiveVideoRect = newActiveVideoRect;
			SizeWindow (gMonitor, width, height, false);
			err = SGSetChannelBounds (gVideoChannel, &(gMonitor->portRect));
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
			MapRect (&newBounds, &adjustedActiveVideoRect, &(gMonitor->portRect));
		}
		else	// Can't tell if we've been adjusted (digitizer rect is smaller on all sides
				// than the active source rect)
		{
			newBounds = newVideoRect;
			MapRect (&newBounds, &gActiveVideoRect, &(gMonitor->portRect));
		}
		width = newBounds.right - newBounds.left;
		height = newBounds.bottom - newBounds.top;
		err = SGSetChannelBounds (gVideoChannel, &newBounds);
	}

	// Clean out the part of the port that isn't being drawn in
	deadRgn = NewRgn();
	if (deadRgn != nil)
	{
		Rect	boundsRect;
		err = SGGetChannelBounds (gVideoChannel, &boundsRect);
		err = XorRectToRgn (&boundsRect, &(gMonitor->portRect), &deadRgn);
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

	seqGragModalFilterUPP = (SGModalFilterUPP)NewSGModalFilterProc(SeqGrabberModalFilterProc);
	err = SGSettingsDialog (gSeqGrabber, gSoundChannel, 0,
		nil, 0L, seqGragModalFilterUPP, (long) StripAddress ((Ptr) gMonitor));
	DisposeRoutineDescriptor(seqGragModalFilterUPP);
}

/* ---------------------------------------------------------------------- */

void DoResize(short divisor)
{
	short		width, height;
	GrafPtr		savedPort;
	Rect		curBounds, maxBoundsRect;
	RgnHandle	deadRgn;
	ComponentResult	err;

	// New width and height
	width = (gActiveVideoRect.right - gActiveVideoRect.left) / divisor;
	height = (gActiveVideoRect.bottom - gActiveVideoRect.top) / divisor;
	
	gQuarterSize = (divisor == 4 ? true : false);
	gHalfSize = (divisor == 2 ? true : false);
	gFullSize = (divisor == 1 ? true : false);
	
	// Resize the monitor
	GetPort (&savedPort);
	MacSetPort (gMonitor);
	err = SGPause (gSeqGrabber, true);
	err = SGGetChannelBounds (gVideoChannel, &curBounds);
	maxBoundsRect = gMonitor->portRect;
	SizeWindow (gMonitor, width, height, false);
	MapRect (&curBounds, &maxBoundsRect, &(gMonitor->portRect));
	err = SGSetChannelBounds (gVideoChannel, &curBounds);

	// Clean out part of port we're not drawing in
	deadRgn = NewRgn();
	if (deadRgn != nil)
	{
		Rect	boundsRect;

		err = SGGetChannelBounds (gVideoChannel, &boundsRect);
		err = XorRectToRgn (&boundsRect, &(gMonitor->portRect), &deadRgn);
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
	
	if ((destRgn != nil) && (*destRgn != nil))
	{
		if ((srcRgnA != nil) &&
			(srcRgnB != nil))
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
