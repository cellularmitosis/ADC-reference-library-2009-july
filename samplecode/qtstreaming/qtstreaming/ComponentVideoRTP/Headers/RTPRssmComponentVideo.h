/*
	File:		RTPRssmComponentVideo.h

	Contains:	Declarations for Component Video RTPReassembler
	
	Copyright:	© 1997-1999 by Apple Computer, Inc., all rights reserved.

*/

#ifndef __RTPRSSMCOMPONENTVIDEO__
#define __RTPRSSMCOMPONENTVIDEO__

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
 *	An RTPRssmComponentVideoInstanceData structure stores instance variables
 *	for a Component Video RTPReassembler instance.  The structure declares the
 *	following fields:
 *	
 *		itself						the RTPReassembler instance that maintains
 *									this data structure
 *		
 *		itsBase						the RTPReassembler to which the instance
 *									delegates calls
 *		
 *		itsFinalDerivation			the RTPReassembler instance to which the
 *									instance targets calls to itself
 *		
 *		itsPayloadAttributes		cached attributes of incoming payloads
 *		
 *		itsSavedChunk				the most recent frame of reassembled
 *									sample data
 *
 */

typedef struct
{
	RTPReassembler			itself;
	RTPReassembler			itsBase;
	RTPReassembler			itsFinalDerivation;
	ComponentVideoPayload	itsPayloadAttributes;
	SHChunkRecord *			itsSavedChunk;
} RTPRssmComponentVideoInstanceData;



#endif /* __RTPRSSMCOMPONENTVIDEO__ */
