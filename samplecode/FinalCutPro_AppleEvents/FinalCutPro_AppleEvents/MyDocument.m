/*

File: MyDocument.m

Abstract:   Implementation of NSDocument subclass for Final Cut Pro Apple 
			Event example application (FCP_AE_Tester).  Manages display
			of relevant information in document window, and I/O of XML
			streams sent to or received from an active instance of
			Final Cut Pro.

Version: 1.0

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

Copyright © 2006-7 Apple Computer, Inc., All Rights Reserved

*/ 

#import "MyDocument.h"

@implementation MyDocument

- (NSString *)string
{
	return string;
}

- (void)setString:(NSString *)value
{
	[value retain];
	[string release];
	string = value;
}

- (void)updateString
{
	[self setString:[textView string]];
}

- (void)updateView
{
	[textView setString:[self string]];
}

- (id)init
{
    self = [super init];
    return self;
}

- (void)dealloc
{
	[string release];
	[super dealloc];
}

- (NSString *)windowNibName
{
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	if (!string) {
		[self setString:@""];
	}
	[self updateView];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
	[self updateString];
    return [string dataUsingEncoding:NSUTF8StringEncoding];
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    NSString *aString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
	if (aString == nil) {
		aString = [[NSString alloc] initWithData:data encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF16)];
		if (aString == nil) {
			return NO;
		}
	}
	[self setString:aString];
	[aString release];
	[self updateView];
    return YES;
}

- (void)setWithBytes:(const void *)bytes length:(unsigned)length
{
    NSString *aString = [[NSString alloc] initWithBytes:bytes length:length encoding:NSUTF8StringEncoding];
	if (aString == nil) {
		return;
	}
	[self setString:aString];
	[aString release];
	[self updateView];
}

+ (NSData *)reformat:(NSData *)dataIn
{
	// make task object
    NSTask *aTask = [[NSTask alloc] init];
	
	// make pipes & hook them up
	NSPipe *toPipe = [NSPipe pipe];
	NSPipe *fromPipe = [NSPipe pipe];
	NSFileHandle *writing = [toPipe fileHandleForWriting];
	NSFileHandle *reading = [fromPipe fileHandleForReading];
	[aTask setStandardInput:toPipe];
	[aTask setStandardOutput:fromPipe];
	
	// set environment
	NSDictionary *env = [NSDictionary dictionaryWithObjectsAndKeys:@"	", @"XMLLINT_INDENT", nil];
	[aTask setEnvironment:env];
	
    // set arguments
    NSMutableArray *args = [NSMutableArray array];
    [args addObject:@"--format"];
    [args addObject:@"-"];
    [aTask setArguments:args];
	
	// launch
    [aTask setLaunchPath:@"/usr/bin/xmllint"];
    [aTask launch];
	
	// grab text of doc and write it
	[writing writeData:dataIn];
	
	// send EOF
	[writing closeFile];
	
	// grab returned data and save it
	// NOTE: this is not the right way to do this in general, but may work okay in this context.
	// Basically we are assuming that we don't need to read any output until all the input has
	// been written. Since, as far as I recall, pipes can only contain a relatively small amount
	// of data, this means we are assuming that no substantial output is done by the child process
	// until all the input data has been read. (Otherwise, the child process could block trying to
	// write output to a full pipe that no one is reading.) It is probably the case that 'xmllint'
	// reads all its input first, due to the structure of XML, but this would not be true in general.
	NSData *dataOut = [reading readDataToEndOfFile];
	
	// clean up task object
	[aTask terminate];	// the 'terminate' shouldn't be necessary - I think..
	[aTask release];
	
	return dataOut;
}


@end
