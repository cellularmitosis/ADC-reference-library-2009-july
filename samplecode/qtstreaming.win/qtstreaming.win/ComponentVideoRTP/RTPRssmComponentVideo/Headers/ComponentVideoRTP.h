/*
	File:		ComponentVideoRTP.h

	Contains:	Declarations for Component Video RTP components

	Copyright:	© 1997-1999 by Apple Computer Inc. all rights reserved.

*/



#ifndef __COMPONENTVIDEORTP__
#define __COMPONENTVIDEORTP__



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#ifdef REZ
#	include "ConditionalMacros.r"
#else
#	include <ConditionalMacros.h>
#	include "ComponentVideoPayload.h"
#endif /* REZ */



/* ---------------------------------------------------------------------------
 *		M A C R O S
 * ---------------------------------------------------------------------------
 */

#ifndef REZ
#	if __cplusplus
#		define C_CAST( aType )				( aType )
#		define REINTERPRET_CAST( aType )	reinterpret_cast< aType >
#		define STATIC_CAST( aType )			static_cast< aType >
#		define CONST_CAST( aType )			const_cast< aType >
#	else
#		define __CAST( anExpression )		( anExpression )
#		define C_CAST( aType )				( aType ) __CAST
#		define REINTERPRET_CAST( aType )	C_CAST( aType )
#		define STATIC_CAST( aType )			C_CAST( aType )
#		define CONST_CAST( aType )			C_CAST( aType )
#	endif /* __cplusplus */
#endif /* REZ */



/* ---------------------------------------------------------------------------
 *		C O N S T A N T S
 * ---------------------------------------------------------------------------
 */

#ifdef REZ
enum
{
	kComponentVideoDataFormat	= 'yuv2',	/* kComponentVideoCodecType */
	kComponentManufactureType	= 'SMPL'
};
#else
enum
{
	kComponentVideoDataFormat	= FOUR_CHAR_CODE( 'yuv2' ),	/* kComponentVideoCodecType */
	kComponentManufactureType	= FOUR_CHAR_CODE( 'SMPL' )
};
#endif /* REZ */

enum
{
	kComponentVideoRTPTimeScale = 90000		/* RFC 1890 recommended video clock rate */
};



#endif /* __COMPONENTVIDEORTP__ */
