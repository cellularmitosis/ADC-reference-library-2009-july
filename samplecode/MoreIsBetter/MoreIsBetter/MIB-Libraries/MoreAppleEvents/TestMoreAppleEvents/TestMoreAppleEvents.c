/*
	File:		TestMoreAppleEvents.c

	Contains:	Source for application to test MoreFinderEvents, MoreAppleEvents.

	DRI:		George Warner

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: TestMoreAppleEvents.c,v $
Revision 1.7  2002/11/08 23:01:21         
Convert nil to NULL.

Revision 1.6  2002/10/16 20:37:01        
Cleaned up & commented build macros.
Added test code for MoreFEGetObjCommentCFString.

Revision 1.5  2002/10/03 00:10:49        
Added PBX (framework) paths for includes.

"#defines TEST_MFE_*" added to allow switching testing focus.

SIOUX code disabled in __MACH__ builds.

Test code added for TEST_MFE_GETSELF, TEST_MFE_GETWINDOWS, TEST_MFE_GET_KIND.

More error reporting added.

Revision 1.4  2002/03/18 20:49:20        
changed "if ()... else if ()... else..." code to switch statement.
Added test code for MoreFEMakeObjectsVisible.

Revision 1.3  2002/03/13 19:10:10        
Added test code for MoreFEGetAliasAsObject.

Revision 1.2  2002/03/09 09:52:13         
A major update done by George Warner but checked in by me because he's having problems checking in.


*/
//==================================================================
//	includes
//==================================================================
#if defined(__MACH__)
	#include <Carbon/Carbon.h>

	#include <MoreAppleEvents/MoreAEObjects.h>
	#include <MoreAppleEvents/MoreFinderEvents.h>
	#include <MoreAppleEvents/MoreAppleEvents.h>
#else
	#include <Fonts.h>
	#include <Script.h>
	#include <stdio.h>
	#include <SIOUX.h>
	#include <AEHelpers.h>
	#include <TextUtils.h>
	#include <FinderRegistry.h>

	#include "MoreAEObjects.h"
	#include "MoreFinderEvents.h"
	#include "MoreAppleEvents.h"
#endif
//==================================================================
// compiler directives
//==================================================================
#ifdef FALSE
#undef FALSE
#endif
#define FALSE	0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE	!FALSE
//==================================================================
//These macros determine what objects we are going to use for our tests.
//------------------------------------------------------------------
#define TEST_MFE_GETSELF				FALSE
#define TEST_MFE_GETSELECTION 			TRUE
#define TEST_MFE_GETEVERYITEMONDESKTOP	FALSE
#define TEST_MFE_GETWINDOWS				FALSE

#if (!TEST_MFE_GETSELF && !TEST_MFE_GETSELECTION && !TEST_MFE_GETEVERYITEMONDESKTOP && !TEST_MFE_GETWINDOWS)
	#error "set only one of the above to true"
#endif
//==================================================================
//These macros determine what API's we are going to test.
//------------------------------------------------------------------
#define TEST_MFE_GET_KIND			FALSE
#define TEST_MFE_GET_COMMENT		TRUE
#define TEST_MFE_SET_COMMENT		TRUE
#define TEST_MFE_OPEN_INFO_WINDOW	FALSE
#define TEST_MFE_OPEN_FILE			FALSE
#define TEST_MFE_DUPLICATE			FALSE
#define TEST_MFE_MOVE				FALSE
#define TEST_MFE_GETALIASASOBJECT	FALSE
#define TEST_MFE_MAKEOBJECTSVISIBLE	FALSE
#define TEST_MFE_GETOBJECTPROPERTY	FALSE

#if (!TEST_MFE_GET_KIND && !TEST_MFE_GET_COMMENT && !TEST_MFE_SET_COMMENT && \
	 !TEST_MFE_OPEN_INFO_WINDOW && !TEST_MFE_OPEN_FILE && !TEST_MFE_DUPLICATE && \
	 !TEST_MFE_MOVE && !TEST_MFE_GETALIASASOBJECT && !TEST_MFE_MAKEOBJECTSVISIBLE && \
	 !TEST_MFE_GETOBJECTPROPERTY)
	#error "set one of the above to true"
#endif
//==================================================================
#define PRINT_APPLEEVENTS	 	TRUE	// set this true to AEPrint AppleEvents before sending them.
#define USE_CFSTRINGS			TRUE
//==================================================================
#if 1 || NOT_USED
static long CopyP2CStr(register unsigned char *pPStr,register char *pCStr)
{
	register short	i,len = *pPStr++;
	
	for (i = 1;i <= len; i++)
		*pCStr++ = *pPStr++;
	*pCStr = 0;
	return len;
}
//------------------------------------------------------------------
static long BuildPathVector(FSSpec *pFSSpecPtr,char *path)
{
	char* p = path;
	long len,retLen = 0;

	if (pFSSpecPtr->parID != fsRtParID)
	{
		FSSpec parentFSSpec; 

		if (FSMakeFSSpec(pFSSpecPtr->vRefNum,pFSSpecPtr->parID,"\p",&parentFSSpec) != noErr)
			return 0;
		len = BuildPathVector(&parentFSSpec,p);
		p += len;
		*p++ = ':';
		retLen += len + 1;
	}
	len = CopyP2CStr(pFSSpecPtr->name,p);
	p += len;
	retLen += len;

	*p = 0;	// terminate C string

	return retLen; 
}
//------------------------------------------------------------------
static long GetFullPath(FSSpec *pFSSpecPtr,char *path,OSStatus *pOSStatus)
{
	if (NULL != pOSStatus)
		*pOSStatus = noErr;
	return BuildPathVector(pFSSpecPtr,path);
}
#endif NOT_USED
//------------------------------------------------------------------
static pascal Boolean	MyAEIdleProc(EventRecord *pEventRecord, long *sleepTime, RgnHandle *mouseRgn)
{
	*sleepTime = 10;
	*mouseRgn = NULL;

#if !defined(__MACH__)
	if (pEventRecord != NULL)
	{
		switch (pEventRecord->what)
		{
			KeyDown:
				if (((pEventRecord->modifiers & cmdKey) != 0) &&
					((pEventRecord->message & charCodeMask) == '.'))
					return true;
		}
	}
	SIOUXHandleOneEvent(pEventRecord);
#endif !defined(__MACH__)
	return false;
}
//------------------------------------------------------------------
extern int dprintf(const char *format,  ...);
//------------------------------------------------------------------
int main(void)
{
	AEDescList	objectList = {typeNull,NULL};
	AEDesc		tAEDesc = {typeNull,NULL};
	AEIdleUPP	tAEIdleUPP = NewAEIdleUPP(MyAEIdleProc);
	OSStatus	anErr;

#if !defined(__MACH__)
	// Set the SIOUX window defaults
	SIOUXSettings.autocloseonquit	= FALSE;
	SIOUXSettings.asktosaveonclose	= FALSE;
	SIOUXSettings.showstatusline	= FALSE;
	SIOUXSettings.columns			= 132;
	SIOUXSettings.rows				= 24;
	SIOUXSettings.fontsize			= 10;
	GetFNum("\pMonaco",&SIOUXSettings.fontid);
	SIOUXSettings.standalone		= TRUE;
#endif !defined(__MACH__)

	printf("\nWelcome to TestMoreAppleEvents!\n");

#if TEST_MFE_GETSELF
	{
		ProcessSerialNumber tPSN = {0,kCurrentProcess};
		FSSpec tFSSpec;
		ProcessInfoRec tPIR;

		tPIR.processName = NULL;
		tPIR.processAppSpec = &tFSSpec;

		do {
			anErr = GetProcessInformation(&tPSN,&tPIR);
			if (noErr != anErr)
			{
				printf("\n•GetProcessInformation error: %ld.",anErr);
				break;
			}
			anErr = AECreateList(NULL,0,false,&objectList);
			if (noErr != anErr)
			{
				printf("\n•AECreateList error: %ld.",anErr);
				break;
			}
			anErr = MoreAEOCreateObjSpecifierFromFSSpec(&tFSSpec,&tAEDesc);
			if (noErr != anErr)
			{
				printf("\n•MoreAEOCreateObjSpecifierFromFSSpec error: %ld.",anErr);
				break;
			}
			anErr = AEPutDesc(&objectList,1,&tAEDesc);
			if (noErr != anErr)
			{
				printf("\n•AEPutDesc error: %ld.",anErr);
				break;
			}
		} while (false);
	}
	if (noErr == anErr)
#elif TEST_MFE_GETSELECTION
	anErr = MoreFEGetSelection(tAEIdleUPP,&objectList);
	if (noErr != anErr)
		printf("\n•MoreFEGetSelection error: %ld.",anErr);
	else
#elif TEST_MFE_GETEVERYITEMONDESKTOP
	anErr = MoreFEGetEveryItemOnDesktop(tAEIdleUPP,&objectList);
	if (noErr != anErr)
		printf("\n•MoreFEGetEveryItemOnDesktop error: %ld.",anErr);
	else
#endif

#if TEST_MFE_GETWINDOWS
	anErr = MoreFEGetWindows(&objectList,tAEIdleUPP);
	if (noErr != anErr)
		printf("\n•MoreFEGetWindows error: %ld.",anErr);
	else
	{
		long		numItems;

#if 0	// Set this true to printf the objectList
		Handle strHdl;

		anErr = AEPrintDescToHandle(&objectList,&strHdl);
		if (noErr == anErr)
		{
			char	nul	= '\0';

			PtrAndHand(&nul,strHdl,1);
			printf("\n•main(), objectList = \"%s\".",*strHdl); fflush(stdout);
			DisposeHandle(strHdl);
		}
#endif

		anErr = AECountItems (&objectList, &numItems);
		if (noErr == anErr)
		{
			long index;

			for (index = 1; index <= numItems; index++)
			{
				AEKeyword	keyword;

				anErr = AEGetNthDesc(&objectList,index,typeWildCard,&keyword,&tAEDesc);
				if (noErr != anErr)
				{
					printf("\n•AEGetNthDesc(typeWildCard) error: %ld.",anErr);
					continue;
				}
				else
				{
#if 0	// Set this true to printf each object in the objectList
					Handle strHdl;

					anErr = AEPrintDescToHandle(&tAEDesc,&strHdl);
					if (noErr == anErr)
					{
						char	nul	= '\0';

						PtrAndHand(&nul,strHdl,1);
						printf("\n•main(), object(%d of %d) = \"%s\".",index,numItems,*strHdl); fflush(stdout);
						DisposeHandle(strHdl);
					}
#endif

#if TEST_MFE_GETOBJECTPROPERTY
					AEDesc resultsAEDesc = {typeNull,NULL};

					anErr = MoreFEGetObjectProperty(&tAEDesc,keyAEBounds,&resultsAEDesc,tAEIdleUPP);
					if (noErr != anErr)
					{
						printf("\n•MoreFEGetObjectProperty(keyAEBounds) error: %ld.",anErr);
						continue;
					}
					else
					{
						if (typeQDRectangle == resultsAEDesc.descriptorType)
						{
							Rect	tRect;
							Size	intSize;

							MoreAEGetRawDataFromDescriptor(&resultsAEDesc,(Ptr)&tRect,sizeof(Rect),&intSize);
							printf("\n\tWindow[%ld] bounds = {t: %d, l: %d, b: %d, r: %d}.",
								index,tRect.top,tRect.left,tRect.bottom,tRect.right);
						}
						else
						{
							printf("\n•Unexpected resultsAEDesc.descriptorType: '%4.4s'.",(char*) &resultsAEDesc.descriptorType);
							continue;
						}
					}
#elif PRINT_APPLEEVENTS	// set this true to AEPrint the result
					Handle strHdl;

					anErr = AEPrintDescToHandle(&tAEDesc,&strHdl);
	if (noErr == anErr)
	{
						char	nul	= '\0';

						PtrAndHand(&nul,strHdl,1);
						printf("\n•MoreFEGetWindows:NthDesc(%ld) = \"%s\".",index,*strHdl); fflush(stdout);
						DisposeHandle(strHdl);
					}
#endif	PRINT_APPLEEVENTS
				}	// AEGetNthDesc err?
			}	// for (index = 1; index <= numItems; index++)
		}
		else
			printf("\n•AECountItems error: %ld.",anErr);

		(void) MoreAEDisposeDesc(&objectList);
	}
#else
	{
		long		numItems;

#if TEST_MFE_MAKEOBJECTSVISIBLE
		AEDescList	finderObjectList = {typeNull,NULL};

		anErr = AECreateList(NULL,0,false,&finderObjectList);
		if (noErr != anErr)
		{
			printf("\n•AECreateList error: %ld.",anErr);
			return 0;
		}
#endif TEST_MFE_MAKEOBJECTSVISIBLE

#if 0	// Set this true to printf the objectList
		Handle strHdl;

		anErr = AEPrintDescToHandle(&objectList,&strHdl);
		if (noErr == anErr)
		{
			char	nul	= '\0';

			PtrAndHand(&nul,strHdl,1);
			printf("\n•main(), objectList = \"%s\".",*strHdl); fflush(stdout);
			DisposeHandle(strHdl);
		}
#endif

		anErr = AECountItems (&objectList, &numItems);
		if (noErr == anErr)
		{
			long index;

			//printf("\nAECountItems good! (%ld)",numItems);

			for (index = 1; index <= numItems; index++)
			{
				Str255		tStr255;
				AliasHandle tAliasHandle = NULL;
				FSRef		tFSRef;
				FSSpec		tFSSpec;
				AEKeyword	keyword;

				anErr = AEGetNthDesc(&objectList,index,typeWildCard,&keyword,&tAEDesc);
				if (noErr != anErr)
				{
					printf("\n•AEGetNthDesc(typeWildCard) error: %ld.",anErr);
					continue;
				}
				else
				{
#if 1	// Set this true to printf each object in the objectList
			Handle strHdl;

			anErr = AEPrintDescToHandle(&tAEDesc,&strHdl);
			if (noErr == anErr)
			{
				char	nul	= '\0';

				PtrAndHand(&nul,strHdl,1);
				printf("\n•main(), object(%ld of %ld) = \"%s\".",index,numItems,*strHdl); fflush(stdout);
				DisposeHandle(strHdl);
			}
#endif

					switch (tAEDesc.descriptorType)
					{
					case typeAlias:
						anErr = MoreAECopyDescriptorDataToHandle(&tAEDesc, (Handle*) &tAliasHandle);
						if (noErr != anErr)
						{
							printf("\n•MoreAECopyDescriptorDataToHandle error: %ld.",anErr);
							continue;
						}
						// note: fall thru to FSResolveAliasWithMountFlags below
					case typeObjectSpecifier:
					if (typeObjectSpecifier == tAEDesc.descriptorType)
					{
						anErr = MoreFEGetObjectAsAlias(&tAEDesc,&tAliasHandle,tAEIdleUPP);
							if (noErr != anErr)
							{
								printf("\n•MoreFEGetObjectAsAlias error: %ld.",anErr);
								continue;
							}
						}

						{
							Boolean wasChanged;

							anErr = FSResolveAliasWithMountFlags(NULL,tAliasHandle,
										&tFSRef,&wasChanged,kResolveAliasFileNoUI);
							if (noErr != anErr)
							{
								printf("\n•ResolveAlias[WithMountFlags] error: %ld.",anErr);
								continue;
							}
						}
						break;
					default:
						printf("\ndescriptorType: '%4.4s'.",(char*) &tAEDesc.descriptorType);

						if (typeFSRef != tAEDesc.descriptorType)
						{
							anErr = AECoerceDesc(&tAEDesc,typeFSRef,&tAEDesc);
							if (noErr != anErr)
							{
								printf("\n•AECoerceDesc error: %ld.",anErr);
								continue;
							}
						}
						anErr = AEGetDescData(&tAEDesc,&tFSRef,sizeof(FSRef));
						if (noErr != anErr)
						{
							printf("\n•AEGetDescData error: %ld.",anErr);
							continue;
						}

						anErr = FSNewAlias( NULL, &tFSRef, &tAliasHandle);
						if (noErr != anErr)
						{
							printf("\n•FSNewAlias error: %ld.",anErr);
							continue;
						}
						break;
					}
				}

				// get the FSRef as a FSSpec
				anErr = FSGetCatalogInfo(&tFSRef,kFSCatInfoNone,NULL,NULL,&tFSSpec,NULL);
				if (noErr != anErr) continue;

				GetFullPath(&tFSSpec,(char *) &tStr255,&anErr);
				printf("\n	%ld of %ld : '%s'",index, numItems,tStr255);
#if TEST_MFE_GET_KIND
  #if USE_CFSTRINGS
				{
					CFStringRef tCFString = NULL;
					CFIndex length = 0;
					CFIndex dataSize = 0;

					anErr = MoreFEGetKindCFString(&tFSRef,&tCFString,tAEIdleUPP);
					if (noErr == anErr)
					{
						char* dataPtr = NULL;

					    if (NULL != tCFString)
					    {
					    	length = CFStringGetLength(tCFString);

							dataSize = sizeof(UniChar) * (length + 1);

							dataPtr = (char*) NewPtrClear(dataSize);

							if (NULL != dataPtr)
							{
								if (!CFStringGetCString(tCFString,dataPtr,dataSize,kCFStringEncodingASCII))
								{
									if (!CFStringGetCString(tCFString,dataPtr,dataSize,kCFStringEncodingUTF8))
									{
										DisposePtr(dataPtr);
										dataPtr = NULL;
										length = dataSize = 0;
									}
								}
							}
					    }

						if (length > 0)
							printf(" = {kind = \"%s\"}.",dataPtr);
						else
							printf(" = {kind = \"<none!>\"}.");
						if (NULL != dataPtr)
							DisposePtr(dataPtr);
					} else
						printf("\n•MoreFEGetKindCFString error: %ld.",anErr);
				}
  #else
				{
					Size actualSize;
					anErr = MoreFEGetKind(&tFSSpec,tStr255,tAEIdleUPP);
					actualSize = tStr255[0];
					CopyPascalStringToC(tStr255,(Ptr) tStr255);

					if (noErr == anErr)
					{
						if (actualSize > 0)
						{
							printf("{kind = \"%s\"}.",tStr255);
						}
						else
							printf("{kind = \"<none!>\"}.");
					} else
						printf("\n•MoreFEGetKind error: %ld.",anErr);
				}
  #endif USE_CFSTRINGS
#endif TEST_MFE_GET_KIND

#if TEST_MFE_GET_COMMENT
  #if USE_CFSTRINGS
				{
					CFStringRef tCFString = NULL;
					CFIndex length = 0;
					CFIndex dataSize = 0;
#if 0
					anErr = MoreFEGetObjCommentCFString(tAEDesc,&tCFString,tAEIdleUPP);
#else
					anErr = MoreFEGetCommentCFString(&tFSRef,&tCFString,tAEIdleUPP);
#endif
					if (noErr == anErr)
					{
						char* dataPtr = NULL;

					    if (NULL != tCFString)
					    {
					    	length = CFStringGetLength(tCFString);

							dataSize = sizeof(UniChar) * (length + 1);

							dataPtr = (char*) NewPtrClear(dataSize);

							if (NULL != dataPtr)
							{
								if (!CFStringGetCString(tCFString,dataPtr,dataSize,kCFStringEncodingASCII))
								{
									if (!CFStringGetCString(tCFString,dataPtr,dataSize,kCFStringEncodingUTF8))
									{
										DisposePtr(dataPtr);
										dataPtr = NULL;
										length = dataSize = 0;
									}
								}
							}
					    }

						if (length > 0)
							printf(" = \"%s\".",dataPtr);
						else
							printf(" = \"<no comment!>\".");
    #if TEST_MFE_SET_COMMENT
    					if (length > 0)
							anErr = MoreFESetCommentCFString(&tFSRef,CFSTR(""),tAEIdleUPP);
						else
							anErr = MoreFESetCommentCFString(&tFSRef,CFSTR("What the CF-ork?"),tAEIdleUPP);

						if (noErr == anErr)
							printf("\nMoreFESetComment good!");
						else
							printf("\n•MoreFESetComment error: %ld.",anErr);
    #endif TEST_MFE_SET_COMMENT
						if (NULL != dataPtr)
							DisposePtr(dataPtr);
					} else
						printf("\n•MoreFEGetComment error: %ld.",anErr);
				}
  #else
				{
					Size actualSize;
				anErr = MoreFEGetComment(&tFSSpec,tStr255,tAEIdleUPP);
					actualSize = tStr255[0];
					CopyPascalStringToC(tStr255,(Ptr) tStr255);

					if (noErr == anErr)
					{
						if (actualSize > 0)
						{
							printf(" = \"%s\".",tStr255);
						}
						else
							printf(" = \"<no comment!>\".");
    #if TEST_MFE_SET_COMMENT
					if (tStr255[0])
						anErr = MoreFESetComment(&tFSSpec,"\p",tAEIdleUPP);
					else
						anErr = MoreFESetComment(&tFSSpec,"\pWhat the fork?",tAEIdleUPP);

					if (noErr == anErr)
						printf("\nMoreFESetComment good!");
					else
							printf("\n•MoreFESetComment error: %ld.",anErr);
    #endif TEST_MFE_SET_COMMENT
					} else
						printf("\n•MoreFEGetComment error: %ld.",anErr);
				}
  #endif USE_CFSTRINGS
#endif TEST_MFE_GET_COMMENT

#if TEST_MFE_OPEN_INFO_WINDOW
				anErr = MoreFEOpenInfoWindow(&tFSSpec,tAEIdleUPP);
				if (noErr == anErr)
					printf("\nMoreFEOpenInfoWindow good!");
				else
					printf("\n•MoreFEOpenInfoWindow error: %ld.",anErr);
#endif TEST_MFE_OPEN_INFO_WINDOW

#if TEST_MFE_OPEN_FILE
				anErr = MoreFEOpenFile(&tFSSpec);
				if (noErr == anErr)
					printf("\nMoreFEOpenFile good!");
				else
					printf("\n•MoreFEOpenFile error: %ld.",anErr);
#endif TEST_MFE_OPEN_FILE

#if (TEST_MFE_DUPLICATE || TEST_MFE_MOVE)
				{
					FSSpec folderFSSpec = {0,0,"\p"};

					anErr = FSMakeFSSpec(0,0,"\p",&folderFSSpec);
					if (noErr != anErr)
						printf("\n•FSMakeFSSpec error: %ld.",anErr);
					else
					{
						AEDesc resultObjDesc = {typeNull,NULL};

#if TEST_MFE_DUPLICATE
						anErr = MoreFEDuplicate(&tFSSpec,&folderFSSpec,false,&resultObjDesc,tAEIdleUPP);
						if (noErr != anErr)
							printf("\n•MoreFEDuplicate error: %ld.",anErr);
#elif TEST_MFE_MOVE
						anErr = MoreFEMove(&tFSSpec,&folderFSSpec,false,&resultObjDesc,tAEIdleUPP);
						if (noErr != anErr)
							printf("\n•MoreFEMove error: %ld.",anErr);
#else
	#error "ether TEST_MFE_DUPLICATE or TEST_MFE_MOVE should be true."
#endif

#if PRINT_APPLEEVENTS	// set this true to AEPrint the result
						else
						{
							Handle strHdl;

							anErr = AEPrintDescToHandle(&resultObjDesc,&strHdl);
							if (noErr == anErr)
							{
								char	nul	= '\0';

								PtrAndHand(&nul,strHdl,1);
								printf("\n•MoreFEDuplicate:resultObjDesc=%s.",*strHdl); fflush(stdout);
								DisposeHandle(strHdl);
							}
}
#endif	PRINT_APPLEEVENTS
						
					}
				}
#endif (TEST_MFE_DUPLICATE || TEST_MFE_MOVE)

#if TEST_MFE_GETALIASASOBJECT
				if (NULL == tAliasHandle)
				{
					printf("\n•MoreFEGetAliasAsObject requires valid AliasHandle.");
					continue;
				}
				else
				{
					anErr = MoreFEGetAliasAsObject(tAliasHandle,&tAEDesc,tAEIdleUPP);
					if (noErr != anErr)
						printf("\n•MoreFEGetAliasAsObject error: %ld.",anErr);
#if PRINT_APPLEEVENTS	// set this true to AEPrint the result
					else
					{
						Handle strHdl;

						anErr = AEPrintDescToHandle(&tAEDesc,&strHdl);
						if (noErr == anErr)
						{
							char	nul	= '\0';

							PtrAndHand(&nul,strHdl,1);
							printf("\n•MoreFEGetAliasAsObject:tAEDesc=%s.",*strHdl); fflush(stdout);
							DisposeHandle(strHdl);
						}
					}
#endif	PRINT_APPLEEVENTS
				}
#endif TEST_MFE_GETALIASASOBJECT

#if TEST_MFE_MAKEOBJECTSVISIBLE
			anErr = MoreFEGetAliasAsObject(tAliasHandle,&tAEDesc,tAEIdleUPP);
			if (noErr != anErr)
					printf("\n•MoreFEGetAliasAsObject error: %ld.",anErr);
			else
			{
				anErr = AEPutDesc(&finderObjectList,index,&tAEDesc);
				if (noErr != anErr)
						printf("\n•AEPutDesc error: %ld.",anErr);
			}
#endif
			if (NULL != tAliasHandle)
				DisposeHandle((Handle) tAliasHandle);
			}	// for (index = 1; index <= numItems; index++)
			anErr = noErr;
		}
		else
			printf("\n•AECountItems error: %ld.",anErr);

#if TEST_MFE_MAKEOBJECTSVISIBLE
		anErr = MoreFEMakeObjectsVisible(&finderObjectList,tAEIdleUPP);
		if (noErr != anErr)
			printf("\n•MoreFEMakeObjectsVisible error: %ld.",anErr);
#endif

		(void) MoreAEDisposeDesc(&objectList);
	}
#endif
	DisposeAEIdleUPP(tAEIdleUPP);

	return 0;
}