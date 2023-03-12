/*
	File:		TestMoreOSUtils.c

	Contains:	Simple test program for the MoreOSUtils library.

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

$Log: TestMoreOSUtils.c,v $
Revision 1.5  2002/11/08 23:51:52         
Convert nil to NULL.

Revision 1.4  2001/11/07 15:58:32         
Tidy up headers, add CVS logs, update copyright.


         <3>     28/9/01    Quinn   Added test code for MoreCSCopyUserName and
                                    MoreCSCopyMachineName. Also made build with Carbon and Mach-O
                                    targets.
         <2>     16/3/99    Quinn   Added tests for InterfaceDisableLib.
         <1>      1/3/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Standard Mac OS interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <CFString.h>
#endif

// Standard C interfaces

#include <stdio.h>

// MIB Prototypes

#include "MoreOSUtils.h"

/////////////////////////////////////////////////////////////////

static UInt16 gCodeSnippet[16];

static void PrintCFString(const char *stringType, CFStringRef str)
{
	char tmpStr[256];
	
	if (str == NULL) {
		printf("%s = <NULL>\n", stringType);
	} else {
		if ( CFStringGetCString(str, tmpStr, sizeof(tmpStr), kCFStringEncodingMacRoman) ) {
			fprintf(stderr, "%s = %s\n", stringType, tmpStr);
		} else {
			fprintf(stderr, "%s = <not convertable to MacRoman>\n", stringType);
		}
	}
}

void main(void)
{
	OSStatus err;
	
	printf("Hello Cruel World!\n");
	printf("TestMoreOSUtils\n");
	printf("-- A simple test program for the MoreOSUtils library.\n");
	
	printf("Testing Cache Flushing...\n");
	
	err = MakeDataPowerPCExecutable(gCodeSnippet, sizeof(gCodeSnippet));
	if (err == noErr) {
		// In the classic 68K case, this second call exercises a different
		// code path from the first.
		err = MakeDataPowerPCExecutable(gCodeSnippet, sizeof(gCodeSnippet));
	}
	#if !TARGET_API_MAC_CARBON
		if (err == noErr) {
			err = MakeData68KExecutable(gCodeSnippet, sizeof(gCodeSnippet));
		}
	#endif

	#if TARGET_CPU_68K && !TARGET_RT_MAC_CFM
		TermMoreOSUtils();
	#endif
	
	#if !TARGET_API_MAC_CARBON
		printf("Testing Interrupt Disable/Enable...\n");
		
		{
			UInt16 initialMask;
			UInt16 maskIndex;
			
			// Save the initial interrupt mask for later restoration.
			
			initialMask = GetInterruptMask();
			
			// Set the interrupt mask to every legal value and make
			// sure it sticks.
			
			for (maskIndex = 0; maskIndex <= 7; maskIndex++) {
				(void) SetInterruptMask(maskIndex);
				if (GetInterruptMask() != maskIndex) {
					printf("••• Failed to set mask to %d.\n", maskIndex);
				}
			}
			
			// Check that the "set returns old value" behaviour.
			
			SetInterruptMask(7);
			if (SetInterruptMask(5) != 7) {
				printf("••• SetInterruptMask failed to return old values.\n");
			}
			
			// Restore the initial interrupt mask and make sure that works.
			
			SetInterruptMask(initialMask);
			if (GetInterruptMask() != initialMask) {
				printf("••• Failed to restore interrupt mask.\n");
			}
		}
	#endif
	
	#if TARGET_API_MAC_CARBON
		{
			int i;
			
			// We run this loop more than once because it exercises the 
			// workaround for [2665708].
			
			for (i = 0; i < 10; i++) {
				CFStringRef shortUser;
				CFStringRef longUser;
				CFStringRef machine;
				
				shortUser = MoreCSCopyUserName(true);
				longUser  = MoreCSCopyUserName(false);
				machine   = MoreCSCopyMachineName();
				
				PrintCFString("Short user name", shortUser);
				PrintCFString("Long user name", longUser);
				PrintCFString("Machine name", machine);
			
				if (shortUser != NULL) {
					CFRelease(shortUser);
				}
				if (longUser != NULL) {
					CFRelease(longUser);
				}
				if (machine != NULL) {
					CFRelease(machine);
				}
			}

		}		
	#endif
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}
