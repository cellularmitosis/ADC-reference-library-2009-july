/*
	File:		HelloWorld.c

	Contains:	Source for application to test MoreFinderEvents, MoreAppleEvents.

	Written by:	George Warner

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
*/

#include <Fonts.h>
#include <Script.h>

#include <stdio.h>
#include <SIOUX.h>

#include "MoreFinderEvents.h"

#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif

#define TEST_MOREFEGETSELECTION TRUE
#define TEST_MOREFEGETEVERYITEMONDESKTOP	FALSE

#if (!TEST_MOREFEGETSELECTION && !TEST_MOREFEGETEVERYITEMONDESKTOP)
	#error set one of the above to true
#endif

#define TEST_GET_COMMENT		FALSE
#define TEST_SET_COMMENT		FALSE
#define TEST_OPEN_INFO_WINDOW	FALSE
#define TEST_MFE_OPEN_FILE		TRUE

#if (!TEST_GET_COMMENT && !TEST_SET_COMMENT && !TEST_OPEN_INFO_WINDOW && !TEST_MFE_OPEN_FILE)
	#error set one of the above to true
#endif

//------------------------------------------------------------------
static OSErr ObjectToFSSpec(AEDesc* pAEDesc,FSSpec* pFSSpecPtr)
{
	if (!pAEDesc || !pFSSpecPtr) return paramErr;
	return -1;
}
//------------------------------------------------------------------
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
static long GetFullPath(FSSpec *pFSSpecPtr,char *path,OSErr *pOSErr)
{
	if (NULL != pOSErr)
		*pOSErr = noErr;
	return BuildPathVector(pFSSpecPtr,path);
}
//------------------------------------------------------------------
static pascal Boolean	MyAEIdleProc(EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn)
{
	*sleepTime = 10;
	*mouseRgn = NULL;

	if (theEvent != NULL)
	{
		switch (theEvent->what)
		{
			KeyDown:
				if (((theEvent->modifiers & cmdKey) != 0) &&
					((theEvent->message & charCodeMask) == '.'))
					return true;
		}
	}
	SIOUXHandleOneEvent(theEvent);
	return false;
}
//------------------------------------------------------------------
extern int dprintf(const char *format,  ...);
//------------------------------------------------------------------
int main(void)
{
	Str255		tStr255;
	FSSpec		tFSSpec;
	AEDescList	objectList;
	AEIdleUPP	tAEIdleUPP = NewAEIdleUPP(MyAEIdleProc);
	OSErr		anErr;

	// Set the SIOUX window defaults
	SIOUXSettings.autocloseonquit	= FALSE;
	SIOUXSettings.asktosaveonclose	= FALSE;
	SIOUXSettings.showstatusline	= FALSE;
	SIOUXSettings.columns			= 132;
	SIOUXSettings.rows				= 24;
	SIOUXSettings.fontsize			= 10;
	GetFNum("\pMonaco",&SIOUXSettings.fontid);
	SIOUXSettings.standalone		= TRUE;

	printf("\nWelcome to MAE_TestApp!\n");

#if TEST_MOREFEGETSELECTION
	anErr = MoreFEGetSelection(tAEIdleUPP,&objectList);
#elif TEST_MOREFEGETEVERYITEMONDESKTOP
	anErr = MoreFEGetEveryItemOnDesktop(tAEIdleUPP,&objectList);
#endif
	if (noErr == anErr)
	{
		long		numDocs, index;
		AEKeyword	keyword;

#if TEST_MOREFEGETSELECTION
		printf("\nMoreFEGetSelection good!");
#elif TEST_MOREFEGETEVERYITEMONDESKTOP
		printf("\nMoreFEGetEveryItemOnDesktop good!");
#endif
		anErr = AECountItems (&objectList, &numDocs);
		if (noErr == anErr)
		{
			printf("\nAECountItems good! (%ld)",numDocs);
			for (index = 1; index <= numDocs; index++)
			{
				FSRef	tFSRef;
				AEDesc	tAEDesc;

				anErr = AEGetNthDesc(&objectList,index,typeWildCard,&keyword,&tAEDesc);
				if (noErr != anErr)
				{
					printf("\nAEGetNthDesc(typeWildCard) error: %d.",anErr);
					continue;
				}
				else
				{
					printf("\nAEGetNthDesc(typeWildCard) good, (actual: '%4.4s').",&tAEDesc.descriptorType);

					if (typeObjectSpecifier == tAEDesc.descriptorType)
					{
						AliasHandle tAliasHandle = NULL;

						anErr = MoreFEGetObjectAsAlias(&tAEDesc,&tAliasHandle,tAEIdleUPP);
						if (noErr == anErr)
						{
							Boolean wasChanged;
#if 1
							anErr = ResolveAliasWithMountFlags(NULL,tAliasHandle,
										&tFSSpec,&wasChanged,kResolveAliasFileNoUI);
#else
							anErr = ResolveAlias(NULL,tAliasHandle,&tFSSpec,&wasChanged);
#endif
							if (noErr == anErr)
								DisposeHandle((Handle) tAliasHandle);
							else
							{
								printf("\nResolveAlias[WithMountFlags] error: %d.",anErr);
								continue;
							}
						}
						else
						{
							printf("\nMoreFEGetObjectAsAlias error: %d.",anErr);
							continue;
						}
					}
					else
					{
						if (typeFSRef != tAEDesc.descriptorType)
						{
							anErr = AECoerceDesc(&tAEDesc,typeFSRef,&tAEDesc);
							if (noErr != anErr)
							{
								printf("\nAECoerceDesc error: %d.",anErr);
								continue;
							}
						}
						anErr = AEGetDescData(&tAEDesc,&tFSRef,sizeof(FSRef));
						if (noErr != anErr)
						{
							printf("\nAEGetDescData error: %d.",anErr);
							continue;
						}
					}
				}

				GetFullPath(&tFSSpec,(char *) &tStr255,&anErr);
				printf("\n   Nth: %s",tStr255);
#if TEST_GET_COMMENT
				anErr = MoreFEGetComment(&tFSSpec,tStr255,tAEIdleUPP);
				if (noErr == anErr)
				{
					if (tStr255[0])
						printf("\t\"%#s\".",tStr255);
					else
						printf("\t\"<no comment!>\".");
#if TEST_SET_COMMENT
					if (tStr255[0])
						anErr = MoreFESetComment(&tFSSpec,"\p",tAEIdleUPP);
					else
						anErr = MoreFESetComment(&tFSSpec,"\pWhat the fork?",tAEIdleUPP);

					if (noErr == anErr)
						printf("\nMoreFESetComment good!");
					else
						printf("\nMoreFESetComment error: %d.",anErr);
#endif TEST_SET_COMMENT
				} else
					printf("\nMoreFEGetComment error: %d.",anErr);
#endif TEST_GET_COMMENT

#if TEST_OPEN_INFO_WINDOW
				anErr = MoreFEOpenInfoWindow(&tFSSpec,tAEIdleUPP);
				if (noErr == anErr)
					printf("\nMoreFEOpenInfoWindow good!");
				else
					printf("\nMoreFEOpenInfoWindow error: %d.",anErr);
#endif TEST_OPEN_INFO_WINDOW

#if TEST_MFE_OPEN_FILE
				anErr = MoreFEOpenFile(&tFSSpec);
				if (noErr == anErr)
					printf("\nMoreFEOpenFile good!");
				else
					printf("\nMoreFEOpenFile error: %d.",anErr);
#endif TEST_MFE_OPEN_FILE
			}
			anErr = noErr;
		}
		else
			printf("\nAECountItems error: %d.",anErr);

		(void) AEDisposeDesc(&objectList);
	}
	else
		printf("\nMoreFEGetSelection error: %d.",anErr);

	DisposeAEIdleUPP(tAEIdleUPP);
	return anErr;

	return 0;
}