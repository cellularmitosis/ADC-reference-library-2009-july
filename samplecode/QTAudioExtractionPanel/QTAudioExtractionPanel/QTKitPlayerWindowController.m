//
//  QTKitPlayerWindowController.m
//  QTKitPlayer
//
//  Created by Sayli B Tiger on 5/14/05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import "QTKitPlayerWindowController.h"
#import "MovieDocument.h"
#import "AudioPropInfo.h"


@implementation QTKitPlayerWindowController

- (id) init
{
	self = [super initWithWindowNibName:@"MovieWindow"];
	return self;
}

- (void) dealloc
{
	//remove observers here?
	NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
	[center removeObserver:self name:NSWindowDidBecomeMainNotification object:nil];
	[center removeObserver:self name:NSWindowDidResignMainNotification object:nil];
	[center removeObserver:self name:NSWindowWillCloseNotification object:nil];
	[super dealloc];
}


- (id)drawer
{
    return mDrawer;
}
- (QTMovie *)movie 
{
	return mMovie;
}
-(QTMovieView *) movieView
{
	return mMovieView;
}

- (void)windowDidLoad
{
	NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
	
	if ([self window] != nil)
	{
		[center addObserver:self selector:@selector(mainWindowChanged:) name:NSWindowDidBecomeMainNotification object:[self window]];
		[center addObserver:self selector:@selector(mainWindowChanged:) name:NSWindowDidResignMainNotification object:[self window]];
		//[center addObserver:self selector:@selector(mainWindowChanged:) name:NSWindowWillMiniaturizeNotification object:[self window]];
		[center addObserver:self selector:@selector(windowWillGoAway:) name:NSWindowWillCloseNotification object:[self window]];

	}
}


+ (void) setInfoForNextMovie
{
	MovieDocument *doc;
	NSEnumerator *enumerator = [[[NSDocumentController sharedDocumentController] documents] objectEnumerator];
	while (doc = (MovieDocument*) [enumerator nextObject]) {
	  if ([doc movie] && [[AudioPropInfo audioPropInfo] isVisible])
	  {
		// associate next movie with Audio Prop Info window
		[[AudioPropInfo audioPropInfo] resetInfoPanel:NO];
		[[AudioPropInfo audioPropInfo] setMovie:(QTMovie*) [doc movie]];
		[[AudioPropInfo audioPropInfo] setMovieDocument:doc];
		[[AudioPropInfo audioPropInfo] rebuildInfoPanel];

		break;
	  }
	}
}

-(void) windowWillGoAway:(NSNotification*) notification
{
	if (![NSApp isActive] && [[[NSDocumentController sharedDocumentController] documents] count] >= 1)
	{
	  [QTKitPlayerWindowController setInfoForNextMovie];
	}
	
	NSLog (@"The main window is going away");
	if ([[[NSDocumentController sharedDocumentController] documents] count] == 0)
	{
		// set the Movie Info movie to NULL
		[[AudioPropInfo audioPropInfo] resetInfoPanel:NO];
		[[AudioPropInfo audioPropInfo] setMovie:nil];
		[[AudioPropInfo audioPropInfo] setMovieDocument:nil];
		[[AudioPropInfo audioPropInfo] rebuildInfoPanel];
	}
}

-(void) mainWindowChanged:(NSNotification*) notification
{
    NSString    *notificationName;
	QTMovie*	movie = [(MovieDocument*)[self document] movie];
	
    notificationName = [notification name];

    NSLog (@"The main window has changed");
	if ([[AudioPropInfo audioPropInfo] isVisible] && [notificationName isEqualTo:NSWindowDidBecomeMainNotification])
	{
		if (![movie isEqualTo:[[AudioPropInfo audioPropInfo] movie]])
		{
			[[AudioPropInfo audioPropInfo] resetInfoPanel:NO];
			[[AudioPropInfo audioPropInfo] setMovie:movie];
			[[AudioPropInfo audioPropInfo] setMovieDocument:[self document]];
			[[AudioPropInfo audioPropInfo] rebuildInfoPanel];
		}
	}
}



// Actions ---

- (IBAction)doExport:(id)sender
{
    NSSavePanel *savePanel;

    // init
    savePanel = [NSSavePanel savePanel];

    // run the export sheet
    [savePanel setAccessoryView:mExportAccessoryView];
    [savePanel beginSheetForDirectory:nil file:[[self fileName] lastPathComponent] modalForWindow:mMovieWindow modalDelegate:self
        didEndSelector:@selector(exportPanelDidEnd: returnCode: contextInfo:) contextInfo:nil];
}

- (IBAction)doSetFillColourPanel:(id)sender
{
    NSColorPanel *colorPanel;

    // init
    colorPanel = [NSColorPanel sharedColorPanel];
    [colorPanel setAction:@selector(doSetFillColour:)];
    [colorPanel setTarget:self];
    [colorPanel setColor:[mMovieView fillColor]];

    // run the panel
    [colorPanel makeKeyAndOrderFront:nil];
}

- (IBAction)doSetFillColour:(id)sender
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

//////////
//
// toggleDrawer:
// Toggle the drawer state.
//
//////////

- (IBAction)toggleDrawer:(id)sender
{
	[mDrawer toggle:sender];
}


//////////
//
// setTimeDisplay
// Set the Current Time text field.
//
//////////

- (void)setTimeDisplay
{
	if (mMovie)
	{
		QTTime currentPlayTime = [[mMovie attributeForKey:QTMovieCurrentTimeAttribute] QTTimeValue];
		[mTimeDisplay setStringValue:QTStringFromTime(currentPlayTime)];
	}
}


//////////
//
// setDuration
// Set the Duration text field.
//
//////////

- (void)setDurationDisplay
{
	if (mMovie)
	{
		if ([mMovie attributeForKey:QTMovieHasDurationAttribute])
		{
			NSString *durStr = QTStringFromTime([[mMovie attributeForKey:QTMovieDurationAttribute] QTTimeValue]);
			if (durStr)
				[mDuration setStringValue:durStr];
			
		}
	}
}


//////////
//
// setNormalSize
// Set the Normal Size text field.
//
//////////

- (void)setNormalSizeDisplay
{
    NSMutableString *sizeString = [NSMutableString string];
	NSSize movSize = NSMakeSize(0,0);
	movSize = [[mMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
	
    [sizeString appendFormat:@"%.0f", movSize.width];
    [sizeString appendString:@" x "];
    [sizeString appendFormat:@"%.0f", movSize.height];

    [mNormalSize setStringValue:sizeString];
}


//////////
//
// setCurrentSize
// Set the Current Size text field.
//
//////////

- (void)setCurrentSizeDisplay
{
	NSSize movCurrentSize = NSMakeSize(0,0);
	movCurrentSize = [[mMovie attributeForKey:QTMovieCurrentSizeAttribute] sizeValue];
    NSMutableString *sizeString = [NSMutableString string];
    
	if (mMovie && [mMovieView isControllerVisible])
        movCurrentSize.height -= [mMovieView controllerBarHeight];

    [sizeString appendFormat:@"%.0f", movCurrentSize.width];
    [sizeString appendString:@" x "];
    [sizeString appendFormat:@"%.0f", movCurrentSize.height];
	
    [mCurrentSize setStringValue:sizeString];
}



//////////
//
// setSource:
// Set the Source text field.
//
//////////

- (void)setSource:(NSString *)name
{
    NSArray *pathComponents = [[NSFileManager defaultManager] componentsToDisplayForPath:name];
    NSEnumerator *pathEnumerator = [pathComponents objectEnumerator];
    NSString *component = [pathEnumerator nextObject];
    NSMutableString *displayablePath = [NSMutableString string];
 
    while (component != nil) {
        if ([component length] > 0) {
            [displayablePath appendString:component];
            
            component = [pathEnumerator nextObject];
            if (component != nil)
                [displayablePath appendString:@":"];
        } else {
            component = [pathEnumerator nextObject];
        }
    }

	[mSourceName setStringValue:displayablePath];
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

	[self setCurrentSizeDisplay];
    
    nowSizing = NO;

}


@end
