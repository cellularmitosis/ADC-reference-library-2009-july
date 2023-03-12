/*
	File:		MoreCDs.c

	Contains:	CD-ROM driver utilities.

	Written by:	Quinn

	Copyright:	Copyright (c) 2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreCDs.c,v $
Revision 1.4  2002/11/08 23:28:12         
Include our prototype early to flush out any missing dependencies. Only include TradDriverLoaderLib if not compiling Carbon. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.3  2001/11/07 15:51:32         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>      5/7/01    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreCDs.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Devices.h>
	#include <TextUtils.h>
#endif

// MIB Prototypes

#include "MoreMemory.h"

#if !TARGET_API_MAC_CARBON
	#include "TradDriverLoaderLib.h"
#endif

/////////////////////////////////////////////////////////////////

extern pascal UInt8 MoreCDsBCDToByte(UInt8 bcd)
	// See comment in header.
{
	UInt8 firstDigit;
	UInt8 secondDigit;
	
	firstDigit  = ((bcd >> 4) & 0x0F);
	secondDigit =  (bcd & 0x0F);

	assert(firstDigit < 10);
	assert(secondDigit < 10);

	return (firstDigit * 10) + secondDigit;
}

extern pascal UInt8 MoreCDsByteToBCD(UInt8 byte)
	// See comment in header.
{
	assert(byte < 100);
	return ((byte / 10) << 4) | (byte % 10);
}

extern pascal void MoreCDsLBATimeToMSFTime(CDLBATime lbaTime, CDMSFTime *msfTime)
	// See comment in header.
{
	assert(msfTime != NULL);
	
	// I force all arithmetic to type long for voodoo reasons.  
	// I’ve been bitten by this too many times.

	msfTime->minutes	= (lbaTime / 75L) / 60L;
	msfTime->seconds	= (lbaTime / 75L) % 60L;
	msfTime->frames		=  lbaTime % 75L;
}

extern pascal CDLBATime MoreCDsMSFTimeToLBATime(const CDMSFTime *msfTime)
	// See comment in header.
{
	CDLBATime result;
	
	assert(msfTime->seconds < 60);
	assert(msfTime->frames  < 75);
	
	// I force all arithmetic to type long for voodoo reasons.  
	// I’ve been bitten by this too many times.

	result =  ((CDLBATime) msfTime->minutes) * 75L * 60L
			+ ((CDLBATime) msfTime->seconds) * 75L
			+ ((CDLBATime) msfTime->frames);
	
	// To catch any arithmetic errors (surprisingly common in routines like this) 
	// I’ve added code to check the results of this routine against the inverse 
	// operation, MoreCDsLBATimeToMSFTime.
		
	#if MORE_DEBUG
		{
			CDMSFTime msfTmp;
			
			MoreCDsLBATimeToMSFTime(result, &msfTmp);
			assert(msfTmp.minutes == msfTime->minutes);
			assert(msfTmp.seconds == msfTime->seconds);
			assert(msfTmp.frames  == msfTime->frames );
		}
	#endif

	return result;
}

extern pascal CDLBATime MoreCDsMinSecFrameToLBATime(UInt8 minutes, UInt8 seconds, UInt8 frames)
	// See comment in header.
{
	CDMSFTime tmp;
	
	tmp.control = 0;
	tmp.minutes = minutes;
	tmp.seconds = seconds;
	tmp.frames  = frames;
	return MoreCDsMSFTimeToLBATime(&tmp);
}

extern pascal void MoreCDsMSFTimeFromBCD(CDMSFTime *msfTime)
	// See comment in header.
{
	msfTime->minutes = MoreCDsBCDToByte(msfTime->minutes);
	msfTime->seconds = MoreCDsBCDToByte(msfTime->seconds);
	msfTime->frames  = MoreCDsBCDToByte(msfTime->frames );
}

extern pascal void MoreCDsMSFTimeToBCD(CDMSFTime *msfTime)
	// See comment in header.
{
	msfTime->minutes = MoreCDsByteToBCD(msfTime->minutes);
	msfTime->seconds = MoreCDsByteToBCD(msfTime->seconds);
	msfTime->frames  = MoreCDsByteToBCD(msfTime->frames );
}

#if !TARGET_API_MAC_CARBON

extern pascal OSStatus MoreCDsReadTOCFirstAndLastTrack(SInt16 driveNum, UInt16 *firstTrack, UInt16 *lastTrack, UInt16 *trackCount)
	// See comment in header.
{
	OSStatus 	  err;
	ParamBlockRec pb;
	UInt16		  localFirstTrack;
	UInt16		  localLastTrack;
	
	pb.cntrlParam.ioNamePtr  = NULL;
	pb.cntrlParam.ioVRefNum  = driveNum;
	pb.cntrlParam.ioCRefNum  = MoreGetDriveRefNum(driveNum);
	pb.cntrlParam.csCode     = kReadTOC;
	pb.cntrlParam.csParam[0] = kCDReadTOCFirstAndLastTrack;
	err = PBControlSync(&pb);
	if (err == noErr) {
		localFirstTrack = MoreCDsBCDToByte((pb.cntrlParam.csParam[0] >> 8) & 0x0FF);
		localLastTrack  = MoreCDsBCDToByte( pb.cntrlParam.csParam[0] & 0x0FF);
		if (firstTrack != NULL) {
			*firstTrack = localFirstTrack;
		}
		if (lastTrack != NULL) {
			*lastTrack  = localLastTrack;
		}
		if (trackCount != NULL) {
			*trackCount = localLastTrack - localFirstTrack + 1;
		}
	}
	return err;
}

extern pascal OSStatus MoreCDsReadTOCLastSessionLeadOut(SInt16 driveNum, CDLBATime *leadOut)
	// See comment in header.
{
	OSStatus 		 	err;
	ParamBlockRec 	 	pb;
	CDMSFTime 			leadOutTmp;
	
	// Most of the other calls support parameters being NULL if you don’t want 
	// the value.  However, this routine only returns one parameter, so if 
	// you don’t want its value don’t call it!
	assert(leadOut != NULL);

	pb.cntrlParam.ioNamePtr  = NULL;
	pb.cntrlParam.ioVRefNum  = driveNum;
	pb.cntrlParam.ioCRefNum  = MoreGetDriveRefNum(driveNum);
	pb.cntrlParam.csCode     = kReadTOC;
	pb.cntrlParam.csParam[0] = kCDReadTOCLastSessionLeadOut;
	err = PBControlSync(&pb);
	if (err == noErr) {
		leadOutTmp.control = 0;
		// Move the first three bytes of csParam into the minutes, 
		// seconds, and frames fields of leadOutTmp.
		BlockMoveData(&pb.cntrlParam.csParam[0], &leadOutTmp.minutes, 3);
		MoreCDsMSFTimeFromBCD(&leadOutTmp);
		*leadOut = MoreCDsMSFTimeToLBATime(&leadOutTmp);
	}
	return err;

}

extern pascal OSStatus MoreCDsReadTOCTrackStarts(SInt16 driveNum, UInt16 firstTrack, UInt16 trackCount, 
									CDMSFTime trackStarts[])
	// See comment in header.
{
	OSStatus 		 err;
	ParamBlockRec 	 pb;
	UInt16			 trackIndex;
	
	assert(trackStarts != NULL);
	assert(sizeof(CDMSFTime) == 4);

	pb.cntrlParam.ioNamePtr  = NULL;
	pb.cntrlParam.ioVRefNum  = driveNum;
	pb.cntrlParam.ioCRefNum  = MoreGetDriveRefNum(driveNum);
	pb.cntrlParam.csCode     = kReadTOC;
	pb.cntrlParam.csParam[0] = kCDReadTOCTrackStarts;
	*((CDMSFTime **) &pb.cntrlParam.csParam[1]) = trackStarts;
	pb.cntrlParam.csParam[3] = trackCount * sizeof(CDMSFTime);
	pb.cntrlParam.csParam[4] = MoreCDsByteToBCD(firstTrack) << 8;
	err = PBControlSync(&pb);
	if (err == noErr) {
		// Convert results from BCD to binary.
		for (trackIndex = 0; trackIndex < trackCount; trackIndex++) {
			MoreCDsMSFTimeFromBCD(&trackStarts[trackIndex]);
		}
	}
	return err;
}

extern pascal OSStatus MoreCDsReadTOCSessionInfo(SInt16 driveNum, UInt16 *firstSession, UInt16 *lastSession, 
													UInt16 *firstTrackOfLastSession,
													CDMSFTime *infoForFirstTrackOfLastSession)
	// See comment in header.
{
	OSStatus 		 	err;
	ParamBlockRec 	 	pb;
	
	assert(sizeof(*infoForFirstTrackOfLastSession) == 4);

	pb.cntrlParam.ioNamePtr  = NULL;
	pb.cntrlParam.ioVRefNum  = driveNum;
	pb.cntrlParam.ioCRefNum  = MoreGetDriveRefNum(driveNum);
	pb.cntrlParam.csCode     = kReadTOC;
	pb.cntrlParam.csParam[0] = kCDReadTOCSessionInfo;
	err = PBControlSync(&pb);
	if (err == noErr) {
		if (firstSession != NULL) {
			*firstSession            = MoreCDsBCDToByte(pb.cntrlParam.csParam[0]);
		}
		if (lastSession != NULL) {
			*lastSession             = MoreCDsBCDToByte(pb.cntrlParam.csParam[1]);
		}
		if (firstTrackOfLastSession != NULL) {
			*firstTrackOfLastSession = MoreCDsBCDToByte(pb.cntrlParam.csParam[2]);
		}
		if (infoForFirstTrackOfLastSession != NULL) {
			BlockMoveData(&pb.cntrlParam.csParam[3], infoForFirstTrackOfLastSession, sizeof(*infoForFirstTrackOfLastSession));
			MoreCDsMSFTimeFromBCD(infoForFirstTrackOfLastSession);
		}
	}
	return err;

}

static void TransferTOCEntry(Ptr source, CDTOCEntry *dest)
{
	// Copy entry from buffer to the caller’s array.
	
	BlockMoveData(source, dest, kCDTOCEntrySize);
	
	// Now un-BCDify each of the fields in the client’s entry.
	// But only if they’re a reasonably values.  The point field 
	// is either a BCD encoded track number, or a magic number like 
	// $A0, $A1, or $A2.  So we only de-BCD point if its in the 
	// track range.  Also, tracks beyond the last track have 255 in
	// the MSF fields which we leave there to a) signify to the caller
	// that these fields aren’t meaningful, and b) avoid an assert 
	// in MoreCDsBCDToByte.
	
	if (dest->point <= 0x99) {
		dest->point    = MoreCDsBCDToByte(dest->point);
	}
	if (dest->pMinutes != 255) {
		dest->pMinutes 	= MoreCDsBCDToByte(dest->pMinutes);
	}
	if (dest->pSeconds != 255) {
		dest->pSeconds 	= MoreCDsBCDToByte(dest->pSeconds);
	}
	if (dest->pFrames  != 255) {
		dest->pFrames 	= MoreCDsBCDToByte(dest->pFrames);
	}
}

extern pascal OSStatus MoreCDsReadTOCEntireTOC(SInt16 driveNum, MoreCDsEntireTOC *entireTOC)
	// See comment in header.
{
	OSStatus 		 err;
	ParamBlockRec 	 pb;
	Ptr				 buffer;
	ItemCount		 entryIndex;
	Ptr		 		 pointerToEntryInBuffer;
	
	assert(entireTOC != NULL);
	
	buffer = NewPtrClear(512);
	err = MoreMemError(buffer);
	if (err == noErr) {
		pb.cntrlParam.ioNamePtr  = NULL;
		pb.cntrlParam.ioVRefNum  = driveNum;
		pb.cntrlParam.ioCRefNum  = MoreGetDriveRefNum(driveNum);
		pb.cntrlParam.csCode     = kReadTOC;
		pb.cntrlParam.csParam[0] = kCDReadTOCEntireTOC;
		*((Ptr *) &pb.cntrlParam.csParam[1]) = buffer;
		err = PBControlSync(&pb);
	}
	if (err == noErr) {
		pointerToEntryInBuffer = buffer + 1;		// Point to first entry in buffer.

		// Transfer TOC entries for points A0, A1, and A2.
		//
		// When stepping to the next entry in buffer, we have to do 
		// use pointer arithmetic rather than array indexing because 
		// kCDTOCEntrySize is 5, and there’s no way to get the 
		// compiler to index an array whose elements are of an odd size.
		
		TransferTOCEntry(pointerToEntryInBuffer, &entireTOC->a0);
		pointerToEntryInBuffer += kCDTOCEntrySize;

		TransferTOCEntry(pointerToEntryInBuffer, &entireTOC->a1);
		pointerToEntryInBuffer += kCDTOCEntrySize;
		
		TransferTOCEntry(pointerToEntryInBuffer, &entireTOC->a2);
		pointerToEntryInBuffer += kCDTOCEntrySize;

		// Now transfer each of the tracks.
		
		for (entryIndex = 0; entryIndex < 99; entryIndex++) {
			TransferTOCEntry(pointerToEntryInBuffer, &entireTOC->tracks[entryIndex]);
			pointerToEntryInBuffer += kCDTOCEntrySize;
		}
	}
	
	// Clean up.
	
	if (buffer != NULL) {
		DisposePtr(buffer);
		assert( MemError() == noErr );
	}
	return err;
}

extern pascal OSStatus MoreCDsReadTOCAllQSubCodeEntries(SInt16 driveNum, UInt16 firstSession, 
											ItemCount entriesRequested,
											ItemCount *entriesReturned,
											CDQSubCodeEntry entries[])
	// See comment in header.
{
	OSStatus 		 err;
	ParamBlockRec 	 pb;
	Ptr				 buffer;
	ByteCount		 bufferSize;
	ItemCount		 entryIndex;
	ByteCount		 numberOfBytesOfEntriesReturned;
	ItemCount		 entriesToCopy;
	Ptr				 pointerToEntryInBuffer;
	CDQSubCodeEntry* thisEntry;
	
	assert(entriesReturned != NULL);
	assert(entries != NULL);
	assert(sizeof(CDMSFTime) == 4);

	bufferSize = 4 + entriesRequested * kCDQSubCodeEntrySize;
	assert(bufferSize <= 32767);
	
	buffer = NewPtr(bufferSize);
	err = MoreMemError(buffer);
	if (err == noErr) {
		pb.cntrlParam.ioNamePtr  = NULL;
		pb.cntrlParam.ioVRefNum  = driveNum;
		pb.cntrlParam.ioCRefNum  = MoreGetDriveRefNum(driveNum);
		pb.cntrlParam.csCode     = kReadTOC;
		pb.cntrlParam.csParam[0] = kCDReadTOCAllQSubCodeEntries;
		*((Ptr *) &pb.cntrlParam.csParam[1]) = buffer;
		pb.cntrlParam.csParam[3] = bufferSize;
		pb.cntrlParam.csParam[4] = MoreCDsByteToBCD(firstSession) << 8;
		err = PBControlSync(&pb);
	}
	if (err == noErr) {
		// Get the count of the number of bytes of Q sub-code entries returned, 
		// from the first two bytes of the buffer.  Note that this the count 
		// in the buffer includes two header bytes (the first and last session 
		// number), which we subtract away to get the number of bytes of 
		// actual Q sub-code entries.
		
		numberOfBytesOfEntriesReturned = *((UInt16 *) buffer) - 2;
		
		// We should have got an even number of entries back.  Let the caller 
		// know that number.  Note that "/" rounds down, so if the assert isn’t 
		// true then we’ll only return the whole entries that we found.
		
		assert( (numberOfBytesOfEntriesReturned % kCDQSubCodeEntrySize) == 0 );
		*entriesReturned = numberOfBytesOfEntriesReturned / kCDQSubCodeEntrySize;
		
		// CD-ROM driver returns the byte count of the number of entries available, 
		// which isn’t really what the documentation says but we can handle it.  
		// Basically, if we get more entries back that we asked for, lets just copy 
		// the entries we have space for.  But we want to return the full count of 
		// entries in entriesReturned so that the client knows that they've missed some.
		
		entriesToCopy = *entriesReturned;
		if (entriesToCopy > entriesRequested) {
			entriesToCopy = entriesRequested;
		}
		
		pointerToEntryInBuffer = buffer + 4;		// Point to first entry in buffer.
		for (entryIndex = 0; entryIndex < entriesToCopy; entryIndex++) {
			
			// Copy entry from buffer to the caller’s array.
			
			thisEntry = &entries[entryIndex];
			BlockMoveData(pointerToEntryInBuffer, thisEntry, kCDQSubCodeEntrySize);
			
			// Now un-BCDify each of the fields in the client’s entry.

			thisEntry->trackNumber 	= MoreCDsBCDToByte(thisEntry->trackNumber);
			thisEntry->minutes     	= MoreCDsBCDToByte(thisEntry->minutes);
			thisEntry->seconds     	= MoreCDsBCDToByte(thisEntry->seconds);
			thisEntry->frames 		= MoreCDsBCDToByte(thisEntry->frames);
			thisEntry->zero 		= MoreCDsBCDToByte(thisEntry->zero);
			thisEntry->pMinutes 	= MoreCDsBCDToByte(thisEntry->pMinutes);
			thisEntry->pSeconds 	= MoreCDsBCDToByte(thisEntry->pSeconds);
			thisEntry->pFrames 		= MoreCDsBCDToByte(thisEntry->pFrames);
			thisEntry->pad 			= 0;

			// Step to the next entry in buffer.  We have to do this 
			// using pointer arithmetic rather than array indexing because 
			// kCDQSubCodeEntrySize is 11, and there’s no way to get the 
			// compiler to index an array whose elements are of an odd size.
			
			pointerToEntryInBuffer += kCDQSubCodeEntrySize;
		}
	}
	
	// Clean up.
	
	if (buffer != NULL) {
		DisposePtr(buffer);
		assert( MemError() == noErr );
	}
	
	return err;
}

static OSStatus BuildTrackListFromStartsArray(UInt16 firstTrack, UInt16 trackCount, 
											CDMSFTime *					trackStarts,
											CDLBATime					endOfLastTrack,
											MoreCDsAudioTrackListHandle trackList)
	// This routine builds a track list array in the trackList handle 
	// based on the track information supplied in firstTrack, trackCount, 
	// trackStarts, and endOfLastTrack.  The track numbers are one-based, 
	// although trackStarts is zero-based (confusing huh? wish I was 
	// programming in Pascal!).
{
	OSStatus  err;
	UInt16    trackIndex;
	CDLBATime thisTrackStartTime;
	CDLBATime nextTrackStartTime;
	
	assert(firstTrack > 0);		// firstTrack is a track number, not an index into trackStarts
	assert(trackCount > 0);		// If there are no audio tracks, we should not have been called.
	
	SetHandleSize( (Handle) trackList, trackCount * sizeof(MoreCDsAudioTrackEntry) );
	err = MemError();
	if (err == noErr) {
		// Now fill out all of the previous entries in the track list 
		// array, using the algorithm that length(track N) is 
		// start(track N + 1) - start(track N).  Note that the last track 
		// is special because its end is actually the start of the lead out.
		
		nextTrackStartTime = endOfLastTrack;
		trackIndex = trackCount;
		while (trackIndex != 0) {
			trackIndex -= 1;
			(*trackList)[trackIndex].trackNumber = firstTrack + trackIndex;
			thisTrackStartTime                   = MoreCDsMSFTimeToLBATime(&trackStarts[(firstTrack - 1) + trackIndex]);
			(*trackList)[trackIndex].trackStart  = thisTrackStartTime;
			(*trackList)[trackIndex].trackLength = nextTrackStartTime - thisTrackStartTime;
			nextTrackStartTime = thisTrackStartTime;
		}
	}
	return err;
}

static void AdjustEndOfLastAudioTrackOnEnhancedCD(SInt16 driveNum, UInt16 lastAudioTrack, CDLBATime *endOfLastAudioTrack)
	// There are a variety of ways to mix audio and data tracks on a CD.  One common 
	// method is known as a "pressed multisession CD", specified in the Blue Book and 
	// marketted under various names like "Enhanced CD" or "CD Extra".  In this approach 
	// the CD contains two sessions: the first session contains audio tracks and the 
	// second session contains a data track (typically in ISO-9660 format).  To retain 
	// compatibility with existing CD players, there is an extended lead out (150 seconds) 
	// after the last audio track and before the first data track.  [My understanding is 
	// that really old and dumb CD players require this lead out to detect the end of the 
	// disc.]  Moreover, there is a second lead out after the last data track.
	//
	// This structure causes problems for the traditional algorithm for calculating track 
	// lengths:
	// 
	// 1. length(track N) := start(track N+1) - start(track N)
	// 2. length of last track := start(lead out) - start(last track)
	// 
	// because start(lead out) (as returned by MoreCDsLastSessionLeadOut) is the start of 
	// the second lead out (after the data tracks) not the start of the first lead out.
	// 
	// To get around this problem we read a low-level representation of the TOC and 
	// look for the A2 TOC entry.  The Read Book (CD-DA) standard defines that point 
	// A2 references the start of the lead out, and for an Enhanced CD this lead out 
	// is the one between the audio and the data.  This address becomes the end of 
	// the last audio track as far as our calculations are concerned.
	//
	// Note that the driver calls required by this routine are not supported on 
	// all Apple CD players, so this routine is expected to fail on that hardware.
	// Rather than fail the entire operation, however, this routine is defined to 
	// ignore the error and preserve the value in *endOfLastAudioTrack.  If 
	// you're running on old hardware you may get a bogus last track length for 
	// Enhanced CDs (then again, maybe the old hardware will only see the first 
	// lead out, and so give the right result -- I don’t have access to the old 
	// hardware on which to test this).
{
	OSStatus 		err;
	UInt16			sessionCount;
	UInt16			sessionIndex;
	CDQSubCodeEntry qSubCodeEntries[3];
	ItemCount		qSubCodeEntriesReturned;

	// Start by getting a count of the sessions on the disc.
	
	err = MoreCDsReadTOCSessionInfo(driveNum, NULL, &sessionCount, NULL, NULL);
	if (err == noErr) {
	
		// Now iterate over each session that has an A1 and A2 TOC entry, looking 
		// for a session whose A1 TOC entry refers to our last audio track.  If we 
		// find that session then its A2 TOC entry is the start of the audio lead out.
		//
		// For those unfamiliar with the Red Book (CD-DA) standard, TOC entry A0 
		// holds the first track number in its PMIN field (pMinutes in our data 
		// structure), TOC entry A1 holds the last track number, and TOC entry A2 
		// holds the address of the audio lead out.
		//
		// Note that MoreCDsAllQSubCodeEntries returns A0, A1, and A2 in that order 
		// before the actual track TOC entries.  In this case we’re only asking 
		// for 3 entries so we only get those entries.
		// 
		// This gist of this code was stolen from the "Audio CD Extension".
		
		for (sessionIndex = 1; sessionIndex <= sessionCount; sessionIndex++) {
			err = MoreCDsReadTOCAllQSubCodeEntries(driveNum, sessionIndex, 3, &qSubCodeEntriesReturned, qSubCodeEntries);
			if (   (err == noErr)										// If driver supports call, and
				&& (qSubCodeEntriesReturned  >= 3)						// Enough TOC entries for audio on this session, and
				&& (qSubCodeEntries[1].point == 0xA1)					// TOC entry A1 is present, and
				&& (qSubCodeEntries[2].point == 0xA2)					// TOC entry A1 is present, and
				&& (lastAudioTrack == qSubCodeEntries[1].pMinutes)		// Right Ax TOC entries.
			   ) {
				*endOfLastAudioTrack = MoreCDsMinSecFrameToLBATime(qSubCodeEntries[2].pMinutes, qSubCodeEntries[2].pSeconds, qSubCodeEntries[2].pFrames);
				break;
			}
		}
	}
	// Ignore error.  See comments above.
}

extern pascal OSStatus MoreCDsAudioTrackList(SInt16 driveNum, MoreCDsAudioTrackListHandle trackList)
	// See comment in header.
{
	OSStatus 		err;
	UInt16			firstTrack;
	UInt16			trackCount;
	UInt16   		firstAudioTrack;
	UInt16			audioTrackCount;
	UInt16			trackIndex;
	CDMSFTime *		trackStarts;
	CDLBATime		endOfLastAudioTrack;
	Boolean			allAudioTracks;
	Boolean			allDataTracks;

	trackStarts = NULL;

	// Get the first track number and a count of the tracks.
			
	err = MoreCDsReadTOCFirstAndLastTrack(driveNum, &firstTrack, NULL, &trackCount);
	if (trackCount == 0) {
		err = -1;				// We’re not going anywhere near a disc with no tracks!
	}

	// Read each track’s control info and start time into trackStarts.
	
	if (err == noErr) {
		trackStarts = (CDMSFTime *) NewPtr(sizeof(*trackStarts) * trackCount);
		err = MoreMemError(trackStarts);
	}
	if (err == noErr) {
		err = MoreCDsReadTOCTrackStarts(driveNum, firstTrack, trackCount, trackStarts);
	}

	// Scan the track list to determine whether all tracks are audio or data.
	
	if (err == noErr) {
		allAudioTracks = true;
		allDataTracks = true;
		
		for (trackIndex = 0; trackIndex < trackCount; trackIndex++) {
			if (trackStarts[trackIndex].control & kCDControlDataTrackMask) {
				allAudioTracks = false;
			} else {
				allDataTracks = false;
			}
		}
		if (allDataTracks) {
			err = -2;			// If the disc is just a data disc, return an error.
		}
	}

	// Get the start of the last session’s lead out.  For a normal 
	// (single session) audio disc this is the end of the last audio track.
	
	if (err == noErr) {
		err = MoreCDsReadTOCLastSessionLeadOut(driveNum, &endOfLastAudioTrack);
	}

	if (err == noErr) {

		// If no error at this point, then we know that we have at least one 
		// audio track.  Let’s establish preliminary values for firstAudioTrack 
		// and audioTrackCount.
		
		firstAudioTrack = firstTrack;
		audioTrackCount = trackCount;
		
		// If we have any data tracks, go make some adjustments (voodoo).
		
		if (!allAudioTracks) {
			// If the disc starts with data tracks, remove them from the beginning of the disc 
			// by incrementing firstAudioTrack and decrementing audioTrackCount.  This handles 
			// discs like "Phil and Dave’s Excellent CD", where the first track is a 
			// data track.
			
			while (    (audioTrackCount > 0)
					&& ((trackStarts[firstAudioTrack - 1].control & kCDControlDataTrackMask) != 0) ) {
				firstAudioTrack += 1;
				audioTrackCount -= 1;
			}
			if (audioTrackCount == 0) {
				// This is weird.  The first scan of the tracks indicates that there 
				// was an audio track, but now we can’t find it.
				assert(false);
				err = -3;
			}
			
			// If the disc ends with data tracks, remove them from the end of the disc 
			// by decrementing audioTrackCount and by setting endOfLastAudioTrack to the start 
			// of the track we deleted.
			
			if (err == noErr) {
				// Set trackIndex to the index of the last track that we think is 
				// an audio track.  We subtract two because a) firstAudioTrack is one-based 
				// and trackStarts is zero-based, and b) track number + count - 1 is the track 
				// number of the last track.
				
				trackIndex = firstAudioTrack + audioTrackCount - 1 - 1;

				while ( (trackIndex > 0) && ((trackStarts[trackIndex].control & kCDControlDataTrackMask) != 0)) {
					endOfLastAudioTrack = MoreCDsMSFTimeToLBATime(&trackStarts[trackIndex]);
					audioTrackCount -= 1;
					trackIndex -= 1;
				}
			}
			
			// Now we handle a super-wacky special case involving Enhanced CDs. 
			// See the comments for the routine for more information.
			
			if (err == noErr) {
				AdjustEndOfLastAudioTrackOnEnhancedCD(driveNum, (firstAudioTrack + audioTrackCount - 1), &endOfLastAudioTrack);
			}
		}
	}

	// Now place the results into the handle supplied by the user.
	
	if (err == noErr) {
		err = BuildTrackListFromStartsArray(firstAudioTrack, audioTrackCount, trackStarts, endOfLastAudioTrack, trackList);
	}

	// Clean up.
	
	if (trackStarts != NULL) {
		DisposePtr( (Ptr) trackStarts );
		assert( MemError() == noErr );
	}
	
	return err;
}


extern pascal void MoreIsDriveCDROM(SInt16 drive, MoreCDsCDROMResponse *response)
	// See comment in header.
{
	DriverRefNum refNum;
	DriverFlags drvrFlags;
	Str255 drvrName;
    DriverGestaltParam pb;
	
	assert(drive > 0);
	assert(response != NULL);
	
	*response = kMoreDriveUnableToDetermineCDROM;
	
	refNum = MoreGetDriveRefNum(drive);
	if (refNum != 0) {
		if ( TradGetDriverInformation(refNum, NULL, &drvrFlags, drvrName, NULL) == noErr ) {
		
			// Step 1 -- if the driver supports driver gestalt, we
			// exclusively rely on its response for the kdgDeviceType
			// selector.
			
			if ( TradDriverGestaltIsOn(drvrFlags) ) {
		        pb.ioVRefNum = drive;
		        pb.ioCRefNum = refNum;
		        pb.csCode = kDriverGestaltCode;
		        pb.driverGestaltSelector = kdgDeviceType;

		        if ( PBStatusSync((ParmBlkPtr) &pb) == noErr ) {
		        	if (GetDriverGestaltDevTResponse(&pb)->deviceType == kdgCDType) {
		        	    *response = kMoreDriveIsCDROM;
		        	} else {
		        	    *response = kMoreDriveIsNotCDROM;
		        	}
		        }
			}
			
			// Step 2 -- if the above didn't work, we only say it's a CD-ROM
			// if the driver is ".AppleCD".

			if (*response == kMoreDriveUnableToDetermineCDROM) {
				if ( EqualString(drvrName, "\p.AppleCD", false, true) ) {
		       	    *response = kMoreDriveIsCDROM;
				} else if ( EqualString(drvrName, "\p.AFPTranslator", false, true) ) {
					// ".AFPTranslator" does not respond to Driver Gestalt,
					// which is pretty lame IMHO.  [Radar ID ]  Regardless,
					// we know it's not a CD-ROM.
		       	    *response = kMoreDriveIsNotCDROM;
				} else if ( EqualString(drvrName, "\p.Sony", false, true) ) {
		       	    *response = kMoreDriveIsNotCDROM;
				}
			}
		}
	}
}

#endif
