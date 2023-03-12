/*
	File:		SlideShowImport.r
	
	Description: SlideShowImporter component resources

	Author:		Developer Technical Support

	Copyright: 	� Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):

		<3>		02/15/02    SRK		adapted for use with SGPanel sample
		<2>	 	11/17/00	ERA		updating for PPC and X	
		<1>	 	11/28/99	JSAM	first file
*/




#define SystemSevenOrLater	1

/*
    thng_RezTemplateVersion:
        0 - original 'thng' template    <-- default
        1 - extended 'thng' template	<-- used for multiplatform things
        2 - extended 'thng' template including resource map id
*/

#define thng_RezTemplateVersion 1

/*
    cfrg_RezTemplateVersion:
        0 - original					<-- default
        1 - extended
*/

#define cfrg_RezTemplateVersion 1


#if TARGET_REZ_CARBON_MACHO
    #include <Carbon/Carbon.r>
    #include <QuickTime/QuickTime.r>
#else
    #include "ConditionalMacros.r"
    #include "MacTypes.r"
    #include "Components.r"
    #include "QuickTimeComponents.r"
    #include "ImageCompression.r"
    #include "CodeFragments.r"
    #include "Dialogs.r"
    #include "Icons.r"
#endif


#include "SlideShowImportVersion.h"
#include "SlideShowImportConstants.h"

#define	kSlideShowImportShortFileTypeNamesRes	513
#define	kSlideShowImportNameRes					512
#define	kSlideShowImportInfoRes					513

#define kSlideShowImportFlags \
	(movieImportSubTypeIsFileExtension | canMovieImportFiles | canMovieImportInPlace | hasMovieImportMIMEList | canMovieImportDataReferences)

#define kTheComponentType 			'eat '
#define kTheComponentSubType		'QTSL'
#define kTheComponentManufacturer	'ACME'


// These flags specify information about the capabilities of the component


// Component Manager Thing 'thng'
resource 'thng' (kSlideShowImportThingRes) {
	kTheComponentType,						// Type
	kTheComponentSubType,					// Subtype
	kTheComponentManufacturer,				// Manufacturer
#if TARGET_REZ_MAC_68K
	kSlideShowImportFlags,					// Component flags
	0,										// Component flags Mask
	kTheComponentType,						// Code Type
	kSlideShowImportThingRes,				// Code ID
#else
	0,										// - use componentHasMultiplePlatforms
	0,
	0,
	0,
#endif
	'strn', 								// Name Type
	kSlideShowImportNameRes,				// Name ID
	'stri',									// Info Type
	kSlideShowImportInfoRes,				// Info ID
	0,										// Icon Type
	0,										// Icon ID
#if TARGET_REZ_MAC_68K || TARGET_REZ_WIN32	// Version
	kSlideShowImportVersion,
#else
	kSlideShowImportVersionPPC,
#endif
	componentHasMultiplePlatforms + 		// Registration flags
	componentDoAutoVersion,
	0,										// Resource ID of Icon Family
	{
#if TARGET_OS_MAC							// COMPONENT PLATFORM INFORMATION ----------------------
		#if TARGET_REZ_CARBON_CFM					// COMPONENT PLATFORM INFORMATION ----------------------
			kSlideShowImportFlags,			// Component Flags 
			'cfrg',								// Special Case: data-fork based code fragment
			128,	 								/* Code ID usage for CFM components:
													0 (kCFragResourceID) - This means the first member in the code fragment;
														Should only be used when building a single component per file. When doing so
														using kCFragResourceID simplifies things because a custom 'cfrg' resource is not required
													n - This value must match the special 'cpnt' qualifier 1 in the custom 'cfrg' resource */
			platformPowerPCNativeEntryPoint,	// Platform Type (response from gestaltComponentPlatform or failing that, gestaltSysArchitecture)
		#elif TARGET_REZ_CARBON_MACHO
			kSlideShowImportFlags,
			'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
			kSlideShowImportThingRes,								// ID of 'dlle' resource
			platformPowerPCNativeEntryPoint,
		#elif TARGET_REZ_MAC_PPC
			kSlideShowImportFlags,
			kTheComponentType,									// Code Type
			kSlideShowImportThingRes,						// Code ID
			platformPowerPC,
		#elif TARGET_REZ_MAC_68K
			kSlideShowImportFlags,
			kTheComponentType,
			kSlideShowImportThingRes,
			platform68k,
		#else
			#error "At least one TARGET_REZ_XXX_XXX platform must be defined."
		#endif
#endif
#if TARGET_OS_WIN32
	kSlideShowImportFlags, 
	'dlle',
	kSlideShowImportThingRes,
	platformWin32,
#endif
	},
};

resource 'strn' (kSlideShowImportNameRes) {
	"SlideShow"
};

resource 'stri' (kSlideShowImportInfoRes) {
	"Imports a SlideShow text file."
};

// Component Name
resource 'STR ' (kSlideShowImportThingRes) {
	"Slide Show Import Component"
};

resource 'STR#' (kSlideShowImportShortFileTypeNamesRes) {
	{
	"QTSL"
	}
};


resource 'thnr' (kSlideShowImportThingRes)
{
	{
	kMimeInfoMimeTypeTag, 1, 0,
	kMimeInfoMimeTypeTag, kSlideShowImportThingRes, 0
	}
};

resource 'mime' (kSlideShowImportThingRes) {
	{
	kMimeInfoMimeTypeTag,      1, "application/x-qt-slideshow";
	kMimeInfoFileExtensionTag, 1, "qtsl";
	kMimeInfoDescriptionTag,   1, "QT Slide Show";
	};
};

#if TARGET_REZ_CARBON_CFM
// Custom extended code fragment resource
// CodeWarrior will correctly adjust the offset and length of each
// code fragment when building a MacOS Merge target
resource 'cfrg' (0) {
	{
		extendedEntry {
			kPowerPCCFragArch,					// archType
			kIsCompleteCFrag,					// updateLevel
			kNoVersionNum,						// currentVersion
			kNoVersionNum,						// oldDefVersion
			kDefaultStackSize,					// appStackSize
			kNoAppSubFolder,					// appSubFolderID
			kImportLibraryCFrag,				// usage
			kDataForkCFragLocator,				// where
			kZeroOffset,						// offset
			kCFragGoesToEOF,					// length
			"Slide Show Import Component",	// member name
			
			// Start of extended info.
			
			'cpnt',								// libKind (not kFragComponentMgrComponent == 'comp' as you might expect)
			"\0x00\0x80",						// qualifier 1 - hex 0x0080 (128) matches Code ID in 'thng'
			"",									// qualifier 2
			"",									// qualifier 3
			"Slide Show Import Component",	// intlName, localised
		};
	};
};
#endif

#if	TARGET_REZ_CARBON_MACHO || TARGET_REZ_WIN32
// Code Entry Point for Mach-O and Windows
	resource 'dlle' (kSlideShowImportThingRes) {
		"SlideShowImportComponentDispatch"
	};
#endif

resource 'vers' (128)
	{
	0x01,
	0x00,
	alpha,
	1,
	0,
	"1.0a1",
	"SlideShow Importer 1.0a1"
	};