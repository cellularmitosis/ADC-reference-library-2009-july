/*
	File:		RTPMPIMAAudio.c

	Contains:	Definition of IMA Audio RTPMediaPacketizer

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
		
		RTPMPDoUserDialog()				Present a dialog of user-adjustable
										options.
		
		RTPMPSetSettingsFromAtomContainerAtAtom()
										Update instance variables with saved
										user settings.
		
		RTPMPGetSettingsIntoAtomContainerAtAtom()
										Return current settings of user
										adjustable options.
										
		RTPMPGetMediaSettingsAsText()	Return current settings of user
										adjustable options as a string.
*/



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#include "RTPMPIMAAudio.h"
#include "RTPMPIMAAudioResources.h"
#include <FixMath.h>
#include <Math64.h>
#include <Resources.h>
#include <TextUtils.h>



/* ---------------------------------------------------------------------------
 *		R T P    M E D I A    P A C K E T I Z E R    P R O T O T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	QTStreamingComponents.k.h uses these macros to declare prototypes for
 *	the RTPMediaPacketizer calls defined in this file.
 *
 */

#define RTPMP_BASENAME()				RTPMPIMAAudio_
#define RTPMP_GLOBALS()					RTPMPIMAAudioInstanceData **

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
#define COMPONENT_DISPATCH_FILE			"RTPMPIMAAudioDispatch.h"
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

#if TARGET_OS_WIN32
#	define DragWindow( window, startPt, boundsRect )
#endif



enum
{
	__kNoFlags						= 0,
	__kFlush						= true,
	__kDontFlush					= !__kFlush,
	__kDefaultPacketDurationLimit	= 200,			/* 200ms.  See RFC 1890, section 4.1 */
	__kDefaultInterleaveCount		= 3,
	__kTypicalMTUSize				= 1500,			/* Ethernet */
	__kTypicalNetworkHeaderSize		= 20,			/* IP, no options */
	__kTypicalTransportHeaderSize	= 8,			/* UDP */
	__kTypicalRTPHeaderSize			= 12,			/* no CSRCs, no extension */
	__kDefaultPacketSizeLimit		=
		__kTypicalMTUSize - __kTypicalNetworkHeaderSize - __kTypicalTransportHeaderSize -
		__kTypicalRTPHeaderSize
};

enum
{
	__kOKButton				= 1,
	__kCancelButton			= 2,
	__kInterleavingMenu		= 3
};

enum
{
	__kInterleaveCountAtomType = 'ilvc'
};



typedef UInt32	__TStoredInterleaveCount;



/* ---------------------------------------------------------------------------
 *		__Drag()
 * ---------------------------------------------------------------------------
 *
 *	Drag the dialog window on mouseDown.
 *
 */

static
Boolean
__Drag(
	DialogPtr				aDialog,
	const EventRecord *		anEvent )
{
	Boolean		theResult = false;
	WindowPtr	theWindow;
	short		thePart;
	Rect		theBoundary;
	
	
	if( anEvent->what == mouseDown )
	{
		thePart = MacFindWindow( anEvent->where, &theWindow );
		
		if( theWindow == aDialog  &&  thePart == inDrag )
		{
			theBoundary = ( **GetGrayRgn() ).rgnBBox;
			
			DragWindow( aDialog, anEvent->where, &theBoundary );
			
			theResult = true;
		}
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__SettingsDialogFilter()
 * ---------------------------------------------------------------------------
 *
 *	Drag the dialog window if appropriate.  Otherwise dispatch first to the
 *	custom event filter, if any, passed to RTPMPDoUserDialog(), and then, if
 *	necessary, to the standard event filter.
 *
 */

static
pascal
Boolean
__SettingsDialogFilter(
	DialogPtr		aDialog,
	EventRecord *	anEvent,
	short *			anItem )
{
	Boolean			theResult = false;
	ModalFilterUPP	theCustomFilter;
	
	
	if( __Drag( aDialog, anEvent ) )
	{
		*anItem = 0;
		theResult = true;
	}
	
	else
	{
		theCustomFilter = REINTERPRET_CAST( ModalFilterUPP )( GetWRefCon( aDialog ) );
		
		if( theCustomFilter )
			theResult = CallModalFilterProc( theCustomFilter, aDialog, anEvent, anItem );
		
		if( !theResult )
			theResult = StdFilterProc( aDialog, anEvent, anItem );
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__GreatestCommonFactor()
 * ---------------------------------------------------------------------------
 *
 *	Compute the greatest common factor of two positive integers.
 *
 */

static
UInt32
__GreatestCommonFactor(
	UInt32	inDividend,
	UInt32	inDivisor )
{
	UInt32	theRemainder;
	
	
	while( inDivisor )
	{
		theRemainder = inDividend % inDivisor;
		inDividend = inDivisor;
		inDivisor = theRemainder;
	}
	
	return( inDividend );
}



/* ---------------------------------------------------------------------------
 *		__LockInstanceData()
 * ---------------------------------------------------------------------------
 *
 *	Lock relocatable block containing instance data.
 *
 */

static
void
__LockInstanceData(
	RTPMPIMAAudioInstanceData **	inGlobals )
{
	if( !( **inGlobals ).itsLockCount )
	{
		if( ( **inGlobals ).itsInSystemHeap )
			HLock( REINTERPRET_CAST( Handle )( inGlobals ) );
		else
			HLockHi( REINTERPRET_CAST( Handle )( inGlobals ) );
	}
	
	( **inGlobals ).itsLockCount++;
}



/* ---------------------------------------------------------------------------
 *		__UnlockInstanceData()
 * ---------------------------------------------------------------------------
 *
 *	Unlock relocatable block containing instance data.
 *
 */

static
void
__UnlockInstanceData(
	RTPMPIMAAudioInstanceData **	inGlobals )
{
	--( **inGlobals ).itsLockCount;
	
	if( !( **inGlobals ).itsLockCount )
		HUnlock( REINTERPRET_CAST( Handle )( inGlobals ) );
}



/* ---------------------------------------------------------------------------
 *		__InterleaveGroupFrameCount()
 * ---------------------------------------------------------------------------
 *
 *	Compute the number of frames in an interleave group according to the
 *	following constraints:
 *
 *		- no packet exceeds the packet size limit
 *
 *		- no packet exceeds the packet duration limit
 *
 *		- the frame count is a multiple of the channel count
 *
 *		- the group duration is an integer number of clock cycles
 *
 */

static
UInt32
__InterleaveGroupFrameCount(
	RTPMPIMAAudioInstanceData **	inGlobals )
{
	UInt32	theResult;
	UInt64	theDurationLimit;
	UInt64	theSampleRate;
	UInt32	theTimewiseLimit;
	UInt32	theInterleaveCount;
	UInt32	theChannelCount;
	UInt32	theFrameDurationRemainder;
	UInt32	theIntegerDurationFrameCount;
	UInt32	theConstrainingFactor;
	UInt32	theCommonFactor;
	
	
	/* Compute size-wise limit per payload. */
	
	theResult =
		( ( **inGlobals ).itsPayloadSizeLimit -
			( sizeof( IMAAudioPayload ) - sizeof( IMAAudioFrame ) ) ) /
		sizeof( IMAAudioFrame );
	
	
	/* Compute time-wise limit per payload. */
	
	theDurationLimit = U64SetU( ( **inGlobals ).itsPayloadDurationLimit );
	theSampleRate =
		U64SetU( IMAAudioPayloadSampleRate( &( **inGlobals ).itsPayloadAttributes ) );
	
	theTimewiseLimit =
		Fix2Long(
			U32SetU(
				U64Div(
					U64Multiply( theDurationLimit, theSampleRate ),
					U64SetU( 1000UL * kIMAAudioPayloadFrameSampleCount ) ) ) );
	
	
	/* Select lesser limit. */
	
	if( theResult > theTimewiseLimit )
		theResult = theTimewiseLimit;
	
	
	/* Compute limit for entire interleave group. */
	
	theInterleaveCount =
		IMAAudioPayloadInterleaveCount( &( **inGlobals ).itsPayloadAttributes );
	
	theResult *= theInterleaveCount;
	
	
	/*	Compute the least number of frames that, given their sample rate,
		have an integer duration with respect to the RTP clock rate of
		55125 Hz.  As explained in IMAAudioPayload.h, the integer-duration
		frame count will be no greater than four.  An interleave group has
		integer duration when its frame count is a multiple of the integer-
		duration frame count. */
	
	theFrameDurationRemainder =
		U64Mod(
			U64Multiply(
				U64SetU( STATIC_CAST( UInt32 )( kIMAAudioPayloadRTPTimeScale ) << 16 ),
				U64SetU( kIMAAudioPayloadFrameSampleCount ) ),
			theSampleRate );
	
	if( theFrameDurationRemainder )
	{
		theIntegerDurationFrameCount =
			U32SetU( U64Div( theSampleRate, theFrameDurationRemainder ) );
	}
	
	else
	{
		theIntegerDurationFrameCount = 1;
	}
	
	
	/*	Restrict group frame count to a multiple of both the channel count and
		the integer-duration frame count. */
	
	theChannelCount = IMAAudioPayloadChannelCount( &( **inGlobals ).itsPayloadAttributes );
	
	theCommonFactor =
		__GreatestCommonFactor( theIntegerDurationFrameCount, theChannelCount );
	
	theConstrainingFactor =
		( theIntegerDurationFrameCount / theCommonFactor ) *
			( theChannelCount / theCommonFactor );
	
	theResult = ( theResult / theConstrainingFactor ) * theConstrainingFactor;
	
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__UpdateLimits()
 * ---------------------------------------------------------------------------
 *
 *	Update values used to construct network packets.
 *
 */

static
void
__UpdateLimits(
	RTPMPIMAAudioInstanceData **	inGlobals )
{
	if( ( **inGlobals ).itsPayloadAttributesInitialized )
	{
		/*	Update count of frames in each interleave group. */
		
		( **inGlobals ).itsInterleaveGroupFrameCount =
			__InterleaveGroupFrameCount( inGlobals );
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
	RTPMPIMAAudioInstanceData **	inGlobals,
	const RTPMPSampleDataParams *	inSampleData )
{
	ComponentResult			theError = noErr;
	SInt32					theFlags;
	SoundDescriptionHandle	theDescription;
	CompressionInfo			theCompressionInfo;
	UnsignedFixed			thePayloadSampleRate;
	
	
	/*	Check the SampleDescription if it is the first description or if the cached
		sample description seed doesn't match the current seed. */
	
	if(
		!( **inGlobals ).itsPayloadAttributesInitialized  ||
		( **inGlobals ).itsSampleDescriptionSeed != inSampleData->sampleDescSeed )
	{
		/*	If the RTPMPSetSampleData() implementation queues data, then any
			queued sample data, which uses the obsolete SampleDescription, must
			be flushed before updating to the new SampleDescription. */
		
		theError = RTPMPFlush( ( **inGlobals ).itsFinalDerivation, 0, &theFlags );
		
		if( !theError )
		{
			theDescription =
				REINTERPRET_CAST( SoundDescriptionHandle )(
					inSampleData->sampleDescription );
			
			theError =
				GetCompressionInfo(
					fixedCompression, ( **theDescription ).dataFormat,
					( **theDescription ).numChannels, ( **theDescription ).sampleSize,
					&theCompressionInfo );
			
			
			/*	Update the payload header that the packetizer includes in each network
				packet. */
			
			IMAAudioPayloadSetChannelCount(
				&( **inGlobals ).itsPayloadAttributes, ( **theDescription ).numChannels );
			
			thePayloadSampleRate =
				IMAAudioPayloadSetSampleRate(
					&( **inGlobals ).itsPayloadAttributes,
					( **theDescription ).sampleRate );
			
			( **inGlobals ).itsPayloadAttributesInitialized = true;
			
			
			IMAAudioQueueSetFlowControl(
				&( **inGlobals ).itsAudioQueue, ( **theDescription ).sampleRate,
				thePayloadSampleRate, ( **theDescription ).numChannels );
			
			
			/*	Update values used to construct network packets. */
			
			__UpdateLimits( inGlobals );
			
			
			/*	Update the cached sample description seed to indicate that the
				packetizer state is now consistent with the new SampleDescription. */
			
			( **inGlobals ).itsSampleDescriptionSeed = inSampleData->sampleDescSeed;
		}
	}
	
	return( theError );
}



/* ---------------------------------------------------------------------------
 *		__SampleBlockDuration()
 * ---------------------------------------------------------------------------
 *
 *	Compute the duration of the described sample block.
 *
 */

static
UInt32
__SampleBlockDuration(
	const RTPMPSampleDataParams *	inSampleData )
{
	UInt32					theResult;
	UInt32					theSampleCount;
	SoundDescriptionHandle	theDescription;
	UInt64					theFixedSampleCount;
	
	
	if( inSampleData->duration )
	{
		theResult = inSampleData->duration;
	}
	
	else
	{
		theDescription =
			REINTERPRET_CAST( SoundDescriptionHandle )( inSampleData->sampleDescription );
		
		theSampleCount =
			kIMAAudioPayloadFrameSampleCount *
			( inSampleData->dataLength / sizeof( IMAAudioFrame ) );
		
		theFixedSampleCount = U64Multiply( U64SetU( theSampleCount ), U64SetU( fixed1 ) );
		
		theResult =
			U32SetU(
				U64Div(
					U64Multiply(
						theFixedSampleCount, U64SetU( kIMAAudioPayloadRTPTimeScale ) ),
					U64SetU( ( **theDescription ).sampleRate ) ) );
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__BeginInterleaveGroup()
 * ---------------------------------------------------------------------------
 *
 *	Use the RTPPacketBuilder to create a new packet group and all the packets
 *	in that group.  For this packetizer, a packet group contains one
 *	interleave group.
 *	
 *	Each network packet consists of a fixed header followed by sample data.
 *	This function adds the fixed header to each packet.
 *
 */

static
ComponentResult
__BeginInterleaveGroup(
	const IMAAudioPayload *		inPayloadAttributes,
	TimeValue64					inTimestamp,
	UInt32						inFrameCount,
	RTPPacketBuilder			inPacketBuilder,
	RTPPacketGroupRef *			outPacketGroup,
	RTPPacketRef *				outPackets )
{
	ComponentResult		theResult;
	UInt32				theInterleaveCount;
	UInt32				thePayloadFrameCount;
	UInt32				theIndex;
	IMAAudioPayload		theHeader;
	UInt32				theHeaderSize;
	UInt32				thePayloadSizeLimit;
	
	
	theResult =
		RTPPBBeginPacketGroup( inPacketBuilder, __kNoFlags, inTimestamp, outPacketGroup );
	
	theInterleaveCount = IMAAudioPayloadInterleaveCount( inPayloadAttributes );
	thePayloadFrameCount = ( inFrameCount + theInterleaveCount - 1 ) / theInterleaveCount;
	
	theHeader = *inPayloadAttributes;
	theHeaderSize = sizeof( theHeader ) - sizeof( IMAAudioFrame );
	
	thePayloadSizeLimit =
		theHeaderSize + ( thePayloadFrameCount * sizeof( IMAAudioFrame ) );
	
	
	/*	If there aren't enough frames to fill an entire interleave group, omit the
		empty packets. */
	
	if( theInterleaveCount > inFrameCount )
		theInterleaveCount = inFrameCount;
	
	for( theIndex = 0; theIndex < theInterleaveCount  &&  theResult == noErr; theIndex++ )
	{
		theResult =
			RTPPBBeginPacket(
				inPacketBuilder, __kNoFlags, *outPacketGroup, thePayloadSizeLimit,
				&outPackets[ theIndex ] );
		
		if( theResult == noErr )
		{
			/*	The header is added to the network packet as literal data.  Literal
				data is written directly to the network packet.  If the
				RTPPacketBuilder is storing network packet data to disk, it must
				store a copy of literal data. */
			
			theResult =
				RTPPBAddPacketLiteralData(
					inPacketBuilder, __kNoFlags, *outPacketGroup, outPackets[ theIndex ],
					REINTERPRET_CAST( UInt8 * )( &theHeader ), theHeaderSize, NULL );
			
			IMAAudioPayloadIncrementInterleaveIndex( &theHeader );
		}
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__EndInterleaveGroup()
 * ---------------------------------------------------------------------------
 *
 *	Use the RTPPacketBuilder to close the current packet group and all its
 *	packets.
 *
 */

static
ComponentResult
__EndInterleaveGroup(
	const IMAAudioPayload *		inPayloadAttributes,
	UInt32						inFrameCount,
	RTPPacketBuilder			inPacketBuilder,
	RTPPacketGroupRef			inPacketGroup,
	const RTPPacketRef *		inPackets )
{
	ComponentResult		theResult = noErr;
	UnsignedFixed		theSampleRate;
	UInt32				theDuration;
	UInt32				theInterleaveCount;
	UInt32				theIndex;
	
	
	theSampleRate = IMAAudioPayloadSampleRate( inPayloadAttributes );
	
	theDuration =
		U32SetU(
			U64Div(
				U64Multiply(
					U64SetU( inFrameCount * kIMAAudioPayloadFrameSampleCount ),
					U64SetU( STATIC_CAST( UInt32 )( kIMAAudioPayloadRTPTimeScale ) << 16 ) ),
				U64SetU( theSampleRate ) ) );
	
	theDuration /= IMAAudioPayloadChannelCount( inPayloadAttributes );
	
	theInterleaveCount = IMAAudioPayloadInterleaveCount( inPayloadAttributes );
	
	
	/*	If there aren't enough frames to fill an entire interleave group, the empty
		packets have been omitted. */
	
	if( theInterleaveCount > inFrameCount )
		theInterleaveCount = inFrameCount;
	
	
	/*	To simplify reassembly, this packetizer uses the timestamp of the start
		of the group as the timestamp for each network packet in the group.  When
		reassembling, the derived RTPReassembler relies on the QuickTime Streaming
		base RTPReassembler to group together packets that have the same timestamp.
		
		A packet's duration is the time between its timestamp and the timestamp
		of the next packet.  Since these packets share the same timestamp, most
		have zero duration.  The last packet in the group reflects the duration
		of the entire group. */
	
	for(
		theIndex = 0; theIndex < ( theInterleaveCount - 1 )  &&  theResult == noErr;
		theIndex++ )
	{
		theResult =
			RTPPBEndPacket(
				inPacketBuilder, __kNoFlags, inPacketGroup, inPackets[ theIndex ],
				0 /* inTimeOffset */ , 0 /* inDuration */ );
	}
	
	if( theResult == noErr )
	{
		/*	The packetizer sets the RTP/AVP marker bit in the last network
			packet of an interleave group.  The QuickTime Streaming base
			RTPReassembler can better assist in reassembling the payload data
			if this bit is used to mark the end of a packet group. */
		
		theResult =
			RTPPBEndPacket(
				inPacketBuilder, kRTPPBSetMarkerFlag, inPacketGroup, inPackets[ theIndex ],
				0, theDuration );
		
		if( theResult == noErr )
		{
			/*	For this packetizer, every group contains only sync samples.  That
				means the sample data for the group can be decoded independently of
				any previous sample data.  When randomly accessing stored movies,
				a streaming server can look for sync samples. */
			
			theResult =
				RTPPBEndPacketGroup( inPacketBuilder, kRTPPBSyncSampleFlag, inPacketGroup );
		}
		
		else
		{
			RTPPBEndPacketGroup( inPacketBuilder, kRTPPBDontSendFlag, inPacketGroup );
		}
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__PacketizeInterleaveGroup()
 * ---------------------------------------------------------------------------
 *
 *	Use the RTPPacketBuilder and the queued sample data to construct a single
 *	interleave group of network packets.
 *	
 */

static
ComponentResult
__PacketizeInterleaveGroup(
	IMAAudioQueue *				inAudioQueue,
	const IMAAudioPayload *		inPayloadAttributes,
	UInt32						inInterleaveGroupFrameCount,
	RTPPacketBuilder			inPacketBuilder )
{
	ComponentResult			theResult;
	IMAAudioPayload			thePayloadAttributes;
	UInt32					theFrameCount;
	UInt32					theInterleaveCount;
	IMAAudioQueueElement	theFrameInfo;
	RTPPacketGroupRef		thePacketGroup;
	RTPPacketRef			thePackets[ kIMAAudioPayloadInterleaveCountLimit ];
	UInt32					theFrameIndex;
	
	
	/* Get the total queued frame count. */
	
	theFrameCount = IMAAudioQueueCount( inAudioQueue );
	
	if( theFrameCount )
	{
		/* Limit the frame count to a single interleave group. */
		
		if( theFrameCount >= inInterleaveGroupFrameCount )
			theFrameCount = inInterleaveGroupFrameCount;
		
		thePayloadAttributes = *inPayloadAttributes;
		
		theInterleaveCount = IMAAudioPayloadInterleaveCount( &thePayloadAttributes );
		
		IMAAudioPayloadSetInterleaving(
			&thePayloadAttributes, theInterleaveCount, theFrameCount );
		
		
		/* Initialize the interleave group. */
		
		IMAAudioQueueDequeue( inAudioQueue, &theFrameInfo );
		
		theResult =
			__BeginInterleaveGroup(
				&thePayloadAttributes, theFrameInfo.itsTimestamp, theFrameCount,
				inPacketBuilder, &thePacketGroup, thePackets );
		
		
		/* Interleave audio frames into network packets. */
		
		theFrameIndex = 0;
		
		do
		{
			/*	The RTPPacketBuilder provides a routine specifically for adding
				sample data.  For stored movies, the RTPPacketBuilder need not
				store a copy of sample data, since the data is already stored
				in the movie. */
			
			theResult =
				RTPPBAddPacketSampleData(
					inPacketBuilder, __kNoFlags, thePacketGroup,
					thePackets[ theFrameIndex % theInterleaveCount ],
					theFrameInfo.itsSampleDataParams, theFrameInfo.itsOffset,
					sizeof( IMAAudioFrame ), NULL );
			
			theFrameIndex++;
		}
		while(
			theResult == noErr  &&  theFrameIndex < theFrameCount  &&
			IMAAudioQueueDequeue( inAudioQueue, &theFrameInfo ) );
		
		
		/* End the interleave group. */
		
		if( theResult == noErr )
		{
			theResult =
				__EndInterleaveGroup(
					&thePayloadAttributes, theFrameCount, inPacketBuilder,
					thePacketGroup, thePackets );
		}
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		__ProcessAudioQueue()
 * ---------------------------------------------------------------------------
 *
 *	When flushing, packetize all queued audio data.  Otherwise, packetize
 *	interleave groups continuously until there is not enough data to fill an
 *	interleave group.
 *
 */

static
ComponentResult
__ProcessAudioQueue(
	RTPMPIMAAudioInstanceData **	inGlobals,
	Boolean							inFlush )
{
	ComponentResult		theResult = noErr;
	IMAAudioPayload		thePayloadAttributes = ( **inGlobals ).itsPayloadAttributes;
	UInt32				theMinimumFrameCount;
	UInt32				theFrameCount;
	
	
	__LockInstanceData( inGlobals );
	
	if( inFlush )
		theMinimumFrameCount = 1;
	else
		theMinimumFrameCount = ( **inGlobals ).itsInterleaveGroupFrameCount;
	
	for(
		theFrameCount = IMAAudioQueueCount( &( **inGlobals ).itsAudioQueue );
		theFrameCount >= theMinimumFrameCount  &&  theResult == noErr;
		theFrameCount = IMAAudioQueueCount( &( **inGlobals ).itsAudioQueue ) )
	{
		if( theFrameCount > ( **inGlobals ).itsInterleaveGroupFrameCount )
			theFrameCount = ( **inGlobals ).itsInterleaveGroupFrameCount;
		
		theResult =
			__PacketizeInterleaveGroup(
				&( **inGlobals ).itsAudioQueue, &thePayloadAttributes, theFrameCount,
				( **inGlobals ).itsPacketBuilder );
	}
	
	__UnlockInstanceData( inGlobals );
	
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
 *	Allocate and initialize storage for instance variables.  When a packetizer
 *	is opened, it is not always called to packetize data, so this function
 *	doesn't perform any allocations or time-consuming operations that are
 *	needed only for packetizing sample data.  The RTPMPInitialize()
 *	implementation performs such operations.
 *
 *	Since the RTPMPGetSettingsAsText() implementation and the
 *	RTPMPDoUserDialog() implementation might be called before calling the
 *	RTPMPInitialize() implementation, this function must initialize any
 *	instance variables used by those functions.
 *	
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_Open(
	RTPMPIMAAudioInstanceData **	inGlobals,
	ComponentInstance				self )
{
	ComponentResult		theResult = noErr;
	RTPMediaPacketizer	theBase;
	
	
	inGlobals =
		REINTERPRET_CAST( RTPMPIMAAudioInstanceData ** )(
			NewHandleClear( sizeof( **inGlobals ) ) );
	
	if( inGlobals )
	{
		( **inGlobals ).itself = self;
		( **inGlobals ).itsFinalDerivation = self;
		( **inGlobals ).itsInSystemHeap =
			( HandleZone( REINTERPRET_CAST( Handle )( inGlobals ) ) == SystemZone() );
		( **inGlobals ).itsLockCount = 0;
		( **inGlobals ).itsInitialized = false;
		( **inGlobals ).itsPayloadAttributesInitialized = false;
		
		IMAAudioPayloadInitialize( &( **inGlobals ).itsPayloadAttributes );
		
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
RTPMPIMAAudio_Close(
	RTPMPIMAAudioInstanceData **	inGlobals,
	ComponentInstance				self )
{
#pragma unused( self )
	
	IMAAudioQueue	theAudioQueue;
	
	
	if( inGlobals )
	{
		if( ( **inGlobals ).itsInitialized )
		{
			theAudioQueue = ( **inGlobals ).itsAudioQueue;
			IMAAudioQueueFlush( &theAudioQueue );
		}
		
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
RTPMPIMAAudio_Version(
	RTPMPIMAAudioInstanceData **	inGlobals )
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
RTPMPIMAAudio_Target(
	RTPMPIMAAudioInstanceData **	inGlobals,
	ComponentInstance				target )
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
RTPMPIMAAudio_Initialize(
	RTPMPIMAAudioInstanceData **	inGlobals,
	SInt32							inFlags )
{
	ComponentResult		theResult;
	
	
	if( CallComponentCanDo( ( **inGlobals ).itsBase, kRTPMPInitializeSelect ) )
		theResult = RTPMPInitialize( ( **inGlobals ).itsBase, inFlags );
	else
		theResult = noErr;
	
	if( theResult == noErr )
	{
		( **inGlobals ).itsExpectedTimestamp = 0;
		( **inGlobals ).itsPacketBuilder = NULL;
		( **inGlobals ).itsPayloadSizeLimit = __kDefaultPacketSizeLimit;
		( **inGlobals ).itsPayloadDurationLimit = __kDefaultPacketDurationLimit;
		( **inGlobals ).itsInterleaveGroupFrameCount = 0;
		
		IMAAudioQueueInitialize( &( **inGlobals ).itsAudioQueue );
		
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
 *	data is in IMA Audio format.
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_PreflightMedia(
	RTPMPIMAAudioInstanceData **	inGlobals,
	OSType							inMediaType,
	SampleDescriptionHandle			inSampleDescription )
{
#pragma unused( inGlobals )
	
	ComponentResult			theResult;
	SoundDescriptionHandle	theDescription;
	
	
	theDescription = REINTERPRET_CAST( SoundDescriptionHandle )( inSampleDescription );
	
	if(
		inMediaType != SoundMediaType  ||
		( **theDescription ).dataFormat != kIMACompression )
	{
		theResult = qtsUnsupportedDataTypeErr;
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
 *	successive calls is non-decreasing, but could be non-contiguous--that is,
 *	the data might have gaps.  This implementation guarantees that any queued
 *	data is always contiguous by flushing when it detects non-contiguous data.
 *	
 *	The RTPMPSampleDataParams structure includes a SampleDescription.  This
 *	implementation calls __UpdateSampleDescription() to detect when the
 *	SampleDescription changes and to make any updates necessary to accomodate
 *	the new SampleDescription.
 *
 *	The function then queues the sample data parameters and calls
 *	__ProcessAudioQueue() to packetize queued sample data.
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_SetSampleData(
	RTPMPIMAAudioInstanceData **	inGlobals,
	const RTPMPSampleDataParams *	inSampleData,
	SInt32 *						outFlags )
{
	ComponentResult		theResult;
	SInt32				theFlags;
	
	
	/* Ignore requests that have no data */
	
	if( inSampleData->dataLength )
	{
		/* Flush before queueing non-contiguous data. */
		
		if( inSampleData->timeStamp != 	( **inGlobals ).itsExpectedTimestamp )
			theResult = RTPMPFlush( ( **inGlobals ).itsFinalDerivation, 0, &theFlags );
		else
			theResult = noErr;
		
		if( theResult == noErr )
		{
			theResult = __UpdateSampleDescription( inGlobals, inSampleData );
			
			if( theResult == noErr )
			{
				__LockInstanceData( inGlobals );
				IMAAudioQueueEnqueue( &( **inGlobals ).itsAudioQueue, inSampleData );
				__UnlockInstanceData( inGlobals );
				
				theResult = __ProcessAudioQueue( inGlobals, __kDontFlush );
				
				if( theResult == noErr )
				{
					( **inGlobals ).itsExpectedTimestamp =
						inSampleData->timeStamp + __SampleBlockDuration( inSampleData );
				}
			}
		}
	}
	
	*outFlags = 0;
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPFlush() implementation
 * ---------------------------------------------------------------------------
 *
 *	Finish any packetization in progress.
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_Flush(
	RTPMPIMAAudioInstanceData **	inGlobals,
	SInt32							inFlags,
	SInt32 *						outFlags )
{
	ComponentResult		theResult;
	
	
	theResult = __ProcessAudioQueue( inGlobals, __kFlush );
	
	__LockInstanceData( inGlobals );
	IMAAudioQueueFlush( &( **inGlobals ).itsAudioQueue );
	__UnlockInstanceData( inGlobals );
	
	( **inGlobals ).itsExpectedTimestamp = 0;
	
	if(
		theResult == noErr  &&
		CallComponentCanDo( ( **inGlobals ).itsBase, kRTPMPFlushSelect ) )
	{
		theResult = RTPMPFlush( ( **inGlobals ).itsBase, inFlags, outFlags );
	}
	
	else
	{
		*outFlags = 0;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPReset() implementation
 * ---------------------------------------------------------------------------
 *
 *	Abort any packetization in progress and prepare to packetize a new,
 *	possibly unrelated, data stream.  This implementation flushes its audio
 *	data queue, invalidates its cached payload attributes, and resets its
 *	base.
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_Reset(
	RTPMPIMAAudioInstanceData **	inGlobals,
	SInt32							inFlags )
{
	ComponentResult		theResult;
	
	
	__LockInstanceData( inGlobals );
	IMAAudioQueueFlush( &( **inGlobals ).itsAudioQueue );
	__UnlockInstanceData( inGlobals );
	
	( **inGlobals ).itsExpectedTimestamp = 0;
	( **inGlobals ).itsPayloadAttributesInitialized = false;
	
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
RTPMPIMAAudio_GetInfo(
	RTPMPIMAAudioInstanceData **	inGlobals,
	OSType							inSelector,
	void *							ioParams )
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
					theEncodingName, kRTPMPIMAAudioStringListResource,
					kRTPMPIMAAudioProtocolEncodingString );
			
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
			*STATIC_CAST( TimeScale * )( ioParams ) = kIMAAudioPayloadRTPTimeScale;
		break;
		
		
		case kRTPMPMinPayloadSize:
			*STATIC_CAST( UInt32 * )( ioParams ) = sizeof( IMAAudioPayload );
		break;
		
		
		case kRTPMPPayloadNameInfo:
			theError =
				GetComponentIndString(
					REINTERPRET_CAST( Component )( ( **inGlobals ).itself ),
					STATIC_CAST( StringPtr )( ioParams ),
					kRTPMPIMAAudioStringListResource, kRTPMPIMAAudioHIEncodingString );
		break;
		
		
		case kRTPMPMinPacketDuration:
			theError = qtsBadSelectorErr;
		break;
		
		
		case kRTPMPRequiredSampleDescriptionInfo:
		case kRTPMPSuggestedRepeatPktCountInfo:
		case kRTPMPSuggestedRepeatPktSpacingInfo:
		case kRTPMPMaxPartialSampleSizeInfo:
		case kRTPMPPreferredBufferDelayInfo:
		default:
			theError = RTPMPGetInfo( ( **inGlobals ).itsBase, inSelector, ioParams );
		break;
	}
	
	return( theError );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetTimeScale() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_SetTimeScale(
	RTPMPIMAAudioInstanceData **	inGlobals,
	TimeScale						inTimeScale )
{
	( **inGlobals ).itsMediaTimeScale = inTimeScale;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetTimeScale() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_GetTimeScale(
	RTPMPIMAAudioInstanceData **	inGlobals,
	TimeScale *						outTimeScale )
{
	*outTimeScale = ( **inGlobals ).itsMediaTimeScale;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetTimeBase() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_SetTimeBase(
	RTPMPIMAAudioInstanceData **	inGlobals,
	TimeBase						inTimeBase )
{
	( **inGlobals ).itsMediaTimeBase = inTimeBase;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetTimeBase() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_GetTimeBase(
	RTPMPIMAAudioInstanceData **	inGlobals,
	TimeBase *						outTimeBase )
{
	*outTimeBase = ( **inGlobals ).itsMediaTimeBase;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPHasCharacteristic() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_HasCharacteristic(
	RTPMPIMAAudioInstanceData **	inGlobals,
	OSType							inSelector,
	Boolean *						outHasIt )
{
	ComponentResult		theResult = noErr;
	
	
	switch( inSelector )
	{
		case kRTPMPNoSampleDataRequiredCharacteristic:
		case kRTPMPHasUserSettingsDialogCharacteristic:
			*outHasIt = true;
		break;
		
		
		case kRTPMPPartialSamplesRequiredCharacteristic:
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
RTPMPIMAAudio_SetPacketBuilder(
	RTPMPIMAAudioInstanceData **	inGlobals,
	ComponentInstance				inPacketBuilder )
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
RTPMPIMAAudio_GetPacketBuilder(
	RTPMPIMAAudioInstanceData **	inGlobals,
	ComponentInstance *				outPacketBuilder )
{
	*outPacketBuilder = ( **inGlobals ).itsPacketBuilder;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetMediaType() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_SetMediaType(
	RTPMPIMAAudioInstanceData **	inGlobals,
	OSType							inMediaType )
{
#pragma unused( inGlobals )
	ComponentResult		theResult;
	
	
	if( inMediaType == SoundMediaType )
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
RTPMPIMAAudio_GetMediaType(
	RTPMPIMAAudioInstanceData **	inGlobals,
	OSType *						outMediaType )
{
#pragma unused( inGlobals )

	*outMediaType = SoundMediaType;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetMaxPacketSize() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_SetMaxPacketSize(
	RTPMPIMAAudioInstanceData **	inGlobals,
	UInt32							inMaxPacketSize )
{
	( **inGlobals ).itsPayloadSizeLimit = inMaxPacketSize;
	
	__UpdateLimits( inGlobals );
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetMaxPacketSize() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_GetMaxPacketSize(
	RTPMPIMAAudioInstanceData **	inGlobals,
	UInt32 *						outMaxPacketSize )
{
	*outMaxPacketSize = ( **inGlobals ).itsPayloadSizeLimit;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPSetMaxPacketDuration() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_SetMaxPacketDuration(
	RTPMPIMAAudioInstanceData **	inGlobals,
	UInt32							inMaxPacketDuration )
{
	( **inGlobals ).itsPayloadDurationLimit = inMaxPacketDuration;
	
	__UpdateLimits( inGlobals );
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetMaxPacketDuration() implementation
 * ---------------------------------------------------------------------------
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_GetMaxPacketDuration(
	RTPMPIMAAudioInstanceData **	inGlobals,
	UInt32 *						outMaxPacketDuration )
{
	*outMaxPacketDuration = ( **inGlobals ).itsPayloadDurationLimit;
	
	return( noErr );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPDoUserDialog() implementation
 * ---------------------------------------------------------------------------
 *
 *	Present a dialog of user-adjustable options and update instance variables
 *	according to the user's choices.  This implementation allows the user to
 *	select an interleave count.
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_DoUserDialog(
	RTPMPIMAAudioInstanceData **	inGlobals,
	ModalFilterUPP					inFilterUPP,
	Boolean *						outCanceled )
{
	ComponentResult		theResult;
	ModalFilterUPP		theModalFilter;
	short				theItem;
	DialogPtr			theDialog;
	short				theSavedResources;
	short				theResources;
	short				theItemType;
	Rect				theItemRect;
	ControlHandle		theInterleavingMenu;
	UInt32				theInterleaveCount;
	
	
	theModalFilter = NewModalFilterProc( __SettingsDialogFilter );
	
	if( theModalFilter )
	{
		theSavedResources = CurResFile();
		
		theResources =
			OpenComponentResFile( REINTERPRET_CAST( Component )( ( **inGlobals ).itself ) );
		
		if( theResources > 0 )
		{
			theDialog =
				GetNewDialog(
					kRTPMPIMAAudioSettingsDialogResource, NULL,
					REINTERPRET_CAST( WindowPtr )( -1L ) );
			
			if( theDialog )
			{
				SetDialogDefaultItem( theDialog, __kOKButton );
				SetDialogCancelItem( theDialog, __kCancelButton );
				
				GetDialogItem(
					theDialog, __kInterleavingMenu, &theItemType,
					REINTERPRET_CAST( Handle * )( &theInterleavingMenu ), &theItemRect );
				
				theInterleaveCount =
					IMAAudioPayloadInterleaveCount( &( **inGlobals ).itsPayloadAttributes );
				
				if( theInterleaveCount > 1 )
					theInterleaveCount++;
				
				SetControlValue( theInterleavingMenu, theInterleaveCount );
				HiliteControl( theInterleavingMenu, kControlNoPart );
				
				MacShowWindow( theDialog );
				
				if( theModalFilter )
					SetWRefCon( theDialog, REINTERPRET_CAST( long )( inFilterUPP ) );
				else
					theModalFilter = inFilterUPP;
				
				do
				{
					ModalDialog( theModalFilter, &theItem );
				}
				while( theItem != __kOKButton  && theItem != __kCancelButton );
				
				if( theItem == __kOKButton )
				{
					theInterleaveCount = GetControlValue( theInterleavingMenu );
					
					if( theInterleaveCount > 2 )
						--theInterleaveCount;
					
					IMAAudioPayloadSetInterleaving(
						&( **inGlobals ).itsPayloadAttributes, theInterleaveCount, 0 );
				}
				
				*outCanceled = ( theItem == __kCancelButton );
				
				DisposeDialog( theDialog );
			}
			
			else
			{
				theResult = ResError();
				
				if( theResult == noErr )
					theResult = memFullErr;
			}
			
			UseResFile( theSavedResources );
			CloseComponentResFile( theResources );
			
			theResult = noErr;
		}
		
		else
		{
			theResult = ResError();
			
			if( theResult == noErr )
				theResult = memFullErr;
		}
		
		DisposeRoutineDescriptor( theModalFilter );
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
 *		+ RTPMPSetSettingsFromAtomContainerAtAtom() implementation
 * ---------------------------------------------------------------------------
 *
 *	Update instance variables according to the saved user settings in the
 *	given atom container.
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_SetSettingsFromAtomContainerAtAtom(
	RTPMPIMAAudioInstanceData **	inGlobals,
	QTAtomContainer					inAtomContainer,
	QTAtom							inParentAtom )
{
	ComponentResult				theResult = noErr;
	QTAtom						theAtom;
	QTAtomID					theAtomID;
	Ptr							theAtomData;
	long						theDataSize;
	__TStoredInterleaveCount	theInterleaveCount;
	
	
	if( inAtomContainer )
	{
		theAtom =
			QTFindChildByIndex(
				inAtomContainer, inParentAtom, __kInterleaveCountAtomType, 1, &theAtomID );
		
		if( theAtom )
		{
			QTLockContainer( inAtomContainer );
			
			QTGetAtomDataPtr(
				inAtomContainer, theAtom, &theDataSize, &theAtomData );
			
			if( theDataSize == sizeof( theInterleaveCount ) )
			{
				/* Convert from storage byte order to platform byte order. */
				
				theInterleaveCount =
					EndianU32_BtoN(
						*REINTERPRET_CAST( __TStoredInterleaveCount * )(
							theAtomData ) );
				
				if( theInterleaveCount > kIMAAudioPayloadInterleaveCountLimit )
				{
					theResult = qtsBadDataErr;
				}
				
				else
				{
					IMAAudioPayloadSetInterleaving(
						&( **inGlobals ).itsPayloadAttributes, theInterleaveCount, 0 );
				}
			}
			
			else
			{
				theResult = qtsBadDataErr;
			}
			
			QTUnlockContainer( inAtomContainer );
		}
	}
	
	else
	{
		theResult = paramErr;
	}

	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetSettingsIntoAtomContainerAtAtom() implementation
 * ---------------------------------------------------------------------------
 *
 *	Return current settings of user-adjustable options in the given atom
 *	container.
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_GetSettingsIntoAtomContainerAtAtom(
	RTPMPIMAAudioInstanceData **	inGlobals,
	QTAtomContainer					inOutAtomContainer,
	QTAtom							inParentAtom )
{
	ComponentResult				theResult;
	QTAtom						theAtom;
	QTAtomID					theAtomID;
	__TStoredInterleaveCount	theInterleaveCount;
	
	
	if( inOutAtomContainer )
	{
		/* Convert from platform byte order to storage byte order. */
		
		theInterleaveCount =
			EndianU32_NtoB(
				IMAAudioPayloadInterleaveCount( &( **inGlobals ).itsPayloadAttributes ) );
		
		
		/* If the atom already exists, update it.  Otherwise, insert a new one. */
		
		theAtom =
			QTFindChildByIndex(
				inOutAtomContainer, inParentAtom, __kInterleaveCountAtomType, 1, &theAtomID );
		
		if( theAtom )
		{
			theResult =
				QTSetAtomData(
					inOutAtomContainer, theAtom, sizeof( theInterleaveCount ),
					&theInterleaveCount );
		}
		
		else
		{
			theResult =
				QTInsertChild(
					inOutAtomContainer, inParentAtom, __kInterleaveCountAtomType,
					0 /* id */, 0 /* index */, sizeof( theInterleaveCount ),
					&theInterleaveCount, NULL /* atom */);
		}
	}
	
	else
	{
		theResult = paramErr;
	}
	
	return( theResult );
}



/* ---------------------------------------------------------------------------
 *		+ RTPMPGetMediaSettingsAsText() implementation
 * ---------------------------------------------------------------------------
 *
 *	Return current settings of user-adjustable options as a string.  This
 *	implementation returns the interleave count as colon-delimited name-value
 *	pair:
 *
 *		"Interleaving: <interleave count>"
 *
 *			or
 *
 *		"Interleaving: None"
 *
 */

EXTERN_API( ComponentResult )
RTPMPIMAAudio_GetSettingsAsText(
	RTPMPIMAAudioInstanceData **	inGlobals,
	Handle *						text )
{
	ComponentResult		theResult;
	Str255				theSettings;
	UInt32				theInterleaveCount;
	unsigned char		theSavedCharacter;
	int					theLength;
	
	
	theResult =
		GetComponentIndString(
			REINTERPRET_CAST( Component )( ( **inGlobals ).itself ), theSettings,
			kRTPMPIMAAudioStringListResource, kRTPMPIMAAudioSettingsString );
	
	if( theResult == noErr )
	{
		theLength = theSettings[ 0 ];
		theSavedCharacter = theSettings[ theLength ];
		
		theInterleaveCount =
			IMAAudioPayloadInterleaveCount( &( **inGlobals ).itsPayloadAttributes );
		
		if( theInterleaveCount > 1 )
		{
			NumToString( theInterleaveCount, &theSettings[ theLength ] );
		}
		
		else
		{
			GetComponentIndString(
				REINTERPRET_CAST( Component )( ( **inGlobals ).itself ),
				&theSettings[ theLength ], kRTPMPIMAAudioStringListResource,
				kRTPMPIMAAudioNoInterleavingString );
		}
		
		theSettings[ 0 ] += theSettings[ theLength ];
		theSettings[ theLength ] = theSavedCharacter;
		theLength = theSettings[ 0 ];
		
		*text = NewHandle( theLength );
		
		if( *text )
			BlockMoveData( &theSettings[ 1 ], **text, theLength );
		else
			theResult = MemError();
	}
	
	else
	{
		*text = 0;
	}
	
	return( theResult );
}
