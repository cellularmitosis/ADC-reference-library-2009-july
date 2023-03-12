/*
	File:		Demo.c
	
	Description:	This file demonstrates the use of the GetPID calls.
	
	Author:		Chad Jones 

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
*/

#include "GetPID.h"

void DemoGetAllPIDsForProcessName(const char* ProcessName)
{
    const int kPIDArrayLength = 100;

    pid_t MyArray [kPIDArrayLength];
    unsigned int NumberOfMatches;
    int Counter, Error;

    /* Here we are calling the function GetAllPIDsForProcessName which wil give us the PIDs
     * of the process name we pass.  
     * First Argument: The BSD process name of the process we want to lookup.  In this case the
     *		the process name passed to us.
     * Second Argument: A preallocated array of pid_t.  This is where the PIDs of matching processes
     *		will be placed on return.  This case our array we already made.
     * Third Argument: The number of pid_t entries located in the array of pid_t (argument 2).  Thus
     *  	this is the size of the array in number of PIDs in the array.  Size here is the size of
     * 		the array we already allocated (kPIDArrayLength).
     * Forth Argument: On calling this is a pointer to a pre-allocated unsigned integer.  On return
     * 		this will hold the number of PIDs placed into the pid_t array (array passed in argument 2).
     * Fifth Argument: A pointer to a pre-allocated integer.  On return this will be the sysctl error returned.
     *		Note here we can also pass NULL in which case this argument will be ignored (as we do here).
     * Return Value: An error indicating success (zero result) or failure (non-zero) result.
     * 	   	Also, if return value is -1 then the requested process doesn't exist (not found).
     *		If the return value is -5 then the buffer we passed wasn't big enough.  (see function description for
     *    	for details).
     */
    Error = GetAllPIDsForProcessName(ProcessName, MyArray, kPIDArrayLength, &NumberOfMatches, NULL);

    if (Error == 0) //success!  Print out info
    {
        printf("%s PID(s):    \t", ProcessName);
        
        for (Counter = 0 ; Counter < NumberOfMatches ; Counter++)
        {
            printf("%d", (int) MyArray[Counter]);
            
            if (Counter != NumberOfMatches - 1)
                {printf(", ");}
        } 
    }
    else if (Error == -1) //can't find process
    {
        printf("Process Not Found:\t\"%s\"", ProcessName);
    }
    else //other error
    {
        printf("Other Error Getting PID.  Error Code: %d\n", Error);
    }
    printf("\n");
}

int main()
{

    // --- Demo DemoGetAllPIDsForProcessName --- //

    printf("Demo DemoGetAllPIDsForProcessName: \n");
    //Note the BSD name is case sensitive for matching
    DemoGetAllPIDsForProcessName("Finder");

    //On a typical OSX machine there are several nfsiod processes running.  Printing out their PIDs.
    DemoGetAllPIDsForProcessName("nfsiod");

    DemoGetAllPIDsForProcessName("cron");

    DemoGetAllPIDsForProcessName("Bogus Process!");


    // --- Demo the convinence call GetPIDForProcessName --- //
    
    printf("------------\nDemo GetPIDForProcessName:\n");

    /* Here a PID of -1 returned indicates the that there was an error getting a single PID.  This can happen
     * if the process doesn't exist or even if there is more than one process with the given Process Name.
     * In any case if you get an error result you should probably call GetAllPIDsForProcessName for more
     * information (or to get the PID list)
     */
    printf("Finder PID: %d\n", GetPIDForProcessName("Finder"));

    //On a typical OSX machine there are several nfsiod processes running. 
    printf("nfsiod PID: %d\n", GetPIDForProcessName("nfsiod"));

    printf("cron PID: %d\n", GetPIDForProcessName("cron"));

    printf("BogusProcess! PID: %d\n", GetPIDForProcessName("BogusProcess!"));
    
    return(0);
}

