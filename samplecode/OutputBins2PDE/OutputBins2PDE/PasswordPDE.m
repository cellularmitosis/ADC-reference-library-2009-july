/*

File: PasswordPDE.m

Abstract: Implementation of a slightly more advanced PDE that implements 
	new functionality in Leopard while also being compatible with Tiger.

Version: 1.0

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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#import <Print/PMPrintingDialogExtensions.h>
#import "PasswordPDE.h"
#import "PDEChoice.h"
#import "PDELogging.h"

// Keys to look up localized data for the UI
static NSString* kPasswordPanelNameKey = @"AuthPanel";

// PDE Kind definition
static NSString* kPasswordKindID = @"com.apple.samplecode.OutputBins2.PasswordKind";

static NSString* kAuthUsernameKey = @"com.apple.samplecode.OutputBins2.Username";
static NSString* kAuthPasswordKey = @"com.apple.samplecode.OutputBins2.Password";

@implementation PasswordPDE

- (id)initWithCallback:(PDEPluginCallback*)callback andBundle:(NSBundle*)bundle;
{
	PDEProlog(@"initWithCallback:(%@, %p) andBundle:(%@ %p)", callback, callback, bundle, bundle);
	self = [super init];
		
	// init instance variables
	thePDEName = nil;

	pdePluginCallbackObject = [ callback retain ];
	pluginBundle = [ bundle retain ];
	// ensures that our username & password starts fresh when this print panel is loaded.
	[self restoreValuesAndReturnError:nil];
		
	PDEEpilog(@"initWithCallback:=id(self)");
	return self;
}

/*	The dealloc method is called on object deallocation when garbage collection is not in effect.
	Therefore this is the place to release objects that we've allocated or retained.
*/
- (void)dealloc
{
	[pdePluginCallbackObject release];
	[thePDEName release];
	[pdeView release];

	[super dealloc];
}


/*	The finalize method is only needed when you have resources or data that
	needs to be released when garbage collection is in effect. Since the NSObjects
	represented by our instance variables are garbage collected, there is no
	need to release them in the finalize method. Since this code example has no
	other resources that need to be deallocated when garbage collection is in effect,
	there is no need for a finalize method in this code.
	
	If your code needs to explicitly deallocate resources that are not garbage
	collected even when garbage collection is in place, remove the ifdef here and add
	your needed deallocations prior to calling the superclass'es finalize. In general
	however, it is NOT correct to simply migrate such code from your -dealloc method
	directly to -finalize, as the timing of -finalize is non-deterministic
	
	All of our objects are NSObjects, and thus will be garbage collected automatically
	for us, therefore we don't need to do anything here.
*/
#if 0
- (void)finalize
{
    // Release your resources that are not garbage collected.
    // ...
    
    // Then finalize the superclass.
    [super finalize];
}
#endif

// return the NSView for this PDE
- (NSView*)panelView
{
	PDEProlog(@"panelView:");
	if (pdeView == NULL) {
		[NSBundle loadNibNamed:@"PasswordPDE" owner:self];
	}
	PDEEpilog(@"panelView:=NSView(%@, %p)", pdeView, pdeView);
	return pdeView;
}

// Return the localized name for this PDE. This will be used as its title in the
// menu of printing panes.
- (NSString*)panelName
{
	PDEProlog(@"panelName:");
	if (thePDEName == NULL) {
		thePDEName = [[pluginBundle localizedStringForKey:kPasswordPanelNameKey value:nil table:nil] retain];
	}
	PDEEpilog(@"panelName:=NSString(%@)", thePDEName);
	return thePDEName;
}

// return the kind ID for this PDE
- (NSString*)panelKind
{
	PDEProlog(@"panelKind:");
	PDEEpilog(@"panelKind:=NSString(%@)", kPasswordKindID);
	return (NSString*)kPasswordKindID;
}

// return an NSArray of PPD option keywords supported by this PDE
// This PDE doesn't support any options, so we return nil.
- (NSArray *)supportedPPDOptionKeys
{
	PDEProlog(@"supportedPPDOptionKeys:");
	PDEEpilog(@"supportedPPDOptionKeys:=NSArray(nil)");
	return nil;
}

// the PDE panel is about to be shown
-(void)willShow
{
	PDEProlog(@"willShow:");
	PDEEpilog(@"willShow:=void");
}

// Checks the print settings to verify that the print destination is one that
// we are interested in restricting.
-(BOOL)authenticateDestination
{
	PMDestinationType destination;
	BOOL authenticate = NO; // Assume that we don't need to verify if we can't determine the print destination.
	PMPrintSession session = [pdePluginCallbackObject printSession];
	// The typecast avoids a conflict between -[NSPrintInfo printSettings] and
	// -[NSObject(PDEPlugInCallbackProtocol) printSettings].
	PMPrintSettings settings = (PMPrintSettings)[pdePluginCallbackObject printSettings];
	OSStatus err = PMSessionGetDestinationType(session, settings, &destination);
	if(err == noErr)
	{
		// Require authentication if we are going to printer or preview (as preview can print directly)
		// This also covers the case where the user cancels the print operation.
		authenticate = (destination == kPMDestinationPrinter) || (destination == kPMDestinationPreview);
	}
	return authenticate;
}

-(BOOL)validateAndAlert:(BOOL)mustPass
{
	if([self authenticateDestination])
	{
		NSString * username = [usernameView stringValue];
		NSString * password = [passwordView stringValue];
		bool valid = ([username length] != 0) && ([password length] != 0);
		NSInteger ret = NSOKButton;
		if(!valid)
		{
			NSString * title = ([username length] != 0) ? @"Missing Password" : ([password length] == 0) ? @"Missing Username and Password" : @"Missing Username";
			ret = NSRunAlertPanel(title, @"In order to print to this printer, you must enter both a username and a password", @"OK", mustPass ? nil : @"Later", nil);
		}
		return valid || (ret ==  NSCancelButton);
	}
	return YES;
}

// the PDE panel is about to be hidden
-(BOOL)shouldHide
{
	PDEProlog(@"shouldHide:");
	BOOL hide = [self validateAndAlert:NO];
	PDEEpilog(@"shouldHide:=BOOL(%s)", hide ? "YES" : "NO");
	return hide;
}

// This method will be called to ask you if printing should occur
// This is your opportunity to prevent printing from happening
// Returning YES will cause printing to continue, returning NO will stop it.
// Remember, this is only called on Mac OS X 10.5 or later, so you will not
// be able to stop the print job using this method on Mac OS X 10.4.
-(BOOL)shouldPrint
{
	PDEProlog(@"shouldPrint:");
	BOOL print = [self validateAndAlert:YES];
	PDEEpilog(@"shouldPrint:=BOOL(%s)", print ? "YES" : "NO");
	return print;
}

// This method enables you to catch the help button and determine
// the default print panel help should be shown or not.
// Returning YES will display the default help, returning NO will supress it
// allowing you to show your own help.
-(BOOL)shouldShowHelp
{
	OSStatus err;
	// the log macro is a no-op on release builds
	// so we mark 'err' unused so we avoid the warning
	#pragma unused(err)

	PDEProlog(@"shouldShowHelp:");
	
	PDELog(@"Opening Help Book");
	CFStringRef helpbook = CFSTR("OutputBinsPDE2Help");
	err = AHGotoPage(helpbook, NULL, NULL);
	PDELog(@"Result %i attempting to open %@", err, helpbook);
	
	PDEEpilog(@"shouldShowHelp:=BOOL(%s)", "NO");
	return NO;
}

// read the new settings values and update our internal PDE's settings accordingly
- (BOOL)restoreValuesAndReturnError:(NSError **)error
{
	PDEProlog(@"restoreValuesAndReturnError:(%p)", error);
	
	// Remove any values that currently exist for username & password to ensure that
	// a later user can't print with the previous users's credentials.
	// The typecast avoids a conflict between -[NSPrintInfo printSettings] and
	// -[NSObject(PDEPlugInCallbackProtocol) printSettings].
	PMPrintSettings settings = (PMPrintSettings)[pdePluginCallbackObject printSettings];
	PMPrintSettingsSetValue(settings, (CFStringRef)kAuthUsernameKey, NULL, false);
	PMPrintSettingsSetValue(settings, (CFStringRef)kAuthPasswordKey, NULL, false);
	
	PDEEpilog(@"restoreValuesAndReturnError:=BOOL(YES)");
	return YES;
}

// set UI/stored values to the print settings
- (BOOL)saveValuesAndReturnError:(NSError **)error
{
	PDEProlog(@"saveValuesAndReturnError:(%p)", error);
	// Save the username & password to the print settings. Presumably you would have a CUPS
	// prefilter, port monitor or backend that inspected or transmitted these values
	// before allowing printing. Also, it is unlikely that you would want to send
	// the password, or possibly the username, in the clear as is done here.
	// The typecast avoids a conflict between -[NSPrintInfo printSettings] and
	// -[NSObject(PDEPlugInCallbackProtocol) printSettings].
	PMPrintSettings settings = (PMPrintSettings)[pdePluginCallbackObject printSettings];
	PMPrintSettingsSetValue(settings, (CFStringRef)kAuthUsernameKey, [usernameView stringValue], false);
	PMPrintSettingsSetValue(settings, (CFStringRef)kAuthPasswordKey, [passwordView stringValue], false);
	PDEEpilog(@"saveValuesAndReturnError:=BOOL(YES)");
	return YES;
}

#if 0
#warning This is debug code to show you the username and password.
// On 10.4 it appears that stringValue == nil if the view has no string
// This one liner just makes sure the returned string is non-nil
// so that we don't get an exception adding it to the dictionary.
NSString* EmptyIfNil(NSString *str) { return str == nil ? @"" : str; }

static NSString* kAuthUsernameSummaryKey = @"UsernameSummary";
static NSString* kAuthPasswordSummaryKey = @"PasswordSummary";
-(NSDictionary *)summaryInfo
{
	PDEProlog(@"summaryInfo:");
	NSMutableDictionary *summaryInfoDict = [NSMutableDictionary dictionaryWithCapacity:2];
	// It is unlikely that you would want to display anything in the summary for an authorization print panel
	// but this is useful debugging information.
	[summaryInfoDict setObject:EmptyIfNil([usernameView stringValue]) forKey:[pluginBundle localizedStringForKey:kAuthUsernameSummaryKey value:nil table:nil]];
	[summaryInfoDict setObject:EmptyIfNil([passwordView stringValue]) forKey:[pluginBundle localizedStringForKey:kAuthPasswordSummaryKey value:nil table:nil]];
	PDEEpilog(@"summaryInfo:=NSDictionary(%@, %p)", summaryInfoDict, summaryInfoDict);
	return summaryInfoDict;
}
#endif

@end
