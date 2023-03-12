/*
	File:		EIComponentSampleMachOUniversal.r
	
	Description: All resources for the program

	Author:		DTS
    
	Copyright: 	� Copyright 2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <1> 07/02/2005 first file

*/

/*
	(OBSOLETE) TARGET_REZ_MAC_68K - set to 1 if you're building 68k, platform68k architecture
	TARGET_REZ_MAC_PPC - this define can be used in two ways:
                            (a) set to 1 if you're building Carbon Mach-O for PPC. TARGET_REZ_CARBON_MACHO must also be set to 1,
                                platformPowerPCNativeEntryPoint architecture
                            (b) (OBSOLETE) set to 1 if you're building non-carbon resource-fork based code fragment, platformPowerPC architecture
    TARGET_REZ_MAC_X86 - set to 1 if you're building Carbon Mach-O for Intel, TARGET_REZ_CARBON_MACHO must also be set to 1,
    						platformIA32NativeEntryPoint architecture
	(OBSOLETE) TARGET_REZ_CARBON_CFM - set to 1 if you're building Carbon PPC data-fork based code fragment,
                                       platformPowerPCNativeEntryPoint architecture
	TARGET_REZ_CARBON_MACHO - set to 1 when building Carbon Mach-O components for Mac OS X PPC or Intel, used with TARGET_REZ_MAC_X86 and TARGET_REZ_MAC_PPC
    TARGET_REZ_WIN32 - set to 1 when building for Windows, platformWin32 architecture
*/

// Mac OS X Mach-O Component
// In the target settings of your Xcode project, add one or both of the following defines
// to your OTHER_REZFLAGS depending on the type of Component you want to build:
// PPC only: -d ppc_$(ppc)
// Intel only: -d i386_$(i386)
// Universal Component: -d ppc_$(ppc) -d i386_$(i386)
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

#include "EI_GraphicsImport.r"
#include "EI_GraphicsExport.r"
#include "EI_MovieImport.r"
#include "EI_MovieExport.r"
#include "EI_Codec.r"
