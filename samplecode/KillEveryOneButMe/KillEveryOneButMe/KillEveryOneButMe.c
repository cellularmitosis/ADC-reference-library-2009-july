/*
*	File:		KillEveryOneButMe.c of KillEveryOneButMe
* 
*	Contains:	This sample gives developers a simple demonstration of how to quit other running processes
*				on the system.  This code is intended to be used in kiosks or for demonstrations, etc.
*				Do not abuse the power that this simple application demonstrates.
*
*				This application can be run within the Xcode IDE.
*				However, when Xcode itself is quit by this application then your application
*				will quit as well (not giving the expected results).
*				You can set the MACRO DONTKILLXCODE to 1 to prevent Xcode from being killed.
*
*				This sample lists the running processes on the system and offers two menu items.
*				One allows the user to kill all other running programs, the other will kill all other
*				running programs but sparing the Finder.
*
*				On Mac OS X, some processes such as Loginwindow will not be quit because they are required
*				for Mac OS X to operate properly.
*
*				This application in no way forces applications to quit.
*				It simply offers them an Apple Event telling them to quit.
*				They can remain running (for example if they have unsaved documents) by ignoring the Apple Event.
*	
*	Version:	3.0
* 
*	Created:	8/18/05
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*				("Apple") in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms.  If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under AppleÍs
*				copyrights in this original Apple software (the "Apple Software"), to use,
*				reproduce, modify and redistribute the Apple Software, with or without
*				modifications, in source and/or binary forms; provided that if you redistribute
*				the Apple Software in its entirety and without modifications, you must retain
*				this notice and the following text and disclaimers in all such redistributions of
*				the Apple Software.  Neither the name, trademarks, service marks or logos of
*				Apple Computer, Inc. may be used to endorse or promote products derived from the
*				Apple Software without specific prior written permission from Apple.  Except as
*				expressly stated in this notice, no other rights or licenses, express or implied,
*				are granted by Apple herein, including but not limited to any patent rights that
*				may be infringed by your derivative works or by other works in which the Apple
*				Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*				COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include "KillEveryOneButMe.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* CFStringCreateWithProcessList() 
*
* Purpose:  called to create a CFStringRef containing the list of the currently running processes
*
* Inputs:   none
*
* Returns:  CFStringRef			- returns NULL if the string can't be created
*/
CFStringRef CFStringCreateWithProcessList()
{
	OSStatus status;
    ProcessSerialNumber currentProcessPSN = {kNoProcess, kNoProcess};

	CFMutableStringRef theCFString = CFStringCreateMutable(NULL, 0);
	require(theCFString != NULL, CFStringCreateMutable);
	
    do
    {
        status = GetNextProcess(&currentProcessPSN);
    
        if (status == noErr)
        {
			ProcessInfoRec infoRec;
			infoRec.processInfoLength = sizeof(ProcessInfoRec);
			infoRec.processName = NULL;
			infoRec.processAppSpec = NULL;
			
			if (GetProcessInformation(&currentProcessPSN, &infoRec) == noErr)
			{
				CFStringRef procType = CreateTypeStringWithOSType(infoRec.processType);
				if (procType != NULL)
				{
					CFStringAppend(theCFString, procType);
					CFRelease(procType);
					CFStringAppend(theCFString, CFSTR("  "));
				}

				CFStringRef procSign = CreateTypeStringWithOSType(infoRec.processSignature);
				if (procSign != NULL)
				{
					CFStringAppend(theCFString, procSign);
					CFRelease(procSign);
					CFStringAppend(theCFString, CFSTR("  "));
				}
			}

			CFStringRef processName = NULL;
			CopyProcessName(&currentProcessPSN, &processName);
			if (processName != NULL)
			{
				CFStringAppend(theCFString, processName);
				CFRelease(processName);
			}

			CFStringAppend(theCFString, CFSTR("\n"));
        }
    } while (status == noErr);

CFStringCreateMutable:

	return theCFString;
}   // CFStringCreateWithProcessList

/*****************************************************
*
* SendQuitAppleEventToApplication(ProcessToQuit) 
*
* Purpose:  called to send a 'quit' AppleEvent to the process passed as parameter
*
* Inputs:   none
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus SendQuitAppleEventToApplication(ProcessSerialNumber ProcessToQuit)
{
    OSStatus status;
    AEDesc targetProcess = {typeNull, NULL};
    AppleEvent theEvent = {typeNull, NULL};
    AppleEvent eventReply = {typeNull, NULL}; 

    status = AECreateDesc(typeProcessSerialNumber, &ProcessToQuit, sizeof(ProcessToQuit), &targetProcess);
	require_noerr(status, AECreateDesc);
    
    status = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &targetProcess, kAutoGenerateReturnID, kAnyTransactionID, &theEvent);
	require_noerr(status, AECreateAppleEvent);
    
    status = AESend(&theEvent, &eventReply, kAENoReply + kAEAlwaysInteract, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
	require_noerr(status, AESend);
    
AESend:
AECreateAppleEvent:
AECreateDesc:

	AEDisposeDesc(&eventReply); 
    AEDisposeDesc(&theEvent);
	AEDisposeDesc(&targetProcess);

    return(status);
}   // SendQuitAppleEventToApplication


#define DONTKILLXCODE 1


/*****************************************************
*
* KillEveryone(KillFinderToo) 
*
* Purpose:  called to kill all safely killable applications, leaving alone those who are necessary
*			to the good working of Mac OS X
*
* Inputs:   KillFinderToo       - if true, we also kill the Finder, if false we don't
*
* Returns:  OSStatus			- error code (0 == no error) 
*/
OSStatus KillEveryone(Boolean KillFinderToo)
{
    ProcessSerialNumber nextProcessToKill = {kNoProcess, kNoProcess};
    ProcessSerialNumber ourPSN;
    OSStatus status;
    ProcessInfoRec infoRec;

    Boolean processIsFinder;
    Boolean processIsUs;
    Boolean specialMacOSXProcessWhichWeShouldNotKill;
    Boolean finderFound = false;

    GetCurrentProcess(&ourPSN);
    
    do
    {
        status = GetNextProcess(&nextProcessToKill);
        
        if (status == noErr)
        {
            //First check if its us
            SameProcess(&ourPSN, &nextProcessToKill, &processIsUs);
            
            if (processIsUs == false)
            {
                infoRec.processInfoLength = sizeof(ProcessInfoRec);
                infoRec.processName = NULL;
                infoRec.processAppSpec = NULL;
    
                if (GetProcessInformation(&nextProcessToKill, &infoRec) == noErr)
                {
                    processIsFinder = false;
					if (infoRec.processSignature == 'MACS' && infoRec.processType == 'FNDR') 
					{
						processIsFinder = true;
					}
                    
                    //since this is Mac OS X we need to make sure we don't quit certain applications
					specialMacOSXProcessWhichWeShouldNotKill = false;
					if (infoRec.processSignature == 'lgnw' && infoRec.processType == 'APPL')
					{
						//don't want to quit loginwindow on Mac OS X or system will logout
						specialMacOSXProcessWhichWeShouldNotKill = true;
					}
					else if (infoRec.processSignature == 'dock' && infoRec.processType == 'APPL')
					{
						//don't want to quit Dock on Mac OS X it provides important support (for example Command+Tab switching)
						specialMacOSXProcessWhichWeShouldNotKill = true;
					}
					else if (infoRec.processSignature == 'syui' && infoRec.processType == 'APPL')
					{
						//don't want to quit the SystemUI server on Mac OS X this offers important system support
						specialMacOSXProcessWhichWeShouldNotKill = true;
					}
#if DONTKILLXCODE
					else if (infoRec.processSignature == 'xcde' && infoRec.processType == 'APPL')
					{
						//don't want to quit Xcode so that KillEveryOneButMe stays alive when launched from Xcode
						specialMacOSXProcessWhichWeShouldNotKill = true;
					}
#endif
					else if (infoRec.processSignature == 'bbox' && infoRec.processType == 'APPL')
					{
						//don't want to quit the "special" bluebox envionment process directly (as it can cause havoc).  
						//Instead this process will quit indirectly when the "real" Classic envonment gets its quit event
						specialMacOSXProcessWhichWeShouldNotKill = true;
					}
					else if (infoRec.processSignature == 0 && infoRec.processType == 0)
					{
						//this might be coreaudio. don't want to quit it.
						specialMacOSXProcessWhichWeShouldNotKill = true;
					}
                    
                    if (((processIsFinder == false) || (KillFinderToo == true)) && (specialMacOSXProcessWhichWeShouldNotKill == false))
                    {
                        //ignore return value
                        (void)SendQuitAppleEventToApplication(nextProcessToKill);
                    }
                }
            }
        }
    }
    while (status == noErr);
}   // KillEveryone

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *
