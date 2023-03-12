 /*

File: MovieInspectorController.m

Abstract: This file contains methods which implement an Inspector
               window for a given movie, showing details about the movie
	     such as size, duration, full file path and more.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/


#import "MovieInspectorController.h"
#import "MovieDocument.h"
#import "QTKitPlayerAppDelegate.h"


@implementation MovieInspectorController

#pragma mark Initialization
-(id)init
{
	self = [super initWithWindowNibName:@"Inspector"];
	
	mMovie = nil;

	// we want to be notified when our window
	// is closing so we can do cleanup
	[ [NSNotificationCenter defaultCenter]
	   addObserver:self selector:@selector(closeWin:)
	   name:NSWindowWillCloseNotification object:mInspectorPanel ];

	return self;
}

- (void)dealloc
{
	if (mMovie)
	{
		[mMovie release];
	}

	[super dealloc];
}

#pragma mark Window Handling

//////////
//
// doShowWindow
// Bring the window to the front and show it.
//
//////////

-(void)doShowWindow
{
	[self showWindow:self];
	[mInspectorPanel makeKeyAndOrderFront:self];
}

//////////
//
// closeWin
// Called when our window is closed so we can
// perform cleanup.
//
//////////

- (void)closeWin:(NSNotification *)notification
{
	// did our inspector window close?
	if ([notification object] == mInspectorPanel)
	{
		// tell App. delegate the inspector window has closed
		[[NSApp delegate] inspectorWindowHasClosed];
	}
}

//////////
//
// getMovieForCurrentDocument
// Return the QTMovie instance for the current movie document.
//
//////////

- (QTMovie *)getMovieForCurrentDocument
{
	NSDocumentController *documentController = [NSDocumentController sharedDocumentController];
	MovieDocument *currDoc = [documentController currentDocument];
	if (currDoc)
	{
		return ([currDoc qtmovie]);
	}

	return nil;
}

//////////
//
// updateInspectorWindowForMovie
// Update movie attributes in the Inspector window for the specified movie.
//
//////////

- (void) updateInspectorWindowForMovie:(QTMovie *)inMovie
{
	// save new movie
	[mMovie release];
	mMovie = inMovie;
	[mMovie retain];

	// display attributes for movie
	[self displayMovieDuration];
	[self displayMovieNormalSize];
	[self setCurrentSizeDisplay];
	[self setWindowTitleForMovie];
	[self displayMovieTrackFormat];
	[self setDisplayName];
}

//////////
//
// updateInspectorWindowForCurrentMovie
// Update movie attributes in the Inspector window for the current movie.
//
//////////

- (void) updateInspectorWindowForCurrentMovie
{
	// get QTMovie for current movie document
	QTMovie *currentMovie = [self getMovieForCurrentDocument];
	
	if (currentMovie)
	{
		// update Inspector for current movie
		[self updateInspectorWindowForMovie:currentMovie];
	}
}


#pragma mark Display Routines

//////////
//
// setWindowTitleForMovie
// Set the Window title to the Movie name.
//
//////////

- (void)setWindowTitleForMovie
{
	if (mMovie)
	{
		if ([mMovie attributeForKey:QTMovieDisplayNameAttribute])
		{
			NSString *nameStr = [mMovie attributeForKey:QTMovieDisplayNameAttribute];
			if (nameStr)
			{
				NSMutableString *finalStr = [NSMutableString stringWithString:nameStr];
				[finalStr appendString:@" - Inspector"];
				[mInspectorPanel setTitle:finalStr];
			}
		}
	}
}

//////////
//
// setDisplayName
// Set the Movie path name in the Inspector window.
//
//////////

- (void)setDisplayName
{
	if (mMovie)
	{
		if ([mMovie attributeForKey:QTMovieURLAttribute])
		{
			NSURL *movieURL = [mMovie attributeForKey:QTMovieURLAttribute];
			if (movieURL)
			{
				[mDisplayName setStringValue:[movieURL path]];
			}
		}
	}
}

//////////
//
// displayMovieDuration
// Set the Movie duration text field in the Inspector window.
//
//////////

- (void)displayMovieDuration
{
	if (mMovie)
	{
		if ([mMovie attributeForKey:QTMovieHasDurationAttribute])
		{
			NSString *durStr = QTStringFromTime([[mMovie attributeForKey:QTMovieDurationAttribute] QTTimeValue]);
			if (durStr)
			{
				[mDuration setStringValue:durStr];
			}
		}
	}
}

//////////
//
// displayMovieTrackFormat
// Set the Movie track format summary field in the Inspector window.
//
//////////

- (void)displayMovieTrackFormat
{
	if (mMovie)
	{
		[mFormat setStringValue:@""];
		
		NSMutableString *trackFormatStr = [NSMutableString string];
		
		NSArray *tracksArray = [mMovie tracks];
		int i, tracks = [tracksArray count];
		for (i=0; i<tracks; ++i)
		{
			QTTrack *track = [tracksArray objectAtIndex:i];
			if ([track attributeForKey:QTTrackFormatSummaryAttribute] && ([track isEnabled] == YES))
			{
				NSString *formatStr = [track attributeForKey:QTTrackFormatSummaryAttribute];
				if (formatStr)
				{
					[trackFormatStr appendFormat:@"(Track %i) ", i];
					[trackFormatStr appendString:formatStr];
					[trackFormatStr appendString:@" "];
				}
			}
		}
		
		[mFormat setStringValue:trackFormatStr];
	}
}



//////////
//
// displayMovieNormalSize
// Set the Movie Normal Size text field in the Inspector window.
//
//////////

- (void)displayMovieNormalSize
{
	if (mMovie)
	{
		NSMutableString *sizeString = [NSMutableString string];
		NSSize movSize = NSMakeSize(0,0);
		movSize = [[mMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];

		[sizeString appendFormat:@"%.0f", movSize.width];
		[sizeString appendString:@" x "];
		[sizeString appendFormat:@"%.0f", movSize.height];

		[mNormalSize setStringValue:sizeString];
	}
}

//////////
//
// setCurrentSize
// Set the Movie Current Size text field in the Inspector window.
//
//////////

- (void)setCurrentSizeDisplay
{
	if (mMovie)
	{
		NSSize movCurrentSize = NSMakeSize(0,0);
		movCurrentSize = [[mMovie attributeForKey:QTMovieCurrentSizeAttribute] sizeValue];
		NSMutableString *sizeString = [NSMutableString string];

		[sizeString appendFormat:@"%.0f", movCurrentSize.width];
		[sizeString appendString:@" x "];
		[sizeString appendFormat:@"%.0f", movCurrentSize.height];
		[sizeString appendString:@" (Actual)"];

		[mCurrentSize setStringValue:sizeString];
	}
}

@end
