/*
    File:       Test.m

    Contains:   Class to test AESendThreadSafe module.

    Written by: DTS

    Copyright:  Copyright (c) 2007 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Inc.
                ("Apple") in consideration of your agreement to the following
                terms, and your use, installation, modification or
                redistribution of this Apple software constitutes acceptance of
                these terms.  If you do not agree with these terms, please do
                not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following
                terms, and subject to these terms, Apple grants you a personal,
                non-exclusive license, under Apple's copyrights in this
                original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or
                without modifications, in source and/or binary forms; provided
                that if you redistribute the Apple Software in its entirety and
                without modifications, you must retain this notice and the
                following text and disclaimers in all such redistributions of
                the Apple Software. Neither the name, trademarks, service marks
                or logos of Apple Inc. may be used to endorse or promote
                products derived from the Apple Software without specific prior
                written permission from Apple.  Except as expressly stated in
                this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any
                patent rights that may be infringed by your derivative works or
                by other works in which the Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis. 
                APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
                WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
                MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
                THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT,
                INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
                TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
                DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY
                OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
                OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
                OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF
                SUCH DAMAGE.

    Change History (most recent first):

$Log: Test.m,v $
Revision 1.1  2007/02/09 10:56:00         
First checked in.


*/

/////////////////////////////////////////////////////////////////

#include <libkern/OSAtomic.h>

#include <pthread.h>

#import <Cocoa/Cocoa.h>

#include "AESendThreadSafe.h"

/////////////////////////////////////////////////////////////////

@interface Test : NSObject
{
    // Connection to the "Events to send" input field.
    
    IBOutlet NSTextField *  fEventsToSendText;
    
    // Connections to the various counter fields.
    
    IBOutlet NSTextField *  fMessageCountText;
    IBOutlet NSTextField *  fFailureCountText;
    IBOutlet NSTextField *  fThreadCountText;
    
    // Internal state.
    
    int                     fDefaultEventsToSend;
}

- (IBAction)startThread:(id)sender;
- (IBAction)startMain:(id)sender;

- (void)updateUI:(id)userInfo;
- (void)awakeFromNib;
@end

@implementation Test

// These three counters are updated (using atomic operations) by the various 
// Apple event sending threads and then copied to the UI by a timer (see 
// -[Test updateUI:], below).

static int32_t gThreadCount  = 0;   // number of running Apple event send threads

static int32_t gMessageCount = 0;   // number of Apple events send successfully
static int32_t gFailureCount = 0;   // number of Apple event send failures

static void SendEvent(void)
    // Send a single Apple event to the Finder, asking for its version string.
{
    OSStatus            err;
    OSStatus            junk;
    static const OSType kFinder        = 'MACS';
    static const AEDesc kRootContainer = { typeNull, 0 };
    static const OSType kVersion       = pVersion;
    AEDesc              target         = { typeNull, 0 };
    AppleEvent          event          = { typeNull, 0 };
    AEDesc              keyData        = { typeNull, 0 };
    AEDesc              dirObj         = { typeNull, 0 };
    AppleEvent          reply          = { typeNull, 0 };

    err = noErr;

    // Create the Apple event to get the version property from the Finder.
    
    err = AECreateDesc(typeApplSignature, &kFinder, sizeof(kFinder), &target);
    if (err == noErr) {
        err = AECreateAppleEvent(kAECoreSuite, kAEGetData, &target, kAutoGenerateReturnID, kAnyTransactionID, &event);
    }
    if (err == noErr) {
        err = AECreateDesc(typeType, &kVersion, sizeof(kVersion), &keyData);
    }
    if (err == noErr) {
        err = CreateObjSpecifier(typeText, (AEDesc *) &kRootContainer, formPropertyID, &keyData, false, &dirObj);
    }
    if (err == noErr) {
        err = AEPutParamDesc(&event, keyDirectObject, &dirObj);
    }
    
    // Send that event using the code in the AESendThreadSafe module.
    
    if (err == noErr) {
        err = AESendMessageThreadSafeSynchronous(&event, &reply, kAEDefaultTimeout);
    }
    
    // Record our success or failure.
    
    if (err == noErr) {
        OSAtomicIncrement32Barrier(&gMessageCount);
    } else {
        OSAtomicIncrement32Barrier(&gFailureCount);
    }

    // Clean up.
    
    if (reply.descriptorType != typeNull) {
        junk = AEDisposeDesc(&reply);
        assert(junk == noErr);
    }
    if (dirObj.descriptorType != typeNull) {
        junk = AEDisposeDesc(&dirObj);
        assert(junk == noErr);
    }
    if (keyData.descriptorType != typeNull) {
        junk = AEDisposeDesc(&keyData);
        assert(junk == noErr);
    }
    if (event.descriptorType != typeNull) {
        junk = AEDisposeDesc(&event);
        assert(junk == noErr);
    }
    if (target.descriptorType != typeNull) {
        junk = AEDisposeDesc(&target);
        assert(junk == noErr);
    }
}

static void *AESendThreadEntry(void *param)
    // This is a pthread entry point.  When the user clicks the "Start Thread" 
    // button, we spin up a new pthread (see -[Test startThread:]) to run this 
    // routine.  It sends N Apple events (specified by the incoming parameter) 
    // to the Finder.
{
    int             counter;
    int             eventsToSend;
    
    // Extract the number of events to send from our parameter.
    
    eventsToSend = (int) (intptr_t) param;

    OSAtomicIncrement32Barrier(&gThreadCount);

    // Loop, sending eventsToSend Apple events.  We don't bail out on failure 
    // because we record failed attempts in gFailureCount.
    
    counter = 0;
    for (counter = 0; counter < eventsToSend; counter++) {
        SendEvent();
    }

    OSAtomicDecrement32Barrier(&gThreadCount);
    
    return NULL;
}

- (IBAction)startThread:(id)sender
    // Called when the user clicks the "Start Thread" button in the window. 
    // Starts a pthread to run the AESendThreadEntry function.
{
    int         err;
    pthread_t   createdThread;
    int         eventsToSend;
    
    // Get the number of events to send from the text field.  If the value is bogus, 
    // use a sensible default.
    
    eventsToSend = [fEventsToSendText intValue];
    if (eventsToSend <= 0) {
        eventsToSend = fDefaultEventsToSend;
    }
    
    // Start the pthread to send that many events, then detach it so that it runs 
    // independently.
    
    err = pthread_create(&createdThread, NULL, AESendThreadEntry, (void *) (intptr_t) eventsToSend);
    if (err == 0) {
        err = pthread_detach(createdThread);
    }
    assert(err == 0);
}

- (IBAction)startMain:(id)sender
    // Called when the user clicks the "One from Main Thread" button in the window. 
    // It sends a single Apple event from the main thread.  This lets us test the 
    // main thread detection and handling that's in the AESendThreadSafe module.
{
    SendEvent();
    
    // Might as well sync the text fields immediately rather than wait for the timer 
    // to run.
    
    [self updateUI:nil];
}

- (void)updateUI:(id)userInfo
    // A timer function called every second.  This updates the UI based on the 
    // current value of the three global variables that are updated by the various 
    // Apple event sending threads.  Yeah, this is polling, but it's also sample 
    // code (-:
{
    [fMessageCountText setIntValue:gMessageCount];
    [fFailureCountText setIntValue:gFailureCount];
    [fThreadCountText  setIntValue:gThreadCount ];
}

- (void)awakeFromNib
    // When the NIB is instantiated, latch the default value for the number of 
    // events to send and then start our UI update timer.
{
    fDefaultEventsToSend = [fEventsToSendText intValue];
    (void) [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(updateUI:) userInfo:NULL repeats:YES];
}

@end
