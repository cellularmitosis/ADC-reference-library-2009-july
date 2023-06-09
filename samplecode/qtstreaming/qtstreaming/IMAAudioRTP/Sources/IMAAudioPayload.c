/*
	File:		IMAAudioPayload.c

	Contains:	Definition of IMAAudioPayload operations

	Copyright:	� 1997-1999 by Apple Computer, Inc., all rights reserved.
	
*/



#include "IMAAudioPayload.h"
#include <FixMath.h>
#include <Endian.h>



enum	/* fixed header fields */
{
	__kSampleRateWord			= 0,
	__kSampleRateSize			= 4,
	__kSampleRatePosition		= ( sizeof( UInt16 ) << 3 ) - __kSampleRateSize,
	
	__kChannelCountWord			= 0,
	__kChannelCountSize			= 3,
	__kChannelCountPosition		= __kSampleRatePosition - __kChannelCountSize,
	
	__kInterleaveCountWord		= 0,
	__kInterleaveCountSize		= 3,
	__kInterleaveCountPosition	= __kChannelCountPosition - __kInterleaveCountSize,
	
	__kInterleaveIndexWord		= 0,
	__kInterleaveIndexSize		= __kInterleaveCountSize,
	__kInterleaveIndexPosition	= __kInterleaveCountPosition - __kInterleaveIndexSize,
	
	__kInterleaveTailWord		= 0,
	__kInterleaveTailSize		= __kInterleaveCountSize,
	__kInterleaveTailPosition	= __kInterleaveIndexPosition - __kInterleaveTailSize
};



static unsigned long	__gSampleRates[] = {
							8000,
							11025,
							16000,
							22050,
							24000,
							32000,
							44100,
							48000 };



enum
{
	__kSampleRateCount = sizeof( __gSampleRates ) / sizeof( *__gSampleRates )
};



static
UInt32
__SetBitField(
	UInt16 *	inStorageUnit,
	UInt32		inPosition,
	UInt32		inSize,
	UInt32		inValue )
{
	UInt32	theMask = ( ( 1L << inSize ) - 1 ) << inPosition;
	
	
	*inStorageUnit =
		EndianU16_NtoB(
			( EndianU16_BtoN( *inStorageUnit ) & ( ~theMask ) ) |
			( theMask & ( inValue << inPosition ) ) );
	
	return( inValue & ( theMask >> inPosition ) );
}



static
UInt32
__BitField(
	const UInt16 *	inStorageUnit,
	UInt32			inPosition,
	UInt32			inSize )
{
	return( ( EndianU16_BtoN( *inStorageUnit ) >> inPosition ) & ( ( 1L << inSize ) - 1 ) );
}



UInt32
IMAAudioPayloadChannelCountLimit(
	void )
{
	return( 1L << __kChannelCountSize );
}



UInt32
IMAAudioPayloadInterleaveCountLimit(
	void )
{
	return( 1L << __kInterleaveCountSize );
}



IMAAudioPayload *
IMAAudioPayloadInitialize(
	IMAAudioPayload *	inPayload )
{
	UInt32	theCount = sizeof( *inPayload ) / sizeof( *inPayload->itsFixedHeader );
	
	
	while( theCount )
	{
		--theCount;
		inPayload->itsFixedHeader[ theCount ] = 0;
	}
	
	IMAAudioPayloadSetChannelCount( inPayload, 1 );
	IMAAudioPayloadSetSampleRate( inPayload, 0 );
	IMAAudioPayloadSetInterleaving( inPayload, 1, 0 );
	
	return( inPayload );
}



UInt32
IMAAudioPayloadSetChannelCount(
	IMAAudioPayload *	inPayload,
	UInt32				inChannelCount )
{
	UInt32	theResult;
	
	
	theResult =
		__SetBitField(
			&inPayload->itsFixedHeader[ __kChannelCountWord ], __kChannelCountPosition,
			__kChannelCountSize, inChannelCount - 1 ) + 1;
	
	return( theResult );
}



UnsignedFixed
IMAAudioPayloadSetSampleRate(
	IMAAudioPayload *	inPayload,
	UnsignedFixed		inSampleRate )
{
	UnsignedFixed	theResult;
	UInt32			theRateCode = __kSampleRateCount;
	
	
	do
	{
		--theRateCode;
	}
	while( theRateCode  &&  inSampleRate < Long2Fix( __gSampleRates[ theRateCode ] ) );
	
	theRateCode =
		__SetBitField(
			&inPayload->itsFixedHeader[ __kSampleRateWord ], __kSampleRatePosition,
			__kSampleRateSize, theRateCode );
	
	if( theRateCode < __kSampleRateCount )
		theResult = Long2Fix( __gSampleRates[ theRateCode ] );
	else
		theResult = 0;
	
	return( theResult );
}



UInt32
IMAAudioPayloadSetInterleaving(
	IMAAudioPayload *	inPayload,
	UInt32				inInterleaveCount,
	UInt32				inInterleaveGroupFrameCount )
{
	UInt32	theResult;
	
	
	theResult =
		__SetBitField(
			&inPayload->itsFixedHeader[ __kInterleaveCountWord ],
			__kInterleaveCountPosition, __kInterleaveCountSize,
			inInterleaveCount - 1 ) + 1;
	
	__SetBitField(
		&inPayload->itsFixedHeader[ __kInterleaveIndexWord ], __kInterleaveIndexPosition,
		__kInterleaveIndexSize, 0 );
	
	__SetBitField(
		&inPayload->itsFixedHeader[ __kInterleaveTailWord ], __kInterleaveTailPosition,
		__kInterleaveTailSize, ( inInterleaveGroupFrameCount - 1 ) % theResult );
	
	return( theResult );
}



UInt32
IMAAudioPayloadIncrementInterleaveIndex(
	IMAAudioPayload *	inPayload )
{
	UInt32	theResult;
	
	
	theResult =
		__BitField(
			&inPayload->itsFixedHeader[ __kInterleaveIndexWord ],
			__kInterleaveIndexPosition, __kInterleaveIndexSize );
	
	theResult =
		__SetBitField(
			&inPayload->itsFixedHeader[ __kInterleaveIndexWord ],
			__kInterleaveIndexPosition, __kInterleaveIndexSize,
			( theResult + 1 ) % IMAAudioPayloadInterleaveCount( inPayload ) );
	
	return( theResult );
}



UInt32
IMAAudioPayloadChannelCount(
	const IMAAudioPayload *		inPayload )
{
	return(
		__BitField(
			&inPayload->itsFixedHeader[ __kChannelCountWord ],
			__kChannelCountPosition, __kChannelCountSize ) + 1 );
}



UnsignedFixed
IMAAudioPayloadSampleRate(
	const IMAAudioPayload *		inPayload )
{
	UnsignedFixed	theResult;
	UInt32			theRateCode;
	
	
	theRateCode =
		__BitField(
			&inPayload->itsFixedHeader[ __kSampleRateWord ],
			__kSampleRatePosition, __kSampleRateSize );
	
	if( theRateCode < __kSampleRateCount )
		theResult = Long2Fix( __gSampleRates[ theRateCode ] );
	else
		theResult = 0;
	
	return( theResult );
}



UInt32
IMAAudioPayloadInterleaveCount(
	const IMAAudioPayload *		inPayload )
{
	return(
		__BitField(
			&inPayload->itsFixedHeader[ __kInterleaveCountWord ],
			__kInterleaveCountPosition, __kInterleaveCountSize ) + 1 );
}



UInt32
IMAAudioPayloadInterleaveIndex(
	const IMAAudioPayload *		inPayload )
{
	return(
		__BitField(
			&inPayload->itsFixedHeader[ __kInterleaveIndexWord ],
			__kInterleaveIndexPosition, __kInterleaveIndexSize ) );
}



UInt32
IMAAudioPayloadInterleaveTail(
	const IMAAudioPayload *		inPayload )
{
	return(
		__BitField(
			&inPayload->itsFixedHeader[ __kInterleaveTailWord ],
			__kInterleaveTailPosition, __kInterleaveTailSize ) );
}
