#define UseExtendedThingResource 1

#include "MacTypes.r"
#include "Components.r"
#include "ImageCodec.r"

#if !TARGET_OS_MAC
	#if	TARGET_OS_WIN32
		#define Target_PlatformType      platformWin32
	#endif

	#if TARGET_OS_UNIX
		#if TARGET_CPU_MIPS
			#define Target_PlatformType      platformIRIXmips
		#endif
		#if TARGET_CPU_SPARC
			#define Target_PlatformType      platformSunOSsparc
		#endif
	#endif

	#ifndef Target_PlatformType
		#error get a real platform type
	#endif
#endif

resource 'thng' (200,  "Software vdig") {
	'vdig', 'soft', 'jph ',
#if TARGET_REZ_MAC_68K
	0,		0,			// Flags, Mask
	'CODE',	200,		// 68k Code
#else
	0,		0,			// Flags, Mask
	0,		0,			// 68k Code
#endif
	'strn',	200,		// Name
	'stri',	200,		// Info
	'ICON',	200,		// Icon
	0,
#if !TARGET_REZ_MAC_68K
	componentHasMultiplePlatforms |
#endif
		componentDoAutoVersion,
	0,
	{
#if	TARGET_OS_MAC
#else
		0,
		'dlle',	200,	// Code
		Target_PlatformType,
#endif
	},
};

resource 'strn' (200) {
	"softVdig"
};

resource 'stri' (200) {
	"Software simulation of a video digitizer"
};

#if	TARGET_OS_MAC
#else
	resource 'dlle' (200) {
		"softVdig"
	};
#endif
