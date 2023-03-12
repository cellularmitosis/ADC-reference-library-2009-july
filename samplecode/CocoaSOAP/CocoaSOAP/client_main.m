/*
 File: client_main.m
 
 Abstract: Main program for SOAP client.
 
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


#import <Cocoa/Cocoa.h>
#import "SOAPClient.h"


@interface SOAPAdder : SOAPClient

- (NSNumber *)add:(NSNumber *)num1 to:(NSNumber *)num2;

@end

@implementation SOAPAdder

- (NSNumber *)add:(NSNumber *)num1 to:(NSNumber *)num2 {
    NSError *error = nil;

    // Package the parameters up in a SOAP envelope, matching what the service
    // expects.  Then when the reply comes back, extract the result.

    NSString *xml = [NSString stringWithFormat:@"<?xml version=\"1.0\"?> <soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:ex=\"http://www.apple.com/namespaces/cocoa/soap/example\"> <soap:Body> <ex:MethodName>%@</ex:MethodName> <ex:Parameters> <ex:Parameter>%@</ex:Parameter> <ex:Parameter>%@</ex:Parameter> </ex:Parameters> </soap:Body> </soap:Envelope>", NSStringFromSelector(_cmd), num1, num2];
    NSXMLDocument *doc = [[[NSXMLDocument alloc] initWithXMLString:xml options:NSXMLNodeOptionsNone error:&error] autorelease];
    if (!doc) {
        NSLog(@"error: %@", error);
    }

    NSXMLDocument *replydoc = [self sendMessage:doc waitForReply:YES];

    NSArray *array = [replydoc nodesForXPath:@"soap:Envelope/soap:Body/ex:Result" error:&error];
    id obj = nil;
    if ([array count] > 0) {
        NSXMLNode *node = [array objectAtIndex:0];
        obj = [NSNumber numberWithDouble:[[node objectValue] doubleValue]];
    }
    return obj;
}

@end


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s number number\n", argv[0]);
        exit(0);
    }

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    SOAPAdder *adder = [[SOAPAdder alloc] initWithServerName:@"SOAP adder"];

    if (!adder) {
        NSLog(@"Could not find an adder server");
        exit(1);
    }

    NSNumber *num1 = [NSNumber numberWithDouble:strtod(argv[1], NULL)];
    NSNumber *num2 = [NSNumber numberWithDouble:strtod(argv[2], NULL)];

    NSNumber *result = [adder add:num1 to:num2];
    NSLog(@"result: %@\n", result);

    [adder release];
    [pool release];

    exit(0);
}

