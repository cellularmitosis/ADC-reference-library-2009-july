/*
	File:       SampleApp.m

    Contains:   Application side of the example of how to use BetterAuthorizationSampleLib.

    Written by: DTS

    Copyright:  Copyright (c) 2007 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple's
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of 
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple, Inc. may be used to endorse or promote products derived from the
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

 */
 
#include <unistd.h>
#include <netinet/in.h>

#import <Cocoa/Cocoa.h>

#include "BetterAuthorizationSampleLib.h"

#include "SampleCommon.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Globals

static AuthorizationRef gAuth;

/////////////////////////////////////////////////////////////////
#pragma mark ***** Objective-C Wrapper

// Our trivial application object, SampleApp, is instantiated by our nib.  It 
// has four actions, three for the buttons and one for the Destroy Rights menu item. 
// It has a two outlets, one pointing to the text view where we log our results 
// and the other referencing the "Force failure" checkbox.

@interface SampleApp : NSObject {
    NSTextView *    textView;
    NSButton *      forceFailure;
}

- (IBAction)doGetVersion:(id)sender;
- (IBAction)doGetUIDs:(id)sender;
- (IBAction)doLowNumberedPorts:(id)sender;
- (IBAction)destroyRights:(id)sender;

@end

@implementation SampleApp

- (IBAction)doGetVersion:(id)sender
    // Called when the user clicks the "GetVersion" button.  This is the simplest 
    // possible BetterAuthorizationSample operation, in that it doesn't handle any failures.
{
    OSStatus        err;
    NSString *      bundleID;
    NSDictionary *  request;
    CFDictionaryRef response;

    response = NULL;
    
    // Create our request.  Note that NSDictionary is toll-free bridged to CFDictionary, so 
    // we can use an NSDictionary as our request.  Also, if the "Force failure" checkbox is 
    // checked, we use the wrong command ID to deliberately cause an "unknown command" error 
    // so that we can test that code path.
    
    if ( [forceFailure state] == 0 ) {
        request = [NSDictionary dictionaryWithObjectsAndKeys:@kSampleGetVersionCommand, @kBASCommandKey, nil];
    } else {
        request = [NSDictionary dictionaryWithObjectsAndKeys:@"Utter Gibberish", @kBASCommandKey, nil];
    }
    assert(request != NULL);
    
    bundleID = [[NSBundle mainBundle] bundleIdentifier];
    assert(bundleID != NULL);
    
    // Execute it.
    
	err = BASExecuteRequestInHelperTool(
        gAuth, 
        kSampleCommandSet, 
        (CFStringRef) bundleID, 
        (CFDictionaryRef) request, 
        &response
    );
    
    // If the above went OK, it means that the IPC to the helper tool worked.  We 
    // now have to check the response dictionary to see if the command's execution 
    // within the helper tool was successful.  For the GetVersion command, this 
    // is unlikely to ever fail, but we should still check. 
    
    if (err == noErr) {
        err = BASGetErrorFromResponse(response);
    }
    
    // Log our results.
    
    if (err == noErr) {
        [textView insertText:
            [NSString stringWithFormat:@"version = %@\n", 
                [(NSDictionary *)response objectForKey:@kSampleGetVersionResponse]
            ]
        ];
    } else {
        [textView insertText:
            [NSString stringWithFormat:@"Failed with error %ld.\n", (long) err]
        ];
    }
    
    if (response != NULL) {
        CFRelease(response);
    }
}

- (IBAction)doGetUIDs:(id)sender
    // Called when the user clicks the "GetUIDs" button.  This is a typical BetterAuthorizationSample 
    // privileged operation implemented in Objective-C.
{
    OSStatus        err;
    BASFailCode     failCode;
    NSString *      bundleID;
    NSDictionary *  request;
    CFDictionaryRef response;

    response = NULL;
    
    // Create our request.  Note that NSDictionary is toll-free bridged to CFDictionary, so 
    // we can use an NSDictionary as our request.
    
    request = [NSDictionary dictionaryWithObjectsAndKeys:@kSampleGetUIDsCommand, @kBASCommandKey, nil];
    assert(request != NULL);
    
    bundleID = [[NSBundle mainBundle] bundleIdentifier];
    assert(bundleID != NULL);
    
    // Execute it.
    
	err = BASExecuteRequestInHelperTool(
        gAuth, 
        kSampleCommandSet, 
        (CFStringRef) bundleID, 
        (CFDictionaryRef) request, 
        &response
    );
    
    // If it failed, try to recover.
    
    if ( (err != noErr) && (err != userCanceledErr) ) {
        int alertResult;
        
        failCode = BASDiagnoseFailure(gAuth, (CFStringRef) bundleID);

        // At this point we tell the user that something has gone wrong and that we need 
        // to authorize in order to fix it.  Ideally we'd use failCode to describe the type of 
        // error to the user.
            
        alertResult = NSRunAlertPanel(@"Needs Install", @"BAS needs to install", @"Install", @"Cancel", NULL);
        
        if ( alertResult == NSAlertDefaultReturn ) {
            // Try to fix things.
            
            err = BASFixFailure(gAuth, (CFStringRef) bundleID, CFSTR("InstallTool"), CFSTR("HelperTool"), failCode);

            // If the fix went OK, retry the request.
            
            if (err == noErr) {
                err = BASExecuteRequestInHelperTool(
                    gAuth, 
                    kSampleCommandSet, 
                    (CFStringRef) bundleID, 
                    (CFDictionaryRef) request, 
                    &response
                );
            }
        } else {
            err = userCanceledErr;
        }
    }
    
    // If all of the above went OK, it means that the IPC to the helper tool worked.  We 
    // now have to check the response dictionary to see if the command's execution within 
    // the helper tool was successful.
    
    if (err == noErr) {
        err = BASGetErrorFromResponse(response);
    }
    
    // Log our results.
    
    if (err == noErr) {
        [textView insertText:
            [NSString stringWithFormat:@"RUID = %@, EUID=%@\n", 
                [(NSDictionary *)response objectForKey:@kSampleGetUIDsResponseRUID],
                [(NSDictionary *)response objectForKey:@kSampleGetUIDsResponseEUID]
            ]
        ];
    } else {
        [textView insertText:
            [NSString stringWithFormat:@"Failed with error %ld.\n", (long) err]
        ];
    }
    
    if (response != NULL) {
        CFRelease(response);
    }
}

static OSStatus DoLowNumberedPorts(
	Boolean						forceFailure, 
	int							fdArray[]
)
    // This code shows how to do a typical BetterAuthorizationSample privileged operation 
	// in straight C.  In this case, it does the low-numbered ports operation, which 
    // returns three file descriptors that are bound to low-numbered TCP ports.
{
    OSStatus        err;
    Boolean         success;
    CFBundleRef     bundle;
    CFStringRef     bundleID;
    CFIndex         keyCount;
    CFStringRef     keys[2];
    CFTypeRef       values[2];
    CFDictionaryRef request;
    CFDictionaryRef response;
    BASFailCode     failCode;
    
    // Pre-conditions
    
    assert(fdArray != NULL);
    assert(fdArray[0] == -1);
    assert(fdArray[1] == -1);
    assert(fdArray[2] == -1);
    
    // Get our bundle information.
    
    bundle = CFBundleGetMainBundle();
    assert(bundle != NULL);
    
    bundleID = CFBundleGetIdentifier(bundle);
    assert(bundleID != NULL);
    
    // Create the request.  The request always contains the kBASCommandKey that 
    // describes the command to do.  It also, optionally, contains the 
	// kSampleLowNumberedPortsForceFailure key that tells the tool to always return 
	// an error.  The purpose of this is to test our error handling path (do we leak 
	// descriptors, for example). 
    
    keyCount = 0;
    keys[keyCount]   = CFSTR(kBASCommandKey);
    values[keyCount] = CFSTR(kSampleLowNumberedPortsCommand);
    keyCount += 1;
    
    if (forceFailure) {
        keys[keyCount]   = CFSTR(kSampleLowNumberedPortsForceFailure);
        values[keyCount] = kCFBooleanTrue;
        keyCount += 1;
    }
    
    request = CFDictionaryCreate(
		NULL, 
		(const void **) keys, 
		(const void **) values, 
		keyCount, 
		&kCFTypeDictionaryKeyCallBacks, 
		&kCFTypeDictionaryValueCallBacks
	);
    assert(request != NULL);
    
    response = NULL;
    
    // Execute it.

	err = BASExecuteRequestInHelperTool(
        gAuth, 
        kSampleCommandSet, 
        bundleID, 
        request, 
        &response
    );

    // If it failed, try to recover.

    if ( (err != noErr) && (err != userCanceledErr) ) {
        int alertResult;
        
        failCode = BASDiagnoseFailure(gAuth, bundleID);
        
        // At this point we tell the user that something has gone wrong and that we need 
        // to authorize in order to fix it.  Ideally we'd use failCode to describe the type of 
        // error to the user.
            
        alertResult = NSRunAlertPanel(@"Needs Install", @"BAS needs to install", @"Install", @"Cancel", NULL);
        
        if ( alertResult == NSAlertDefaultReturn ) {
            // Try to fix things.
            
            err = BASFixFailure(gAuth, (CFStringRef) bundleID, CFSTR("InstallTool"), CFSTR("HelperTool"), failCode);

            // If the fix went OK, retry the request.
            
            if (err == noErr) {
                err = BASExecuteRequestInHelperTool(
                    gAuth, 
                    kSampleCommandSet, 
                    bundleID, 
                    request, 
                    &response
                );
            }
        } else {
            err = userCanceledErr;
        }
    }

    // If all of the above went OK, it means that the IPC to the helper tool worked.  We 
    // now have to check the response dictionary to see if the command's execution within 
    // the helper tool was successful.
    
    if (err == noErr) {
        err = BASGetErrorFromResponse(response);
    }
    
    // Extract the descriptors from the response and copy them out to our caller.
    
    if (err == noErr) {
        CFArrayRef      descArray;
        CFIndex         arrayIndex;
        CFIndex         arrayCount;
        CFNumberRef     thisNum;
        
        descArray = (CFArrayRef) CFDictionaryGetValue(response, CFSTR(kBASDescriptorArrayKey));
        assert( descArray != NULL );
        assert( CFGetTypeID(descArray) == CFArrayGetTypeID() );
            
        arrayCount = CFArrayGetCount(descArray);
        assert(arrayCount == kNumberOfLowNumberedPorts);
        
        for (arrayIndex = 0; arrayIndex < kNumberOfLowNumberedPorts; arrayIndex++) {
            thisNum = CFArrayGetValueAtIndex(descArray, arrayIndex);
            assert(thisNum != NULL);
            assert( CFGetTypeID(thisNum) == CFNumberGetTypeID() );
            
            success = CFNumberGetValue(thisNum, kCFNumberIntType, &fdArray[arrayIndex]);
            assert(success);
        }
    }
    
    if (response != NULL) {
        CFRelease(response);
    }
    
    assert( (err == noErr) == (fdArray[0] >= 0) );
    assert( (err == noErr) == (fdArray[1] >= 0) );
    assert( (err == noErr) == (fdArray[2] >= 0) );
    
    return err;
}

- (IBAction)doLowNumberedPorts:(id)sender
    // Called when the user clicks the "LowNumberedPorts" button.  It calls the helper 
    // tool to open three low-numbered ports (which can't otherwise by opened by 
	// non-privileged code).
    //
    // The code is divided into two.  This core code is DoLowNumberedPorts; its goal 
	// is to show how to use the BetterAuthorizationSample library from plain C.  This 
	// routine is an Objective-C wrapper that glues DoLowNumberedPorts into the rest 
	// of the Cocoa application.
{
    OSStatus    err;
    int         junk;
    int         descriptors[kNumberOfLowNumberedPorts] = { -1, -1, -1 };
    uint16_t    ports[kNumberOfLowNumberedPorts];
    int         portIndex;
    
    // Call the C code to do the real work.

    err = DoLowNumberedPorts( [forceFailure state] != 0, descriptors );

    // Log our results.
    
    if (err == noErr) {
        // Get the port numbers for each descriptor.
        
        for (portIndex = 0; portIndex < kNumberOfLowNumberedPorts; portIndex++) {
            int                 sockErr;
            struct sockaddr_in  boundAddr;
            socklen_t           boundAddrLen;
            
            memset(&boundAddr, 0, sizeof(boundAddr));
            boundAddrLen = sizeof(boundAddr);

            sockErr = getsockname(descriptors[portIndex], (struct sockaddr *) &boundAddr, &boundAddrLen);
            assert(sockErr == 0);
            assert(boundAddrLen == sizeof(boundAddr));
            ports[portIndex] = ntohs(boundAddr.sin_port);
        }
    
        // Log it.
        
        [textView insertText:
            [NSString stringWithFormat:@"ports[0] = %u, port[1] = %u, port[2] = %u\n", 
				(unsigned int) ports[0], 
				(unsigned int) ports[1], 
				(unsigned int) ports[2]
			]
        ];
        
        // Close the descriptors.
        
        for (portIndex = 0; portIndex < kNumberOfLowNumberedPorts; portIndex++) {
            junk = close(descriptors[portIndex]);
            assert(junk == 0);
        }
    } else {
        [textView insertText:
            [NSString stringWithFormat:@"Failed with error %ld.\n", (long) err]
        ];
    }
}

- (IBAction)destroyRights:(id)sender
    // Called when the user chooses the "Destroy Rights" menu item.  This is just a testing 
    // convenience; it allows you to destroy the credentials that are stored in the cache 
    // associated with gAuth, so you can force the system to ask you for a password again.  
    // However, this isn't as convenient as you might think because the credentials might 
    // be cached globally.  See DTS Q&A 1277 "Security Credentials" for the gory details.
    //
    // <http://developer.apple.com/qa/qa2001/qa1277.html>
{
    OSStatus    junk;
    
    // Free gAuth, destroying any credentials that it has acquired along the way. 
    
    junk = AuthorizationFree(gAuth, kAuthorizationFlagDestroyRights);
    assert(junk == noErr);
    gAuth = NULL;

    // Recreate it from scratch.
    
    junk = AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &gAuth);
    assert(junk == noErr);
    assert( (junk == noErr) == (gAuth != NULL) );    
}

@end

int main(int argc, char *argv[])
{
    OSStatus    junk;
    
    // Create the AuthorizationRef that we'll use through this application.  We ignore 
    // any error from this.  A failure from AuthorizationCreate is very unusual, and if it 
    // happens there's no way to recover; Authorization Services just won't work.

    junk = AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &gAuth);
    assert(junk == noErr);
    assert( (junk == noErr) == (gAuth != NULL) );

	// For each of our commands, check to see if a right specification exists and, if not,
    // create it.
    //
    // The last parameter is the name of a ".strings" file that contains the localised prompts 
    // for any custom rights that we use.
    
	BASSetDefaultRules(
		gAuth, 
		kSampleCommandSet, 
		CFBundleGetIdentifier(CFBundleGetMainBundle()), 
		CFSTR("SampleAuthorizationPrompts")
	);
    
    // And now, the miracle that is Cocoa...
    
    return NSApplicationMain(argc,  (const char **) argv);
}
