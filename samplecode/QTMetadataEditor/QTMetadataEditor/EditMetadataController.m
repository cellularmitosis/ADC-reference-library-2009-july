//
// File:		EditMetadataController.m
//
// Abstract:	The NSWindowController subclass for the CDMetadataBrowser's main document
//				window.	Allows displaying and editing of various metadata objects in the UI.
//
// Version:		1.0
//				Originally introducted at WWDC 2008 at Session:
//				Extending and Integrating Post-Production Applications with Final Cut Pro
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//

#import "EditMetadataController.h"
#import "MyDocument.h"

@implementation EditMetadataController
-(void)setShouldRefresh:(BOOL)shouldRefresh{
	myShouldRefresh = shouldRefresh;
}
-(BOOL)shouldRefresh{
	return myShouldRefresh;
}

# pragma mark split pane management
//since not all data will have a image representation, we want to hide and show the image
//preview half of the split pane
-(void)collapseBinData{
	NSRect oldBinDataRect;
	oldBinDataRect = [myBinDataPane frame];
	[myBinDataPane setFrameSize:NSMakeSize(0, oldBinDataRect.size.height)];
	[mySplitView display];
}
-(void)uncollapseBinData{
	NSRect oldBinDataRect;
	NSRect oldTextRect;
	oldBinDataRect = [myBinDataPane frame];
	oldTextRect = [myValueInspectorView frame];
	oldTextRect.size.width = (oldTextRect.size.width+oldBinDataRect.size.width-116);
	oldBinDataRect.size.width = 116;
	[myValueInspectorView setFrameSize:oldTextRect.size];
	[myBinDataPane setFrameSize:oldBinDataRect.size];

	[mySplitView display];
}

# pragma mark item to edit
- (void)setEditItem:(MetadataAttributeMO *)itemToEdit{
	if (myEditedItem != nil){
		[myEditedItem release];
		myEditedItem = nil;
	}
	if (myCachedValues != nil){
		[myCachedValues release];
		myCachedValues = nil;
	}
	if (itemToEdit != nil){
		//save the pointer of the item we're editing
		myEditedItem = [itemToEdit retain];

		//save off the original values (to implement cancel)
		myCachedValues = [[itemToEdit dictionaryRepresentation] retain];
	}

	//init 'value' with data - don't do with bindings so we can control bytes properly.
	[self refreshData:self];
}


- (BOOL)editValue:(MetadataAttributeMO *)itemToEdit{
	BOOL		success = YES;
	
	[self setShouldRefresh: YES];
	
	//save the pointer of the item we're editing
	[self setEditItem:itemToEdit];

	//show the inspector window
	[[self window] makeKeyAndOrderFront:self];
	return success;
}

#pragma mark UI actions

- (IBAction)refreshData:(id)sender{
	if ([self shouldRefresh]){
		if (myEditedItem != nil){
			[myEditedItem setControlwithData:myValueInspectorView forEdit:FALSE];
		}
		else [myValueInspectorView setStringValue:@""];
		NSData *binData = [myEditedItem valueForKey:@"dataBin"];
		if (binData != nil){
			NSImage* preview = [[NSImage alloc] initWithData:binData];
			[myImageView setImage:preview];
			[self uncollapseBinData];
		} else {
			[myImageView setImage:nil];
			[self collapseBinData];
		}
	}
}

- (IBAction)commitEditedValue:(id)sender{
	//finish editing
	[myValueInspectorView selectText:self];
	
	// convert from data strings to whatever types and store them.
	if (myEditedItem != nil)
		[myEditedItem setDataForString:[myValueInspectorView stringValue]];
	
	//close out the window
	[myCachedValues release];
	myCachedValues = nil;
	[[self window] performClose:self];
}

- (IBAction)cancelEditedValue:(id)sender{
	//throw away the edited values
	[myValueInspectorView abortEditing];

	//restore the original values
	if (myEditedItem != nil){
		[myEditedItem setValuesForKeysWithDictionary:myCachedValues];
		[myCachedValues release];
		myCachedValues = nil;
	}

	//close the inspector
	[[self window] performClose:self];
}

//override windowShouldClose to cleanup editing
- (BOOL)windowShouldClose:(id)sender{

	[self setShouldRefresh: NO];
	
	//if this window close wasn't initiated by OK or Cancel buttons, cancel
	if (myCachedValues != nil){
		[self cancelEditedValue:sender];
	} else if (myEditedItem){
		//cleanup
		[myEditedItem release];
		myEditedItem = nil;
	}
	
	return YES;
}

#pragma mark editing
/* to support drag and drop in the related text field we have the following two functions:
- (BOOL)addDataFromFilePath:(NSString *)filePath;
- (BOOL)addDataFromFileURL:(NSURL *)fileURL;
*/
- (BOOL)addData:(NSData *)contentsOfFile{
	BOOL success = NO;
	if (contentsOfFile != nil && myEditedItem!= nil && [myEditedItem supportsBinaryFileDrop]){
		[myEditedItem setValue:contentsOfFile forKey:@"data"];
		[self refreshData:self];
		success = YES;
	}
	return success;
}
- (BOOL)addDataFromFilePath:(NSString *)filePath{
	return [self addData:[NSData dataWithContentsOfFile:filePath]];
}
- (BOOL)addDataFromFileURL:(NSURL *)fileURL{
	return [self addData:[NSData dataWithContentsOfURL:fileURL]];
}

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor{
	if (myEditedItem != nil){
		//fill the UI with data from the item to edit
		[myEditedItem setControlwithData:myValueInspectorView forEdit:TRUE];
		return TRUE;
	}
	return FALSE;
}
- (void)controlTextDidEndEditing:(NSNotification *)aNotification{
	if (myEditedItem != nil){
		//push the changes added to the hex editor back to the object being edited,
		[myEditedItem setDataForString:[myValueInspectorView stringValue]];
		
		//refresh the UI with the changes
		[myEditedItem setControlwithData:myValueInspectorView forEdit:FALSE];
	}
}

@end
