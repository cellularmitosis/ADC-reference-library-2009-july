/*
	File:		RTPMPComponentVideo.h

	Contains:	Declarations for Component Video RTPMediaPacketizer
	
	Copyright:	� 1997-1999 by Apple Computer, Inc., all rights reserved.

*/

#ifndef __RTPMPCOMPONENTVIDEO__
#define __RTPMPCOMPONENTVIDEO__

#pragma once



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#include <QTStreamingComponents.h>
#include "ComponentVideoRTP.h"



/* ---------------------------------------------------------------------------
 *		D A T A T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	An RTPMPComponentVideoInstanceData structure stores instance variables for
 *	a Component Video RTPMediaPacketizer instance.  The structure declares the
 *	following fields:
 *	
 *		itself						the RTPMediaPacketizer instance that
 *									maintains this data structure
 *		
 *		itsBase						the RTPMediaPacketizer to which the
 *									instance delegates calls
 *		
 *		itsFinalDerivation			the RTPMediaPacketizer instance to which
 *									the instance targets calls to itself
 *		
 *		itsInitialized				true when the instance has initialized the
 *									remaining fields
 *		
 *		itsSampleDescriptionSeed	the seed of the SampleDescription the
 *									instance is using for sample data
 *		
 *		itsMediaTimeScale			the TimeScale of the source sample data
 *		
 *		itsMediaTimeBase			the TimeBase passed to RTPMPSetTimeBase()
 *		
 *		itsFrameDataSize			number of octets in one frame of sample
 *									data
 *		
 *		itsPayloadDataSize			number of octets of sample data the
 *									instance will include in most payloads
 *		
 *		itsPacketBuilder			the RTPMPPacketBuilder the instance uses
 *									to contruct network packets
 *		
 *		itsPacketSizeLimit			the maximum allowable size, in octets, of
 *									payloads the instance may contruct
 *		
 *		itsPacketDurationLimit		the maximum allowable duration of sample
 *									the instance may encapsulate in a single
 *									payload
 *		
 *		itsPayloadHeader			precomputed values for header the instance
 *									includes with each payload
 *		
 */

typedef struct
{
	RTPMediaPacketizer		itself;
	RTPMediaPacketizer		itsBase;
	RTPMediaPacketizer		itsFinalDerivation;
	Boolean					itsInitialized;
	UInt32					itsSampleDescriptionSeed;
	TimeScale				itsMediaTimeScale;
	TimeBase				itsMediaTimeBase;
	UInt32					itsFrameDataSize;
	UInt32					itsPayloadDataSize;
	RTPPacketBuilder		itsPacketBuilder;
	UInt32					itsPacketSizeLimit;
	UInt32					itsPacketDurationLimit;
	ComponentVideoPayload	itsPayloadHeader;
} RTPMPComponentVideoInstanceData;



#endif /* __RTPMPCOMPONENTVIDEO__ */
