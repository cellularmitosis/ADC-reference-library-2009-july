/*

File: MovieScreen.java

Abstract: Display component for movie playback

Version: 1.2

© Copyright 2006 Apple Computer, Inc. All rights reserved.

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

package duke;

import java.awt.*;
import java.awt.event.*;
import java.io.IOException;
import quicktime.QTException;
import quicktime.io.*;
import quicktime.std.movies.*;
import quicktime.app.view.*;
import quicktime.util.*;

/**
 * This class is just used as a central location for the player, canvas and
 * controller, which are all set up via the getNewMovie method.
 */
public class MovieScreen {
//_________________________ CLASS METHODS
	public MovieScreen() throws QTException {
                try {
                        QTFile qtfile = new QTFile(QTFactory.findAbsolutePath("Sample.mov"));
                        OpenMovieFile openqtfile = OpenMovieFile.asRead(qtfile);
                        Movie movie = Movie.fromFile(openqtfile);
                        MovieController movController = new MovieController(movie);
                        movieComponent = QTFactory.makeQTComponent(movController);

                } catch (IOException e) {}
        }

//_________________________ INSTANCE VARIABLES
        private QTComponent movieComponent = null;
	
//_________________________ INSTANCE METHODS
	/** Stops the player and removes it from the task thread*/
	public void stopPlayer () {
		try {
			if (movieComponent != null && movieComponent instanceof QTComponent)
				movieComponent.getMovieController().play(0);
		} catch (QTException e) {}
	}

	/** Return the current QTComponent. */
	public QTComponent getMovieComponent() { return this.movieComponent; }
	
	public void startPlayer() {
		try {
			if (movieComponent != null && movieComponent instanceof QTComponent)
				movieComponent.getMovieController().play(1);
		} catch (QTException e) {}
	}
	
	public void setLooping (boolean flag) {
		try {
			if (movieComponent != null && movieComponent instanceof QTComponent)
				movieComponent.getMovieController().setLooping(flag);	
		} catch (QTException e) {}
	}
	
	public boolean isLooping () {
		try {
			if (movieComponent != null && movieComponent instanceof QTComponent)
				return movieComponent.getMovieController().getLooping();
		} catch (QTException e) {}
		return false;
	}

	/**
	 * Allow user to pick a new movie. Ordinarily, this method would return a
	 * boolean. But it returns the path string of the chosen movie so we know
	 * where to get the images for the tumbling duke animation.
	 */
	public void getNewMovie (QTFile qtf) throws QTException, IOException {
		stopPlayer();
		
                OpenMovieFile openqtfile = OpenMovieFile.asRead(qtf);
                Movie movie = Movie.fromFile(openqtfile);
                MovieController movController = new MovieController(movie);
                movieComponent.setMovieController(movController);
	}
}
