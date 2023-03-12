/*
	File:		RTPRssmIMAAudio.c

	Contains:	Definition of IMA Audio RTPReassembler

	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved.

	
	
	OVERVIEW
	
	QuickTime Streaming software uses an RTPReassembler to extract sample data
	from incoming RTP packets.  A base RTPReassembler defined by QuickTime
	Streaming does much of the work of reassembling.  A derived RTPReassembler
	extends the base to interpret a specific RTP payload format, to select an
	appropriate StreamHandler to process reassembled sample data, and to
	handle network packet loss.
	
	This reassembler extends or fully implements the following interface and
	delegates all other calls to its base.
		
		
		STANDARD COMPONENT INTERFACE
		----------------------------
		
		CallComponentOpen()				Allocate and initialize storage for
										instance variables.
		
		CallComponentClose()			Reverse the effects of
										CallComponentOpen().
		
		CallComponentVersion()			Return the instance's version.
		
		CallComponentTarget()			Update the instance's inheritance graph.
		
		
		
		RTP REASSEMBLER INTERFACE
		------------------------------
		
		RTPRssmInitialize()				Prepare to reassemble sample data.
		
		RTPRssmComputeChunkSize()		Determine the size of buffer needed for
										the data in a list of network packets.
		
		RTPRssmAdjustPacketParams()		Adjust fields of an RTPRssmPacket to
										reflect the properties of its payload.
		
		RTPRssmCopyDataToChunk()		Copy sample data from a packet list to
										a data buffer.
		
		RTPRssmSendPacketList()			Prepare to process or discard a list of
										packets.
		
		RTPRssmHasCharacteristic()		Indicate whether the reassembler has the
										specified characteristic.
		
		RTPRssmReset()					End processing of the current stream of
										network packets.
*/



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#include "RTPRssmIMAAudio.h"
#include "RTPRssmIMAAudioResources.h"
#include <FixMath.h>
#include <string.h>



/* ---------------------------------------------------------------------------
 *		R T P    R E A S S E M B L E R    P R O T O T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	QTStreamingComponents.k.h uses these macros to declare prototypes for
 *	the RTPReassembler calls defined in this file.
 *
 */

#define RTPRSSM_BASENAME()				RTPRssmIMAAudio_
#define RTPRSSM_GLOBALS()				RTPRssmIMAAudioInstanceData **

#include <QTStreamingComponents.k.h>



/* ---------------------------------------------------------------------------
 *		C O M P O N E N T    D I S P A T C H    H E L P E R
 * ---------------------------------------------------------------------------
 *
 *	ComponentDispatchHelper.c uses these macros to define a dispatcher and to
 *	declare prototypes for the core component calls defined in this file.  For
 *	Mac OS, it defines the routine descriptor that serves as the component
 *	entry point.  The name of the routine descriptor is the macro expansion of
 *		
 *		CALLCOMPONENT_BASENAME()##ComponentDispatchRD
 *	
 *	The name of the dispatcher is the macro expansion of
 *	
 *		CALLCOMPONENT_BASENAME()##ComponentDispatch
 *
 */

#define CALLCOMPONENT_BASENAME()		RTPRSSM_BASENAME()
#define CALLCOMPONENT_GLOBALS()			RTPRSSM_GLOBALS() storage
#define COMPONENT_DISPATCH_FILE			"RTPRssmIMAAudioDispatch.h"
#define COMPONENT_C_DISPATCHER			1
#define COMPONENT_UPP_SELECT_ROOT()		RTPRssm
#define GET_DELEGATE_COMPONENT()		( ( **storage ).itsBase )

#include <ComponentDispatchHelper.c>



#pragma mark *        INTERNAL IMPLEMENTATION
#pragma mark -
/* ---------------------------------------------------------------------------
 *		I N T E R N A L    I M P L E M E N T A T I O N
 * ---------------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------------
 *		__GetNewSampleDescription()
 * ---------------------------------------------------------------------------
 *
 *	Create a new SampleDescription for this reassembler's payload using the
 *	given payload attributes.
 *
 */

static
ComponentResult
__GetNewSampleDescription(
	const IMAAudioPayload *		inPayloadAttributes,
	SampleDescriptionHandle *	outDescription )
{
	ComponentResult			theResult = noErr;
	SoundDescriptionHandle	theDescription;
	
	
	theDescription =
		REINTERPRET_CAST( SoundDescriptionHandle )(
			NewHandleClear( sizeof( **theDescription ) ) );
	
	if( theDescription )
	{
		*outDescription = REINTERPRET_CAST( SampleDescriptionHandle )( theDescription );
		
		( **theDescription ).descSize = sizeof( **theDescription );
		( **theDescription ).dataFormat = kIMAAudioDataFormat;
		( **theDescription ).resvd1 = 0;
		( **theDescription ).resvd2 = 0;
		( **theDescription ).dataRefIndex = 0;
		( **theDescription ).version = 0;
		( **theDescription ).revlevel = 0;
		( **theDescription ).vendor = kComponentManufactureType;
		
		( **theDescription ).numChannels =
			IMAAudioPayloadChannelCount( inPayloadAttributes );
		
		( **theDescription ).sampleSize = 16;
		( **theDescription ).compressionID = 0;
		( **theDescription ).packetSize = 0;
		( **theDescription ).sampleRate =
			IMAAudioPayloadSampleRate( inPayloadAttributes );;
	}
	
	else
	{
		*outDescription = 0;
		
		theResult = MemError();
		
		if( theResult == noErr )
			theResult = memFullErr;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__GetPayload()
 * ---------------------------------------------------------------------------
 *
 *	Return a pointer to the start of a packet's payload.
 *
 */

static
IMAAudioPayload *
__GetPayload(
	RTPRssmPacket *		inPacket )
{
	return( 
		REINTERPRET_CAST( IMAAudioPayload * )(
			&inPacket->streamBuffer->rptr[ inPacket->transportHeaderLength ] ) );
}



/* ---------------------------------------------------------------------------
 *		__FillMissingData()
 * ---------------------------------------------------------------------------
 *
 *	Synthesize data for missing audio frames.  This implementation zeros out
 *	missing frames to synthesize silence.
 */

static
void
__FillMissingData(
	RTPRssmPacket *		inPackets,
	UInt16				inGroupStartSequenceNumber,
	UInt32				inInterleaveCount,
	SHChunkRecord *		inChunk )
{
	IMAAudioFrame *		theFrames;
	UInt32				theFrameCount;
	UInt16				theExpectedSequenceNumber;
	RTPRssmPacket *		thePacket;
	UInt16				theMissingPacketCount;
	UInt32				theIndex;
	
	
	/*
		For now, this function is just for looks, as it doesn't actually
		produce the desired audible effect.  The problem is that the
		decompressor doesn't use all data in each frame, so it doesn't
		necessarily decode zeroed frames as silence, and inserting zeroed
		frames into the data stream usually distorts the decoding of some
		subsequent frames.
		
		IMPORTANT	The sequence number computations in this function work
					because the packets are in order and the separation
					between the packets is guaranteed to be within the
					precision of the RTP sequence number field (i.e., less
					than 16 bits, or 65536 packets).  When computing with
					sequence numbers of packets that might be separated by
					more than 65535 packets, a reassembler must extend the
					precision of the sequence numbers, for instance by
					using the inNumWraparounds parameter of
					RTPRssmHandleNewPacket() or by using the timeStamp
					field of RTPRssmPacket structures.
	*/
	
	theFrames =
		REINTERPRET_CAST( IMAAudioFrame * )( CONST_CAST( UInt8 * )( inChunk->dataPtr ) );
	
	theFrameCount = inChunk->dataSize / sizeof( IMAAudioFrame );
	
	theExpectedSequenceNumber = inGroupStartSequenceNumber;
	
	for( thePacket = inPackets; thePacket; thePacket = thePacket->next )
	{
		theMissingPacketCount = thePacket->sequenceNum - theExpectedSequenceNumber;
		
		if( theMissingPacketCount )
		{
			for(
				theIndex = theExpectedSequenceNumber - inGroupStartSequenceNumber;
				theIndex < theFrameCount; theIndex += inInterleaveCount )
			{
				memset(
					&theFrames[ theIndex ], 0,
					theMissingPacketCount * sizeof( IMAAudioFrame ) );
			}
		}
		
		theExpectedSequenceNumber = thePacket->sequenceNum + 1;
	}
	
	theMissingPacketCount =
		inGroupStartSequenceNumber + inInterleaveCount - theExpectedSequenceNumber;
	
	if( theMissingPacketCount )
	{
		for(
			theIndex = theExpectedSequenceNumber - inGroupStartSequenceNumber;
			theIndex < theFrameCount; theIndex += inInterleaveCount )
		{
			memset(
				&theFrames[ theIndex ], 0,
				theMissingPacketCount * sizeof( IMAAudioFrame ) );
		}
	}
}



/* ---------------------------------------------------------------------------
 *		__InterleaveGroupDataSize()
 * ---------------------------------------------------------------------------
 *
 *	Compute the number of octets of data in the interleave group to which the
 *	given packet belongs.
 *
 */

static
UInt32
__InterleaveGroupDataSize(
	RTPRssmPacket *		inPacket )
{
	UInt32						theResult;
	const IMAAudioPayload *		thePayload;
	UInt32						theInterleaveCount;
	UInt32						theInterleaveIndex;
	UInt32						theInterleaveTail;
	
	
	thePayload = __GetPayload( inPacket );
	
	theInterleaveCount = IMAAudioPayloadInterleaveCount( thePayload );
	theInterleaveIndex = IMAAudioPayloadInterleaveIndex( thePayload );
	theInterleaveTail = IMAAudioPayloadInterleaveTail( thePayload );
	
	theResult = theInterleaveCount * inPacket->dataLength;
	
	if( theInterleaveIndex <= theInterleaveTail )
	{
		theResult -=
			sizeof( IMAAudioFrame ) * ( theInterleaveCount - theInterleaveTail - 1 );
	}
	
	else
	{
		theResult += sizeof( IMAAudioFrame ) * ( theInterleaveTail + 1 );
	}
	
	return( theResult );
}



#pragma mark -
#pragma mark *        STANDARD COMPONENT INTERFACE
#pragma mark -
/* ---------------------------------------------------------------------------
 *		S T A N D A R D    C O M P O N E N T    I N T E R F A C E
 * ---------------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------------
 *		+ CallComponentOpen() implementation
 * ---------------------------------------------------------------------------
 *
 *	Allocate and initialize storage for instance variables.  When a
 *	reassembler is opened, it is not always called to reassemble data, so this
 *	function doesn't perform any allocations or time-consuming operations that
 *	are needed only to reassemble sample data.  The RTPRssmInitialize()
 *	implementation performs such operations.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_Open(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	ComponentInstance				self )
{
	ComponentResult		theResult = noErr;
	RTPReassembler		theBase;
	
	
	inGlobals =
		REINTERPRET_CAST( RTPRssmIMAAudioInstanceData ** )(
			NewHandleClear( sizeof( **inGlobals ) ) );
	
	if( inGlobals )
	{
		( **inGlobals ).itself = self;
		( **inGlobals ).itsFinalDerivation = self;
		
		SetComponentInstanceStorage( self, REINTERPRET_CAST( Handle )( inGlobals ) );
		
		theResult =
			OpenADefaultComponent(
				kRTPReassemblerType, kRTPBaseReassemblerType, &theBase );
		
		if( theResult == noErr )
		{
			( **inGlobals ).itsBase = theBase;
			theResult = CallComponentTarget( ( **inGlobals ).itsBase, self );
		}
	}
	
	else
	{
		theResult = MemError();
		
		if( theResult == noErr )
			theResult = memFullErr;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ CallComponentClose() implementation
 * ---------------------------------------------------------------------------
 *
 *	Reverse the effects of the CallComponentOpen() implementation.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_Close(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	ComponentInstance				self )
{
#pragma unused( self )
	
	if( inGlobals )
	{
		if( ( **inGlobals ).itsBase )
			CloseComponent( ( **inGlobals ).itsBase );
		
		DisposeHandle( REINTERPRET_CAST( Handle )( inGlobals ) );
	}
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ CallComponentVersion() implementation
 * ---------------------------------------------------------------------------
 *
 *	Return the instance's version.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_Version(
	RTPRssmIMAAudioInstanceData **	inGlobals )
{
#pragma unused( inGlobals )
	
	return( kComponentVersion );
}



/* ---------------------------------------------------------------------------
 *		+ CallComponentTarget() implementation
 * ---------------------------------------------------------------------------
 *
 *	Update the instance's inheritance graph with a new most-derived instance.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_Target(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	ComponentInstance				target )
{
	ComponentResult		theResult;
	
	
	if( ( **inGlobals ).itsBase )
		theResult = CallComponentTarget( ( **inGlobals ).itsBase, target );
	else
		theResult = noErr;
	
	if( theResult == noErr )
		( **inGlobals ).itsFinalDerivation = target;
	
	return( theResult );
}



#pragma mark -
#pragma mark *        RTP REASSEMBLER INTERFACE
#pragma mark -
/* ---------------------------------------------------------------------------
 *		R T P    R E A S S E M B L E R    I N T E R F A C E
 * ---------------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------------
 *		+ RTPRssmInitialize() implementation
 * ---------------------------------------------------------------------------
 *
 *	Prepare to reassemble sample data.  This implementation initializes
 *	instance variables that represent the payload state, opens a new
 *	StreamHandler to process reassembled sample data, and sets options that
 *	determine how its base reassembler handles network packets.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_Initialize(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	RTPRssmInitParams *				inInitParams )
{
	ComponentResult				theResult = noErr;
	IMAAudioPayload				thePayloadAttributes;
	SampleDescriptionHandle		theDescription;
	
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPRssmInitializeSelect ) )
		theResult = RTPRssmInitialize( ( **inGlobals ).itsBase, inInitParams );
	
	if( theResult == noErr )
	{
		IMAAudioPayloadInitialize( &thePayloadAttributes );
		
		theResult =
			__GetNewSampleDescription( &thePayloadAttributes, &theDescription );
		
		( **inGlobals ).itsPayloadAttributes = thePayloadAttributes;
		
		if( theResult == noErr )
		{
			theResult =
				RTPRssmNewStreamHandler(
					( **inGlobals ).itsFinalDerivation, SoundMediaType,
					theDescription, kIMAAudioPayloadRTPTimeScale, NULL );
			
			DisposeHandle( REINTERPRET_CAST( Handle )( theDescription ) );
			
			if( theResult == noErr )
			{
				theResult =
					RTPRssmSetCapabilities(
						( **inGlobals ).itsFinalDerivation,
						kRTPRssmTrackLostPacketsFlag | kRTPRssmQueueAndUseMarkerBitFlag,
						-1L );
			}
		}
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmComputeChunkSize() implementation
 * ---------------------------------------------------------------------------
 *
 *	Use the packet list to determine the size of the data buffer needed to
 *	pass data from these network packets to the StreamHandler.
 *	
 *	For this reassembler, the chunk size is the audio data size for the entire
 *	interleave group to which the given packets belong.  (The data is
 *	packetized such that the base reassembler can automatically group
 *	incoming packets by interleave group, so the packets in the list will be
 *	all those packets, and only those packets, from a single interleave
 *	group.)  The size cannot be computed by summing the data sizes of network
 *	packets received, since some packets might be lost.  Instead, the size is
 *	determnined, through a call to __InterleaveGroupDataSize() defined above,
 *	by inspecting the payload header and data size of the first packet.
 *	
 *	This function also updates the StreamHandler's SampleDescription when the
 *	sample rate or channel count changes.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_ComputeChunkSize(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	RTPRssmPacket *					inPacketListHead,
	SInt32 							inFlags,
	UInt32 *						outChunkDataSize )
{
#pragma unused( inFlags )

	ComponentResult				theResult = noErr;
	const IMAAudioPayload *		thePayload;
	UInt32						theChannelCount;
	UnsignedFixed				theSampleRate;
	SampleDescriptionHandle		theDescription;
	
	
	thePayload = __GetPayload( inPacketListHead );
	
	theChannelCount = IMAAudioPayloadChannelCount( &( **inGlobals ).itsPayloadAttributes );
	theSampleRate = IMAAudioPayloadSampleRate( &( **inGlobals ).itsPayloadAttributes );
	
	if(
		IMAAudioPayloadChannelCount( thePayload ) != theChannelCount  ||
		IMAAudioPayloadSampleRate( thePayload ) != theSampleRate )
	{
		theResult = __GetNewSampleDescription( thePayload, &theDescription );
		
		if( theResult == noErr )
		{
			theResult =
				RTPRssmSetSampleDescription(
					( **inGlobals ).itsFinalDerivation, theDescription );
			
			if( theResult == noErr )
				( **inGlobals ).itsPayloadAttributes = *thePayload;
			
			DisposeHandle( REINTERPRET_CAST( Handle )( theDescription ) );
		}
	}
	
	*outChunkDataSize = __InterleaveGroupDataSize( inPacketListHead );
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmAdjustPacketParams() implementation
 * ---------------------------------------------------------------------------
 *
 *	Adjust fields of the RTPRssmPacket to reflect the properties of its
 *	payload.  The payload format used by this reassembler defines a
 *	fixed-length header that immediately precedes the audio data.  Overriding
 *	RTPRssmAdjustPacketParams() gives the reassembler a chance to update the
 *	RTPRssmPacket data structure for each packet to reflect its payload header
 *	length and audio data length.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_AdjustPacketParams(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	RTPRssmPacket *					inPacket,
	SInt32							inFlags )
{
#pragma unused( inGlobals, inFlags )
	
	UInt32				theHeaderSize;
	IMAAudioPayload *	thePayload;
	
	
	theHeaderSize = sizeof( thePayload->itsFixedHeader );
	
	inPacket->payloadHeaderLength = theHeaderSize;
	inPacket->dataLength -= theHeaderSize;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmCopyDataToChunk() implementation
 * ---------------------------------------------------------------------------
 *
 *	Copy sample data from a packet list to the data buffer described by an
 *	SHChunkRecord.  The payloads processed by this reassembler indicate where
 *	interleave audio data from each payload.  This function iterates through
 *	the packets, copying the data to the correct position in the chunk.  The
 *	function then calls __FillMissingData(), defined above, to synthesize lost
 *	packet data.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_CopyDataToChunk(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	RTPRssmPacket *					inPacketListHead,
	UInt32							inMaxChunkDataSize,
	SHChunkRecord *					inChunk,
	SInt32							inFlags )
{
#pragma unused( inGlobals, inFlags )
	
	ComponentResult				theResult = noErr;
	UInt32						theFrameCount;
	IMAAudioFrame *				theFrames;
	RTPRssmPacket *				thePacket;
	const IMAAudioPayload *		thePayload;
	UInt16						theGroupStartSequenceNumber;
	UInt32						theInterleaveCount;
	UInt32						theInterleaveIndex;
	UInt32						thePacketFrameCount;
	UInt32						theSourceIndex;
	UInt32						theTargetIndex;
	
	
	/*	IMPORTANT	The sequence number computations in this function work
					because the packets are in order and the separation
					between the packets is guaranteed to be within the
					precision of the RTP sequence number field (i.e., less
					than 16 bits, or 65536 packets).  When computing with
					sequence numbers of packets that might be separated by
					more than 65535 packets, a reassembler must extend
					the precision of the sequence numbers, for instance by
					using the inNumWraparounds parameter of
					RTPRssmHandleNewPacket() or by using the timeStamp
					field of RTPRssmPacket structures.
	*/
	
	inChunk->dataSize = __InterleaveGroupDataSize( inPacketListHead );
	
	if( inChunk->dataSize > inMaxChunkDataSize )
	{
		/* this should never happen */
		
		theResult = buffersTooSmall;
	}
	
	else
	{
		theFrameCount = inChunk->dataSize / sizeof( IMAAudioFrame );
		theFrames =
			REINTERPRET_CAST( IMAAudioFrame * )(
				CONST_CAST( UInt8 * )( inChunk->dataPtr ) );
		
		thePayload = __GetPayload( inPacketListHead );
		
		theInterleaveCount = IMAAudioPayloadInterleaveCount( thePayload );
		
		theGroupStartSequenceNumber =
			inPacketListHead->sequenceNum - IMAAudioPayloadInterleaveIndex( thePayload );
		
		for( thePacket = inPacketListHead; thePacket; thePacket = thePacket->next )
		{
			thePayload = __GetPayload( thePacket );
			
			theInterleaveIndex = thePacket->sequenceNum - theGroupStartSequenceNumber;
			thePacketFrameCount = thePacket->dataLength / sizeof( IMAAudioFrame );
			
			for( theSourceIndex = 0; theSourceIndex < thePacketFrameCount; theSourceIndex++ )
			{
				theTargetIndex =
					( theSourceIndex * theInterleaveCount ) + theInterleaveIndex;
				
				if( theTargetIndex < theFrameCount )
				{
					theFrames[ theTargetIndex ] =
						thePayload->itsAudioFrames[ theSourceIndex ];
				}
			}
		}
		
		__FillMissingData(
			inPacketListHead, theGroupStartSequenceNumber, theInterleaveCount, inChunk );
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmSendPacketList() implementation
 * ---------------------------------------------------------------------------
 *
 *	Prepare to process or discard a list of packets.  If the base detects that
 *	some network packets were lost, overriding RTPRssmSendPacketList() gives
 *	the reassembler a chance to decide whether it can handle the remaining
 *	packets.  This reassembler tolerates any amount of packet loss, so this
 *	function simply clears the kRTPRssmLostSomePackets flag before delegating
 *	to the base.  When a reassembler is able to recover from packet loss,
 *	turning off the kRTPRssmLostSomePackets flag before delegating the call to
 *	the base reassembler prevents the base from discarding the packets.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_SendPacketList(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	RTPRssmPacket *					inPacketListHead,
	const TimeValue64 *				inLastChunkPresentationTime,
	SInt32							inFlags )
{
	return(
		RTPRssmSendPacketList(
			( **inGlobals ).itsBase, inPacketListHead, inLastChunkPresentationTime,
			inFlags &= ~kRTPRssmLostSomePackets ) );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmHasCharacteristic() implementation
 * ---------------------------------------------------------------------------
 *
 *	Indicate whether the reassembler has the specified characteristic.  This
 *	reassembler requires ordered packet lists.  All other characteristics are
 *	delegated to the base.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_HasCharacteristic(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	OSType 							inCharacteristic,
	Boolean *						outHasIt )
{
	ComponentResult		theResult;
	
	
	if( inCharacteristic == kRTPRssmRequiresOrderedPacketsCharacteristic )
	{
		*outHasIt = true;
		theResult = noErr;
	}

	else
	{
		theResult =
			RTPRssmHasCharacteristic( ( **inGlobals ).itsBase, inCharacteristic, outHasIt );
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmReset() implementation
 * ---------------------------------------------------------------------------
 *
 *	End processing of the current stream of network packets and prepare to
 *	process a new, possibly unrelated, stream.  This implementation simply
 *	resets its base.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmIMAAudio_Reset(
	RTPRssmIMAAudioInstanceData **	inGlobals,
	SInt32							inFlags )
{
	ComponentResult		theResult;
	
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPRssmResetSelect ) )
		theResult = RTPRssmReset( ( **inGlobals ).itsBase, inFlags );
	else
		theResult = noErr;
	
	return( theResult );
}
