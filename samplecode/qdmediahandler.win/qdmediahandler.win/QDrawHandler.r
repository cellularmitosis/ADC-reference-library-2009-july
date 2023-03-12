//////////
//
//	File:		QDrawHandler.r
//
//	Contains:	Resources for creating a derived media handler component for QuickDraw pictures.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1993-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	09/15/00	rtm		minor changes in platform stuff
//	   <1>	 	01/13/99	rtm		first file
//
//
//////////

//////////
//
// defines needed before including headers
//
//////////

// use the platform info array in the 'thng' resource for 68K and PowerPC, or Windows
#define thng_RezTemplateVersion			1


//////////
//
// header files
//
//////////

#include "MacTypes.r"
#include "Components.r"
#include "Icons.r"

#include "QDMediaCommon.h"


//////////
//
// component resource for QuickDraw media handler
//
//////////

resource 'thng' (kQDMH_ComponentResID, kQDMH_Name, purgeable) {
	'mhlr',											// component type
	kQDMH_MediaType,								// component subtype
	kQDMH_ComponentManufacturer,					// component manufacturer
	0x00000000,										// component flags
	0x00000000,										// component flags mask
	0,												// no 68K code
	0,												// no 68K code
	'STR ',											// component name resource type
	kQDMH_NameStringResID,							// component name resource ID
	'STR ',											// component info resource type
	kQDMH_InfoStringResID,							// component info resource ID
	'ICON',											// component icon resource type
	kQDMH_IconResID,								// component icon resource ID
	kQDMH_Version,
	componentDoAutoVersion | componentHasMultiplePlatforms, 0,
	{
#if TARGET_OS_MAC
		0, 'mhlr', kQDMH_ComponentResID, platformPowerPC
#else
		0, 'dlle', kQDMH_ComponentResID, platformWin32
#endif
	}
};


//////////
//
// name and info string resources
//
//////////

resource 'STR ' (kQDMH_NameStringResID, kQDMH_Name, purgeable) {
	"QuickDraw Media Handler Component"
};

resource 'STR ' (kQDMH_InfoStringResID, kQDMH_Name, purgeable) {
	"A derived media handler for QuickDraw pictures"
};


//////////
//
// icon
//
//////////

resource 'ICON' (kQDMH_IconResID, kQDMH_Name, purgeable) {
	$"00000000000000000000000000000000"
	$"00000000003FC00001C0380002040400"
	$"022A840003915F80027FE44002000720"
	$"020004A0020004A0020004A0020004A0"
	$"020004A0020007200200044002000780"
	$"0200040001C03800003FC00000001000"
	$"00888000002020000085400000100000"
	$"0042"
};


#if !TARGET_OS_MAC
resource 'dlle' (kQDMH_ComponentResID, kQDMH_Name) {
	"QDMH_ComponentDispatch"
};
#endif




