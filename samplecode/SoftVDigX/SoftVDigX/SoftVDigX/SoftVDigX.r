/*
	File:		SoftVDigX.r
	
	Description: QuickTime Compressed Source VDig component resources
	
	Author:		QuickTime Engineering, Developer Techical Support
								
	Copyright: 	© Copyright 1993-2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):
       <2>		07/23/03	dts		added changes to build universal binary
	   <1>		07/23/03	dts		updated and initial release for X
*/

//
// Mac OS X Mach-O Component: Set TARGET_REZ_CARBON_MACHO to 1
//
// In the target settings of your Xcode project, add one or both of the
// following defines to your OTHER_REZFLAGS depending on the type of component
// you want to build:
//
//      PPC only: -d ppc_$(ppc)
//      x86 only: -d i386_$(i386)
//      Universal Binary: -d ppc_$(ppc) -d i386_$(i386)
//
// Windows Component: Set TARGET_REZ_CARBON_MACHO to 0
// ---------------------------------------------------

// Set to 1 == building Mac OS X
#define TARGET_REZ_CARBON_MACHO 1

#if TARGET_REZ_CARBON_MACHO
    #if defined(ppc_YES)
        // PPC architecture
        #define TARGET_REZ_MAC_PPC 1
    #else
        #define TARGET_REZ_MAC_PPC 0
    #endif

    #if defined(i386_YES)
        // x86 architecture
        #define TARGET_REZ_MAC_X86 1
    #else
        #define TARGET_REZ_MAC_X86 0
    #endif

    #define TARGET_REZ_WIN32 0
#else
    // Must be building on Windows
    #define TARGET_REZ_WIN32 1
#endif

// extended 'thng' template
#define thng_RezTemplateVersion 1

#include <Carbon/Carbon.r>
#include <QuickTime/QuickTime.r>

// for more information regarding component 'thng' resources see Technical Note 2012
// http://developer.apple.com/technotes/tn/tn2012.html
resource 'thng' (200, "Software vDig") {
	'vdig',									// Type
	'soft',									// Subtype
	'appl',									// Manufacturer
	0,										// - use componentHasMultiplePlatforms
	0,
	0,
	0,
	'strn', 								// Name Type
	200,									// Name ID
	'stri',									// Info Type
	200,									// Info ID
	0,										// Icon Type
	0,										// Icon ID
	0,										// Version
	componentHasMultiplePlatforms + 		// Registration flags
	componentDoAutoVersion,
	0,										// Resource ID of Icon Family
	{
#if TARGET_OS_MAC    
    #if TARGET_REZ_CARBON_MACHO
        #if !(TARGET_REZ_MAC_PPC || TARGET_REZ_MAC_X86)
            #error "Platform architecture not defined!"
        #endif
        
        #if TARGET_REZ_MAC_PPC
            0,									// Component Flags
            'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
            200,								// ID of 'dlle' resource
            platformPowerPCNativeEntryPoint,
        #endif
        
        #if TARGET_REZ_MAC_X86
            0,									// Component Flags
            'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
            200,								// ID of 'dlle' resource
            platformIA32NativeEntryPoint,
        #endif
    #endif
#endif

#if TARGET_REZ_WIN32
    #error
#endif

	},
};

// strings to be retrived by GetComponentIndString()
resource 'STR#' (200, "Digitizer Strings")
{
	{
		"SoftVDigX Video",				// device name
		"SoftVDigX 2vuy 8-bit 4:2:2"	// image description name
	}
};

resource 'strn' (200)
{
	"SoftVDigX"
};

resource 'stri' (200)
{
	"Groovy simulation of a compressed source video digitizer"
};

// Mach-O entry point
resource 'dlle' (200)	
{
    "SoftVDigXComponentDispatch"
};
