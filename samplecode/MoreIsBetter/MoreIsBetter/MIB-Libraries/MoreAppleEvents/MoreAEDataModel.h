/*
	File:		MoreAEDataModel.h

	Contains:	Helpers for AEDataModel stuff.

	DRI:		Quinn

	Copyright:	Copyright (c) 2000-2002 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreAEDataModel.h,v $
Revision 1.1  2002/11/08 22:46:06         
A new module split off from MoreAppleEvents. This is necessary because MoreAE is dependent on Carbon, whereas this stuff in only dependent on the ApplicationServices framework.


*/

#pragma once
//********************************************************************************
//	A private conditionals file to setup the build environment for this project.
#include "MoreSetup.h"

//**********	Universal Headers		****************************************
#if MORE_FRAMEWORK_INCLUDES
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <AEDataModel.h>
	#if TARGET_API_MAC_CARBON
		#include <CFString.h>
	#endif
#endif
//********************************************************************************
#ifdef __cplusplus
	extern "C" {
#endif

//*******************************************************************************
// Initialises desc to the null descriptor (typeNull,NULL).
extern pascal void MoreAENullDesc(AEDesc* desc);
//*******************************************************************************
// Disposes of desc and initialises it to the null descriptor.
extern pascal void MoreAEDisposeDesc(AEDesc* desc);

//*******************************************************************************
// These routines are only implemented in PreCarbon.o for PowerPC
// So for 68K we had to write our own versions
#if TARGET_CPU_68K || !ACCESSOR_CALLS_ARE_FUNCTIONS
extern pascal Size AEGetDescDataSize( const AEDesc* pAEDesc);
extern pascal OSErr AEGetDescData(
					const AEDesc*	pAEDesc,
					void*			pDataPtr,
					Size 			pMaxSize);
#endif TARGET_CPU_68K || !ACCESSOR_CALLS_ARE_FUNCTIONS

/********************************************************************************
	These are generic routines that are used to extract data of
	different types from descriptor records.

	pAEDesc			==>	The descriptor we want the data from
	pDestPtr		<==	Where we want to store the data from this desc.
	pMaxSize		==>	The maxium amount of data we can store.
	pActualSize		<==	The actual amount of data stored.

	RESULT CODES
	____________
	noErr			   0	No error	
	____________
*/
extern pascal void MoreAEGetRawDataFromDescriptor(const AEDesc* pAEDesc,
							 Ptr	pDestPtr,
							 Size	pMaxSize,
							 Size*	pActualSize);
#if TARGET_API_MAC_CARBON
extern pascal OSErr MoreAEGetCFStringFromDescriptor(const AEDesc* pAEDesc,CFStringRef* pCFStringRef);
#endif
extern pascal OSErr MoreAEGetPStringFromDescriptor(const AEDesc* pAEDesc,StringPtr pStringPtr);
extern pascal OSErr MoreAEGetCStringFromDescriptor(const AEDesc* pAEDesc,char* pCharPtr);
extern pascal OSErr MoreAEGetShortFromDescriptor(const AEDesc* pAEDesc,SInt16* pResult);
extern pascal OSErr MoreAEGetBooleanFromDescriptor(const AEDesc* pAEDesc,Boolean* pResult);
extern pascal OSErr MoreAEGetLongFromDescriptor(const AEDesc* pAEDesc,long* pResult);
extern pascal OSErr MoreAEGetOSTypeFromDescriptor(const AEDesc* pAEDesc,OSType* pResult);

/********************************************************************************
	Creates an AEDesc from a CFString.

  pCFStringRef	==>	The CFString we want the data from
  pAEDesc		<==	Where we want to store the AEDesc

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/

#if TARGET_API_MAC_CARBON
extern pascal OSErr MoreAECreateAEDescFromCFString(const CFStringRef pCFStringRef,AEDesc* pAEDesc);
#endif

//*******************************************************************************
// This routine creates a new handle and puts the contents of the desc
// in that handle.  Carbon�s opaque AEDesc�s means that we need this
// functionality a lot.

extern pascal OSStatus MoreAECopyDescriptorDataToHandle(const AEDesc* pAEDesc,Handle* pDescData);


//*******************************************************************************
// This routine prints a descriptor
extern pascal int MoreAEfprintDesc(void* pFILE, const AEDesc * pAEDesc);

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
