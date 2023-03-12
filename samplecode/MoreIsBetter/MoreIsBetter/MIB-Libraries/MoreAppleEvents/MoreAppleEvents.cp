/*
	File:		MoreAppleEvents.cp

	Contains:	Apple Event Manager utilities.

	DRI:		George Warner

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

$Log: MoreAppleEvents.cp,v $
Revision 1.25  2002/11/25 18:40:02         
Convert OSErr to OSStatus; also got rid of "implicit arithmetic conversion" warnings.

Revision 1.24  2002/11/08 22:53:36         
Convert nil to NULL. Convert MoreAssertQ to assert. Moved a bunch of stuff to MoreAEDataModel. Include our header early so as to detect any missing dependencies in the header.

Revision 1.23  2002/10/16 20:32:11        
added MoreAEfprintDesc routine and changed desc parameters to pAEDesc.

Revision 1.22  2002/10/03 00:02:40        
bug fix - corrected improper usage of PSN fields & constants.

Revision 1.21  2002/03/08 23:51:00        
More cleanup.
Fixed memory leak in MoreAETellSelfToSetCFStringRefProperty

Revision 1.20  2002/03/07 20:31:51        
General clean up.
Added recovery code to MoreAESendEventReturnPString.
New API: MoreAECreateAEDescFromCFString.

Revision 1.19  2002/02/19 18:54:57        
Written by: => DRI:

Revision 1.18  2002/01/16 19:11:06        
err => anErr, (anErr ?= noErr) => (noErr ?= anErr)
Added MoreAESendEventReturnAEDesc, MoreAESendEventReturnAEDescList,
MoreAETellSelfToSetCFStringRefProperty,  & MoreAEGetCFStringFromDescriptor routines.

Revision 1.17  2001/11/07 15:50:50         
Tidy up headers, add CVS logs, update copyright.


        <16>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <15>     8/28/01    gaw     CodeBert (error -> pError, theAppleEvent -> pAppleEvent, etc.)
        <14>     15/2/01    Quinn   MoreAECreateAppleEventTargetID is not supported for Carbon
                                    builds because all of its required declarations are defined
                                    CALL_NOT_IN_CARBON. Also some minor fixes prompted by gcc
                                    warnings.
        <13>     26/5/00    Quinn   Eliminate bogus consts detected by MPW's SC compiler.
        <12>     4/26/00    gaw     Fix bug, swapped creator & file type parameters
        <11>     27/3/00    Quinn   Remove MoreAEDeleteItemFromRecord.  It's functionality is
                                    covered by AEDeleteKeyDesc.
        <10>     20/3/00    Quinn   Added routines to deal with "missing value".  Added
                                    MoreAECopyDescriptorDataToHandle.  Added
                                    MoreAEDeleteItemFromRecord.
         <9>      3/9/00    gaw     Y2K!
         <8>      3/9/00    gaw     API changes for MoreAppleEvents
         <7>      3/9/00    GW      Intergrating AppleEvent Helper code. First Check In
         <6>      6/3/00    Quinn   Added a bunch of trivial wrapper routines.  George may come
                                    along and change all these soon, but I needed them for MoreOSL.
         <5>      1/3/00    Quinn   Change the signature for AEGetDescData to match the version we
                                    actually shipped.
         <4>     2/15/99    PCG     add AEGetDescDataSize for non-Carbon clients
         <3>     1/29/99    PCG     add AEGetDescData
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <2>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <2>     6/16/98    PCG     CreateProcessTarget works with nil PSN
         <1>     6/16/98    PCG     initial checkin
*/

//	Conditionals to setup the build environment the way we like it.
#include "MoreSetup.h"

//**********	Our Prototypes			****************************************
#include "MoreAppleEvents.h"

#if !MORE_FRAMEWORK_INCLUDES
	//**********	Universal Headers		****************************************
	#include <AERegistry.h>
	#include <AEHelpers.h>
	#include <AEObjects.h>
	#include <AEPackObject.h>
	#include <ASRegistry.h>
	//#include <FinderRegistry.h>
	#include <Gestalt.h>
#endif
#include <cstdio>

//**********	Project Headers			****************************************
#include "MoreAEDataModel.h"
#include "MoreAEObjects.h"
#include "MoreProcesses.h"
#include "MoreMemory.h"

//**********	Private Definitions		****************************************
enum {
	kFinderFileType			= 'FNDR',
	kFinderCreatorType		= 'MACS',
	kFinderProcessType		= 'FNDR',
	kFinderProcessSignature	= 'MACS'
};
static AEIdleUPP gAEIdleUPP = NULL;
//*******************************************************************************
#pragma mark ==> Create Target Descriptors for AEvents ¥
/********************************************************************************
	Create and return an AEDesc for the process target with the specified PSN.
	If no PSN is supplied the use the current process

	pAEEventClass	==>		The class of the event to be created.
	pAEEventID		==>		The ID of the event to be created.
	pAppleEvent		==>		Pointer to an AppleEvent where the
							event record will be returned.
					<==		The Apple event.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	memFullErr		-108	Not enough room in heap zone	
	____________
*/
pascal OSStatus MoreAECreateProcessTarget(ProcessSerialNumber* pPSN, AEDesc* pAppleEvent)
{
	ProcessSerialNumber self;

	if (!pPSN)
	{
		pPSN = &self;

		self.lowLongOfPSN		= kCurrentProcess;
		self.highLongOfPSN		= 0;
	}

	return AECreateDesc (typeProcessSerialNumber,pPSN,sizeof(*pPSN),pAppleEvent);
}	// MoreAECreateProcessTarget
//*******************************************************************************
#pragma mark ==> Create AEvents ¥
/********************************************************************************
	Create and return an AppleEvent of the given class and ID. The event will be
	targeted at the current process, with an AEAddressDesc of type
	typeProcessSerialNumber.

	pAEEventClass	==>		The class of the event to be created.
	pAEEventID		==>		The ID of the event to be created.
	pAppleEvent		==>		Pointer to an AppleEvent where the
							event record will be returned.
					<==		The Apple event.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	memFullErr		-108	Not enough room in heap zone	
	____________
*/
pascal OSStatus MoreAECreateAppleEventSelfTarget(
					AEEventClass pAEEventClass,
					AEEventID pAEEventID,
					AppleEvent* pAppleEvent)
{
	OSStatus anError = noErr;
	
	ProcessSerialNumber		selfPSN = {0, kCurrentProcess};
	
	anError = MoreAECreateAppleEventProcessTarget( &selfPSN, pAEEventClass, pAEEventID, pAppleEvent );
	
	return ( anError );
}//end MoreAECreateAppleEventSelfTarget

/********************************************************************************
	Create and return an AppleEvent of the given class and ID. The event will be
	targeted at the process specified by the target type and creator codes, 
	with an AEAddressDesc of type typeProcessSerialNumber.

	pType			==>		The file type of the process to be found.
	pCreator		==>		The creator type of the process to be found.
	pAEEventClass	==>		The class of the event to be created.
	pAEEventID		==>		The ID of the event to be created.
	pAppleEvent		==>		Pointer to an AppleEvent where the
							event record will be returned.
					<==		The Apple event.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	memFullErr		-108	Not enough room in heap zone	
	procNotFound	Ð600	No eligible process with specified descriptor
	____________
*/
pascal	OSStatus	MoreAECreateAppleEventSignatureTarget(
						OSType pType,
												  		OSType pCreator,
												  		AEEventClass pAEEventClass,
												  		AEEventID pAEEventID,
						AppleEvent* pAppleEvent )
{
	OSStatus anError = noErr;
	
	ProcessSerialNumber		psn = {0, kNoProcess};
	
	// <12> bug fix, pCreator & pType parameters swapped.
	anError = MoreProcFindProcessBySignature( pCreator, pType, &psn );
	if ( noErr == anError )
	{
		anError = MoreAECreateAppleEventProcessTarget( &psn, pAEEventClass, pAEEventID, pAppleEvent );
	}
	return anError;
}//end MoreAECreateAppleEventSignatureTarget

/********************************************************************************
	Create and return an AppleEvent of the given class and ID. The event will be
	targeted at the application with the specific creator.

	psnPtr			==>		Pointer to the PSN to target the event with.
	pAEEventClass	==>		The class of the event to be created.
	pAEEventID		==>		The ID of the event to be created.
	pAppleEvent		==>		Pointer to an AppleEvent where the
							event record will be returned.
					<==		The Apple event.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	memFullErr		-108	Not enough room in heap zone	
	procNotFound	Ð600	No eligible process with specified descriptor
	____________
*/
pascal OSStatus MoreAECreateAppleEventCreatorTarget(
							const AEEventClass pAEEventClass,
							const AEEventID pAEEventID,
							const OSType pCreator,
							AppleEvent* pAppleEvent)
{
	OSStatus 	anError;
	AEDesc 		targetDesc;
	
	assert(pAppleEvent != NULL);

	MoreAENullDesc(&targetDesc);
	
	anError = AECreateDesc(typeApplSignature, &pCreator, sizeof(pCreator), &targetDesc);
	if (noErr == anError)
		anError = AECreateAppleEvent(pAEEventClass, pAEEventID, &targetDesc, 
									kAutoGenerateReturnID, kAnyTransactionID, pAppleEvent);
	MoreAEDisposeDesc(&targetDesc);
	
	return anError;
}//end MoreAECreateAppleEventCreatorTarget

/********************************************************************************
	Create and return an AppleEvent of the given class and ID. The event will be
	targeted with the provided PSN.

	psnPtr			==>		Pointer to the PSN to target the event with.
	pAEEventClass	==>		The class of the event to be created.
	pAEEventID		==>		The ID of the event to be created.
	pAppleEvent		==>		Pointer to an AppleEvent where the
							event record will be returned.
					<==		The Apple event.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	memFullErr		-108	Not enough room in heap zone	
	procNotFound	Ð600	No eligible process with specified descriptor
	____________
*/
pascal	OSStatus	MoreAECreateAppleEventProcessTarget(
						const ProcessSerialNumberPtr psnPtr,
						AEEventClass pAEEventClass,
						AEEventID pAEEventID,
						AppleEvent* pAppleEvent )
{
	OSStatus	anError = noErr;
	AEDesc		targetAppDesc = {typeNull,NULL};
	
	anError = AECreateDesc (typeProcessSerialNumber, psnPtr, sizeof( ProcessSerialNumber ), &targetAppDesc);

	if ( noErr == anError )
	{
		anError = AECreateAppleEvent( pAEEventClass, pAEEventID, &targetAppDesc,
									kAutoGenerateReturnID, kAnyTransactionID, pAppleEvent);
	}
	
	MoreAEDisposeDesc( &targetAppDesc );
	
	return anError;
}//end MoreAECreateAppleEventProcessTarget

/********************************************************************************
	Create and return an AppleEvent of the given class and ID. The event will be
	targeted with the provided TargetID.

	pTargetID		==>		Pointer to the TargetID to target the event with.
	pAEEventClass	==>		The class of the event to be created.
	pAEEventID		==>		The ID of the event to be created.
	pAppleEvent		==>		Pointer to an AppleEvent where the
							event record will be returned.
					<==		The Apple event.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	memFullErr		-108	Not enough room in heap zone	
	procNotFound	Ð600	No eligible process with specified descriptor
	____________
*/
#if CALL_NOT_IN_CARBON

// See comment in header file.

pascal	OSStatus	MoreAECreateAppleEventTargetID(
					const TargetID* pTargetID,
			  		AEEventClass pAEEventClass,
			  		AEEventID pAEEventID,
					AppleEvent* pAppleEvent )
{
	OSStatus	anError = noErr;
	AEDesc		targetAppDesc = {typeNull,NULL};
	
	anError = AECreateDesc (typeTargetID, pTargetID, sizeof( TargetID ), &targetAppDesc);

	if ( noErr == anError )
	{
		anError = AECreateAppleEvent( pAEEventClass, pAEEventID, &targetAppDesc,
									kAutoGenerateReturnID, kAnyTransactionID, pAppleEvent);
	}
	
	MoreAEDisposeDesc( &targetAppDesc );
	
	return anError;
}//end MoreAECreateAppleEventTargetID

#endif

#pragma mark ==> Send AppleEvents ¥
#if 0
//¥	De-appreciated! Don't use! Use one of the more specific routines (w/idle proc) below.
pascal OSErr MoreAESendAppleEvent (const AppleEvent* pAppleEvent, AppleEvent* pAEReply)
{
	OSErr anErr = noErr;

	AESendMode aeSendMode = kAEAlwaysInteract | kAECanSwitchLayer;

	if (pAEReply)
	{
		aeSendMode |= kAEWaitReply;

		MoreAENullDesc(pAEReply);
	}

	anErr = AESend (pAppleEvent, pAEReply, aeSendMode, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);

	return anErr;
}//end MoreAESendAppleEvent
#endif 0

/********************************************************************************
	Send the provided AppleEvent using the provided idle function.
	Will wait for a reply if an idle function is provided, but no result will be returned.

	pIdleProcUPP	==>		The idle function to use when sending the event.
	pAppleEvent		==>		The event to be sent.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	
	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal	OSStatus	MoreAESendEventNoReturnValue(
					const AEIdleUPP pIdleProcUPP,
					const AppleEvent* pAppleEvent )
{
	OSStatus	anError = noErr;
	AppleEvent	theReply = {typeNull,NULL};
	AESendMode	sendMode;

	if (NULL == pIdleProcUPP)
		sendMode = kAENoReply;
	else
		sendMode = kAEWaitReply;

	anError = AESend( pAppleEvent, &theReply, sendMode, kAENormalPriority, kNoTimeOut, pIdleProcUPP, NULL );
	if ((noErr == anError) && (kAEWaitReply == sendMode))
		anError = MoreAEGetHandlerError(&theReply);

	MoreAEDisposeDesc( &theReply );
	
	return anError;
}//end MoreAESendEventNoReturnValue

/********************************************************************************
	Send the provided AppleEvent using the provided idle function.
	Return the direct object as a AEDesc of pAEDescType

	pIdleProcUPP	==>		The idle function to use when sending the event.
	pAppleEvent		==>		The event to be sent.
	pDescType		==>		The type of value returned by the event.
	pAEDescList		<==		The value returned by the event.

	RESULT CODES
	____________
	noErr			   0	No error	
	paramErr		 -50	No idle function provided

	and any other error that can be returned by AESend
	or the handler in the application that gets the event.
	____________
*/
pascal OSStatus MoreAESendEventReturnAEDesc(
						const AEIdleUPP		pIdleProcUPP,
						const AppleEvent	*pAppleEvent,
						const DescType		pDescType,
						AEDesc				*pAEDesc)
{
	OSStatus anError = noErr;

	//	No idle function is an error, since we are expected to return a value
	if (pIdleProcUPP == NULL)
		anError = paramErr;
	else
	{
		AppleEvent theReply = {typeNull,NULL};
		AESendMode sendMode = kAEWaitReply;

		anError = AESend(pAppleEvent, &theReply, sendMode, kAENormalPriority, kNoTimeOut, pIdleProcUPP, NULL);
		//	[ Don't dispose of the event, it's not ours ]
		if (noErr == anError)
		{
			anError = MoreAEGetHandlerError(&theReply);

			if (!anError && theReply.descriptorType != typeNull)
			{
				anError = AEGetParamDesc(&theReply, keyDirectObject, pDescType, pAEDesc);
			}
			MoreAEDisposeDesc(&theReply);
		}
	}
	return anError;
}	// MoreAESendEventReturnAEDesc

/********************************************************************************
	Send the provided AppleEvent using the provided idle function.
	Return the direct object as a AEDescList

	pIdleProcUPP	==>		The idle function to use when sending the event.
	pAppleEvent		==>		The event to be sent.
	pAEDescList		<==		The value returned by the event.

	RESULT CODES
	____________
	noErr			   0	No error	
	paramErr		 -50	No idle function provided

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAESendEventReturnAEDescList(
					const AEIdleUPP pIdleProcUPP,
					const AppleEvent* pAppleEvent,
					AEDescList* pAEDescList)
{
	return MoreAESendEventReturnAEDesc(pIdleProcUPP,pAppleEvent,typeAEList,pAEDescList);
}	// MoreAESendEventReturnAEDescList

/********************************************************************************
	Send the provided AppleEvent using the provided idle function.
	Return data (at pDataPtr) of type pDesiredType

	pIdleProcUPP	==>		The idle function to use when sending the event.
	pAppleEvent		==>		The event to be sent.
	theValue		<==		The value returned by the event.

	RESULT CODES
	____________
	noErr			   0	No error	
	paramErr		 -50	No idle function provided

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAESendEventReturnData(
						const AEIdleUPP		pIdleProcUPP,
						const AppleEvent	*pAppleEvent,
						DescType			pDesiredType,
						DescType*			pActualType,
						void*		 		pDataPtr,
						Size				pMaximumSize,
						Size 				*pActualSize)
{
	OSStatus anError = noErr;

	//	No idle function is an error, since we are expected to return a value
	if (pIdleProcUPP == NULL)
		anError = paramErr;
	else
	{
		AppleEvent theReply = {typeNull,NULL};
		AESendMode sendMode = kAEWaitReply;

		anError = AESend(pAppleEvent, &theReply, sendMode, kAENormalPriority, kNoTimeOut, pIdleProcUPP, NULL);
		//	[ Don't dispose of the event, it's not ours ]
		if (noErr == anError)
		{
			anError = MoreAEGetHandlerError(&theReply);

			if (!anError && theReply.descriptorType != typeNull)
			{
				anError = AEGetParamPtr(&theReply, keyDirectObject, pDesiredType,
							pActualType, pDataPtr, pMaximumSize, pActualSize);
			}
			MoreAEDisposeDesc(&theReply);
		}
	}
	return anError;
}	// MoreAESendEventReturnData

/********************************************************************************
	Send the provided AppleEvent using the provided idle function.
	Return a SInt16 (typeSmallInteger).

	pIdleProcUPP	==>		The idle function to use when sending the event.
	pAppleEvent		==>		The event to be sent.
	theValue		<==		The value returned by the event.
	
	RESULT CODES
	____________
	noErr			   0	No error	
	paramErr		 -50	No idle function provided
	
	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAESendEventReturnSInt16(
						const AEIdleUPP pIdleProcUPP,
						const AppleEvent* pAppleEvent,
						SInt16* pValue)
{
	DescType			actualType;
	Size 				actualSize;

	return MoreAESendEventReturnData(pIdleProcUPP,pAppleEvent,typeShortInteger,
				&actualType,pValue,sizeof(SInt16),&actualSize);
}	// MoreAESendEventReturnSInt16

/********************************************************************************
	Send the provided AppleEvent using the provided idle function.
	Returns a PString.

	pIdleProcUPP	==>		The idle function to use when sending the event.
	pAppleEvent		==>		The event to be sent.
	pStr255			<==		The value returned by the event.

	RESULT CODES
	____________
	noErr			   0	No error	
	paramErr		 -50	No idle function provided

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/

pascal OSStatus MoreAESendEventReturnPString(
						const AEIdleUPP pIdleProcUPP,
						const AppleEvent* pAppleEvent,
						Str255 pStr255)
{
	DescType			actualType;
	Size 				actualSize;
	OSStatus			anError;

	anError = MoreAESendEventReturnData(pIdleProcUPP,pAppleEvent,typePString,
				&actualType,pStr255,sizeof(Str255),&actualSize);

	if (errAECoercionFail == anError)
	{
		anError =  MoreAESendEventReturnData(pIdleProcUPP,pAppleEvent,typeChar,
			&actualType,(Ptr) &pStr255[1],sizeof(Str255),&actualSize);
		if (actualSize < 256)
			pStr255[0] = (UInt8) actualSize;
		else
			anError = errAECoercionFail;
	}
	return anError;
}	// MoreAESendEventReturnPString
#pragma mark ==> Functions for talking to ourselfs

/********************************************************************************
	Send an AppleEvent of the specified Class & ID to myself using the 
	default idle function.

	pEventID	==>		The event to be sent.

	RESULT CODES
	____________
	noErr			   0	No error	

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAESendToSelfNoReturnValue(
				const AEEventClass pEventClass,
				const AEEventID pEventID)
{
	AppleEvent 	tAppleEvent = {typeNull,NULL};	//	If you always init AEDescs, it's always safe to dispose of them.
	OSStatus 	anError = noErr;

	if (NULL == gAEIdleUPP)
		gAEIdleUPP = NewAEIdleUPP(MoreAESimpleIdleFunction);

	anError = MoreAECreateAppleEventSelfTarget(pEventClass,pEventID,&tAppleEvent);
	if (noErr == anError)
	{
		AEDesc containerObj = {typeNull,NULL};	// start with the null (application) container
		AEDesc propertyObject = {typeNull,NULL};

		anError = MoreAEOCreatePropertyObject(pSelection, &containerObj, &propertyObject);
		if (noErr == anError)
		{
			anError = AEPutParamDesc(&tAppleEvent, keyDirectObject, &propertyObject);
			MoreAEDisposeDesc(&propertyObject);		//	Always dispose of objects as soon as you are done (helps avoid leaks)
			if (noErr == anError)
				anError = MoreAESendEventNoReturnValue(gAEIdleUPP, &tAppleEvent);
		}
		MoreAEDisposeDesc(&tAppleEvent);			// always dispose of AEDescs when you are finished with them
	}
	return anError;
}	// MoreAESendToSelfNoReturnValue

/********************************************************************************
	Send an AppleEvent of the specified Class & ID to myself using the 
	default idle function. Wait for a reply and extract a SInt16 result.

	pEventClass		==>		The event class to be sent.
	pEventID		==>		The event ID to be sent.
	pValue			<==		Where the return SInt16 will be stored.

	RESULT CODES
	____________
	noErr			   0	No error	

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAESendToSelfReturnSInt16(
				const AEEventClass pEventClass,
				const AEEventID pEventID,
				SInt16* pValue)
{
	AppleEvent 	tAppleEvent = {typeNull,NULL};	//	If you always init AEDescs, it's always safe to dispose of them.
	OSStatus 	anError = noErr;

	if (NULL == gAEIdleUPP)
		gAEIdleUPP = NewAEIdleUPP(MoreAESimpleIdleFunction);

	anError = MoreAECreateAppleEventSelfTarget(pEventClass,pEventID,&tAppleEvent);
	if (noErr == anError)
	{
		AEDesc containerObj = {typeNull,NULL};	// start with the null (application) container
		AEDesc propertyObject = {typeNull,NULL};

		anError = MoreAEOCreatePropertyObject(pSelection, &containerObj, &propertyObject);
		if (noErr == anError)
		{
			anError = AEPutParamDesc(&tAppleEvent, keyDirectObject, &propertyObject);
			MoreAEDisposeDesc(&propertyObject);	//	Always dispose of objects as soon as you are done (helps avoid leaks)
			if (noErr == anError)
				anError = MoreAESendEventReturnSInt16(gAEIdleUPP, &tAppleEvent, pValue);
		}
		MoreAEDisposeDesc(&tAppleEvent);	// always dispose of AEDescs when you are finished with them
	}
	return anError;
}//end MoreAESendToSelfReturnSInt16

/********************************************************************************
	Send a get data (kAEGetData) AppleEvent to myself using the 
	default idle function. Wait for a reply and extract a SInt16 result.

	pPropType		==>		The property type.
	pValue			<==		Where the resulting SInt16 will be stored.

	RESULT CODES
	____________
	noErr			   0	No error	

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAETellSelfToGetSInt16Property(const DescType pPropType,SInt16* pValue)
{
	AppleEvent 	tAppleEvent = {typeNull,NULL};	//	If you always init AEDescs, it's always safe to dispose of them.
	OSStatus 	anError = noErr;

	if (NULL == gAEIdleUPP)
		gAEIdleUPP = NewAEIdleUPP(MoreAESimpleIdleFunction);

	anError = MoreAECreateAppleEventSelfTarget(kAECoreSuite,kAEGetData,&tAppleEvent);
	if (noErr == anError)
	{
		AEDesc containerObj = {typeNull,NULL};	// start with the null (application) container
		AEDesc propertyObject = {typeNull,NULL};

		anError = MoreAEOCreatePropertyObject(pPropType, &containerObj, &propertyObject);
		if (noErr == anError)
		{
			anError = AEPutParamDesc(&tAppleEvent, keyDirectObject, &propertyObject);
			MoreAEDisposeDesc(&propertyObject);	//	Always dispose of objects as soon as you are done (helps avoid leaks)
			if (noErr == anError)
				anError = MoreAESendEventReturnSInt16(gAEIdleUPP, &tAppleEvent, pValue);
		}
		MoreAEDisposeDesc(&tAppleEvent);	// always dispose of AEDescs when you are finished with them
	}
	return anError;
}//end MoreAETellSelfToGetSInt16Property

/********************************************************************************
	Send a get data (kAEGetData) AppleEvent to myself using the 
	default idle function. Wait for a reply and extract a Str255 result.

	pPropType		==>		The property type.
	pValue			<==		Where the resulting Str255 will be stored.

	RESULT CODES
	____________
	noErr			   0	No error	

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAETellSelfToGetStr255Property(const DescType pPropType,Str255 pValue)
{
	AppleEvent 	tAppleEvent = {typeNull,NULL};	//	If you always init AEDescs, it's always safe to dispose of them.
	OSStatus 	anError = noErr;

	if (NULL == gAEIdleUPP)
		gAEIdleUPP = NewAEIdleUPP(MoreAESimpleIdleFunction);

	anError = MoreAECreateAppleEventSelfTarget(kAECoreSuite,kAEGetData,&tAppleEvent);
	if (noErr == anError)
	{
		AEDesc containerObj = {typeNull,NULL};	// start with the null (application) container
		AEDesc propertyObject = {typeNull,NULL};

		anError = MoreAEOCreatePropertyObject(pPropType, &containerObj, &propertyObject);
		if (noErr == anError)
		{
			anError = AEPutParamDesc(&tAppleEvent, keyDirectObject, &propertyObject);
			MoreAEDisposeDesc(&propertyObject);	//	Always dispose of objects as soon as you are done (helps avoid leaks)
			if (noErr == anError)
				anError = MoreAESendEventReturnPString(gAEIdleUPP, &tAppleEvent, pValue);
		}
		MoreAEDisposeDesc(&tAppleEvent);	// always dispose of AEDescs when you are finished with them
	}
	return anError;
}//end MoreAETellSelfToGetStr255Property

/********************************************************************************
	Send a set data (kAESetData) AppleEvent to myself with a SInt16 parameter
	and using the default idle function.

	pPropType		==>		The property type.
	pValue			==>		The SInt16 value to be set.

	RESULT CODES
	____________
	noErr			   0	No error	

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAETellSelfToSetSInt16Property(const DescType pPropType,SInt16 pValue)
{
	AppleEvent	tAppleEvent = {typeNull,NULL};	//	If you always init AEDescs, it's always safe to dispose of them.
	OSStatus 	anError = noErr;

	if (NULL == gAEIdleUPP)
		gAEIdleUPP = NewAEIdleUPP(MoreAESimpleIdleFunction);

	anError = MoreAECreateAppleEventSelfTarget(kAECoreSuite,kAESetData,&tAppleEvent);
	if (noErr == anError)
	{
		AEDesc containerObj = {typeNull,NULL};	// start with the null (application) container
		AEDesc propertyObject = {typeNull,NULL};

		anError = MoreAEOCreatePropertyObject(pPropType, &containerObj, &propertyObject);
		if (noErr == anError)
		{
			anError = AEPutParamDesc(&tAppleEvent, keyDirectObject, &propertyObject);
			MoreAEDisposeDesc(&propertyObject);	//	Always dispose of objects as soon as you are done (helps avoid leaks)
			if (noErr == anError)
			{
				anError = AEPutParamPtr(&tAppleEvent, keyAEData, typeSInt16, &pValue, sizeof(SInt16));
				if (noErr == anError)
					anError = MoreAESendEventNoReturnValue(gAEIdleUPP, &tAppleEvent);
			}
		}
		MoreAEDisposeDesc(&tAppleEvent);	// always dispose of AEDescs when you are finished with them
	}
	return anError;
}//end MoreAETellSelfToSetSInt16Property

/********************************************************************************
	Send a set data (kAESetData) AppleEvent to myself with a Pascal string
	parameter and using the default idle function.

	pEventID			==>		The event to be sent.
	pValue				==>		The Str255 to be sent.

	RESULT CODES
	____________
	noErr			   0	No error	

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAETellSelfToSetStr255Property(const DescType pPropType,Str255 pValue)
{
	AppleEvent 	tAppleEvent = {typeNull,NULL};	//	If you always init AEDescs, it's always safe to dispose of them.
	OSStatus 	anError = noErr;

	if (NULL == gAEIdleUPP)
		gAEIdleUPP = NewAEIdleUPP(MoreAESimpleIdleFunction);

	anError = MoreAECreateAppleEventSelfTarget(kAECoreSuite,kAESetData,&tAppleEvent);
	if (noErr == anError)
	{
		AEDesc containerObj = {typeNull,NULL};	// start with the null (application) container
		AEDesc propertyObject = {typeNull,NULL};

		anError = MoreAEOCreatePropertyObject(pPropType, &containerObj, &propertyObject);
		if (noErr == anError)
		{
			anError = AEPutParamDesc(&tAppleEvent, keyDirectObject, &propertyObject);
			MoreAEDisposeDesc(&propertyObject);
			if (noErr == anError)
			{
				anError = AEPutParamPtr(&tAppleEvent, keyAEData, typePString, pValue, pValue[0] + 1);
				if (noErr == anError)
					anError = MoreAESendEventNoReturnValue(gAEIdleUPP, &tAppleEvent);
			}
		}
		MoreAEDisposeDesc(&tAppleEvent);	// always dispose of AEDescs when you are finished with them
	}
	return anError;
} // MoreAETellSelfToSetStr255Property

/********************************************************************************
	Send a set data (kAESetData) AppleEvent to myself with a CFStringRef
	parameter and using the default idle function.

	pEventID			==>		The event to be sent.
	pValue				==>		The CFString to be sent.

	RESULT CODES
	____________
	noErr			   0	No error	

	and any other error that can be returned by AESend or the handler
	in the application that gets the event.
	____________
*/
pascal OSStatus MoreAETellSelfToSetCFStringRefProperty(
				const DescType pPropType,
				const CFStringRef pCFStringRef)
{
	AppleEvent tAppleEvent = {typeNull,NULL};	//	If you always init AEDescs, it's always safe to dispose of them.
	CFIndex length = CFStringGetLength(pCFStringRef);
    const UniChar* dataPtr = CFStringGetCharactersPtr(pCFStringRef);
	const UniChar* tempPtr = NULL;
	OSStatus anError = noErr;

    if (dataPtr == NULL)
	{
		tempPtr = (UniChar*) NewPtr( (Size) (length * sizeof(UniChar)) );

		if (NULL == tempPtr) return memFullErr;

		CFStringGetCharacters(pCFStringRef, CFRangeMake(0,length), (UniChar*) tempPtr);
		dataPtr = tempPtr;
	}

	if (NULL == gAEIdleUPP)
		gAEIdleUPP = NewAEIdleUPP(MoreAESimpleIdleFunction);

	anError = MoreAECreateAppleEventSelfTarget(kAECoreSuite,kAESetData,&tAppleEvent);
	if (noErr == anError)
	{
		AEDesc containerObj = {typeNull,NULL};	// start with the null (application) container
		AEDesc propertyObject = {typeNull,NULL};

		anError = MoreAEOCreatePropertyObject(pPropType, &containerObj, &propertyObject);
		if (noErr == anError)
		{
			anError = AEPutParamDesc(&tAppleEvent, keyDirectObject, &propertyObject);
			MoreAEDisposeDesc(&propertyObject);
			if (noErr == anError)
			{
				anError = AEPutParamPtr(&tAppleEvent, keyAEData, typeUnicodeText, dataPtr, (Size) (length * sizeof(UniChar)) );
				if (noErr == anError)
					anError = MoreAESendEventNoReturnValue(gAEIdleUPP, &tAppleEvent);
			}
		}
		MoreAEDisposeDesc(&tAppleEvent);	// always dispose of AEDescs when you are finished with them
	}
	if (NULL != tempPtr)
		DisposePtr((Ptr) tempPtr);

	return anError;
}	// MoreAETellSelfToSetCFStringRefProperty
//*******************************************************************************
#pragma mark ==> Misc. AE utility functions ¥
//*******************************************************************************
// Appends each of the items in pSourceList to the pDestList.
pascal OSStatus MoreAEAppendListToList(const AEDescList* pSourceList, AEDescList* pDestList)
{
	OSStatus  anError;
	AEKeyword junkKeyword;
	SInt32    listCount;
	SInt32    listIndex;
	AEDesc	  thisValue;

	assert(pSourceList != NULL);
	assert(pDestList   != NULL);

	anError = AECountItems(pSourceList, &listCount);
	if (noErr == anError) {
		for (listIndex = 1; listIndex <= listCount; listIndex++) {
			MoreAENullDesc(&thisValue);
			
			anError = AEGetNthDesc(pSourceList, listIndex, typeWildCard, &junkKeyword, &thisValue);
			if (noErr == anError) {
				anError = AEPutDesc(pDestList, 0, &thisValue);
			}
			
			MoreAEDisposeDesc(&thisValue);
			if (noErr != anError) {
				break;
			}
		}
	}

	return anError;
}//end MoreAEAppendListToList

//*******************************************************************************
// This routine takes a result descriptor and an error.
// If there is a result to add to the reply it makes sure the reply isn't
// NULL itself then adds the error to the reply depending on the error
// and the type of result.
pascal OSStatus MoreAEMoreAESetReplyErrorNumber (OSErr pOSErr, AppleEvent* pAEReply)
{
	OSStatus anError = noErr;

	if (pAEReply->dataHandle)
	{
		if (!MoreAssertPCG (pAEReply->descriptorType == typeAppleEvent))
			anError = paramErr;
		else
			anError = AEPutParamPtr (pAEReply,keyErrorNumber,typeShortInteger,&pOSErr,sizeof(pOSErr));
	}

	return anError;
}//end MoreAEMoreAESetReplyErrorNumber
//*******************************************************************************
// This routine takes a result descriptor, a reply descriptor and an error.
// If there is a result to add to the reply it makes sure the reply isn't
// NULL itself then adds the result to the reply depending on the error
// and the type of result.

pascal OSStatus MoreAEAddResultToReply(const AEDesc* pResult, AEDesc* pAEReply, const OSErr pError)
{
	OSStatus anError;

	// Check that the pAEReply is not NULL and there is a result to put in it  
	if (typeNull == pAEReply->descriptorType || typeNull == pResult->descriptorType)
		return (pError);
	
	if (noErr == pError)
		anError = AEPutParamDesc(pAEReply, keyDirectObject, pResult);
	else
	{
		switch (pResult->descriptorType)
		{
			case typeInteger:
				anError = AEPutParamDesc(pAEReply, keyErrorNumber, pResult);
				break;
				
			case typeChar:
				anError = AEPutParamDesc(pAEReply, keyErrorString, pResult);
				break;
				
			default:
				anError = errAETypeError;
		}
		
		if (noErr == anError)
			anError = pError;		// Don't loose that error
	}
	return (anError);
}//end MoreAEAddResultToReply
//*******************************************************************************
//	Name:		MoreAEGotRequiredParams
//	Function:	Checks that all parameters defined as 'required' have been read

pascal OSStatus	MoreAEGotRequiredParams(const AppleEvent* pAppleEventPtr)
{
	DescType	returnedType;
	Size		actualSize;
	OSStatus	anError;
	
	// look for the keyMissedKeywordAttr, just to see if it's there
	
	anError = AEGetAttributePtr(pAppleEventPtr, keyMissedKeywordAttr, typeWildCard,
												&returnedType, NULL, 0, &actualSize);
	
	switch (anError)
	{
		case errAEDescNotFound:		// attribute not there means we
			anError = noErr;			// got all required parameters.
			break;
			
		case noErr:					// attribute there means missed
			anError = errAEParamMissed;	// at least one parameter.
			break;
			
		// default:		pass on unexpected error in looking for the attribute
	}
	return (anError);
} // GotReqiredParams

/********************************************************************************
	Takes a reply event checks it for any errors that may have been returned
	by the event handler. A simple function, in that it only returns the error
	number. You can often also extract an error string and three other error
	parameters from a reply event.
	
	Also see:
		IM:IAC for details about returned error strings.
		AppleScript developer release notes for info on the other error parameters.
	
	pAEReply	==>		The reply event to be checked.

	RESULT CODES
	____________
	noErr				    0	No error	
	????					??	Pretty much any error, depending on what the
								event handler returns for it's errors.
*/
pascal	OSStatus	MoreAEGetHandlerError(const AppleEvent* pAEReply)
{
	OSStatus	anError = noErr;
	OSErr		handlerErr;
	
	DescType	actualType;
	long		actualSize;
	
	if ( pAEReply->descriptorType != typeNull )	// there's a reply, so there may be an error
	{
		OSErr	getErrErr = noErr;
		
		getErrErr = AEGetParamPtr( pAEReply, keyErrorNumber, typeShortInteger, &actualType,
									&handlerErr, sizeof( OSErr ), &actualSize );
		
		if ( getErrErr != errAEDescNotFound )	// found an errorNumber parameter
		{
			anError = handlerErr;					// so return it's value
		}
	}
	return anError;
}//end MoreAEGetHandlerError

/********************************************************************************
	Get the class and ID from an AppleEvent.

	pAppleEvent			==>		The event to get the class and ID from.
	pAEEventClass	   output:	The event's class.
	pAEEventID		   output:	The event's ID.
	
	RESULT CODES
	____________
	noErr					    0	No error	
	memFullErr				 -108	Not enough room in heap zone	
	errAEDescNotFound 		-1701	Descriptor record was not found	
	errAENotAEDesc			-1704	Not a valid descriptor record	
	errAEReplyNotArrived	-1718	Reply has not yet arrived	
*/	
pascal OSStatus MoreAEExtractClassAndID(
					const AppleEvent* pAppleEvent,
					AEEventClass* pAEEventClass,
					AEEventID* pAEEventID )
{
	DescType	actualType;
	Size		actualSize;
	OSStatus	anError;

	anError = AEGetAttributePtr( pAppleEvent, keyEventClassAttr, typeType, &actualType,
								pAEEventClass, sizeof( pAEEventClass ), &actualSize );
	if ( noErr == anError )
	{
		anError = AEGetAttributePtr( pAppleEvent, keyEventIDAttr, typeType, &actualType,
									pAEEventID, sizeof( pAEEventID ), &actualSize );
	}
	return ( anError );
}//end ExtractClassAndID

/********************************************************************************
	A very simple idle function. It simply ignors any event it receives,
	returns 30 for the sleep time and NULL for the mouse region.
	
	Your application should supply an idle function that handles any events it
	might receive. This is especially important if your application has any windows.
	
	Also see:
		IM:IAC for details about idle functions.
		Pending Update Perils technote for more about handling low-level events.
*/	
pascal	Boolean	MoreAESimpleIdleFunction(
	EventRecord* event,
	long* sleepTime,
	RgnHandle* mouseRgn )
{
#pragma unused( event )
	*sleepTime = 30;
	*mouseRgn = NULL;
	
	return ( false );
}//end MoreAESimpleIdleFunction

/********************************************************************************
	Is the Apple Event Manager present.
	
	RESULT CODES
	____________
	true	The Apple Event Manager is present
	false	It isn't
*/
pascal Boolean MoreAEHasAppleEvents(void)
{
	static	long		gHasAppleEvents = kFlagNotSet;
	
	if ( gHasAppleEvents == kFlagNotSet )
	{
		long	response;
		
		if ( Gestalt( gestaltAppleEventsAttr, &response ) == noErr )
			gHasAppleEvents = ( response & (1L << gestaltAppleEventsPresent) ) != 0;
	}
	return (gHasAppleEvents != 0);
}//end MoreAEHasAppleEvents
//*******************************************************************************
// Did this AppleEvent come from the Finder?
pascal OSStatus MoreAEIsSenderFinder (const AppleEvent* pAppleEvent, Boolean* pIsFinder)
{
	OSStatus				anError = noErr;
	DescType				actualType;
	ProcessSerialNumber		senderPSN;
	Size					actualSize;

	if (!MoreAssertPCG (pAppleEvent && pIsFinder))							return paramErr;
	if (!MoreAssertPCG (pAppleEvent->descriptorType == typeAppleEvent))	return paramErr;
	if (!MoreAssertPCG (pAppleEvent->dataHandle))							return paramErr;

	anError = AEGetAttributePtr (pAppleEvent, keyAddressAttr, typeProcessSerialNumber, &actualType,
		(Ptr) &senderPSN, sizeof (senderPSN), &actualSize);
	if (MoreAssertPCG (noErr == anError))
	{
		if (!MoreAssertPCG (actualType == typeProcessSerialNumber))
			anError = paramErr;
		else if (!MoreAssertPCG (actualSize == sizeof (senderPSN)))
			anError = paramErr;
		else
		{
			ProcessInfoRec processInfo;

			if (!(anError = MoreProcGetProcessInformation (&senderPSN,&processInfo)))
			{
				*pIsFinder = (	processInfo.processSignature == kFinderProcessSignature && 
								processInfo.processType == kFinderProcessType);
			}
		}		
	}

	return anError;
}//end MoreAEIsSenderFinder
//*******************************************************************************
// This routine returns true if and only if pAEDesc is the "missing value" value.
pascal Boolean MoreAEIsMissingValue(const AEDesc* pAEDesc)
{
	DescType missing;
	
	return (pAEDesc->descriptorType == typeType)
			&& (AEGetDescDataSize(pAEDesc) == sizeof(missing))
			&& (AEGetDescData(pAEDesc, &missing, sizeof(missing)) == noErr)
			&& (missing == cMissingValue);
}//end MoreAEIsMissingValue

//*******************************************************************************
// This routine creates a descriptor that represents the missing value.
pascal OSStatus MoreAECreateMissingValue(AEDesc* pAEDesc)
{
	const static DescType missingValue = cMissingValue;
	
	return AECreateDesc(typeType, &missingValue, sizeof(missingValue), pAEDesc);
}//end MoreAECreateMissingValue
