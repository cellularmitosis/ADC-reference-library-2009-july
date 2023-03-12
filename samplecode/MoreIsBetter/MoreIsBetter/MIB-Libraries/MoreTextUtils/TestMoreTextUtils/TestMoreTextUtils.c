/*
	File:		TestMoreTextUtils.c

	Contains:	Program to test the MoreTextUtils module.

	Written by:	Quinn

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

$Log: TestMoreTextUtils.c,v $
Revision 1.3  2002/11/09 00:12:44         
Convert MoreAssertQ to assert.

Revision 1.2  2001/11/07 15:58:42         
Tidy up headers, add CVS logs, update copyright.


         <1>     11/4/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#include <MacTypes.h>
#include <PLStringFuncs.h>
#include <Resources.h>

// MIB interfaces

#include "MoreTextUtils.h"
#include "MoreResources.h"
#include "MoreMemory.h"

// Standard C interfaces

#include <stdio.h>

/////////////////////////////////////////////////////////////////

static void BasicTests(void)
{
	OSStatus err;
	Handle   strList;
	SInt16   count;
	Str255   tmpStr;
	
	// Some basics (MoreStrListNew, MoreStrListAppend, MoreStrListCount)
	
	err = MoreStrListNew(&strList);
	assert(err == noErr);
	err = MoreStrListAppend(strList, "\pString 1 with length");
	assert(err == noErr);
	err = MoreStrListAppend(strList, "\pString 2 with random length extra");
	assert(err == noErr);
	err = MoreStrListAppend(strList, "\pString 3 with extra");
	assert(err == noErr);
	count = MoreStrListCount(strList);
	if (count != 3) {
		printf("••• Test failure -- count != 3\n");
	}

	// MoreStrListGetIndexed

	err = MoreStrListGetIndexed(strList, 1, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pString 1 with length") != 0 ) {
		printf("••• Test failure -- item 1 != “String 1 with length”\n");
	}
	err = MoreStrListGetIndexed(strList, 2, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pString 2 with random length extra") != 0 ) {
		printf("••• Test failure -- item 2 != “String 2 with random length extra”\n");
	}
	err = MoreStrListGetIndexed(strList, 3, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pString 3 with extra") != 0 ) {
		printf("••• Test failure -- item . != “String 3 with extra”\n");
	}
	
		// out of bounds should yield an error
		
	if ( MoreStrListGetIndexed(strList, 0, tmpStr) != paramErr ) {
		printf("••• Test failure -- MoreStrListGetIndexed bounds check (1)\n");
	}
	if ( MoreStrListGetIndexed(strList, -1, tmpStr) != paramErr ) {
		printf("••• Test failure -- MoreStrListGetIndexed bounds check (2)\n");
	}
	if ( MoreStrListGetIndexed(strList, -32768, tmpStr) != paramErr ) {
		printf("••• Test failure -- MoreStrListGetIndexed bounds check (3)\n");
	}
	if ( MoreStrListGetIndexed(strList, 4, tmpStr) != paramErr ) {
		printf("••• Test failure -- MoreStrListGetIndexed bounds check (4)\n");
	}
	if ( MoreStrListGetIndexed(strList, 32767, tmpStr) != paramErr ) {
		printf("••• Test failure -- MoreStrListGetIndexed bounds check (5)\n");
	}

	// MoreStrListSetIndexed

	err = MoreStrListSetIndexed(strList, 3, "\pNew String 3 plus stuff");
	assert(err == noErr);
	err = MoreStrListSetIndexed(strList, 2, "\pNew String 2 plus");
	assert(err == noErr);
	err = MoreStrListSetIndexed(strList, 1, "\pNew String 1");
	assert(err == noErr);

	err = MoreStrListGetIndexed(strList, 2, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 2 plus") != 0 ) {
		printf("••• Test failure -- item 1 != “New String 2 plus”\n");
	}
	err = MoreStrListGetIndexed(strList, 1, tmpStr);
	assert(err == noErr);

	if ( PLstrcmp(tmpStr, "\pNew String 1") != 0 ) {
		printf("••• Test failure -- item 2 != “New String 1”\n");
	}
	err = MoreStrListGetIndexed(strList, 3, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 3 plus stuff") != 0 ) {
		printf("••• Test failure -- item . != “New String 3 plus stuff”\n");
	}

		// out of bounds should yield an error
		
	if ( MoreStrListSetIndexed(strList, 0, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListSetIndexed bounds check (1)\n");
	}
	if ( MoreStrListSetIndexed(strList, -1, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListSetIndexed bounds check (2)\n");
	}
	if ( MoreStrListSetIndexed(strList, -32768, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListSetIndexed bounds check (3)\n");
	}
	if ( MoreStrListSetIndexed(strList, 4, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListSetIndexed bounds check (4)\n");
	}
	if ( MoreStrListSetIndexed(strList, 32767, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListSetIndexed bounds check (5)\n");
	}

	// MoreStrListDeleteIndexed

		// out of bounds should yield an error
		
	if ( MoreStrListDeleteIndexed(strList, 0) != paramErr ) {
		printf("••• Test failure -- MoreStrListDeleteIndexed bounds check (1)\n");
	}
	if ( MoreStrListDeleteIndexed(strList, -1) != paramErr ) {
		printf("••• Test failure -- MoreStrListDeleteIndexed bounds check (2)\n");
	}
	if ( MoreStrListDeleteIndexed(strList, -32768) != paramErr ) {
		printf("••• Test failure -- MoreStrListDeleteIndexed bounds check (3)\n");
	}
	if ( MoreStrListDeleteIndexed(strList, 4) != paramErr ) {
		printf("••• Test failure -- MoreStrListDeleteIndexed bounds check (4)\n");
	}
	if ( MoreStrListDeleteIndexed(strList, 32767) != paramErr ) {
		printf("••• Test failure -- MoreStrListDeleteIndexed bounds check (5)\n");
	}

		// first
	
	err = MoreStrListDeleteIndexed(strList, 1);
	assert(err == noErr);

	count = MoreStrListCount(strList);
	if (count != 2) {
		printf("••• Test failure -- count != 2\n");
	}
	err = MoreStrListGetIndexed(strList, 1, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 2 plus") != 0 ) {
		printf("••• Test failure -- item 1 != “New String 2 plus”\n");
	}
	err = MoreStrListGetIndexed(strList, 2, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 3 plus stuff") != 0 ) {
		printf("••• Test failure -- item . != “New String 3 plus stuff”\n");
	}
	
		// last

	err = MoreStrListDeleteIndexed(strList, 2);
	assert(err == noErr);

	count = MoreStrListCount(strList);
	if (count != 1) {
		printf("••• Test failure -- count != 1\n");
	}
	err = MoreStrListGetIndexed(strList, 1, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 2 plus") != 0 ) {
		printf("••• Test failure -- item 1 != “New String 2 plus”\n");
	}

		// last remaining

	err = MoreStrListDeleteIndexed(strList, 1);
	assert(err == noErr);

	count = MoreStrListCount(strList);
	if (count != 0) {
		printf("••• Test failure -- count != 0\n");
	}
	
	// Reset the string.

	err = MoreStrListAppend(strList, "\pString 1");
	assert(err == noErr);
	err = MoreStrListAppend(strList, "\pString 2 with random length extra");
	assert(err == noErr);
	err = MoreStrListAppend(strList, "\pString 3 with extra");
	assert(err == noErr);

	// MoreStrListInsertAtIndex

	err = MoreStrListInsertAtIndex(strList, 1, "\pNew String 1");
	assert(err == noErr);
	err = MoreStrListInsertAtIndex(strList, 3, "\pNew String 3");
	assert(err == noErr);
	err = MoreStrListInsertAtIndex(strList, 5, "\pNew String 5");
	assert(err == noErr);
	err = MoreStrListInsertAtIndex(strList, 7, "\pNew String 7");
	assert(err == noErr);
	
	count = MoreStrListCount(strList);
	if (count != 7) {
		printf("••• Test failure -- count != 7\n");
	}
	err = MoreStrListGetIndexed(strList, 1, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 1") != 0 ) {
		printf("••• Test failure -- item . != “New String 1”\n");
	}
	err = MoreStrListGetIndexed(strList, 2, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pString 1") != 0 ) {
		printf("••• Test failure -- item . != “String 1”\n");
	}
	err = MoreStrListGetIndexed(strList, 3, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 3") != 0 ) {
		printf("••• Test failure -- item . != “New String 3”\n");
	}
	err = MoreStrListGetIndexed(strList, 4, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pString 2 with random length extra") != 0 ) {
		printf("••• Test failure -- item . != “String 2 with random length extra”\n");
	}
	err = MoreStrListGetIndexed(strList, 5, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 5") != 0 ) {
		printf("••• Test failure -- item . != “New String 5”\n");
	}
	err = MoreStrListGetIndexed(strList, 6, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pString 3 with extra") != 0 ) {
		printf("••• Test failure -- item . != “String 3 with extra”\n");
	}
	err = MoreStrListGetIndexed(strList, 7, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\pNew String 7") != 0 ) {
		printf("••• Test failure -- item . != “New String 7”\n");
	}

		// out of bounds should yield an error
		
	if ( MoreStrListInsertAtIndex(strList, 0, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListInsertAtIndex bounds check (1)\n");
	}
	if ( MoreStrListInsertAtIndex(strList, -1, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListInsertAtIndex bounds check (2)\n");
	}
	if ( MoreStrListInsertAtIndex(strList, -32768, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListInsertAtIndex bounds check (3)\n");
	}
	if ( MoreStrListInsertAtIndex(strList, 9, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListInsertAtIndex bounds check (4)\n");
	}
	if ( MoreStrListInsertAtIndex(strList, 32767, "\pFrog") != paramErr ) {
		printf("••• Test failure -- MoreStrListInsertAtIndex bounds check (5)\n");
	}
	
	// MoreStrListEmpty

		// emptying a valid string list
			
	err = MoreStrListEmpty(strList);
	assert(err == noErr);
	count = MoreStrListCount(strList);
	if (count != 0) {
		printf("••• Test failure -- empty count != 0 (1)\n");
	}
	
		// emptying a bogus string list
		
	SetHandleSize(strList, 666);
	assert(MemError() == noErr);
	
	err = MoreStrListEmpty(strList);
	assert(err == noErr);
	count = MoreStrListCount(strList);
	if (count != 0) {
		printf("••• Test failure -- empty count != 0 (2)\n");
	}
	
	if (strList != NULL) {
		DisposeHandle(strList);
		assert(MemError() == noErr);
	}
}

static void PurgeableResourceTests(void)
{
	OSStatus err;
	Handle   strList;
	SInt16   count;
	Str255   tmpStr;
	
	strList = Get1Resource('STR#', 128);
	err = MoreResError(strList);
	assert(err == noErr);
	
	EmptyHandle(strList);
	assert(MemError() == noErr);
	assert(*strList == NULL);
	
	// Tests that the standard entry/exit code will actually load the resource.
	
	count = MoreStrListCount(strList);
	if (count != 3) {
		printf("••• Test failure -- MoreStrListCount did not load resource\n");
	}
	if ( HGetState(strList) != (kHandlePurgeableMask | kHandleIsResourceMask) ) {
		printf("••• Test failure -- count changed the flags\n");
	}

	// Do similar tests with MoreStrListInsertAtIndex, where index is
	// count plus one.  This is interesting because in that case it
	// calls MoreStrListAppend, which means we go through the standard
	// entry/exit twice.
	
	err = MoreStrListInsertAtIndex(strList, 4, "\p!");
	assert(err == noErr);
	
	err = MoreStrListGetIndexed(strList, 4, tmpStr);
	assert(err == noErr);
	if ( PLstrcmp(tmpStr, "\p!") != 0 ) {
		printf("••• Test failure -- MoreStrListGetIndexed\n");
	}
	if ( HGetState(strList) != (kHandlePurgeableMask | kHandleIsResourceMask) ) {
		printf("••• Test failure -- MoreStrListInsertAtIndex changed the flags\n");
	}
	
	if (strList != NULL) {
		// Not strictly necessary, but I want to avoid any possibility 
		// of the changed resource being written back.
		ReleaseResource(strList);	
		assert(ResError() == noErr);
	}
	
	// Now try similar tests with a purged memory handle.
	
	strList = NewHandle(0);
	err = MoreMemError(strList);
	assert(err == noErr);
	
	HPurge(strList);
	assert(MemError() == noErr);
	EmptyHandle(strList);
	assert(MemError() == noErr);
	
	// This call actually drops into MacsBug with an assertion failure because
	// the pre-condition check for strList being valid fails.  That’s annoying from
	// a testing point of view, but fixing it is too much grief right now.
	
	if ( MoreStrListGetIndexed(strList, 1, tmpStr) != resNotFound ) {
		printf("••• Test failure -- wrong error when LoadResource on memory handle\n");
	}
	
	if (strList != NULL) {
		DisposeHandle(strList);
		assert(MemError() == noErr);
	}
}

void main(void)
{
	char commandStr[256];
	
	printf("Hello Cruel World!\n");
	printf("TestMoreTextUtils.c\n");
	
	printf("a) basic tests\n");
	printf("b) tests with purgeable resource\n");
	printf("\n");
	printf("Choose a test:\n");
	
	gets(commandStr);
	switch (commandStr[0]) {
		case 'a':
			BasicTests();
			break;
		case 'b':
			PurgeableResourceTests();
			break;
	}
	
	printf("Done.  Press command-Q to Quit.\n");
}