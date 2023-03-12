/*

File: main.m

Abstract: Main entry point, showing a simple usage of PortMapper class

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

Copyright © 2007 Apple Inc. All Rights Reserved.

*/


#import "PortMapper.h"


/** A trivial class that just demonstrates how to create a PortMapper and listen for changes. */
@interface PortMapperTest : NSObject
{
    PortMapper *_mapper;
}

@end


@implementation PortMapperTest


- (id) init
{
    self = [super init];
    if( self ) {
        // Create PortMapper. To experiment, you could turn on Web Sharing, then change 
		// this port number to 80 -- that will make your computer's local Apache web
		// server reachable from the outside world while this program is running.
        _mapper = [[PortMapper alloc] initWithPort: 7];
        
        // Optionally, request a public port number.
        // The NAT is free to ignore this and return a random number instead.
        _mapper.desiredPublicPort = 22222;
        
        // Now open the mapping (asynchronously):
        if( [_mapper open] ) {
            NSLog(@"Opening port mapping...");
            // Now listen for notifications to find out when the mapping opens, fails, or changes:
            [[NSNotificationCenter defaultCenter] addObserver: self 
                                                     selector: @selector(portMappingChanged:) 
                                                         name: PortMapperChangedNotification 
                                                       object: _mapper];
        } else {
            // PortMapper failed -- this is unlikely, but be graceful:
            NSLog(@"!! Error: PortMapper wouldn't start: %i",_mapper.error);
            [self release];
            return nil;
        }
    }
    return self;
}


- (void) portMappingChanged: (NSNotification*)n
{
    // This is where we get notified that the mapping was created, or that no mapping exists,
    // or that mapping failed.
    if( _mapper.error )
        NSLog(@"!! PortMapper error %i", _mapper.error);
    else {
        NSString *message = @"";
        if( !_mapper.isMapped )
            message = @" (no address translation!)";
        NSLog(@"** Public address:port is %@:%hu%@", _mapper.publicAddress,_mapper.publicPort,message);
    }
}


- (void) dealloc
{
    [_mapper close];
    [_mapper release];
    [super dealloc];
}


@end



int main (int argc, const char * argv[]) 
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    // Here's how to simply obtain your local and public address(es):
    NSString *addrStr = [PortMapper localAddress];
    NSLog(@"** Local address is %@", addrStr);
    if( [PortMapper localAddressIsPrivate] )
        NSLog(@"** ...that's a private address");
    addrStr = [PortMapper findPublicAddress];
    NSLog(@"** Public address is %@", addrStr);

    // Start up the test class to create a mapping:
    PortMapperTest *test = [[PortMapperTest alloc] init];
    
    // Now let the runloop run, to give the mapping time to be set up:
    NSLog(@"Running the runloop...");
    [[NSRunLoop currentRunLoop] run];
        
    [test release];
    [pool release];
    return 0;
}
