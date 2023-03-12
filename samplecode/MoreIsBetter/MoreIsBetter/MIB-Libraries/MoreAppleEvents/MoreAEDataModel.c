/*
	File:		MoreAEDataModel.c

	Contains:	Helpers for AEDataModel stuff.

	DRI:		Quinn

	Copyright:	Copyright (c) 2000-2002 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreAEDataModel.c,v $
Revision 1.2  2003/04/16 12:57:38         
Implicit arithmetic conversions compatibility.

Revision 1.1  2002/11/08 22:46:02         
A new module split off from MoreAppleEvents. This is necessary because MoreAE is dependent on Carbon, whereas this stuff in only dependent on the ApplicationServices framework.


*/

//*******************************************************************************
//**********	Our Prototypes 	    	****************************************
#include "MoreAEDataModel.h"

//**********	Universal Headers		****************************************
#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacErrors.h>
	#include <MacMemory.h>
	#include <AERegistry.h>
#endif

//**********	MoreIsBetter Headers	****************************************
#include "MoreMemory.h"

//*******************************************************************************
#pragma mark ==> AEDesc Constructor & Destructor •
//*******************************************************************************
// Initialises desc to the null descriptor (typeNull, NULL).
pascal void MoreAENullDesc(AEDesc* desc)
{
	assert(desc != NULL);

	desc->descriptorType = typeNull;
	desc->dataHandle     = NULL;
}//end MoreAENullDesc

//*******************************************************************************
// Disposes of desc and initialises it to the null descriptor.
pascal void MoreAEDisposeDesc(AEDesc* desc)
{
	OSStatus junk;

	assert(desc != NULL);
	
	junk = AEDisposeDesc(desc);
	assert(junk == noErr);

	MoreAENullDesc(desc);
}//end MoreAEDisposeDesc

//*******************************************************************************
#pragma mark ==> AEDesc Data Accessors for 68K •
//*******************************************************************************
// These routines are only implemented in PreCarbon.o for PowerPC
// So for 68K we had to write our own versions

#if (!ACCESSOR_CALLS_ARE_FUNCTIONS && TARGET_CPU_68K)

pascal Size AEGetDescDataSize(	const AEDesc* pAEDesc)
{
	return GetHandleSize(pAEDesc->dataHandle);
}//end AEGetDescDataSize

pascal OSErr AEGetDescData(	const AEDesc*	pAEDesc,
				void*			pDataPtr,
				Size 			pMaxSize)
{
	Size copySize = AEGetDescDataSize(pAEDesc);

	if ((NULL == pAEDesc) || (NULL == pDataPtr))
		return paramErr;

	if (pMaxSize < copySize)
		copySize = pMaxSize;

	BlockMoveData(*pAEDesc->dataHandle,pDataPtr,copySize);

	return noErr;
}//end AEGetDescData
#endif (!ACCESSOR_CALLS_ARE_FUNCTIONS && TARGET_CPU_68K)

//*******************************************************************************
#pragma mark ==> Get Data From Descriptors •

/********************************************************************************
	This is the generic routine that all the other's use instead of
	duplicating all this code unnecessarily.

	pAEDesc			==>		The descriptor we want the data from
	pDestPtr		<==		Where we want to store the data from this desc.
	pMaxSize		==>		The maxium amount of data we can store.
	pActualSize		<==		The actual amount of data stored.

	RESULT CODES
	____________
	noErr			   0	No error	
	____________
*/
pascal void MoreAEGetRawDataFromDescriptor(
					const AEDesc* pAEDesc,
							 Ptr     pDestPtr,
							 Size    pMaxSize,
					Size*	pActualSize)
{
	Size copySize;

	if (pAEDesc->dataHandle) 
	{
		*pActualSize = AEGetDescDataSize(pAEDesc);

		if (*pActualSize < pMaxSize)
			copySize = *pActualSize;
		else
			copySize = pMaxSize;

		 AEGetDescData(pAEDesc,pDestPtr,copySize);
	}
	else
		*pActualSize = 0;
}//end MoreAEGetRawDataFromDescriptor

/********************************************************************************
	Extract a CFString from a descriptor.

  pAEDesc		==>		The descriptor we want the data from
  pCFStringRef	<==		Where we want to store the pascal string

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/

#if TARGET_API_MAC_CARBON

pascal OSErr MoreAEGetCFStringFromDescriptor(const AEDesc* pAEDesc, CFStringRef* pCFStringRef)
{
	AEDesc		uniAEDesc = {typeNull, NULL};
	OSErr		anErr;

	if (NULL == pCFStringRef)
		return paramErr;

	anErr = AECoerceDesc(pAEDesc, typeUnicodeText, &uniAEDesc);
	if (noErr == anErr)
	{
		if (typeUnicodeText == uniAEDesc.descriptorType)
		{
	        Size bufSize = AEGetDescDataSize(&uniAEDesc);
	        Ptr buf = NewPtr(bufSize);

	        if ((noErr == (anErr = MemError())) && (NULL != buf))
	        {
	            anErr = AEGetDescData(&uniAEDesc, buf, bufSize);
	            if (noErr == anErr)
	                *pCFStringRef = CFStringCreateWithCharacters(kCFAllocatorDefault, (UniChar*) buf, bufSize / (Size) sizeof(UniChar));

	            DisposePtr(buf);
	        }
		}
		MoreAEDisposeDesc(&uniAEDesc);
	}
	return (anErr);
}//end MoreAEGetCFStringFromDescriptor

#endif

/********************************************************************************
	Extract a pascal string a descriptor.

  pAEDesc		==>		The descriptor we want the data from
  pStringPtr	<==		Where we want to store the pascal string

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/
pascal OSErr MoreAEGetPStringFromDescriptor(const AEDesc* pAEDesc, StringPtr pStringPtr)
{
	Size         stringSize;
	AEDesc       resultDesc = {typeNull, NULL};
	OSErr        anErr;
	
	anErr = AECoerceDesc(pAEDesc, typeChar, &resultDesc);
	if (noErr != anErr) goto done;
	
	pStringPtr[0] = 0;
	
	MoreAEGetRawDataFromDescriptor(&resultDesc,(Ptr) &pStringPtr[1],sizeof(Str255),&stringSize);
	if (stringSize <= 255) 
		pStringPtr[0] = (UInt8) stringSize;
	else
		anErr = errAECoercionFail;

done:		
	MoreAEDisposeDesc(&resultDesc);
		
	return(anErr);
}//end MoreAEGetPStringFromDescriptor

/********************************************************************************
	Extract a C string from the descriptor.

  pAEDesc		==>		The descriptor we want the data from
  pStringPtr	<==		Where we want to store the C string

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/
pascal OSErr MoreAEGetCStringFromDescriptor(const AEDesc* pAEDesc, char* pCharPtr)
{
	Size         stringSize;
	AEDesc       resultDesc = {typeNull, NULL};
	OSErr        anErr;
	
	anErr = AECoerceDesc(pAEDesc, typeChar, &resultDesc);
	if (noErr == anErr)
	{
		MoreAEGetRawDataFromDescriptor(&resultDesc, pCharPtr, 1024, &stringSize);
		pCharPtr[stringSize] = 0;
	}
	MoreAEDisposeDesc(&resultDesc);
		
	return (anErr);
}//end MoreAEGetCStringFromDescriptor

/********************************************************************************
	Extract a short from a descriptor.

  pAEDesc		==>		The descriptor we want the data from
  pStringPtr	<==		Where we want to store the short.

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/

pascal OSErr MoreAEGetShortFromDescriptor(const AEDesc* pAEDesc, SInt16* pResult)
{
	OSErr   myErr;
	Size    intSize;
	AEDesc  resultDesc;

	*pResult = 0;

	myErr = AECoerceDesc(pAEDesc,typeShortInteger,&resultDesc);

	if (myErr==noErr) 
	{
		MoreAEGetRawDataFromDescriptor(&resultDesc,(Ptr) pResult,sizeof(SInt16),&intSize);
		if (intSize>2) 
			myErr = errAECoercionFail;
	}

	MoreAEDisposeDesc(&resultDesc);

	return(myErr);
}//end MoreAEGetShortFromDescriptor

/********************************************************************************
	Extract a Boolean from a descriptor.

  pAEDesc		==>		The descriptor we want the data from
  pStringPtr	<==		Where we want to store the boolean.

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/

pascal OSErr MoreAEGetBooleanFromDescriptor(const AEDesc* pAEDesc,Boolean* pResult)
{
	AEDesc  resultDesc;
	Size    boolSize;
	OSErr   myErr;

	*pResult = false;
	myErr = AECoerceDesc(pAEDesc,typeBoolean,&resultDesc);

	if (myErr==noErr) 
	{
		MoreAEGetRawDataFromDescriptor(&resultDesc,(Ptr)pResult,
			sizeof(Boolean),&boolSize);
		if ( ((ByteCount) boolSize) > sizeof(Boolean)) 
			myErr = errAECoercionFail;
	}

	MoreAEDisposeDesc(&resultDesc);

	return(myErr);
}//end MoreAEGetBooleanFromDescriptor

/********************************************************************************
	Extract a long from a descriptor.

  pAEDesc		==>		The descriptor we want the data from
  pStringPtr	<==		Where we want to store the long.

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/

pascal OSErr MoreAEGetLongFromDescriptor(const AEDesc* pAEDesc, long* pResult)
{
	OSErr   myErr;
	Size    intSize;
	AEDesc  resultDesc;

	*pResult = 0;
	myErr = AECoerceDesc(pAEDesc,typeLongInteger,&resultDesc);

	if (myErr==noErr) 
	{
		MoreAEGetRawDataFromDescriptor(&resultDesc,(Ptr)pResult,sizeof(long),&intSize);
		if (intSize>4) 
			myErr = errAECoercionFail;
	}

	MoreAEDisposeDesc(&resultDesc);

	return(myErr);
} /*MoreAEGetLongIntFromDescriptor*/

/********************************************************************************
	Extract a OSType from a descriptor.

  pAEDesc		==>		The descriptor we want the data from
  pResult		<==		Where we want to store the OSType.

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/

pascal OSErr MoreAEGetOSTypeFromDescriptor(const AEDesc* pAEDesc,OSType* pResult)
{
	return (MoreAEGetLongFromDescriptor(pAEDesc,(long*) pResult));
}//end MoreAEGetOSTypeFromDescriptor

/********************************************************************************
	Creates an AEDesc from a CFString.

  pCFStringRef			==>			The CFString we want the data from
  pAEDesc		<==		Where we want to store the AEDesc

  RESULT CODES
  ____________
  noErr			   0	No error	
  ____________
*/

#if TARGET_API_MAC_CARBON

pascal OSErr MoreAECreateAEDescFromCFString(const CFStringRef pCFStringRef,AEDesc* pAEDesc)
{
	OSErr		anErr = paramErr;

	if ((NULL != pCFStringRef) && (NULL != pAEDesc))
	{
		CFIndex length = CFStringGetLength(pCFStringRef);
		Size	dataSize = length * (Size) sizeof(UniChar);
		UniChar* data = (UniChar*) NewPtr(dataSize);

		if (NULL == data)
		{
			anErr = MemError();
			if (noErr == anErr)
				anErr = memFullErr;
		}
		else
		{
			CFStringGetCharacters(pCFStringRef,CFRangeMake(0,length),data);
			anErr = AECreateDesc(typeUnicodeText,data,dataSize,pAEDesc);
			DisposePtr((Ptr) data);
		}
	}
	return (anErr);
}//end MoreAECreateAEDescFromCFString

#endif

//*******************************************************************************
// This routine creates a new handle and puts the contents of the desc
// in that handle.  Carbon’s opaque AEDesc’s means that we need this
// functionality a lot.
pascal OSStatus MoreAECopyDescriptorDataToHandle(const AEDesc* pAEDesc, Handle* pDescData)
{
	OSStatus anErr;
	OSStatus junk;
	
	assert(pAEDesc   != NULL);
	assert(pDescData  != NULL);
	assert(*pDescData == NULL);
	
	*pDescData = NewHandle(AEGetDescDataSize(pAEDesc));
	anErr = MoreMemError(*pDescData);
	if (noErr == anErr) {
		HLock(*pDescData);
		junk = AEGetDescData(pAEDesc, **pDescData, AEGetDescDataSize(pAEDesc));
		assert(junk == noErr);
		HUnlock(*pDescData);
	}
	return anErr;
}//end MoreAECopyDescriptorDataToHandle

#if 0
 //*******************************************************************************
// This routine prints a descriptor
pascal int MoreAEfprintDesc(void* pFILE, const AEDesc * pAEDesc)
{
	int result = 0;
	Handle strHdl;
	OSErr anErr = AEPrintDescToHandle(pAEDesc,&strHdl);

	if (noErr == anErr)
	{
		char	nul	= '\0';

		PtrAndHand(&nul,strHdl,1);
		result = fprintf(pFILE,"%s",*strHdl);
		fflush(pFILE);
		DisposeHandle(strHdl);
	}
	return result;
}
#endif