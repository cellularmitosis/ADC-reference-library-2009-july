/*
	File:		TestDriverMain.c

	Contains:	Implementation of a very stupid device driver.

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

$Log: TestDriverMain.c,v $
Revision 1.4  2001/11/07 15:57:46         
Tidy up headers, add CVS logs, update copyright.


         <3>     20/9/01    Quinn   Upgrade to CWPro7.
         <2>    22/11/00    Quinn   Switch to "MacTypes.h".
         <1>     25/2/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

/////////////////////////////////////////////////////////////////
// Mac OS Interfaces

#include <MacTypes.h>
#include <Devices.h>

/////////////////////////////////////////////////////////////////

// IMPORTANT:
// I no longer build this driver from source because 
// CodeWarrior Pro 7 does not support building 68K code.
// So I've included a compiled version of the binary with 
// MoreIsBetter.  Also, I've left the TestDriver.mcp project 
// in CodeWarrior Pro 6 format, so you can still build the 
// driver with the last CodeWarrior that supports 68K.

static pascal OSErr DRVROpen(ParmBlkPtr paramBlock, DCtlPtr devCtlEnt)
{
	#pragma unused (paramBlock)
	#pragma unused (devCtlEnt)
	return noErr;
}

static pascal OSErr DRVRPrime(ParmBlkPtr paramBlock, DCtlPtr devCtlEnt)
{
	#pragma unused (paramBlock)
	#pragma unused (devCtlEnt)
	return -1;
}

static pascal OSErr DRVRControl(ParmBlkPtr paramBlock, DCtlPtr devCtlEnt)
{
	#pragma unused (paramBlock)
	#pragma unused (devCtlEnt)
	return controlErr;
}

static pascal OSErr DRVRStatus(ParmBlkPtr paramBlock, DCtlPtr devCtlEnt)
{
	#pragma unused (paramBlock)
	#pragma unused (devCtlEnt)
	OSErr err;
	CntrlParamPtr cpb;
	
	cpb = (CntrlParamPtr) paramBlock;
	
	switch ( cpb->csCode ) {
		case 666:
			cpb->csParam[0] = cpb->ioCRefNum;
			err = noErr;
			break;
		default:
			err = statusErr;
	}
	
	return err;
}

static pascal OSErr DRVRClose(ParmBlkPtr paramBlock, DCtlPtr devCtlEnt)
{
	#pragma unused (paramBlock)
	#pragma unused (devCtlEnt)
	return noErr;
}

OSErr main(ParmBlkPtr paramBlock, DCtlPtr devCtlEnt, short dispatch)
{
//
//	Here A4 is already setup to point to our data segment. There is no need
//	to call long oldA4=SetCurrentA4();/SetA4(oldA4); as in code resources.
//
//	However you have to still have to use "#include <SetupA4.h>;RememberA4();..."
//	when using callback functions.
//
// If your routine returns 1 (asynch request can't be completed right away)
// the Metrowerks startup code will correctly jump to jIODone.  You don't need
// to worry about this issue in this driver.
//
	OSErr	err = noErr;
	
	switch(dispatch)
	{
	case 0: //	Open
		err = DRVROpen(paramBlock, devCtlEnt);
		break;
	case 1: //	Prime		return 1 if async request cannot be completed right away
		err = DRVRPrime(paramBlock, devCtlEnt);
		break;
	case 2: //	Control		return 1 if async request cannot be completed right away
		err = DRVRControl(paramBlock, devCtlEnt);
		break;
	case 3: //	Status		return 1 if async request cannot be completed right away
		err = DRVRStatus(paramBlock, devCtlEnt);
		break;
	case 4: //	Close
		err = DRVRClose(paramBlock, devCtlEnt);
		break;
	}
	return err;
}
