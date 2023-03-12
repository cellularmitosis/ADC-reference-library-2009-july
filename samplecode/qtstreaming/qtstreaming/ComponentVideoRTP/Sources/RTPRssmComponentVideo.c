/*
	File:		RTPRssmComponentVideo.c

	Contains:	Definition of Component Video RTPReassembler

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
		
		RTPRssmGetInfo()				Return the requested information.
		
		RTPRssmHasCharacteristic()		Indicate whether the reassembler has the
										specified characteristic.
		
		RTPRssmReset()					End processing of the current stream of
										network packets.
*/



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#include "RTPRssmComponentVideo.h"
#include "RTPRssmComponentVidResources.h"
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

#define RTPRSSM_BASENAME()				RTPRssmComponentVideo_
#define RTPRSSM_GLOBALS()				RTPRssmComponentVideoInstanceData **

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
#define COMPONENT_DISPATCH_FILE			"RTPRssmComponentVideoDispatch.h"
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

enum
{
	__kDefaultWidth				= 160,
	__kDefaultHeight			= 120,
	__kHorizontalResolution		= 72,
	__kVertivalResolution		= 72,
	__kDepth					= 24
};



typedef RTPRSSM_GLOBALS()	__InstanceData;



/* ---------------------------------------------------------------------------
 *		__FrameDataSize()
 * ---------------------------------------------------------------------------
 *
 *	Compute the number of octets in one frame of Component Video data having
 *	the given dimensions.
 *	
 *	YUV 4:2:2 encoding uses four octets to represent a pair of pixels.
 *	The number of octets in a frame is therefore
 *	
 *		padded-width x height x 2 octets
 *	
 *	where padded-width is the row width in pixels padded to an
 *	even number.
 *
 */

static
UInt32
__FrameDataSize(
	UInt16	inWidth,
	UInt16	inHeight )
{
	return( ( ( ( inWidth + 1 ) & ( ~1L ) ) * inHeight ) << 1 );
}



/* ---------------------------------------------------------------------------
 *		__ReleaseChunk()
 * ---------------------------------------------------------------------------
 *
 *	Release any previously saved chunk.
 *
 */

static
SHChunkRecord *
__ReleaseChunk(
	__InstanceData		inGlobals )
{
	SHChunkRecord *		theResult = ( **inGlobals ).itsSavedChunk;
	
	
	if( ( **inGlobals ).itsSavedChunk )
	{
		RTPRssmDecrChunkRefCount(
			( **inGlobals ).itsFinalDerivation, ( **inGlobals ).itsSavedChunk );
		
		( **inGlobals ).itsSavedChunk = NULL;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__SaveChunk()
 * ---------------------------------------------------------------------------
 *
 *	Release any previously saved chunk and save the given chunk.
 *
 */

static
ComponentResult
__SaveChunk(
	__InstanceData		inGlobals,
	SHChunkRecord *		inChunk )
{
	ComponentResult		theResult;
	SHChunkRecord *		thePreviousChunk;
	
	
	thePreviousChunk = __ReleaseChunk( inGlobals );
	
	if( inChunk )
	{
		theResult = RTPRssmIncrChunkRefCount( ( **inGlobals ).itsFinalDerivation, inChunk );
		
		if( theResult == noErr )
			( **inGlobals ).itsSavedChunk = inChunk;
		else
			theResult = __SaveChunk( inGlobals, thePreviousChunk );
	}
	
	else
	{
		theResult = noErr;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__GetNewSampleDescription()
 * ---------------------------------------------------------------------------
 *
 *	Create a new SampleDescription for this reassembler's payload format using
 *	the given dimensions.
 *
 */

static
ComponentResult
__GetNewSampleDescription(
	__InstanceData				inGlobals,
	UInt16						inWidth,
	UInt16						inHeight,
	SampleDescriptionHandle *	outDescription )
{
	ComponentResult			theResult = noErr;
	ImageDescriptionHandle	theDescription;
	Str255					theCodecName;
	
	
	theDescription =
		REINTERPRET_CAST( ImageDescriptionHandle )(
			NewHandleClear( sizeof( **theDescription ) ) );
	
	if( theDescription )
	{
		*outDescription = REINTERPRET_CAST( SampleDescriptionHandle )( theDescription );
		
		( **theDescription ).idSize = sizeof( **theDescription );
		( **theDescription ).cType = kComponentVideoDataFormat;
		( **theDescription ).version = 1;
		( **theDescription ).revisionLevel = 1;
		( **theDescription ).vendor = kComponentManufactureType;
		( **theDescription ).temporalQuality = codecNormalQuality;
		( **theDescription ).spatialQuality = codecNormalQuality;
		( **theDescription ).width = inWidth;
		( **theDescription ).height = inHeight;
		( **theDescription ).hRes = Long2Fix( __kHorizontalResolution );
		( **theDescription ).vRes = Long2Fix( __kVertivalResolution );
		( **theDescription ).dataSize = 0;
		( **theDescription ).frameCount = 1;
		( **theDescription ).depth = __kDepth;
		( **theDescription ).clutID = -1;
		
		theCodecName[ 0 ] = '\0';	/* in case string resource doesn't load */
		
		GetComponentIndString(
			REINTERPRET_CAST( Component )( ( **inGlobals ).itself ),
			theCodecName, kRTPRssmComponentVideoStringListResource,
			kRTPRssmComponentVideoCodecNameString );
		
		BlockMoveData(
			theCodecName, ( **theDescription ).name, sizeof( ( **theDescription ).name ) );
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
ComponentVideoPayload *
__GetPayload(
	RTPRssmPacket *		inPacket )
{
	return( 
		REINTERPRET_CAST( ComponentVideoPayload * )(
			&inPacket->streamBuffer->rptr[ inPacket->transportHeaderLength ] ) );
}



/* ---------------------------------------------------------------------------
 *		__GetData()
 * ---------------------------------------------------------------------------
 *
 *	Return a pointer to a packet's image data.
 *
 */

static
UInt8 *
__GetData(
	RTPRssmPacket *		inPacket )
{
	return( 
		&inPacket->streamBuffer->rptr[
				inPacket->transportHeaderLength + inPacket->payloadHeaderLength ] );
}



/* ---------------------------------------------------------------------------
 *		__FillMissingData()
 * ---------------------------------------------------------------------------
 *
 *	If the source chunk exists, copy the specified interval from the source
 *	chunk to the target chunk.  Otherwise, set the specified interval of the
 *	target chunk to black.
 *
 */

static
void
__FillMissingData(
	UInt32					inOffset,
	UInt32					inLength,
	const SHChunkRecord *	inSourceChunk,
	SHChunkRecord *			inTargetChunk )
{
	if( inSourceChunk )
	{
		BlockMoveData(
			&inSourceChunk->dataPtr[ inOffset ],
			CONST_CAST( UInt8 * )( &inTargetChunk->dataPtr[ inOffset ] ),
			inLength );
	}
	
	else
	{
		memset(
			CONST_CAST( UInt8 * )( &inTargetChunk->dataPtr[ inOffset ] ),
			0, inLength );
	}
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
RTPRssmComponentVideo_Open(
	__InstanceData		inGlobals,
	ComponentInstance	self )
{
	ComponentResult		theResult = noErr;
	RTPReassembler		theBase;
	
	
	inGlobals =
		REINTERPRET_CAST( __InstanceData )( NewHandleClear( sizeof( **inGlobals ) ) );
	
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
RTPRssmComponentVideo_Close(
	__InstanceData		inGlobals,
	ComponentInstance	self )
{
#pragma unused( self )
	
	if( inGlobals )
	{
		__ReleaseChunk( inGlobals );
		
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
RTPRssmComponentVideo_Version(
	__InstanceData		inGlobals )
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
RTPRssmComponentVideo_Target(
	__InstanceData		inGlobals,
	ComponentInstance	target )
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
RTPRssmComponentVideo_Initialize(
	__InstanceData			inGlobals,
	RTPRssmInitParams *		inInitParams )
{
	ComponentResult				theResult = noErr;
	SampleDescriptionHandle		theDescription;
	
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPRssmInitializeSelect ) )
		theResult = RTPRssmInitialize( ( **inGlobals ).itsBase, inInitParams );
	
	if( theResult == noErr )
	{
		( **inGlobals ).itsSavedChunk = NULL;
		
		/*	The reassembler hasn't received a payload description yet, but it can't
			create a new StreamHandler without a SampleDescription.  The reassembler
			initializes its payload attributes with a default description that it
			will use to create a new StreamHandler, then immediately invalidates the
			description.  This causes the reassembler to search incoming network
			packets for a payload description.  If the incoming payload description
			matches the default, the StreamHandler need not be updated. */
		
		ComponentVideoPayloadInitialize(
			&( **inGlobals ).itsPayloadAttributes, __kDefaultWidth, __kDefaultHeight );
		
		ComponentVideoPayloadSetDescription( &( **inGlobals ).itsPayloadAttributes, 0, 0 );
		
		ComponentVideoPayloadSetOffset(
			&( **inGlobals ).itsPayloadAttributes,
			__FrameDataSize( __kDefaultWidth, __kDefaultHeight ) );
		
		theResult =
			__GetNewSampleDescription(
				inGlobals, __kDefaultWidth, __kDefaultHeight, &theDescription );
		
		if( theResult == noErr )
		{
			theResult =
				RTPRssmNewStreamHandler(
					( **inGlobals ).itsFinalDerivation, VideoMediaType,
					theDescription, kComponentVideoRTPTimeScale, NULL );
			
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
 *	pass data from these packets to the StreamHandler.  The chunk size for
 *	this reassembler is the image data size for one frame. The size cannot be
 *	computed by summing the data sizes of network packets received, since some
 *	packets might be lost.  Overriding RTPRssmComputeChunkSize() gives the
 *	reassembler a chance to compute the chunk size from the current payload
 *	description.
 *
 *	If the description of the received data is not known, this function
 *	searches the given packet list for a description.  The payload description
 *	is not known if no payload description is cached, or if the description
 *	seed of the incoming network packets doesn't match that of the cached
 *	description.
 *	
 *	(The payload description is also unknown if too many packets have been
 *	lost since the last frame.  Specifically, if the description seeds could
 *	have recycled since the last packet received, then the cached description
 *	is not guaranteed to match the current description, even if it shares the
 *	same seed.  However, since the payload format for this reassembler cycles
 *	through 2^31 seeds, this implementation does not bother to detect this
 *	condition.)
 *	
 *	If a description is found, this function caches the payload description,
 *	updates the StreamHandler's SampleDescription, and updates the Offset
 *	field of the instance's payload attributes with the frame data size
 *	required for that SampleDescription.
 *	
 *	This function always returns the pre-computed data size stored in the
 *	Offset field of the instance's payload attributes instance variable.
 *	Even when the current payload description cannot be determined, the
 *	Offset field gives the chunk size appropriate for the StreamHandler's
 *	current SampleDescription.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmComponentVideo_ComputeChunkSize(
	__InstanceData		inGlobals,
	RTPRssmPacket *		inPacketListHead,
	SInt32 				inFlags,
	UInt32 *			outChunkDataSize )
{
#pragma unused( inFlags )

	ComponentResult				theResult = noErr;
	RTPRssmPacket *				thePacket;
	ComponentVideoPayload *		thePayload;
	UInt16						theWidth;
	UInt16						theHeight;
	SampleDescriptionHandle		theDescription;
	
	
	thePayload = __GetPayload( inPacketListHead );
	
	if(
		ComponentVideoPayloadDescriptionSeed( &( **inGlobals ).itsPayloadAttributes ) !=
			ComponentVideoPayloadDescriptionSeed( thePayload ) )
	{
		ComponentVideoPayloadSetDescription( &( **inGlobals ).itsPayloadAttributes, 0, 0 );
	}
	
	
	/*	If the instance doesn't have a cached payload description, iterate through the
		packet list until it caches an incoming payload description. */
	
	for(
		thePacket = inPacketListHead;
		thePacket  &&
			!ComponentVideoPayloadHasDescription( &( **inGlobals ).itsPayloadAttributes );
		thePacket = thePacket->next )
	{
		thePayload = __GetPayload( thePacket );
		
		if( ComponentVideoPayloadHasDescription( thePayload ) )
		{
			theWidth = ComponentVideoPayloadWidth( thePayload );
			theHeight = ComponentVideoPayloadHeight( thePayload );
			
			
			/*	If the imcoming payload description matches the description last used,
				simply cache the incoming description.  If they don't match, update
				the StreamHandler's SampleDescription, update the Offset field of the
				reassembler's payload attributes, and release any saved chunk as well. */
			
			if(
				theWidth ==
					ComponentVideoPayloadWidth( &( **inGlobals ).itsPayloadAttributes )  &&
				theHeight ==
					ComponentVideoPayloadHeight( &( **inGlobals ).itsPayloadAttributes ) )
			{
				ComponentVideoPayloadCopyDescription(
					&( **inGlobals ).itsPayloadAttributes, thePayload );
			}
			
			else
			{
				theResult =
					__GetNewSampleDescription(
						inGlobals, theWidth, theHeight, &theDescription );
				
				if( theResult == noErr )
				{
					ComponentVideoPayloadSetOffset(
						&( **inGlobals ).itsPayloadAttributes,
						__FrameDataSize( theWidth, theHeight ) );
					
					ComponentVideoPayloadCopyDescription(
						&( **inGlobals ).itsPayloadAttributes, thePayload );
					
					__ReleaseChunk( inGlobals );
					
					theResult =
						RTPRssmSetSampleDescription(
							( **inGlobals ).itsFinalDerivation, theDescription );
					
					DisposeHandle( REINTERPRET_CAST( Handle )( theDescription ) );
				}
			}
		}
	}
	
	/*	The current data size for incoming frames is always stored in the Offset
		field of the reassembler's payload attributes instance variable. */
	
	*outChunkDataSize =
		ComponentVideoPayloadOffset( &( **inGlobals ).itsPayloadAttributes );
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmAdjustPacketParams() implementation
 * ---------------------------------------------------------------------------
 *
 *	Adjust fields of the RTPRssmPacket to reflect the properties of its
 *	payload.  The payload format used by this reassembler defines a
 *	variable-length header that immediately precedes the image data.
 *	Overriding RTPRssmAdjustPacketParams() gives the reassembler a chance to
 *	update the RTPRssmPacket data structure for each packet to reflect its
 *	payload header length and image data length.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmComponentVideo_AdjustPacketParams(
	__InstanceData		inGlobals,
	RTPRssmPacket *		inPacket,
	SInt32				inFlags )
{
#pragma unused( inGlobals, inFlags )
	
	UInt32						theHeaderSize;
	ComponentVideoPayload *		thePayload;
	
	
	thePayload = __GetPayload( inPacket );
	
	if( ComponentVideoPayloadHasDescription( thePayload ) )
		theHeaderSize = sizeof( *thePayload );
	else
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
 *	in the frame the image data from each payload belongs.  This function
 *	iterates through the packets, copying the data to the correct position in
 *	the chunk.  The packet list is ordered by the base reassembler.  Where
 *	packets are missing, this function calls __FillMissingData(), defined
 *	above, to fill in lost packet data with data from the previous frame.
 *
 *	If the payload description has changed without being detected, the image
 *	data might overflow or underflow the buffer.  This function detects these
 *	conditions and invalidates the cached payload description.
 *	
 */

EXTERN_API( ComponentResult )
RTPRssmComponentVideo_CopyDataToChunk(
	__InstanceData		inGlobals,
	RTPRssmPacket *		inPacketListHead,
	UInt32				inMaxChunkDataSize,
	SHChunkRecord *		inChunk,
	SInt32				inFlags )
{
#pragma unused( inFlags )
	
	ComponentResult				theResult = noErr;
	RTPRssmPacket *				thePacket;
	ComponentVideoPayload *		thePayload;
	UInt32						theExpectedOffset;
	UInt32						theOffset;
	
	
	if(
		inMaxChunkDataSize <
			ComponentVideoPayloadOffset( &( **inGlobals ).itsPayloadAttributes ) )
	{
		/* This should never happen. */
		
		theResult = buffersTooSmall;
	}
	
	
	/* The first packet should contain data for the start of the image buffer. */
	
	theExpectedOffset = 0;
	
	for(
		thePacket = inPacketListHead; thePacket  &&  theResult == noErr;
		thePacket = thePacket->next )
	{
		thePayload = __GetPayload( thePacket );
		theOffset = ComponentVideoPayloadOffset( thePayload );
		
		if( theOffset + thePacket->dataLength <= inMaxChunkDataSize )
		{
			/*	The base reassembler keeps the packets ordered.  If a network packet's
				data doesn't immediately follow the data from the previous packet in
				the list, then one or more packets must have been lost. */
			
			if( theExpectedOffset < theOffset )
			{
				__FillMissingData(
					theExpectedOffset, theOffset - theExpectedOffset,
					( **inGlobals ).itsSavedChunk, inChunk );
			}
			
			BlockMoveData(
				__GetData( thePacket ),
				CONST_CAST( UInt8 * )( &inChunk->dataPtr[ theOffset ] ),
				thePacket->dataLength );
			
			theExpectedOffset = theOffset + thePacket->dataLength;
		}
		
		else
		{
			/*	The image data for the current packet would overflow the image buffer.
				Therefore the cached payload description is inconsistent with the
				incoming packet. */
			
			ComponentVideoPayloadSetDescription(
				&( **inGlobals ).itsPayloadAttributes, 0, 0 );
		}
		
		if( !thePacket->next )
		{
			inChunk->dataSize =
				ComponentVideoPayloadOffset( &( **inGlobals ).itsPayloadAttributes );
			
			
			/*	If the last packet's data doesnt fill the rest of the image buffer, one
				or more packets might be missing.  If the RTP/AVP marker bit is set,
				indicating the last packet of the frame, then the incoming data has
				underflowed the data buffer, so the cached payload description is
				inconsistent with the incoming packet. */
			
			if( theExpectedOffset < inChunk->dataSize )
			{
				__FillMissingData(
					theExpectedOffset, inChunk->dataSize - theExpectedOffset,
					( **inGlobals ).itsSavedChunk, inChunk );
				
				if( thePacket->flags & kRTPRssmPacketHasMarkerBitSet )
				{
					ComponentVideoPayloadSetDescription(
						&( **inGlobals ).itsPayloadAttributes, 0, 0 );
				}
			}
			
			__SaveChunk( inGlobals, inChunk );
		}
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmSendPacketList() implementation
 * ---------------------------------------------------------------------------
 *
 *	Prepare to process or discard a list of packets.  This reassembler
 *	tolerates packet loss only if it knows the current payload description.
 *	If the base detects that some network packets were lost, overriding
 *	RTPRssmSendPacketList() gives the reassembler a chance to decide whether
 *	it can handle the remaining packets.  This implementation inspects the
 *	packet list to determine whether its payload description is known and
 *	updates the kRTPRssmLostSomePackets flag accordingly.  When the
 *	reassembler is able to recover from packet loss, turning off the
 *	kRTPRssmLostSomePackets flag before delegating the call to the base
 *	reassembler prevents the base from discarding the packets.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmComponentVideo_SendPacketList(
	__InstanceData			inGlobals,
	RTPRssmPacket *			inPacketListHead,
	const TimeValue64 *		inLastChunkPresentationTime,
	SInt32					inFlags )
{
	ComponentVideoPayload *		thePayload;
	RTPRssmPacket *				thePacket;
	
	
	if( inFlags & kRTPRssmLostSomePackets )
	{
		thePayload = __GetPayload( inPacketListHead );
		
		
		/*	If the current payload description is already cached, turn off the packet
			loss flag.  Otherwise, turn off the flag only if the packet list
			contains a payload description. */
		
		if(
			ComponentVideoPayloadHasDescription( &( **inGlobals ).itsPayloadAttributes )  &&
			ComponentVideoPayloadDescriptionSeed( &( **inGlobals ).itsPayloadAttributes ) ==
				ComponentVideoPayloadDescriptionSeed( thePayload ) )
		{
			inFlags &= ~kRTPRssmLostSomePackets;
		}
		
		else
		{
			for(
				thePacket = inPacketListHead;
				thePacket  &&  ( inFlags & kRTPRssmLostSomePackets );
				thePacket = thePacket->next )
			{
				thePayload = __GetPayload( thePacket );
				
				
				/*	If a payload description is found, clear the kRTPRssmLostSomePackets
					flag.  The reassembler will compensate for lost packet data. */
				
				if( ComponentVideoPayloadHasDescription( thePayload ) )
					inFlags &= ~kRTPRssmLostSomePackets;
			}
		}
	}
	
	
	/* Delegate the remaining work to the base reassembler. */
	
	return(
		RTPRssmSendPacketList(
			( **inGlobals ).itsBase, inPacketListHead, inLastChunkPresentationTime,
			inFlags ) );
}



/* ---------------------------------------------------------------------------
 *		+ RTPRssmGetInfo() implementation
 * ---------------------------------------------------------------------------
 *
 *	Return the information indicated by the selector.  This implemenation
 *	computes a result for the following selectors and delegates all others to
 *	its base.
 *
 *
 *		kQTSSourceDimensionsInfo	ioParams points to an QTSDimensionParams
 *									structure where the implementation returns
 *									the width and height from its cached
 *									payload description.
 *
 *		kQTSSourceClipRectInfo
 *		kQTSSourceBoundingRectInfo	ioParams points to a Rect.  The
 *									implementation returns a Rect whose top
 *									left corner is at the origin and whose
 *									width and height are those from its cached
  *									payload description.
*
 */

EXTERN_API( ComponentResult )
RTPRssmComponentVideo_GetInfo(
	__InstanceData		inGlobals,
	OSType 				inSelector,
	void *				ioParams )
{
	ComponentResult			theResult = noErr;
	QTSDimensionParams *	theDimensions;
	Rect *					theRect;
	
	
	switch( inSelector )
	{
		case kQTSSourceDimensionsInfo:
			theDimensions = STATIC_CAST( QTSDimensionParams * )( ioParams );
			
			theDimensions->width =
				Long2Fix(
					ComponentVideoPayloadWidth( &( **inGlobals ).itsPayloadAttributes ) );
			
			theDimensions->height =
				Long2Fix(
					ComponentVideoPayloadHeight( &( **inGlobals ).itsPayloadAttributes ) );
		break;
		
		
		case kQTSSourceClipRectInfo:
		case kQTSSourceBoundingRectInfo:
			theRect = STATIC_CAST( Rect * )( ioParams );
			
			MacSetRect(
				theRect, 0, 0,
				ComponentVideoPayloadWidth( &( **inGlobals ).itsPayloadAttributes ),
				ComponentVideoPayloadHeight( &( **inGlobals ).itsPayloadAttributes ) );
		break;
		
		
		default:
			theResult =
				RTPRssmGetInfo( ( **inGlobals ).itsBase, inSelector, ioParams );
		break;
	}
	
	return( theResult );
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
RTPRssmComponentVideo_HasCharacteristic(
	__InstanceData		inGlobals,
	OSType 				inCharacteristic,
	Boolean *			outHasIt )
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
 *	process a new, possibly unrelated, stream.  This implementation releases
 *	any saved chunk and invalidates its cached payload description before
 *	resetting its base.
 *
 */

EXTERN_API( ComponentResult )
RTPRssmComponentVideo_Reset(
	__InstanceData		inGlobals,
	SInt32				inFlags )
{
	ComponentResult		theResult;
	
	
	__ReleaseChunk( inGlobals );
	ComponentVideoPayloadSetDescription( &( **inGlobals ).itsPayloadAttributes, 0, 0 );
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPRssmResetSelect ) )
		theResult = RTPRssmReset( ( **inGlobals ).itsBase, inFlags );
	else
		theResult = noErr;
	
	return( theResult );
}
