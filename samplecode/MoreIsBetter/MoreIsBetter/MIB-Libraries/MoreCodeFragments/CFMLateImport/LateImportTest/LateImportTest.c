/*
	File:		LateImportTest.c

	Contains:	CFM late import test framework.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: LateImportTest.c,v $
Revision 1.5  2002/11/08 23:11:27         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.4  2001/11/07 15:50:12         
Tidy up headers, add CVS logs, update copyright.


         <3>     19/9/01    Quinn   Take advantage of new, simpler API. No more dummy CFM exports!
         <2>    18/10/99    Quinn   Expanded tests to cover some extra cases.
         <1>     15/6/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

#include "MoreSetup.h"

// Mac OS Interfaces

#include <MacTypes.h>

// Standard C Interfaces

#include <stdio.h>

// MIB Interfaces

#include "CFMLateImport.h"

// Project Interfaces

#include "TestLibrary.h"

/////////////////////////////////////////////////////////////////
#pragma mark ----- Test Framework -----

static void CallLateImportedRoutines(void)
	// This routine uses the data and code imports we just
	// late imported.  I put it in a separate routine because
	// the compiler was caching certain variables (such as
	// the address of gTestData) in registers and wouldn't let go.
	// Putting these accesses in a separate procedure defeats that
	// optimisation and allows the code to work as expected.
	// Something to be wary of.
{
	assert(&gTestData != NULL);
	assert(TestRoutine1 != NULL);
	assert(TestRoutine2 != NULL);

	gTestData = 0;
	TestRoutine1();
	TestRoutine2();
	assert(gTestData == 3);
	printf("gTestData = %ld\n", gTestData);
}

static CFragConnectionID 			gMyCFragConnection = NULL;
static CFragSystem7DiskFlatLocator	gMyFragLocator;
static FSSpec						gMyFragFSSpec;

extern OSErr MyCFragInitFunction(const CFragInitBlock *initBlock);

extern OSErr MyCFragInitFunction(const CFragInitBlock *initBlock)
{
	gMyCFragConnection = initBlock->connectionID;
	assert(initBlock->fragLocator.where == kDataForkCFragLocator);
	gMyFragFSSpec  = *initBlock->fragLocator.u.onDisk.fileSpec;
	gMyFragLocator = initBlock->fragLocator.u.onDisk;
	gMyFragLocator.fileSpec = &gMyFragFSSpec;
	return noErr;
}

void main(void)
{
	OSStatus err;
	CFragConnectionID implConn;
	Ptr junkMain;
	Str255 junkMessage;
	FSSpec fss;
	
	printf("Hello Cruel World!\n");
	printf("LateImportTest.c\n");
	
	assert(gMyCFragConnection != NULL);

	err = noErr;
	if (TestRoutine1 == NULL || TestRoutine2 == NULL) {
		err = FSMakeFSSpec(0, 0, "\p:TestLibrary:TestLibraryImpl", &fss);
		if (err == noErr) {
			err = GetDiskFragment(&fss, 0, kCFragGoesToEOF, fss.name, kReferenceCFrag, &implConn, &junkMain, junkMessage);
		}
		if (err == noErr) {
			err = CFMLateImportLibrary(&gMyFragLocator, gMyCFragConnection, MyCFragInitFunction, "\pTestLibrary", implConn);
		}
	}
	if (err == noErr) {
		CallLateImportedRoutines();
	}
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
	
	printf("Done.  Press command-Q to Quit.\n");
}

