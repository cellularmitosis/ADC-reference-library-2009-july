/*
File: EvalWindowController.m

Abstract: Controller of the evaluation window.

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

Copyright © 2007 Apple Inc., All Rights Reserved

*/
#import "EvalWindowController.h"
#import "DataInfo.h"
#import "URLLoader.h"
#import <PubSub/PubSub.h>
#import <LSMClassifier.h>

enum {
	kSheetReturnOK,
	kSheetReturnCancel
};

/*!
 * @abstract Private routines.
 */
@interface EvalWindowController(Private)

/*!
 * @abstract Categorize the data.
 */
- (void) processFeedData:(NSData*)data fromURL:(NSURL*)url;

/*!
 * @abstract Recursively find all files in a directory and its sub directories.
 *           And add those file paths into array.
 */
- (void) appendPathsAt:(NSString*)path toURLArray:(NSMutableArray*)array;


/*!
 * @abstract Callback function used by [NSApp beginSheet:modalForWindow:modalDelegate:didEndSelector:contextInfo:],
 *           when open the sheet to ask users to enter a URL.
 */
- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode  contextInfo:(void  *)contextInfo;
@end

@implementation EvalWindowController

- (void) awakeFromNib {
	topLevelDataInfo = [[CategoryDataInfo alloc] initWithTitle:@"Categories"];
	_urlLoader = [[URLLoader alloc] initWithDelegate:self];
	_classifier = [LSMClassifier new];
	
	[outlineView expandItem:topLevelDataInfo];
}

- (void)dealloc
{		
	
	if (topLevelDataInfo)
		[topLevelDataInfo release];
	
	if (_urlLoader)
		[_urlLoader release];
	
	if (_classifier)
		[_classifier release];
	
	[super dealloc];
}


- (IBAction)doLoadMap:(id)sender {
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	[panel setCanChooseFiles:YES];
	[panel setCanChooseDirectories:NO];
	
	NSMutableArray* allowedTypes = [NSMutableArray new];
	[allowedTypes addObject:@"lsm"];
	if ([panel runModalForDirectory:@"~" file:nil types:allowedTypes] == NSOKButton) {
		NSString* mapPath = [[panel filenames] objectAtIndex:0];
		
		//read the map into classifier
		if ([_classifier readFromFile:mapPath with:kLSMCEvaluation] == noErr) {
			[self log:[NSString stringWithFormat:@"Loaded map from %@\n", mapPath]];
			[topLevelDataInfo removeAllChildren];
			NSEnumerator* mapCatEnum = [_classifier categoryEnumerator];
			NSString* mapCatName;
			
			//add available category names in the map into the outline view data source.
			while (mapCatName = [mapCatEnum nextObject]) {
				CategoryDataInfo* catInfo = [[CategoryDataInfo alloc] initWithTitle:mapCatName];
				[topLevelDataInfo addChild:catInfo];
				[catInfo release];
			}
			
			//update the outline view.
			[self reloadOutlineView];
			[self setBusy:NO];
		}
		else {
			[self log:[NSString stringWithFormat:@"Failed to load map from %@\n", mapPath]];
		}
	}
	[allowedTypes release];
}

- (IBAction)doAddFile:(id)sender {
	//Ask the user to choose the file or files in a directory that he/she wants to categorize.
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	[panel setCanChooseFiles:YES];
	[panel setCanChooseDirectories:YES];
	[panel setAllowsMultipleSelection:YES];
	
	if ([panel runModalForDirectory:@"~" file:nil types:nil] == NSOKButton) {
		NSArray* selected = [panel filenames];
		NSMutableArray* pendingURLs = [NSMutableArray array];
		unsigned i=0;
		for (; i<[selected count]; ++i) {
			[self appendPathsAt:[selected objectAtIndex:i] toURLArray:pendingURLs];
		}
		
		//Start loading the URLs. (Asynchronously)
		[_urlLoader load:pendingURLs];
		
		//set UI to busy.
		[self setUICancellableBusy:@"Fetching data ..."];		
		
	}
}

- (void)doAddURL:(id)sender {
	//Open a sheet to ask the user to enter an URL that he/she wants to categorize.
	//we pre-store some URLs in the bundle, read those URLs.
	NSString* testFilePath =
			[[NSBundle mainBundle] pathForResource:@"test_urls" ofType:@"plist"];	
	if (testFilePath) {
		NSArray* testURLs = [NSArray arrayWithContentsOfFile:testFilePath];
		if (testURLs)
			[urlBox addItemsWithObjectValues:testURLs];
	}

	[urlBox setStringValue:@"http://"];
	[NSApp beginSheet:openURLSheet modalForWindow:window  modalDelegate:(self) 
		didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

 - (void)doCancel:(id)sender {
	[_urlLoader reset];
	[self log:@"Cancelled by user\n"];
	[self setUIIdle];
}

- (void)doCancelSheet:(id)sender {
	[NSApp endSheet:openURLSheet returnCode:kSheetReturnCancel];
}

- (void)doOKSheet:(id)sender {
	[NSApp endSheet:openURLSheet returnCode:kSheetReturnOK];
}

//////////////////// Private //////////////////////
- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode  contextInfo:(void  *)contextInfo {
	[sheet orderOut:self];
	
	//return if user pressed "Cancel"
	if (returnCode == kSheetReturnCancel) return;
	
	NSMutableString* enteredStr = [NSMutableString stringWithString:[[urlBox stringValue] lowercaseString]];
	
	//If the user use "feed:" as URL schema, replace it with "http:",
	//because NSURLConnection doesn't support "feed:".
	if ([enteredStr hasPrefix:@"feed:"]) {
		[enteredStr replaceCharactersInRange:NSMakeRange(0,5) withString:@"http:"];
	}
	
	NSArray* pendingURLs = [NSArray arrayWithObject:[NSURL URLWithString:enteredStr]];
	
	//Start loading the URL. (Asynchronously)
	[_urlLoader load:pendingURLs];
	
	//set UI to busy.
	[self setUICancellableBusy:@"Fetching data ..."];
}


- (void) processFeedData:(NSData*) data  fromURL:(NSURL*)url {
	if (data == nil) {
		[self log:[NSString stringWithFormat:@"Failed to read %@\n", url]];
	}
	else {
		//create a PSFeed instance.
		PSFeed* feed = [[PSFeed alloc] initWithData:data URL:url];
		if (feed == nil || [feed title] == nil) 
			[self log:[NSString stringWithFormat:@"Failed to parse data from %@\n", url]];
		else {
			FeedDataInfo* feedInfo = [[FeedDataInfo alloc] initWithFeed:feed];
			
			//get categorization result. Here we are only interested in the best matching category.
			LSMClassifierResult* result = [_classifier createResultFor:[feedInfo plainText] upTo:1 with:0];
			if (result == nil) 
				[self log:[NSString stringWithFormat:@"Failed to categorize feed \"%@\"\n", [feedInfo title]]];
			else {
				NSString* catName = [result getCategoryName:0];
				[self log:[NSString stringWithFormat:@"feed \"%@\" matches category \"%@\" with score %@\n",
				[feedInfo title], catName, [result getScore:0]]];
				[feedInfo setScore:[result getScore:0]];
				
				//add the feed into corresponding category in the outline view data source.
				NSEnumerator* catEnum = [topLevelDataInfo childEnumerator];
				CategoryDataInfo* catInfo;
				while(catInfo = [catEnum nextObject]) {
					if ([[catInfo title] isEqualToString:catName]) {
						[catInfo addChild:feedInfo];
						break;
					}
				}
			}
			[feedInfo release];
		}
		
		if (feed) [feed release];
	}
	
}

- (void) appendPathsAt:(NSString*)path toURLArray:(NSMutableArray*)array {
	if ([path hasPrefix:@"."]) return;
	
	NSFileManager* fileManager = [NSFileManager defaultManager];
	BOOL isDir;
	
	if (![fileManager fileExistsAtPath:path isDirectory:&isDir]) return;
	
	if (isDir) {
		//If it's a directory, recursively call this routine.
		NSArray* contents = [fileManager directoryContentsAtPath:path];
		NSEnumerator* contentEnum = [contents objectEnumerator];
		NSString* contentName;
		while (contentName = [contentEnum nextObject]) {
			if ([contentName hasPrefix:@"."]) continue;
			[self appendPathsAt:[NSString stringWithFormat:@"%@/%@", path, contentName] toURLArray:array];
		}
	}
	else
		//If it's a file, append its path to array.
	[array addObject:[NSURL fileURLWithPath:path]];
	
	
}


////////////////// URLoader delegate //////////////////
- (void)URLLoader:(URLLoader*)theURLLoader didBeginURL:(NSURL*)aURL {
	[self log:[NSString stringWithFormat:@"Began loading %@\n", aURL]];
}

- (void)URLLoader:(URLLoader*)theURLLoader didFinishURL:(NSURL*)aURL {
	[self log:[NSString stringWithFormat:@"Finished loading %@\n", aURL]];
}

- (void)URLLoaderDidFinishAll:(URLLoader*)theURLLoader {
	[self setUIAllBusy:@"Parsing data ..."];
	
	//we have done fetching all URI, now parse them.
	NSEnumerator* urlEnum = [_urlLoader urlEnumerator];
	NSURL* url;
	
	//check if all URLs have been loaded successfully.
	while(url = [urlEnum nextObject]) {
		NSData* data = [_urlLoader dataForURL:url];
		if (data == nil) {
			[self log:[NSString stringWithFormat:@"Failed to load from %@\n", url]];
			continue;
		}	
		
		[self processFeedData:data fromURL:url];
		[self reloadOutlineView];
	}
	[_urlLoader reset];
	
	[self setUIIdle];
}


@end
