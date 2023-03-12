/*
	File:		RTPMPComponentVideo.c

	Contains:	Definition of Component Video RTPMediaPacketizer

	Copyright:	© 1997-1999 by Apple Computer, Inc., all rights reserved.
	
	
	
	OVERVIEW
	
	QuickTime Streaming software uses an RTPMediaPacketizer to encapsulate
	sample data in network packets.  When preparing to packetize sample data,
	Streaming software first determines a packetizer's capabilities and
	operational parameters.  If the packetizer is suitable, the Streaming
	software provides the packetizer with an RTPPacketBuilder.  The packetizer
	uses the RTPPacketBuilder to construct network packets from sample data.
	The Streaming software specifies what sample data to packetize by calling
	the packetizer's RTPMPSetSampleData() implementation.
	
	This packetizer implements the following interface and delegates all other
	calls to a base RTPMediaPacketizer defined by QuickTime Streaming.
		
		
		STANDARD COMPONENT INTERFACE
		----------------------------
		
		CallComponentOpen()				Allocate and initialize storage for
										instance variables.
		
		CallComponentClose()			Reverse the effects of
										CallComponentOpen().
		
		CallComponentVersion()			Return the instance's version.
		
		CallComponentTarget()			Update the instance's inheritance graph.
		
		
		
		RTP MEDIA PACKETIZER INTERFACE
		------------------------------
		
		RTPMPInitialize()				Prepare to packetize sample data.
		
		RTPMPPreflightMedia()			Determine whether the packetizer can
										packetize the described data.
		
		RTPMPSetSampleData()			Use the instance's RTPPacketBuilder to
										packetize sample data.
		
		RTPMPFlush()					Finish any packetization in progress.
		
		RTPMPReset()					Abort any packetization in progress.
		
		RTPMPGetInfo()					Return the requested information.
		
		RTPMPSetTimeScale()
		
		RTPMPGetTimeScale()
		
		RTPMPSetTimeBase()
		
		RTPMPGetTimeBase()
		
		RTPMPHasCharacteristic()
		
		RTPMPSetPacketBuilder()
		
		RTPMPGetPacketBuilder()
		
		RTPMPSetMediaType()
		
		RTPMPGetMediaType()
		
		RTPMPSetMaxPacketSize()
		
		RTPMPGetMaxPacketSize()
		
		RTPMPSetMaxPacketDuration()
		
		RTPMPGetMaxPacketDuration()		
*/



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#include "RTPMPComponentVideo.h"
#include "RTPMPComponentVideoResources.h"



/* ---------------------------------------------------------------------------
 *		R T P    M E D I A    P A C K E T I Z E R    P R O T O T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	QTStreamingComponents.k.h uses these macros to declare prototypes for
 *	the RTPMediaPacketizer calls defined in this file.
 *
 */

#define RTPMP_BASENAME()				RTPMPComponentVideo_
#define RTPMP_GLOBALS()					RTPMPComponentVideoInstanceData **

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

#define CALLCOMPONENT_BASENAME()		RTPMP_BASENAME()
#define CALLCOMPONENT_GLOBALS()			RTPMP_GLOBALS() storage
#define COMPONENT_DISPATCH_FILE			"RTPMPComponentVideoDispatch.h"
#define COMPONENT_C_DISPATCHER			1
#define COMPONENT_UPP_SELECT_ROOT()		RTPMP
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
	__kNoLimit						= 0xFFFFFFFF,
	__kNoFlags						= 0,
	__kDefaultPacketDurationLimit	= __kNoLimit,
	__kPayloadDataSizeMask			= 0xFFFFFFFC,
	__kTypicalMTUSize				= 1500,			/* Ethernet */
	__kTypicalNetworkHeaderSize		= 20,			/* IP, no options */
	__kTypicalTransportHeaderSize	= 8,			/* UDP */
	__kTypicalRTPHeaderSize			= 12,			/* no CSRCs, no extension */
	__kDefaultPacketSizeLimit		=
		__kTypicalMTUSize - __kTypicalNetworkHeaderSize - __kTypicalTransportHeaderSize -
		__kTypicalRTPHeaderSize
};



/* ---------------------------------------------------------------------------
 *		__UpdateDataSizes()
 * ---------------------------------------------------------------------------
 *
 *	Update data sizes the instance uses to construct network packets.
 *
 */

static
void
__UpdateDataSizes(
	RTPMPComponentVideoInstanceData **	inGlobals )
{
	UInt16	theWidth;
	UInt16	theHeight;
	
	
	theWidth = ComponentVideoPayloadWidth( &( **inGlobals ).itsPayloadHeader );
	theHeight = ComponentVideoPayloadHeight( &( **inGlobals ).itsPayloadHeader );
	
	
	/*	YUV 4:2:2 encoding uses four octets to represent a pair of pixels.
	 	The number of octets in a frame is therefore
	 	
	 		padded-width x height x 2 octets
	 	
	 	where padded-width is the row width in pixels padded to an
	 	even number. */
	
	( **inGlobals ).itsFrameDataSize =
		( ( ( theWidth + 1 ) & ( ~1L ) ) * theHeight ) << 1;
	
	
	/*	This packetizer sends an integral number of sample packets in
		each network packet.  Since YUV 4:2:2 encodes samples in four-octet
		sample packets, the amount of data sent in a network packet will be
		a multiple of four octets.  Most network packets will carry a fixed
		header and enough sample data to fill the network packet as closely
		as possible to the network packet size limit. */
	
	( **inGlobals ).itsPayloadDataSize =
		( ( **inGlobals ).itsPacketSizeLimit -
			sizeof( ( **inGlobals ).itsPayloadHeader.itsFixedHeader ) ) &
		__kPayloadDataSizeMask;
	
	
	/*	Whatever data would be left over after dividing a frame into
		maximal-length network packets must fit into the first packet along
		with a fixed header and a payload description.   If the leftover
		data won't fit, adjust the size of the other network packets. */
	
	while(
		( ( ( **inGlobals ).itsFrameDataSize % ( **inGlobals ).itsPayloadDataSize ) +
			sizeof( ( **inGlobals ).itsPayloadHeader ) >
		( **inGlobals ).itsPacketSizeLimit ) )
	{
		( **inGlobals ).itsPayloadDataSize -= 4;
	}
}



/* ---------------------------------------------------------------------------
 *		__UpdateSampleDescription()
 * ---------------------------------------------------------------------------
 *
 *	Determine whether the SampleDescription of incoming sample data has
 *	changed and update instance variables accordingly.
 *
 */

static
ComponentResult
__UpdateSampleDescription(
	RTPMPComponentVideoInstanceData **	inGlobals,
	const RTPMPSampleDataParams *		inSampleData )
{
	ComponentResult			theError = noErr;
	SInt32					theFlags;
	ImageDescriptionHandle	theDescription;
	
	
	/*	Check the SampleDescription if it is the first description or if the cached
		sample description seed doesn't match the current seed. */
	
	if(
		!ComponentVideoPayloadHasDescription( &( **inGlobals ).itsPayloadHeader )  ||
		( **inGlobals ).itsSampleDescriptionSeed != inSampleData->sampleDescSeed )
	{
		theDescription =
			REINTERPRET_CAST( ImageDescriptionHandle )( inSampleData->sampleDescription );
		
		
		/* Only update if the differences in the new SampleDescription are pertinent. */
		
		if(
			ComponentVideoPayloadWidth( &( **inGlobals ).itsPayloadHeader ) !=
				( **theDescription ).width  ||
			ComponentVideoPayloadHeight( &( **inGlobals ).itsPayloadHeader ) !=
				( **theDescription ).height )
		{
			/*	If the RTPMPSetSampleData() implementation queues data, then any
				queued sample data, which uses the obsolete SampleDescription, must
				be flushed before updating to the new SampleDescription. */
			
			theError = RTPMPFlush( ( **inGlobals ).itsFinalDerivation, 0, &theFlags );
			
			if( !theError )
			{
				/*	Update the payload description that the packetizer includes in
					some network packets. */
				
				ComponentVideoPayloadSetDescription(
					&( **inGlobals ).itsPayloadHeader, ( **theDescription ).width,
					( **theDescription ).height );
				
				
				/*	Update data sizes the packetizer uses to construct network
					packets. */
				
				__UpdateDataSizes( inGlobals );
			}
		}
		
		
		/*	Update the cached sample description seed to indicate that the
			packetizer state is now consistent with the new SampleDescription. */
		
		if( !theError )
			( **inGlobals ).itsSampleDescriptionSeed = inSampleData->sampleDescSeed;
	}
	
	return( theError );
}



/* ---------------------------------------------------------------------------
 *		__PacketizeSampleData()
 * ---------------------------------------------------------------------------
 *
 *	Construct network packets from the specified sample data.
 *
 */

static
ComponentResult
__PacketizeSampleData(
	RTPMPComponentVideoInstanceData **	inGlobals,
	const RTPMPSampleDataParams *		inSampleData )
{
	ComponentResult			theError;
	RTPPacketGroupRef		thePacketGroup;
	UInt32					theDataOffset;
	RTPPacketRef			thePacket;
	UInt32					theDataSize;
	ComponentVideoPayload	theHeader;
	UInt32					theHeaderSize;
	
	
	/* This packetizer sends all the data for one frame in a single packet group. */
	
	theError =
		RTPPBBeginPacketGroup(
			( **inGlobals ).itsPacketBuilder, __kNoFlags, inSampleData->timeStamp,
			&thePacketGroup );
	
	if( !theError )
	{
		/*	Most network packets for the frame will be uniformly sized, with a
			fixed header and a fixed amount of sample data.  The first network
			packet will include whatever data is left over (taken from the
			start of the sample data, not the end), as well as a fixed header
			and a payload description. */
		
		theDataOffset = 0;
		theDataSize = ( **inGlobals ).itsFrameDataSize % ( **inGlobals ).itsPayloadDataSize;
		
		theHeader = ( **inGlobals ).itsPayloadHeader;
		theHeaderSize = sizeof( theHeader );
		
		
		/*	Construct network packets until the sample data for this frame is exhausted. */
		
		while( theDataOffset < ( **inGlobals ).itsFrameDataSize  &&  !theError )
		{
			theError =
				RTPPBBeginPacket(
					( **inGlobals ).itsPacketBuilder, __kNoFlags, thePacketGroup,
					( **inGlobals ).itsPacketSizeLimit, &thePacket );
			
			if( !theError )
			{
				/*	The header (with optional payload description) is added to the
					network packet as literal data.  The data is written directly
					to the network packet.  If the RTPPacketBuilder is storing
					network packet data to disk, it must store a copy of literal data. */
				
				theError =
					RTPPBAddPacketLiteralData(
						( **inGlobals ).itsPacketBuilder, __kNoFlags, thePacketGroup,
						thePacket, REINTERPRET_CAST( UInt8 * )( &theHeader ),
						theHeaderSize, NULL );
				
				if( !theError )
				{
					/*	The RTPPacketBuilder provides a routine specifically for adding
						sample data.  For stored movies, the RTPPacketBuilder need not
						store a copy of sample data, since the data is already stored
						in the movie. */
					
					theError =
						RTPPBAddPacketSampleData(
							( **inGlobals ).itsPacketBuilder, __kNoFlags, thePacketGroup,
							thePacket, CONST_CAST( RTPMPSampleDataParams * )( inSampleData ),
							theDataOffset, theDataSize, NULL );
					
					if( !theError )
					{
						/*	The packetizer sets the RTP/AVP marker bit in the last network
							packet of a frame.  The QuickTime Streaming base RTPReassembler
							can better assist in reassembling the payload data if this bit
							is used to mark the end of a packet group.
							
							Most payloads have no duration, except for the last payload,
							which absorbs the duration for the entire frame. */
						
						if( theDataOffset + theDataSize < ( **inGlobals ).itsFrameDataSize )
						{
							theError =
								RTPPBEndPacket(
									( **inGlobals ).itsPacketBuilder, __kNoFlags,
									thePacketGroup, thePacket, 0 /* inTimeOffset */,
									0 /* inDuration */ );
						}
						
						else
						{
							theError =
								RTPPBEndPacket(
									( **inGlobals ).itsPacketBuilder, kRTPPBSetMarkerFlag,
									thePacketGroup, thePacket, 0 /* inTimeOffset */,
									inSampleData->duration );
						}
					}
				}
			}
			
			
			/*	For this packetizer, packets after the first network packet of a frame
				are uniformly sized and have no payload description. */
			
			if( theDataOffset )
			{
				theDataOffset += theDataSize;
			}
			
			else
			{
				theDataOffset += theDataSize;
				theDataSize = ( **inGlobals ).itsPayloadDataSize;
				theHeaderSize = sizeof( theHeader.itsFixedHeader );
				
				ComponentVideoPayloadSetDescription( &theHeader, 0, 0 );
			}
			
			
			/*	Update the Offset field of the header to indicate the next block
				of sample data. */
			
			ComponentVideoPayloadSetOffset( &theHeader, theDataOffset );
		}
		
		
		if( theError )
		{
			RTPPBEndPacketGroup(
				( **inGlobals ).itsPacketBuilder, kRTPPBDontSendFlag, thePacketGroup );
		}
		
		else
		{
			/*	For this packetizer, every group contains only sync samples.  That
				means the sample data for the group can be decoded independently of
				any previous sample data.  When randomly accessing stored movies,
				a streaming server can look for sync samples. */
			
			theError =
				RTPPBEndPacketGroup(
					( **inGlobals ).itsPacketBuilder, kRTPPBSyncSampleFlag, thePacketGroup );
		}
	}
	
	return( theError );
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
 *	Allocate and initialize storage for instance variables.  When a packetizer
 *	is opened, it is not always called to packetize data, so this function
 *	doesn't perform any allocations or time-consuming operations that are
 *	needed only for packetizing sample data.  The RTPMPInitialize()
 *	implementation performs such operations.
 *
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_Open(
	RTPMPComponentVideoInstanceData **	inGlobals,
	ComponentInstance					self )
{
	ComponentResult		theResult = noErr;
	RTPMediaPacketizer	theBase;
	
	
	inGlobals =
		REINTERPRET_CAST( RTPMPComponentVideoInstanceData ** )(
			NewHandleClear( sizeof( **inGlobals ) ) );
	
	if( inGlobals )
	{
		( **inGlobals ).itself = self;
		( **inGlobals ).itsFinalDerivation = self;
		( **inGlobals ).itsInitialized = false;
		
		SetComponentInstanceStorage( self, REINTERPRET_CAST( Handle )( inGlobals ) );
		
		theResult =
			OpenADefaultComponent(
				kRTPMediaPacketizerType, kRTPBaseMediaPacketizerType, &theBase );
		
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
RTPMPComponentVideo_Close(
	RTPMPComponentVideoInstanceData **	inGlobals,
	ComponentInstance					self )
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
RTPMPComponentVideo_Version(
	RTPMPComponentVideoInstanceData **	inGlobals )
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
RTPMPComponentVideo_Target(
	RTPMPComponentVideoInstanceData **	inGlobals,
	ComponentInstance					target )
{
	ComponentResult		theResult;
	
	
	if( ( **inGlobals ).itsBase )
		theResult = ComponentSetTarget( ( **inGlobals ).itsBase, target );
	else
		theResult = noErr;
	
	if( theResult == noErr )
		( **inGlobals ).itsFinalDerivation = target;
	
	return( theResult );
}



#pragma mark -
#pragma mark *        RTP MEDIA PACKETIZER INTERFACE
#pragma mark -
/* ---------------------------------------------------------------------------
 *		R T P    M E D I A    P A C K E T I Z E R    I N T E R F A C E
 * ---------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------
 *		+ RTPMPInitialize() implementation
 * ---------------------------------------------------------------------------
 *
 *	Prepare to packetize sample data.  This implementation initializes
 *	instance variables that represent the packetization state.
 *
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_Initialize(
	RTPMPComponentVideoInstanceData **	inGlobals,
	SInt32								inFlags )
{
	ComponentResult		theResult;
	
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPMPInitializeSelect ) )
		theResult = RTPMPInitialize( ( **inGlobals ).itsBase, inFlags );
	else
		theResult = noErr;
	
	if( theResult == noErr )
	{
		( **inGlobals ).itsPacketBuilder = NULL;
		( **inGlobals ).itsPacketSizeLimit = __kDefaultPacketSizeLimit;
		( **inGlobals ).itsPacketDurationLimit = __kDefaultPacketDurationLimit;
		
		ComponentVideoPayloadInitialize( &( **inGlobals ).itsPayloadHeader, 0, 0 );
		
		( **inGlobals ).itsInitialized = true;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPPreflightMedia() implementation
 * ---------------------------------------------------------------------------
 *
 *	Determine whether the packetizer can packetize data described by the
 *	given SampleDescription.  This implementation verifies that the sample
 *	data is in Component Video format, and that the image has positive width
 *	and height.
 *
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_PreflightMedia(
	RTPMPComponentVideoInstanceData **	inGlobals,
	OSType								inMediaType,
	SampleDescriptionHandle				inSampleDescription )
{
#pragma unused( inGlobals )
	
	ComponentResult			theResult;
	ImageDescriptionHandle	theDescription;
	
	
	theDescription = REINTERPRET_CAST( ImageDescriptionHandle )( inSampleDescription );
	
	if(
		inMediaType != VideoMediaType  ||
		( **theDescription ).cType != kComponentVideoDataFormat )
	{
		theResult = qtsUnsupportedDataTypeErr;
	}
	
	else if( ( **theDescription ).width <= 0  ||  ( **theDescription ).height <= 0 )
	{
		theResult = qtsUnsupportedFeatureErr;
	}
	
	else
	{
		theResult = noErr;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetSampleData() implementation
 * ---------------------------------------------------------------------------
 *
 *	Use the instance's RTPPacketBuilder to packetize the sample data described
 *	by the RTPMPSampleDataParams parameter.  The sample time of the data in
 *	successive calls is non-decreasing.  In the case of Component Video, this
 *	function is called with parameters describing one frame of data at a time.
 *	
 *	The RTPMPSampleDataParams structure includes a SampleDescription.  This
 *	implementation calls __UpdateSampleDescription() to detect when the
 *	SampleDescription changes and to make any updates necessary to accomodate
 *	the new SampleDescription.
 *
 *	The function then calls __PacketizeSampleData() to divide the data into
 *	network packets using the instance's RTPPacketBuilder.
 *
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_SetSampleData(
	RTPMPComponentVideoInstanceData **	inGlobals,
	const RTPMPSampleDataParams *		inSampleData,
	SInt32 *							outFlags )
{
	ComponentResult				theError = noErr;
	
	
	/* Ignore requests that have no data */
	
	if( inSampleData->dataLength )
	{
		theError = __UpdateSampleDescription( inGlobals, inSampleData );
		
		if( !theError )
			theError = __PacketizeSampleData( inGlobals, inSampleData );
	}
	
	*outFlags = 0;
	
	return( theError );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPFlush() implementation
 * ---------------------------------------------------------------------------
 *
 *	Finish any packetization in progress.  This packetizer doesn't defer any
 *	packetization, so its RTPMPFlush() implementation just gives its base a
 *	chance to flush.
 *
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_Flush(
	RTPMPComponentVideoInstanceData **	inGlobals,
	SInt32				inFlags,
	SInt32 *			outFlags )
{
	ComponentResult		theResult;
	
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPMPFlushSelect ) )
	{
		theResult = RTPMPFlush( ( **inGlobals ).itsBase, inFlags, outFlags );
	}
	
	else
	{
		theResult = noErr;
		*outFlags = 0;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPReset() implementation
 * ---------------------------------------------------------------------------
 *
 *	Abort any packetization in progress and prepare to packetize a new data
 *	stream.  This implementation reinitializes its packetization state and
 *	resets its base.
 *
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_Reset(
	RTPMPComponentVideoInstanceData **	inGlobals,
	SInt32								inFlags )
{
	ComponentResult		theResult;
	
	
	ComponentVideoPayloadInitialize( &( **inGlobals ).itsPayloadHeader, 0, 0 );
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPMPResetSelect ) )
		theResult = RTPMPReset( ( **inGlobals ).itsBase, inFlags );
	else
		theResult = noErr;
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetInfo() implementation
 * ---------------------------------------------------------------------------
 *
 *	Return the information indicated by the selector.  This implemenation
 *	computes a result for the following selectors and delegates all others to
 *	its base.
 *
 *
 *		kRTPMPPayloadTypeInfo	ioParams points to an RTPMPPayloadTypeParams
 *								structure.  This implementation fills in this
 *								structure to indicate it uses a dynamic AVP
 *								payload type.  It copies its payload encoding
 *								name to a buffer described by this structure.
 *
 *		kRTPMPRTPTimeScaleInfo	ioParams points to a TimeScale where the
 *								implementation returns the clock rate, in
 *								Hertz, to be used for RTP timestamps.
 *
 *		kRTPMPMinPayloadSize	ioParams points to a UInt32 where the
 *								implementation returns the number of octets
 *								needed for the fixed header and payload
 *								description used by this packetizer.
 *
 *		kRTPMPPayloadNameInfo	ioParams points to a Str255 where the
 *								implementation returns a human-readable name
 *								for the payload encoding used by this
 *								packetizer.
 *
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_GetInfo(
	RTPMPComponentVideoInstanceData **	inGlobals,
	OSType								inSelector,
	void *								ioParams )
{
	ComponentResult				theError = noErr;
	RTPMPPayloadTypeParams *	thePayloadInfo;
	Str255						theEncodingName;
	
	
	switch( inSelector )
	{
		case kRTPMPPayloadTypeInfo:
			thePayloadInfo = STATIC_CAST( RTPMPPayloadTypeParams * )( ioParams );
			thePayloadInfo->flags = kRTPPayloadTypeDynamicFlag;
			thePayloadInfo->payloadNumber = kRTPPayload_Unknown;
			
			theError =
				GetComponentIndString(
					REINTERPRET_CAST( Component )( ( **inGlobals ).itself ),
					theEncodingName, kRTPMPComponentVideoStringListResource,
					kRTPMPComponentVideoProtocolEncodingString );
			
			if( !theError )
			{
				if( thePayloadInfo->nameLength < ( theEncodingName[ 0 ] + 1 ) )
				{
					theError = paramErr;
				}
				
				else
				{
					BlockMoveData(
						&theEncodingName[ 1 ], thePayloadInfo->payloadName,
						theEncodingName[ 0 ] );
					
					thePayloadInfo->payloadName[ theEncodingName[ 0 ] ] = '\0';
				}
				
				thePayloadInfo->nameLength = theEncodingName[ 0 ] + 1;
			}
		break;
		
		
		case kRTPMPRTPTimeScaleInfo:
			*STATIC_CAST( TimeScale * )( ioParams ) = kComponentVideoRTPTimeScale;
		break;
		
		
		case kRTPMPMinPayloadSize:
			*STATIC_CAST( UInt32 * )( ioParams ) = sizeof( ComponentVideoPayload );
		break;
		
		
		case kRTPMPPayloadNameInfo:
			theError =
				GetComponentIndString(
					REINTERPRET_CAST( Component )( ( **inGlobals ).itself ),
					STATIC_CAST( StringPtr )( ioParams ),
					kRTPMPComponentVideoStringListResource,
					kRTPMPComponentVideoHIEncodingString );
		break;
		
		
		case kRTPMPRequiredSampleDescriptionInfo:
		case kRTPMPMinPacketDuration:
		case kRTPMPSuggestedRepeatPktCountInfo:
		case kRTPMPSuggestedRepeatPktSpacingInfo:
		case kRTPMPMaxPartialSampleSizeInfo:
		case kRTPMPPreferredBufferDelayInfo:
		default:
			theError =
				RTPMPGetInfo( ( **inGlobals ).itsBase, inSelector, ioParams );
		break;
	}
	
	return( theError );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetTimeScale() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_SetTimeScale(
	RTPMPComponentVideoInstanceData **	inGlobals,
	TimeScale							inTimeScale )
{
	( **inGlobals ).itsMediaTimeScale = inTimeScale;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetTimeScale() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_GetTimeScale(
	RTPMPComponentVideoInstanceData **	inGlobals,
	TimeScale *							outTimeScale )
{
	*outTimeScale = ( **inGlobals ).itsMediaTimeScale;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetTimeBase() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_SetTimeBase(
	RTPMPComponentVideoInstanceData **	inGlobals,
	TimeBase							inTimeBase )
{
	( **inGlobals ).itsMediaTimeBase = inTimeBase;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetTimeBase() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_GetTimeBase(
	RTPMPComponentVideoInstanceData **	inGlobals,
	TimeBase *							outTimeBase )
{
	*outTimeBase = ( **inGlobals ).itsMediaTimeBase;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPHasCharacteristic() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_HasCharacteristic(
	RTPMPComponentVideoInstanceData **	inGlobals,
	OSType								inSelector,
	Boolean *							outHasIt )
{
	ComponentResult		theResult = noErr;
	
	
	switch( inSelector )
	{
		case kRTPMPNoSampleDataRequiredCharacteristic:
			*outHasIt = true;
		break;
		
		
		case kRTPMPPartialSamplesRequiredCharacteristic:
		case kRTPMPHasUserSettingsDialogCharacteristic:
		case kRTPMPPrefersReliableTransportCharacteristic:
		case kRTPMPRequiresOutOfBandDimensionsCharacteristic:
			*outHasIt = false;
		break;
		
		
		default:
			theResult =
				RTPMPHasCharacteristic( ( **inGlobals ).itsBase, inSelector, outHasIt );
		break;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetPacketBuilder() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_SetPacketBuilder(
	RTPMPComponentVideoInstanceData **	inGlobals,
	ComponentInstance					inPacketBuilder )
{
	ComponentResult		theError;
	SInt32				theFlags;
	
	
	if( ( **inGlobals ).itsInitialized  &&  ( **inGlobals ).itsPacketBuilder )
		theError = RTPMPFlush( ( **inGlobals ).itsFinalDerivation, 0, &theFlags );
	else
		theError = noErr;
	
	if( !theError )
		( **inGlobals ).itsPacketBuilder = inPacketBuilder;
	
	return( theError );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetPacketBuilder() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_GetPacketBuilder(
	RTPMPComponentVideoInstanceData **	inGlobals,
	ComponentInstance *					outPacketBuilder )
{
	*outPacketBuilder = ( **inGlobals ).itsPacketBuilder;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetMediaType() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_SetMediaType(
	RTPMPComponentVideoInstanceData **	inGlobals,
	OSType								inMediaType )
{
#pragma unused( inGlobals )
	ComponentResult		theResult;
	
	
	if( inMediaType == VideoMediaType )
		theResult = noErr;
	else
		theResult = qtsBadDataErr;
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetMediaType() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_GetMediaType(
	RTPMPComponentVideoInstanceData **	inGlobals,
	OSType *							outMediaType )
{
#pragma unused(inGlobals)

	*outMediaType = VideoMediaType;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetMaxPacketSize() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_SetMaxPacketSize(
	RTPMPComponentVideoInstanceData **	inGlobals,
	UInt32								inMaxPacketSize )
{
	( **inGlobals ).itsPacketSizeLimit = inMaxPacketSize;
	
	__UpdateDataSizes( inGlobals );
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetMaxPacketSize() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_GetMaxPacketSize(
	RTPMPComponentVideoInstanceData **	inGlobals,
	UInt32 *							outMaxPacketSize )
{
	*outMaxPacketSize = ( **inGlobals ).itsPacketSizeLimit;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetMaxPacketDuration() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_SetMaxPacketDuration(
	RTPMPComponentVideoInstanceData **	inGlobals,
	UInt32								inMaxPacketDuration )
{
	( **inGlobals ).itsPacketDurationLimit = inMaxPacketDuration;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetMaxPacketDuration() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPComponentVideo_GetMaxPacketDuration(
	RTPMPComponentVideoInstanceData **	inGlobals,
	UInt32 *							outMaxPacketDuration )
{
	*outMaxPacketDuration = ( **inGlobals ).itsPacketDurationLimit;
	
	return( noErr );
}
