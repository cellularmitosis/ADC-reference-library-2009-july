/*

File: GridSampleConnectionController.m

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

#import "GridSampleConnectionController.h"
#import "GridSampleMainWindowController.h"

@interface GridSampleConnectionController (Private)

- (void)setConnection:(XGConnection *)connection;
- (void)setController:(XGController *)controller;
- (void)setMainWindowController:(GridSampleMainWindowController *)mainWindowController;

@end

@implementation GridSampleConnectionController

+ (id)connectionController;
{
	return [[[self alloc] init] autorelease];
}

- (id)init;
{
	self = [super init];
	
	if (self != nil) {
	
		[self setMainWindowController:[GridSampleMainWindowController mainWindowController]];
	}
	
	return self;
}

- (void)dealloc;
{
	[self setMainWindowController:nil];
    [self setConnection:nil];
    [self setController:nil];
    
    [super dealloc];
}

- (void)setConnectionWithNetServiceDomain:(NSString *)domain type:(NSString *)type name:(NSString *)name authenticator:(XGAuthenticator *)authenticator;
{
	_type = GridSampleConnectionControllerNetServiceType;

	NSNetService *netService = [[[NSNetService alloc] initWithDomain:domain type:type name:name] autorelease];
	
	XGConnection *connection = [[[XGConnection alloc] initWithNetService:netService] autorelease];
	
	[connection setAuthenticator:authenticator];
	
	XGController *controller = [[[XGController alloc] initWithConnection:connection] autorelease];
	
	[self setConnection:connection];
	[self setController:controller];
}

- (void)setConnectionWithHostname:(NSString *)hostname portnumber:(int)portnumber authenticator:(XGAuthenticator *)authenticator;
{
	_type = GridSampleConnectionControllerHostnameType;
	
	XGConnection *connection = [[[XGConnection alloc] initWithHostname:hostname portnumber:portnumber] autorelease];
	
	[connection setAuthenticator:authenticator];
	
	XGController *controller = [[[XGController alloc] initWithConnection:connection] autorelease];
	
	[self setConnection:connection];
	[self setController:controller];
}

#pragma mark *** Accessors ***

- (GridSampleConnectionControllerType)type;
{
	return _type;
}

- (NSString *)name;
{
	return [[self connection] name];
}

- (NSString *)servicePrincipal;
{
	NSString *servicePrincipal = [[self connection] servicePrincipal];
	
	if (servicePrincipal == nil) servicePrincipal = [NSString stringWithFormat:@"xgrid/%@", [self name]];
	
	return servicePrincipal;
}

- (XGConnection *)connection;
{
    return _connection;
}

- (XGController *)controller;
{
    return _controller;
}

- (GridSampleMainWindowController *)mainWindowController;
{
	return _mainWindowController;
}

#pragma mark *** Convenience methods ***

- (void)begin;
{
	[[[self mainWindowController] window] makeKeyAndOrderFront:self];
	
	[[self mainWindowController] performSelector:@selector(showConnectSheet) withObject:nil afterDelay:1.0];
}

#pragma mark *** Private methods ***

- (void)setConnection:(XGConnection *)connection;
{
    if (_connection != connection) {
        
        if ([_connection delegate] == self) {
            
            [_connection setDelegate:nil];
        }
                
        [_connection release];
        
        _connection = [connection retain];
        
        [_connection setDelegate:self];
    }
}

- (void)setController:(XGController *)controller;
{
    if (_controller != controller) {
        
        [_controller autorelease];
        _controller = [controller retain];
    }
}

- (void)setMainWindowController:(GridSampleMainWindowController *)mainWindowController;
{
	if (_mainWindowController != mainWindowController) {
	
		if ([_mainWindowController connectionController] == self) {
		
			[_mainWindowController setConnectionController:nil];
		}
		
		[_mainWindowController autorelease];
		_mainWindowController = [mainWindowController retain];
		
		[_mainWindowController setConnectionController:self];
	}
}

#pragma mark *** XGConnection delegate methods ***

- (void)connectionDidOpen:(XGConnection *)connection;
{
	_connectionDidOpen = YES;

	[[self mainWindowController] connectionDidOpen];
}

- (void)connectionDidClose:(XGConnection *)connection;
{
	// check for success
    if ([connection error] != nil && [[connection error] code] != 200) {
        
		// check for authentication errors
		if ([[connection error] code] == 530) {
			
			// authentication required; check for authenticator 
			if ([connection authenticator] == nil) {
				
				// no authenticator; request authentication from user
				[[self mainWindowController] connectionAuthenticationNeeded];
			}
			else {
				
				// authenticator; authentication wasn't sufficient
				[[self mainWindowController] connectionAuthenticationFailed];
			}
		}
		else if ([[connection error] code] == 535) {
			
			// authentication failure; show alert to user
			[[self mainWindowController] connectionAuthenticationFailed];
		}
		else {
			
			// unknown error; check if we opened
			if (_connectionDidOpen == NO) {
			
				// we never opened
				[[self mainWindowController] connectionDidNotOpen];
			}
			else {
			
				// we opened; this is a true close
				[[self mainWindowController] connectionDidClose];
			}
		}
    }
	else {
	
		[[self mainWindowController] connectionDidClose];
	}
	
	// the connection is not open
	_connectionDidOpen = NO;
}

@end
