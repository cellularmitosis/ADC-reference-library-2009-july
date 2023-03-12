/*
 
 File: QTStreamingApplet.java
 
 Abstract: Applet which loads a QuickTime movie from a network connection
 
 Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
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
 
 Copyright © 2006 Apple Computer, Inc., All Rights Reserved
 
 */
 
import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import quicktime.*;
import quicktime.std.*;
import quicktime.io.QTFile;
import quicktime.app.view.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;

public class QTStreamingApplet extends Applet {

    private Vector urlTable;
    private TextField urlTextField;
    private PopupMenu pm;
    private QTComponent myQTCanvas;
    
	public void init () {
		try {
			if (QTSession.isInitialized() == false)
				QTSession.open();
			// set up a QTCanvas which will disply its content 
			// at its original size of smaller and centered in the space given
			// to the QTCanvas when the applet is layed out
			setLayout (new BorderLayout());
			urlTable = new Vector();
			add (bottomPanel(), "South");
						
		} catch (QTException qtE) {
			//in this case the only QTException is in QTSession.open()
			throw new RuntimeException (qtE.getMessage());		
		}
	}	
	
	public void start () { 
	}
	
	public void stop () { 
            try{
		if (myQTCanvas != null)
			myQTCanvas.setMovie(null);
		} catch (QTException qtE) {
				//in this case the only QTException is in QTSession.open()
			throw new RuntimeException (qtE.getMessage());		
		}
	}
	
	public void destroy () {
		QTSession.close();
	}
		
		
	/**
	 * creates drawable object from the URL the user has entered
	 * or from the menu item chosen
	 */
	private void createNewMovieFromURL(String selURL) throws QTException {
		String url = urlTextField.getText();	
                DataRef dRef = new DataRef(selURL);
                Movie mov = Movie.fromDataRef (dRef,StdQTConstants.newMovieActive);
                MovieController mc = new MovieController(mov);
                if (myQTCanvas == null) {
                    myQTCanvas = QTFactory.makeQTComponent(mc);
                    add((Component)myQTCanvas, "Center");
				}else {
                    myQTCanvas.setMovieController (mc);
                }
				// We have just added a component. 
				// so we must revalidate the Applet's Layout.
				validate();
	}
	
	public Component bottomPanel () {
		Panel row2 =  new Panel();
		row2.setLayout(new FlowLayout(FlowLayout.CENTER));
		row2.setBackground(Color.white);	
			
		urlTextField = new TextField ("rtsp://... Enter a URL to a remote movie", 40); //Enter URL to movie here
		urlTextField.setFont (new Font ("Dialog", Font.PLAIN, 10));
		urlTextField.setEditable (true);
		urlTextField.addActionListener (new ActionListener () {
			
			public void actionPerformed (ActionEvent ae) {
					try {
						createNewMovieFromURL(urlTextField.getText());
					} catch (QTException e) {
							//probably a non-fatal error that the Applet 
							//should deal with more informatively
						e.printStackTrace();
					}
			}
		});

		row2.add(urlTextField);
		
		return row2;
	}
	
    //workaround for java 1.3 bug - throws IllegalStateException on quit
    public java.awt.im.InputContext getInputContext() {
        return null;
    }

}
