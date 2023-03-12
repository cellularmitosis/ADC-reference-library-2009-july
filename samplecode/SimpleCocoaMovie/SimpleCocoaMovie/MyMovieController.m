//////////
//
//	File:		MyMovieController.m
//
//	Contains:	Implementation file for the MyMovieController class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	6/18/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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

//////////
//
// header files
//
//////////

#import "MyMovieController.h"
#include <QuickTime/QuickTime.h>
#include "QTUtils.h"

//////////
//
// globals
//
//////////

static void * gMyMovieControllerObject;

//////////
//
// implementation
//
//////////

@implementation MyMovieController

//////////
//
// goToBeginning
//
// Called when the go to beginning button is pressed
// We call the NSMovieView gotoBeginning method here
// to go to the beginning of the movie
//
//////////

- (IBAction)goToBeginning:(id)sender
{
    /* save current movie state */
    [self saveCurrentMoviePlayingState];
    
    /* go to the beginning of the movie */
    [movieViewObject gotoBeginning:sender];
    
    /* restore movie state */
    [self restoreMoviePlayingState:sender];
}

//////////
//
// goToEnd
//
// Called when the go to beginning button is pressed
// We call the NSMovieView goToEnd method here to go
// to the end of the movie
//
//////////

- (IBAction)goToEnd:(id)sender
{
    /* go to the end of the movie */
    [movieViewObject gotoEnd:sender];
}


//////////
//
// play
//
// Called when the play button is pressed.
// We call the NSMovieView start method to
// play the movie
//
//////////

- (IBAction)play:(id)sender
{
    /* has the movie just finished playing? */
    if (IsMovieDone(gMovie))
    {
        /* yes, so go to the beginning */
        [movieViewObject gotoBeginning:sender];
    }

    /* is the movie currently playing? */
    if ([movieViewObject isPlaying])
    {
        /* yes, so stop it */
        [movieViewObject stop:sender];
    }
    else	/* movie is not currently playing */
    {
        /* go ahead and start it playing */
        [movieViewObject start:sender];
    }
    
    /* change the button text to "pause" */
    [sender setTitle:@"||"];
    
    /* set the button action method so the next
        time it is invoked, our "stop" method
        will instead be called */
    [sender setAction:@selector(stop:)];
}


//////////
//
// stop
//
// Called when the stop button is pressed.
// We call the NSMovieView stop method to
// stop the movie
//
//////////

- (IBAction)stop:(id)sender
{
    /* stop the movie */
    [movieViewObject stop:sender];
    /* adjust the play button state */
    [self resetPlayButtonForMovieStopState:sender];
}

//////////
//
// setVolume
//
// Called when the volume slider is pressed.
// We call the Movie toolbox SetMovieVolume
// function method to adjust the movie volume.
//
//////////

- (IBAction)setVolume:(id)sender
{
    /* set the movie volume to correspond to the
        current value of the slider */
    SetMovieVolume([[movieViewObject movie] QTMovie],[setVolumeSlider floatValue]);
}

//////////
//
// stepBack
//
// Called when the step-back button is pressed.
// We call the NSMovieView function setpBack
// to step the movie.
//
//////////

- (IBAction)stepBack:(id)sender
{
    /* save the current movie play state */
    [self saveCurrentMoviePlayingState];

    /* step the movie back */
    [movieViewObject stepBack:sender];

    /* restore the movie play state */
    [self restoreMoviePlayingState:sender];
}

//////////
//
// stepForward
//
// Called when the step-forward button is pressed.
// We call the NSMovieView function stepForward
// to step the movie.
//
//////////

- (IBAction)stepForward:(id)sender
{
    /* has the movie just finished playing? */
    if (IsMovieDone(gMovie))
    {
        /* yes, so go to beginning */
        GoToBeginningOfMovie(gMovie);
    }

    /* save the current play state */
    [self saveCurrentMoviePlayingState];

    /* step the movie forward */
    [movieViewObject stepForward:sender];

    /* restore the movie play state */
    [self restoreMoviePlayingState:sender];
}

//////////
//
// resetPlayButtonForMovieStopState
//
// This method adjusts the play button text
// and action method so the next time it
// is pressed the "play" method is invoked
//
//////////

- (void)resetPlayButtonForMovieStopState:(id)sender
{
    /* reset button text to "play" text */
    [playButton setTitle:@">"];
    /* reset button action method to our "play" method
        so the next time the button is pressed this
        method will instead be invoked */
    [playButton setAction:@selector(play:)];

}

//////////
//
// showTheWindow
//
// This method is used to make a window
// visible and bring it to the front
//
//////////

- (void)showTheWindow:(NSWindow *)window
{
    /* is the window visible? */
    if (![window isVisible])
    {
        /* window is not visible, so let's make it visible */
        [window setFrame:[window frame] display:YES];
    }
    /* make the window accept keys, and make it the
        front window */
    [window makeKeyAndOrderFront:menuItem_New];

}

//////////
//
// movieProperties
//
// Called when the Properties menu item in the Movie
// menu is selected. We use the NSWindow class methods
// to show the movie properties window.
//
//////////

- (IBAction)movieProperties:(id)sender
{
    /* display the movie properties window */
    [self showTheWindow:moviePropertiesWindow];
}

//////////
//
// newWindow
//
// Called when the "New" menu item in the file
// menu is selected. We simply display the 
// window by making it visible and bringing it
// to the front.
//
//////////

- (IBAction)newWindow:(id)sender
{
    [self showTheWindow:movieWindow];
}

//////////
//
// awakeFromNib
//
// Called after all our objects are unarchived and
// connected but just before the interface is made visible
// to the user. We will do custom initialization of our
// objects here.
//
//////////

- (void)awakeFromNib
{
    /* retrieve our movie file from our bundle resource */
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"MovieFile" ofType:@"mov"];
    NSURL *movieUrl = [NSURL fileURLWithPath:filePath];
    /* create an NSMovie object from our QuickTime movie */
    NSMovie *theMovie = [[NSMovie alloc] initWithURL:movieUrl byReference:YES];
    /* retrieve the QuickTime-style movie (type "Movie" from QuickTime/Movies.h) */
    Movie qtmovie = [theMovie QTMovie];
    
    /* save (for later use) the QuickTime-style movie type for our NSMovie */
    gMovie = qtmovie;

    /* assign the NSMovie object (which we made above) for our NSMovieView object */
    [movieViewObject setMovie: theMovie];

    /* set the high/low ranges for our volume slider */
    [setVolumeSlider setMaxValue: 256];
    [setVolumeSlider setMinValue: 0];

    [menuItem_New setEnabled:NSOffState];
    
    /* we must initialize the movie toolbox before calling
        any of it's functions */
    EnterMovies();

    /* save a copy of the MyMovieController object so we can later call methods
        in the class from _outside_ the actual class implementation */
    saveObjectForCallback(self);
    
    /* setup a callback for our movie so QuickTime will call us when the
        movie finishes playing */
    gMoviePlayingCompleteCallBack =  NewQTCallBackUPP(&MyCallBackProc);
	[self setupMoviePlayingCompleteCallback:qtmovie callbackUPP:gMoviePlayingCompleteCallBack];
    
    /* set the volume slider to the current volume of our movie */
    [setVolumeSlider setFloatValue: GetMovieVolume(qtmovie)];
    
    /* the movie properties window will display miscellaneous
        properties for the movie - here we do the setup for
        all the controls in this window */
    [self setMoviePropertyWindowControlValues:qtmovie];
    
}

//////////
//
// setMoviePropertyWindowControlValues
//
// The movie properties window displays miscellaneous
// properties for the movie. Here we setup the values
// for each of the controls in our window.
//
//////////

- (void)setMoviePropertyWindowControlValues:(Movie)qtmovie
{
    if (QTUtils_IsAutoPlayMovie (qtmovie))
    {
        [autoPlayEnabled setStringValue:@"Yes"];
    }
    else
    {
        [autoPlayEnabled setStringValue:@"No"];
    }

    [trackCount setIntValue:GetMovieTrackCount(qtmovie)];

    [self buildTrackMediaTypesArray:qtmovie];
        
    [movieTimeScale setStringValue:GetMovieTimeScaleAsString(qtmovie)];
    [movieDuration setStringValue:GetMovieDurationAsString(qtmovie)];
}

//////////
//
// tableView
//
// Our movie properties window contains a NSTableView control.
// An NSTableView control must implement the routines
// defined in the NSTableDataSource protocol. This is our
// implementation of the tableView method for populating
// the NSTableView with data items.
//
//////////

- (id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
    row:(int)rowIndex
{
    /* turn editing off for each cell */
    [aTableColumn setEditable:NO];
    
    /* populate the list with data from our movie track media types array */
    return ([movieTrackMediaTypesArray objectAtIndex:rowIndex]);
}

//////////
//
// numberOfRowsInTableView
//
// Our movie properties window contains a NSTableView control.
// An NSTableView control must implement the routines
// defined in the NSTableDataSource protocol. This is our
// implementation of the numberOfRowsInTableView method which
// returns a count of the number of data items in the table.
//
//////////

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return [movieTrackMediaTypesArray count];
}

//////////
//
// init
//
// Our controller's initialization method
//
//////////

- (id)init
{
    /* allocate an array to store the track media types for
        our movie - these will be displayed in our NSTableView
        control in our movie properties window */
    movieTrackMediaTypesArray=[[NSMutableArray alloc] init];
    
    return self;
}

//////////
//
// dealloc
//
// Our controller's dealloc method. We'll dispose of
// any objects we created previously.
//
//////////

-(void)dealloc
{
    [movieTrackMediaTypesArray release];
    DisposeCallBack(gQtCallBack);
}

//////////
//
// buildTrackMediaTypesArray
//
// Build an array of track media types for our movie. These
// will be displayed in our NSTableView control in our
// movie properties window.
//
//////////

- (void)buildTrackMediaTypesArray:(Movie)qtmovie
{
    short i;
    
    for (i = 0; i < GetMovieTrackCount(qtmovie); ++i)
    {
		Str255			mediaName;
        OSErr			myErr;
        Track			movieTrack = GetMovieIndTrack(qtmovie, i+1); /* tracks start at index 1 */
        Media			trackMedia = GetTrackMedia(movieTrack);
        MediaHandler	trackMediaHandler = GetMediaHandler(trackMedia);
		
        // get the text of the media type
		myErr = MediaGetName(trackMediaHandler, mediaName, 0, NULL);

        // add the media type string to our array (as an NSString)
        [movieTrackMediaTypesArray insertObject:[NSString stringWithCString:&mediaName[1] length:mediaName[0]] atIndex:i];
    }

}

//////////
//
// saveCurrentMoviePlayingState
//
// Save the current movie playing state. This is
// necessary because certain operations (step
// forward, etc.) will stop the movie, and we
// want to preserve the state between these
// operations.
//
//////////

- (void)saveCurrentMoviePlayingState
{
    gIsMoviePlaying = [movieViewObject isPlaying];
    gMoviePlaybackRate = [movieViewObject rate];
}

//////////
//
// restoreMoviePlayingState
//
// Restore the current movie playing state. This is
// necessary because certain operations (step
// forward, etc.) will stop the movie, and we
// want to preserve the state between these
// operations.
//
//////////

- (void)restoreMoviePlayingState:(id)sender
{
    if (gIsMoviePlaying)
    {
        [movieViewObject start:sender];
    }
    [movieViewObject setRate:gMoviePlaybackRate];
}

//////////
//
// restoreMoviePlayCompleteCallBack
//
// Once our movie callback is called, QuickTime removes us from the
// callback chain - therefore, we must specify the callback
// again or QuickTime won't call us.
//
//////////

- (void)restoreMoviePlayCompleteCallBack
{
    [self setupMoviePlayingCompleteCallback:gMovie callbackUPP:gMoviePlayingCompleteCallBack];
}

//////////
//
// setupMoviePlayingCompleteCallback
//
// Here we specify QuickTime call us when the movie stops playing. This
// is necessary so we can adjust our play button to the proper settings.
//
//////////

- (void)setupMoviePlayingCompleteCallback:(Movie)theMovie callbackUPP:(QTCallBackUPP) callbackUPP
{
    TimeBase tb = GetMovieTimeBase (theMovie);
    OSErr err = noErr;
    
    gQtCallBack = NewCallBack (tb, callBackAtExtremes);
    if (gQtCallBack != NULL)
    {
        err = CallMeWhen (gQtCallBack, 
                        callbackUPP, 
                        NULL,			/* refCon */
                        triggerAtStop, 	/* flags - call us when stopped */
                        NULL, 			/* param2 - don't care */
                        NULL);			/* param3 - don't care */
    }
}

@end


//////////
//
// adjustPlayButtonForMyMovieControllerObject
//
// Once the movie has stopped playing, QuickTime will call
// us so we can adjust our play button settings.
//
//////////

void adjustPlayButtonForMyMovieControllerObject()
{
    [(MyMovieController *)gMyMovieControllerObject resetPlayButtonForMovieStopState:nil];
}

//////////
//
// MyCallBackProc
//
// This is our movie timebase callback routine. We'll tell
// QuickTime to call this routine whenever our movie has
// stopped playing.
//
//////////

pascal void MyCallBackProc (QTCallBack cb, long refcon)
{
    adjustPlayButtonForMyMovieControllerObject();
    [(MyMovieController *)gMyMovieControllerObject restoreMoviePlayCompleteCallBack];
}

//////////
//
// saveObjectForCallback
//
// This routine stores a reference to our MyMovieController object. We'll
// need this so we can call into methods in this class from outside the
// implementation of the class methods.
//
//////////

void saveObjectForCallback(void *theObject)
{
   gMyMovieControllerObject = theObject;
}


