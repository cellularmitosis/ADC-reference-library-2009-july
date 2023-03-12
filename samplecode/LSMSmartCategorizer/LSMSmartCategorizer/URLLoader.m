/*
File: URLLoading.m

Abstract: A Class controlling loading multiple URLs.

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

#import "URLLoader.h"

/*!
 * @abstract Private routines.
 */
@interface URLLoader(Private)
- (void)notifyDidBeginURL:(NSURL*)aURL;
- (void)notifyDidFinishURL:(NSURL*)aURL;
- (void)notifyDidFinishAll;
@end

@implementation URLLoader

- (id)initWithDelegate:(id)delegate {
	self = [super init];
	if (self == nil) return nil;
	
	fFinished = [NSMutableDictionary new];
	fPending = [NSMutableDictionary new];
	fLock = [NSRecursiveLock new];
	
	fDelegate = delegate;
	return self;
}

- (void)dealloc {
	[fLock lock];
	[fPending release];
	[fFinished release];
	[fLock unlock];
	
	[fLock release];
	[super dealloc];
}
	
- (void)load:(NSArray*)URLs {
	[fLock lock];
	[fPending removeAllObjects];
	[fFinished removeAllObjects];
	
	unsigned i=0;
	for (; i<[URLs count]; ++i) {
		//create a URLDataReceiver instance for each URL.
		URLDataReceiver* receiver = [[URLDataReceiver alloc] 
										initWithURL:[URLs objectAtIndex:i]
										delegate:self];
										
		//put the newly created receiver into pending list.
		[fPending setObject:receiver forKey:[URLs objectAtIndex:i]];
		[receiver startLoading];
		[receiver release];
		
		//notify the delegate.
		[self notifyDidBeginURL:[URLs objectAtIndex:i]];
	}
	[fLock unlock];
}

- (void)cancel {
	[fLock lock];
	[fPending removeAllObjects];
	[fFinished removeAllObjects];
	[fLock unlock];
}

- (void)reset {
	[self cancel];
}

- (NSData*)dataForURL:(NSURL*)aURL {
	[fLock lock];
	URLDataReceiver* receiver = [fFinished objectForKey:aURL];
	[fLock unlock];
	
	if (receiver == nil)
		return nil;
	else
		return [receiver receivedData];
}

- (NSEnumerator*)dataEnumerator {
	return [fFinished objectEnumerator];
}

- (NSEnumerator*)urlEnumerator {
	return [fFinished keyEnumerator];
}

/////////////////// URLDataReceiverDelegate Protocol //////////////////
- (void)URLDataReceiverDidFinish:(URLDataReceiver*)dataReceiver {
	[fLock lock];
	NSURL* url = [dataReceiver url];
	
	//move receiver from pending list to finished list.
	[fPending removeObjectForKey:url];
	[fFinished setObject:dataReceiver forKey:url];
	
	//notify delegate that this url has finished.
	[self notifyDidFinishURL:url];
	
	//if pending list is empty, notify delegate that all URLs have finished.
	if ([fPending count] == 0)
		[self notifyDidFinishAll];
	[fLock unlock];
}

///////////////// Private /////////////////////////
- (void)notifyDidBeginURL:(NSURL*)aURL {
	if (fDelegate && [fDelegate respondsToSelector:@selector(URLLoader:didBeginURL:)])
		[fDelegate performSelector:@selector(URLLoader:didBeginURL:) withObject:self withObject:aURL];
}

- (void)notifyDidFinishURL:(NSURL*)aURL {
	if (fDelegate && [fDelegate respondsToSelector:@selector(URLLoader:didFinishURL:)])
		[fDelegate performSelector:@selector(URLLoader:didFinishURL:) withObject:self withObject:aURL];
}

- (void)notifyDidFinishAll {
	if (fDelegate && [fDelegate respondsToSelector:@selector(URLLoaderDidFinishAll:)])
		[fDelegate performSelector:@selector(URLLoaderDidFinishAll:) withObject:self];
}
@end
