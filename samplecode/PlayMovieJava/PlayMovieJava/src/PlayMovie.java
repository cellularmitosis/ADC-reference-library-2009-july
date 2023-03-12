/*
	File:		PlayMovie.java
	
	Description:	Display frame for playback of the selected movie.

	Author:		Apple Computer, Inc.

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):
            11/22/2002	md	new SampleCode revisions

*/

import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.io.IOException;

import quicktime.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.app.display.*;
import quicktime.app.players.*;

public class PlayMovie extends Frame implements Errors {	
	public static void main (String args[]) {
		try { 
			QTSession.open();
			// make a window and show it - we only have one window/one movie at a time
			PlayMovie pm = new PlayMovie("QT in Java");
			pm.show();
			pm.toFront();
		} catch (QTException e) {
			// at this point we close down QT if an exception is generated because it means
			// there was a problem with the initialization of QT>
			e.printStackTrace();
			QTSession.close ();
		}
	}

	public PlayMovie (String title) {
		super (title);
		myQTCanvas = new QTCanvas();
		
		new FileMenu (this);
		 			
		add(myQTCanvas);			
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				goAway();
			}
			
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}

	private QTDrawable myPlayer;
	private Movie m;
	private QTCanvas myQTCanvas;	
	
	// This will resize the window to the size of the new movie
	public void createNewMovieFromURL (String theURL) {
		try {
			// create the DataRef that contains the information about where the movie is
			DataRef urlMovie = new DataRef(theURL);
			
			// create the movie 
			m = Movie.fromDataRef (urlMovie,StdQTConstants.newMovieActive);
			
			// This shows the steps to use the three different Objects to present a Movie
				// QTPlayer -> presents the MovieController allowing the user to interact with the movie and control its playback/presentation
				// MoviePlayer -> presents the Movie directly to the screen - application could provide its own controls
				// MoviePresenter -> puts the Movie into an offscreen buffer, the buffer is then drawn to the screen - application could provide its own controls
			if (true) { // QTPlayer
				MovieController mc = new MovieController (m);			
				mc.setKeysEnabled (true);
				myPlayer = new QTPlayer (mc);
			} else if (false) {	// make a MoviePlayer version
				myPlayer = new MoviePlayer (m);
			} else if (false) {	// make a MoviePresenter out of this
				myPlayer = new MoviePresenter (m);
			}
			
			myQTCanvas.setClient (myPlayer, true);
			
			// this will set the size of the enclosing frame to the size of the incoming movie
			pack();
		
			//no user control over MoviePlayer or MoviePresenter so set rate
			if (false)
				((Playable)myPlayer).setRate(1);
			
		} catch (QTException err) {
			err.printStackTrace();
		}
	}
	
	public QTDrawable getPlayer () { return myPlayer; }
	
	public QTCanvas getCanvas () { return myQTCanvas; }
	
	public Movie getMovie () throws QTException {
		return m;
	}
	
	public static void goAway () {
		QTSession.close();
		System.exit(0);	
	}	

	void stopPlayer () {
		try {
			if (m != null)
				m.setRate(0);
		} catch (QTException err) {
			err.printStackTrace();
		}
	}
}
