/*
	File:		RTPRssmIMAAudioResources.h

	Contains:	Declarations for IMA Audio RTPReassembler resources

	Copyright:	© 1997-1999 by Apple Computer Inc. all rights reserved.

*/



#ifndef __RTPRSSMCOMPONENTVIDRESOURCES__
#define __RTPRSSMCOMPONENTVIDRESOURCES__



#include "IMAAudioRTPResources.h"



/* ---------------------------------------------------------------------------
 *		M A C R O S
 * ---------------------------------------------------------------------------
 */


/*	change these for your component */
/*	type and ID have to match what's in the code warrior project! */

#define COMPONENT_NAME_STRING					"Sample IMA Audio Reassembler"
#define COMPONENT_INFO_STRING					"Sample IMA Audio Reassembler"

#define COMPONENT_ENTRY_POINT_STRING			"RTPRssmIMAAudio_ComponentDispatch"
#define COMPONENT_PPC_PEF_STRING				"RTPRssmIMAAudio.pef"
#define COMPONENT_68K_CODE_STRING				"RTPRssmIMAAudio.rsrc"



/* ---------------------------------------------------------------------------
 *		C O N S T A N T S
 * ---------------------------------------------------------------------------
 */


enum
{
	kComponentType					= kRTPReassemblerType,
	kComponentSubType				= kIMAAudioDataFormat
};

enum
{
	kComponentBaseID				= 256,
	kComponentBaseIDPPC				= kComponentBaseID
};

#define kComponentBaseID68K			( kComponentBaseID + 1 )

enum
{
	kComponentVersion				= 0x00010001,
	kComponentFlags					= 0
};

#define kComponentRegFlags			( componentDoAutoVersion | componentHasMultiplePlatforms )



#endif /* __RTPRSSMCOMPONENTVIDRESOURCES__ */
