/*
	File:		MoreResources.h

	Contains:	Resource Manager utilities

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

$Log: MoreResources.h,v $
Revision 1.7  2002/11/08 23:59:42         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.6  2001/11/07 15:55:02         
Tidy up headers, add CVS logs, update copyright.


         <5>     24/9/01    Quinn   Fixes to compile with C++ activated.
         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>     20/3/00    Quinn   #include <Files.h> to get it to build.
         <2>      3/9/00    gaw     API changes for MoreAppleEvents
         <1>      6/3/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <MacTypes.h>
	#include <Files.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
	Returns ResError() or, if it’s noErr and p is NULL,
	returns resNotFound.  The rationale for this routine
	is that, under some documented circumstances, GetResource
	can return NULL but not set ResErr.

	pResHdl		input:	a resource handle

	RESULT CODES
	____________
	Same as results for ResError
	____________
*/
extern pascal OSErr MoreResError(const void *pResHdl);
/******************************************************************************
	Given an FSSpec to a file, open it with write permission.
	Check to make sure that the file reference returned actually has write
	permission and return an error if it doesn't.

	pFSSpec		input:	The file to be opened.
	pRefNum			output:	File reference for the newly opened file.
	
	RESULT CODES
	____________
		Same as results for FSpOpenResFile()
	____________
*/
extern pascal OSErr MoreResOpenResFileForWrite(const FSSpec *pFSSpec, SInt16 *pRefNum);
/******************************************************************************
	Utility routine to check that a file opened with read/write permission
	does in fact have read/write permission.
	
	It is possible to get a read-only file reference for a resource file that has
	been opened with read/write permission.  This routine verify that a file open
	with read/write permission does in fact have write permission.

	pRefNum			input:	File reference number for file to be checked.
	
	RESULT CODES
	____________
		true		The resource file was really opened with write permission
		false		It wasn't
	____________
*/
extern pascal Boolean MoreResIsResFileRefNumWritable(const SInt16 pRefNum);
//******************************************************************************
#ifdef __cplusplus
}
#endif
