/*

File: AEController.m

Abstract:   UI controller class implementation for Apple Event example 
            application (FCP_AE_Tester).

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

#import "AEController.h"
#import "FCP_AppleEvents.h"
#import "apple_event.h"


@implementation AEController

- (id)init
{
    self = [super initWithWindowNibName:@"AEDialog"];
    if (self) {
		msgToSend = 0;
		fileURL = nil;
		fileSendMode = sendAsFSRef;
    }
    return self;
}

- (void)windowDidLoad
{
	[self addColumnItems:columnPopUp1];
	[self addColumnItems:columnPopUp2];
}

- (void)addColumnItems:(NSPopUpButton *)button
{
	char *list[] = {
		"name",
		"duration",
		"in",  
		"out", 
		"start",
		"end", 
		"scene",
		"shottake",
		"lognote",
		"good",
		"label",
		"label2",
		"mastercomment1",
		"mastercomment2",
		"mastercomment3",
		"mastercomment4",
		"clipcommenta",
		"clipcommentb",
		0
	};
	char **p;
	
	for (p = list; *p != 0; p++) {
		[button addItemWithTitle:[NSString stringWithCString:*p encoding:NSASCIIStringEncoding]];
	}
}

- (IBAction)clearProjectData:(id)sender
{
	if (fileURL != nil)
		[fileURL release];
	
	fileURL = nil;

	[fileToSend setStringValue: @""];
}

- (IBAction)switchToPath:(id)sender
{
	NSString *pathString = nil;
	
	if (fileURL != nil) {
		pathString = [fileURL path];
		[fileURL release];
	}
	fileURL = nil;
	
	if (pathString == nil) {
		[fileToSend setStringValue: @""];
	} else {
		[fileToSend setStringValue: pathString];
	}
	[fileToSend setEditable:YES];
}

- (IBAction)chooseFileToSend:(id)sender
{
	NSOpenPanel * op = [NSOpenPanel openPanel];
	
	// pick a media file or a project, depending on the selected command
	if (msgToSend == kFCPUpdateMediaFile)
	{
		[op setTitle:@"Select a media file"];
		[op setCanChooseFiles: YES];
		[op setCanChooseDirectories: NO];
		[op setAllowsMultipleSelection:NO];
		
		[op beginSheetForDirectory:nil file:nil types: [NSArray arrayWithObjects:@"mov", @"MooV", nil]
					modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) 
					   contextInfo:nil];
	}
	else
	{
		[op setTitle:@"Select an FCP project file"];
		[op setCanChooseFiles: YES];
		[op setCanChooseDirectories: NO];
		[op setAllowsMultipleSelection:NO];
		
		[op beginSheetForDirectory:nil file:nil types: [NSArray arrayWithObjects:@"fcp",nil]
					modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) 
					   contextInfo:nil];
	}
}

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	if (returnCode == NSOKButton)
	{
		[self setSelectedFileURL:[[(NSOpenPanel*)sheet URLs] objectAtIndex:0]];
		
		NSString *pathString = [fileURL path];
		
		if (pathString != nil)
		{
			NSString *newShortName = [pathString lastPathComponent];
			
			[fileToSend setStringValue: newShortName];
		}
		[fileToSend setEditable:NO];
	}
}

- (void)setSelectedFileURL:(NSArray *)newURL
{
	if (newURL != nil)
	{
		// Release the old path
		if (fileURL != nil)
			[fileURL release];
		
		// Save the new URL
		fileURL = [newURL retain];
	}
}

- (IBAction)setSendMode:(id)sender
{
	fileSendMode = [sender tag];
}

- (IBAction)setMsg:(id)sender
{
	int tag;
	int tab;
	OSType msg;
	
	if (sender == nil) {
		// this is Controller.m sending us a "choose default"
		tag = 0;
	} else {
		tag = [sender tag];
	}
	
	switch (tag) {
	case 0:		msg = kFCPOpenProjectFile;			tab = 1;	break;
	case 1:		msg = kFCPSaveAndCloseProjectFile;	tab = 2;	break;
	case 2:		msg = kFCPGetDocumentXML;			tab = 3;	break;
	case 3:		msg = kFCPImportXMLToDocument;		tab = 4;	break;
	case 4:		msg = kFCPSelectItemInBrowser;		tab = 7;	break;
	case 5:		msg = kFCPOpenItemInProject;		tab = 5;	break;
	case 6:		msg = kFCPFindItemsInProject;		tab = 6;	break;
	case 7:		msg = kFCPGetItemXML;				tab = 3;	break;
	case 8:		msg = kFCPGetAllEffectsXML;			tab = 3;	break;
	case 9:		msg = kFCPGetAllOpenProjects;		tab = 1;	break;
	case 10:	msg = kFCPUpdateMediaFile;			tab = 1;	break;

	default:	return;
	}
	
	if (msg == kFCPGetItemXML) {
		[uuidLabel setHidden:NO];
		[uuidToGet setHidden:NO];
	} else {
		[uuidLabel setHidden:YES];
		[uuidToGet setHidden:YES];
	}
	
	[event selectItemWithTag:tag];
	[tabView selectTabViewItemWithIdentifier:[[NSNumber numberWithInt:tab] stringValue]];
	msgToSend = msg;
}

- (IBAction)sendAppleEvent:(id)sender
{
	apple_event *evt = [[apple_event alloc] init];
	[evt create:msgToSend class:kFCPEventClass dest:kFCPEventClass];
	
	if (msgToSend ==  kFCPGetAllEffectsXML) {
		[evt version:[xmlVersion stringValue]];
		[evt send];
		[evt createDocFromXML:([reformatButton state] == NSOnState)];

	} else if (msgToSend == kFCPGetAllOpenProjects) {
		[evt send];
		[evt createDocFromList];
		
	} else if (msgToSend == kFCPUpdateMediaFile) {
		[evt sendAFile:[fileToSend stringValue] 
					url:fileURL 
					key:kFCPMediaFileKey
					as:fileSendMode];
		[evt send];

	} else {
		// do project related event
		[evt sendAFile:[fileToSend stringValue] 
					url:fileURL 
					key:kFCPProjectFileKey
					as:fileSendMode];

		if (msgToSend == kFCPOpenProjectFile) {
			[evt send];

		} else if (msgToSend == kFCPSaveAndCloseProjectFile) {
			[evt save:[saveButton state]];
			[evt send];

		} else if (msgToSend == kFCPGetDocumentXML) {
			[evt version:[xmlVersion stringValue]];
			[evt send];
			[evt createDocFromXML:([reformatButton state] == NSOnState)];

		} else if (msgToSend == kFCPImportXMLToDocument) {
			[evt addDoc];
			[evt send];

		} else if (msgToSend == kFCPFindItemsInProject) {
			[evt logicOr:[logicButton state]];
			[evt criteria:[stringToFind1 stringValue]
					match:[[matchPopUp1 selectedItem] tag]
				   column:[columnPopUp1 titleOfSelectedItem]
					 omit:([omitButton1 state] == NSOnState)];
			if (! [[stringToFind2 stringValue] isEqualToString:@""]) {
				[evt criteria:[stringToFind2 stringValue]
						match:[[matchPopUp2 selectedItem] tag]
					   column:[columnPopUp2 titleOfSelectedItem]
						 omit:([omitButton2 state] == NSOnState)];
			}
			[evt addList];
			[evt send];

		} else if (msgToSend == kFCPOpenItemInProject) {
			[evt uuid:[uuidToActOn stringValue]];
			[uuidToGet setStringValue:[uuidToActOn stringValue]];
			[evt send];

		} else if (msgToSend == kFCPSelectItemInBrowser) {
			[evt uuids:[uuidsToSelect string]];
			[evt send];

		} else if (msgToSend == kFCPGetItemXML) {
			[evt version:[xmlVersion stringValue]];
			[evt uuid:[uuidToGet stringValue]];
			[uuidToActOn setStringValue:[uuidToGet stringValue]];
			[evt send];
			[evt createDocFromXML:([reformatButton state] == NSOnState)];

		} else {
			// do nothing - unrecognized message
		}
	}
}

@end
