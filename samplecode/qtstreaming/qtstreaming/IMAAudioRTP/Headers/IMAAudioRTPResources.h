/*
	File:		IMAAudioRTPResources.h

	Contains:	Declarations for IMA Audio RTP component resources

	Copyright:	� 1997-1999 by Apple Computer Inc. all rights reserved.

*/



#ifndef __RTPRSSMCOMPONENTVIDEORESOURCES__
#define __RTPRSSMCOMPONENTVIDEORESOURCES__



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#ifdef REZ
#	define thng_RezTemplateVersion	2	/* use extended 'thng' resource with resource map */
#	include "Components.r"
#	include "RTPDefines.h"
#else
#	include <Components.h>
#	include <QTStreamingComponents.h>
#endif /* REZ */



#include "IMAAudioRTP.h"



/* ---------------------------------------------------------------------------
 *		M A C R O S
 * ---------------------------------------------------------------------------
 */

#define IMA_AUDIO_PROTOCOL_ENCODING_STRING	"X-Sample-IMA-ADPCM-4-1-v0"
#define IMA_AUDIO_HI_ENCODING_STRING		"Sample IMA 4:1 Audio"



#endif /* __RTPRSSMCOMPONENTVIDEORESOURCES__ */
