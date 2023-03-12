/*
File: DataInfo.h

Abstract: Datasource class for NSOutlineView.

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

#import "DataInfo.h"
#import <PubSub/PubSub.h> /**< Provides new feeds parsing funcationalities.*/

/////////// DataInfo //////////////
@implementation DataInfo

- (unsigned) numberOfChildren {
	return 0;
}

- (BOOL)isLeaf {
	return YES;
}

- (NSString*)title {
	return @"";
}

- (NSURL*)url {
	return nil;
}

- (NSString*)urlString {
	return @"";
}

- (NSString*)score {
	return @"";
}

@end


/////////// URLDataInfo //////////////

@implementation URLDataInfo

- (id)init {
	self = [super init];
	
	if (self) {
		fTitle = nil;
		fURL = nil;
	}
	
	return self;
}

- (id)initWithURL:(NSURL*)aURL andTitle:(NSString*)aTitle {
	[super init];
	
	if (self) {
		fURL = [aURL copy];
		fTitle = [aTitle copy];
	}
	return self;
}

- (void)dealloc {
	if (fURL) [fURL release];
	if (fTitle) [fTitle release];
	[super dealloc];
}

- (unsigned) numberOfChildren {
	return 0;
}

- (BOOL) isLeaf {
	return YES;
}

- (NSString*)title {
	return fTitle;
}

- (NSURL*)url {
	return fURL;
}

- (NSString*)urlString {
	return [fURL absoluteString];
}

@end

//////////// FeedDataInfo //////////////
@implementation FeedDataInfo

- (id)initWithFeed:(PSFeed*)feed {
	self = [super init];
	if (self == nil) return nil;
	
	fFeed = [feed retain];
	fScore = nil;
	
	return self;
}

- (void)dealloc {
	[fFeed release];
	if (fScore) [fScore release];
	[super dealloc];
}

- (unsigned)numberOfChildren {
	return 0;
}

- (BOOL)isLeaf {
	return YES;
}

- (NSString*)title {
	return [fFeed title];
}

- (NSURL*)url {
	return [fFeed URL];
}

- (NSString*)urlString {
	return [[fFeed URL] absoluteString];
}

- (NSString*)plainText {
	NSMutableString* finalString = [NSMutableString string];
	
	//cancatenate plain text from each entry.
	NSEnumerator* entryEnum = [fFeed entryEnumeratorSortedBy:nil];
	PSEntry* entry;
	while (entry = [entryEnum nextObject]) {
		NSString* theTitle = [entry title];
		[finalString appendString:theTitle];
		
		//not every entry has all the fields.
		//so we need to check for nil.
		PSContent* content;
		if (content = [entry content])
		{
			NSString* plainText = [content plainTextString];
			if (plainText)
				[finalString appendString:plainText];
		}
		
		if (content = [entry summary])
		{
			NSString* plainText = [content plainTextString];
			if (plainText)
				[finalString appendString:plainText];
		}
	}
	
	return finalString;
}

- (NSString*)score {
	return [fScore stringValue];
}

- (void)setScore:(NSNumber*)score {
	if (fScore) [fScore release];
	fScore = [score retain];
}

- (PSFeed*)feed {
	return fFeed;
}

@end

/////////// CategoryDataInfo //////////////
@implementation CategoryDataInfo

- (id) init {
	self = [super init];
	if (self) {
		fTitle = nil;
		fChildren = [NSMutableArray new];
	}
	return self;
}

- (id) initWithTitle:(NSString*)aTitle {
	self = [super init];
	if (self) {
		fTitle = [aTitle copy];
		fChildren = [NSMutableArray new];
	}
	return self;
}

- (void) dealloc {
	if (fTitle) [fTitle release];
	if (fChildren) [fChildren release];
	[super dealloc];
}

- (unsigned) numberOfChildren {
	return [fChildren count];
}

- (BOOL) isLeaf {
	return NO;
}

- (NSString*) title {
	return fTitle;
}

- (NSURL*) url {
	return nil;
}

- (void)addChild:(DataInfo*)child {
	[fChildren addObject:child];
}

- (NSEnumerator*)childEnumerator {
	return [fChildren objectEnumerator];
}

- (DataInfo*) childAt:(unsigned)index {
	return [fChildren objectAtIndex:index];
}

- (void)removeAllChildren {
	[fChildren removeAllObjects];
}
@end