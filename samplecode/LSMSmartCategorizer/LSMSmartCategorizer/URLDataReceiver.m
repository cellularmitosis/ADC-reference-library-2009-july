/*
File: URLDataReceiver.m

Abstract: A Class encapsulating the asynchronous loading functionality of NSURLConnection.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, 
Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple,
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

Copyright ¬© 2007 Apple Inc., All Rights Reserved

*/

#import "URLDataReceiver.h"

/*!
 * @abstract Status of URLDataReceiver.
 */
enum {
	DRInit = 1,
	DRLoading,
	DRFinished,
	DRCancelled
};

/*!
 * @abstract Private routines.
 */
@interface URLDataReceiver(Private)
- (void)notifyDidFinish;
- (void)setError:(NSError*)error;
@end

@implementation URLDataReceiver

- (id)initWithURL:(NSURL*)aURL delegate:(id)delegate {
	self = [super init];
	
	if (self == nil) return nil;
	
	fStatus = DRLoading;
	fURL = [aURL copy];
	fReceived = [NSMutableData new];
	fError = nil;
	fDelegate = delegate;
	fLock = [NSRecursiveLock new];
	fConnection = nil;
	
	return self;
}

- (void) dealloc {
	//cancel it if we are loading.
	[fLock lock];
	if (fStatus == DRLoading) [self cancel];
	[fLock unlock];
	
	if (fURL) [fURL release];
	if (fReceived) [fReceived release];
	if (fError) [fError release];
	if (fLock) [fLock release];
	if (fConnection) [fConnection release];
	
	[super dealloc];
}
	
	

- (void)startLoading {
	[fLock lock];
	
	fStatus = DRLoading;
	fConnection = [[NSURLConnection alloc] 
					initWithRequest:[NSURLRequest requestWithURL:fURL]
					delegate:self];
	[fLock unlock];
}

- (void)cancel {
	[fLock lock];
	
	//return right away if we are not loading.
	if (fStatus != DRLoading) {
		[fLock unlock];
		return;
	}
	
	fStatus = DRCancelled;
	
	if (fConnection) [fConnection cancel];
	[fReceived setLength:0];
	
	[fLock unlock];
}

- (NSData*)receivedData {
	return fReceived;
}

- (NSURL*)url {
	return fURL;
}

- (NSError*)lastError {
	return fError;
}

/////////// NSURLConnection delegate functions //////////////////////////

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
	[fLock lock];
	
	NSAssert(fStatus == DRLoading, @"I should be loadiing.");
	[fReceived appendData:data];
	[fLock unlock];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
	[fLock lock];
	NSAssert(fStatus != DRFinished, @"Why am I here if I already finished.");
	fStatus = DRFinished;
	[fReceived setLength:0];
	[self setError:error];
	[self notifyDidFinish];
	[fLock unlock];
}

- (void)connection:(NSURLConnection *)connection 
				didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
	[fLock lock];
	NSAssert(fStatus != DRFinished, @"Why am I here if I already finished.");
	[fConnection cancel];
	fStatus = DRFinished;
	[fReceived setLength:0];
	[self setError:nil];
	[self notifyDidFinish];
	[fLock unlock];
}

- (NSURLRequest *)connection:(NSURLConnection *)connection 
				willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)redirectResponse {
	return request;
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
	[fLock lock];
	NSAssert(fStatus != DRFinished, @"Why am I here if I already finished.");
	fStatus = DRFinished;
	[self setError:nil];
	[self notifyDidFinish];
	[fLock unlock];
}

/////////////// Private Routines //////////////////

- (void)notifyDidFinish {
	//notify the delegate.
	if (fDelegate && [fDelegate respondsToSelector:@selector(URLDataReceiverDidFinish:)])
		[fDelegate performSelector:@selector(URLDataReceiverDidFinish:) withObject:self];
 }


- (void)setError:(NSError*)error {
	if (fError) [fError release];
	if (error) 
		fError = [error retain];
	else
		fError = nil;
}

@end
