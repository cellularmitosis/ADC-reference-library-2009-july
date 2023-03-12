/*
	File:		main.c
	
	Description:	This file is the demo file for the SimplePing API and sample.
        
	Author:		Chad Jones 

	Copyright:  	Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
        10/3/03		Chad Jones	Added comment indicating sample requires build tools 
                                        ProjectBuilder 2.1 or later
*/

//Note this sample requires Mac OS X 10.2.x or later and 
//ProjectBuilder version 2.1 or later.

#include "SimplePing.h"

int main(int argc, char* argv[])
{
    //Address can either be domain name style (e.g. www.yahoo.com) or IP style (e.g. 17.11.203.2)
    char* defaultAddressToPing = "www.google.com";
    char* remoteHostToPing;
    const int kNumberOfPacketsToSend = 4;
    const int kPingTimeoutInSeconds = 2; //timeout we wait for responses after each packet sent
    int error;
    
    if (argc == 2) 
    {
        //address was provided in input.  Use address provided
        remoteHostToPing = argv[1];
    }
    else 
    {
        //no address was provided use default (www.google.com).
        remoteHostToPing = defaultAddressToPing;
    }
    
    /* Here we are calling SimplePing which does all the work of pinging the remote host for us.
     * First Argument: The host to ping expressed as a C-String.  On calling SimplePing this
     *    variable will hold a description of the host as a C-String.  The string can either be
     *    a hostname (e.g. "www.google.com") or an IP address (e.g. "17.203.23.111")
     * Second Argument: The number of ping packets to send to the remote host before stopping.
     *    Also, for each ping packet simple ping will show some output on if there was a 
     *    reply, round trip time, etc.  
     * Third Argument: The time to wait for responses to the ping before stopping.  Note we will
     *    wait for responses even after a successful response has been received up until 
     *    the timeout has occurred
     * Forth Argument: An integer indicating if we want to return immediately after getting the desired
     *    response or if we wait for the timeout (even after a successful response) to see if any other
     *    packets come.  If the integer is zero then we will wait for the timeout and non-zero indicates
     *    we want to return immediately.  Here we pass zero to wait for the full timeout.
     * Return Value: This is an UNIX integer error code.  On success the return value will be zero.  On
     *    failure the value will be a UNIX error code.  You can find the list of UNIX error codes
     *    and their meanings in /usr/include/sys/errno.h
     */ 
    error = SimplePing(remoteHostToPing, kNumberOfPacketsToSend, kPingTimeoutInSeconds, 0);
    
    //If there was an error return one instead of zero.
    if (error != 0)
    {
        return(1);
    }
    
    return(0);
}
