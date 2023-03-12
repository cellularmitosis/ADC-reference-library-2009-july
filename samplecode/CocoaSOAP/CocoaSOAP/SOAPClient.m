/*
 File: SOAPClient.m
 
 Abstract: Implementation file for a basic SOAP client.
 
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
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 */ 

#import "SOAPClient.h"
#include <netinet/in.h>
#include <arpa/inet.h>


@implementation SOAPClient

- (id)initWithServerName:(NSString *)name {
    // Find one of the named services, then extract the port number and address.
    // We need to do this because we need to use an NSURL to make the requests.

    NSNetService *service = [[[NSNetService alloc] initWithDomain:@"local." type:@"_http._tcp." name:name] autorelease];
    if (!service) {
        [self release];
        return nil;
    }
    [service scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:@"PrivateSOAPMode"];
    [service resolveWithTimeout:8.0];
    CFAbsoluteTime deadline = CFAbsoluteTimeGetCurrent() + 8.0;
    CFTimeInterval remaining;
    while ((remaining = (deadline - CFAbsoluteTimeGetCurrent())) > 0 && [[service addresses] count] == 0) {
        CFRunLoopRunInMode((CFStringRef)@"PrivateSOAPMode", remaining, true);
    }
    [service stop];
    NSArray *addresses = [service addresses];
    if (0 == [addresses count]) {
        [self release];
        return nil;
    }

    NSData *address = [addresses objectAtIndex:0];
    struct sockaddr_in *address_sin = (struct sockaddr_in *)[address bytes];
    struct sockaddr_in6 *address_sin6 = (struct sockaddr_in6 *)[address bytes];
    const char *formatted;
    char buffer[1024];
    in_port_t port = 0;
    if (AF_INET == address_sin->sin_family) {
        formatted = inet_ntop(AF_INET, &(address_sin->sin_addr), buffer, sizeof(buffer));
        port = ntohs(address_sin->sin_port);
        url = [[NSURL alloc] initWithString:[NSString stringWithFormat:@"http://%s:%d/", formatted, port]];
    } else if (AF_INET6 == address_sin6->sin6_family) {
        formatted = inet_ntop(AF_INET6, &(address_sin6->sin6_addr), buffer, sizeof(buffer));
        port = ntohs(address_sin6->sin6_port);
        url = [[NSURL alloc] initWithString:[NSString stringWithFormat:@"http://[%s]:%d/", (0 ? formatted : "::1"), port]];
    }
    // Note that using an IPv6 address in the URL seems to not work, unless
    // the address is the trivial loopback address (that is, localhost). Even
    // using the address of the local machine seems not to work -- no data
    // will get to the server, but no exception gets raised.

    if (!url) {
        [self release];
        return nil;
    }
    return self;
}

- (void)dealloc {
    [url release];
    [super dealloc];
}

- (NSXMLDocument *)sendMessage:(NSXMLDocument *)xml waitForReply:(BOOL)b {
    NSData *data = [xml XMLData];

    NSMutableURLRequest *request = [[[NSMutableURLRequest alloc] initWithURL:url cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:60.0] autorelease];
    [request setHTTPMethod:@"POST"];
    [request setHTTPBody:data];
    [request setValue:[NSString stringWithFormat:@"%d", [data length]] forHTTPHeaderField:@"Content-Length"];
    MyURLConnection *conn = [[[MyURLConnection alloc] initWithRequest:request delegate:self] autorelease];

    xml = nil;
    if (b) {
        while (![conn isFinished]) {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        }
        NSError *error = nil;
        xml = [[[NSXMLDocument alloc] initWithData:[conn data] options:NSXMLNodeOptionsNone error:&error] autorelease];
    }

    return xml;
}

- (void)connection:(MyURLConnection *)urlconn didReceiveResponse:(NSURLResponse *)response {
NSLog(@"getting response");
    [urlconn setResponse:response];
}

- (void)connection:(MyURLConnection *)urlconn didReceiveData:(NSData *)data {
    [urlconn appendData:data];
}

- (void)connectionDidFinishLoading:(MyURLConnection *)urlconn {
    [urlconn setFinished:YES];
}

@end

