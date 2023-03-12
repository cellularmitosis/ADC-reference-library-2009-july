//////////
//
//	File:		MyMovieController.java
//
//	Contains:	Implementation file for the MyMovieController class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	9/01/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
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

import com.apple.cocoa.foundation.*;
import com.apple.cocoa.application.*;

import quicktime.std.movies.Movie.*;
import quicktime.std.movies.media.*;
import quicktime.QTException.*;
import quicktime.QTSession.*;
import quicktime.std.clocks.*;

public class MyMovieController extends NSObject {

    private NSTimer 	updateButtonTimer;
    private boolean		isMoviePlaying;
    private float 		moviePlaybackRate;
    private NSSelector 	playSelector,stopSelector;
    
    private NSButton 	goToBeginningButton;
    private NSButton 	goToEndButton;
    private NSButton 	playButton;
    private NSButton 	stepBackButton;
    private NSButton 	stepForwardButton;
    private NSSlider 	setVolumeSlider;

    private NSWindow 	movieWindow;

    private NSMovieView 	movieViewObject;

    private NSMenuItem 	menuItem_New;

    //////////
    //
    // MyMovieController
    //
    // Default constructor for this class
    //
    //////////

    public MyMovieController()
    {
        super();
    }
    
    //////////
    //
    // goToBeginning
    //
    // Called when the go to beginning button is pressed
    // We call the NSMovieView gotoBeginning method here
    // to go to the beginning of the movie
    //
    //////////
    
    public void goToBeginning(Object sender) 
    {
            /* save current movie state */
        saveCurrentMoviePlayingState();
        
        /* go to the beginning of the movie */
        movieViewObject.gotoBeginning(sender);
        
        /* restore movie state */
        restoreMoviePlayingState(sender);
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
    
    public void goToEnd(Object sender)
    {
        /* go to the end of the movie */
        movieViewObject.gotoEnd(sender);
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
    
    public void newWindow(Object sender)
    {
        showTheWindow(movieWindow);
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
    
    public void stop(Object sender)
    {
        /* stop the movie */
        movieViewObject.stop(sender);
        
        /* adjust the play button state */
        resetPlayButtonForMovieStopState(sender);
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
    
    public void play(Object sender)
    {
        try {        
                /* is the movie currently playing? */
                if (movieViewObject.isPlaying())
                {
                    /* yes, so stop it */
                    movieViewObject.stop(sender);
                }
                else	/* movie is not currently playing */
                {
                    /* go ahead and start it playing */
                    movieViewObject.start(sender);
                    
                    /* change the button text to "pause" */
                    playButton.setTitle("||");
            }
            
            /* set the button action method so the next
                time it is invoked, our "stop" method
                will instead be called */
            playButton.setAction(stopSelector);
        } 
        catch (Exception e) 
        {
            e.printStackTrace();
        };
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
    
    public void setVolume(Object sender)
    {
        try 
        {
            /* set the movie volume to correspond to the
                current value of the slider */
            movieViewObject.setVolume(setVolumeSlider.floatValue());
        } 
        catch (Exception e) 
        {
            e.printStackTrace();
        };
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
    
    public void stepBack(Object sender)
    {
        /* save the current movie play state */
        saveCurrentMoviePlayingState();
    
        /* step the movie back */
        movieViewObject.stepBack(sender);
    
        /* restore the movie play state */
        restoreMoviePlayingState(sender);
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
    
    public void stepForward(Object sender)
    {
        try
        {        
            /* save the current play state */
            saveCurrentMoviePlayingState();
        
            /* step the movie forward */
            movieViewObject.stepForward(sender);
        
            /* restore the movie play state */
            restoreMoviePlayingState(sender);
        } 
        catch (Exception e) 
        {
            e.printStackTrace();
        };
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
    
    public void resetPlayButtonForMovieStopState(Object sender)
    {
        /* reset button text to "play" text */
        playButton.setTitle(">");
        /* reset button action method to our "play" method
            so the next time the button is pressed this
            method will instead be invoked */
        playButton.setAction(playSelector);
    
    }
    
    //////////
    //
    // showTheWindow
    //
    // This method is used to make a window
    // visible and bring it to the front
    //
    //////////
    
    public void showTheWindow(NSWindow window)
    {
        /* is the window visible? */
        if (!window.isVisible())
        {
            /* window is not visible, so let's make it visible */
            window.setFrame(window.frame(), true);
        }
        /* make the window accept keys, and make it the
            front window */
        window.makeKeyAndOrderFront(menuItem_New);
    
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
    
    void awakeFromNib()
    {
        try
        {
            /* retrieve our movie file from our bundle resource */
            String filePath = NSBundle.mainBundle().pathForResource("MovieFile", "mov");
            /* create an NSMovie object from our QuickTime movie */
            NSMovie theMovie = new NSMovie(new java.net.URL("file://" + filePath),true);
        
            /* assign the NSMovie object (which we made above) for our NSMovieView object */
            movieViewObject.setMovie(theMovie);
                
            /* set the high/low ranges for our volume slider */
            setVolumeSlider.setMaxValue(1.0);
            setVolumeSlider.setMinValue(0.0);
        
            menuItem_New.setEnabled(true);
                                    
            /* set the volume slider to the maximum volume */
            setVolumeSlider.setFloatValue(0.5F);
            
            /* we'll use these play selectors to toggle modes for
                our "play" button */
            playSelector = new NSSelector("play", new Class[] {getClass()} );
            stopSelector = new NSSelector("stop", new Class[] {getClass()} );

            /* create a timer which we'll use to update our "play" button text */
            updateButtonTimer = new NSTimer(
                                            2.0,
                                            this,
                                            new NSSelector("timerFired", new Class[] {getClass()} ),
                                            null,
                                            true);

            /* register our NSTimer for the default run loop */
            NSRunLoop.currentRunLoop().addTimerForMode(updateButtonTimer, NSRunLoop.DefaultRunLoopMode);
            
            /* we want notification when the application quits
                so we can clean up */
            NSNotificationCenter.defaultCenter().addObserver(this,
                                                        new NSSelector("applicationWillTerminate", new Class[] {NSNotification.class}),
                                                        "NSApplicationWillTerminateNotification",
                                                        null);
        } 
        catch (Exception e) 
        {
            e.printStackTrace();
        };
        
    }
        
    //////////
    //
    // timerFired
    //
    // When our timer fires we'll use the opportunity
    // to adjust the play button text for the stop state
    //
    //////////

   public void timerFired(Object sender)
    {
        /* is the movie done playing? */
        if (!movieViewObject.isPlaying())
        {
            /* movie is done - so reset play button text */
            resetPlayButtonForMovieStopState(sender);
        }
    }

    
    //////////
    //
    // applicationWillTerminate
    //
    // Called when the application is ready to quit.
    // We'll use this opportunity to clean up.
    //
    //////////
    
    public void applicationWillTerminate(NSNotification notification)
    {
        playSelector = null;
        stopSelector = null;
        
        NSRunLoop.currentRunLoop().removeTimerForMode(updateButtonTimer,NSRunLoop.DefaultRunLoopMode);
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
    
    public void saveCurrentMoviePlayingState()
    {
        isMoviePlaying = movieViewObject.isPlaying();
        moviePlaybackRate = movieViewObject.rate();
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
    
    public void restoreMoviePlayingState(Object sender)
    {
        if (isMoviePlaying)
        {
            movieViewObject.start(sender);
        }
        movieViewObject.setRate(moviePlaybackRate);
    }
    
}
