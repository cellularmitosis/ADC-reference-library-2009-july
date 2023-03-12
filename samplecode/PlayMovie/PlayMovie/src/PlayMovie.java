/*

File: PlayMovie.java

Abstract: Simple movie playback using QTJ

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

import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.io.IOException;
import javax.swing.*;

import quicktime.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.app.view.*;

public class PlayMovie extends Frame implements Errors {	

        static final private String theTextMovieName = "Sample.mov";
        
        private JDialog errorDialog;

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
		
		new FileMenu (this);
		 					
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				goAway();
			}
			
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
                });
	
                try {
                    //open and display a sample movie in the project folder
                    createNewMovieFromURL("file://" + QTFactory.findAbsolutePath (theTextMovieName));
		}
		catch (IOException ioe) {
                    showErrorDialog(ioe.toString());
                    System.exit(0);
		}
        }

	private Movie m;
	private MovieController mc;	
	private QTComponent qtc = null;
        
	// This will resize the window to the size of the new movie
	public void createNewMovieFromURL (String theURL) {
		try {
			// create the DataRef that contains the information about where the movie is
			DataRef urlMovie = new DataRef(theURL);
			
			// create the movie 
			m = Movie.fromDataRef (urlMovie,StdQTConstants.newMovieActive);
                        
                        // create the movie controller
                        mc = new MovieController (m);
                        
                        // create and add a QTComponent if we haven't done so yet, otherwise set qtc's movie controller
                        if (qtc == null)
                        {
                            qtc = QTFactory.makeQTComponent(mc);
                            add((Component)qtc);
                        } else {
                            qtc.setMovieController(mc);
                        }
			
			// this will set the size of the enclosing frame to the size of the incoming movie
			pack();
		
			// start up the movie
                        m.setRate(1);
                        
		} catch (QTException err) {
			err.printStackTrace();
		}
	}
	
	public MovieController getMovieController () { return mc; }
	
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
        
        	/**
	 * Displays an error dialog reporting any problems encountered
	 * 
	 * @param theError the error string
 	 */
	private void showErrorDialog(String theError) {
		errorDialog = new JDialog();
		errorDialog.setModal(true);
		errorDialog.setResizable(false);
		Container c = errorDialog.getContentPane();
		c.setLayout(new BoxLayout(c,BoxLayout.Y_AXIS));
		
		JPanel jp = new JPanel();
		jp.setLayout(new BoxLayout(jp,BoxLayout.Y_AXIS));
		jp.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));				

		JLabel jl = new JLabel(theError);
		jl.setMaximumSize(new Dimension(270,20));
		
		jp.add(jl);
		
		JPanel jp2 = new JPanel();
		jp2.setLayout(new BoxLayout(jp2,BoxLayout.Y_AXIS));
		jp2.setBorder(BorderFactory.createEmptyBorder(5,125,15,10));

		JButton jb = new JButton("OK");
		jb.setMaximumSize(new Dimension(70,20));

		jp2.add(jb);
		
		errorDialog.getContentPane().add(jp, "South");
		errorDialog.getContentPane().add(jp2, "South");
		jb.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				errorDialog.dispose();
			}
		});
		errorDialog.setBounds(0,0,300,100);
		errorDialog.show();
	}
}
