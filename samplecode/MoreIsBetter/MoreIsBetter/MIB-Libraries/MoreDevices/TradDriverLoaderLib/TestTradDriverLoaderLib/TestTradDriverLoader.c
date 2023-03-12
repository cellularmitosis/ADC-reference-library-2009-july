/*
	File:		TestTradDriverLoaderLib.c

	Contains:	Program to test TradDriverLoaderLib.

	Written by:	Quinn

	Copyright:	Copyright (c) 1996-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: TestTradDriverLoader.c,v $
Revision 1.3  2002/11/08 23:23:25         
Convert nil to NULL.

Revision 1.2  2001/11/07 15:58:50         
Tidy up headers, add CVS logs, update copyright.


         <1>    22/11/00    Quinn   First checked in. Converted from Pascal
                                    TestTradDriverLoaderLib.p,1.
*/

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#include <MacTypes.h>
#include <LowMem.h>
#include <Files.h>
#include <Devices.h>
#include <PLStringFuncs.h>

// Stardard C Interfaces

#include <stdio.h>

// MIB Prototypes
		
#include "TradDriverLoaderLib.h"

/////////////////////////////////////////////////////////////////

static void CheckTestResult(OSErr err, ConstStr255Param mesg)
{
	if (err == noErr) {
		printf("  Passed %#s.\n", mesg);
	} else {
		printf("  •••Failed %#s with error %d.\n", mesg, err);
	}
}
	
static void CheckTestResultBool(Boolean b, ConstStr255Param mesg)
{
	if (b) {
		printf("  Passed %#s.\n", mesg);
	} else {
		printf("  •••Failed %#s.\n", mesg);
	}
}

static void TestTradHighestUnitNumber(void)
{
	printf("TestTradHighestUnitNumber\n");
	printf("\n");
	CheckTestResultBool( TradHighestUnitNumber() == (LMGetUnitTableEntryCount() - 1) , "\pcheck against LMGetUnitTableEntryCount");
}

static void TestInstallOpenRemove(void)
{
	OSErr 			err;
	DriverRefNum 	refNum;
	ParamBlockRec 	cpb;

	printf("TestInstallOpenRemove\n");
	printf("\n");
	
	/* Install */
	err = TradInstallDriverFromResource(0, "\p.Test", 48, TradHighestUnitNumber() + 1, &refNum);
	CheckTestResult(err, "\pinstall");
	
	/* Open */
	if (err == noErr) {
		err = TradOpenInstalledDriver(refNum, fsRdWrPerm);
		CheckTestResult(err, "\popen");
	}
	
	/* Functional */
	if (err == noErr) {
		cpb.cntrlParam.ioCRefNum = refNum;
		cpb.cntrlParam.csCode    = 666;
		err = PBStatusSync(&cpb);
		CheckTestResult(err, "\pstatus call");
	}
	if (err == noErr) {
		CheckTestResultBool(cpb.cntrlParam.csParam[0] == refNum, "\presults from status call");
	}
	
	/* Remove */
	if (err == noErr) {
		err = TradRemoveDriver(refNum, false);
		CheckTestResult(err, "\premove");
	}

	printf("\n");
} /* TestInstallOpenRemove */

static void TestGestaltCalls(void)
{
	OSErr 			err;
	DriverRefNum 	refNum;
	DriverFlags 	flags;

	printf("TestGestaltCalls\n");
	printf("\n");

	/* Install */
	err = TradInstallDriverFromResource(0, "\p.Test", 48, TradHighestUnitNumber() + 1, &refNum);
	CheckTestResult(err, "\pinstall");

	/* Open */
	if (err == noErr) {
		err = TradOpenInstalledDriver(refNum, fsRdWrPerm);
		CheckTestResult(err, "\popen");
	}

	/* Gestalt On */
	if (err == noErr) {
		err = TradDriverGestaltOn(refNum);
		CheckTestResult(err, "\pset Gestalt on");
	}
	if (err == noErr) {
		err = TradGetDriverInformation(refNum, NULL, &flags, NULL, NULL);
		CheckTestResultBool((err == noErr) && TradDriverGestaltIsOn(flags), "\pconfirm Gestalt on");
	}

	/* Gestalt Off */
	if (err == noErr) {
		err = TradDriverGestaltOff(refNum);
		CheckTestResult(err, "\pset Gestalt off");
	}
	if (err == noErr) {
		err = TradGetDriverInformation(refNum, NULL, &flags, NULL, NULL);
		CheckTestResultBool((err == noErr) && not TradDriverGestaltIsOn(flags), "\pconfirm Gestalt off");
	}

	/* Remove */
	if (err == noErr) {
		err = TradRemoveDriver(refNum, false);
		CheckTestResult(err, "\premove");
	}

	printf("\n");
} /* TestGestaltCalls */


static void TestInfoCalls(void)
{
	OSErr 			err;
	ItemCount 		refNumsCount;
	ItemCount 		i;
	DriverRefNum 	refNums[1000];
	UnitNumber 		thisUnit;
	DriverFlags 	flags;
	Str255 			name;
	DRVRHeaderPtr 	driverHeader;

	printf("TestInfoCalls\n");
	printf("\n");

	refNumsCount = 1000;
	err = TradLookupDrivers(0, TradHighestUnitNumber(), false, &refNumsCount, refNums);
	CheckTestResult(err, "\plookup drivers");
	if (err == noErr) {
		for (i = 0; i < refNumsCount; i++) {
			err = TradGetDriverInformation(refNums[i], &thisUnit, &flags, name, &driverHeader);
			if (err == noErr) {
				printf("  refNum=%d, unit=%d, flags=%d, name=“%#s”, addr=%08lx\n", 
						  refNums[i], thisUnit, flags, name, driverHeader);
			} else {
				printf("  •••Error %d getting info for refnum %d.\n", err, refNums[i]);
			}
		}
	}

	printf("\n");		
} /* TestInfoCalls */

static void TestAddLots(void)
{
	OSErr 			err;
	DriverRefNum 	refNum;
	UInt8 			highChar;
	UInt8 			lowChar;
	Str255 			rsrcName;

	printf("TestAddLots\n");
	printf("\n");
	
	/* 	Add 2 * 26 == 52) drivers to the unit table and open them all!
		Have to stop before 'T' otherwise our unique names clash with '.Test'.
		Note that this is limited by kMaximumNumberOfUnitTableEntries defined
		in "TradDriverLoaderLib.c".
	*/
	
	for (highChar = 'A'; highChar <= 'B'; highChar++) {
		for (lowChar = 'A'; lowChar <= 'Z'; lowChar++) {
			PLstrcpy(rsrcName, "\p.Test");
			err = TradInstallDriverFromResource(0, rsrcName, 48, TradHighestUnitNumber() + 1, &refNum);
			if (err == noErr) {
				rsrcName[2] = highChar;
				rsrcName[3] = lowChar;
				err = TradRenameDriver(refNum, rsrcName);
			}
			if (err == noErr) {
				err = TradOpenInstalledDriver(refNum, fsRdWrPerm);
			}
			if (err != noErr) {
				printf("  •••Adding lots failed at '%c%c' with error %d.\n", highChar, lowChar, err);
				break;
			}
		}
	}
	
	printf("  Drivers added successfully!\n");
	
	/* Remove all of the added units. */

	for (highChar = 'A'; highChar <= 'B'; highChar++) {
		for (lowChar = 'A'; lowChar <= 'Z'; lowChar++) {
			PLstrcpy(rsrcName, "\p.Test");
			rsrcName[2] = highChar;
			rsrcName[3] = lowChar;
			err = OpenDriver(rsrcName, &refNum);
			if (err == noErr) {
				err = TradRemoveDriver(refNum, false);
			}
			if (err != noErr) {
				printf("  •••Removing lots failed at '%c%c' with error %d.\n", highChar, lowChar, err);
				break;
			}
		}
	}
	
	printf("  Drivers removed successfully!\n");

	printf("\n");
} /* TestAddLots */

extern void main(void)
{
	printf("Hello Cruel World!\n");
	printf("\n");
	printf("TestTradDriverLoaderLib\n");
	printf("-- a test tool for the TradDriverLoaderLib library.\n");
	printf("\n");
	TestInstallOpenRemove();
	TestGestaltCalls();
	TestInfoCalls();
	TestAddLots();
	printf("Done.  Press command-Q to Quit.\n");
}
