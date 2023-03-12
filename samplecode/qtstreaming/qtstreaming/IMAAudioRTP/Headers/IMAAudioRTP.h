/*
	File:		IMAAudioRTP.h

	Contains:	Declarations for IMA Audio RTP components

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
#	include "IMAAudioPayload.h"
#endif /* REZ */



/* ---------------------------------------------------------------------------
 *		C O N S T A N T S
 * ---------------------------------------------------------------------------
 */

#ifdef REZ
enum
{
	kIMAAudioDataFormat			= 'ima4',	/* kIMACompression */
	kComponentManufactureType	= 'SMPL'
};
#else
enum
{
	kIMAAudioDataFormat			= FOUR_CHAR_CODE( 'ima4' ),	/* kIMACompression */
	kComponentManufactureType	= FOUR_CHAR_CODE( 'SMPL' )
};
#endif /* REZ */



#endif /* __COMPONENTVIDEORTP__ */
