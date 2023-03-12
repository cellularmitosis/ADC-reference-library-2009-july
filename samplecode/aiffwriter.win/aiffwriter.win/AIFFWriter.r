//////////
//
//	File:		AIFFWriter.r
//
//	Contains:	Resources for creating a fat sound output component 'thng' resource.
//
//	Written by:	Mark Cookson, Apple Developer Technical Support
//				Based on code by Kip Olson.
//	Revised by: Tim Monroe
//
//	Copyright:	� 1993-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	03/05/99	rtm		first inherited file; changed some compiler conditionals
//									to support Windows compiles
//
//
//////////


//////////
//
// defines needed before including headers
//
//////////

// use the platform info array in the 'thng' resource for 68K and PowerPC, or Windows
#define UseExtendedThingResource		1


//////////
//
// header files
//
//////////

#include "MacTypes.r"
#include "Components.r"
#include "Sound.r"
#include "Icons.r"


//////////
//
// constants
//
//////////

#define kAIFFWriterResID			1000					// 'thng', 'STR ', 'ICON' resources
#define kAIFFWriterSubType			'hack'					// OS Type for component

#define kAIFFWriterVersion			0x00010000				// version for this sifter
#define kAIFFWriterManufacturer		'????'					// manufacturer

// resource names will be created for debug versions
#ifdef _DEBUG
#define kAIFFWriterName				"AIFF Writer"
#else
#define kAIFFWriterName				""
#endif

#define kAIFFWriterInfoStringID		1001


//////////
//
// component resource for AIFF writer output component
//
//////////

resource 'thng' (kAIFFWriterResID, kAIFFWriterName, purgeable) {
	kSoundOutputDeviceType,
	kAIFFWriterSubType,
	kAIFFWriterManufacturer,
	cmpWantsRegisterMessage,
	0,
	kSoundComponentType,
	kAIFFWriterResID,				
	'STR ',
	kAIFFWriterResID,							// name
	'STR ',
	kAIFFWriterInfoStringID,					// info
	'ICON',
	kAIFFWriterResID,							// icon
	kAIFFWriterVersion,												
	componentDoAutoVersion | componentHasMultiplePlatforms, 0,
	{
#if TARGET_OS_MAC
		0, kSoundComponentType, kAIFFWriterResID, platform68k;
		0, kSoundComponentPPCType, kAIFFWriterResID, platformPowerPC
#else
		0, 'dlle', kAIFFWriterResID, platformWin32
#endif
	}
};

resource 'ICON' (kAIFFWriterResID, kAIFFWriterName, purgeable) {
	$"0000 0000 0000 0000 0000 0000 0003 8000"
	$"0003 8000 0003 8000 0003 8000 0003 8000"
	$"0003 8000 0003 8000 0003 8000 0003 8000"
	$"000F E000 0007 C000 0003 8000 0001 0000"
	$"0000 0000 0000 0000 7FFF FFFE 8000 0001"
	$"8000 0001 8000 0001 8000 0001 8000 0001"
	$"8000 0001 8800 0001 8000 0001 8000 0001"
	$"7FFF FFFE"
};

resource 'STR ' (kAIFFWriterResID, kAIFFWriterName, purgeable) {
	kAIFFWriterName
};

resource 'STR ' (kAIFFWriterInfoStringID, kAIFFWriterName, purgeable) {
	"AIFF Writer example sound output device. � 1993-1999, Apple Computer."
};

resource 'vers' (1) {
	0x2, 0x0, beta, 0x1,
	0,
	"2.0b1",
	"AIFF Writer v2.0b1, � Apple Computer, Inc. 1993-1998"
};

#if !TARGET_OS_MAC
resource 'dlle' (kAIFFWriterResID, kAIFFWriterName) {
	"_SoundOutputComponentDispatch"
};
#endif

