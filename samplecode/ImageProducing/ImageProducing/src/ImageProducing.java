/*

File: ImageProducing.java

Abstract: Generates AWT Image objects from a QuickTime movie

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
import java.awt.image.*;
import java.awt.event.*;
import java.io.*;

import quicktime.qd.*;
import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.GraphicsImporter;
import quicktime.std.movies.*;
import quicktime.io.*;
import quicktime.util.*;

import quicktime.app.view.*;
import quicktime.util.*;
import quicktime.std.image.GraphicsMode;
import javax.swing.*;

import ip.*;

/** Draws images that are produced by the QTImageProducer that uses a MoviePlayer as the source
 */
public class ImageProducing extends JFrame implements MovieDrawingComplete, QDConstants, StdQTConstants {		
	public static void main (String args[]) {
		try {
			QTSession.open();
			QTFile f1 = new QTFile (QTFactory.findAbsolutePath ("jumps.mov").getPath());			
			ImageProducing pm = new ImageProducing (f1);
			pm.pack();
			pm.show();
			pm.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	
	ImageProducing (QTFile movFile) throws QTException {
		super ("Consumer");

		OpenMovieFile openMovieFile = OpenMovieFile.asRead(movFile);
		myMovie = Movie.fromFile (openMovieFile);
		myMovie.getTimeBase().setFlags (loopTimeBase);
		MoviePlayer moviePlayer = new MoviePlayer (myMovie);
		QDRect r = moviePlayer.getDisplayBounds();
		Dimension d = new Dimension (r.getWidth(), r.getHeight());
		imgProducer = new QTImageProducer (moviePlayer, d);

			//this tells us that the movie has redrawn and 
			//we use this to redraw the QTImageProducer - which will 
			//supply more pixel data to its registered consumers
		myMovie.setDrawingCompleteProc (movieDrawingCallWhenChanged, this);

		JPanel panel = new JPanel (new BorderLayout());
		getContentPane().add ("Center", panel);
		panel.add ("South", new ControlPanel (moviePlayer));

		IPJComponent canvas = new IPJComponent (d, imgProducer);
		panel.add("Center", canvas);
                                        
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				try {
					//have to remove a callback we install
					myMovie.removeDrawingCompleteProc();
				} catch (QTException ex) {}
				QTSession.close();
				dispose();
			}
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}	

	QTImageProducer imgProducer;
	Movie myMovie;
	
	public int execute (Movie myMovie) {
		try {
			imgProducer.updateConsumers (null);
		} catch (QTException e) {
			return e.errorCode();
		}
		return 0;
	}

	static class IPJComponent extends JComponent {
		IPJComponent (Dimension prefSize, QTImageProducer ip) {
			pSize = prefSize;
			img = createImage (ip);
			prepareImage (img, this);
		}
		
		private Dimension pSize;
		private Image img;
		
	    public Dimension getPreferredSize() {
	    	return pSize;
	    }

		public void paint (Graphics g) {
			g.drawImage (img, 0, 0, pSize.width, pSize.height, this);
		}
		// stops flicker as we have no background color to erase
		public void update (Graphics g) {
			paint (g);
		}
	}
}

