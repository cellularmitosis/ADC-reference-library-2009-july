/*
	File:		MoreCFMPatches.c

	Contains:	Test CFM patching technology.

	Written by:	Quinn

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreCFMPatchesTest.c,v $
Revision 1.3  2002/11/08 23:53:12         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.2  2001/11/07 15:51:55         
Tidy up headers, add CVS logs, update copyright.


         <1>     16/3/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#include <CodeFragments.h>
#include <LowMem.h>

// ANSI C Interfaces

#include <stdio.h>
#include <string.h>

// Project Interfaces

#include "MoreCFMPatches.h"

/////////////////////////////////////////////////////////////////
#pragma mark ---- Basic Functionality -----

// In this test, we patch LMGetScrDmpEnb twice, call through
// to the existing routine in both patches, while printing out
// entry and exit markers.  Seems to work (-:

typedef pascal UInt8 (*LMGetScrDmpEnbProc)(void);

static pascal UInt8 MyLMGetScrDmpEnb(void)
{
	UInt8 result;
	
	printf("    Enter MyLMGetScrDmpEnb\n");
	result = ( (LMGetScrDmpEnbProc) gMoreCFMPatchesCallThrough)();
	printf("    Leave MyLMGetScrDmpEnb\n");
	return result;
}

static pascal UInt8 MyLMGetScrDmpEnb2(void)
{
	UInt8 result;

	printf("  Enter MyLMGetScrDmpEnb2\n");
	result = ( (LMGetScrDmpEnbProc) gMoreCFMPatchesCallThrough)();
	printf("  Leave MyLMGetScrDmpEnb2\n");
	return result;
}

static void BasicFunctionality(void)
{
	OSStatus err;
	OSStatus junk;
	CFragConnectionID connID;
	Ptr junkMain;
	Str255 junkMessage;
	CFragSymbolClass junkSymClass;
	Ptr symAddr;
	UInt8 junkUInt8;
	void *origCodePointer;
	
	printf("Basic functionality testing...\n");	
	err = GetSharedLibrary("\pInterfaceLib", kPowerPCCFragArch, kFindCFrag, &connID, &junkMain, junkMessage);
	if (err == noErr) {
		err = FindSymbol(connID, "\pLMGetScrDmpEnb", &symAddr, &junkSymClass);
	}
	if (err == noErr) {
		origCodePointer = ((TVector *)symAddr)->codePointer;
		err = MorePatchTVector((TVector *) symAddr, (TVector *) MyLMGetScrDmpEnb,  'KWN1', NULL);
		if (err == noErr) {
			assert(origCodePointer != ((TVector *)symAddr)->codePointer);
			err = MorePatchTVector((TVector *) symAddr, (TVector *) MyLMGetScrDmpEnb2, 'KWN2', NULL);
			if (err == noErr) {
				junkUInt8 = LMGetScrDmpEnb();
				junk = MoreUnpatchTVector((TVector *) symAddr, (TVector *) MyLMGetScrDmpEnb2);
				assert(junk == noErr);
			}
			junk = MoreUnpatchTVector((TVector *) symAddr, (TVector *) MyLMGetScrDmpEnb);
			assert(junk == noErr);
		}
		assert(origCodePointer == ((TVector *)symAddr)->codePointer);
	}
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ---- Ordering Tests -----

typedef void (*OrderProc)(char *str);

static void OBase(char *str)
{
	strcpy(str, "OBase");
}

static void O1(char *str)
{
	((OrderProc) gMoreCFMPatchesCallThrough)(str);
	strcat(str, "->O1");
}

static void O2(char *str)
{
	((OrderProc) gMoreCFMPatchesCallThrough)(str);
	strcat(str, "->O2");
}

static void O3(char *str)
{
	((OrderProc) gMoreCFMPatchesCallThrough)(str);
	strcat(str, "->O3");
}

static void OrderingTests(void)
	// Tests that patches are applied in the order we expected
	// and that they are executed in the order they're applied.
{
	OSStatus junk;
	char str[256];
	OrderProc obaseProc;

	printf("Ordering tests...\n");
	
	// We call OBase through a routine pointer to force it to go through
	// the TVector.  Otherwise, because the routine is implemented in the
	// same file as the caller, the compiler optimises the TVector goop away.
		
	obaseProc = OBase;
	
	obaseProc(str);
	assert(strcmp(str, "OBase") == 0);
	
	junk = MorePatchTVector((TVector *) OBase, (TVector *) O1,  'Oone', NULL);
	assert(junk == noErr);
	
	obaseProc(str);
	assert(strcmp(str, "OBase->O1") == 0);
	
	junk = MorePatchTVector((TVector *) OBase, (TVector *) O2, 'Otwo', NULL);
	assert(junk == noErr);

	obaseProc(str);
	assert(strcmp(str, "OBase->O1->O2") == 0);

	junk = MorePatchTVector((TVector *) OBase, (TVector *) O3, 'Otri', NULL);
	assert(junk == noErr);

	obaseProc(str);
	assert(strcmp(str, "OBase->O1->O2->O3") == 0);

	// Here we try a difficult case, namely pulling a patch out
	// of the middle of the patch chain.
		
	junk = MoreUnpatchTVector((TVector *) OBase, (TVector *) O2);
	assert(junk == noErr);

	obaseProc(str);
	assert(strcmp(str, "OBase->O1->O3") == 0);

	junk = MoreUnpatchTVector((TVector *) OBase, (TVector *) O1);
	assert(junk == noErr);

	obaseProc(str);
	assert(strcmp(str, "OBase->O3") == 0);
	
	junk = MoreUnpatchTVector((TVector *) OBase, (TVector *) O3);
	assert(junk == noErr);

	obaseProc(str);
	assert(strcmp(str, "OBase") == 0);
	
	printf("Success!\n");
}

#pragma mark ---- Information Tests -----

static OSStatus PrintPatchChain(TVector *patchedTVector)
{
	OSStatus err;
	ItemCount patchCount;
	ItemCount patchIndex;
	OSType  creator;
	TVector *tVector;
	void    *refcon;
	
	err = MoreCountPatches(patchedTVector, &patchCount);
	if (err == noErr) {
		for (patchIndex = 1; patchIndex <= patchCount; patchIndex++) {
			err = MoreGetIndexedPatchInfo(patchedTVector, patchIndex, kPatchInfoCreator, &creator, sizeof(creator));
			if (err == noErr) {
				err = MoreGetIndexedPatchInfo(patchedTVector, patchIndex, kPatchInfoTVector, &tVector, sizeof(tVector));
			}
			if (err == noErr) {
				err = MoreGetIndexedPatchInfo(patchedTVector, patchIndex, kPatchInfoRefcon, &refcon, sizeof(refcon));
			}
			if (err == noErr) {
				printf("%ld $%08lx '%4.4s' $%ld -> ", patchIndex, tVector, &creator, refcon);
			} else {
				break;
			}
		}
		printf("\n");
	}
	
	return err;
}

static void InformationTests(void)
	// Tests that the patch information routines are
	// working as expected.
{
	OSStatus junk;

	printf("Information tests...\n");
	
	junk = MorePatchTVector((TVector *) OBase, (TVector *) O1, 'Oone', (void *) 1);
	assert(junk == noErr);
	junk = PrintPatchChain((TVector *)  OBase);
	assert(junk == noErr);
	junk = MorePatchTVector((TVector *) OBase, (TVector *) O2, 'Otwo', (void *) 2);
	assert(junk == noErr);
	junk = PrintPatchChain((TVector *)  OBase);
	assert(junk == noErr);
	junk = MorePatchTVector((TVector *) OBase, (TVector *) O3, 'Otri', (void *) 3);
	assert(junk == noErr);
	junk = PrintPatchChain((TVector *)  OBase);
	assert(junk == noErr);

	junk = MoreUnpatchTVector((TVector *) OBase, (TVector *) O2);
	assert(junk == noErr);
	junk = PrintPatchChain((TVector *) OBase);
	assert(junk == noErr);
	junk = MoreUnpatchTVector((TVector *) OBase, (TVector *) O1);
	assert(junk == noErr);
	junk = PrintPatchChain((TVector *) OBase);
	assert(junk == noErr);
	junk = MoreUnpatchTVector((TVector *) OBase, (TVector *) O3);
	assert(junk == noErr);
	junk = PrintPatchChain((TVector *) OBase);
	assert(junk == kVectorNotPatchedByUsErr);
	
	printf("Success!\n");
}

/////////////////////////////////////////////////////////////////
#pragma mark ---- Main Line -----

void main(void)
{
	printf("Hello Cruel World!\n");
	printf("MoreCFMPatchesTest\n");
	printf("-- A simple test program for the MoreCFMPatches library.\n");

	BasicFunctionality();
	printf("\n");
	OrderingTests();
	printf("\n");
	InformationTests();
	printf("\n");
	
	printf("Done.  Press command-Q to Quit.\n");
}