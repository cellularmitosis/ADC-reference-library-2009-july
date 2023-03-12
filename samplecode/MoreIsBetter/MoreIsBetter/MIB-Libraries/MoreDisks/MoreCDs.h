/*
	File:		MoreCDs.h

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

$Log: MoreCDs.h,v $
Revision 1.4  2002/11/08 23:27:25         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.3  2001/11/07 15:51:36         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>      5/7/01    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <MacTypes.h>
	#include <Devices.h>
	#include <DriverGestalt.h>
#endif

// MIB Interfaces

#include "MoreDisks.h"			// CD-ROM control and status selectors are defined here

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// Carbon

// Most of these routines aren’t in Carbon because there’s no way 
// to do this using Carbon APIs.  You either have to use Device 
// Manager (on Mac OS 9) or IOKit (on Mac OS X).  I might create 
// a suitable abstraction layer that works on both platforms 
// at some point, but right now this stuff is Mac OS 9 only.

/////////////////////////////////////////////////////////////////
// Terminology

// If you're confused by CD terminology, try looking in the various 
// glossaries on the Internet.  One useful example is:
//
//     <http://www.discusa.com/glossary/glomain.htm>

/////////////////////////////////////////////////////////////////
// Compact Disc Basic Data Structures

// These structures reflect low-level data structures present on 
// the CD.  As such, all of these structures must be in 68K 
// alignment.

// CDQSubCodeEntry is a structure that holds a TOC entry (Q sub-code).

// CDMSFTime is the standard way of returning a position on the CD 
// in MSF (minutes, seconds, frames) format.  It also contains a 
// control field, which is convenient in many cases.

// CDLBATime is an equivalent way of representing a position on the CD. 
// It is a count of frames from the beginning of the disc (logical block 
// addressing).  MoreCDs uses these units in all APIs because they are 
// much easier to understand and operate upon.

// Note:
// Fields marked with "*" are BCD when read from the driver but are converted 
// to binary by all MoreCDs APIs.

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

struct CDQSubCodeEntry {
	UInt8 sessionNumber;
	UInt8 controlAndADR;
	UInt8 trackNumber;			// *
	UInt8 point;
	UInt8 minutes;				// *
	UInt8 seconds;				// *
	UInt8 frames;				// *
	UInt8 zero;					// *
	UInt8 pMinutes;				// *
	UInt8 pSeconds;				// *
	UInt8 pFrames;				// *
	UInt8 pad;
};
typedef struct CDQSubCodeEntry CDQSubCodeEntry;

// IMPORTANT:
// I’ve added a pad byte at the end of the CDQSubCodeEntry structure because 
// Mac OS compilers put one there for you anyway: you just can’t define a 
// structure with an odd size.  So, instead of using sizeof(CDQSubCodeEntry)
// you have to use the symbolic constant defined below.

struct CDMSFTime {
	UInt8  control;
	UInt8  minutes;				// *
	UInt8  seconds;				// *
	UInt8  frames;				// *
};
typedef struct CDMSFTime CDMSFTime;

enum {
	kCDQSubCodeEntrySize = 11
};

struct CDTOCEntry {
	UInt8	control;
	UInt8	point;				// *
	UInt8	pMinutes;			// *
	UInt8	pSeconds;			// *
	UInt8	pFrames;			// *
	UInt8	pad;
};
typedef struct CDTOCEntry CDTOCEntry;

enum {
	kCDTOCEntrySize = 5
};

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

typedef UInt32 CDLBATime;

// Flags for the control field of CDMSFTime.  These are defined by the 
// various CD standards.

enum {
	kCDControlAudioPreEmphasisBit  = 0,		// bit 0’s meaning depends on track type
	kCDControlDataIncrementalBit   = 0,
	kCDControlCopyPermittedBit     = 1,
	kCDControlDataTrackBit         = 2,
	kCDControlAudioFourChannelsBit = 3,

	kCDControlAudioPreEmphasisMask  = 0x01,
	kCDControlDataIncrementalMask   = 0x01,
	kCDControlCopyPermittedMask     = 0x02,
	kCDControlDataTrackMask         = 0x04,
	kCDControlAudioFourChannelsMask = 0x08
};

/////////////////////////////////////////////////////////////////
// CD-ROM Basics

// These routines convert to and from common CD-ROM data structures.

extern pascal UInt8 MoreCDsBCDToByte(UInt8 bcd);
	// Converts a binary coded decimal (BCD) value 
	// to its binary representation.

extern pascal UInt8 MoreCDsByteToBCD(UInt8 byte);
	// Convert a binary number to its BCD representation.

extern pascal void MoreCDsLBATimeToMSFTime(CDLBATime lbaTime, CDMSFTime *msfTime);
	// Convert a logical block address CD time to a 
	// minutes-seconds-frames CD time.

extern pascal CDLBATime MoreCDsMSFTimeToLBATime(const CDMSFTime *msfTime);
	// Convert a minutes-seconds-frames CD time to a 
	// logical block address CD time.

extern pascal CDLBATime MoreCDsMinSecFrameToLBATime(UInt8 minutes, UInt8 seconds, UInt8 frames);
	// Convert a minutes-seconds-frames CD time (specified 
	// as individual values) to a logical block address CD time.

extern pascal void MoreCDsMSFTimeFromBCD(CDMSFTime *msfTime);
	// Converts the fields of a CDMSFTime structure from 
	// BCD to binary.

extern pascal void MoreCDsMSFTimeToBCD(CDMSFTime *msfTime);
	// Converts the fields of a CDMSFTime structure from 
	// binary to BCD.

/////////////////////////////////////////////////////////////////
// Read TOC Operations

// The traditional Mac OS CD-ROM driver API supports 6 different 
// "read table of contents" operations.  These routines provide 
// simple abstraction layers on top of the raw driver calls.

// Sub-operation codes for the kReadTOC CD-ROM driver Control request.

enum {
	kCDReadTOCFirstAndLastTrack 	= 1,
	kCDReadTOCLastSessionLeadOut 	= 2,
	kCDReadTOCTrackStarts 			= 3,
	kCDReadTOCEntireTOC 			= 4,			// not available on all drives
	kCDReadTOCSessionInfo 			= 5,			// not available on all drives
	kCDReadTOCAllQSubCodeEntries 	= 6				// not available on all drives
};

#if CALL_NOT_IN_CARBON

extern pascal OSStatus MoreCDsReadTOCFirstAndLastTrack(SInt16 driveNum, UInt16 *firstTrack, UInt16 *lastTrack, UInt16 *trackCount);
	// Returns information about the first, last, and total number 
	// of tracks on the CD.  This is basically a nice wrapper around 
	// kReadTOC/kCDReadTOCFirstAndLastTrack.  Track numbers are one-based.
	// Any of the output parameters may be NULL.

extern pascal OSStatus MoreCDsReadTOCLastSessionLeadOut(SInt16 driveNum, CDLBATime *leadOut);
	// Returns the address of the start of last session’s lead out.
	// This is basically a nice wrapper around kReadTOC/kCDReadTOCLastSessionLeadOut.
	// leadOut must not be NULL.

extern pascal OSStatus MoreCDsReadTOCTrackStarts(SInt16 driveNum, UInt16 firstTrack, UInt16 trackCount, 
									CDMSFTime trackStarts[]);
	// Returns information about tracks firstTrack..(firstTrack+trackCount-1)
	// in the buffer pointed to by trackStarts.  That buffer should have at 
	// least trackCount entries.  The returned information includes the 
	// track start time and the track control flags.
	// This is basically a nice wrapper around kReadTOC/kCDReadTOCTrackStarts.  
	// Track numbers are one-based, although the trackStarts array is zero-based.
	// trackStarts must not be NULL.

extern pascal OSStatus MoreCDsReadTOCSessionInfo(SInt16 driveNum, UInt16 *firstSession, UInt16 *lastSession, 
													UInt16 *firstTrackOfLastSession,
													CDMSFTime *infoForFirstTrackOfLastSession);
	// Returns information about the sessions on a CD.  The information returned in 
	// infoForFirstTrackOfLastSession includes the track start time and the track 
	// control flags.
	//
	// This is basically a nice wrapper around kReadTOC/kCDReadTOCSessionInfo.  
	// Track numbers are one-based.  Session numbers are one-based.  This call 
	// can fail (probably with a controlErr) if the CD-ROM drive is too old.
	// Any of the output parameters may be NULL.

struct MoreCDsEntireTOC {
	CDTOCEntry	a0;
	CDTOCEntry	a1;
	CDTOCEntry	a2;
	CDTOCEntry	tracks[99];
};
typedef struct MoreCDsEntireTOC MoreCDsEntireTOC;

extern pascal OSStatus MoreCDsReadTOCEntireTOC(SInt16 driveNum, MoreCDsEntireTOC *entireTOC);
	// This is basically a nice wrapper around kReadTOC/kCDReadTOCEntireTOC. 
	// It reads TOC entries from the CD and copies them to the entireTOC 
	// structure.
	// 
	// The underlying operation is not supported by all Apple CD-ROM drives. 
	// If you get an error, probably controlErr, it means that the drive 
	// doesn’t support this operation.

extern pascal OSStatus MoreCDsReadTOCAllQSubCodeEntries(SInt16 driveNum, UInt16 firstSession, 
											ItemCount entriesRequested,
											ItemCount *entriesReturned,
											CDQSubCodeEntry entries[]);
	// This is basically a nice wrapper around kReadTOC/kCDReadTOCAllQSubCodeEntries.
	// Returns low-level TOC information for the session specified by 
	// firstSession.  The information is returned in the array pointed 
	// to by entries.  That array must include at least entriesRequested 
	// element.  entriesReturned may be NULL.  If it’s not NULL it is set 
	// to the number of TOC entries available.  You can use this to 
	// resize the entries array and call this routine again to get all 
	// the TOC entries.
	// 
	// The underlying operation is not supported by all Apple CD-ROM drives. 
	// If you get an error, probably controlErr, it means that the drive 
	// doesn’t support this operation.
	//
	// IMPORTANT:
	// CDQSubCodeEntry is a true array, which means that you can index 
	// it using C array indexing notation.  Furthermore, any field that 
	// is returned in BCD by the driver has been converted to binary.
	//
	// The CD driver call on which this is layered is defined to return 
	// TOC entries A0, A1, and A2 in that order before any track TOC entries.
	//
	// This call does not return the session information that is
	// part of the header of the data returned by the underlying CD-ROM 
	// driver call.  The reason?  You can just as easily get these 
	// numbers using MoreCDsSessionInfo.

#endif

/////////////////////////////////////////////////////////////////
// High-Level Operations

// The MoreCDsAudioTrackEntry structure is used to by MoreCDsAudioTrackList to 
// return information about the tracks on a disc.

struct MoreCDsAudioTrackEntry {
	UInt16 		trackNumber;				// The CD track number of this track.
	CDLBATime 	trackStart;					// The start of the track, in frames.
	CDLBATime 	trackLength;				// The length of the track, in frames.
};
typedef struct MoreCDsAudioTrackEntry MoreCDsAudioTrackEntry, *MoreCDsAudioTrackListPtr, **MoreCDsAudioTrackListHandle;

#if CALL_NOT_IN_CARBON

extern pascal OSStatus MoreCDsAudioTrackList(SInt16 driveNum, MoreCDsAudioTrackListHandle trackList);
	// Generates a list of all the audio tracks on the disc in the drive 
	// whose drive number is driveNum.  You must create the trackList handle 
	// and pass it into this routine; the routine will resize it appropriately.
	// 
	// This routine will return an error if driveNum is not on an audio disc.

#endif

/////////////////////////////////////////////////////////////////
// Identifying CD-ROM Drives

typedef UInt32 MoreCDsCDROMResponse;
enum {
	kMoreDriveUnableToDetermineCDROM = 0,
	kMoreDriveIsCDROM = 1,
	kMoreDriveIsNotCDROM = 2
};

#if CALL_NOT_IN_CARBON

extern pascal void MoreIsDriveCDROM(SInt16 drive, MoreCDsCDROMResponse *response);
	// Sets *response to one of the constants shown above to indicate
	// whether the drive is a CD-ROM drive.

#endif

#ifdef __cplusplus
}
#endif
