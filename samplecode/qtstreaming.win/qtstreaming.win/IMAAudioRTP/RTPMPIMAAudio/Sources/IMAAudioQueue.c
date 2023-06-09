/*
	File:		IMAAudioQueue.c

	Contains:	Definition of operations for IMAAudioQueue, a queue datatype
				for IMA audio sample data.

	Copyright:	� 1997-1999 by Apple Computer, Inc., all rights reserved.

*/



#include "IMAAudioQueue.h"
#include "IMAAudioPayload.h"
#include <Math64.h>



static
RTPMPSampleDataParams **
__CopySampleDataParams(
	const RTPMPSampleDataParams *	inSampleDataParams )
{
	RTPMPSampleDataParams **	theResult;
	
	
	theResult =
		REINTERPRET_CAST( RTPMPSampleDataParams ** )(
			NewHandle( sizeof( *inSampleDataParams ) ) );
	
	**theResult = *inSampleDataParams;
	
	return( theResult );
}



static
RTPMPSampleDataParams *
__LockSampleDataParams(
	RTPMPSampleDataParams **	inSampleDataParams )
{
	Handle	theHandle = REINTERPRET_CAST( Handle )( inSampleDataParams );
	
	
	if( HandleZone( theHandle ) == SystemZone() )
		HLock( theHandle );
	else
		HLockHi( theHandle );
	
	return( *inSampleDataParams );
}



static
void
__DisposeSampleDataParams(
	RTPMPSampleDataParams **	inSampleDataParams )
{
	DisposeHandle( REINTERPRET_CAST( Handle )( inSampleDataParams ) );
}



static
UInt64
__ConsumedDuration(
	IMAAudioQueue *		inQueue,
	UInt32				inChannel )
{
	UInt32	theFrameCount;
	UInt64	theSampleCount;
	UInt64	theResult;
	
	
	theFrameCount =
		( inQueue->__itsConsumedFrameCount + inQueue->__itsChannelCount - inChannel - 1 ) /
			inQueue->__itsChannelCount;
	
	theSampleCount =
		U64Multiply(
			U64SetU( theFrameCount ), U64SetU( kIMAAudioPayloadFrameSampleCount ) );
	
	theResult =
		S64Div(
			U64Multiply(
				theSampleCount,
				U64SetU( STATIC_CAST( UInt32 )( kIMAAudioPayloadRTPTimeScale ) << 16 ) ),
			S64SetU( inQueue->__itsIncomingSampleRate ) );
	
	return( theResult );
}



static
UInt64
__DequeuedDuration(
	IMAAudioQueue *		inQueue,
	UInt32				inChannel )
{
	UInt32	theFrameCount;
	UInt64	theSampleCount;
	UInt64	theResult;
	
	
	theFrameCount =
		( inQueue->__itsDequeuedFrameCount + inQueue->__itsChannelCount - inChannel - 1 ) /
			inQueue->__itsChannelCount;
	
	theSampleCount =
		U64Multiply(
			U64SetU( inQueue->__itsDequeuedFrameCount / inQueue->__itsChannelCount ),
			U64SetU( kIMAAudioPayloadFrameSampleCount ) );
	
	theResult =
		S64Div(
			U64Multiply(
				theSampleCount,
				U64SetU( STATIC_CAST( UInt32 )( kIMAAudioPayloadRTPTimeScale ) << 16 ) ),
			S64SetU( inQueue->__itsOutgoingSampleRate ) );
	
	return( theResult );
}



static
void
__GetNextFrame(
	IMAAudioQueue *			inQueue,
	IMAAudioQueueElement *	outElement )
{
	RTPMPSampleDataParams **	theSampleDataParams;
	
	
	theSampleDataParams =
		STATIC_CAST( RTPMPSampleDataParams ** )( QueueHead( &inQueue->__itsQueue ) );
	
	if( inQueue->__itsCurrentOffset >= ( **theSampleDataParams ).dataLength )
	{
		inQueue->__itsCurrentOffset = 0;
		
		QueueDequeue( &inQueue->__itsQueue );
		__DisposeSampleDataParams( theSampleDataParams );
		
		theSampleDataParams =
			STATIC_CAST( RTPMPSampleDataParams ** )( QueueHead( &inQueue->__itsQueue ) );
	}
	
	if( inQueue->__itsCurrentOffset == 0 )
		__LockSampleDataParams( theSampleDataParams );
	
	outElement->itsChannel = inQueue->__itsConsumedFrameCount % inQueue->__itsChannelCount;
	
	outElement->itsTimestamp =
		S64Add(
			inQueue->__itsStartTime,
			__DequeuedDuration( inQueue, outElement->itsChannel ) );
	
	outElement->itsSampleDataParams = *theSampleDataParams;
	outElement->itsOffset = inQueue->__itsCurrentOffset;
	
	inQueue->__itsCurrentOffset += sizeof( IMAAudioFrame );
	--inQueue->__itsFrameCount;
	inQueue->__itsConsumedFrameCount++;
}



static
void
__Reset(
	IMAAudioQueue *		inQueue )
{
	QueueInitialize( &inQueue->__itsQueue );
	inQueue->__itsStartTime = S64SetU( 0 );
	inQueue->__itsFrameCount = 0;
	inQueue->__itsCurrentOffset = 0;
	inQueue->__itsConsumedFrameCount = 0;
	inQueue->__itsDequeuedFrameCount = 0;
}



extern
void
IMAAudioQueueInitialize(
	IMAAudioQueue *		inQueue )
{
	__Reset( inQueue );
	
	IMAAudioQueueSetFlowControl( inQueue, 0, 0, 1 );
}



extern
UnsignedFixed
IMAAudioQueueSetFlowControl(
	IMAAudioQueue *		inQueue,
	UnsignedFixed		inIncomingSampleRate,
	UnsignedFixed		inOutgoingSampleRate,
	UInt16				inChannelCount )
{
	UnsignedFixed	theResult;
	
	
	if( inIncomingSampleRate >= inOutgoingSampleRate )
		theResult = inOutgoingSampleRate;
	else
		theResult = inIncomingSampleRate;
	
	if( inQueue->__itsFrameCount )
	{
		if(
			inIncomingSampleRate != inQueue->__itsIncomingSampleRate  ||
			theResult != inQueue->__itsOutgoingSampleRate )
		{
			IMAAudioQueueFlush( inQueue );
		}
	}
	
	inQueue->__itsIncomingSampleRate = inIncomingSampleRate;
	inQueue->__itsOutgoingSampleRate = theResult;
	inQueue->__itsChannelCount = inChannelCount;
	
	return( theResult );
}



extern
UInt32
IMAAudioQueueCount(
	const IMAAudioQueue *	inQueue )
{
	UInt32	theResult;
	UInt64	theIncomingFrameCount;
	UInt64	theIncomingSampleRate;
	UInt64	theDequeuedFrameCount;
	UInt64	theOutgoingSampleRate;
	UInt64	thePendingTime;
	
	
	if( inQueue->__itsIncomingSampleRate  &&  inQueue->__itsFrameCount )
	{
		theIncomingFrameCount =
			U64SetU( inQueue->__itsConsumedFrameCount + inQueue->__itsFrameCount );
		
		theIncomingSampleRate = U64SetU( inQueue->__itsIncomingSampleRate );
		theDequeuedFrameCount = U64SetU( inQueue->__itsDequeuedFrameCount );
		theOutgoingSampleRate = U64SetU( inQueue->__itsOutgoingSampleRate );
		
		thePendingTime =
			U64Subtract(
				U64Multiply( theOutgoingSampleRate, theIncomingFrameCount ),
				U64Multiply( theIncomingSampleRate, theDequeuedFrameCount ) );
		
		theResult = U32SetU( U64Div( thePendingTime, theIncomingSampleRate ) );
		
		if( U64Mod( thePendingTime, theIncomingSampleRate ) )
			theResult++;
	}
	
	else
	{
		theResult = 0;
	}
	
	return( theResult );
}



extern
RTPMPSampleDataParams *
IMAAudioQueueEnqueue(
	IMAAudioQueue *					inQueue,
	const RTPMPSampleDataParams *	inSampleDataParams )
{
	RTPMPSampleDataParams *		theResult;
	
	
	theResult =
		( RTPMPSampleDataParams * ) QueueEnqueue(
			&inQueue->__itsQueue, __CopySampleDataParams( inSampleDataParams ) );
	
	if( theResult )
	{
		if( inQueue->__itsFrameCount == 0  &&  inQueue->__itsConsumedFrameCount == 0 )
			inQueue->__itsStartTime = inSampleDataParams->timeStamp;
		
		inQueue->__itsFrameCount +=
			inSampleDataParams->dataLength / sizeof( IMAAudioFrame );
	}
	
	return( theResult );
}



extern
Boolean
IMAAudioQueueDequeue(
	IMAAudioQueue *			inQueue,
	IMAAudioQueueElement *	outElement )
{
	Boolean		theResult = false;
	UInt32		theInitialFrameCount = inQueue->__itsFrameCount;
	
	
	while( inQueue->__itsFrameCount  && !theResult )
	{
		__GetNextFrame( inQueue, outElement );
		
		theResult =
			U64Compare(
				__ConsumedDuration( inQueue, outElement->itsChannel ),
				__DequeuedDuration( inQueue, outElement->itsChannel ) ) > 0;
	}
	
	if( theInitialFrameCount > inQueue->__itsFrameCount  && theResult )
		inQueue->__itsDequeuedFrameCount++;
	else
		theResult = false;
	
	return( theResult );
}



extern
void
IMAAudioQueueFlush(
	IMAAudioQueue *		inQueue )
{
	UInt32						theCount;
	RTPMPSampleDataParams **	theSampleDataParams;
	
	
	for( theCount = QueueCount( &inQueue->__itsQueue ); theCount; --theCount )
	{
		theSampleDataParams =
			STATIC_CAST( RTPMPSampleDataParams ** )( QueueDequeue( &inQueue->__itsQueue ) );
		
		__DisposeSampleDataParams( theSampleDataParams );
	}
	
	__Reset( inQueue );
}
