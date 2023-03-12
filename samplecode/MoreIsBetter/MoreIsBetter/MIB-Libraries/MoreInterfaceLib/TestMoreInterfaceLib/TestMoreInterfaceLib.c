/*
	File:		TestMoreInterfaceLib.c

	Contains:	Test rig for MoreInterfaceLib.

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

$Log: TestMoreInterfaceLib.c,v $
Revision 1.5  2002/11/08 23:32:08         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.4  2001/11/07 15:58:06         
Tidy up headers, add CVS logs, update copyright.


         <3>     16/9/99    Quinn   Add real code to test the FSM FCB accessors.
         <2>     15/6/99    Quinn   Link with the Extended Disk Init Package routines, just to test
                                    their linkage.
         <1>     20/4/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

/////////////////////////////////////////////////////////////////
// Mac OS Interfaces

#include <MacTypes.h>
#include <Devices.h>
#include <Gestalt.h>
#include <TextUtils.h>

/////////////////////////////////////////////////////////////////
// ANSI C Interfaces

#include <stdio.h>

/////////////////////////////////////////////////////////////////
// MIB Interfaces

#include "MoreInterfaceLib.h"

/////////////////////////////////////////////////////////////////

static void DoTestGestaltValue(void)
{
	OSStatus err;
	SInt32 tmp;
	
	err = MoreNewGestaltValue('Foo!', 666);
	if (err == noErr) {
		err = Gestalt('Foo!', &tmp);
		if (err == noErr && tmp != 666) {
			err = -1;
		}
	}
	if (err == noErr) {
		err = MoreReplaceGestaltValue('Foo!', 667);
		if (err == noErr) {
			err = Gestalt('Foo!', &tmp);
			if (err == noErr && tmp != 667) {
				err = -2;
			}
		}
	}
	if (err == noErr) {
		err = MoreSetGestaltValue ('Foo!', 668);
		if (err == noErr) {
			err = Gestalt('Foo!', &tmp);
			if (err == noErr && tmp != 668) {
				err = -2;
			}
		}
	}
	if (err == noErr) {
		err = MoreDeleteGestaltValue('Foo!');
		if (err == noErr) {
			if ( Gestalt('Foo!', &tmp) == noErr) {
				err = -3;
			}
		}
	}
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

/////////////////////////////////////////////////////////////////

static void DoTestExtendedDiskInit(Boolean mustBeFalse)
{
	printf("The purpose of this test is simple to test the linkage.\n");
	if (mustBeFalse) {
		OSStatus err;
		
		err = MoreDIXFormat(1, false, 0, NULL);
		err = MoreDIXZero(1, NULL, 0, 0, 0, 0, NULL);
		err = MoreDIReformat(1, 0, NULL, NULL);
	}
}

/////////////////////////////////////////////////////////////////

static void DoTestFSM()
{
	// MoreUTFindDrive
	{
		SInt16 firstDriveNum;
		DrvQElPtr foundDrive;
	
		firstDriveNum = ((DrvQElPtr) GetDrvQHdr()->qHead)->dQDrive;
		
		if ( MoreUTFindDrive(firstDriveNum, &foundDrive) != noErr ||
					foundDrive->dQDrive != firstDriveNum) {
			printf("•••MoreUTFindDrive failed.\n");
		}
	}
	
	// MoreUTLocateFCB
	{
		VCBPtr firstVCB;
		SInt16 foundRefNum;
		FCBRecPtr foundFCB;
		OSStatus junk;
		OSStatus err;
		UInt32 sysVersion;
				
		// We're assuming that the boot volume is first in the VCB,
		// that the system file is named "System", and that it's
		// file reference number is 2.  All of this is not guaranteed
		// to be true, but it's good enough for this test tool..
		
		firstVCB = ((VCBPtr) GetVCBQHdr()->qHead);
		if ( MoreUTLocateFCB(firstVCB, 0, "\pSystem", &foundRefNum, &foundFCB) == noErr
					&& foundRefNum == 2
					&& EqualString(foundFCB->fcbCName, "\pSystem", false, true)) {

			// Get system version.
			
			junk = Gestalt(gestaltSystemVersion, (SInt32 *) &sysVersion);
			assert(junk == noErr);
			
			// We now search for the second file called' "System".  We
			// find it on Mac OS 8.5 and later (because the data fork
			// of the System file is open) but not on Mac OS 8.1 or earlier.
			
			err = MoreUTLocateNextFCB(firstVCB, 0, "\pSystem", &foundRefNum, &foundFCB);
			if ( err == noErr && sysVersion < 0x0850
					|| err != noErr && sysVersion >= 0x0850 ) {
				printf("•••MoreUTLocateNextFCB failed.\n");
			}			
		} else {
			printf("•••MoreUTLocateFCB failed.\n");
		}
	}
	
	// MoreUTIndexFCB
	{
		OSErr err;
		Boolean found;
		VCBPtr firstVCB;
		SInt16 foundRefNum;
		FCBRecPtr foundFCB;

		firstVCB = ((VCBPtr) GetVCBQHdr()->qHead);
		found = false;
		foundRefNum = 0;
		do {
			err = MoreUTIndexFCB(firstVCB, &foundRefNum, &foundFCB);
			if (err == noErr) {
				found = EqualString(foundFCB->fcbCName, "\pSystem", false, true);
			}
		} while ( err == noErr && ! found );
		if ( ! found ) {
			printf("•••MoreUTIndexFCB failed.\n");
		}
	}
	
	// MoreUTResolveFCB
	{
		FCBRecPtr foundFCB;

		if ( MoreUTResolveFCB(2, &foundFCB) != noErr || ! EqualString(foundFCB->fcbCName, "\pSystem", false, true)) {
			printf("•••MoreUTResolveFCB failed.\n");
		}
	}
	
	printf("FSM test done.\n");
}

/////////////////////////////////////////////////////////////////

void main(void)
{
	char commandStr[256];
	
	printf("Hello Cruel World!\n");
	printf("TestMoreInterfaceLib\n");
	printf("-- A simple test program for the MoreInterfaceLib library.\n");
	
	printf("What test would you like to run?\n");
	printf("a) Gestalt Value\n");
	printf("X) Extended Disk Init\n");
	printf("f) FSM test\n");
	printf("Enter a command:\n");
	gets(commandStr);
	switch (commandStr[0]) {
		case 'a':
			DoTestGestaltValue();
			break;
		case 'X':
			DoTestExtendedDiskInit(false);
			break;
		case 'f':
			DoTestFSM();
			break;
		default:
			printf("Huh?\n");
			break;
	}

	printf("Done.  Press Q to Quit.\n");
}
