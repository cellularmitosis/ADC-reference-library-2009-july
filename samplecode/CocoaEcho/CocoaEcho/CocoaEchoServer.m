/*
 File: CocoaEchoServer.m
 
 Abstract: A Foundation tool that offers a simple echo service.
 
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

#import <Foundation/Foundation.h>
#import "TCPServer.h"

@interface EchoServer : TCPServer {
@private
    CFMutableDictionaryRef connections;
}

- (void)handleNewConnectionFromAddress:(NSData *)addr inputStream:(NSInputStream *)istr outputStream:(NSOutputStream *)ostr;

@end

@implementation EchoServer

- (void)setupInputStream:(NSInputStream *)istream outputStream:(NSOutputStream *)ostream {
    [istream retain];
    [ostream retain];
    [istream setDelegate:self];
    [ostream setDelegate:self];
    [istream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [ostream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    CFDictionarySetValue(connections, istream, ostream);
    [istream open];
    [ostream open];
    NSLog(@"Added connection.");
}

- (void)shutdownInputStream:(NSInputStream *)istream outputStream:(NSOutputStream *)ostream {
    [istream close];
    [ostream close];
    [istream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [ostream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [istream setDelegate:nil];
    [ostream setDelegate:nil];
    CFDictionaryRemoveValue(connections, istream);
    [istream release];
    [ostream release];
    NSLog(@"Connection closed.");
}

- (void)handleNewConnectionFromAddress:(NSData *)addr inputStream:(NSInputStream *)istr outputStream:(NSOutputStream *)ostr {
    if (!connections) {
        connections = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    }
    [self setupInputStream:istr outputStream:ostr];
}

- (void)stream:(NSStream *)aStream handleEvent:(NSStreamEvent)streamEvent {
    NSInputStream * istream;
    NSOutputStream * ostream;
    switch(streamEvent) {
        case NSStreamEventHasBytesAvailable:;
            istream = (NSInputStream *)aStream;
            ostream = (NSOutputStream *)CFDictionaryGetValue(connections, istream);
            
            uint8_t buffer[2048];
            int actuallyRead = [istream read:(uint8_t *)buffer maxLength:2048];
            if (actuallyRead > 0) {
                [ostream write:buffer maxLength:actuallyRead];
            }
                break;
        case NSStreamEventEndEncountered:;
            istream = (NSInputStream *)aStream;
            ostream = nil;
            if (CFDictionaryGetValueIfPresent(connections, istream, (const void **)&ostream)) {
                [self shutdownInputStream:istream outputStream:ostream];
            }
                break;
        case NSStreamEventHasSpaceAvailable:
        case NSStreamEventErrorOccurred:
        case NSStreamEventOpenCompleted:
        case NSStreamEventNone:
        default:
            break;
    }
}

@end

#pragma mark -
#pragma mark Initialzation & Server startup

int main(const int argc, const char **argv) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSRunLoop * rl = [NSRunLoop currentRunLoop];
    EchoServer * echo = [[EchoServer alloc] init];
    NSError * startError = nil;
    [echo setType:@"_cocoaecho._tcp."];
    if (![echo start:&startError] ) {
        NSLog(@"Error starting server: %@", startError);
    } else {
        NSLog(@"Starting server on port %d", [echo port]);
    }
    [rl run];
    [pool release];
    return 0;
}