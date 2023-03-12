//  Controller.m
//  TextLinks

//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//	See legal notice at end of file.

//	This application demonstrates how to programmatically create links in text,
//	and optionally handle clicks in those links. See README.rtf for more information.

//	MSM
//	version 0.2 July 2003 -- fixes logic for mouse tracking
//	version 0.1 February 2003


#import "Controller.h"


//	Tags in createLinkType NSMatrix
typedef enum
{	linkToURLFromSelectionTag = 0,		// create a link which shows the URL
    linkToOtherURLTag = 1,				// create a link which doesn't show the URL
    linkToAlertTag = 2					// create a link to something other than a URL
} linkTag;

typedef enum
{	linkShowLinkInfo = 0,				// show the NSLink part of the attributes
    linkShowAttributes = 1				// show all attributes
} showTag;


@implementation Controller

#pragma mark PRIVATE INSTANCE METHODS

//	Return any text selected in the main text-view, cleaned up a little bit.
- (NSString *) selectedText
{
    NSString	*result;

    result = [[[textView textStorage] string] substringWithRange: [textView selectedRange]];

    //	Remove trailing newlines and other junk, which will mess up the radio button we fill in.
    //	(Really should remove newlines in the middle, too.)
    return [result stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]];
}

- (NSObject *) urlFromString: (NSString *) aString; // RETURN: nil means user's been alerted to the error
{
    NSURL	*url;

    url = [NSURL URLWithString: aString];
    if (url == nil)
        NSRunAlertPanel (@"Create Link",
            @"Ò%@Ó isnÕt a valid URL",
            nil, nil, nil,
            aString);					// argument for %@ in the string above

    return url;
}

//	Create and return the object which we'll link to. This can be an NSURL for a traditional link,
//	or anything else.
//	This method gets invoked only from -createLink:, and is broken out to simplify that method.
- (NSObject *) linkObject;				// RETURN: nil means user's been alerted to the error
{
    NSString	*alertText;

    switch ((linkTag) [createLinkTypeMatrix selectedTag])
    {
        //	If the user selects Òhttp://www.apple.comÓ, create a link to that address.
        case linkToURLFromSelectionTag:
            return [self urlFromString: [self selectedText]];

        //	No matter what the user selected, create a link to the address in the field.
        case linkToOtherURLTag:
            if ([createLinkURLField stringValue] == 0)
            {
                [createLinkURLField selectText: nil]; // make it easy for them to correct
                NSRunAlertPanel (@"Create Link", @"Please enter a URL.", nil, nil, nil);
                return nil;
            }

            return [self urlFromString: [createLinkURLField stringValue]];

        //	No matter what the user selected, create a link to an NSString instance.
        case linkToAlertTag:
            alertText = [createLinkAlertField stringValue];
            if ([alertText length] == nil)
            {
                [createLinkAlertField selectText: nil];	// make it easy for them to correct
                NSRunAlertPanel (@"Create Link", @"Please enter the text to alert.", nil, nil, nil);
                return nil;
            }
            return alertText;
            break;

        default:						// don't expect this
            return nil;
    }
}

//	Find the attributes of the text the mouse is pointing to in the NSTextView and display them.
- (void) showAttributesUnderMouse
{
    NSPoint			mouseLoc, mouseLocInContentView;
    NSView			*contentView;
    int				glyphIndex, charIndex;
    NSDictionary	*attributes;

    //	Limit the work we do if the app isnÕt active
    //	(Would be even better to stop the timer entirely...)
    if (! [[NSApplication sharedApplication] isActive])
    {
        [feedbackField setStringValue: @"(the application is not active)"];
        [feedbackField setTextColor: [NSColor darkGrayColor]];

        return;
    }

    //	Get mouse location in window coordinates
    mouseLoc = [[textView window] mouseLocationOutsideOfEventStream];

    //	Convert to view coordinates
    mouseLoc = [textView convertPoint: mouseLoc  fromView: nil];

    //	Note that using [[textView textContainer] containsPoint: mouseLoc] won't work,
    //	because the container for a flexible-height text-view is ÒbottomlessÓ -- it
    //	will consider the mouse to be inside the text even when the mouse is below it.

    //	And... using [textView mouse: mouseLoc  inRect: [textView bounds]] won't work,
    //	because the text view itself can grow below the bottom of the scroll-view. So use
    //	the scroll-view's content view.
    contentView = [[textView enclosingScrollView] contentView];
    mouseLocInContentView = [contentView convertPoint: mouseLoc  fromView: textView];

    if ([contentView mouse: mouseLocInContentView  inRect: [contentView bounds]])
    {
        //	Map the mouse location to the glyph index.
        glyphIndex = [[textView layoutManager] glyphIndexForPoint: mouseLoc  inTextContainer: [textView textContainer]];

        //	Glyphs aren't always the same as characters in a string, so now find the character index in the string.
        charIndex = [[textView layoutManager] characterIndexForGlyphAtIndex: glyphIndex];
    }
    else
    {
        //	Flag that we're not in view
        charIndex = -1;
    }

    //	It's tempting to cache ÒcharIndexÓ here, then compare to the cache to avoid
    //	updating the field. But if the user (for example) bolds the font with the
    //	keyboard, the attributes could change without the character index changing...

    if (charIndex != -1)
    {
        //	They're pointing at some text; get its attributes and show them.
        attributes = [[textView textStorage] attributesAtIndex: charIndex  effectiveRange: NULL];

        if ([feedbackTypeMatrix selectedTag] == linkShowLinkInfo)
        {
            NSObject	*link;

            link = [attributes objectForKey: @"NSLink"];
            if (link == nil)
            {
                [feedbackField setStringValue: @"(the mouse is not over a link)"];
            }
            else
            {
                [feedbackField setStringValue:
                    [NSString stringWithFormat: @"link class = %@\nlink value = %@",
                        [link class], link]];
            }
        }
        else
            [feedbackField setStringValue: [attributes description]];

        [feedbackField setTextColor: [NSColor blackColor]];
    }
    else
    {
        //	Hint that they should move the mouse into the text-view to see something in this field
        [feedbackField setStringValue: @"(point at some text to see its link)"];
        [feedbackField setTextColor: [NSColor redColor]];
    }
}


#pragma mark PUBLIC INSTANCE METHODS -- NSNibAwaking PROTOCOL

- (void) awakeFromNib
{
    //	On startup, begin redisplaying the attributes every 1/10th of a second.
    //	(It'd be smarter to do this only when the mouse enters, moves, or exits,
    //	 but this is just an example app.)
    [[NSTimer scheduledTimerWithTimeInterval: 0.1
        target: self
        selector: @selector(showAttributesUnderMouse)
        userInfo: nil
        repeats: YES] retain];

    //	Fake a selection change so the matrix reflects the starting situation.
    [self textViewDidChangeSelection: nil];
}


#pragma mark PUBLIC INSTANCE METHODS -- NSTextField DELEGATE METHODS

//	If they type into a field, select the relevant radio button for them.
- (void) controlTextDidChange: (NSNotification *) notification
{
    if ([notification object] == createLinkURLField)
        [createLinkTypeMatrix selectCellWithTag: linkToOtherURLTag];

    if ([notification object] == createLinkAlertField)
        [createLinkTypeMatrix selectCellWithTag: linkToAlertTag];
}


#pragma mark PUBLIC INSTANCE METHODS -- NSTextView DELEGATE METHODS

//	Handle a click in a link.
- (BOOL) textView: (NSTextView *) textView
    clickedOnLink: (id) link
    atIndex: (unsigned) charIndex
{
    if ([link isKindOfClass: [NSString class]])
    {
        NSRunAlertPanel (@"Whee!",
                    [NSString stringWithFormat: @"You clicked a link containing the text Ò%@Ó", link],
                    nil, nil, nil);
        return YES;
    }

    //	We don't need to check for a link belonging to the NSURL class;
    //	NSTextView automatically does that.

    //	If we don't recognize the link object, say so.
    return NO;
}

//	When the selection changes in the big text-view,
//	update the title of the relevant radio button.
- (void) textViewDidChangeSelection: (NSNotification *) aNotification
{
    NSString	*selected, *title;
    BOOL		enable;
    NSCell		*matrixCell;

    //	Assume 'aNotification' is for our NSTextView -- it's the only one in the app
    selected = [self selectedText];

    //	Truncate a really long selection, to avoid messing up the matrix display.
    if ([selected length] > 30)
        selected = [[selected substringToIndex: 30] stringByAppendingString: @"É"];

    if ([selected length] == 0)
    {
        title = @"Éto (selected text)";
        enable = NO;
    }
    else
    {
        title = [NSString stringWithFormat: @"Éto \"%@\"", selected];
        enable = YES;
    }

    matrixCell = [createLinkTypeMatrix cellWithTag: linkToURLFromSelectionTag];
    [matrixCell setTitle: title];
    [matrixCell setEnabled: enable];
    //	If we've just disabled the selected cell, we should really push the selection to another cell...
}

//	Optionally return the cursor for a given type of link.
//	This isn't really an NSTextView delegate method, but our NSTextView subclass sends it to the delegate.
- (NSCursor *) cursorForLink: (NSObject *) linkObject
    atIndex: (unsigned) charIndex
    ofTextView: (NSTextView *) aTextView
{
    //	Return something distinctive for alert strings.
    if ([linkObject isKindOfClass: [NSString class]])
        return [NSCursor arrowCursor];

    //	Ignore everything else; the text-view subclass will default.
    return nil;
}


#pragma mark PUBLIC INSTANCE METHODS -- INTERFACE BUILDER ACTIONS

//	Turn the selection in the text-view into a link, optionally blue-and-underlined.
- (IBAction) createLink: (id) sender
{
    NSRange					selection;
    NSObject				*linkObject;
    NSMutableDictionary		*linkAttributes;

    selection = [textView selectedRange];
    if (selection.length == 0)
    {
        NSRunAlertPanel (@"Create Link", @"Please select some text before creating a link.", nil, nil, nil);
        return;
    }

    //	Get the object which will go into the link. If it comes back nil, the user's been alerted.
    linkObject = [self linkObject];
    if (linkObject == nil)
        return;

    //	Start to build attributes
    //		NSLinkAttributeName => the object
    linkAttributes = [NSMutableDictionary dictionaryWithObject: linkObject
        forKey: NSLinkAttributeName];

    //	If the user wants it to look like a traditional link, add these attributes
    //		NSForegroundColorAttributeName => blue
    //		NSUnderlineStyleAttributeName => YES
    if ([blueUnderlineCheckbox state] == NSOnState)
    {
        [linkAttributes setObject: [NSColor blueColor]  forKey: NSForegroundColorAttributeName];
        [linkAttributes setObject: [NSNumber numberWithBool: YES]  forKey: NSUnderlineStyleAttributeName];
    }

    //	Add the attributes. This adds the link (and perhaps color and underline) attributes to the selected range.
    //	(Don't "set" the attributes, which would lose unrelated attributes like the ruler.)
    //	Note that we don't need to follow this with -edited:range:changeInLength:;
    //	that's only for folks who subclass NSTextStorage.
	[[textView textStorage] addAttributes: linkAttributes  range: selection];

    //	Tickle the AppKit into calling our -resetCursorRects implementation.
    //	One would expect [[textView window] invalidateCursorRectsForView: textView]
    //	to work, but it doesn't for us, so just go the direct route...
    [[textView window] resetCursorRects];
}

//	When they click a radio button, select the relevant field (if any) for them.
//	(Would be even better to select only if the field doesn't already have a selection.)
- (IBAction) linkTypeSelected: (id) sender
{
    switch ((linkTag) [createLinkTypeMatrix selectedTag])
    {
        case linkToURLFromSelectionTag:
            break;

        case linkToOtherURLTag:
            [createLinkURLField selectText: nil];
            break;

        case linkToAlertTag:
            [createLinkAlertField selectText: nil];
            break;

        default:						// don't expect this
            break;
    }
}

@end


/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
