/*
        Document.m
        Copyright (c) 1995-2005 by Apple Computer, Inc., all rights reserved.
        Author: Ali Ozer

        Document object for TextEdit.
        Needs to be switched over to the NSDocument object.
*/
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

#import <Cocoa/Cocoa.h>
#import <math.h>
#import <stdio.h>				// for NULL
#import <objc/objc-runtime.h>			// for objc_msgSend
#import "Document.h"
#import "MultiplePageView.h"
#import "Preferences.h"
#import "EncodingManager.h"
#import "ScalingScrollView.h"

// Functions implemented later in this file
static int nextAvailableUntitledDocNumber(void);

static Document *transientDocument = nil;
static NSRect transientDocumentWindowFrame;

// We have disabled use of per-document zones, but have a way to reenable them in case we need to do per-document memory analysis
static BOOL useZones = NO;

static NSZone *zoneForNewDocument(void) {
    return useZones ? NSCreateZone(NSPageSize(), NSPageSize(), YES) : NSDefaultMallocZone();
}


/* A very simple container class which is used to collect the outlets from loading the encoding accessory.  No implementation provided, because all of the references are weak and don't need retain/release.  Would be nice to be able to switch to a mutable dictionary here at some point.
*/
@interface OpenSaveAccessoryOwner : NSObject {
@public
    IBOutlet NSView *accessoryView;
    IBOutlet EncodingPopUpButton *encodingPopUp;
    IBOutlet NSButton *checkBox;
}
@end

@implementation OpenSaveAccessoryOwner
@end


@implementation Document

- (void)setupInitialTextViewSharedState {
    NSTextView *textView = [self firstTextView];
    
    [textView setUsesFontPanel:YES];
    [textView setUsesFindPanel:YES];
    [textView setDelegate:self];
    [textView setAllowsUndo:YES];
    [textView setAllowsDocumentBackgroundColorChange:YES];
    [textView setContinuousSpellCheckingEnabled:[[Preferences objectForKey:CheckSpellingAsYouType] boolValue]];
    [self setRichText:[[Preferences objectForKey:RichText] boolValue] dealWithAttachments:NO showRuler:NO];
    if ([self isRichText]) [self showRulerDelayed:YES];		// Bring up the ruler delayed to speed up launch a bit
    [self setHyphenationFactor:0.0];
}

- (id)init {
    static NSPoint cascadePoint = {0.0, 0.0};
    NSLayoutManager *layoutManager;
    NSZone *zone = [self zone];
        
    self = [super init];
    textStorage = [[NSTextStorage allocWithZone:zone] init];

    if (![NSBundle loadNibNamed:@"DocumentWindow" owner:self])  {
        NSLog(@"Failed to load DocumentWindow.nib");
        [self release];
        return nil;
    }
    
    layoutManager = [[NSLayoutManager allocWithZone:zone] init];
    [textStorage addLayoutManager:layoutManager];
    [layoutManager setDelegate:self];
    [layoutManager release];
    
    [self setEncoding:UnknownStringEncoding];

    // This gives us our first view
    [self setHasMultiplePages:[[Preferences objectForKey:ShowPageBreaks] boolValue] force:YES];

    // This ensures the first view gets set up correctly
    [self setupInitialTextViewSharedState];

    [[self window] setDelegate:self];

    // Set the window size from defaults...
    if ([self hasMultiplePages]) {
        [self setViewSize:[[scrollView documentView] pageRectForPageNumber:0].size];
    } else {
        int windowHeight = [[Preferences objectForKey:WindowHeight] intValue];
        int windowWidth = [[Preferences objectForKey:WindowWidth] intValue];
        NSFont *font = [Preferences objectForKey:[self isRichText] ? RichTextFont : PlainTextFont];
        NSSize size;
        size.height = ceil([layoutManager defaultLineHeightForFont:font] * windowHeight);
	size.width = [font widthOfString:@"x"]; /* will be 0 if can't be rendered */
	if (size.width == 0.0) size.width = [font widthOfString:@" "]; /* try for space width */
	if (size.width == 0.0) size.width = [font maximumAdvancement].width; /* or max width */
        size.width  = ceil(size.width * windowWidth);
        [self setViewSize:size];
    }

    if (NSEqualPoints(cascadePoint, NSZeroPoint)) {	/* First time through... */
        NSRect frame = [[self window] frame];
	cascadePoint = NSMakePoint(frame.origin.x, NSMaxY(frame));
    }
    cascadePoint = [[self window] cascadeTopLeftFromPoint:cascadePoint];

    // Register for appropriate notifications from this window's undo manager to control the state of the close box 
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(undoManagerChangeUndone:) name:NSUndoManagerDidUndoChangeNotification object:[self undoManager]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(undoManagerChangeDone:) name:NSUndoManagerDidRedoChangeNotification object:[self undoManager]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(undoManagerChangeDone:) name:NSUndoManagerWillCloseUndoGroupNotification object:[self undoManager]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(undoManagerCheckpoint:) name:NSUndoManagerCheckpointNotification object:[self undoManager]];

    return self;
}

- (id)initWithPath:(NSString *)filename encoding:(unsigned)encoding ignoreRTF:(BOOL)ignoreRTF ignoreHTML:(BOOL)ignoreHTML uniqueZone:(BOOL)flag error:(NSError **)errorPtr {
    if (!(self = [self init])) {
        if (flag) NSRecycleZone([self zone]);
	// Return a generic read error; this is a very unlikely error case
	if (errorPtr) *errorPtr = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileReadUnknownError userInfo:[NSDictionary dictionaryWithObjectsAndKeys:filename, NSFilePathErrorKey, nil]];
        return nil;
    }
    uniqueZone = flag;	/* So if something goes wrong we can recycle the zone correctly in dealloc */

    [self setDocumentName:filename];

    if (filename) {
	if (![self loadFromPath:filename encoding:encoding ignoreRTF:ignoreRTF ignoreHTML:ignoreHTML error:errorPtr]) {
            // Cancel showing the ruler - this removes the instance from the runloop and allows it to be released sooner
            [self showRulerDelayed:NO];	
            [self release];
            return nil;
        }
        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:documentName]];
        openedIgnoringRTF = ignoreRTF;
        openedIgnoringHTML = ignoreHTML;
        if ([self isRichText]) {
            if ([self isReadOnly]) {
                [self showRulerDelayed:NO];	// Cancel the ruler; no need to show a ruler for readonly docs
            } else {
                [self showRuler:nil];		// Show the ruler immediately (doing it delayed causes a glitch...)
            }
        }
    }
    [[self firstTextView] setSelectedRange:NSMakeRange(0, 0)];
    
    [self setDocumentEdited:NO];
    [[self undoManager] removeAllActions];

    return self;
}

/* Calls the above routine with the default values for ignoreRTF and ignoreHTML
*/
- (id)initWithPath:(NSString *)filename encoding:(unsigned)encoding uniqueZone:(BOOL)flag error:(NSError **)errorPtr {
    BOOL ignoreRTF = [[Preferences objectForKey:IgnoreRichText] boolValue];
    BOOL ignoreHTML = [[Preferences objectForKey:IgnoreHTML] boolValue];
    return [self initWithPath:filename encoding:encoding ignoreRTF:ignoreRTF ignoreHTML:ignoreHTML uniqueZone:flag error:errorPtr];
}

/* Opens a new, untitled document. The argument specifies whether the document is created automatically by the system (as opposed to by the user).
*/
+ (id)openUntitled:(BOOL)isOpenedAutomatically {
    Document *document = [[self allocWithZone:zoneForNewDocument()] initWithPath:nil encoding:UnknownStringEncoding uniqueZone:useZones error:NULL];
    if (document) {
        transientDocument = nil;	// Opening a new untitled does not close previous...
	[document setDocumentName:nil];
        [document showWindowBehindDocument:nil];
        if (isOpenedAutomatically) {
            transientDocument = document;
            transientDocumentWindowFrame = [[document window] frame];
        }
        return document;
    } else {
        return nil;
    }
}

+ (id)openDocumentWithPath:(NSString *)filename encoding:(unsigned)encoding ignoreRTF:(BOOL)ignoreRTF ignoreHTML:(BOOL)ignoreHTML behind:(Document *)otherDoc error:(NSError **)errorPtr {
    Document *document = [self documentForPath:filename];
    if (document && [document isEditedExternally:nil]) {	// If the document is already open, but has been edited externally
        if ([document isDocumentEdited]) {		//  and has also been edited in TextEdit, do the conservative thing and don't open
            NSBeginAlertSheet(NSLocalizedString(@"Couldn\\U2019t reopen file", @"Title of alert indicating file couldn't be reloaded"),
                NSLocalizedString(@"OK", @"OK"), 
                nil, nil, [document window], self, NULL, NULL, document, 
                NSLocalizedString(@"The file you are trying to open, \\U201c%@\\U201d, is already open, has unsaved changes, and has been changed by another application. Because opening the file would cause you to lose your changes, request to open the file has been ignored.", @"Message indicating document couldn't be opened because doing so would cause changes to be lost."),
                    displayName([document documentName]));
            return document;
        } else {	// If the document in TextEdit doesn't have any edits, then simply do a revert
            [document doRevert];
        }
    }
    if (!document) {
        document = [[self allocWithZone:zoneForNewDocument()] initWithPath:filename encoding:encoding ignoreRTF:ignoreRTF ignoreHTML:ignoreHTML uniqueZone:useZones error:errorPtr];
	if (!document) return nil;
    }
    [document showWindowBehindDocument:otherDoc];
    // Do foreground layout only for the front-most document; others can be laid out in the background
    if (!otherDoc) [document doForegroundLayoutToCharacterIndex:[[Preferences objectForKey:ForegroundLayoutToIndex] intValue]];
    return document;
}

+ (id)openDocumentWithPath:(NSString *)filename encoding:(unsigned)encoding behind:(Document *)otherDoc error:(NSError **)errorPtr {
    BOOL ignoreRTF = [[Preferences objectForKey:IgnoreRichText] boolValue];
    BOOL ignoreHTML = [[Preferences objectForKey:IgnoreHTML] boolValue];
    return [self openDocumentWithPath:filename encoding:encoding ignoreRTF:ignoreRTF ignoreHTML:ignoreHTML behind:otherDoc error:errorPtr];
}

/* Clear the delegates of the text views and window, then release all resources and go away...
*/
- (void)dealloc {
    // Remove this document from the notification center as the observer of any notifications...
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    if (rulerIsBeingDisplayed) [self showRulerDelayed:NO];	// This cancels outstanding request to show ruler...
    [[self firstTextView] setDelegate:nil];
    [[self layoutManager] setDelegate:nil];
    [[self window] setDelegate:nil];
    [richTextDocumentFormatAccessory release];
    [documentName release];
    [revertDocumentName release];
    [fileModDate release];
    [textStorage release];
    [printInfo release];
    [author release];
    [comment release];
    [subject release];
    [title release];
    [keywords release];
    [copyright release];
    [lastChangedDocumentProperty release];

    if (uniqueZone) NSRecycleZone([self zone]);
    if (transientDocument == self) transientDocument = nil;

    [super dealloc];
}

- (NSDictionary *)defaultTextAttributes:(BOOL)forRichText {
    static NSParagraphStyle *defaultRichParaStyle = nil;
    NSMutableDictionary *textAttributes = [[[NSMutableDictionary alloc] initWithCapacity:2] autorelease];
    if (isRichText) {
	[textAttributes setObject:[Preferences objectForKey:RichTextFont] forKey:NSFontAttributeName];
	if (defaultRichParaStyle == nil) {	// We do this once...
	    int cnt;
            NSString *measurementUnits = [[NSUserDefaults standardUserDefaults] objectForKey:@"AppleMeasurementUnits"];
            float tabInterval = ([@"Centimeters" isEqual:measurementUnits]) ? (72.0 / 2.54) : (72.0 / 2.0);  // Every cm or half inch
	    NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
	    [paraStyle setTabStops:[NSArray array]];	// This first clears all tab stops
	    for (cnt = 0; cnt < 12; cnt++) {	// Add 12 tab stops, at desired intervals...
                NSTextTab *tabStop = [[NSTextTab alloc] initWithType:NSLeftTabStopType location:tabInterval * (cnt + 1)]; 
		[paraStyle addTabStop:tabStop];
	 	[tabStop release];
	    }
	    defaultRichParaStyle = [paraStyle copy];
	    [paraStyle release];
	}
	[textAttributes setObject:defaultRichParaStyle forKey:NSParagraphStyleAttributeName];
    } else {
	NSFont *plainFont = [Preferences objectForKey:PlainTextFont];
	unsigned tabWidth = [[Preferences objectForKey:TabWidth] intValue];
	float charWidth;
	if ([plainFont glyphIsEncoded:(NSGlyph)' ']) {
	    charWidth = [[plainFont screenFontWithRenderingMode:NSFontDefaultRenderingMode] advancementForGlyph:(NSGlyph)' '].width;
	} else {
	    charWidth = [[plainFont screenFontWithRenderingMode:NSFontDefaultRenderingMode] maximumAdvancement].width;
	}

	// Now use a default paragraph style, but with the tab width adjusted
	NSMutableParagraphStyle *mStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
	[mStyle setTabStops:[NSArray array]];
	[mStyle setDefaultTabInterval:(charWidth * tabWidth)];
	NSParagraphStyle *style = [mStyle copy];
        [textAttributes setObject:style forKey:NSParagraphStyleAttributeName];
	[mStyle release];
	[style release];

	// Also set the font
	[textAttributes setObject:plainFont forKey:NSFontAttributeName];
    }
    return textAttributes;
}

- (NSTextView *)firstTextView {
    return [[self layoutManager] firstTextView];
}

/* We take "viewSize" to mean the pure size of the text area, so margins or line fragment paddings don't count. The ruler ends up being included though.
*/
- (NSSize)viewSize {
    NSSize size = [NSScrollView contentSizeForFrameSize:[scrollView frame].size hasHorizontalScroller:[scrollView hasHorizontalScroller] hasVerticalScroller:[scrollView hasVerticalScroller] borderType:[scrollView borderType]];
    if (![self hasMultiplePages]) {
        size.width -= (defaultTextPadding() * 2.0);
    } 
    return size;
}

- (void)setViewSize:(NSSize)size {
    NSWindow *window = [scrollView window];
    NSRect origWindowFrame = [window frame];
    NSSize scrollViewSize;
    if (![self hasMultiplePages]) {
        size.width += (defaultTextPadding() * 2.0);
    }
    scrollViewSize = [NSScrollView frameSizeForContentSize:size hasHorizontalScroller:[scrollView hasHorizontalScroller] hasVerticalScroller:[scrollView hasVerticalScroller] borderType:[scrollView borderType]];
    [window setContentSize:scrollViewSize];
    [window setFrameTopLeftPoint:NSMakePoint(origWindowFrame.origin.x, NSMaxY(origWindowFrame))];
}

/* Show the window for the document, but also replace the transient untitled document if there's one. If otherDoc is provided, bring up document behind this other document. Otherwise come to front, as key.
*/
- (void)showWindowBehindDocument:(Document *)otherDoc {
    BOOL closeTransient = transientDocument && NSEqualRects(transientDocumentWindowFrame, [[transientDocument window] frame]);
    if (closeTransient) {
        [[self window] setFrameTopLeftPoint:NSMakePoint(transientDocumentWindowFrame.origin.x, NSMaxY(transientDocumentWindowFrame))];
    }
    int otherWindowNumber = otherDoc ? [[otherDoc window] windowNumber] : 0;
    if (otherWindowNumber) {
	[[self window] orderWindow:NSWindowBelow relativeTo:otherWindowNumber];
    } else {
	[[self window] makeKeyAndOrderFront:nil];
    }
    if (closeTransient) {	// Should be ready to close, unedited
        [transientDocument close:nil];
    }
}

/* This method causes the text to be laid out in the foreground (approximately) up to the indicated character index.
*/
- (void)doForegroundLayoutToCharacterIndex:(unsigned)loc {
    unsigned len;
    if (loc > 0 && (len = [[self textStorage] length]) > 0) {
        NSRange glyphRange;
        if (loc >= len) loc = len - 1;
        /* Find out which glyph index the desired character index corresponds to */
        glyphRange = [[self layoutManager] glyphRangeForCharacterRange:NSMakeRange(loc, 1) actualCharacterRange:NULL];
        if (glyphRange.location > 0) {
            /* Now cause layout by asking a question which has to determine where the glyph is */
            (void)[[self layoutManager] textContainerForGlyphAtIndex:glyphRange.location - 1 effectiveRange:NULL];
        }
    }
}

+ (NSString *)cleanedUpPath:(NSString *)filename {
    NSString *resolvedSymlinks = [filename stringByResolvingSymlinksInPath];
    if ([resolvedSymlinks length] > 0) {
        NSString *standardized = [resolvedSymlinks stringByStandardizingPath];
        return [standardized length] ? standardized : resolvedSymlinks;
    }
    return filename;
}

// newModDateIfKnown is optional (as a perf optimization); if not available, obtain it for the existing document name 
//
- (BOOL)isEditedExternally:(NSDate *)newModDateIfKnown {
    NSDate *curModDate = [self fileModDate];
    if (!curModDate) return NO;	// Not much we can do if we don't know anything about the file from before
    if (!newModDateIfKnown && !(newModDateIfKnown = [[[NSFileManager defaultManager] fileAttributesAtPath:[self documentName] traverseLink:YES] fileModificationDate])) return NO;
    return [curModDate isEqual:newModDateIfKnown] ? NO : YES;
}

- (void)setFileModDate:(NSDate *)date {
    if (date != fileModDate) {
        [fileModDate release];
        fileModDate = [date copy];
    }
}

- (NSDate *)fileModDate {
    return [[fileModDate retain] autorelease];
}

- (void)setDocumentName:(NSString *)filename updateIcon:(BOOL)updateIcon {
    if (filename) {
        BOOL same = [filename isEqual:documentName];
        [documentName autorelease];
	[revertDocumentName autorelease];	// Name the document will use to revert to (having a separate ivar allows docs whose formats have changed to revert)
        documentName = [[[self class] cleanedUpPath:filename] copyWithZone:[self zone]];
        revertDocumentName = [documentName copyWithZone:[self zone]];
        if (same && updateIcon) [[self window] setTitleWithRepresentedFilename:@""];	// Workaround NSWindow optimization
        [[self window] setTitleWithRepresentedFilename:documentName];
        if (uniqueZone) NSSetZoneName([self zone], documentName);
    } else {
	NSString *untitled = [self untitledDocumentName:UnknownStringEncoding];
	[[self window] setTitle:untitled];
        if (uniqueZone) NSSetZoneName([self zone], untitled);
        documentName = nil;
    }
}

- (void)setDocumentName:(NSString *)filename {
    [self setDocumentName:filename updateIcon:NO];
}

- (NSString *)documentName {
    return documentName;
}

// Note that because setDocumentEdited: is called from undo notifications, undoable actions do not need to concern themselves with calling this.
//
- (void)setDocumentEdited:(BOOL)flag {
    if (flag != isDocumentEdited) {
        isDocumentEdited = flag;
        [[self window] setDocumentEdited:isDocumentEdited];
        if (transientDocument == self) transientDocument = nil;
    }
    if (!isDocumentEdited) changeCount = 0;
}

- (BOOL)isDocumentEdited {
    return isDocumentEdited;
}

- (void)setReadOnly:(BOOL)flag {
    isReadOnly = flag;
    [[self firstTextView] setEditable:!isReadOnly];
}

- (BOOL)isReadOnly {
    return isReadOnly;
}

- (void)setBackgroundColor:(NSColor *)color {
    [[self firstTextView] setBackgroundColor:color];
}

- (NSColor *)backgroundColor {
    return [[self firstTextView] backgroundColor];
}


- (NSTextStorage *)textStorage {
    return textStorage;
}

- (NSWindow *)window {
    return [[self firstTextView] window];
}

- (BOOL)hasSheet {
    return [[self window] attachedSheet] ? YES : NO;
}

- (NSUndoManager *)undoManager {
    return [[self window] undoManager];
}

- (NSLayoutManager *)layoutManager {
    return [[textStorage layoutManagers] objectAtIndex:0];
}

- (void)printInfoUpdated {
    if ([self hasMultiplePages]) {
        unsigned cnt, numberOfPages = [self numberOfPages];
        MultiplePageView *pagesView = [scrollView documentView];
        NSArray *textContainers = [[self layoutManager] textContainers];

        [pagesView setPrintInfo:printInfo];
        
        for (cnt = 0; cnt < numberOfPages; cnt++) {
            NSRect textFrame = [pagesView documentRectForPageNumber:cnt];
            NSTextContainer *textContainer = [textContainers objectAtIndex:cnt];
            [textContainer setContainerSize:textFrame.size];
            [[textContainer textView] setFrame:textFrame];
        }
    }
}

- (void)setPrintInfo:(NSPrintInfo *)anObject {
    if (printInfo == anObject) return;

    [printInfo autorelease];
    printInfo = [anObject copyWithZone:[self zone]];
    [self printInfoUpdated];
}

// Create and return the printInfo lazily

- (NSPrintInfo *)printInfo {
    if (printInfo == nil) {
        [self setPrintInfo:[NSPrintInfo sharedPrintInfo]];
        [printInfo setHorizontalPagination:NSFitPagination];
        [printInfo setHorizontallyCentered:NO];
        [printInfo setVerticallyCentered:NO];
        [printInfo setLeftMargin:72.0];
        [printInfo setRightMargin:72.0];
        [printInfo setTopMargin:72.0];
        [printInfo setBottomMargin:72.0];
    }
    return printInfo;
}

- (NSSize)paperSize {
    return [[self printInfo] paperSize];
}

- (void)setPaperSize:(NSSize)size {
    [[self printInfo] setPaperSize:size];
    [self printInfoUpdated];
}

/* Multiple page related code */

- (unsigned)numberOfPages {
    return hasMultiplePages ? [[scrollView documentView] numberOfPages] : 1;
}

- (BOOL)hasMultiplePages {
    return hasMultiplePages;
}

- (void)addPage {
    NSZone *zone = [self zone];
    unsigned numberOfPages = [self numberOfPages];
    MultiplePageView *pagesView = [scrollView documentView];
    NSSize textSize = [pagesView documentSizeInPage];
    NSTextContainer *textContainer = [[NSTextContainer allocWithZone:zone] initWithContainerSize:textSize];
    NSTextView *textView;
    [pagesView setNumberOfPages:numberOfPages + 1];
    textView = [[NSTextView allocWithZone:zone] initWithFrame:[pagesView documentRectForPageNumber:numberOfPages] textContainer:textContainer];
    [textView setHorizontallyResizable:NO];
    [textView setVerticallyResizable:NO];
    [pagesView addSubview:textView];
    [[self layoutManager] addTextContainer:textContainer];
    [textView release];
    [textContainer release];
}

- (void)removePage {
    unsigned numberOfPages = [self numberOfPages];
    NSArray *textContainers = [[self layoutManager] textContainers];
    NSTextContainer *lastContainer = [textContainers objectAtIndex:[textContainers count] - 1];
    MultiplePageView *pagesView = [scrollView documentView];
    [pagesView setNumberOfPages:numberOfPages - 1];
    [[lastContainer textView] removeFromSuperview];
    [[lastContainer layoutManager] removeTextContainerAtIndex:[textContainers count] - 1];
}

/* This method sets whether the document has multiple pages or not. It can be called at anytime. force indicates whether to do the task even if the values are the same (usually needed for the first call).
*/   
- (void)setHasMultiplePages:(BOOL)flag force:(BOOL)force {
    NSZone *zone = [self zone];

    if (!force && (hasMultiplePages == flag)) return;
    
    hasMultiplePages = flag;

    if (hasMultiplePages) {
        NSTextView *textView = [self firstTextView];
        MultiplePageView *pagesView = [[MultiplePageView allocWithZone:zone] init];

        [scrollView setDocumentView:pagesView];

        [pagesView setPrintInfo:[self printInfo]];
        // Add the first new page before we remove the old container so we can avoid losing all the shared text view state.
        [self addPage];
        if (textView) {
            [[self layoutManager] removeTextContainerAtIndex:0];
        }
        [scrollView setHasHorizontalScroller:YES];

        // Make sure the selected text is shown
        [[self firstTextView] scrollRangeToVisible:[[self firstTextView] selectedRange]];

        NSRect visRect = [pagesView visibleRect];
	NSRect pageRect = [pagesView pageRectForPageNumber:0];
        if (visRect.size.width < pageRect.size.width) {	// If we can't show the whole page, tweak a little further
            NSRect docRect = [pagesView documentRectForPageNumber:0];
            if (visRect.size.width >= docRect.size.width) {	// Center document area in window
                visRect.origin.x = docRect.origin.x - floor((visRect.size.width - docRect.size.width) / 2);
                if (visRect.origin.x < pageRect.origin.x) visRect.origin.x = pageRect.origin.x;
            } else {	// If we can't show the document area, then show left edge of document area (w/out margins)
                visRect.origin.x = docRect.origin.x;
            }
            [pagesView scrollRectToVisible:visRect];
        }
    } else {
        NSSize size = [scrollView contentSize];
        NSTextContainer *textContainer = [[NSTextContainer allocWithZone:zone] initWithContainerSize:NSMakeSize(size.width, FLT_MAX)];
        NSTextView *textView = [[NSTextView allocWithZone:zone] initWithFrame:NSMakeRect(0.0, 0.0, size.width, size.height) textContainer:textContainer];

        // Insert the single container as the first container in the layout manager before removing the existing pages in order to preserve the shared view state.
        [[self layoutManager] insertTextContainer:textContainer atIndex:0];

        if ([[scrollView documentView] isKindOfClass:[MultiplePageView class]]) {
            NSArray *textContainers = [[self layoutManager] textContainers];
            unsigned cnt = [textContainers count];
            while (cnt-- > 1) {
                [[self layoutManager] removeTextContainerAtIndex:cnt];
            }
        }

        [textContainer setWidthTracksTextView:YES];
        [textContainer setHeightTracksTextView:NO];		/* Not really necessary */
        [textView setHorizontallyResizable:NO];			/* Not really necessary */
        [textView setVerticallyResizable:YES];
	[textView setAutoresizingMask:NSViewWidthSizable];
        [textView setMinSize:size];	/* Not really necessary; will be adjusted by the autoresizing... */
        [textView setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];	/* Will be adjusted by the autoresizing... */  

        /* The next line should cause the multiple page view and everything else to go away */
        [scrollView setDocumentView:textView];
        [scrollView setHasHorizontalScroller:NO];
        
        [textView release];
        [textContainer release];

        // Show the selected region
        [[self firstTextView] scrollRangeToVisible:[[self firstTextView] selectedRange]];
    }
    [[scrollView window] makeFirstResponder:[self firstTextView]];
    [[scrollView window] setInitialFirstResponder:[self firstTextView]];	// So focus won't be stolen (2934918)
}

- (void)setHasMultiplePages:(BOOL)flag {
    [self setHasMultiplePages:flag force:NO];
}

/* Used when converting to plain text
*/
- (void)removeAttachments {
    NSTextStorage *attrString = [self textStorage];
    NSTextView *view = [self firstTextView];
    unsigned loc = 0;
    unsigned end = [attrString length];
    [attrString beginEditing];
    while (loc < end) {	/* Run through the string in terms of attachment runs */
        NSRange attachmentRange;	/* Attachment attribute run */
        NSTextAttachment *attachment = [attrString attribute:NSAttachmentAttributeName atIndex:loc longestEffectiveRange:&attachmentRange inRange:NSMakeRange(loc, end-loc)];
        if (attachment != nil) {	/* If there is an attachment, make sure it is valid */
            unichar ch = [[attrString string] characterAtIndex:loc];
            if (ch == NSAttachmentCharacter) {
		if ([view shouldChangeTextInRange:NSMakeRange(loc, 1) replacementString:@""]) {
		    [attrString replaceCharactersInRange:NSMakeRange(loc, 1) withString:@""];
		    [view didChangeText];
		}
                end = [attrString length];	/* New length */
            } else {
                loc++;	/* Just skip over the current character... */
            }
        } else {
            loc = NSMaxRange(attachmentRange);
        }
    }
    [attrString endEditing];
}

/* Hyphenation related methods.
*/
- (void)setHyphenationFactor:(float)factor {
    [[self layoutManager] setHyphenationFactor:factor];
}

- (float)hyphenationFactor {
    return [[self layoutManager] hyphenationFactor];
}

/* Encoding...
*/
- (unsigned)encoding {
    return documentEncoding;
}

- (void)setEncoding:(unsigned)encoding {
    documentEncoding = encoding;
}

- (BOOL)converted {
    return convertedDocument;
}

- (void)setConverted:(BOOL)flag {
    convertedDocument = flag;
}

- (BOOL)lossy {
    return lossyDocument;
}

- (void)setLossy:(BOOL)flag {
    lossyDocument = flag;
}

/* Method to lazily display ruler. Call with YES to display, NO to cancel display; this method doesn't remove the ruler. 
*/
- (void)showRulerDelayed:(BOOL)flag {
    if (!flag && rulerIsBeingDisplayed) {
        [[self class] cancelPreviousPerformRequestsWithTarget:self selector:@selector(showRuler:) object:self];
    } else if (flag && !rulerIsBeingDisplayed) {
        [self performSelector:@selector(showRuler:) withObject:self afterDelay:0.0];
    }
    rulerIsBeingDisplayed = flag;
}

- (void)showRuler:(id)obj {
    if (rulerIsBeingDisplayed && !obj) [self showRulerDelayed:NO];	// Cancel outstanding request, if not coming from the delayed request
    if ([[Preferences objectForKey:ShowRuler] boolValue]) [[self firstTextView] setRulerVisible:YES];
}

/* Doesn't check to see if the prev value is the same --- Otherwise the first time doesn't work...
   attachmentFlag allows for optimizing some cases where we know we have no attachments, so we don't need to scan looking for them.
*/
- (void)setRichText:(BOOL)flag dealWithAttachments:(BOOL)attachmentFlag showRuler:(BOOL)rulerFlag {
    NSTextView *view = [self firstTextView];
    NSDictionary *textAttributes;
    NSParagraphStyle *paragraphStyle;

    isRichText = flag;
    
    [view setRichText:isRichText];
    [view setUsesRuler:isRichText];	// If NO, this correctly gets rid of the ruler if it was up
    if (!isRichText && rulerIsBeingDisplayed) [self showRulerDelayed:NO];	// Cancel delayed ruler request
    if (isRichText && rulerFlag) [self showRuler:nil];
    [view setImportsGraphics:isRichText];

    textAttributes = [self defaultTextAttributes:isRichText];
    paragraphStyle = [textAttributes objectForKey:NSParagraphStyleAttributeName];

    // Note, since the textview content changes (removing attachments and changing attributes) create undo actions inside the textview, we do not execute them here if we're undoing or redoing
    if (![[self undoManager] isUndoing] && ![[self undoManager] isRedoing]) {
	if (!isRichText && attachmentFlag) [self removeAttachments];
	NSRange range = NSMakeRange(0, [textStorage length]);
        if ([view shouldChangeTextInRange:range replacementString:nil]) {
	    [textStorage setAttributes:textAttributes range: range];
	    [view didChangeText];
	}
    }
    [view setTypingAttributes:textAttributes];
    [view setDefaultParagraphStyle:paragraphStyle];
    
    if (!isRichText) {
	[self clearDocumentProperties];
    } else {
	[self setDocumentPropertiesToDefaults];
    }
}

- (void)setRichText:(BOOL)flag {
    [self setRichText:flag dealWithAttachments:YES showRuler:YES];
}

- (BOOL)isRichText {
    return isRichText;
}


/* Document properties management */

/* Table mapping document property keys "company", etc, to text system document attribute keys (NSCompanyDocumentAttribute, etc)
*/
- (NSDictionary *)documentPropertyToAttributeNameMappings {
    static NSDictionary *dict = nil;
    if (!dict) dict = [[NSDictionary alloc] initWithObjectsAndKeys:
	NSCompanyDocumentAttribute, @"company", 
	NSAuthorDocumentAttribute, @"author", 
	NSKeywordsDocumentAttribute, @"keywords", 
  	NSCopyrightDocumentAttribute, @"copyright", 
	NSTitleDocumentAttribute, @"title", 
	NSSubjectDocumentAttribute, @"subject", 
	NSCommentDocumentAttribute, @"comment", nil];
    return dict;
}

- (NSArray *)knownDocumentProperties {
    return [[self documentPropertyToAttributeNameMappings] allKeys];
}

/* If there are document properties and they are not the same as the defaults established in preferences, return YES
*/
- (BOOL)hasDocumentProperties {
    NSEnumerator *keyEnumerator = [[self knownDocumentProperties] objectEnumerator];
    NSString *key;
    while (key = [keyEnumerator nextObject]) {
	id value = [self valueForKey:key];
	if (value && ![value isEqual:[Preferences objectForKey:key]]) return YES;
    }
    return NO;
}

/* This actually clears all properties (rather than setting them to default values established in preferences)
*/
- (void)clearDocumentProperties {
    NSEnumerator *keyEnumerator = [[self knownDocumentProperties] objectEnumerator];
    NSString *key;
    while (key = [keyEnumerator nextObject]) [self setValue:nil forKey:key];
}

/* This sets document properties to values established in defaults
*/
- (void)setDocumentPropertiesToDefaults {
    NSEnumerator *keyEnumerator = [[self knownDocumentProperties] objectEnumerator];
    NSString *key;
    while (key = [keyEnumerator nextObject]) [self setValue:[Preferences objectForKey:key] forKey:key];
}

/* We implement a setValue:forDocumentProperty: to work around NSUndoManager bug where prepareWithInvocationTarget: fails to freeze-dry invocations with "known" methods such as setValue:forKey:.  
*/
- (void)setValue:(id)value forDocumentProperty:(NSString *)property {
    id oldValue = [self valueForKey:property];
    if (value == oldValue || [value isEqual:oldValue]) return;	// Workaround for bug 3752890 "Becoming non-editable causes textfield value to be committed instead of discarded"
    // We want to do coalesced undo of changes to a single field. Although this would normally be done by undo grouping, another way to approach is to avoid registering unnecessary undo events altogether. We do that here, by checking the "checkpoint count" (which serves as generation count) before registering.
    if ((lastDocumentPropertyChangeCheckpointCount != [self undoCheckpointCount]) || ![lastChangedDocumentProperty isEqual:property]) {
	lastDocumentPropertyChangeCheckpointCount = [self undoCheckpointCount] + 1;
	[lastChangedDocumentProperty release];
	lastChangedDocumentProperty = [property copy];
	[[[self undoManager] prepareWithInvocationTarget:self] setValue:oldValue forDocumentProperty:property];
	[[self undoManager] setActionName:NSLocalizedString(property, "")];	// Potential strings for action names are listed below (for genstrings to pick up)
    }
    // Then we call the regular KVC mechanism to get the value to be properly set
    [super setValue:value forKey:property];
}

- (void)setValue:(id)value forKey:(NSString *)key {
    if ([[self knownDocumentProperties] containsObject:key]) { 
	[self setValue:value forDocumentProperty:key];	// We take a side-trip to this method to register for undo
    } else {
	[super setValue:value forKey:key];  // In case some other KVC call is sent to Document, we treat it normally
    }
}

/* For genstrings:
    NSLocalizedStringWithDefaultValue(@"author", @"", @"", @"Change Author", @"Undo menu change string, without the 'Undo'");
    NSLocalizedStringWithDefaultValue(@"copyright", @"", @"", @"Change Copyright", @"Undo menu change string, without the 'Undo'");
    NSLocalizedStringWithDefaultValue(@"subject", @"", @"", @"Change Subject", @"Undo menu change string, without the 'Undo'");
    NSLocalizedStringWithDefaultValue(@"title", @"", @"", @"Change Title", @"Undo menu change string, without the 'Undo'");
    NSLocalizedStringWithDefaultValue(@"company", @"", @"", @"Change Company", @"Undo menu change string, without the 'Undo'");
    NSLocalizedStringWithDefaultValue(@"comment", @"", @"", @"Change Comment", @"Undo menu change string, without the 'Undo'");
    NSLocalizedStringWithDefaultValue(@"keywords", @"", @"", @"Change Keywords", @"Undo menu change string, without the 'Undo'");
*/


/* Outlet methods */

- (void)setScrollView:(id)anObject {
    scrollView = anObject;
    [scrollView setHasVerticalScroller:YES];
    [scrollView setHasHorizontalScroller:NO];
    [[scrollView contentView] setAutoresizesSubviews:YES];
    [[scrollView contentView] setBackgroundColor:[NSColor controlColor]];
    if (NSInterfaceStyleForKey(NSInterfaceStyleDefault, scrollView) == NSWindows95InterfaceStyle) {
        [scrollView setBorderType:NSBezelBorder];
    }
}


/* User commands... */

- (BOOL)printDocumentModally:(BOOL)modalFlag {
    BOOL success = YES;
    NSPrintInfo *tempPrintInfo = [self printInfo];
    if ([[Preferences objectForKey:NumberPagesWhenPrinting] boolValue]) {
	tempPrintInfo = [[tempPrintInfo copy] autorelease];
	[[tempPrintInfo dictionary] setValue:[NSNumber numberWithBool:YES] forKey:NSPrintHeaderAndFooter];
    }
    NSPrintOperation *op = [NSPrintOperation printOperationWithView:[scrollView documentView] printInfo:tempPrintInfo];
    [op setShowPanels:YES];
    [self doForegroundLayoutToCharacterIndex:INT_MAX];	// Make sure the whole document is laid out before printing
    if (modalFlag) {
        success = [op runOperation];
    } else {
        [op runOperationModalForWindow:[self window] delegate:nil didRunSelector:NULL contextInfo:NULL];
    }
    // The return value is interesting only in the modal case, where cancelling or failure from one printing should cancel the rest.
    return success;
}

- (void)printDocument:(id)sender {
    [self printDocumentModally:NO];
}

/* Toggles read-only state of the document
*/
- (void)toggleReadOnly:(id)sender {
    [[self undoManager] registerUndoWithTarget:self selector:@selector(toggleReadOnly:) object:nil];
    [[self undoManager] setActionName:[self isReadOnly] ?
        NSLocalizedString(@"Allow Editing", @"Menu item to make the current document editable (not read-only)") :
        NSLocalizedString(@"Prevent Editing", @"Menu item to make the current document read-only")];
    [self setReadOnly:![self isReadOnly]];
}

/* This is an undoable entry point for switching a document between rich and plain states.
*/
- (void)setDocumentState:(BOOL)rich name:(NSString *)name encoding:(unsigned)enc converted:(BOOL)conv {
    [[[self undoManager] prepareWithInvocationTarget:self] setDocumentState:isRichText name:documentName encoding:documentEncoding converted:convertedDocument];
    [self setRichText:rich];
    [self setEncoding:enc];
    [self setConverted:conv];
    [self setDocumentName:name];

    // We would normally provide a localized undo action name here, but since this change is coming after "localization freeze," we are left with a string which we don't need to localize
    [[self undoManager] setActionName:@""];
}

/* doToggleRich, called from toggleRich: or the endToggleRichSheet:... alert panel method, toggles the isRichText state
*/
- (void)doToggleRich {
    [self setDocumentState:!isRichText name:nil encoding:UnknownStringEncoding converted:NO];
}

/* toggleRich: puts up an alert before ultimately calling doToggleRich
*/
- (void)toggleRich:(id)sender {
    int length = [textStorage length];
    NSRange range;
    NSDictionary *attrs;
    // If we are rich and any of the text attrs have been changed from the default, or there are any doc properties, put up an alert first...
    if (isRichText && ((length > 0) && (attrs = [textStorage attributesAtIndex:0 effectiveRange:&range]) && ((attrs == nil) || (range.length < length) || ![[self defaultTextAttributes:YES] isEqual:attrs])) || [self hasDocumentProperties]) {
        NSBeginAlertSheet(NSLocalizedString(@"Convert this document to plain text?", @"Title of alert confirming Make Plain Text"),
                        NSLocalizedString(@"OK", @"OK"), NSLocalizedString(@"Cancel", @"Button choice allowing user to cancel."), nil, [self window], 
                        self, NULL, @selector(didEndToggleRichSheet:returnCode:contextInfo:), NULL,
                        NSLocalizedString(@"If you convert this document, you will lose all text styles (such as fonts and colors) and document properties.", @"Subtitle of alert confirming Make Plain Text"));
    } else {
        [self doToggleRich];
    }
}

- (void)didEndToggleRichSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    if (returnCode == NSAlertDefaultReturn) [self doToggleRich];
}


- (void)togglePageBreaks:(id)sender {
    [self setHasMultiplePages:![self hasMultiplePages]];
}

- (void)toggleHyphenation:(id)sender {
    [self setHyphenationFactor:([self hyphenationFactor] > 0.0) ? 0.0 : 0.9];	/* Toggle between 0.0 and 0.9 */
    if ([self isRichText]) [self setDocumentEdited:YES];
}

- (void)doPageLayout:(id)sender {
    NSPrintInfo *tempPrintInfo = [[self printInfo] copy];
    NSPageLayout *pageLayout = [NSPageLayout pageLayout];
    [pageLayout beginSheetWithPrintInfo:tempPrintInfo modalForWindow:[self window] delegate:self didEndSelector:@selector(didEndPageLayout:returnCode:contextInfo:) contextInfo:(void *)tempPrintInfo];
}

- (void)didEndPageLayout:(NSPageLayout *)pageLayout returnCode:(int)result contextInfo:(void *)contextInfo {
    NSPrintInfo *tempPrintInfo = (NSPrintInfo *)contextInfo;
    if (result == NSOKButton) [self setPrintInfo:tempPrintInfo];
    [tempPrintInfo release];
}

/* doRevert does the actual reverting...
*/
- (void)doRevert {
    NSRange prevSelectedRange = [[self firstTextView] selectedRange];
    if (![self loadFromPath:revertDocumentName encoding:documentEncoding ignoreRTF:openedIgnoringRTF ignoreHTML:openedIgnoringHTML error:NULL]) {
        NSString *alertTitle = [NSString stringWithFormat:NSLocalizedString(@"Couldn\\U2019t revert to saved version of \\U201c%@\\U201d.", @"Title of alert panel indicating file couldn't be reverted."), displayName(revertDocumentName)];
        // No endSheet method required for the alert, as there's only one possible response and nothing to clean up
        NSBeginAlertSheet(alertTitle, 
            NSLocalizedString(@"OK", @"OK"), nil, nil, [self window],  
            self, NULL, NULL, NULL, 
            NSLocalizedString(@"The file has been removed.", @"Subtitle of alert panel indicating file couldn't be reverted."));
    } else {
        // Restore selection, if still within bounds
        if (NSMaxRange(prevSelectedRange) <= [textStorage length]) {
            [[self firstTextView] setSelectedRange:prevSelectedRange];
        } else {
            [[self firstTextView] setSelectedRange:NSMakeRange(0, 0)];
        }
        [self setDocumentName:revertDocumentName];
	[[self undoManager] removeAllActions];
        [self setDocumentEdited:NO];
        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:revertDocumentName]];
    }
}

/* revert: puts up an alert or calls doRevert directly
*/
- (void)revert:(id)sender {
    if (revertDocumentName != nil) {
        if ([self isDocumentEdited]) {
            NSString *alertTitle = [NSString stringWithFormat:NSLocalizedString(@"Revert to saved version of \\U201c%@\\U201d?", @"Title of alert panel warning user about effects of reverting."), displayName(revertDocumentName)];
            NSBeginAlertSheet(alertTitle, 
                NSLocalizedString(@"OK", @"OK"), NSLocalizedString(@"Cancel", @"Button choice allowing user to cancel."), nil, [self window], self, NULL, @selector(didEndRevertSheet:returnCode:contextInfo:), NULL,
                NSLocalizedString(@"Reverting will lose your current changes.", @"Subtitle of alert panel warning user about effects of reverting."));
        } else {
            [self doRevert];
        }
    }
}

- (void)didEndRevertSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    if (returnCode == NSAlertDefaultReturn) [self doRevert];
}

- (void)close:(id)sender {
    [[self window] close];
}

/* This no longer has a menu item ("Save To...") by default. It basically allows saving the file elsewhere w/out changing where it is.
*/
- (void)saveTo:(id)sender {
    [self saveDocument:YES rememberName:NO shouldClose:NO];
}

- (void)saveAs:(id)sender {
    [self saveDocument:YES rememberName:YES shouldClose:NO];
}

- (void)save:(id)sender {
    [self saveDocument:NO rememberName:YES shouldClose:NO];
}

/* Used for chaining saves in cases where all edited documents need to be saved. When one document is saved, it calls this method with the flag argument indicating whether the save was cancelled or not.
*/
+ (void)saveAllEnumeration:(BOOL)cont {
    if (cont) {
        NSArray *windows = [NSApp windows];
        unsigned count = [windows count];
        while (count--) {
            NSWindow *window = [windows objectAtIndex:count];
            Document *document = [Document documentForWindow:window];
            if (document) {
                if ([document isDocumentEdited]) {
                    [document saveDocument:NO name:nil rememberName:YES shouldClose:NO whenDone:@selector(saveAllEnumeration:)];
                    return;
                }
            }
        }
    }
}

/* Same as above, but once there are no more documents to save, quits the app.
*/
+ (void)reviewChangesAndQuitEnumeration:(BOOL)cont {
    if (cont) {
        NSArray *windows = [NSApp windows];
        unsigned count = [windows count];
        while (count--) {
            NSWindow *window = [windows objectAtIndex:count];
            Document *document = [Document documentForWindow:window];
            if (document) {
                if ([document isDocumentEdited]) {
                    [document askToSave:@selector(reviewChangesAndQuitEnumeration:)];
                    return;
                }
            }
        }
    }
    // if we get to here, either cont was YES and we reviewed all documents, or cont was NO and we don't want to quit
    [NSApp replyToApplicationShouldTerminate:cont];
}

/* Loads the "encoding" accessory view used in save plain and open panels. There is a checkbox in the accessory which has different purposes in each case; so we let the caller set the title and other info for that checkbox.
*/
+ (NSView *)encodingAccessory:(unsigned)encoding includeDefaultEntry:(BOOL)includeDefaultItem encodingPopUp:(NSPopUpButton **)popup checkBox:(NSButton **)button {
    OpenSaveAccessoryOwner *owner = [[[OpenSaveAccessoryOwner alloc] init] autorelease];
    // Rather than caching, load the accessory view everytime, as it might appear in multiple panels simultaneously.
    if (![NSBundle loadNibNamed:@"EncodingAccessory" owner:owner])  {
        NSLog(@"Failed to load EncodingAccessory.nib");
        return nil;
    }
    if (popup) *popup = owner->encodingPopUp;
    if (button) *button = owner->checkBox;
    [[EncodingManager sharedInstance] setupPopUp:owner->encodingPopUp selectedEncoding:encoding withDefaultEntry:includeDefaultItem];
    return [owner->accessoryView autorelease];
}

+ (void)openWithEncodingAccessory:(BOOL)flag {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    BOOL ignoreRichPref = [[Preferences objectForKey:IgnoreRichText] boolValue];
    BOOL ignoreHTMLPref = [[Preferences objectForKey:IgnoreHTML] boolValue];
    NSButton *ignoreRichTextButton;
    NSPopUpButton *encodingPopUp;
    
    if (flag) {
        [panel setAccessoryView:[self encodingAccessory:[[Preferences objectForKey:PlainTextEncodingForRead] intValue] includeDefaultEntry:YES encodingPopUp:&encodingPopUp checkBox:&ignoreRichTextButton]];
        [ignoreRichTextButton setTitle:NSLocalizedString(@"Ignore rich text commands", @"Checkbox indicating that when opening a rich text file, the rich text should be ignored (causing the file to be loaded as plain text)")];
        // Also set tooltip: "If selected, HTML and RTF files will be loaded as plain text, allowing you to see and edit the HTML or RTF directives"
        // If the ignoreRichText and ignoreHTML preference values do not agree, then the initial state of the ignore button
        // in the panel should be "mixed" state, indicating it will do the appropriate thing depending on the file selected
        if (ignoreRichPref != ignoreHTMLPref) {
            [ignoreRichTextButton setAllowsMixedState:YES];
            [ignoreRichTextButton setState:NSMixedState];
        } else {
            if ([ignoreRichTextButton allowsMixedState]) [ignoreRichTextButton setAllowsMixedState:NO];
            [ignoreRichTextButton setState:ignoreRichPref ? NSOnState : NSOffState];
        }
    }
    [panel setAllowsMultipleSelection:YES];
    if ([[Preferences objectForKey:OpenPanelFollowsMainWindow] boolValue]) [panel setDirectory:[Document directoryOfMainWindow]];
    if ([panel runModal]) {
        NSMutableArray *unopenedFilenames = nil;
        NSArray *filenames = [panel filenames];
	NSError *error;
        unsigned cnt, numFiles = [filenames count];

	if (flag) {
	    if ([ignoreRichTextButton state] == NSOffState) {
                ignoreRichPref = ignoreHTMLPref = NO;
            } else if ([ignoreRichTextButton state] == NSOnState) {
                ignoreRichPref = ignoreHTMLPref = YES;
            } // Otherwise they both remain at their respective state...
        }

	Document *previousDocument = nil;   // We open multiply docs open front-to-back, for performance reasons
        for (cnt = 0; cnt < numFiles; cnt++) {
            NSString *filename = [filenames objectAtIndex:cnt];
	    Document *document = [Document openDocumentWithPath:filename encoding:flag ? [[encodingPopUp selectedItem] tag] : UnknownStringEncoding ignoreRTF:ignoreRichPref ignoreHTML:ignoreHTMLPref behind:previousDocument error:&error];
            if (!document) {
		if (!unopenedFilenames) unopenedFilenames = [NSMutableArray array];
		if (!error) error = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileReadUnknownError userInfo:[NSDictionary dictionaryWithObject:filename forKey:NSFilePathErrorKey]];  // If no error came back for some reason, create a generic one
		[unopenedFilenames addObject:error];
	    } else {
		previousDocument = document;
	    }
	}
	if (unopenedFilenames) [self displayOpenFailures:unopenedFilenames someSucceeded:([unopenedFilenames count] < numFiles)];
    }
}

+ (void)open:(id)sender {
    [self openWithEncodingAccessory:YES];
}

/* Put up panel indicating failure to open one or more files. errors is an array containing NSErrors for files which couldn't be opened; alertTitle is the title to be shown. 
*/
+ (void)displayOpenFailures:(NSArray *)errors someSucceeded:(BOOL)someFilesOpened {
    int numFailedFiles = [errors count];

    if (numFailedFiles == 1) {	// Just one file failed
	[[NSAlert alertWithError:[errors objectAtIndex:0]] runModal];
    } else {
	// Choose appropriate string, based on whether all files failed or not
	NSString *alertTitle = someFilesOpened ? 
	    NSLocalizedString(@"Couldn\\U2019t open some of the specified files.", @"User tried to open more than one file; some (more than one) of them failed to open.") :
	    NSLocalizedString(@"Couldn\\U2019t open any of the specified files.", @"User tried to open more than one file; all of them failed to open.");
	// Now include the file names along with failure reasons in the string
	NSMutableString *errorMessage = [NSMutableString string];
	int cnt, numFiles = [errors count];
	if (numFiles > 5) numFiles = 3;	// If many, just show a subset
	for (cnt = 0; cnt < numFiles; cnt++) {
	    NSError *error = [errors objectAtIndex:cnt];
	    NSString *path = [[error userInfo] objectForKey:NSFilePathErrorKey];
	    if (path) {
		NSString *reason = [error localizedFailureReason];
		if (reason) {
		    [errorMessage appendFormat:NSLocalizedString(@"%@ (%@)", @"String for alert panel showing filename which failed to open followed by the reason why it failed to open, for instance: 'My File (You do not have enough access privileges)'"), displayName(path), reason];
		} else {
		    [errorMessage appendString:displayName(path)];
		}
		if (cnt < numFiles-1) [errorMessage appendString:@"\n"];
	    }
	}
	if (numFiles < numFailedFiles) [errorMessage appendFormat:@"\n%@", NSLocalizedString(@"(And others)", @"Shown when many files failed to open. Shown at the end of the list of filenames, when there are too many to show.")];
	(void)NSRunAlertPanel(alertTitle, @"%@", NSLocalizedString(@"OK", @"OK"), nil, nil, errorMessage);
    }
}


/* Returns YES if the document can be closed. If the document is edited, puts up an alert and returns NO and gives the user a chance to save. 
*/
- (BOOL)canCloseDocument {
    if (isDocumentEdited) {
        [self askToSave:NULL];
        return NO;
    }
    return YES;
}

/* This method will put up an alert asking whether the document should be saved; if yes, then goes on to put up panels and such. The specified callback will be called with YES or NO at the end (NO if user cancelled the save).
*/
- (void)askToSave:(SEL)callback {
    [[self window] makeKeyAndOrderFront:nil];
    NSBeginAlertSheet(NSLocalizedString(@"Do you want to save changes to this document before closing?", @"Title in the alert panel  when the user tries to close a window containing an unsaved document."),
        NSLocalizedString(@"Save", @"Button choice which allows the user to save the document."),
        NSLocalizedString(@"Don\\U2019t Save", @"Button choice which allows the user to cancel the save of a document."),
        NSLocalizedString(@"Cancel", @"Button choice allowing user to cancel."), 
        [self window], self, @selector(willEndCloseSheet:returnCode:contextInfo:), @selector(didEndCloseSheet:returnCode:contextInfo:), (void *)callback,
        NSLocalizedString(@"If you don\\U2019t save, your changes will be lost.", @"Subtitle in the alert panel when the user tries to close a window containing an unsaved document."));
}

/* We implement willEnd to check for NSAlertAlternateReturn here, because if the user indicates "close anyway," we want the window to go away immediately, rather than having the sheet slide up first.
*/
- (void)willEndCloseSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    if (returnCode == NSAlertAlternateReturn) {		/* "Don't Save" */
        [[self window] close];
        if (contextInfo) ((void (*)(id, SEL, BOOL))objc_msgSend)([self class], (SEL)contextInfo, YES);	// Send callback (YES indicates continue saving)
    }
}

- (void)didEndCloseSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    if (returnCode == NSAlertDefaultReturn) {		/* "Save" */
        [self saveDocument:NO name:nil rememberName:YES shouldClose:YES whenDone:(SEL)contextInfo];
    } else if (returnCode == NSAlertOtherReturn) {	/* "Cancel" */
        if (contextInfo) ((void (*)(id, SEL, BOOL))objc_msgSend)([self class], (SEL)contextInfo, NO);	// Send callback indicating save cancelled
    }
}



/* Saving */

/* The following few methods are for saving documents from the UI. The situation is a bit complicated due to full support for doc-modal sheets. Overall, here's the flow of save calls; the three main routines can put up panels which call the indented methods:

   saveDocument:name:rememberName:shouldClose:whenDone: (calls getDocumentNameAndSave:)
      if change doc format:  didEndFormatWarningSheet:returnCode:contextInfo: (calls back into getDocumentNameAndSave:)
      if change encoding:    didEndEncodingSheet:returnCode:contextInfo: (calls back into getDocumentNameAndSave:)
   getDocumentNameAndSave: (calls doSaveWithName:overwriteOK:)
      if need file name:     didEndSaveSheet:returnCode:contextInfo: (calls back into doSaveWithName:overwriteOK:)
   doSaveWithName:overwriteOK:
      if couldn't save file: didEndSaveErrorAlert:returnCode:contextInfo: (calls back into doSaveWithName:overwriteOK:)
*/
static DocumentSaveInfo *allocateDocumentContext(void) {
    return malloc(sizeof(DocumentSaveInfo));
}

static void deallocateDocumentContext(DocumentSaveInfo *docInfo) {
    [docInfo->nameForSaving release];
    [docInfo->fileToBeRemoved release];
    free(docInfo);
}


/* Action method for the "Append '.txt' extension" button
*/
- (void)appendPlainTextExtensionChanged:(id)sender {
    NSSavePanel *panel = (NSSavePanel *)[sender window];
    BOOL state = [sender state];
    [panel setAllowsOtherFileTypes:state];
    [panel setRequiredFileType:state ? @"txt" : @""];
}

/* Methods to deal with the rich text document format popup

   For genstrings:
    NSLocalizedString(@"Rich Text Format (RTF)", @"Name for Rich Text Format (RTF) documents to be displayed in the format popup of the save dialog")
    NSLocalizedString(@"HTML", @"Name for HTML documents to be displayed in the format popup of the save dialog")
    NSLocalizedString(@"Word Format", @"Name for Microsoft Word documents to be displayed in the format popup of the save dialog")
    NSLocalizedString(@"Word XML Format", @"Name for Microsoft WordML documents to be displayed in the format popup of the save dialog")
    NSLocalizedString(@"Rich Text With Graphics Format (RTFD)", @"Name for Rich Text With Graphics Format (RTFD) documents to be displayed in the format popup of the save dialog")
    NSLocalizedString(@"Web Archive", @"Name for WebArchive documents to be displayed in the format popup of the save dialog")
*/
#define NumChooseableRichTextDocumentFormats 4
const static unsigned chooseableRichTextDocumentFormats[NumChooseableRichTextDocumentFormats] = {RichTextStringEncoding, HTMLStringEncoding, DocStringEncoding, WordMLStringEncoding};
NSString *chooseableRichTextDocumentFormatNames[NumChooseableRichTextDocumentFormats] = {@"Rich Text Format (RTF)", @"HTML", @"Word Format", @"Word XML Format"};
NSString *chooseableRichTextDocumentFormatFileTypes[NumChooseableRichTextDocumentFormats] = {@"rtf", @"html", @"doc", @"xml"};

#define NumChooseableRichTextWithGraphicsDocumentFormats 2
const static unsigned chooseableRichTextWithGraphicsDocumentFormats[NumChooseableRichTextDocumentFormats] = {RichTextWithGraphicsStringEncoding, WebArchiveStringEncoding};
NSString *chooseableRichTextWithGraphicsDocumentFormatNames[NumChooseableRichTextDocumentFormats] = {@"Rich Text With Graphics Format (RTFD)", @"Web Archive"};
NSString *chooseableRichTextWithGraphicsDocumentFormatFileTypes[NumChooseableRichTextDocumentFormats] = {@"rtfd", @"webarchive"};

- (void)setupRichTextDocumentFormatAccessory:(NSSavePanel *)panel withDocumentFormat:(unsigned)currentlySelectedDocumentFormat {
    int cnt;
    if (!richTextDocumentFormatAccessory) {	// Loaded per document, as there might be multiple save panels active at any one time
        if (![NSBundle loadNibNamed:@"RichTextDocumentFormatAccessory" owner:self])  {
            NSLog(@"Failed to load RichTextDocumentFormatAccessory.nib");
            return;		// If for some reason the accessory can't be found, the user just won't be able to choose the document format
        }
    }
    [richTextDocumentFormatPopUp removeAllItems];
    for (cnt = 0; cnt < NumChooseableRichTextDocumentFormats; cnt++) [richTextDocumentFormatPopUp addItemWithTitle:NSLocalizedString(chooseableRichTextDocumentFormatNames[cnt], "")];
    for (cnt = 0; cnt < NumChooseableRichTextDocumentFormats; cnt++) if (chooseableRichTextDocumentFormats[cnt] == currentlySelectedDocumentFormat) [richTextDocumentFormatPopUp selectItemAtIndex:cnt];
    [richTextDocumentFormatPopUp setAction:@selector(richTextDocumentFormatChanged:)];
    [panel setAccessoryView:richTextDocumentFormatAccessory];
    [self richTextDocumentFormatChanged:richTextDocumentFormatPopUp];	// To initialize further
}

- (void)richTextDocumentFormatChanged:(id)sender {
    NSSavePanel *panel = (NSSavePanel *)[sender window];
    [panel setRequiredFileType:chooseableRichTextDocumentFormatFileTypes[[sender indexOfSelectedItem]]];
}

- (void)setupRichTextWithGraphicsDocumentFormatAccessory:(NSSavePanel *)panel withDocumentFormat:(unsigned)currentlySelectedDocumentFormat {
    int cnt;
    if (!richTextDocumentFormatAccessory) {	// Loaded per document, as there might be multiple save panels active at any one time
        if (![NSBundle loadNibNamed:@"RichTextDocumentFormatAccessory" owner:self])  {
            NSLog(@"Failed to load RichTextDocumentFormatAccessory.nib");
            return;		// If for some reason the accessory can't be found, the user just won't be able to choose the document format
        }
    }
    [richTextDocumentFormatPopUp removeAllItems];
    for (cnt = 0; cnt < NumChooseableRichTextWithGraphicsDocumentFormats; cnt++) [richTextDocumentFormatPopUp addItemWithTitle:NSLocalizedString(chooseableRichTextWithGraphicsDocumentFormatNames[cnt], "")];
    for (cnt = 0; cnt < NumChooseableRichTextWithGraphicsDocumentFormats; cnt++) if (chooseableRichTextWithGraphicsDocumentFormats[cnt] == currentlySelectedDocumentFormat) [richTextDocumentFormatPopUp selectItemAtIndex:cnt];
    [richTextDocumentFormatPopUp setAction:@selector(richTextWithGraphicsDocumentFormatChanged:)];
    [panel setAccessoryView:richTextDocumentFormatAccessory];
    [self richTextWithGraphicsDocumentFormatChanged:richTextDocumentFormatPopUp];	// To initialize further
}

- (void)richTextWithGraphicsDocumentFormatChanged:(id)sender {
    NSSavePanel *panel = (NSSavePanel *)[sender window];
    [panel setRequiredFileType:chooseableRichTextWithGraphicsDocumentFormatFileTypes[[sender indexOfSelectedItem]]];
}


/* Returns the untitled document name for this document. The type argument, applicable only for rich documents, determines whether the extension for that type should be appended to the name. Pass in UnknownStringEncoding if not desired. For plain text documents, we add "txt" if user wanted it added, by having specified preference.
*/
- (NSString *)untitledDocumentName:(unsigned)type {
    NSString *untitled;
    if (untitledDocNumber == 0) untitledDocNumber = nextAvailableUntitledDocNumber();
    if (untitledDocNumber > 1) {
        // "LaunchTime" table contains the very few strings needed at launch time... A little performance optimization.
        untitled = [NSString stringWithFormat:NSLocalizedStringFromTable(@"Untitled %d", @"LaunchTime", @"Name of new, untitled document with sequence number"), untitledDocNumber];
    } else {
        untitled = NSLocalizedStringFromTable(@"Untitled", @"LaunchTime", @"Name of new, untitled document");
    }
    if ([self isRichText]) {	// Run through the types and pick the right extension
        NSString *extension = nil;
	int cnt; 
	if ([textStorage containsAttachments]) {
	    for (cnt = 0; cnt < NumChooseableRichTextWithGraphicsDocumentFormats && !extension; cnt++) if (chooseableRichTextWithGraphicsDocumentFormats[cnt] == type) extension = chooseableRichTextWithGraphicsDocumentFormatFileTypes[cnt];
	} else {
	    for (cnt = 0; cnt < NumChooseableRichTextDocumentFormats && !extension; cnt++) if (chooseableRichTextDocumentFormats[cnt] == type) extension = chooseableRichTextDocumentFormatFileTypes[cnt];
	}
	if (extension) untitled = [untitled stringByAppendingPathExtension:extension];
    } else if ([[Preferences objectForKey:AddExtensionToNewPlainTextFiles] boolValue]) {
        untitled = [untitled stringByAppendingPathExtension:@"txt"];
    }
    return untitled;
}


/* Entry point for saving the document from the UI.    
   First checks to see if any warnings need to be shown; if so, does those; otherwise calls the save routine directly.
   If showSavePanel is YES, of if a name is needed, the save panel will also be shown.
   rememberNewNameAndSuch causes the document's name, encoding, etc to be reset after the save (basically this should be false for a "saveTo" operation and true otherwise).
   shouldClose indicates whether the document should be closed if the save is successfully performed.
*/
- (void)saveDocument:(BOOL)showSavePanel rememberName:(BOOL)rememberNewNameAndSuch shouldClose:(BOOL)shouldClose {
    [self saveDocument:showSavePanel name:nil rememberName:rememberNewNameAndSuch shouldClose:shouldClose whenDone:NULL];
}

/* If pathForSaving is nil, uses document's existing name.
*/
- (void)saveDocument:(BOOL)showSavePanel name:(NSString *)pathForSaving rememberName:(BOOL)rememberNewNameAndSuch shouldClose:(BOOL)shouldClose whenDone:(SEL)callback {
    DocumentSaveInfo *docInfo;

    if ([self hasSheet]) return;	// This means we already have a sheet up. This is to prevent "Save All" type enumerations from getting into here.
        
    docInfo = allocateDocumentContext();
    docInfo->nameForSaving = pathForSaving ? [pathForSaving copy] : [[self documentName] copy];
    docInfo->encodingForSaving = 0;
    docInfo->haveToChangeType = NO;
    docInfo->showEncodingAccessory = NO;
    docInfo->rememberName = rememberNewNameAndSuch;
    docInfo->shouldClose = shouldClose;
    docInfo->showSavePanel = showSavePanel;
    docInfo->whenDoneCallback = callback;
    docInfo->hideExtension = FileExtensionPreviousState;
    docInfo->encodingPopUp = nil;
    docInfo->appendPlainTextExtensionButton = nil;
    docInfo->showRichTextDocumentFormatAccessory = NO;
    docInfo->showRichTextWithGraphicsDocumentFormatAccessory = NO;
    docInfo->doingRTFDConversion = NO;
    docInfo->fileToBeRemoved = nil;

    if ([self isRichText]) {	// Rich document case
        if (docInfo->nameForSaving && [@"rtfd" isEqualToString:[docInfo->nameForSaving pathExtension]]) {
            docInfo->encodingForSaving = RichTextWithGraphicsStringEncoding;
        } else {
            if ([textStorage containsAttachments]) {
                if (documentEncoding == WebArchiveStringEncoding) docInfo->encodingForSaving = WebArchiveStringEncoding;
                else docInfo->encodingForSaving = RichTextWithGraphicsStringEncoding;
            } else {
                if (documentEncoding == HTMLStringEncoding) docInfo->encodingForSaving = HTMLStringEncoding;
                else if (documentEncoding == DocStringEncoding) docInfo->encodingForSaving = DocStringEncoding;
                else if (documentEncoding == WordMLStringEncoding) docInfo->encodingForSaving = WordMLStringEncoding;
                else if (documentEncoding == WebArchiveStringEncoding) docInfo->encodingForSaving = WebArchiveStringEncoding;
                else if (documentEncoding == RichTextWithGraphicsStringEncoding) docInfo->encodingForSaving = RichTextWithGraphicsStringEncoding;
                else docInfo->encodingForSaving = RichTextStringEncoding;
            }
	    if ([self converted] || documentEncoding == SimpleTextStringEncoding) {
                NSString *newFormatName = (docInfo->encodingForSaving == RichTextWithGraphicsStringEncoding) ?
                    NSLocalizedString(@"rich text with graphics (RTFD)", @"Rich text with graphics file format name, displayed in alert") :
                    NSLocalizedString(@"rich text", @"Rich text file format name, displayed in alert");
		if ([self converted]) {
                    NSBeginAlertSheet(NSLocalizedString(@"Please supply a new name.", @"Title of alert panel which brings up a warning while saving, asking for new name"),
			NSLocalizedString(@"Save with new name", @"Button choice allowing user to choose a new name"), NSLocalizedString(@"Cancel", @"Button choice allowing user to cancel."), nil, 
			[self window], self, NULL, @selector(didEndFormatWarningSheet:returnCode:contextInfo:), docInfo,
			NSLocalizedString(@"This document was converted from a format that TextEdit cannot save. It will be saved in %@ format with a new name.", @"Contents of alert panel informing user that they need to supply a new file name because the file needs to be saved using a different format than originally read in"),
			newFormatName);
		} else {
                    NSString *oldFormatName = NSLocalizedString(@"SimpleText", @"SimpleText file format name, displayed in alert");
                    NSBeginAlertSheet(NSLocalizedString(@"Please supply a new name.", @"Title of alert panel which brings up a warning while saving, asking for new name"),
			NSLocalizedString(@"Save with new name", @"Button choice allowing user to choose a new name"), NSLocalizedString(@"Cancel", @"Button choice allowing user to cancel."), nil, 
			[self window], self, NULL, @selector(didEndFormatWarningSheet:returnCode:contextInfo:), docInfo,
			NSLocalizedString(@"TextEdit does not save files in %@ format; document will be saved in %@ format with a new name.", @"Contents of alert panel informing user that they need to supply a new file name because the file needs to be saved using a different format than originally read in"),
			oldFormatName, newFormatName);
		}
                return;
            } else if ((docInfo->encodingForSaving == RichTextWithGraphicsStringEncoding) && docInfo->nameForSaving && ![@"rtfd" isEqualToString:[docInfo->nameForSaving pathExtension]]) {
                // At this point we will provide a warning about RTFD conversion, along with the ability to save with a new name (default button), or overwrite existing name (which will actually save .rtfd and remove .rtf)
                NSString *defaultOption = NSLocalizedString(@"Save with new name", @"Button choice allowing user to choose a new name");
                NSString *saveOption = NSLocalizedString(@"Save", @"Button choice which allows the user to save the document.");
                if (docInfo->showSavePanel || [[NSFileManager defaultManager] fileExistsAtPath:[[docInfo->nameForSaving stringByDeletingPathExtension] stringByAppendingPathExtension:@"rtfd"]]) {	// If we're showing the save panel anyway, or a file with the RTFD name already exists, just show one button allowing users to confirm the save
                    defaultOption = saveOption;
                    saveOption = nil;
                }
                docInfo->doingRTFDConversion = YES;
                NSBeginAlertSheet(NSLocalizedString(@"Are you sure you want to save using RTFD format?", @"Title of alert panel which brings up a warning while saving"),
                    defaultOption, NSLocalizedString(@"Cancel", @"Button choice allowing user to cancel."), saveOption,
                    [self window], self, NULL, @selector(didEndFormatWarningSheet:returnCode:contextInfo:), docInfo,
                    NSLocalizedString(@"This document contains graphics and will be saved using RTFD (RTF with graphics) format. RTFD documents are not compatible with some applications. Save anyway?", @"Contents of alert panel informing user that the document is being converted from RTF to RTFD, and allowing them to cancel, save anyway, or save with new name"));
                return;
            } else if ([self lossy] && !showSavePanel) {
                NSBeginAlertSheet(NSLocalizedString(@"Are you sure you want to overwrite the document?", @"Title of alert panel which brings up a warning about saving over the same document"),
                    NSLocalizedString(@"Save with new name", @"Button choice allowing user to choose a new name"), 
                    NSLocalizedString(@"Cancel", @"Button choice allowing user to cancel."), 
                    NSLocalizedString(@"Overwrite", @"Button choice allowing user to overwrite the document."),
                    [self window], self, NULL, @selector(didEndFormatWarningSheet:returnCode:contextInfo:), docInfo,
                    NSLocalizedString(@"Overwriting this document might cause you to lose some of the original formatting.  Would you like to save the document using a new name?", @"Contents of alert panel informing user that they need to supply a new file name because the save might be lossy"));
                return;
            }
        }
    } else {	// Plain document case; we have to figure out what character encoding to use
        NSString *string = [textStorage string];
        docInfo->showEncodingAccessory = YES;
        docInfo->encodingForSaving = documentEncoding;
        // If there is no existing encoding, or there is, but it is no longer suitable, clear it in preparation for computing a new one
        if ((docInfo->encodingForSaving != UnknownStringEncoding) && ![string canBeConvertedToEncoding:docInfo->encodingForSaving]) {
            docInfo->haveToChangeType = YES;
            docInfo->encodingForSaving = UnknownStringEncoding;
        }
        if (docInfo->encodingForSaving == UnknownStringEncoding) {
            // We try user's selected encoding, followed by their default language encoding, followed by Unicode (if in user's encodings list), followed by all other encodings in their encodings list; if none work Unicode is the last resort
            NSStringEncoding defaultEncoding = [[Preferences objectForKey:PlainTextEncodingForWrite] intValue];
            if ((defaultEncoding != UnknownStringEncoding) && [string canBeConvertedToEncoding:defaultEncoding]) {
                docInfo->encodingForSaving = defaultEncoding;
            } else {
                NSStringEncoding defaultCStringEncoding = [NSString defaultCStringEncoding];
                BOOL hasDefaultCStringEncoding = NO;	// Whether user enabled this encoding
                BOOL hasUnicodeEncoding = NO;		// Whether user enabled this encoding
                NSArray *encodings = [[EncodingManager sharedInstance] enabledEncodings];
                int cnt, numEncodings = [encodings count];
                for (cnt = 0; cnt < numEncodings; cnt++) {
                    NSStringEncoding encoding = [[encodings objectAtIndex:cnt] unsignedIntValue];
                    if (encoding == defaultCStringEncoding) hasDefaultCStringEncoding = YES;
                    if (encoding == NSUnicodeStringEncoding) hasUnicodeEncoding = YES;
                }
                if (hasDefaultCStringEncoding && [string canBeConvertedToEncoding:defaultCStringEncoding]) {
                    docInfo->encodingForSaving = defaultCStringEncoding;
                } else if (hasUnicodeEncoding) {
                    docInfo->encodingForSaving = NSUnicodeStringEncoding;
                } else for (cnt = 0; cnt < numEncodings; cnt++) {	// Otherwise enumerate the encodings and find an appropriate one
                    NSStringEncoding encoding = [[encodings objectAtIndex:cnt] unsignedIntValue];
                    if ((encoding != defaultEncoding) && (encoding != NSUnicodeStringEncoding) && (encoding != NSUTF8StringEncoding) && (encoding != defaultCStringEncoding) && [string canBeConvertedToEncoding:encoding]) {	// Skip over encodings we know work or we know we already checked
                        docInfo->encodingForSaving = encoding;
                        break;
                    }
                }
            }
            if (docInfo->encodingForSaving == UnknownStringEncoding) docInfo->encodingForSaving = NSUnicodeStringEncoding;	// Last resort...
            if (docInfo->haveToChangeType) {
                NSString *alertTitle = [NSString stringWithFormat:NSLocalizedString(@"This document can no longer be saved using its original %@ encoding.", @"Title of alert panel informing user that the file's string encoding needs to be changed."), [NSString localizedNameOfStringEncoding:documentEncoding]];
                NSBeginAlertSheet(alertTitle, NSLocalizedString(@"OK", @"OK"), nil, nil, 
                    [self window], self, NULL, @selector(didEndEncodingSheet:returnCode:contextInfo:), docInfo,
                    NSLocalizedString(@"Please choose another encoding (such as %@).", @"Subtitle of alert panel informing user that the file's string encoding needs to be changed"), [NSString localizedNameOfStringEncoding:docInfo->encodingForSaving]);
                return;
            }
        }
    }

    // If we did not put up a sheet, then we get here, which means we can go on to the save panel
    [self getDocumentNameAndSave:docInfo];
}


/* Called from the alert that asks user whether they're sure they want to save... If yes, we go onto put up a save panel
*/
- (void)didEndFormatWarningSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    DocumentSaveInfo *docInfo = contextInfo;
    if (returnCode == NSAlertDefaultReturn) {	// "Save with new name"
        docInfo->doingRTFDConversion = NO;	// We clear this; the document is saved as a copy
        [docInfo->nameForSaving release];
        docInfo->nameForSaving = nil; 	// force user to provide a new name
        [self getDocumentNameAndSave:docInfo];
    } else if (returnCode == NSAlertOtherReturn) {	// "Overwrite" (when applicable)
        // Note that for RTF -> RTFD conversion, we still need to do a new name
        if (docInfo->doingRTFDConversion) {
            docInfo->fileToBeRemoved = docInfo->nameForSaving;
            docInfo->nameForSaving = [[[docInfo->fileToBeRemoved stringByDeletingPathExtension] stringByAppendingPathExtension:@"rtfd"] copy];
        }
        [self getDocumentNameAndSave:docInfo];
    } else {						// "Cancel" --- NSAlertAlternateReturn
        if (docInfo->whenDoneCallback) ((void (*)(id, SEL, BOOL))objc_msgSend)([self class], docInfo->whenDoneCallback, NO);
        deallocateDocumentContext(docInfo);
    }
}

/* Called from the alert that lets the user know that the encoding needs to change...
*/
- (void)didEndEncodingSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    [self getDocumentNameAndSave:contextInfo];
}

/* Saves the document by first getting the document name (if needed)
*/
- (void)getDocumentNameAndSave:(DocumentSaveInfo *)docInfo {
    if (docInfo->nameForSaving && !docInfo->haveToChangeType && !docInfo->showSavePanel) {	// No save panel necessary!
        [self doSaveWithName:docInfo overwriteOK:[[Preferences objectForKey:OverwriteReadOnlyFiles] boolValue]];
    } else {
        NSString *nameForSaving, *dirForSaving;
        NSSavePanel *panel = [NSSavePanel savePanel];
        BOOL updateEncoding = (docInfo->haveToChangeType || docInfo->showEncodingAccessory) ? YES : NO;
        [panel setCanSelectHiddenExtension:YES];
        switch (docInfo->encodingForSaving) {
            case RichTextStringEncoding:
            case HTMLStringEncoding:
            case DocStringEncoding:
            case WordMLStringEncoding:
                [self setupRichTextDocumentFormatAccessory:panel withDocumentFormat:docInfo->encodingForSaving];
                docInfo->showRichTextDocumentFormatAccessory = YES;
                [panel setTitle:NSLocalizedString(@"Save Rich Text", @"Title of save and alert panels when saving rich text (RTF, Word, or others)")];
                updateEncoding = NO;
                break;
            case RichTextWithGraphicsStringEncoding:
            case WebArchiveStringEncoding:
                [self setupRichTextWithGraphicsDocumentFormatAccessory:panel withDocumentFormat:docInfo->encodingForSaving];
                docInfo->showRichTextWithGraphicsDocumentFormatAccessory = YES;
                [panel setTitle:NSLocalizedString(@"Save Rich Text With Graphics", @"Title of save and alert panels when saving rich text with graphics (RTFD and WebArchive)")];
                updateEncoding = NO;
                break;
            default:
                [panel setTitle:NSLocalizedString(@"Save Plain Text", @"Title of save and alert panels when saving plain text")];
                if ([[Preferences objectForKey:AddExtensionToNewPlainTextFiles] boolValue]) {
                    [panel setRequiredFileType:@"txt"];
                    [panel setAllowsOtherFileTypes:YES];
                }
                if (updateEncoding) {
                    NSString *string;
                    int cnt;
                    [panel setAccessoryView:[[self class] encodingAccessory:docInfo->encodingForSaving includeDefaultEntry:NO  encodingPopUp:&(docInfo->encodingPopUp) checkBox:&(docInfo->appendPlainTextExtensionButton)]];
                    // Set up the checkbox
                    [docInfo->appendPlainTextExtensionButton setTitle:NSLocalizedString(@"If no extension is provided, use \\U201c.txt\\U201d.", @"Checkbox indicating that if the user does not specify an extension when saving a plain text file, .txt will be used")];
                    [docInfo->appendPlainTextExtensionButton setState:[[Preferences objectForKey:AddExtensionToNewPlainTextFiles] boolValue]];
                    [docInfo->appendPlainTextExtensionButton setAction:@selector(appendPlainTextExtensionChanged:)];
                    [docInfo->appendPlainTextExtensionButton setTarget:self];
                    // Further set up the encoding popup
                    cnt = [docInfo->encodingPopUp numberOfItems];
                    string = [textStorage string];
                    if (cnt * [string length] < 5000000) {	// Otherwise it's just too slow; would be nice to make this more dynamic. With large docs and many encodings, the items just won't be validated.
                        while (cnt--) {	// No reason go backwards accept to use one variable instead of two
                            int encoding = [[docInfo->encodingPopUp itemAtIndex:cnt] tag];
                            // Hardwire some encodings known to allow any content
                            if ((encoding != UnknownStringEncoding) && (encoding != NSUnicodeStringEncoding) && (encoding != NSUTF8StringEncoding) && (encoding != NSNonLossyASCIIStringEncoding) &&                                                                     ![string canBeConvertedToEncoding:encoding]) {
                                [[docInfo->encodingPopUp itemAtIndex:cnt] setEnabled:NO];
                            }
                        }
                    }
                }
                break;
        }
        [[self window] makeKeyAndOrderFront:nil];
        nameForSaving = docInfo->nameForSaving ? [docInfo->nameForSaving lastPathComponent] : [self untitledDocumentName:docInfo->encodingForSaving];
        if (docInfo->nameForSaving) {
            dirForSaving = [docInfo->nameForSaving stringByDeletingLastPathComponent];
        } else if ([[Preferences objectForKey:OpenPanelFollowsMainWindow] boolValue]) {
            dirForSaving = [Document directoryOfMainWindow];
        } else {
            dirForSaving = nil;		// Causes save panel to use the last folder that was visited
        }
        [panel beginSheetForDirectory:dirForSaving file:nameForSaving modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(didEndSaveSheet:returnCode:contextInfo:) contextInfo:docInfo];
    }
}

/* Called when the save panel is dismissed. If OK, goes onto call doSaveWithName:overwriteOK:
*/
- (void)didEndSaveSheet:(NSSavePanel *)savePanel returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    DocumentSaveInfo *docInfo = contextInfo;
    [savePanel orderOut:nil];
    [savePanel setDelegate:nil];
    if (returnCode == NSOKButton) {
        [docInfo->nameForSaving release];
        docInfo->nameForSaving = [[savePanel filename] copy];
        docInfo->hideExtension = [savePanel isExtensionHidden] ? FileExtensionHidden : FileExtensionShown;
        if (docInfo->showEncodingAccessory) {
            docInfo->encodingForSaving = [[docInfo->encodingPopUp selectedItem] tag];
        } else if (docInfo->showRichTextDocumentFormatAccessory) {
            docInfo->encodingForSaving = chooseableRichTextDocumentFormats[[richTextDocumentFormatPopUp indexOfSelectedItem]];
        } else if (docInfo->showRichTextWithGraphicsDocumentFormatAccessory) {
            docInfo->encodingForSaving = chooseableRichTextWithGraphicsDocumentFormats[[richTextDocumentFormatPopUp indexOfSelectedItem]];
        }
        [self doSaveWithName:docInfo overwriteOK:[[Preferences objectForKey:OverwriteReadOnlyFiles] boolValue]];
    } else {
        if (docInfo->whenDoneCallback) ((void (*)(id, SEL, BOOL))objc_msgSend)([self class], docInfo->whenDoneCallback, NO);
        deallocateDocumentContext(docInfo);
    }
}

/* Now we have the name; save the document. If not successful, puts up an alert and is eventually called back if user requests overwrite.
*/
- (void)doSaveWithName:(DocumentSaveInfo *)docInfo overwriteOK:(BOOL)overwrite {
    /* The value of updateFileNames: should become conditional on whether we're doing Save To at some point.  Also, we'll want to avoid doing the stuff inside the if if we're doing Save To. */
    SaveStatus status = [self saveToPath:docInfo->nameForSaving encoding:docInfo->encodingForSaving updateFilenames:YES overwriteOK:overwrite hideExtension:docInfo->hideExtension];

    /* If we could not save, but the overwrite flag was turned off, try saving again with it on... */
    if (status == SaveStatusOK) {
        if (docInfo->fileToBeRemoved && ![docInfo->fileToBeRemoved isEqual:docInfo->nameForSaving]) {	// The last part is a sanity check
            // Trash the file that this new save is replacing
            [[NSWorkspace sharedWorkspace] performFileOperation:NSWorkspaceRecycleOperation source:[docInfo->fileToBeRemoved stringByDeletingLastPathComponent] destination:@"" files:[NSArray arrayWithObject:[docInfo->fileToBeRemoved lastPathComponent]] tag:NULL];
        }
        [[self firstTextView] breakUndoCoalescing];	// Tell the text system to put a break in the coalescing of text undo
        if (docInfo->rememberName) {
            [self setConverted:NO];
            [self setLossy:NO];
            [self setEncoding:docInfo->encodingForSaving];
            // If we're overwriting the file, it's locked status (and hence the icon) might change, so update icon
            [self setDocumentName:docInfo->nameForSaving updateIcon:overwrite && [[Preferences objectForKey:SaveFilesWritable] boolValue]];
            [self setDocumentEdited:NO];
            [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:documentName]];
        }
        if (docInfo->whenDoneCallback) ((void (*)(id, SEL, BOOL))objc_msgSend)([self class], docInfo->whenDoneCallback, YES);
        if (docInfo->shouldClose) [[self window] close];
        if (!docInfo->showSavePanel) {
            // NSSavePanel normally tells Finder to update; but if we didn't use the save panel, do it explicitly.
            // Strictly speaking this isn't needed, but might help sync up some state in visible Finder windows.
            [[NSWorkspace sharedWorkspace] noteFileSystemChanged:[docInfo->nameForSaving stringByDeletingLastPathComponent]];
        }
        deallocateDocumentContext(docInfo);
    } else if (status == SaveStatusFileNotWritable && !overwrite) {	
        // Tell the user the save failed, but see if they want to overwrite.
        // If so, didEndSaveErrorAlert:... will call back into this function.
        NSBeginAlertSheet(NSLocalizedString(@"Couldn\\U2019t Save", @"Title of alert indicating file couldn't be saved"),
            NSLocalizedString(@"Don\\U2019t Overwrite", @"Button in alert panel indicating it's not OK to overwrite the document when saving."),
            NSLocalizedString(@"Overwrite", @"Button choice allowing user to overwrite the document."), nil, [self window], self, NULL, @selector(didEndSaveErrorAlert:returnCode:contextInfo:), docInfo, 
            NSLocalizedString(@"The file \\U201c%@\\U201d is read-only. Attempt to overwrite?", @"Message indicating document couldn't be saved because it is read-only and and asking whether we should attempt to overwrite it."), displayName(docInfo->nameForSaving));
    } else if (status == SaveStatusFileEditedExternally && !overwrite) {	
        // Tell the user the file was saved from under TextEdit, but see if they want to overwrite.
        // If so, didEndSaveErrorAlert:... will call back into this function.
        NSBeginAlertSheet(NSLocalizedString(@"The file has been changed by another application since you opened or saved it.", @"Title of alert indicating file was edited externally"),
            NSLocalizedString(@"Don\\U2019t Save", @"Button choice which allows the user to cancel the save of a document."),
            NSLocalizedString(@"Save", @"Button choice which allows the user to save the document."), nil, [self window], self, NULL, @selector(didEndSaveErrorAlert:returnCode:contextInfo:), docInfo, 
            NSLocalizedString(@"Saving might cause you to lose some changes. Save anyway?", @"Message indicating document couldn't be saved because it is read-only and and asking whether we should attempt to overwrite it."));
    } else if (status == SaveStatusDestinationNotWritable) {	
        // Tell the user the save failed because the destination folder is not writable
        NSBeginAlertSheet(NSLocalizedString(@"Couldn\\U2019t Save", @"Title of alert indicating file couldn't be saved"),
            NSLocalizedString(@"OK", @"OK"), nil, nil, [self window], self, NULL, @selector(didEndSaveErrorAlert:returnCode:contextInfo:), docInfo,
            NSLocalizedString(@"You do not have permission to save files in the folder \\U201c%@\\U201d.", @"Message indicating document couldn't be saved because the destination folder is readonly."), displayName([docInfo->nameForSaving stringByDeletingLastPathComponent]));
    } else if (status == SaveStatusEncodingNotApplicable) {
        // Tell the user the save failed; there's nothing else to do!
        // Note that we let this function call didEndSaveErrorAlert:..., just for the cleanup.
        // The only possible response is "OK", which will cause that function to cleanup and be done.
        NSBeginAlertSheet(NSLocalizedString(@"Couldn\\U2019t Save", @"Title of alert indicating file couldn't be saved"),
            NSLocalizedString(@"OK", @"OK"), nil, nil, [self window], self, NULL, @selector(didEndSaveErrorAlert:returnCode:contextInfo:), docInfo,
            NSLocalizedString(@"Couldn\\U2019t save document with the selected text encoding %@. Please choose another encoding.", @"Message indicating document couldn't be saved with the specified text encoding."), 
            [NSString localizedNameOfStringEncoding:docInfo->encodingForSaving]);
    } else {
        // Tell the user the save failed; there's nothing else to do!
        // Note that we let this function call didEndSaveErrorAlert:..., just for the cleanup.
        // The only possible response is "OK", which will cause that function to cleanup and be done.
        NSBeginAlertSheet(NSLocalizedString(@"Couldn\\U2019t Save", @"Title of alert indicating file couldn't be saved"),
            NSLocalizedString(@"OK", @"OK"), nil, nil, [self window], self, NULL, @selector(didEndSaveErrorAlert:returnCode:contextInfo:), docInfo,
            NSLocalizedString(@"Couldn\\U2019t save document as \\U201c%@\\U201d in folder \\U201c%@\\U201d.", @"Message indicating document couldn't be saved under the given name."), displayName(docInfo->nameForSaving), displayName([docInfo->nameForSaving stringByDeletingLastPathComponent]));
    }
}

- (void)didEndSaveErrorAlert:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    DocumentSaveInfo *docInfo = contextInfo;
    
    if (returnCode == NSAlertAlternateReturn) {	// This time, attempt to save with overwrite
        [self doSaveWithName:docInfo overwriteOK:YES];
    } else {	// Otherwise user cancelled the save, give up
        if (docInfo->whenDoneCallback) ((void (*)(id, SEL, BOOL))objc_msgSend)([self class], docInfo->whenDoneCallback, NO);
        deallocateDocumentContext(docInfo);
    }
}


/* Window delegation messages */

- (BOOL)windowShouldClose:(id)sender {
    return [self canCloseDocument];
}

- (void)windowWillClose:(NSNotification *)notification {
    NSWindow *window = [self window];
    [window setDelegate:nil];
    [self release];
}

- (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame {
    return YES;
}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)defaultFrame {
    if (![self hasMultiplePages]) {	// If not wrap-to-page, use the default suggested
        return defaultFrame;
    } else {
        NSRect currentFrame = [window frame];	// Get the current size and location of the window
        NSRect standardFrame;
        NSSize paperSize = [[self printInfo] paperSize];	// Get a frame size that fits the current printable page
        NSRect newScrollView;
    
        // Get a frame for the window content, which is a scrollView
        newScrollView.origin = NSZeroPoint;
        newScrollView.size = [NSScrollView frameSizeForContentSize:paperSize hasHorizontalScroller:[scrollView hasHorizontalScroller] hasVerticalScroller:[scrollView hasVerticalScroller] borderType:[scrollView borderType]];
    
        // The standard frame for the window is now the frame that will fit the scrollView content
        standardFrame.size = [[window class] frameRectForContentRect:newScrollView styleMask:[window styleMask]].size;
        
        // Set the top left of the standard frame to be the same as that of the current window
        standardFrame.origin.y = NSMaxY(currentFrame) - standardFrame.size.height;
        standardFrame.origin.x = currentFrame.origin.x;
    
        return standardFrame;
    }
}

/* Text view delegation messages */

- (BOOL)textView:(NSTextView *)textView clickedOnLink:(id)link atIndex:(unsigned)charIndex {
    NSURL *linkURL = nil;
    
    if ([link isKindOfClass:[NSURL class]]) {	// Handle NSURL links
        linkURL = link;
    } else if ([link isKindOfClass:[NSString class]]) {	// Handle NSString links
        NSURL *baseURL = [self documentName] ? [[NSURL alloc] initFileURLWithPath:[self documentName]] : nil;
        linkURL = [NSURL URLWithString:link relativeToURL:baseURL];
        [baseURL release];
    }
    if (linkURL) {
	// Special case: We want to open HTML types in TextEdit, as presumably that is what was desired
        if ([@"file" isEqual:[linkURL scheme]]) {
            NSString *path = [linkURL path];
            if (path) {
                NSString *extension = [path pathExtension];
                if (extension && [[NSArray arrayWithObjects:@"html", @"HTML", @"htm", @"HTM", nil] containsObject:extension]) {
                    if ([Document openDocumentWithPath:path encoding:UnknownStringEncoding behind:nil error:NULL] != nil) return YES;
                }
            }
        }
        if ([[NSWorkspace sharedWorkspace] openURL:linkURL]) return YES;
    }

    // We only get here on failure... Because we beep, we return YES to indicate "success", so the text system does no further processing.
    NSBeep();
    return YES;
}

- (void)textView:(NSTextView *)view doubleClickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)rect {
    BOOL success = NO;
    NSString *name = [[[cell attachment] fileWrapper] filename];
    if (name && documentName && ![name isEqualToString:@""] && ![documentName isEqualToString:@""]) {
	NSString *fullPath = [documentName stringByAppendingPathComponent:name];
        success = [[NSWorkspace sharedWorkspace] openFile:fullPath];
    }
    if (!success) {
        NSBeep();
    }
}

- (NSArray *)textView:(NSTextView *)view writablePasteboardTypesForCell:(id <NSTextAttachmentCell>)cell atIndex:(unsigned)charIndex {
    NSArray *types = nil;
    NSString *name = [[[cell attachment] fileWrapper] filename];
    if (name && documentName && ![name isEqualToString:@""] && ![documentName isEqualToString:@""]) {
        types = [NSArray arrayWithObject:NSFilenamesPboardType];
    }
    return types;
}

- (BOOL)textView:(NSTextView *)view writeCell:(id <NSTextAttachmentCell>)cell atIndex:(unsigned)charIndex toPasteboard:(NSPasteboard *)pboard type:(NSString *)type {
    BOOL success = NO;
    NSString *name = [[[cell attachment] fileWrapper] filename];
    if ([type isEqualToString:NSFilenamesPboardType] && name && documentName && ![name isEqualToString:@""] && ![documentName isEqualToString:@""]) {
        NSString *fullPath = [documentName stringByAppendingPathComponent:name];
        [pboard setPropertyList:[NSArray arrayWithObject:fullPath] forType:NSFilenamesPboardType];
        success = YES;
    }
    return success;
}

/* Layout manager delegation message */

- (void)layoutManager:(NSLayoutManager *)layoutManager didCompleteLayoutForTextContainer:(NSTextContainer *)textContainer atEnd:(BOOL)layoutFinishedFlag {

    if ([self hasMultiplePages]) {
        NSArray *containers = [layoutManager textContainers];

        if (!layoutFinishedFlag || (textContainer == nil)) {
            // Either layout is not finished or it is but there are glyphs laid nowhere.
            NSTextContainer *lastContainer = [containers lastObject];

            if ((textContainer == lastContainer) || (textContainer == nil)) {
                // Add a new page if the newly full container is the last container or the nowhere container.
                // Do this only if there are glyphs laid in the last container (temporary solution for 3729692, until AppKit makes something better available.)
                if ([layoutManager glyphRangeForTextContainer:lastContainer].length > 0) [self addPage];
            }
        } else {
            // Layout is done and it all fit.  See if we can axe some pages.
            unsigned lastUsedContainerIndex = [containers indexOfObjectIdenticalTo:textContainer];
            unsigned numContainers = [containers count];
            while (++lastUsedContainerIndex < numContainers) {
                [self removePage];
            }
        }
    }
}


/* UndoManager notification messages */

static int undoCheckpointCount = 1;	// Start from one as we start the corresponding ivar from 0

- (int)undoCheckpointCount {
    return undoCheckpointCount;
}

- (void)undoManagerCheckpoint:(NSNotification *)notification {
    undoCheckpointCount++;
}

- (void)undoManagerChangeDone:(NSNotification *)notification {
    changeCount++;
    if (changeCount == 0) [self setDocumentEdited:NO];
    if (changeCount == 1) [self setDocumentEdited:YES];
}

- (void)undoManagerChangeUndone:(NSNotification *)notification {
    changeCount--;
    if (changeCount == 0) [self setDocumentEdited:NO];
    if (changeCount == -1) [self setDocumentEdited:YES];
}


/* Return the document in the specified window.
*/
+ (Document *)documentForWindow:(NSWindow *)window {
    id delegate = [window delegate];
    return (delegate && [delegate isKindOfClass:[Document class]]) ? delegate : nil;
}

/* Return an existing document...
*/
+ (Document *)documentForPath:(NSString *)filename {
    NSArray *windows = [[NSApplication sharedApplication] windows];
    unsigned cnt, numWindows = [windows count];
    filename = [self cleanedUpPath:filename];	/* Clean up the incoming path */
    for (cnt = 0; cnt < numWindows; cnt++) {
        Document *document = [Document documentForWindow:[windows objectAtIndex:cnt]];
	NSString *docName = [document documentName];	
	if (docName && [filename isEqual:[self cleanedUpPath:docName]]) return document;
    }
    return nil;
}

+ (unsigned)numberOfOpenDocuments {
    NSArray *windows = [[NSApplication sharedApplication] windows];
    unsigned cnt, numWindows = [windows count], numDocuments = 0;
    for (cnt = 0; cnt < numWindows; cnt++) {
        if ([Document documentForWindow:[windows objectAtIndex:cnt]]) numDocuments++;
    }
    return numDocuments;
}

/* Menu validation: Arbitrary numbers to determine the state of the menu items whose titles change. Speeds up the validation... Not zero. */   
#define TagForFirst 42
#define TagForSecond 43

void validateToggleItem(NSMenuItem *aCell, BOOL useFirst, NSString *first, NSString *second) {
    if (useFirst) {
        if ([aCell tag] != TagForFirst) {
            [aCell setTitleWithMnemonic:first];
            [aCell setTag:TagForFirst];
        }
    } else {
        if ([aCell tag] != TagForSecond) {
            [aCell setTitleWithMnemonic:second];
            [aCell setTag:TagForSecond];
        }
    }
}

/* Menu validation
*/
- (BOOL)validateMenuItem:(NSMenuItem *)aCell {
    SEL action = [aCell action];
    if (action == @selector(toggleRich:)) {
	validateToggleItem(aCell, [self isRichText], NSLocalizedString(@"&Make Plain Text", @"Menu item to make the current document plain text"), NSLocalizedString(@"&Make Rich Text", @"Menu item to make the current document rich text"));
        if (![[self firstTextView] isEditable] || [self hasSheet]) return NO;
    } else if (action == @selector(toggleReadOnly:)) {
	validateToggleItem(aCell, [self isReadOnly], NSLocalizedString(@"Allow Editing", @"Menu item to make the current document editable (not read-only)"), NSLocalizedString(@"Prevent Editing", @"Menu item to make the current document read-only"));
        if ([self hasSheet]) return NO;
    } else if (action == @selector(togglePageBreaks:)) {
        validateToggleItem(aCell, [self hasMultiplePages], NSLocalizedString(@"&Wrap to Window", @"Menu item to cause text to be laid out to size of the window"), NSLocalizedString(@"&Wrap to Page", @"Menu item to cause text to be laid out to the size of the currently selected page type"));
        if ([self hasSheet]) return NO;
    } else if (action == @selector(toggleHyphenation:)) {
        validateToggleItem(aCell, ([self hyphenationFactor] > 0.0), NSLocalizedString(@"Do not Allow Hyphenation", @"Menu item to disallow hyphenation in the document"), NSLocalizedString(@"Allow Hyphenation", @"Menu item to allow hyphenation in the document"));
        if (![[self firstTextView] isEditable] || [self hasSheet]) return NO;
    } else if (action == @selector(revert:)) {
        if (revertDocumentName == nil || [self hasSheet]) return NO;
    } else if (action == @selector(save:) || action == @selector(saveAs:) || action == @selector(saveTo:) || action == @selector(printDocument:) || action == @selector(doPageLayout:)) {
        // When in plain text mode, savepanel's delegate is the document object itself,
        // which causes the various menu items to be enabled. To prevent this, we validate
        // the menu items depending on whether we're already running a save panel or not...
        // Note that this variable is only set during the savepanel case, due to this delegation issue.
        if ([self hasSheet]) return NO;
    } 
    return YES;
}

/* Returns the directory of the main window; nil if the main window is not saved or there is no main window.
*/
+ (NSString *)directoryOfMainWindow {
    Document *doc = [Document documentForWindow:[NSApp mainWindow]];
    return (doc && [doc documentName]) ? [[doc documentName] stringByDeletingLastPathComponent] : nil;
}

@end


/* Returns the next available untitled document sequence number
*/
static int nextAvailableUntitledDocNumber(void) {
    static int num = 0;
    return ++num;
}

/* Returns the default padding on the left/right edges of text views
*/
float defaultTextPadding(void) {
    static float padding = -1;
    if (padding < 0.0) {
        NSTextContainer *container = [[NSTextContainer alloc] init];
        padding = [container lineFragmentPadding];
        [container release];
    }
    return padding;
}

/* Return a non-blank display name. If the display name is blank, currently returns last path component; should probably do better.
*/
NSString *displayName(NSString *path) {
    NSString *result = [[NSFileManager defaultManager] displayNameAtPath:path];
    if ([result length] == 0) {
        result = [path lastPathComponent];
        if ([result length] == 0) {
            result = path;
        }
    }
    return result;
}



@implementation Document (ScriptingSupport)

// Scripting support.

- (NSScriptObjectSpecifier *)objectSpecifier {
    NSArray *orderedDocs = [NSApp valueForKey:@"orderedDocuments"];
    unsigned theIndex = [orderedDocs indexOfObjectIdenticalTo:self];

    if (theIndex != NSNotFound) {
        NSScriptClassDescription *desc = (NSScriptClassDescription *)[NSScriptClassDescription classDescriptionForClass:[NSApplication class]];
        return [[[NSIndexSpecifier allocWithZone:[self zone]] initWithContainerClassDescription:desc containerSpecifier:nil key:@"orderedDocuments" index:theIndex] autorelease];
    } else {
        return nil;
    }
}

// We already have a textStorage() method implemented above.
- (void)setTextStorage:(id)ts {
    // ts can actually be a string or an attributed string.
    if ([ts isKindOfClass:[NSAttributedString class]]) {
        [[self textStorage] replaceCharactersInRange:NSMakeRange(0, [[self textStorage] length]) withAttributedString:ts];
    } else {
        [[self textStorage] replaceCharactersInRange:NSMakeRange(0, [[self textStorage] length]) withString:ts];
    }
}

- (id)coerceValueForTextStorage:(id)value {
    // We want to just get Strings unchanged.  We will detect this and do the right thing in setTextStorage().  We do this because, this way, we will do more reasonable things about attributes when we are receiving plain text.
    if ([value isKindOfClass:[NSString class]]) {
        return value;
    } else {
        return [[NSScriptCoercionHandler sharedCoercionHandler] coerceValue:value toClass:[NSTextStorage class]];
    }
}

// Since TextEdit's Document class does not (currently) subclass NSDocument, we must support all the NSDocument class keys and custom command handling by hand.  Fortunately this is pretty easy.

// NSDocument's fileName key is the same as our documentName key.
- (NSString *)fileName {
    return [self documentName];
}
    
- (void)setFileName:(NSString *)name {
    [self setDocumentName:name];
}

- (NSString *)lastComponentOfFileName {
    return [[self documentName] lastPathComponent];
}
    
- (void)setLastComponentOfFileName:(NSString *)str {
    NSString *docName = [self documentName];
    NSString *dirPath;

    if ((str == nil) || [str isEqualToString:@""]) {
        // MF:??? Raise?
        return;
    }

    if ((docName == nil) || [docName isEqualToString:@""]) {
        // MF:??? Is this a good default?  Could we get the default save panel location somehow?
        dirPath = NSHomeDirectory();
    } else {
        dirPath = [docName stringByDeletingLastPathComponent];;
    }
    docName = [docName stringByAppendingPathComponent:str];
    [self setDocumentName:docName];
}

- (id)handleSaveScriptCommand:(NSScriptCommand *)command {
    NSDictionary *args = [command evaluatedArguments];
    NSString *file = [args objectForKey:@"File"];

    if (file != nil) {	// We're being given a file name
	BOOL isDir;
	(void)[[NSFileManager defaultManager] fileExistsAtPath:file isDirectory:&isDir];
	// TN2016: "Save in with an unsaved file acts like Save As; save in with an already-saved file acts like Save A Copy."
        if ([self documentName] != nil) {
	    // If the "in" parameter refers to a folder, put the file in the folder
	    if (isDir && ![[file pathExtension] isEqual:@"rtfd"]) file = [file stringByAppendingPathComponent:[[self documentName] lastPathComponent]];
	    [self saveDocument:NO name:file rememberName:NO shouldClose:NO whenDone:NULL];
	} else {
	    if (isDir && ![[file pathExtension] isEqual:@"rtfd"]) { 	    // If the "in" parameter refers to a folder, put the file in the folder, but ask for a file name
		file = [file stringByAppendingPathComponent:[self untitledDocumentName:UnknownStringEncoding]];
		[self saveDocument:YES name:file rememberName:YES shouldClose:NO whenDone:NULL];
	    } else {
		[self saveDocument:NO name:file rememberName:YES shouldClose:NO whenDone:NULL];
	    }
	}
    } else {	    	// No file name supplied
	// TN2016: "Save without 'in' acts like the Save menu command, and may ask the user for a file location"
        if ([self documentName] != nil) {
            // If the document has a filename, use it.
            [self save:nil];
        } else {
            // For now we just always run the save panel.
            [self saveAs:nil];
        }
    }
    return nil;
}

- (id)handleCloseScriptCommand:(NSCloseCommand *)command {
    NSDictionary *args = [command evaluatedArguments];
    NSString *file = [args objectForKey:@"File"];
    NSSaveOptions saveOptions = [command saveOptions];

    if (saveOptions == NSSaveOptionsAsk) {
        if (file != nil) {
            // If we were given a file name to save to, just do it.
            [self setDocumentName:file];
            [self save:nil];

            // Only close if we saved.
            if (![self isDocumentEdited]) {
                [self close:nil];
            }
        } else {
            // If we're dirty, ask if we should save before closing.
            if ([self canCloseDocument]) {
                [self close:nil];
            }
        }
    } else if (saveOptions == NSSaveOptionsYes) {
        // Save before closing.
        if (file != nil) {
            [self setDocumentName:file];
            [self save:nil];
            [self close:nil];
        } else {
            // Only save if we have a file name.  Only close if we saved.
            if ([self documentName] != nil) {
                [self save:nil];
                if (![self isDocumentEdited]) {
                    [self close:nil];
                }
            }
        }
    } else {
        // Don't save, just close.
        [self close:nil];
    }
    return nil;
}

- (id)handlePrintScriptCommand:(NSScriptCommand *)command {
    // This should eventually pay attention to the interaction level for scripting.  For now it always shows the panels.
    [self printDocumentModally:YES];
    return nil;
}

@end
