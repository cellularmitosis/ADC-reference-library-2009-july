/*

File: EatClosedCaption.r

Abstract: Resources for ClosedCaptionImporter target 

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#define TARGET_REZ_CARBON_MACHO 1

#if defined(ppc_YES)
	#define TARGET_REZ_MAC_PPC 1
#else
	#define TARGET_REZ_MAC_PPC 0
#endif

#if defined(i386_YES)
	#define TARGET_REZ_MAC_X86 1
#else
	#define TARGET_REZ_MAC_X86 0
#endif

#if !(TARGET_REZ_MAC_PPC || TARGET_REZ_MAC_X86)
	#error "Platform architecture not defined, TARGET_REZ_MAC_PPC and/or TARGET_REZ_MAC_X86 must be defined!"
#endif

// Mac obsolete, set to 0
#define TARGET_REZ_MAC_68K 0
#define TARGET_REZ_CARBON_CFM 0

// Windows
#define TARGET_REZ_WIN32 0


#define thng_RezTemplateVersion 2

#if TARGET_REZ_CARBON_MACHO
    #include <Carbon/Carbon.r>
    #include <QuickTime/QuickTime.r>
#else
	#include "ConditionalMacros.r"
    #include "MacTypes.r"
    #include "Components.r"
    #include "QuickTimeComponents.r"
    #include "CodeFragments.r"
#endif

#include "EatClosedCaptionVersion.h"

// These flags specify information about the capabilities of the component
// Can import from files
// Can create a movie from a file without having to write to a separate disk file
// Can accept a data reference (such as a handle) as the source for the import
#define kEatClosedCaptionFlags ( \
		canMovieImportFiles | canMovieImportDataReferences | \
		canMovieImportValidateFile | canMovieImportValidateDataReferences | \
		canMovieImportInPlace | hasMovieImportMIMEList)

// Component Manager Thing
resource 'thng' (512, "Closed Caption Movie Importer") {
	'eat ',									// Type
	'CLCP',									// SubType
	'clcp',									// Manufacturer - for 'eat ' the media type supported by the component)
	0, 0, 0, 0,								// not used Component flags, Component flags Mask, Code Type, Code ID
	'STR ',									// Name Type
	512,									// Name ID
	0, 0, 0, 0,								// Info Type, Info ID, Icon Type, Icon ID
	kEatClosedCaptionVersion,				// Version
	componentHasMultiplePlatforms |	componentDoAutoVersion,	// Registration Flags
	0,										// Resource ID of Icon Family
	{
#if TARGET_OS_MAC							// COMPONENT PLATFORM INFORMATION ----------------------
	#if TARGET_REZ_CARBON_MACHO
        #if !(TARGET_REZ_MAC_PPC || TARGET_REZ_MAC_X86)
        	#error "Platform architecture not defined, TARGET_REZ_MAC_PPC and/or TARGET_REZ_MAC_X86 must be defined!"
        #endif
        #if TARGET_REZ_MAC_PPC
            kEatClosedCaptionFlags, 
            'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
            512,								// ID of 'dlle' resource
            platformPowerPCNativeEntryPoint,	// PPC
        #endif
        #if TARGET_REZ_MAC_X86
			kEatClosedCaptionFlags,
			'dlle',
            512,
            platformIA32NativeEntryPoint,		// INTEL
        #endif
	#endif
#endif
#if TARGET_OS_WIN32
		kEatClosedCaptionFlags, 
		'dlle',
		512,
		platformWin32,
#endif
	},
	'thnr', 512							// Component public resource identifier
};

// Component Alias
resource 'thga' (513, "Closed Caption Movie Importer") {
	'eat ',								// Type
	'SCC ', 							// Subtype - this must be in uppercase. It will match an ".scc" suffix case-insensitively
	'clcp',								// Manufaturer - for 'eat ' the media type supported by the component
	kEatClosedCaptionFlags | movieImportSubTypeIsFileExtension,	// Component Flags
	0, 0, 0,							// Component Flags Mask, Code Type, Code ID
	'STR ',								// Name Type
	512,								// Name ID
	0, 0, 0, 0,							// Info Type, Info ID, Icon Type, Icon ID
										// TARGET COMPONENT ---------------
	'eat ',								// Type
	'CLCP',								// SubType
	'clcp',								// Manufaturer
	0, 0,								// Component Flags, Component Flags Mask
	'thnr', 512,						// Component public resource identifier
	cmpAliasOnlyThisFile
};

// Import components should include a public component resource holding the same data that
// MovieImportGetMIMETypeList would return. This public resource's type and ID should be 'mime' and 1,
// respectively. By including this public resource, QuickTime and applications don't need to open the
// import component and call MovieImportGetMIMETypeList to determine the MIME types the importer supports.
// In the absence of this resource, QuickTime and applications will use MovieImportGetMIMETypeList  
//
// Component public resource
resource 'thnr' (512, "Closed Caption Movie Importer") {
	{
		'mime', 1, 0, 
		'mime', 512, 0,
		
		'mcfg', 1, 0,
		'mcfg', 512, 0
	} 
};

// QuickTime Media Configuration Resources ('mcfg' aka kQTMediaConfigResourceType) are used by the QuickTime MIME
// Configuration Control Panel to build and configure its User Interface. The 'mcfg' resource is also used by the
// QuickTime Plug-In to build a list of MIME types it registers for, and to figure out how to open files.
// In a future version of QuickTime for Windows the 'mcfg' resource will be used for the File Type Registration Control Panel.
// Some, but not all of the information contained within the 'mcfg' resources is available in other places in a component.
// However not everything is available, and not all in one place for example, Group ID, Plug-In, Application Flags and so on.
//
// Every Movie Importer ('eat ') and Graphics Importer ('grip') component should really have one.
//
// If either or both of the kQTMediaConfigCanUseApp and kQTMediaConfigCanUsePlugin flags are set, the MIME type will
// automatically show up in the MIME Configuration Control Panel allowing a user to choose how they want QuickTime to handle
// the file, if at all.
// 
// If the kQTMediaConfigUsePluginByDefault flag is set, QuickTime will automatically register the MIME type for the
// QuickTime plug-in with all browsers on both platforms.
//
// Added in QuickTime 6
resource 'mcfg' (512, "Closed Caption Movie Importer")
{
	kVersionDoesntMatter,					// Version of the component this applies to
	
	{
		// The ID of the group this type belongs with, (OSType, one of kQTMediaConfigStreamGroupID, etc.)
		// This flag determines which group this MIME type will be listed under in the MIME Configuration Control Panel
		kQTMediaConfigMiscGroupID,
		
		// MIME config flags (unsigned long, one or more of kQTMediaConfigCanUseApp, etc.)
		kQTMediaConfigUseAppByDefault |		// By default, associate with application specified below instead of the QuickTime plug-in
			kQTMediaConfigCanDoFileAssociation |
			kQTMediaConfigCanUseApp |		// This type can be associated with an application
			kQTMediaConfigBinaryFile,		// The file is binary, not just text

		'SCC ',								// MacOS file type when saved (OSType)
		'TVOD',								// MacOS file creator when saved (OSType)

		// Component information, used by the QuickTime plug-in to find the component to open this type of file
		'eat ',								// Component type (OSType)
		'CLCP',								// Component subtype (OSType)
		'clcp',								// Component manufacturer (OSType)
		kEatClosedCaptionFlags,				// Component flags
		0, 									// Flags mask

		'SCC ',								// Default file extension (OSType) - this must be in uppercase. It will match an ".scc" suffix case-insensitively.
		kQTMediaInfoMiscGroup,				// QT file type group (OSType, one of kQTMediaInfoNetGroup, etc.)

		// Media type synonyms, an array of zero or more Pascal strings - none here
		{
		},

		{
			"Scenarist Closed Caption File",			// Media type description for MIME configuration panel and browser
			"scc",										// File extension(s), comma delimited if more than one
			"QuickTime Player",							// Opening application name for MIME configuration panel and browser
			"Scenarist Closed Caption Movie Importer",	// Missing software description for the missing software dialog
			"Version 1.0",								// Vendor info string (copyright, version, etc)
		},
		
		// Array of one or more MIME types that describe this media type (eg. audio/mpeg, audio/x-mpeg, etc.)
		{
		},
	}
};

// Component Name
resource 'STR ' (512, "Closed Caption Movie Importer") {
	"Scenarist Closed Caption Movie Importer"
};

/* 
	  This is an example of how to build an atom container resource to hold mime types.
	  This component's GetMIMETypeList implementation simply loads this resource and returns it.
	  Please note that atoms of the same type MUST be grouped together within an atom container.
	  (Also note that "video/electric-image" may not have been registered with the IETF.)
*/
resource 'mime' (512, "Closed Caption Movie Importer") {
	{
		kMimeInfoFileExtensionTag, 1, "scc";
		kMimeInfoDescriptionTag,   1, "Scenarist Closed Caption";
	};
};

#if	TARGET_REZ_CARBON_MACHO || TARGET_OS_WIN32
// Code Entry Point for Mach-O and Windows
	resource 'dlle' (512, "Closed Caption Movie Importer") {
		"EatClosedCaptionComponentDispatch"
	};
#endif
