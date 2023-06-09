/*
	File:		ComponentVideoPayload.c

	Contains:	Definition of ComponentVideoPayload operations

	Copyright:	� 1997-1999 by Apple Computer, Inc., all rights reserved.
	
*/



#include "ComponentVideoPayload.h"
#include <Endian.h>
#include <string.h>



enum	/* fixed header fields */
{
	__kDescriptionFlagWord		= 0,
	__kDescriptionFlagPosition	= 31,
	__kDescriptionFlagSize		= 1,
	
	__kDescriptionSeedWord		= 0,
	__kDescriptionSeedPosition	= 0,
	__kDescriptionSeedSize		= 31,
	
	__kOffsetWord				= 1,
	__kOffsetPosition			= 4,
	__kOffsetSize				= 28
};

enum	/* payload description fields */
{
	__kWidthWord				= 0,
	__kWidthPosition			= 15,
	__kWidthSize				= 15,
	
	__kHeightWord				= 0,
	__kHeightPosition			= 0,
	__kHeightSize				= 15
};



static
UInt32
__SetBitField(
	UInt32 *	inStorageUnit,
	UInt32		inPosition,
	UInt32		inSize,
	UInt32		inValue )
{
	UInt32	theMask = ( ( 1UL << inSize ) - 1 ) << inPosition;
	
	
	*inStorageUnit =
		EndianU32_NtoB(
			( EndianU32_BtoN( *inStorageUnit ) & ( ~theMask ) ) |
			( theMask & ( inValue << inPosition ) ) );
	
	return( inValue & ( theMask >> inPosition ) );
}



static
UInt32
__BitField(
	const UInt32 *	inStorageUnit,
	UInt32			inPosition,
	UInt32			inSize )
{
	return( ( EndianU32_BtoN( *inStorageUnit ) >> inPosition ) & ( ( 1UL << inSize ) - 1 ) );
}



UInt32
ComponentVideoPayloadDescriptionSeedLimit(
	void )
{
	return( ( 1UL << __kDescriptionSeedSize ) - 1 );
}



ComponentVideoPayload *
ComponentVideoPayloadInitialize(
	ComponentVideoPayload *		inPayload,
	UInt16						inWidth,
	UInt16						inHeight )
{
	memset( inPayload, 0, sizeof( *inPayload ) );
	
	ComponentVideoPayloadSetDescription( inPayload, inWidth, inHeight );
	
	return( inPayload );
}



UInt32
ComponentVideoPayloadSetOffset(
	ComponentVideoPayload *		inPayload,
	UInt32						inOffset )
{
	UInt32	theResult;
	
	
	theResult =
		__SetBitField(
			&inPayload->itsFixedHeader[ __kOffsetWord ], __kOffsetPosition, __kOffsetSize,
			inOffset >> 2 ) << 2;
	
	return( theResult );
}



UInt32
ComponentVideoPayloadSetDescription(
	ComponentVideoPayload *		inPayload,
	UInt16						inWidth,
	UInt16						inHeight )
{
	UInt32	theResult;
	
	
	theResult = ComponentVideoPayloadDescriptionSeed( inPayload );
	
	if( inWidth  &&  inHeight )
	{
		if(
			inWidth != ComponentVideoPayloadWidth( inPayload )  ||
			inHeight != ComponentVideoPayloadHeight( inPayload ) )
		{
			theResult =
				__SetBitField(
					&inPayload->itsFixedHeader[ __kDescriptionSeedWord ],
					__kDescriptionSeedPosition, __kDescriptionSeedSize, theResult + 1 );
			
			__SetBitField(
				&inPayload->itsDescription[ __kWidthWord ], __kWidthPosition, __kWidthSize,
				inWidth );
			
			__SetBitField(
				&inPayload->itsDescription[ __kHeightWord ], __kHeightPosition,
				__kHeightSize, inHeight );
		}
	}
	
	__SetBitField(
		&inPayload->itsFixedHeader[ __kDescriptionFlagWord ], __kDescriptionFlagPosition,
		__kDescriptionFlagSize, inWidth  &&  inHeight );
	
	return( theResult );
}



UInt32
ComponentVideoPayloadCopyDescription(
	ComponentVideoPayload *			inTargetPayload,
	const ComponentVideoPayload *	inSourcePayload )
{
	UInt32	theResult;
	
	
	__SetBitField(
		&inTargetPayload->itsFixedHeader[ __kDescriptionFlagWord ],
		__kDescriptionFlagPosition, __kDescriptionFlagSize,
		ComponentVideoPayloadHasDescription( inSourcePayload ) );
	
	theResult =
		__SetBitField(
			&inTargetPayload->itsFixedHeader[ __kDescriptionSeedWord ],
			__kDescriptionSeedPosition, __kDescriptionSeedSize,
			ComponentVideoPayloadDescriptionSeed( inSourcePayload ) );
	
	__SetBitField(
		&inTargetPayload->itsDescription[ __kWidthWord ], __kWidthPosition, __kWidthSize,
		ComponentVideoPayloadWidth( inSourcePayload ) );
	
	__SetBitField(
		&inTargetPayload->itsDescription[ __kHeightWord ], __kHeightPosition, __kHeightSize,
		ComponentVideoPayloadHeight( inSourcePayload ) );
	
	return( theResult );
}



Boolean
ComponentVideoPayloadHasDescription(
	const ComponentVideoPayload *	inPayload )
{
	return(
		__BitField(
			&inPayload->itsFixedHeader[ __kDescriptionFlagWord ],
			__kDescriptionFlagPosition, __kDescriptionFlagSize ) );
}



UInt32
ComponentVideoPayloadDescriptionSeed(
	const ComponentVideoPayload *	inPayload )
{
	return(
		__BitField(
			&inPayload->itsFixedHeader[ __kDescriptionSeedWord ],
			__kDescriptionSeedPosition, __kDescriptionSeedSize ) );
}



UInt32
ComponentVideoPayloadOffset(
	const ComponentVideoPayload *	inPayload )
{
	return(
		__BitField(
			&inPayload->itsFixedHeader[ __kOffsetWord ], __kOffsetPosition,
			__kOffsetSize ) << 2 );
}



UInt16
ComponentVideoPayloadWidth(
	const ComponentVideoPayload *	inPayload )
{
	return(
		__BitField(
			&inPayload->itsDescription[ __kWidthWord ], __kWidthPosition, __kWidthSize ) );
}



UInt16
ComponentVideoPayloadHeight(
	const ComponentVideoPayload *	inPayload )
{
	return(
		__BitField(
			&inPayload->itsDescription[ __kHeightWord ], __kHeightPosition,
			__kHeightSize ) );
}
