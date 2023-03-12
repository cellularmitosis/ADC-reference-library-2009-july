/*
	File:		MovieWindowController.mm

	Abstract:	Implements the document's movieview window controller.
				This controller manages the movie view, validates the items
				in the Movie menu and implements the actions associated with
				the items in the Movie menu.


	Version:	1.0

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

	Copyright © 2006-2008 Apple Inc. All Rights Reserved.
*/

#import "MovieWindowController.h"

#define kDefaultWidthForNonvisualMovies	320

@implementation MovieWindowController

#pragma mark
#pragma mark ---- Init/De-alloc ----
- (id) init 
{
	// Load the nib owned by this window controller
	[super initWithWindowNibName:@"MovieWindow"];
	return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	[uiMovieView setMovie:nil];
    [super dealloc];
}


#pragma mark
#pragma mark --- NSWindowController overrride ---
- (void) windowDidLoad
{
	QTMovie *theMovie = (QTMovie*)[[self document] movie]; 
	
	// This is the primary document window. If it is
	// closed, the document is considered closed.
	[self setShouldCloseDocument:YES];
	
	if (theMovie) 
	{
        NSSize contentSize = [[theMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
        [uiMovieView setMovie:theMovie];
		if ([uiMovieView isControllerVisible])
        {
            contentSize.height += [uiMovieView movieControllerBounds].size.height;
            if (contentSize.width == 0) 
			{
                contentSize.width = kDefaultWidthForNonvisualMovies;
            }
        }
        [[uiMovieView window] setContentSize:contentSize];
		
		// Watch for movie resizings
		[[NSNotificationCenter defaultCenter] addObserver:self 
												selector:@selector(sizeWindowToMovie:) 
												name:QTMovieSizeDidChangeNotification 
												object:theMovie];

    }
    else 
	{
        [uiMovieView setMovie:[QTMovie movie]];
	}
}

#pragma mark
#pragma mark --- NSMenu validation protocol ---
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    BOOL	valid = NO;
    SEL		action;

    // init
    action = [menuItem action];

    // validate
    if (action == @selector(doExport:))
	{
        valid = ([[self document] movie] != nil);
	}
	else if (action == @selector(doSetFillColourPanel:))
    {
	    valid = YES;
    }
	else if (action == @selector(doShowController:))
    {
		[menuItem setState:([uiMovieView isControllerVisible] ? NSOnState : NSOffState)];
		valid = YES;
    }
    else if (action == @selector(doPreserveAspectRatio:))
    {
        [menuItem setState:([uiMovieView preservesAspectRatio] ? NSOnState : NSOffState)];
        valid = ([[self document] movie] != nil);
    }
    else if ((action == @selector(doLoop:)) && [[self document] movie])
    {
        [menuItem setState:([[(QTMovie*)[[self document] movie] attributeForKey:QTMovieLoopsAttribute] boolValue] ? NSOnState : NSOffState)];
        valid = YES;
    }
    else if ((action == @selector(doLoopPalindrome:)) && [[self document] movie])
    {
        if ([[(QTMovie*)[[self document] movie] attributeForKey:QTMovieLoopsAttribute] boolValue])
        {
            [menuItem setState:([[(QTMovie*)[[self document] movie] attributeForKey:QTMovieLoopsBackAndForthAttribute] boolValue] ? NSOnState : NSOffState)];
            valid = YES;
        }
    }
    else if (action == @selector(doClone:))
    {
	    valid = ([[self document] movie] != nil);
	}
    else if (action == @selector(selectAll:))
    {
	    valid = ([[self document] movie] != nil);
	}
    else if (action == @selector(setSelectionToNone:))
    {
	    valid = ([[self document] movie] != nil);
	}
    else
    {
	    valid = [super validateMenuItem:menuItem];
	}
    return valid;
}

#pragma mark
#pragma mark --- Getters ---
- (QTMovieView *)movieView 
{
	return uiMovieView;
}

- (NSView *)exportAccessoryView 
{
	return uiExportAccessoryView;
}

- (NSPopUpButton *)exportTypePopUpButton 
{
	return uiExportTypePopUpButton;
}


#pragma mark
#pragma mark --- IB actions ---

// Actions ---
- (IBAction)doExport:(id)sender
{
	
    NSSavePanel *savePanel;
	
	// init
	savePanel = [NSSavePanel savePanel];

    // run the export sheet
    [savePanel setAccessoryView:uiExportAccessoryView];
	[savePanel beginSheetForDirectory:nil file:[[[self document] fileName] lastPathComponent] modalForWindow:[self window] modalDelegate:self
        didEndSelector:@selector(exportPanelDidEnd: returnCode: contextInfo:) contextInfo:nil];

}

- (IBAction)doSetFillColourPanel:(id)sender
{
    NSColorPanel *colorPanel;

    // init
    colorPanel = [NSColorPanel sharedColorPanel];
    [colorPanel setAction:@selector(doSetFillColour:)];
    [colorPanel setTarget:self];
    [colorPanel setColor:[uiMovieView fillColor]];

    // run the panel
    [colorPanel makeKeyAndOrderFront:nil];
}

- (IBAction)doSetFillColour:(id)sender
{
    // update the fill color
    [uiMovieView setFillColor:[sender color]];
}

- (IBAction)doShowController:(id)sender
{
    // toggle the controller visibility
    [uiMovieView setControllerVisible:([sender state] == NSOffState)];
}

- (IBAction)doPreserveAspectRatio:(id)sender
{
    // toggle the aspect ratio preservation
    [uiMovieView setPreservesAspectRatio:([sender state] == NSOffState)];
}

- (IBAction)doLoop:(id)sender
{
    // toggle looping
    [(QTMovie*)[[self document] movie] setAttribute:[NSNumber numberWithBool:([sender state] == NSOffState)] forKey:QTMovieLoopsAttribute];
}

- (IBAction)doLoopPalindrome:(id)sender
{
    // toggle palindrome looping
    [(QTMovie*)[[self document] movie] setAttribute:[NSNumber numberWithBool:([sender state] == NSOffState)] forKey:QTMovieLoopsBackAndForthAttribute];
}

- (IBAction)doClone:(id)sender
{
	//init
    MovieDocument *movieDocument = [MovieDocument movieDocumentWithMovie:(QTMovie*)[[[[self document] movie] copy] autorelease]];
	
    // set up the document
    if (movieDocument)
    {
        // add the document
        [[NSDocumentController sharedDocumentController] addDocument:(NSDocument*)movieDocument];

        // set up the document
        [movieDocument makeWindowControllers];
        [movieDocument showWindows];
    }
}

#pragma mark
#pragma mark --- NSSavePanel delegate ---
- (void)exportPanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    UInt32				selectedItem;
    NSMutableDictionary	*settings = nil;
    static NSArray 		*exportTypes = nil;

    // init
    if (exportTypes == nil)
    {
        exportTypes = [[NSArray arrayWithObjects:
            [NSNumber numberWithLong:'AIFF'], [NSNumber numberWithLong:'BMPf'], [NSNumber numberWithLong:'dvc!'],
            [NSNumber numberWithLong:'FLC '], [NSNumber numberWithLong:'mpg4'], [NSNumber numberWithLong:'MooV'],
            [NSNumber numberWithLong:'embd'], nil] retain];
    }

    // export
    if (returnCode == NSOKButton)
    {
        // init
        selectedItem = [uiExportTypePopUpButton indexOfSelectedItem];
		settings = [NSMutableDictionary dictionaryWithCapacity:5];
        [settings setObject:[NSNumber numberWithBool:YES] forKey:QTMovieExport];

        if ((selectedItem >= 0) && (selectedItem < [exportTypes count]))
        {
		    [settings setObject:[exportTypes objectAtIndex:selectedItem] forKey:QTMovieExportType];
		}	
        // export
        if (![(QTMovie*)[[self document] movie] writeToFile:[sheet filename] withAttributes:settings])
		{
		   NSRunAlertPanel(@"Error", @"Error exporting movie.", nil, nil, nil);
		}   
    }
}


#pragma mark 
#pragma mark --- Notification callback ---
- (void)sizeWindowToMovie:(NSNotification *)notification
{
    NSRect  currWindowBounds, newWindowBounds;
	NSSize  contentSize;
	NSPoint	topLeft;
    static BOOL	nowSizing = NO;
    
    if (nowSizing)
	{
        return;
    }    
    nowSizing = YES;
    
    // get the current location and size of the movie window, so we can keep the top-left corner pegged, i.e. fixed
	currWindowBounds = [[uiMovieView window] frame];

	topLeft.x = currWindowBounds.origin.x;
    topLeft.y = currWindowBounds.origin.y + currWindowBounds.size.height;

    contentSize = [[(QTMovie *)[[self document] movie] attributeForKey:QTMovieCurrentSizeAttribute] sizeValue];
    if ([uiMovieView isControllerVisible])
	{
		// adjust for movie controller height
        contentSize.height += [uiMovieView controllerBarHeight];
		// take into account size difference between movie view
		// and enclosing window
		contentSize.width += (currWindowBounds.size.width - contentSize.width);
	}

    if (contentSize.width == 0)
    {
	    contentSize.width = currWindowBounds.size.width;
	}
    newWindowBounds = [[uiMovieView window] frameRectForContentRect:NSMakeRect(0, 0, contentSize.width, contentSize.height)];

    [[uiMovieView window] setFrame:NSMakeRect(topLeft.x, topLeft.y - newWindowBounds.size.height, newWindowBounds.size.width, newWindowBounds.size.height) display:YES];
    
    nowSizing = NO;
}
@end