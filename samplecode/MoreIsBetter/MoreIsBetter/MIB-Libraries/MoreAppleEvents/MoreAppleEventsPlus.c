/*
	File:		MoreAppleEventsPlus.c

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

$Log: MoreAppleEventsPlus.c,v $
Revision 1.16  2002/11/08 22:55:37         
Convert nil to NULL. Use MoreAEDataModel. Include our header early to flush out any missing dependencies.

Revision 1.15  2002/10/16 20:33:56        
"	File:		MoreAppleEventsPlus.h" should be <*.c>

Revision 1.14  2002/02/19 18:56:15        
Written by: => DRI:
AEDisposeDesc => MoreAEDisposeDesc

Revision 1.13  2002/01/16 19:12:51        
Dummy - Line added to test CVS

Revision 1.12  2001/11/07 15:50:57         
Tidy up headers, add CVS logs, update copyright.


        <11>     21/9/01    Quinn   Get rid of wacky Finder label.
        <10>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <9>     9/13/01    gaw     db corrupt?
         <8>     9/13/01    gaw     fix MoreAEAccessW[I]ldFromWild typo
         <7>     9/13/01    gaw     remove "extern" from MoreAEAccessWildFromWild
         <6>     8/28/01    gaw     Changed pRefcon's to SInt32's
         <5>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <4>      3/9/00    gaw     Fixed OpaqueAEDataStorage.dataHandle references in
                                    MoreAEAddPropertyToList for CARBON
         <3>      3/9/00    gaw     Fix AEDesc.dataHandle references
         <2>      3/9/00    gaw     API changes for MoreAppleEvents
*/

//********************************************************************************
//**********	Our Prototypes			****************************************
#include "MoreAppleEventsPlus.h"
//**********	Universal Headers		****************************************
#if ! MORE_FRAMEWORK_INCLUDES
	#include <AERegistry.h>
	#include <AEObjects.h>
	#include <AEPackObject.h>
	#include <FinderRegistry.h>
	#include <Gestalt.h>
#endif
//**********	Project Headers			****************************************
#include "MoreProcesses.h"
#include "MoreAppleEvents.h"
#include "MoreAEDataModel.h"
#include "MoreAEObjects.h"


//**********	Private Definitions		****************************************
static MoreAEHandle_GetDataProcPtr gMoreAEHandle_GetDataProcPtr;
static MoreAEHandle_SetDataProcPtr gMoreAEHandle_SetDataProcPtr;
//*******************************************************************************
#pragma mark AEHandler & AEAccessor data records �
//*******************************************************************************

static MoreAEHandlerRec gMoreAEHandlerRecs[] =
{
	{kAECoreSuite, kAEGetData, MoreAEHandleGetDataAEvent, NULL, 0L},
	{kAECoreSuite, kAESetData, MoreAEHandleSetDataAEvent, NULL, 0L},
	{typeWildCard, typeWildCard, MoreAEHandleWildAEvent, NULL, 0L}
};

#define kNumberofAEHandlerRecs ((SInt32) sizeof(gMoreAEHandlerRecs) / sizeof(MoreAEHandlerRec))

static MoreAEAccessorRec gMoreAEAccessorRecs[] =
{
	{typeWildCard, typeWildCard, MoreAEAccessWildFromWild, NULL, 0L}
};

#define kNumberofAEAccessorRecs ((SInt32) sizeof(gMoreAEAccessorRecs) / sizeof(MoreAEAccessorRec))
/********************************************************************************
	Initialize AEHP
*/
pascal OSErr MoreAEInitialize(
	const MoreAEHandle_GetDataProcPtr pMoreAEHandle_GetDataProcPtr,
	const MoreAEHandle_SetDataProcPtr pMoreAEHandle_SetDataProcPtr
)
{
	gMoreAEHandle_GetDataProcPtr = pMoreAEHandle_GetDataProcPtr;
	gMoreAEHandle_SetDataProcPtr = pMoreAEHandle_SetDataProcPtr;

	MoreAEInstallAEHandlers(gMoreAEHandlerRecs, kNumberofAEHandlerRecs);
	MoreAEInstallAEAccessors(gMoreAEAccessorRecs, kNumberofAEAccessorRecs);

	return noErr;
}
//*******************************************************************************
#pragma mark ==> AEhandlers �
//*******************************************************************************
// This routine installs all the AppleEvent handlers used by this application.
pascal void MoreAEInstallAEHandlers(MoreAEHandlerRec* pMoreAEHandlerRecs,const SInt32 pCount)
{
	SInt32	index;
	OSErr	anErr;

	for (index = 0;index < pCount;index++)
	{
		// first we see if there is already an accessor installed by
		// SimpleApp. (remember it's UPP & refCom so we can call thru)
		anErr = AEGetEventHandler(
			pMoreAEHandlerRecs[index].fAEEventClass,
			pMoreAEHandlerRecs[index].fAEEventID, 
			&pMoreAEHandlerRecs[index].fOldAEEventHandlerUPP, 
			&pMoreAEHandlerRecs[index].fOldRefCon, 
			false);

		if (noErr != anErr)
			pMoreAEHandlerRecs[index].fOldAEEventHandlerUPP = NULL;
		else
		{
			// remove the old handler
			anErr = AERemoveEventHandler(
				pMoreAEHandlerRecs[index].fAEEventClass,
				pMoreAEHandlerRecs[index].fAEEventID, 
				pMoreAEHandlerRecs[index].fOldAEEventHandlerUPP, 
				false);
		}

		// now install ours
		anErr = AEInstallEventHandler(
			pMoreAEHandlerRecs[index].fAEEventClass,
			pMoreAEHandlerRecs[index].fAEEventID, 
			NewAEEventHandlerUPP(pMoreAEHandlerRecs[index].fAEEventHandlerProcPtr),
			index, false);
	}
}
//*******************************************************************************
// this is how we call thru to any previously installed AEHandler.
pascal OSErr MoreAECallPreviousAEHandler(
						const MoreAEHandlerRec* pMoreAEHandlerRecs,
						const AEEventClass	pAEEventClass,
						const AEEventID		pAEEventID,
						const AppleEvent	*pAppleEvent,
						AppleEvent 			*pAEReply,
						SInt32 				pRefcon)
{
	SInt32	index = pRefcon, count = kNumberofAEHandlerRecs;
	OSErr	anErr = errAEEventNotHandled;

	// we use the pRefcon as an index into our AEHandler table
	if ((index >= 0) && (index < count))
	{
		// if the old accessor wasn't NULL...
		if (NULL != pMoreAEHandlerRecs[index].fOldAEEventHandlerUPP)
		{
			// and the AEvent class & ID's match...
			if ((pMoreAEHandlerRecs[index].fAEEventClass == pAEEventClass) &&
				(pMoreAEHandlerRecs[index].fAEEventID == pAEEventID))
			{
				// then let's try it!
				anErr = InvokeAEEventHandlerUPP(
							pAppleEvent,pAEReply,
							pMoreAEHandlerRecs[index].fOldRefCon,
							pMoreAEHandlerRecs[index].fOldAEEventHandlerUPP);
			}
		}
	}
	return (anErr);
}
//*******************************************************************************
#pragma mark ==> AEAccessors �
//*******************************************************************************
// This routine installs all the object accessors used by this application.
pascal void MoreAEInstallAEAccessors(MoreAEAccessorRec* pMoreAEAccessorRecs,const SInt32 pCount)
{
	SInt32	index;
	OSErr anErr;

	anErr = AEObjectInit();
	if (noErr != anErr)
		return;

	for (index = 0;index < pCount;index++)
	{
		// first we see if there is already an accessor installed by
		// SimpleApp. (remember it's UPP & refCom so we can call thru)
		anErr = AEGetObjectAccessor(
			pMoreAEAccessorRecs[index].fDesiredClass,
			pMoreAEAccessorRecs[index].fContainerType, 
			&pMoreAEAccessorRecs[index].fOldOSLAccessorUPP, 
			&pMoreAEAccessorRecs[index].fOldRefCon, 
			false);

		if (noErr != anErr)
			pMoreAEAccessorRecs[index].fOldOSLAccessorUPP = NULL;
		else
		{
			// remove it
			anErr = AERemoveObjectAccessor(
				pMoreAEAccessorRecs[index].fDesiredClass,
				pMoreAEAccessorRecs[index].fContainerType, 
				pMoreAEAccessorRecs[index].fOldOSLAccessorUPP, 
				false);
		}

		// now install ours
		anErr = AEInstallObjectAccessor(
			pMoreAEAccessorRecs[index].fDesiredClass,
			pMoreAEAccessorRecs[index].fContainerType, 
			NewOSLAccessorUPP(pMoreAEAccessorRecs[index].fOSLAccessorProcPtr),
			index, false);
	}
}
//*******************************************************************************
// This is so can try to call a pre-existing accessor (from SimpleAppLib)
pascal OSErr MoreAECallPreviousOSLAccessor(
									const MoreAEAccessorRec* pMoreAEAccessorRecs,
									DescType pDesiredClass, 
									const AEDesc* pContainerToken, 
									DescType pContainerClass, 
									DescType pKeyForm, 
									const AEDesc* pKeyData, 
									AEDesc* pToken, 
									SInt32 pRefcon)
{
	SInt32	index = pRefcon, count = kNumberofAEAccessorRecs;
	OSErr	anErr = errAENoSuchObject;

	// we use the pRefcon as an index into our AEHandler table
	if ((index >= 0) && (index < count))
	{
		// if the old accessor wasn't NULL...
		if (NULL != pMoreAEAccessorRecs[index].fOldOSLAccessorUPP)
		{
			// and the class & container types match
			if ((pMoreAEAccessorRecs[index].fDesiredClass == pDesiredClass) &&
				(pMoreAEAccessorRecs[index].fContainerType == pDesiredClass))
			{
				// then let's try it!
				anErr = InvokeOSLAccessorUPP(
							pDesiredClass, 
							pContainerToken, 
							pContainerClass, 
							pKeyForm, 
							pKeyData, 
							pToken, 
							pMoreAEAccessorRecs[index].fOldRefCon,
							pMoreAEAccessorRecs[index].fOldOSLAccessorUPP);
			}
		}
	}
	return (anErr);
}

//*******************************************************************************
#pragma mark ==> My handers and accessors �
//*******************************************************************************
// This is a wild card accessor that get's called if we don't have a
// more specific accessor installed. (It's great for debugging!)
pascal OSErr MoreAEAccessWildFromWild(
									DescType pDesiredClass, 
									const AEDesc* pContainerToken, 
									DescType pContainerClass, 
									DescType pKeyForm, 
									const AEDesc* pKeyData, 
									AEDesc* pToken, 
									SInt32 pRefcon)
{
	DescType		requestedProperty;
	OSErr			anErr;

	// try to call the previous accessor
	anErr = MoreAECallPreviousOSLAccessor(
		gMoreAEAccessorRecs, pDesiredClass, pContainerToken, 
		pContainerClass, pKeyForm, pKeyData, pToken, pRefcon);
	if (noErr == anErr)	// if it was good
		return (anErr);	// return noErr

	anErr = AEGetDescData(pKeyData,&requestedProperty,sizeof(DescType));
	anErr = errAEEventNotHandled;

// so the handler we want to install would be:
// AEInstallObjectAccessor( 
//		pDesiredClass,pContainerClass,
//		NewOSLAccessorProc(access_<pDesiredClass>From<pContainerClass>), 0, false);

	// Now dump Apple event info
#if 0
	if (formPropertyID == pKeyForm)
		dprintf("|MoreAEAccessWildFromWild('%.4s' (= '%.4s'),'%.4s')-I-Debug;",
			&pKeyForm, &requestedProperty, &pContainerClass);
	else
		dprintf("|MoreAEAccessWildFromWild('%.4s','%.4s')-I-Debug;",
			&pKeyForm, &pContainerClass);
#endif 0
	return anErr;
}

//*******************************************************************************
// this is our get data AppleEvent handler.
pascal OSErr MoreAEHandleGetDataAEvent(
						const AppleEvent *pAppleEvent,
						AppleEvent *pAEReply,
						SInt32 pRefcon)
{
#pragma unused (pRefcon)

	AEDesc		directObj = {typeNull, NULL};
	AEDesc		result = {typeNull, NULL};
	Size		actualSize;
	DescType	returnedType;
	DescType	reqType;
	OSErr		reqTypeErr,
				anErr;
	
	// extract the direct object, which is the object whose data is to be returned
	anErr = AEGetParamDesc(pAppleEvent, keyDirectObject, typeWildCard, &directObj);
	if (noErr != anErr) goto done;
		
	// now the get the type of data wanted - optional (ignore any error)
	reqTypeErr = AEGetParamPtr(pAppleEvent, keyAERequestedType, typeType, 
					&returnedType, (Ptr)&reqType, sizeof(reqType), &actualSize);

	// see if they're any parameters we missed
	anErr = MoreAEGotRequiredParams(pAppleEvent);
	if (noErr != anErr) goto done;
	
	// get the actual data from the direct object
	if (NULL != gMoreAEHandle_GetDataProcPtr)
		anErr = (*gMoreAEHandle_GetDataProcPtr)(&directObj, &result);
	else
		anErr = errAEEventNotHandled;

	if (noErr == anErr)	// add the data to the reply
		anErr = MoreAEAddResultToReply(&result, pAEReply, anErr);

done:				
	if (directObj.dataHandle)
		  MoreAEDisposeDesc(&directObj);
	if (result.dataHandle)
		  MoreAEDisposeDesc(&result);
		
	return (anErr);
}

//*******************************************************************************
// this is our set data AppleEvent handler.
pascal OSErr MoreAEHandleSetDataAEvent(
						const AppleEvent *pAppleEvent,
						AppleEvent *pAEReply,
						SInt32 pRefcon)
{
#pragma unused (pAEReply, pRefcon)
	
	AEDesc 	directObj = {typeNull, NULL};
	AEDesc 	dataDesc = {typeNull, NULL};
	OSErr  	anErr;
			
		// pick up the direct object, which is the object whose data is to be set
	anErr = AEGetParamDesc(pAppleEvent,  keyDirectObject,
									typeWildCard, &directObj);
	if (noErr != anErr) goto done;
		
		// now the data to set it to - typeWildCard means get as is
		// e.g. this is the name of the font for text
	anErr = AEGetParamDesc(pAppleEvent, keyAEData, typeWildCard, &dataDesc);
	if (noErr != anErr) goto done;
	
		// missing any parameters?
	anErr = MoreAEGotRequiredParams(pAppleEvent);
	if (noErr != anErr) goto done;
	
		// set the data
	if (NULL != gMoreAEHandle_SetDataProcPtr)
		anErr = (*gMoreAEHandle_SetDataProcPtr)(&directObj, &dataDesc);
	else
		anErr = errAEEventNotHandled;

done:			
	if (directObj.dataHandle)
		MoreAEDisposeDesc(&directObj);
	if (dataDesc.dataHandle)
		MoreAEDisposeDesc(&dataDesc);

	return (anErr);
}

//*******************************************************************************
// This is a typeWild event hander that will catch any AppleEvents
// that we haven't installed handers for. (Great for debugging!)
pascal OSErr MoreAEHandleWildAEvent(
						const AppleEvent *pAppleEvent,
						AppleEvent *pAEReply,
						SInt32 pRefcon)
{
	AEEventClass	tAEEventClass;
	AEEventID		tAEEventID;
	OSType			actualType;	// returned from AEGetAttributePtr() (not used)
	Size			actualSize;	// returned from AEGetAttributePtr() (not used)
	OSErr 			anErr;

	// Determine this AppleEvents Class and ID.
	anErr = AEGetAttributePtr(pAppleEvent, keyEventClassAttr, typeType, 
				&actualType, &tAEEventClass, sizeof(AEEventClass), &actualSize);
	if (noErr != anErr) tAEEventClass = typeWildCard;

	// Determine this AppleEvents Class and ID.
	anErr = AEGetAttributePtr(pAppleEvent, keyEventIDAttr, typeType, 
				&actualType, &tAEEventID, sizeof(AEEventID), &actualSize);
	if (noErr != anErr) tAEEventID = typeWildCard;

	// Try to call thru to any previously installed AEHandler.
	anErr = MoreAECallPreviousAEHandler(
		gMoreAEHandlerRecs, tAEEventClass, tAEEventID,pAppleEvent,pAEReply,pRefcon);
	if (noErr == anErr)
		return anErr;

#if 0
	// This dumps the AppleEvent info to MacsBug.
	dprintf("|MoreAEHandleWildAEvent('%.4s','%.4s')-I-Debug;aevt %8.8X;",
		&tAEEventClass, &tAEEventID,pAppleEvent);
#endif 0

	return errAEEventNotHandled;
}
// ----------------------------------------------------------------------
//	Name:		MoreAEAddPropertyToList
//	Function:	Append this property (& it's data) to a list
// ----------------------------------------------------------------------
pascal OSErr MoreAEAddPropertyToList(const DescType pPropDescType, AEDesc* pListAEDesc)
{
	AEDesc	objTokenDesc = {typeNull, NULL};
	AEDesc  itemDesc = {typeNull, NULL};
	OSErr	anErr = noErr;

	objTokenDesc.descriptorType = pPropDescType;

	if (NULL != gMoreAEHandle_GetDataProcPtr)
		anErr = (*gMoreAEHandle_GetDataProcPtr)(&objTokenDesc, &itemDesc);
	else
		anErr = errAEEventNotHandled;

	if (noErr == anErr)
	{
		Size tSize = AEGetDescDataSize(&itemDesc);
		Ptr tPtr = NewPtr(tSize);

		if (NULL == tPtr)
			anErr = MemError();
		else
		{
			anErr = AEGetDescData(&itemDesc,tPtr,tSize);
			if (noErr == anErr)
				anErr = AEPutKeyPtr(pListAEDesc, pPropDescType,
							itemDesc.descriptorType,tPtr,tSize);
			DisposePtr(tPtr);
		}
	}

	if (itemDesc.dataHandle)
		MoreAEDisposeDesc(&itemDesc);

	return (anErr);
}
