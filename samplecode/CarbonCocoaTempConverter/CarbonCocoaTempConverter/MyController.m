/*
 
 File: MyController.m
 
 Abstract: Main controller class. 
		   Loads and manages Carbon and Cocoa windows that allow users to convert Fahrenheit values to Celsius.
           Provides a routine to handle events in the Carbon window.
 
 Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
 */

#import "MyController.h"

#pragma mark Carbon Methods
/*
	Handles the "Convert" button's events in the Carbon window.
*/
pascal OSStatus CarbonWindowCommandHandler(EventHandlerCallRef handlerRef, EventRef event, void *userData)
{
#pragma unused (handlerRef)
    OSStatus err = eventNotHandledErr;
    HICommand command; 
    GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    /* Calls the ConvertCommandHandler routine if commandID is equal to kConvertCommand */
    switch(command.commandID) {
        case kConvertCommand:
			ConvertCommandHandler((WindowRef) userData);
            err = noErr;
            break;
    }
    return err;
}


/*
	Called when users click the "Convert" button in the Carbon window.
	Takes a number that represents a fahrenheit value, converts it to Celsius, and displays the result.
*/ 
pascal void ConvertCommandHandler(WindowRef window)
{
    ControlHandle fahrenheitField, celsiusField;
    ControlID fahrenheitControlID = { kConvertSignature, kFahrenheitFieldID };
    ControlID celsiusControlID = { kConvertSignature, kCelsiusFieldID };
    CFStringRef	text;
    Size actualSize;
    double fahrenheitTemp, celsiusTemp;
	
	/* Fetch the fahrenheit and celsius textfields in the carbon window */
    GetControlByID(window, &fahrenheitControlID, &fahrenheitField);
    GetControlByID(window, &celsiusControlID, &celsiusField );
	
    /* Retrieve the content of the fahrenheit textfield */
    GetControlData(fahrenheitField, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &text, &actualSize);
	/* Get the double value of the retrieved content */
	fahrenheitTemp = CFStringGetDoubleValue(text);
    CFRelease(text);
	
	/* Convert the fahrenheit value and assign the result to celsiusTemp */
    celsiusTemp = (fahrenheitTemp - 32.0) * 5.0 / 9.0; 
	
    text = CFStringCreateWithFormat(NULL, NULL, CFSTR("%g"), celsiusTemp);
	/* Set the value of the celsius textfield */
    SetControlData(celsiusField, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &text);
    CFRelease(text);
	DrawOneControl(celsiusField);
}


@implementation MyController
- (id)init
{
	self = [super init];
	return self;
}

#pragma mark Cocoa Methods
/*
	Called when users click the "Convert" button in the Cocoa window. 
	Takes a number that represents a fahrenheit value, converts it to Celsius, and displays the result.
*/ 
- (IBAction)fahrenheitToCelsius:(id)sender
{
    double fahrenheitTemp, celsiusTemp;
    /* Get the fahrenheit value */
    fahrenheitTemp = [fahrenheitField doubleValue];
	/* Convert it to Celsius */
    celsiusTemp = (fahrenheitTemp - 32.0) * 5.0 / 9.0;
	/* Round and display the result */
    [celsiusField setDoubleValue:celsiusTemp];
}

/*  
	This routine runs whenever the Cocoa object is unpacked from the nib file; it's a good place to do initialization.
*/
-(void)awakeFromNib
{
    CFBundleRef bundleRef;
    IBNibRef nibRef;
    OSStatus err;
    
    /* The Cocoa nib was loaded for us by default at app launch time.
	 But now, we need to explicitly load the Carbon nib and its window. */
    bundleRef = CFBundleGetMainBundle();
    err = CreateNibReferenceWithCFBundle(bundleRef,CFSTR("carbonnib"), &nibRef);
    if (err!=noErr){
		NSLog(@"failed to create carbon nib reference");
	}
    
    /* Ok, the Carbon nib is loaded, so now we need to load the window from it */
	err = CreateWindowFromNib(nibRef, CFSTR("CarbonWin"), &window);
	if (err != noErr){
		NSLog(@"failed to create carbon window from nib");
	}
	
    DisposeNibReference(nibRef);
    
    /* Create a Cocoa window to wrap the Carbon window and make sure events are handled properly,
	 since this is a Cocoa app with a main runloop controlled by Cocoa */
    cocoaFromCarbonWin = [[NSWindow alloc] initWithWindowRef:window];
    
    /* Bring the "Carbon" window to the front */
    [cocoaFromCarbonWin makeKeyAndOrderFront:nil];
    
    /* Carbon Event handler that listens for command events in the Carbon window */
    InstallWindowEventHandler(window,NewEventHandlerUPP(CarbonWindowCommandHandler),1, &commSpec, (void *) window, NULL);
}


- (void)dealloc
{
	[cocoaFromCarbonWin release];
	[super dealloc];
}
@end
