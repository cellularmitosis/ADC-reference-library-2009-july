/*
	File:		MoreProcesses.h

	Contains:	Process Manager utilities.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: MoreProcesses.h,v $
Revision 1.10  2003/03/28 16:15:21         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.9  2002/11/08 23:57:21         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.8  2001/11/07 15:54:50         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <6>     20/3/00    Quinn   Added MoreProcIsProcessAtFront.  Tidied up some copy and paste
                                    errors in the comments.
         <5>      3/9/00    gaw     API changes for MoreAppleEvents
         <4>      6/3/00    Quinn   Added MoreGetProcessAppFile.
         <3>     1/22/99    PCG     TARGET_CARBON
         <2>    11/11/98    PCG     fix headers
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <6>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <5>     11/9/98    PCG     fix comment 4
         <4>     11/9/98    PCG     add copyright blurb
         <3>      9/9/98    PCG     re-work import and export pragmas
         <2>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <Processes.h>
#endif

enum { kSignatureFinder = 'MACS' };

#ifdef __cplusplus
	extern "C" {
#endif

/******************************************************************************
	Return a ProcessSerialNumber for a process whose signature (creator)
	matches the input values.  This routine will find the first process that
	matches the creator.
	
	The ProcessSerialNumber will be kNoProcess is the requested process cannot
	be found. 

	pCreator			input:	The creator of the process to be found.
	pPSN				input:	Pointer to a ProcessSerialNumber.
						output:	A valid PSN or kNoProcess in no match is found.

	RESULT CODES
	____________
	noErr			   0	No error
	procNotFound	�600	No process matched specified creator
	____________
*/

extern pascal OSStatus MoreProcFindProcessByCreator(const OSType pCreator,ProcessSerialNumber *pPSN);
/******************************************************************************
	Return a ProcessSerialNumber for a process whose signature (type and creator)
	matches the input values.  This routine will find the first process that
	matches the type and creator.
	
	The ProcessSerialNumber will be kNoProcess is the requested process cannot
	be found. 

	pCreator			input:	The creator type of the process to be found.
	pType				input:	The file type of the process to be found.
	pPSN				input:	Pointer to a ProcessSerialNumber.
						output:	A valid PSN or kNoProcess in no match is found.

	RESULT CODES
	____________
	noErr			   0	No error
	procNotFound	�600	No process matched specified type and creator
	____________
*/

extern pascal OSStatus MoreProcFindProcessBySignature(const OSType pCreator,const OSType pType,ProcessSerialNumber *pPSN);
/******************************************************************************
	Returns the name of the process specified by ProcessSerialNumberPtr.
	
	The string pointed to by pProcessName will be untouched if the process
	can't be found. 

	pPSN			input:	The process whose name you want (NULL for current)
	pProcessName	input:	Pointer to a Str31 for the process name.
					output:	The process name.
	
	RESULT CODES
	____________
	noErr			   0	No error
	paramErr		 �50	Process serial number is invalid
	____________
*/
extern pascal	OSStatus MoreProcGetProcessName(
								const ProcessSerialNumberPtr pPSN,
								StringPtr pProcessName);
/******************************************************************************
	Returns information about the process specified by ProcessSerialNumberPtr.
	
	pProcessType and pCreator will be untouched if the process
	can't be found. 

	pPSN				input:	The process whose info you want (NULL for current).
	pProcessType		output:	The process's type.
	pCreator			output:	The process's signature.
	
	RESULT CODES
	____________
	noErr			   0	No error
	paramErr		 �50	Process serial number is invalid
	____________
*/
extern pascal	OSStatus	MoreProcGetProcessTypeSignature(
						const ProcessSerialNumberPtr pPSN,
						OSType *pProcessType,
						OSType *pCreator);
/******************************************************************************
	Returns the FSSpec for the current process
	
	pFSSpec				output:	The process's FSSpec.
	
	RESULT CODES
	____________
	noErr			   0	No error
	paramErr		 �50	pFSSpec is NULL
	____________
*/
extern pascal OSStatus MoreProcGetCurrentProcessFSSpec(FSSpec *pFSSpec);
/******************************************************************************
	Returns the FSSpec for the process specified by ProcessSerialNumberPtr.
	
	pPSN				input:	The process's whose file you want (NULL for current).
	pAppFile			output:	The process's appFile.
	
	RESULT CODES
	____________
	noErr			   0	No error
	paramErr		 �50	Process serial number is invalid (or pPSN or pAppFile is NULL)
	____________
*/
extern pascal OSStatus MoreProcGetProcessAppFile(const ProcessSerialNumber *pPSN,FSSpec *pAppFile);
/******************************************************************************
	Returns the Process Information for the process specified by pPSN.
	
	pPSN				input:	The process's Serial Number
	pPIR				output:	The process's Information.
	
	RESULT CODES
	____________
	noErr			   0	No error
	paramErr		 �50	Process serial number is invalid (or pPSN or pPIR is NULL)
	____________
*/
extern pascal OSStatus MoreProcGetProcessInformation(const ProcessSerialNumber *pPSN,ProcessInfoRec *pPIR);
/******************************************************************************
	Returns true if the process specified by pPSN is a background only application.
	
	pPSN				input:	The process's Serial Number
	pIsBOA				output:	The process's Information.
	
	RESULT CODES
	____________
	noErr			   0	No error
	paramErr		 �50	Process serial number is invalid (or pPSN or pIsBOA is NULL)
	____________
*/
extern pascal OSStatus MoreProcIsProcessBackgroundOnly(const ProcessSerialNumber *pPSN,Boolean *pIsBOA);

//*****************************************************************************
// Returns true if the process specified by pPSN is the front process.
// Pass NULL to pPSN to test the current process.

extern pascal Boolean MoreProcIsProcessAtFront(const ProcessSerialNumber *pPSN);

#ifdef __cplusplus
	}
#endif
