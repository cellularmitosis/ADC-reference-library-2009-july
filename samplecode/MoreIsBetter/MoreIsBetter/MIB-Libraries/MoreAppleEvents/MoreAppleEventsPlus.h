/*
	File:		MoreAppleEventsPlus.h

	Contains:	Functions to help you when you are building and sending Apple events.

	DRI:		George Warner

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreAppleEventsPlus.h,v $
Revision 1.10  2002/11/08 22:54:23         
When using framework includes, explicitly include the frameworks we need.

Revision 1.9  2002/02/19 18:56:34        
Written by: => DRI:

Revision 1.8  2001/11/07 15:51:05         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Get rid of wacky Finder label.
         <6>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <5>     9/13/01    gaw     fix MoreAEAccessW[I]ldFromWild typo
         <4>     8/31/01    gaw     Fixed trailing garbage (CVS?)
         <3>     8/28/01    gaw     Changed pRefcon's to SInt32's
         <2>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <1>      3/9/00    GW      Intergrating AppleEvent Helper code. First Check In
*/

#pragma once

//********************************************************************************
//	A private conditionals file to setup the build environment for this project.
#include "MoreSetup.h"
//********************************************************************************
//	System includes
#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <AERegistry.h>
	#include <AEObjects.h>
	#include <AEPackObject.h>
	#include <Gestalt.h>
	#include <Processes.h>
#endif
//********************************************************************************
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
//*******************************************************************************
//**********	Private Definitions		*****************************************
//*******************************************************************************
#pragma mark Typedefs, structs, enums, etc. �
//*******************************************************************************

typedef struct MoreAEHandler_Struct {
	const AEEventClass			fAEEventClass;
	const AEEventID				fAEEventID;
	const AEEventHandlerProcPtr	fAEEventHandlerProcPtr;
	AEEventHandlerUPP 			fOldAEEventHandlerUPP;
	SInt32						fOldRefCon;
} MoreAEHandlerRec;

typedef struct MoreAEAccessor_Struct {
	const DescType				fDesiredClass;
	const DescType				fContainerType;
	const OSLAccessorProcPtr	fOSLAccessorProcPtr;
	OSLAccessorUPP				fOldOSLAccessorUPP;
	SInt32						fOldRefCon;
} MoreAEAccessorRec;

//*******************************************************************************
// ProcPtr's for our Get/Set Data routines
//*******************************************************************************
typedef pascal OSErr (*MoreAEHandle_GetDataProcPtr)(const AEDesc* pObjAEDesc, AEDesc* pDataAEDesc);
typedef pascal OSErr (*MoreAEHandle_SetDataProcPtr)(const AEDesc* pObjAEDesc, AEDesc* pDataAEDesc);
//*******************************************************************************
extern pascal OSErr MoreAEInitialize(
	const MoreAEHandle_GetDataProcPtr pMoreAEHandle_GetDataProcPtr,
	const MoreAEHandle_SetDataProcPtr pMoreAEHandle_SetDataProcPtr
);

//*******************************************************************************
#pragma mark ==> AEhandlers �
//*******************************************************************************
// This routine installs all the AppleEvent handlers used by this application.
extern pascal void MoreAEInstallAEHandlers(MoreAEHandlerRec* pMoreAEHandlerRecs,const SInt32 pCount);

//*******************************************************************************
// this is how we call thru to any previously installed AEHandler.
extern pascal OSErr MoreAECallPreviousAEHandler(
						const MoreAEHandlerRec* pMoreAEHandlerRecs,
						const AEEventClass	pAEEventClass,
						const AEEventID		pAEEventID,
						const AppleEvent	*pAppleEvent,
						AppleEvent 			*pAEReply,
						SInt32 				pRefcon);
//*******************************************************************************
// call this to install your AEAccessors
extern pascal void MoreAEInstallAEAccessors(MoreAEAccessorRec* pMoreAEAccessorRecs,const SInt32 pCount);
//*******************************************************************************
// this routine calls thru to a previously installed OSLAccessor
extern pascal OSErr MoreAECallPreviousOSLAccessor(
									const MoreAEAccessorRec *pMoreAEAccessorRecs,
									DescType pDesiredClass, 
									const AEDesc* pContainerToken, 
									DescType pContainerClass, 
									DescType pKeyForm, 
									const AEDesc* pKeyData, 
									AEDesc* pToken, 
									SInt32 pRefcon);
//*******************************************************************************
// This is an accessor that gets called if a more specific accessor
// isn't found.  (Great for debugging!)
extern pascal OSErr MoreAEAccessWildFromWild(
									DescType pDesiredClass, 
									const AEDesc* pContainerToken, 
									DescType pContainerClass, 
									DescType pKeyForm, 
									const AEDesc* pKeyData, 
									AEDesc* pToken, 
									SInt32 pRefcon);
//*******************************************************************************
// this is our get data AppleEvent handler.
extern pascal OSErr MoreAEHandleGetDataAEvent(
						const AppleEvent *pAppleEvent,
						AppleEvent *pAEReply,
						SInt32 pRefcon);

//*******************************************************************************
// this is our set data AppleEvent handler.
extern pascal OSErr MoreAEHandleSetDataAEvent(
						const AppleEvent *pAppleEvent,
						AppleEvent *pAEReply,
						SInt32 pRefcon);

//*******************************************************************************
// This is a typeWild event hander that will catch any AppleEvents
// that we haven't installed handers for. (Great for debugging!)
extern pascal OSErr MoreAEHandleWildAEvent(
						const AppleEvent *pAppleEvent,
						AppleEvent *pAEReply,
						SInt32 pRefcon);
//********************************************************************************
// Append this property (& it's data) to a list
extern pascal OSErr MoreAEAddPropertyToList(const DescType pPropDescType, AEDesc* pListAEDesc);

//********************************************************************************
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
