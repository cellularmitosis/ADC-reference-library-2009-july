/*
	File:		DelegateOnly_Codec.r
	
	Description: Decompressor component resources

	Author:		dts

	Copyright: 	© Copyright 2003-2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): 10/10/05 Updated for Universal Binaries
                                        2003 initial release
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

#define	kCodec_CodecFormatType	'DelO'			// delegate only
#define	kCodec_CodecFormatName	"2vuy"  

// These flags specify information about the capabilities of the component
#define kCodec_DecoFlags codecInfoDepth24

// These flags specify the possible format of compressed data produced by the component
// and the format of compressed files that the component can handle during decompression
#define kCodec_FormatFlags codecInfoDepth24

// Component Description
resource 'cdci' (256) {
	kCodec_CodecFormatName,	// Type
	1,						// Version
	1,						// Revision level
	'DTS ',					// Manufacturer
	kCodec_DecoFlags,		// Decompression Flags
	0,						// Compression Flags
	kCodec_FormatFlags,		// Format Flags
	128,					// Compression Accuracy
	128,					// Decomression Accuracy
	200,					// Compression Speed
	200,					// Decompression Speed
	128,					// Compression Level
	0,						// Reserved
	1,						// Minimum Height
	1,						// Minimum Width
	0,						// Decompression Pipeline Latency
	0,						// Compression Pipeline Latency
	0						// Private Data
};

resource 'thng' (256) {
	decompressorComponentType,				// Type			
	kCodec_CodecFormatType,					// SubType
	'DTS ',									// Manufacturer
	kCodec_DecoFlags,						// Component flags
	0,										// Component flags Mask
	0,										// - No 68k use componentHasMultiplePlatforms
	0,										
	'STR ',									// Name Type
	256,									// Name ID
	'STR ',									// Info Type
	257,									// Info ID
	0,										// Icon Type
	0,										// Icon ID
	0,										// Version
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
            kCodec_DecoFlags, 
            'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
            256,								// ID of 'dlle' resource
            platformPowerPCNativeEntryPoint,    // PowerPC-based Macintosh
        #endif
        #if TARGET_REZ_MAC_X86
            kCodec_DecoFlags, 
            'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
            256,								// ID of 'dlle' resource
            platformIA32NativeEntryPoint,       // Intel-based Macintosh
        #endif
    #else
        #error "TARGET_REZ_CARBON_MACHO should be defined."
    #endif
#elif TARGET_OS_WIN32
    #if TARGET_REZ_WIN32
        kCodec_DecoFlags, 
        'dlle',
        256,
        platformWin32,
    #else
        #error "TARGET_REZ_WIN32 should be defined."
    #endif
#else
    #error "I have no idea what you're trying to do!"
#endif
	};
};

// Component Name
resource 'STR ' (256) {
	"Delegate Only Codec"
};

// Component Information
resource 'STR ' (257) {
	"Delgates everything to the 2vuy Codec"
};

#if	TARGET_REZ_CARBON_MACHO || TARGET_REZ_WIN32
// Code Entry Point for Mach-O and Windows
	resource 'dlle' (256) {
		"DelegateOnly_ImageCodecComponentDispatch"
	};
#endif
