/*
	File:		RTPMPIMAAudioResources.h

	Contains:	Declarations for IMA Audio RTPMediaPacketizer resources

	Copyright:	© 1997-1999 by Apple Computer Inc. all rights reserved.

*/



#ifndef __RTPMPCOMPONENTVIDEORESOURCES__
#define __RTPMPCOMPONENTVIDEORESOURCES__



#include "IMAAudioRTPResources.h"



/* ---------------------------------------------------------------------------
 *		M A C R O S
 * ---------------------------------------------------------------------------
 */

/*	change these for your component */
/*	type and ID have to match what's in the code warrior project! */

#define COMPONENT_NAME_STRING					"Sample IMA Audio Media Packetizer"
#define COMPONENT_INFO_STRING					"Sample IMA Audio Media Packetizer"

#define COMPONENT_ENTRY_POINT_STRING			"RTPMPIMAAudio_ComponentDispatch"
#define COMPONENT_PPC_PEF_STRING				"RTPMPIMAAudio.pef"
#define COMPONENT_68K_CODE_STRING				"RTPMPIMAAudio.rsrc"



enum
{
	kComponentType					= kRTPMediaPacketizerType,
	kComponentSubType				= kIMAAudioDataFormat
};

enum
{
	kComponentBaseID				= 128,
	kComponentBaseIDPPC				= kComponentBaseID
};

#define kComponentBaseID68K			( kComponentBaseID + 1 )

enum
{
	kComponentVersion				= 0x00010001,
	kComponentFlags					= 0
};

#define kComponentRegFlags			( componentDoAutoVersion | componentHasMultiplePlatforms )

enum
{
	kRTPMPIMAAudioStringListResource			= kComponentBaseID,
		kRTPMPIMAAudioProtocolEncodingString	= 1,
		kRTPMPIMAAudioHIEncodingString			= 2,
		kRTPMPIMAAudioSettingsString			= 3,
		kRTPMPIMAAudioNoInterleavingString		= 4,
	
	kRTPMPIMAAudioSettingsDialogResource		= kComponentBaseID
};



#endif /* __RTPMPCOMPONENTVIDEORESOURCES__ */
