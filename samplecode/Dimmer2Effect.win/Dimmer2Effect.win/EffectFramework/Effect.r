/*
	File:		Effect.r
	
	Description: Effect Resources

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 1997-2002 Apple Computer, Inc. All rights reserved.
	
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
		<1>	 	05/10/02	era		updated for X
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
#elif TARGET_REZ_CARBON_CFM
	#include <Carbon.r>
	#include <QuickTime.r>
#else
    #include "ConditionalMacros.r"
    #include "MacTypes.r"
    #include "Components.r"
    #include "ImageCodec.r"
    #include "CodeFragments.r"
#endif

#include "EffectDefinitions.h"

#define kEffectCodecRezInfo		"Dimmer2 Effect Codec Info"
#define	kEffectCodecFormatName	"Dimmer2 Effect"
#define	kEffectCodecFormatType	'dimt'
#define	kEffectCodecWhatName	OSType { "dimt" }

// These flags specify information about the capabilities of the component
// Works with 1-bit, 2-bit, 4-bit, 8-bit, 16-bit and 32-bit Pixel Maps
// It can spool image data and the codec implements get effects parameter list call
#define kDecoFlags (codecInfoDoes32 + codecInfoDoes16 + codecInfoDoes8 + codecInfoDoes4 + codecInfoDoes2 + codecInfoDoes1 + codecInfoDoesSpool  + codecInfoHasEffectParameterList)

// These flags specify the possible format of compressed data produced by the component
// and the format of compressed files that the component can handle during decompression
// The component can decompress from files at 1-bit, 2-bit, 4-bit, 8-bit, 16-bit and 32-bit,
// 1-bit monochrome, 2-bit grayscale, 4-bit grayscale, 8-bit grayscale and the data can be stored in lossless format 
#define kDataFormatFlags (codecInfoDepth1 + codecInfoDepth2 + codecInfoDepth4 + codecInfoDepth8 + codecInfoDepth16 + codecInfoDepth32 + codecInfoDepth33 + codecInfoDepth34 + codecInfoDepth36 + codecInfoDepth40 + codecInfoDoesLossless)

// This structure defines the capabilities of the codec.
// Component Description
resource 'cdci' (kEffect_cdci_ResID, kEffectCodecRezInfo, locked) {
	kEffectCodecFormatName,	// Type
	kVersionNumber,			// Version
	0,						// Revision level
	'appl',					// Manufacturer
	kDecoFlags,				// Decompression Flags 		- depth and etc. supported directly on decompress
	0,						// Compression Flags 		- depth and etc. supported directly on compress
	kDataFormatFlags,		// Format Flags				- which data formats do we understand
	100,					// Compression Accuracy		- compress accuracy (0-255) (relative to format)
	100,					// Decomression Accuracy 	- decompress accuracy (0-255) (relative to format)
	100,					// Compression Speed		- millisecs to compress 320x240 image on base Mac
	100,					// Decompression Speed		- millisecs to compress 320x240 image on base Mac
	100,					// Compression Level		- compression level (0-255) (relative to format)
	0,						// Reserved
	2,						// Minimum Height
	2,						// Minimum Width
	0,						// Decompression Pipeline Latency
	0,						// Compression Pipeline Latency
	0						// Private Data
};

// ====================================================================================
// The Thing resources
// ====================================================================================
resource 'thng' (kEffect_thng_ResID, "Dimmer2 Effect", locked) {
	decompressorComponentType,				// Type	- always 'imdc' for effects	
	kEffectCodecFormatType,					// SubType - your format
	'appl',									// Manufacturer
	kDecoFlags,								// Component flags
	0,										// Component flags Mask
	0,										// - use componentHasMultiplePlatforms
	0,
	'STR ',									// Name Type
	kEffectNameResID,						// Name ID
	'STR ',									// Info Type
	kEffectDescriptionResID,				// Info ID
	0,										// Icon Type
	0,										// Icon ID
	kDimmerEffectVersion,					// Version
	componentHasMultiplePlatforms +			// Registration Flags 
	componentDoAutoVersion,
	0,										// Resource ID of Icon Family
	{
#if TARGET_OS_MAC
	#if TARGET_REZ_CARBON_CFM
		#define USE_CUSTOM_CFRG_RESOURCE 0	// Single component in file - Custom 'cfrg' not used
		
		kDecoFlags,							// Component Flags 						
		'cfrg',								// Special Case: data-fork based code fragment
		0,	 								/* Code ID usage for CFM components:
												0 (kCFragResourceID) - This means the first member in the code fragment;
													Should only be used when building a single component per file. When doing so
													using kCFragResourceID simplifies things because a custom 'cfrg' resource is not required
												n - This value must match the special 'cpnt' qualifier 1 in the custom 'cfrg' resource */
		platformPowerPCNativeEntryPoint,	// Platform Type (response from gestaltComponentPlatform or failing that, gestaltSysArchitecture)
	#elif TARGET_REZ_CARBON_MACHO
		kDecoFlags, 
		'dlle',								// Code Resource type - Entry point found by symbol name 'dlle' resource
		kEffect_dlle_ResID,					// ID of 'dlle' resource
		platformPowerPCNativeEntryPoint,
	#elif TARGET_REZ_MAC_PPC
		kDecoFlags, 
		'imdc',								// Code Type
		kEffectCodeID,						// Code ID
		platformPowerPC,
	#elif TARGET_REZ_MAC_68K
		#error "TARGET_REZ_MAC_68K not supported."
	#else
		#error "At least one TARGET_REZ_XXX_XXX platform must be defined."
	#endif
#elif TARGET_OS_WIN32
	kDecoFlags, 
	'dlle',
	kEffect_dlle_ResID,
	platformWin32,
#else
	#error "At least one TARGET_OS_XXX platform must be defined."
#endif
	};
};

#if TARGET_REZ_CARBON_CFM
#if USE_CUSTOM_CFRG_RESOURCE
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
			"Dimmer2 Effect",					// member name
			
			// Start of extended info.
			
			'cpnt',								// libKind (not kFragComponentMgrComponent == 'comp' as you might expect)
			"\0x00\0x82",						// qualifier 1 - hex 0x0082 (130) must match Code ID in 'thng'
			"",									// qualifier 2
			"",									// qualifier 3
			"Dimmer2 QuickTime Effect",			// intlName, localised
		};
	};
};
#endif
#endif

#if	TARGET_REZ_CARBON_MACHO || TARGET_OS_WIN32
// Code Entry Point for Mach-O and Windows
resource 'dlle' (kEffect_dlle_ResID) {
	"EffectsFrameComponentDispatch"
};
#endif

// ====================================================================================
// Name and description strings
// ====================================================================================

resource 'STR ' (kEffectNameResID) {
	"Dimmer2 Effect"
};

resource 'STR ' (kEffectDescriptionResID) {
	"Fades down from one source, then fades up on a second source"
};

resource 'vers' (1) {
	0x01, 0x01, release, 0x00,
	verUS,
	"1.0.1",
	"1.0.1 ©1997-2002 Apple Computer Inc."
};

// ====================================================================================
// The parameter descriptions
// You must change these to reflect the paramaters that your effect takes.
// Each effect has a set of standard resources that must be present.
// A set of optional resources is required for each parameter your effect has.
// ====================================================================================

/*
The standard information stored in an 'atms' resource is made up of three required atoms
and seven optional atoms.

	The three required atoms are:

	kParameterTitleName 
	      A string used as the title of the standard parameters dialog box.

	kParameterWhatName 
	      An OSType containing the name of this effect component. 

	kParameterSourceCountName 
	      A long integer containing the maximum number of sources that the effect can take.

	The seven optional atoms are

	kParameterAlternateCodecName 
	      An OSType containing the unique identifier of another effect component that should be
	      used to replace this effect if this effect cannot be used.

	kParameterInfoLongName 
	      A string containing the long version of the name of the effect.

	kParameterInfoCopyright 
	      A string containing a copyright statement for the effect.

	kParameterInfoDescription 
	      A string containing a brief description of what the effect does.

	kParameterInfoWindowTitle 
	      A string containing the title of the window that displays the information contained in the optional atoms.

	kEffectMajorClassType (added in QuickTime 6)
		The Major Class for an effect defines the purpose of an effect to allow applications to perform better
   		filtering. It is not intended that the user see effects grouped by major class. Effects that fail to
   		include a kEffectMajorClassType will be classified as Miscellaneous.
   		
   	kEffectMinorClassType (added in QuickTime 6)
   		Like the Major Class, the Minor Class of an effect serves to group the effect into a more refined
   		definition. Unlike the Major Class, however, the Minor Class is intended to be used for grouping for
   		the purposes of User Interface presentation. Effects that fail to include a kEffectMinorClassType
   		will be classified as Miscellaneous.

The Parameter Information in an 'atms' Resource

For each parameter of the effect, your 'atms' resource must contain a set of atoms in the 'atms'
resource that describes that parameter. This description includes the name of the parameter, the
type and range of values it can take, and hints on appropriate user interface element for setting
this parameter.

A complete description of the information you need to provide for each parameter can be found in
"The Parameter Description Format".
http://developer.apple.com/techpubs/quicktime/qtdevdocs/REF/refEffects.35.htm

	For a basic parameter, there are five atoms that you should supply:

	kParameterAtomTypeAndID 
	      Contains the type and ID of the parameter (an OSType and a long integer, respectively),
	      the atom flags for the parameter, and a string containing the name of the parameter.

	kParameterDataType 
	      A long integer containing the type of the parameter, such as kParameterTypeDataFixed.

	kParameterDataRange 
	      A set of typed values describing the range of values the parameter can take. The number
	      and type of values you supply depend on the value of the kParameterDataType atom.

	kParameterDataBehavior 
	      two long integer values containing the behavior type, such as kParameterItemControl for a
	      slider, and any flags, such as kAtomNotInterpolated for a parameter that cannot be tweened.

	kParameterDataDefaultItem 
	      the default value of the parameter. Again, the type of this value will depend on the type of
	      the kParameterDataType atom. This atom must be a correctly-formatted parameter atom that can
	      be passed back to your component by client software without modification.
*/

#define kCrossFadeTransitionType OSType { "dslv" }

resource 'atms' (kEffect_atms_ResID)
{
	// number of root atoms
	10 + 6, // 10 Standard Atoms + 6 Parameter Atoms
{
	// ---------------------------------------
	// The Standard Resources
	// These must be present - they describe your effect
	// ---------------------------------------
	
	// The name of the parameters - Required
	kParameterTitleName, kParameterTitleID, noChildren,
	{
		string { "Dimmer2 Effect Parameters" };
	};
	
	// The name (subType) of the effect component - Required
	kParameterWhatName, kParameterWhatID, noChildren,
	{
		kEffectCodecWhatName;
	};

	// The maximum number of sources the effect takes - Required
	kParameterSourceCountName, kParameterSourceCountID, noChildren,
	{
		long { "2" };
	};
	
	// Alternate Codec - Optional
	// If this codec can't be found, try this other one as a replacement
	kParameterAlternateCodecName, kParameterAlternateCodecID, noChildren,
	{
		kCrossFadeTransitionType;
	};
	
	// Information atoms - Optional
	kParameterInfoLongName, kParameterInfoIDs, noChildren,
	{
		string { "Dimmer2 Effect" };
	};
	
	kParameterInfoCopyright, kParameterInfoIDs, noChildren,
	{
		string { "Sample QuickTime Effect by Apple Computer, Inc." };
	};
	
	kParameterInfoDescription, kParameterInfoIDs, noChildren,
	{
		string { "Fades down from one source, then fades up on a second source." };
	};
	
	kParameterInfoWindowTitle, kParameterInfoIDs, noChildren,
	{
		string { "Dimmer2 Effect Information…" };
	};

// QuickTime 6 Effect Classes
// With an ever increasing number of effect components, it has become difficult for applications and
// users to navigate through the effect list. The effect class atoms can be used for tagging
// effects into useful catagories. These atoms do nothing in previous version of QuickTime.
#if (UNIVERSAL_INTERFACES_VERSION <= 0x0341) || (UNIVERSAL_INTERFACES_VERSION == 0x0400)
	// Older interfaces so define this stuff
	#define kEffectMajorClassType 'clsa'
	#define kEffectMajorClassID    (1)
	#define kTransitionMajorClass  OSType { "tran" }  // multisource morph effects
	#define kEffectMinorClassType  'clsi'
	#define kEffectMinorClassID    (1)
	#define kTransitionMinorClass  OSType { "tran" }  // "Transitions"
#endif

	// Major Class of effect - Optional but a good thing to have for QT 6+
	kEffectMajorClassType, kEffectMajorClassID, noChildren,
	{
		kTransitionMajorClass;
	};
	
	// Minor Class of effect - Option but a good thing to have for QT 6+
	kEffectMinorClassType, kEffectMinorClassID, noChildren,
	{
		kTransitionMinorClass;
	};
	
	// ---------------------------------------
	// Optional Resources
	// There is a standard set of resources
	// for each parameter your effect takes.
	// See the QuickTime Documentation for
	// descriptions of the resources you must
	// supply for your effect
	// ---------------------------------------

	// The percentage parameter defines the starting and end
	// point of the effect. For example, instead of fading down
	// from full intensity to black, then up again, you could
	// fade down from a medium intensity, then back up to full
	// intensity by specifying a starting percentage of 25% and
	// an ending percentage of 100%
	
	// The general information about the parameter
	kParameterAtomTypeAndID, 1, noChildren,
	{
		OSType { "pcnt" };				// The parameter code
		long { "1" };					// The parameter ID
		kAtomNoFlags;					// Parameter flags
		string { "Percentage" };		// The parameter name
	};
	
	// The data type of the parameter
	kParameterDataType, 1, noChildren,
	{
		kParameterTypeDataFixed;
	};
	
	// The range of values the parameter can take			
	kParameterDataRange, 1, noChildren,
	{
		long { "0" };			// Minimum value (0.0 fixed)
		long { "65536" };		// Maximum value (1.0 fixed)
		long { "6553600" };		// Scale. This value will scale the value set
								// by the user into the range 0…100 (100.0 fixed)
		long { "0" };			// Precision
	};

	// The parameter's "behavior"
	kParameterDataBehavior, 1, noChildren,
	{
		kParameterItemControl;	// Display this item using a control. In the case
								// of a parameter of data type Fixed, this will result
								// (in the standard parameters dialog box) in a slider
								// being displayed which the user can use to set the
								// value of the parameter.
		long { "0" };			// Flags
	};

	// How the parameter's value will be used.
	kParameterDataUsage, 1, noChildren,
	{
		kParameterUsagePercent;	// The parameter stores a percentage
	};
	
	// The default value for the parameter.
	// In this case this is a tween record	
	kParameterDataDefaultItem, 1, 2,
	{
	};
		// The tween record
		'twnt', 1, noChildren,
		{
			kParameterTypeDataFixed;	// This is a fixed-point tween
		};
		'data', 1, noChildren,
		{
			long { "0" };				// The default lower value (0%)
			long { "65536" };			// The default upper value (100%)
		};
};
};