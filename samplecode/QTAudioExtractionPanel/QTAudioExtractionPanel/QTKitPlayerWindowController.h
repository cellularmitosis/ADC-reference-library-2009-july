//
//  QTKitPlayerWindowController.h
//  QTKitPlayer
//
//  Created by Sayli B Tiger on 5/14/05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

@interface QTKitPlayerWindowController : NSWindowController {
	
	IBOutlet NSWindow		*mMovieWindow;
    IBOutlet QTMovieView	*mMovieView;

        // export
    IBOutlet NSView			*mExportAccessoryView;
    IBOutlet NSPopUpButton	*mExportTypePopUpButton;

		// movie attributes drawer
    IBOutlet NSDrawer		*mDrawer;

		// movie attributes drawer elements
    IBOutlet NSTextField	*mCurrentSize;
    IBOutlet NSTextField	*mDuration;
    IBOutlet NSTextField	*mNormalSize;
    IBOutlet NSTextField	*mSourceName;
    IBOutlet NSTextField	*mTimeDisplay;
	
        // movie document
    QTMovie					*mMovie;

}
- (id)drawer;
- (QTMovie *)movie;
- (QTMovieView *) movieView;

-(void) windowWillGoAway:(NSNotification*) notification;


    // actions
- (IBAction)doExport:(id)sender;
- (IBAction)doSetFillColourPanel:(id)sender;
- (IBAction)doSetFillColour:(id)sender;
- (IBAction)doShowController:(id)sender;
- (IBAction)doPreserveAspectRatio:(id)sender;
- (IBAction)doLoop:(id)sender;
- (IBAction)doLoopPalindrome:(id)sender;
- (IBAction)doClone:(id)sender;

- (IBAction)toggleDrawer:(id)sender;

 // methods
- (void)setDurationDisplay;
- (void)setNormalSizeDisplay;
- (void)setCurrentSizeDisplay;
- (void)setSource:(NSString *)name;
- (void)setTimeDisplay;


@end
