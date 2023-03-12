/*

File:<AppCustomPDE.m>

Abstract: <Sample code for Cocoa print dialog extension>

Version: <1.0>

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
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

Copyright © 2007 Apple Computer, Inc., All Rights Reserved

*/

#import "AppCustomPDE.h"
#import "PDECommon.h"
#import <Print/PMPrintingDialogExtensions.h>

NSBundle* pluginBundle = nil;

Boolean doPrintTitles = kPrintTitlesOnlyDefault;

@implementation AppCustomPDEPlugIn

// Called when the PDE Bunddle is loaded
- (BOOL)initWithBundle:(NSBundle*)theBundle
{
	if (pluginBundle == NULL) {
		pluginBundle = [theBundle retain];
	}
	
	return YES;
}

// Called when the Print Dialog is first shown in order to collect the PDE to be displayed.
- (NSArray*)PDEPanelsForType:(NSString*)pdeType withHostInfo:(id)host
{
	PDEPluginCallback* callback = (PDEPluginCallback*)host;

	NSMutableArray* pdes = [NSMutableArray array];
	// We only want Application PDEs, so ignore this message if that is not what is being asked for.
	if ([pdeType isEqual:(NSString*)kAppPrintDialogTypeIDStr]) {
		AppCustomPDE* appCustomPDE = [[[AppCustomPDE alloc] initWithCallback:callback] autorelease];
		[pdes addObject:appCustomPDE];
	}

	return [pdes count] ? pdes : nil;
}
@end


@interface AppCustomPDE (private_routines)
- (void) updateUI;
@end


@implementation AppCustomPDE

// Called when the PDE has been requested. Save the callback so that you can access print settings.
- (id)initWithCallback:(PDEPluginCallback*)callback
{
	self = [super init];

	pdePluginCallbackObject = [callback retain];
	
	return self;
}

// for any necessary deallocations prior to PDE termination. Called
// when garbage collection is not in effect.
- (void)dealloc
{
	[appCustomPDEView release];
	[pdePluginCallbackObject release];
	[pluginBundle release];

	[super dealloc];
}

/*
    The finalize method is only needed when you have resources or data that
    needs to be released when garbage collection is in effect. Since the NSObjects
	represented by our instance variables are garbage collected, there is no
	need to release them in the finalize method. Since this code example has no
	other resources that need to be deallocated when garbage collection is in effect,
	there is no need for a finalize method in this code.
	
	If your code needs to explicitly deallocate resources that are not garbage
	collected even when garbage collection is in effect, remove the ifdef here and add
	your needed deallocations prior to calling the superclass's finalize.
*/
#if 0
- (void)finalize {
    // Release your resources that are not garbage collected.
    // ...
    
    // Then finalize the superclass.
    [super finalize];
}
#endif

// return the NSView for this PDE
- (NSView*)panelView
{
	if (appCustomPDEView == nil)
		[NSBundle loadNibNamed:@"AppCustomPDE" owner:self];

	if (appCustomPDEView == nil)
		NSLog(@"Couldn't load view from nib file!");

	return appCustomPDEView;
}

// return the name for this PDE
- (NSString*)panelName
{
	if (thePDEName == NULL)
		thePDEName = @"Print Titles";
		
	return thePDEName;
}

// return the kind ID for this PDE
- (NSString*)panelKind
{
	return (NSString*)kMyAppCustomPDEKindID;
}

// return an NSArray of PPD option keywords supported by this PDE
// This PDE supports no PPD option keys
- (NSArray *)supportedPPDOptionKeys;
{
	return nil;
}

// the PDE panel is about to be shown.
- (void)willShow
{
	[self updateUI];
}		

// all panel settings are valid so it is OK for the panel to be hidden
- (BOOL)shouldHide
{
	return YES;
}

// set stored values from ticket(s)
- (BOOL)restoreValuesAndReturnError:(NSError **)error
{
#pragma unused (error)
	OSStatus err = noErr;
	CFBooleanRef settingsCFBool;
	PMPrintSettings printSettings = [ pdePluginCallbackObject printSettings ];
	
	err = PMPrintSettingsGetValue(printSettings, kMyApplicationPrintSettingsKey, (CFTypeRef*)&settingsCFBool);

	// Validate that we read CFBoolean data and if so, use the result.
	if((err == noErr) && (CFGetTypeID(settingsCFBool) == CFBooleanGetTypeID())) {
		doPrintTitles = CFBooleanGetValue(settingsCFBool);
	} else {
		// if not, use the a default value instead.
		doPrintTitles = kPrintTitlesOnlyDefault;
	}

	// only update the UI settings if the panel has been loaded.
	if (appCustomPDEView != NULL)
		[self updateUI];
		
	return YES;
}

// set UI/stored values to ticket(s)
- (BOOL)saveValuesAndReturnError:(NSError **)error
{
#pragma unused (error)
	OSStatus err = noErr;
	PMPrintSettings printSettings = [ pdePluginCallbackObject printSettings ];
		
	err = PMPrintSettingsSetValue(printSettings, 
				kMyApplicationPrintSettingsKey, 
				doPrintTitles ? kCFBooleanTrue : kCFBooleanFalse, 
				false);
	
	// This sample always returns yes, even if there is an error. Otherwise the user can't
	// switch out of this panel.
	return YES;
}

// move stored values to the UI
- (void) updateUI
{
	[ printTitlesButton setState: doPrintTitles ? NSOnState : NSOffState ];
}

- (IBAction)handlePrintTitlesButton:(id)sender
{
	// Use the button state to determine our setting.
	doPrintTitles = [ sender state ] == NSOnState;
}


// format info for Summary panel
- (NSDictionary *) summaryInfo
{
	NSMutableDictionary *summaryInfoDict = [NSMutableDictionary dictionaryWithCapacity:0];
	
	// Of course these values should be localized.
	[summaryInfoDict setObject: ( [ printTitlesButton state ] == NSOnState ? @"Yes" : @"No" ) forKey:@"PrintTitles"];
	
	return summaryInfoDict;
}

@end
