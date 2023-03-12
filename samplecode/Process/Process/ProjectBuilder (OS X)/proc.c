/*
	File:		proc.c

	Contains:	A simple demonstration of the Process Manager API.

	Written by: James "im" Beninghaus	

	Copyright:	Copyright © 1985-1999 by Apple Computer, Inc., All Rights Reserved.

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
                7/2003		MK	Updated for Project Builder
				7/27/1999	KG	Updated for Metrowerks Codewarror Pro 2.1
                                fixed I/O printf formatting bug
				

*/


#include <Carbon/Carbon.h>
#include	<stdio.h>

int main (void) 
{
	
		auto OSErr osErr = noErr;
		auto ProcessSerialNumber process;
		auto ProcessInfoRec procInfo;
        auto Str255 procName;
		auto DateTimeRec launchDateTime;		
        auto FSSpec appFSSpec;
        char cstrProcName[34];
        unsigned int size;
	
	printf("Process Name                     Number             Type    Signature  Mode     Location Size     FreeMem  Launcher          LaunchDate          "                        
		"\n"
		"-------------------------------- -----------------  ----    ---------  -------- -------- -------- -------- ----------------- -------------------"
		"\n"
	);
	
	process.highLongOfPSN = kNoProcess;
	process.lowLongOfPSN  = kNoProcess;
	
	procInfo.processInfoLength = sizeof(ProcessInfoRec);
    procInfo.processName = procName;
    procInfo.processAppSpec = &appFSSpec;


    
	while (procNotFound != (osErr = GetNextProcess(&process))) {
		if (noErr == (osErr = GetProcessInformation(&process, &procInfo))) {
			SecondsToDate(procInfo.processLaunchDate, &launchDateTime);
            
            //copy process name to c string
            size = (unsigned int) procName[0];
            memcpy(cstrProcName, procName + 1, size);
            cstrProcName[size] = '\0';
            
			printf(
				"%-33s %08lx.%08lx '%c%c%c%c'  '%c%c%c%c'  %08lx %08lx %08lx %08lx %08lx.%08lx %2d/%2d/%2d %2d:%02d:%02d\n",
				cstrProcName,
				procInfo.processNumber.highLongOfPSN,
				procInfo.processNumber.lowLongOfPSN,
				((char *) &procInfo.processType)[0],
				((char *) &procInfo.processType)[1],
				((char *) &procInfo.processType)[2],
				((char *) &procInfo.processType)[3],
				((char *) &procInfo.processSignature)[0],
				((char *) &procInfo.processSignature)[1],
				((char *) &procInfo.processSignature)[2],
				((char *) &procInfo.processSignature)[3],
				procInfo.processMode,
				(unsigned long) procInfo.processLocation,
				procInfo.processSize,
				procInfo.processFreeMem,
				procInfo.processLauncher.highLongOfPSN,
				procInfo.processLauncher.lowLongOfPSN,
				launchDateTime.month,
				launchDateTime.day,
				launchDateTime.year,
				launchDateTime.hour,
				launchDateTime.minute,
				launchDateTime.second
			);
		}
	}
    
    return 0;
}