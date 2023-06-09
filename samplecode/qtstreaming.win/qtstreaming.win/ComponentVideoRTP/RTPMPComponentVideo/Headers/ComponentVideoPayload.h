/*
	File:		ComponentVideoPayload.h

	Contains:	Declaration of ComponentVideoPayload datatype and its operations

	Copyright:	� 1997-1999 by Apple Computer Inc. all rights reserved.

	
	
	X-Sample-YUV-422-v0 Payload Format
	-----------------------------------------------------------------
	
	                     1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	|D|                      Description Seed                       |
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	|                        Offset                         |  MBZ  |
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	:                      Payload Description                      :
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	:                          Image Data                           :
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	
	
	D - 1 bit
	
		Set if, and only if, Payload Description is present.
	
	
	Description Seed - 31 bits
	
		A payload description seed that increments cyclically
		whenever the payload description changes.
		
		
	Offset - 28 bits
	
		Positive offset from the start of the current frame to the
		start of image data contained in this payload as counted in
		32-bit words. Multiply by 4 to compute the offset in octets.
	
	
	MBZ - 4 bits
	
		Reserved.  Must be zero.
	
	
	Payload Description - described below
		
		Present if, and only if, the D bit is set.
	
	
	Image Data - 0 or more 32-bit words
		
		YUV 4:2:2 encoded image data.  Might be omitted if Payload
		Description is present.
	
	
	
	Payload Description Format
	-----------------------------------------------------------------
	
	                     1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	| Z |            Width            |            Height           |
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	
	
	Z - 2 bits
	
		Reserved.  Must be zero.
	
	
	Width - 15 bits
	
		Positive integer specifying width of current frame in pixels.
	
	
	Height - 15 bits
		
		Positive integer specifying height of current frame in pixels.

*/



#ifndef __COMPONENTVIDEOPAYLOAD__
#define __COMPONENTVIDEOPAYLOAD__



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#include <MacTypes.h>



/* ---------------------------------------------------------------------------
 *		C O N S T A N T S
 * ---------------------------------------------------------------------------
 */

enum
{
	kComponentVideoPayloadFixedHeaderWordCount	= 2,
	kComponentVideoPayloadDescriptionWordCount	= 1
};



/* ---------------------------------------------------------------------------
 *		D A T A T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	The ComponentVideoPayload structure represents the payload format defined
 *	above.  Unfortunately, the position of bit fields in C data structures is
 *	implementation-dependent, so the data structure cannot declare individual
 *	fields of the payload format.
 *
 */



typedef struct
{
	UInt32	itsFixedHeader[ kComponentVideoPayloadFixedHeaderWordCount ];
	union
	{
		UInt32	itsDescription[ kComponentVideoPayloadDescriptionWordCount ];
		UInt32	itsImageData[ 1 ];
	};
} ComponentVideoPayload;



/* ---------------------------------------------------------------------------
 *		P R O T O T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	These functions get and set fields of the ComponentVideoPayload.  Some
 *	functions affect multiple fields, and some functions expect or return
 *	values that differ from the representation encoded in the payload:
 *
 *		ComponentVideoPayloadSetOffset()
 *		ComponentVideoPayloadOffset()			These functions expect or
 *												return the offset in octets,
 *												not 32-bit words
 *
 *		ComponentVideoPayloadSetDescription()	This function implicitly sets
 *												the description flag (D) field
 *												if inWidth and inHeight are
 *												both non-zero.  If inWidth or
 *												inHeight is zero, this
 *												function clears the
 *												description flag, but does not
 *												modify the Width or Height
 *												fields in the given
 *												ComponentPayloadDescription.
 *		
 *		ComponentVideoPayloadCopyDescription()	This function copies the
 *												description flag (D) and
 *												Description Seed fields as
 *												well as the Payload
 *												Description field.
 */

UInt32
ComponentVideoPayloadDescriptionSeedLimit(
	void );

ComponentVideoPayload *
ComponentVideoPayloadInitialize(
	ComponentVideoPayload *		inPayload,
	UInt16						inWidth,
	UInt16						inHeight );

UInt32
ComponentVideoPayloadSetOffset(
	ComponentVideoPayload *		inPayload,
	UInt32						inOffset );

UInt32
ComponentVideoPayloadSetDescription(
	ComponentVideoPayload *		inPayload,
	UInt16						inWidth,
	UInt16						inHeight );

UInt32
ComponentVideoPayloadCopyDescription(
	ComponentVideoPayload *			inTargetPayload,
	const ComponentVideoPayload *	inSourcePayload );

Boolean
ComponentVideoPayloadHasDescription(
	const ComponentVideoPayload *	inPayload );

UInt32
ComponentVideoPayloadDescriptionSeed(
	const ComponentVideoPayload *	inPayload );

UInt32
ComponentVideoPayloadOffset(
	const ComponentVideoPayload *	inPayload );

UInt16
ComponentVideoPayloadWidth(
	const ComponentVideoPayload *	inPayload );

UInt16
ComponentVideoPayloadHeight(
	const ComponentVideoPayload *	inPayload );



#endif /* __COMPONENTVIDEOPAYLOAD__ */
