//
//	File:		CFPreferences_SharedAppValues.h
//
//	Contains:	Definition of the interfaces to <CFPreferences_SharedAppValues.c>
// 
//  Version:    1.0
// 
//  Created:    8/31/06
//	
//  Copyright:  Copyright  2006 Apple Computer, Inc., All Rights Reserved
// 
//	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
//				consideration of your agreement to the following terms, and your use, installation, modification 
//				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
//				not agree with these terms, please do not use, install, modify or redistribute this Apple 
//				software.
//
//				In consideration of your agreement to abide by the following terms, and subject to these terms, 
//				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
//				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
//				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
//				redistribute the Apple Software in its entirety and without modifications, you must retain this 
//				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
//				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
//				endorse or promote products derived from the Apple Software without specific prior written 
//				permission from Apple.  Except as expressly stated in this notice, no other rights or 
//				licenses, express or implied, are granted by Apple herein, including but not limited to any 
//				patent rights that may be infringed by your derivative works or by other works in which the 
//				Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
//				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
//				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
//				OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
//				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
//				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
//				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
//				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
//				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#ifndef __CFPREFERENCES_SHAREDAPPVALUES__
#define __CFPREFERENCES_SHAREDAPPVALUES__

//*****************************************************
#pragma mark - includes & imports
//-----------------------------------------------------

#include <Carbon/Carbon.h>

//*****************************************************
#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
#pragma pack(2)
#endif

//*****************************************************
#pragma mark - typedef's, struct's, enums, defines, etc.
//-----------------------------------------------------

//*****************************************************
#pragma mark - exported globals
//-----------------------------------------------------

//*****************************************************
#pragma mark - exported function prototypes
//-----------------------------------------------------

// ****************************************************
// 
// CFPreferences_CopySharedAppValue( inKey, inApplicationID )
//
// Purpose:  This is a special version of CFPreferencesCopyAppValue that acesses
//           preferences stored in a shared location that writeable by non - admin users
//           ( /Users/Shared/Library/Preferences ).
//
// Inputs:   inKey - the key, must not be null
//           inApplicationID - the Application ID
//
// Returns:  CFPropertyListRef - the CFProperty list, Null if key not found
//
// Notes:    If not null then Caller must release the returned value
//

extern CFPropertyListRef CFPreferences_CopySharedAppValue( CFStringRef inKey, CFStringRef inApplicationID );


// ****************************************************
// 
// CFPreferences_SetSharedAppValue( inKey, inValue, inApplicationID )
//
// Purpose:  This is a special version of CFPreferences_SetAppValue that stores preferences in a 
//           location that is writeable by non - admin users ( /Users/Shared/Library/Preferences ).
//
// Inputs:   inKey - the key, must not be null
//           inValue - the value to be saved
//           inApplicationID - the Application ID
//
// Returns:  Boolean - TRUE if successful
//

extern Boolean CFPreferences_SetSharedAppValue( CFStringRef inKey, CFPropertyListRef inValue, CFStringRef inApplicationID );

//*****************************************************
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif	// __CFPREFERENCES_SHAREDAPPVALUES__ //
