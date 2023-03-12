/*

File: PlaySound.java

Abstract: Plays sound/MIDI files without any UI

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

import quicktime.*;
import quicktime.io.*;
import quicktime.app.time.*;
import quicktime.app.view.*;
import quicktime.std.movies.*;
import quicktime.std.*;

import java.awt.*;
import java.awt.event.*;
import java.io.*;

/**
 * This sample demonstrates how to play a sound or a midi file without using a QTCanvas.
 * All user interface elements are using AWT.
 */
public class PlaySound extends Frame
{
    Button playMidi, playSnd;
    Movie movie;
    
    static public void main (String[ ] args) {
        try {
            QTSession.open();
            PlaySound theSoundPlayer = new PlaySound();
        }
        catch (Exception e) {
            e.printStackTrace();
            QTSession.close();
        }
    }
    
    /**
     * Create the user interface
     */
    PlaySound () {
    	playMidi = new Button ("Play Midi File");
    	playSnd  = new Button ("Play Sound File");
    	
    	this.add (playMidi);
    	this.add (playSnd );
    	
    	playMidi.addActionListener( new Action() );
    	playSnd.addActionListener( new Action() );
    	
    	this.setLayout (new FlowLayout());
    	setVisible (true);
        
        addWindowListener(new WindowAdapter () {
                public void windowClosing (WindowEvent e) {
                        QTSession.close();
                        dispose();
                }
                public void windowClosed (WindowEvent e) {
                        System.exit(0);
                }
        } );
    
        pack();
    }
    	
    /**
     * The Action class responds to user interaction with the buttons
     */
    class Action implements ActionListener {
        public void actionPerformed (ActionEvent event) {
            Object object = event.getSource();
            try {
                if (object == playMidi) {
                    if (movie != null) {
                        movie.setRate(0);
                        movie.setActive(false);
                        TaskAllMovies.removeMovie();
                    }
                    playFile ("jazz.mid");
                } else if (object == playSnd) {
                    if (movie != null) {
                        movie.setRate(0);
                        movie.setActive(false);
                        TaskAllMovies.removeMovie();
                    }
                    playFile ("sin440.aif");
                }
            }catch (StdQTException ex) {
                ex.printStackTrace();
            }
        }
    }
    
    /**
     * The playFile method loads and plays an audio file with the given name
     * or partial path. If a sound is already playing, it will be stopped before
     * the new sound is played.
     * @param fileName the name of the sound file to be played
     */
    public void playFile (String fileName) {
    	try {
            String soundLocation = QTFactory.findAbsolutePath(fileName).getPath();
			
            OpenMovieFile fileIn = OpenMovieFile.asRead (new QTFile(soundLocation));
            movie = Movie.fromFile (fileIn);
            TaskAllMovies.addMovieAndStart();
            movie.setActive(true);
            movie.setRate(1);
	 	} catch (IOException ioe) {
	 		ioe.printStackTrace();
	 	} catch (QTException qte) {
	 		qte.printStackTrace();
	 	}
    }
}