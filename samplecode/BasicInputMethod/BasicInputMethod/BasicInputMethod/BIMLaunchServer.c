/*
    File:	 BIMLaunchServer.c
	
    Description: Handles the Basic Server process, launching it if it is not already running.

    Author:	 SC

    Copyright: 	 © Copyright 2000-2001 Apple Computer, Inc. All rights reserved.

    Disclaimer:	 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

                 2001/04/20	SC	The server app is now located inside the text services
                                        component bundle. Changed the code to launch the server
                                        appropriately.
                 2001/03/09	SC	In order to avoid process switching the first time the
                                        server is launched, changed the code yet again to launch
                                        the server using LaunchApplication() and removed the code
                                        that sends an Apple event to the Finder.
                                        Changed the code that locates the server to use FindFolder
                                        instead of finding it using our refNum.
		 2000/08/02	SC	Changed again to launch the server by sending an Apple
					event to the Finder.
		 2000/08/01	SC	Added a workaround that brings our process into the
					foreground after launching the server
		 2000/07/31	SC	Added code to launch the server by sending an Apple
					event to the Finder (commented out)
					Added a timeout feature for launching
		 2000/07/28	SC	Changed to include Carbon.h
		 2000/07/09	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BIM.h"
#include "BIMClientServer.h"

//  Local functions

static OSStatus BIMLaunchWithFSSpec( FSSpec *fileSpec );

/************************************************************************************************
*
*  BIMLaunchServer
*
*  Checks to see if the server is running. If not, it is launched. Control does not return from
*  this function until the launch is complete.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMLaunchServer( void )
{
    OSStatus		result;
    long		timeout;
    CFBundleRef		myComponentBundle;
    CFMessagePortRef	serverPortRef;
    CFURLRef		sharedSupportURL;
    CFURLRef		serverURL;
    FSRef		serverFSRef;
    FSSpec		serverFileSpec;

    serverPortRef = NULL;
    sharedSupportURL = NULL;
    serverURL = NULL;

    //  Obtain a reference to the server port. If it exists, then the server is already up and
    //  running so just return.
    
    serverPortRef = CFMessagePortCreateRemote( NULL, CFSTR(kBasicServerPortName) );
    if( serverPortRef ) {
        CFRelease( serverPortRef );
        return noErr;
    }
    
    //  The server is not running, so launch it now.
    
    //  Obtain a reference to our text service component bundle. We can't use CFBundleGetMainBundle
    //  because we are running inside another application's context so we find our bundle using the
    //  bundle identifier in our Info.plist file (this is specified in the target settings for
    //  BasicInputMethod).

    myComponentBundle = CFBundleGetBundleWithIdentifier( CFSTR( "com.apple.dts.BasicInputMethod" ) );
    
    //  If we got a reference to the bundle, locate the "SharedSupport" directory inside the bundle.
    
    if( myComponentBundle )
        sharedSupportURL = CFBundleCopySharedSupportURL( myComponentBundle );
    
    //  If we found the "SharedSupport" directory, append the name of our server application to the
    //  URL so we can identify it.
    
    if( sharedSupportURL )
        serverURL = CFURLCreateCopyAppendingPathComponent( NULL, sharedSupportURL,
                                                           CFSTR( "BasicServer.app" ), FALSE );
    
    //  We need to do some extra work here. Since LaunchApplication only takes an FSSpec as a parameter,
    //  me must convert the server URL into an FSRef and then convert it into an FSSpec. Whew!
    
    if( serverURL && CFURLGetFSRef( serverURL, &serverFSRef) )
	result = FSGetCatalogInfo( &serverFSRef, kFSCatInfoNone, NULL, NULL, &serverFileSpec, NULL );
    else
        result = -2;

    //  Launch the server application.
    
    if( result == noErr )
        result = BIMLaunchWithFSSpec( &serverFileSpec );
    
    //  Wait for the server to come online (timeout in 20 seconds -- this is arbitrary and may need
    //  adjusting for certain situations such as launching over a network)

    timeout = TickCount() + 1200;
    while( result == noErr && serverPortRef == NULL ) {

        //  Get a reference to the server port.

	serverPortRef = CFMessagePortCreateRemote( NULL, CFSTR(kBasicServerPortName) );

        if( serverPortRef == NULL && TickCount() > timeout )
            result = -1;
    }

    //	Dispose of everything.
    
    if( serverPortRef )
        CFRelease( serverPortRef );
    if( serverURL )
        CFRelease( serverURL );
    if( sharedSupportURL )
        CFRelease( sharedSupportURL );
        
    if( result == -1 )
        fprintf( stderr, "A timeout occured while trying to launch the UI server.\n" );
    else if( result == -2 )
        fprintf( stderr, "Unable to locate the SharedSupport directory inside the component.\n" );
    else if( result )
        fprintf( stderr, "An error of type %d occured while trying to launch the UI server.\n", (int)result );
        
    return result;
}

/************************************************************************************************
*
*  BIMLaunchWithFSSpec
*
*  Launches the application with the given fileSpec.
*
*	In	fileSpec		A pointer to the file specification record of the
*					application to launch.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

static OSStatus BIMLaunchWithFSSpec( FSSpec *fileSpec )
{
    LaunchParamBlockRec		launchParams;
    
    launchParams.launchBlockID = extendedBlock;
    launchParams.launchEPBLength = extendedBlockLen;
    launchParams.launchFileFlags = 0;
    launchParams.launchControlFlags = launchNoFileFlags + launchContinue + launchDontSwitch;
    launchParams.launchAppSpec = fileSpec;
    launchParams.launchAppParameters = NULL;
    return LaunchApplication( &launchParams );
}