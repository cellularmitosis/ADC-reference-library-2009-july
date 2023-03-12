/*
	File:		SoftVOut.r
	
	Description: Video Output component resources

	Author:		QuickTime Engineering, dts

	Copyright: 	© Copyright 1998 - 2005 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
				
	Change History (most recent first): <2> 08/02/2005 updated for universal binary
                                        <1> 12/20/2001 Initial Release

*/

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
        1 - extended					<-- we use the extended version
*/
#define cfrg_RezTemplateVersion 1

#define videoOutputComponentType 'vout'

#if TARGET_REZ_CARBON_MACHO
    #include <Carbon/Carbon.r>
    #include <QuickTime/QuickTime.r>
#else
    #include "ConditionalMacros.r"
    #include "MacTypes.r"
    #include "Components.r"
    #include "ImageCodec.r"
    #include "CodeFragments.r"
#endif

#if	TARGET_REZ_CARBON_MACHO || TARGET_REZ_WIN32
// Code Entry Point for Mach-O and Windows
	resource 'dlle' (200) {
		"SoftVout_ComponentDispatch"
	};
#endif

resource 'thng' (200, "Software Video Output Compnent") {
	videoOutputComponentType,				// Type			
	'soft',									// SubType
	'appl',									// Manufacturer
	0,										// Component flags
	0,										// Component flags Mask	
	0,										// - no 68k, use componentHasMultiplePlatforms
	0,
	'strn',									// Name Type
	200,									// Name ID
	'stri',									// Info Type
	200,									// Info ID
	0,										// Icon Type
	0,										// Icon ID
	0, 										// Version - 0 causes Component Mgr to use the
											// result of Version message for component version
	componentHasMultiplePlatforms +			// Registration Flags 
	componentDoAutoVersion,
	0,										// Resource ID of Icon Family
	{
#if TARGET_OS_MAC
    #if TARGET_REZ_CARBON_MACHO
        #if !(TARGET_REZ_MAC_PPC || TARGET_REZ_MAC_X86)
            #error "Platform architecture not defined!"
        #endif
        
        #if TARGET_REZ_MAC_PPC
            0, 
            'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
            200,								// ID of 'dlle' resource
            platformPowerPCNativeEntryPoint,    // PPC
        #endif
        
        #if TARGET_REZ_MAC_X86
            0, 
            'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
            200,								// ID of 'dlle' resource
            platformIA32NativeEntryPoint,       // IA32
        #endif
        
    #elif TARGET_REZ_CARBON_CFM
            #error "Building a data-fork based CFM component is no longer recommended or supported"
            0,									// Component Flags 						
            'cfrg',								// Special Case: data-fork based code fragment
            200,	 							/* Code ID usage for CFM components:
                                                    0 (kCFragResourceID) - This means the first member in the code fragment;
                                                        Should only be used when building a single component per file. When doing so
                                                        using kCFragResourceID simplifies things because a custom 'cfrg' resource is not required
                                                    n - This value must match the special 'cpnt' qualifier 1 in the custom 'cfrg' resource */
            platformPowerPCNativeEntryPoint,	// Platform Type (response from gestaltComponentPlatform or failing that, gestaltSysArchitecture)
        #endif
    #elif TARGET_REZ_MAC_68K
        #error "Building a 68k component is no longer recommended or supported"
    #endif
        
#if TARGET_OS_WIN32
	0, 
	'dlle',
	200,
	platformWin32,
#endif
	};
};

// Component Name
resource 'strn' (200)
{
	"SoftVOut"
};

// Component Information
resource 'stri' (200)
{
	"Software simulation of a Video Output Component"
};

#if TARGET_REZ_CARBON_CFM
#error "Building a data-fork based CFM component is no longer recommended or supported"
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
			"SoftVOut",							// member name
			
			// Start of extended info.
			
			'cpnt',								// libKind (not kFragComponentMgrComponent == 'comp' as you might expect)
			"\0x00\0xC8",						// qualifier 1 - hex 0x00C8 (200) matches Code ID in 'thng'
			"",									// qualifier 2
			"",									// qualifier 3
			"SoftVOut",							// intlName, localised
		};
	};
};
#endif