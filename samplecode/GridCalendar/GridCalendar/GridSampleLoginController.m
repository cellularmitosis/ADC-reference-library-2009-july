/*

File: GridSampleLoginController.m

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

Copyright 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "GridSampleLoginController.h"
#import "GridSampleMainWindowController.h"
#import "GridSampleApplicationDelegate.h"
#import "GridSampleServiceBrowser.h"
#import "GridSampleConnectionController.h"

static const int GridSampleLoginControllerOKReturnCode = 0;
static const int GridSampleLoginControllerCancelReturnCode = 1;
static const int GridSampleLoginControllerAuthenticationNeededReturnCode = 2;

static const int GridSampleLoginControllerAuthenticationMethodSSOTag = 0;
static const int GridSampleLoginControllerAuthenticationMethodPasswordTag = 1;

@interface GridSampleLoginController (Private)

- (void)connectionDidCloseAlertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo;

@end

@implementation GridSampleLoginController

- (id)initWithMainWindowController:(GridSampleMainWindowController *)mainWindowController;
{
	self = [super init];
	
	if (self != nil) {
	
		_mainWindowController = mainWindowController;
	}
	
	return self;
}

- (void)dealloc;
{
	[_netServicesArrayController release];
	[_connectSheet release];
	[_authenticationNeededSheet release];
	[_connectFieldValue release];
	[_connectStatusFieldValue release];
	[_passwordFieldValue release];
	
	[super dealloc];
}

- (void)loadNib;
{
	[NSBundle loadNibNamed:@"Login" owner:self];
}

- (GridSampleMainWindowController *)mainWindowController;
{
	return _mainWindowController;
}

- (void)showConnectSheet;
{
	if (_connectSheet == nil) [self loadNib];
	
	NSArray *netServiceNames = [[[NSApp delegate] serviceBrowser] valueForKeyPath:@"netServices.name"];
	
	if ([netServiceNames count] > 0) {
		
		[self setValue:[netServiceNames objectAtIndex:0] forKey:@"connectFieldValue"];
	}
	
	[self setValue:@"" forKey:@"connectStatusFieldValue"];

	[NSApp beginSheet:_connectSheet modalForWindow:[[self mainWindowController] window] modalDelegate:self didEndSelector:@selector(connectSheetDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

- (void)showAuthenticationNeededSheet;
{
	if (_authenticationNeededSheet == nil) [self loadNib];
	
	[self setValue:@"" forKey:@"passwordFieldValue"];

	[NSApp beginSheet:_authenticationNeededSheet modalForWindow:[[self mainWindowController] window] modalDelegate:self didEndSelector:@selector(authenticationNeededSheetDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

- (void)showConnectionDidCloseSheet;
{
    NSString *informativeText = @"The connection to the service \"%1$@\" has been lost: %2$@";
    NSString *connectionName = [[[[self mainWindowController] connectionController] connection] name];
    NSError *connectionError = [[[[self mainWindowController] connectionController] connection] error];
	
	NSString *recoverySuggestion = [NSString localizedStringWithFormat:informativeText, connectionName, connectionError];
    
	NSDictionary *userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
		@"Connection Failure", NSLocalizedDescriptionKey,
		recoverySuggestion, NSLocalizedRecoverySuggestionErrorKey,
		nil];
	
	NSError *error = [NSError errorWithDomain:@"" code:0 userInfo:userInfo];
	
	NSAlert *alert = [NSAlert alertWithError:error];
	
	if ([[self mainWindowController] window] != nil) {
		
		[alert beginSheetModalForWindow:[[self mainWindowController] window] modalDelegate:self didEndSelector:@selector(connectionDidCloseAlertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
	}
	else {
		
		[self connectionDidCloseAlertDidEnd:alert returnCode:[alert runModal] contextInfo:NULL];
	}
}

- (void)connectionDidOpen;
{
	BOOL wasAuthenticating = _isAuthenticating;
	
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isAuthenticating"];
	[self setValue:@"" forKey:@"connectStatusFieldValue"];

	if (wasAuthenticating == NO) {
	
		[NSApp endSheet:_connectSheet returnCode:GridSampleLoginControllerOKReturnCode];
	}
	else {
	
		[NSApp endSheet:_authenticationNeededSheet returnCode:GridSampleLoginControllerOKReturnCode];
	}
}

- (void)connectionDidNotOpen;
{
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isAuthenticating"];
	[self setValue:@"Connection failed" forKey:@"connectStatusFieldValue"];
}

- (void)connectionWasCanceled;
{
	BOOL wasAuthenticating = _isAuthenticating;

	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isAuthenticating"];
	
	if (wasAuthenticating == NO) {
	
		[self setValue:@"Connection canceled" forKey:@"connectStatusFieldValue"];
	}
	else {
	
		[self setValue:@"Authentication canceled" forKey:@"connectStatusFieldValue"];
	}
}

- (void)connectionAuthenticationNeeded;
{
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isAuthenticating"];
	[self setValue:@"" forKey:@"connectStatusFieldValue"];

	[NSApp endSheet:_connectSheet returnCode:GridSampleLoginControllerAuthenticationNeededReturnCode];
	
	[self showAuthenticationNeededSheet];
}

- (void)connectionAuthenticationFailed;
{
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isConnecting"];
	[self setValue:[NSNumber numberWithBool:NO] forKey:@"isAuthenticating"];
	[self setValue:@"Authentication failed" forKey:@"connectStatusFieldValue"];
}

- (IBAction)connectSheetCancel:(id)sender;
{
	if (_isConnecting == YES) {
	
		[[[[self mainWindowController] connectionController] connection] close];
		
		[[self mainWindowController] connectionWasCanceled];
	}
	else {
	
		[NSApp endSheet:_connectSheet returnCode:GridSampleLoginControllerCancelReturnCode];
	}
}

- (IBAction)connectSheetOK:(id)sender;
{
	if ([_connectSheet makeFirstResponder:nil] == NO) {
		
		NSBeep();
		return;
	}
	
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"isConnecting"];
	[self setValue:@"Connecting..." forKey:@"connectStatusFieldValue"];
		
	NSNetService *netService = [[[NSApp delegate] serviceBrowser] netServiceWithName:_connectFieldValue];
	
	GridSampleConnectionController *connectionController = [[self mainWindowController] connectionController];
	
	if (netService != nil) {
		
		[connectionController setConnectionWithNetServiceDomain:[netService domain] type:[netService type] name:[netService name] authenticator:nil];
	}
	else {
		
		[connectionController setConnectionWithHostname:_connectFieldValue portnumber:0 authenticator:nil];
	}
	
	[[connectionController connection] open];
}

- (IBAction)authenticationNeededSheetCancel:(id)sender;
{
	if (_isAuthenticating == YES) {
	
		[[[[self mainWindowController] connectionController] connection] close];
		
		[[self mainWindowController] connectionWasCanceled];
	}
	else {
	
		[NSApp endSheet:_authenticationNeededSheet returnCode:GridSampleLoginControllerCancelReturnCode];
	}
}

- (IBAction)authenticationNeededSheetOK:(id)sender;
{
	if ([_authenticationNeededSheet makeFirstResponder:nil] == NO) {
		
		NSBeep();
		return;
	}
	
	[self setValue:[NSNumber numberWithBool:YES] forKey:@"isAuthenticating"];
	[self setValue:@"Authenticating..." forKey:@"connectStatusFieldValue"];

	GridSampleConnectionController *connectionController = [[self mainWindowController] connectionController];

	if (_authenticationMatrixTag == GridSampleLoginControllerAuthenticationMethodSSOTag) {
		
		XGGSSAuthenticator *authenticator = [[[XGGSSAuthenticator alloc] init] autorelease];
		
		[authenticator setServicePrincipal:[connectionController servicePrincipal]];

		[[connectionController connection] setAuthenticator:authenticator];
		
		[[connectionController connection] open];
	}
	else if (_authenticationMatrixTag == GridSampleLoginControllerAuthenticationMethodPasswordTag) {
		
		XGTwoWayRandomAuthenticator *authenticator = [[[XGTwoWayRandomAuthenticator alloc] init] autorelease];
		
		[authenticator setUsername:@"one-xgrid-client"];
		[authenticator setPassword:_passwordFieldValue];
		
		[[connectionController connection] setAuthenticator:authenticator];
		
		[[connectionController connection] open];
	}
}

- (void)connectSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
	[sheet orderOut:self];
	
	if (returnCode == GridSampleLoginControllerOKReturnCode) {
		
		// do nothing
	}
	else if (returnCode == GridSampleLoginControllerAuthenticationNeededReturnCode) {

		// do nothing
	}
	else {
	
		[NSApp terminate:self];
	}
}

- (void)authenticationNeededSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
	[sheet orderOut:self];

	if (returnCode == GridSampleLoginControllerOKReturnCode) {

		// do nothing
	}
	else {
	
		[self showConnectSheet];
	}
}

- (void)connectionDidCloseAlertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo;
{
	[self performSelector:@selector(showConnectSheet) withObject:nil afterDelay:0.0];
}

@end
