
/*
File:			MovieDocument.m

Abstract:		Demonstrates how to build and extend a QuickTime media player 
				application, using the new QTKit framework available in QuickTime 7.
			  

Version:		1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/


#import "MovieDocument.h"
#import <QTKit/QTKit.h>

#pragma mark ********* defines *********

#define kDefaultWidthForNonvisualMovies	320


@implementation MovieDocument

// Class methods ---

+ (id)movieDocumentWithMovie:(QTMovie *)movie
{
    return [[(MovieDocument *)[self alloc] initWithMovie:movie] autorelease];
}


// Inits ---

-(void)awakeFromNib
{
	
}

- (id)initWithMovie:(QTMovie *)movie
{
    [super init];

    // init
    [self setFileType:@"MovieDocument"];
    [self setMovie:movie];

    if (mMovie == nil)
    {
        [self release];
        self = nil;
    }

    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

	[mMovieView setMovie:nil];
    [self setMovie:nil];

    [super dealloc];
}


#pragma mark ********* NSMenu validation protocol *********

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    BOOL	valid = NO;
    SEL		action;

    // init
    action = [menuItem action];

    // validate
    if (action == @selector(doSetFillColorPanel:))
        valid = YES;
		
    else if (action == @selector(doShowController:))
    {
        [menuItem setState:([mMovieView isControllerVisible] ? NSOnState : NSOffState)];
        valid = YES;
    }
    else if (action == @selector(doPreserveAspectRatio:))
    {
        [menuItem setState:([mMovieView preservesAspectRatio] ? NSOnState : NSOffState)];
        valid = (mMovie != nil);
    }
    else if ((action == @selector(doLoop:)) && mMovie)
    {
        [menuItem setState:([[mMovie attributeForKey:QTMovieLoopsAttribute] boolValue] ? NSOnState : NSOffState)];
        valid = YES;
    }
    else if ((action == @selector(doLoopPalindrome:)) && mMovie)
    {
        if ([[mMovie attributeForKey:QTMovieLoopsAttribute] boolValue])
        {
            [menuItem setState:([[mMovie attributeForKey:QTMovieLoopsBackAndForthAttribute] boolValue] ? NSOnState : NSOffState)];
            valid = YES;
        }
    }
   
    else if (action == @selector(doClone:))
        valid = (mMovie != nil);

    else if (action == @selector(selectAll:))
        valid = (mMovie != nil);

    else if (action == @selector(setSelectionToNone:))
        valid = (mMovie != nil);
   
    else
        valid = [super validateMenuItem:menuItem];

    return valid;
}

#pragma mark ********* Setters **************

// Setters ---

- (void)setMovie:(QTMovie *)movie
{
    [movie retain];
    [mMovie release];
    mMovie = movie;
}


- (void)sizeWindowToMovie:(NSNotification *)notification
{
    NSRect	currWindowBounds, newWindowBounds;
    NSPoint	topLeft;
    static BOOL	nowSizing = NO;
    
    if (nowSizing)
        return;
        
    nowSizing = YES;
    
    // get the current location and size of the movie window, so we can keep the top-left corner pegged, i.e. fixed
    currWindowBounds = [[mMovieView window] frame];
    topLeft.x = currWindowBounds.origin.x;
    topLeft.y = currWindowBounds.origin.y + currWindowBounds.size.height;

    NSSize contentSize = [[mMovie attributeForKey:QTMovieCurrentSizeAttribute] sizeValue];
    if ([mMovieView isControllerVisible])
	{
		// adjust for movie controller height
        contentSize.height += [mMovieView controllerBarHeight];
		// take into account size difference between movie view
		// and enclosing window
		contentSize.width += (currWindowBounds.size.width - contentSize.width);
	}

    if (contentSize.width == 0)
        contentSize.width = currWindowBounds.size.width;
	
    newWindowBounds = [[mMovieView window] frameRectForContentRect:NSMakeRect(0, 0, contentSize.width, contentSize.height)];

    [[mMovieView window] setFrame:NSMakeRect(topLeft.x, topLeft.y - newWindowBounds.size.height, newWindowBounds.size.width, newWindowBounds.size.height) display:YES];
	    
    nowSizing = NO;

}

#pragma mark ********* NSDocument overrides **************

// NSDocument overrides ---

- (NSString *)windowNibName
{
    return @"MovieDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)windowController
{
    // set up the movie view
	if (mMovie)
    {
        NSSize contentSize = [[mMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
        [mMovieView setMovie:mMovie];
        if ([mMovieView isControllerVisible])
        {
            contentSize.height += [mMovieView movieControllerBounds].size.height;
            if (contentSize.width == 0)
            {
                contentSize.width = kDefaultWidthForNonvisualMovies;
            }
        }
        [[mMovieView window] setContentSize:contentSize];
    }
    else
        [mMovieView setMovie:[QTMovie movie]];

	// set the editable state
	[mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
	
    // watch for movie resizings
    if (mMovie)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(sizeWindowToMovie:) 
													 name:QTMovieSizeDidChangeNotification 
												   object:mMovie];
	}
}

- (BOOL)writeWithBackupToFile:(NSString *)fullDocumentPath ofType:(NSString *)type saveOperation:(NSSaveOperationType)saveOperation
{
    BOOL success = NO;

    // update/save
    if (saveOperation == NSSaveOperation)
        success = [mMovie updateMovieFile];
    else
        success = [super writeWithBackupToFile:fullDocumentPath ofType:type saveOperation:saveOperation];

    return success;
}

- (NSData *)dataRepresentationOfType:(NSString *)docType
{
    return [mMovie movieFormatRepresentation];
}

- (BOOL)readFromFile:(NSString *)fileName ofType:(NSString *)type
{
    BOOL	success = NO;

    // read the movie
    if ([QTMovie canInitWithFile:fileName])
    {
        if ([type isEqualTo:@"MovieDocumentData"])
        {
            NSData	*data = [NSData dataWithContentsOfFile:fileName];
			[self setMovie:[QTMovie movieWithData:data error:nil]];
        }
        else
        {
            [self setMovie:((QTMovie *)[QTMovie movieWithFile:fileName error:nil])];
        }
		
		success = (mMovie != nil);

    }

    return success;
}



- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename 
{
    BOOL isDir = NO;

    [[NSFileManager defaultManager] fileExistsAtPath:filename isDirectory:&isDir];

    if (isDir)
    {
        return YES;
    }
    else
    {
        return [QTMovie canInitWithFile:filename];
    }
}


- (void)printShowingPrintPanel:(BOOL)showPanels
{
    NSPrintOperation *printOperation;

    // init
    printOperation = [NSPrintOperation printOperationWithView:mMovieView printInfo:[self printInfo]];
    [printOperation setShowPanels:showPanels];

    // print
    [self runModalPrintOperation:printOperation delegate:nil didRunSelector:nil contextInfo:nil];
}


#pragma mark ********* actions **************

// Actions ---


- (IBAction)doSetFillColorPanel:(id)sender
{
    NSColorPanel *colorPanel;

    // init
    colorPanel = [NSColorPanel sharedColorPanel];
    [colorPanel setAction:@selector(doSetFillColor:)];
    [colorPanel setTarget:self];
    [colorPanel setColor:[mMovieView fillColor]];

    // run the panel
    [colorPanel makeKeyAndOrderFront:nil];
}

- (IBAction)doSetFillColor:(id)sender
{
    // update the fill color
    [mMovieView setFillColor:[sender color]];
}

- (IBAction)doShowController:(id)sender
{
    // toggle the controller visibility
    [mMovieView setControllerVisible:([sender state] == NSOffState)];
}

- (IBAction)doPreserveAspectRatio:(id)sender
{
    // toggle the aspect ratio preservation
    [mMovieView setPreservesAspectRatio:([sender state] == NSOffState)];
}

- (IBAction)doLoop:(id)sender
{
    // toggle looping
    [mMovie setAttribute:[NSNumber numberWithBool:([sender state] == NSOffState)] forKey:QTMovieLoopsAttribute];
}

- (IBAction)doLoopPalindrome:(id)sender
{
    // toggle palindrome looping
    [mMovie setAttribute:[NSNumber numberWithBool:([sender state] == NSOffState)] forKey:QTMovieLoopsBackAndForthAttribute];
}

- (IBAction)doClone:(id)sender
{
    MovieDocument *movieDocument;

    // init
    movieDocument = [MovieDocument movieDocumentWithMovie:[[mMovie copy] autorelease]];

    // set up the document
    if (movieDocument)
    {
        // add the document
        [[NSDocumentController sharedDocumentController] addDocument:movieDocument];

        // set up the document
        [movieDocument makeWindowControllers];
        [movieDocument showWindows];
    }
}


@end

