/*

File: GridSampleServiceBrowser.m

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

#import "GridSampleServiceBrowser.h"

NSString *GridSampleServiceType = @"_xgrid._tcp";

@implementation GridSampleServiceBrowser

- (id)initWithDomain:(NSString *)domain;
{
    self = [super init];
    
    if (self != nil) {
        
        _domain = [domain copy];

        _netServiceBrowser = [[NSNetServiceBrowser alloc] init];
        
        if (_netServiceBrowser == nil) {
            
            [self autorelease];
            return nil;
        }
        
        [_netServiceBrowser setDelegate:self];
        
        _netServices = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)init;
{
    return [self initWithDomain:@""];
}

- (void)dealloc;
{
    [_domain release];
    [_netServiceBrowser release];
    [_netServices release];

    [super dealloc];
}

- (NSString *)domain;
{
    return _domain;
}

- (unsigned int)countOfNetServices;
{
    return [_netServices count];
}

- (id)objectInNetServicesAtIndex:(unsigned int)index;
{
    return [_netServices objectAtIndex:index];
}

- (void)insertObject:(id)object inNetServicesAtIndex:(unsigned int)index;
{
    [_netServices insertObject:object atIndex:index];
}

- (void)removeObjectFromNetServicesAtIndex:(unsigned int)index;
{
    [_netServices removeObjectAtIndex:index];
}

- (void)replaceObjectInNetServicesAtIndex:(unsigned int)index withObject:(id)object;
{
    [_netServices replaceObjectAtIndex:index withObject:object];
}

- (NSNetService *)netServiceWithName:(NSString *)name;
{
	NSEnumerator *enumerator = [_netServices objectEnumerator];
	
	id netService;
	
	while (netService = [enumerator nextObject]) {
		
		if ([name isEqualToString:[netService name]] == YES) {
			
			return [[netService retain] autorelease];
		}
	}
	
	return nil;
}

- (void)browse;
{
    if (_didStart == NO) {
        
        [_netServices removeAllObjects];
        
        [_netServiceBrowser searchForServicesOfType:GridSampleServiceType inDomain:_domain];
        
        _didStart = YES;
    }
}

- (void)stop;
{
    if (_didStart == YES) {
        
        [_netServiceBrowser stop];
        
        _didStart = NO;
    }
}

- (void)netServiceBrowserWillSearch:(NSNetServiceBrowser *)netServiceBrowser;
{
    // ignored
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser
             didNotSearch:(NSDictionary *)errorDict;
{
    if (_didStart == YES) {
        
        _didStart = NO;
    }
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser
           didFindService:(NSNetService *)netService
               moreComing:(BOOL)moreComing;
{
    [self insertObject:netService inNetServicesAtIndex:[self countOfNetServices]];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser
         didRemoveService:(NSNetService *)netService
               moreComing:(BOOL)moreComing;
{
    int i = [self countOfNetServices];
    
    while (i--) {
     
        NSNetService *currentNetService = [self objectInNetServicesAtIndex:i];
        
        if ([[currentNetService name] isEqualToString:[netService name]] == YES) {
            
            [self removeObjectFromNetServicesAtIndex:i];
            break;
        }
    }
}

@end
