/*
    File:       AuthForAll.c

    Contains:   Common application logic.

    Written by: DTS

    Copyright:  Copyright (c) 2005 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

$Log: AuthForAll.c,v $
Revision 1.1  2005/08/01 13:43:22         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// System interfaces

#include <assert.h>
#include <Carbon/Carbon.h>
#include <Security/Security.h>

// Implementation prototypes

#include "AuthForAllImpl.h"

/////////////////////////////////////////////////////////////////

static WindowRef gWindow;

static const ControlID kStatusTextID = { 'RSta', 0 };

static ControlRef gStatusText;

static void DisplayError(OSStatus errNum)
    // Displays an error dialog for the error in errNum.
{
    OSStatus    junk;
    SInt16      junkItemHit;
    Str255      expStr;

    if ( (errNum != noErr) && (errNum != userCanceledErr) && (errNum != errAuthorizationCanceled) ) {

        NumToString(errNum, expStr);

        junk = StandardAlert(
            kAlertStopAlert, 
            "\pAn error occured.", 
            expStr, 
            NULL, 
            &junkItemHit
        );
        assert(junk == noErr);
    }
}

static OSStatus SetStatusText(CFStringRef statusStr)
    // Sets the status text static text to statusStr.
{
    OSStatus    err;

    err = SetControlData(
        gStatusText, 
        kControlEntireControl, 
        kControlStaticTextCFStringTag, 
        sizeof(statusStr), 
        &statusStr
    );
    if (err == noErr) {
        Draw1Control(gStatusText);
    }

    return err;
}

static void DoActionAlpha(void)
    // This routine shows the outline for a self-limiting action 
    // within an application.  The basic idea is to acquire a right 
    // immediately before doing the action, and to not do the action 
    // if you can't acquire the right.
    //
    // Normally this would always result in authentication dialog, but 
    // in this application we've initially defined the "alpha" right to 
    // be "always allow" (see SetupAuthorization).
{
    OSStatus                         err;

    // First set the status string to "" so that it remains that way 
    // if we get an unexpected error.

    err = SetStatusText(CFSTR(""));

    // Request the application-specific right.

    if (err == noErr) {
        err = AcquireRight(kAlphaRightName);
    }

    // Update the status string to display whether we got the right.

    if (err == noErr) {
        // If you were writing a real application, you would go ahead 
        // and actually perform the action at this point.
        err = SetStatusText(CFSTR("Action allowed"));
    } else if (err == errAuthorizationDenied) {
        err = SetStatusText(CFSTR("Action not allowed"));
    }

    // Display an unexpected errors.

    DisplayError(err);
}

static void DoActionBeta(void)
    // An exact analogue to DoActionAlpha, except that the "beta" 
    // right is initialised to require an admin password.
{
    OSStatus                         err;

    // First set the status string to "" so that it remains that way 
    // if we get an unexpected error.

    err = SetStatusText(CFSTR(""));

    // Request the application-specific right.

    if (err == noErr) {
        err = AcquireRight(kBetaRightName);
    }

    // Update the status string to display whether we got the right.

    if (err == noErr) {
        // If you were writing a real application, you would go ahead 
        // and actually perform the action at this point.
        err = SetStatusText(CFSTR("Action allowed"));
    } else if (err == errAuthorizationDenied) {
        err = SetStatusText(CFSTR("Action not allowed"));
    }

    // Display an unexpected errors.

    DisplayError(err);
}

static const EventTypeSpec kMyEventTypes[] = {
    {kEventClassCommand, kEventCommandProcess}
};

static EventHandlerUPP gMyEventHandlerUPP;         // -> MyEventHandler

static OSStatus MyEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
    // Handles the kEventClassCommand/kEventCommandProcess Carbon event.
{
    OSStatus    err;
    HICommand   command;

    err = eventNotHandledErr;
    switch ( GetEventClass(inEvent) ) {
        case kEventClassCommand:
            switch ( GetEventKind(inEvent) ) {
                case kEventCommandProcess:
                    err = GetEventParameter(
                        inEvent,
                        kEventParamDirectObject, 
                        typeHICommand,
                        NULL,
                        sizeof(command),
                        NULL,
                        &command
                    );
                    if (err == noErr) {
                        switch (command.commandID) {
                            case 'Alfa':
                                DoActionAlpha();
                                break;
                            case 'Beta':
                                DoActionBeta();
                                break;
                            default:
                                err = eventNotHandledErr;
                                break;
                        }
                    }
                    break;
                default:
                    // do noting
                    break;
            }
            break;
        default:
            // do noting
            break;
    }
    
    return err;
}

int main(int argc, char* argv[])
{
    IBNibRef        nibRef;
    
    OSStatus        err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gWindow);
    require_noerr( err, CantCreateWindow );

    // Grab a reference to the rule edit text field.

    err = GetControlByID(gWindow, &kStatusTextID, &gStatusText);
    require_noerr( err, CantGetRuleTextControl );

    // Install an application event handler.

    gMyEventHandlerUPP = NewEventHandlerUPP(MyEventHandler);
    assert(gMyEventHandlerUPP != NULL);

    err = InstallWindowEventHandler(gWindow, gMyEventHandlerUPP, GetEventTypeCount(kMyEventTypes), kMyEventTypes, NULL, NULL);
    require_noerr( err, CantInstallHandler );

    err = SetupAuthorization();
    require_noerr( err, CantCreateAuthorization );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( gWindow );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateAuthorization:
CantInstallHandler:
CantGetRuleTextControl:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
    DisplayError(err);

    return EXIT_SUCCESS;
}
