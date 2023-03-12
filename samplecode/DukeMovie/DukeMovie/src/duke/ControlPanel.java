/*

File: ControlPanel.java

Abstract: Set of AWT controls for move playback

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
import java.io.*;

import quicktime.io.QTFile;
import quicktime.util.QTUtils;
import quicktime.*;
import quicktime.app.view.*;
import quicktime.std.StdQTException;
import quicktime.std.movies.MovieController;


/**
 * A simple panel of AWT objects to demonstrate how AWT can be used to control a
 * QuickTime movie.
 */
public class ControlPanel extends Panel implements Errors {
//_________________________ CLASS METHODS
	public ControlPanel(Color bgColor, MovieScreen ms, Frame frame) {
		this.ms = ms;
                this.frame = frame;
        
		setBackground(bgColor);
		setLayout(new FlowLayout(FlowLayout.CENTER, 10, 3));
		setFont(new Font("Geneva", Font.BOLD, 10));
		
		Action actionListener = new Action();
		Item itemListener = new Item();
	
		// start, stop, loop and get a new movie
		movieButtons.setBackground(getBackground());
		movieButtons.setLayout(new GridLayout(4, 1, 3, 3));
		movieButtons.add(startButton);
		movieButtons.add(stopButton);
		loopButton.setState(false);
		movieButtons.add(loopButton);
		movieButtons.add(getMovieButton);
		add(movieButtons);
		
		startButton.addActionListener( actionListener );
		stopButton.addActionListener( actionListener );
		loopButton.addItemListener( itemListener );
		getMovieButton.addActionListener( actionListener );
		
		// choose a visibility option
		visiblity.setBackground(getBackground());
		visiblity.setLayout(new GridLayout(3, 1));
		visiblity.add(visibleButton);
		visiblity.add(invisibleButton);
		add(visiblity);
		
		visibleButton.addItemListener( itemListener );
		invisibleButton.addItemListener( itemListener );
	}

	public Dimension getPreferredSize () { return new Dimension (100, 130); }
//_________________________ INSTANCE VARIABLES
	private MovieScreen ms;
        private Frame frame;

	private Panel  movieButtons   = new Panel();
	private Button startButton    = new Button("Start");
	private Button stopButton     = new Button("Stop");
	private Button getMovieButton = new Button("Get QT Media");

	private Checkbox loopButton = new Checkbox("Looping");

	private Panel         visiblity		 = new Panel();
	private CheckboxGroup checkBoxGrp2           = new CheckboxGroup();
	private Checkbox      visibleButton  = new Checkbox("Visible", checkBoxGrp2, true);
	private Checkbox      invisibleButton= new Checkbox("Invisible", checkBoxGrp2, false);

//_________________________ INSTANCE METHODS
	public void doGetMovie () {
		try {
			ms.stopPlayer ();
			QTFile qtf = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
// Below is code to use the Standard Java File Dialog, i.e no fancy Quicktime preview dialog.
//			FileDialog fd = new FileDialog ((Frame)getParent(), "Choose a QuickTime file to view:");
//			QTFileTypesFilter f = new QTFileTypesFilter();
//			f.set3DAccept(false);
//			fd.setFilenameFilter(f);
//			fd.show();
//			String fileDir = fd.getDirectory();
//			String fileName = fd.getFile();
//			if (fileDir != null && fileName != null) {
//				QTFile qtf = new QTFile (fileDir + fileName);
				ms.getNewMovie(qtf);
                                frame.pack();
//			}
		} catch (QTException e) {
		} catch (Exception e) {
		} finally { QTUtils.reclaimMemory(); }
	}
	
	/** Handle all the AWT checkbox item events here. */
	class Item implements ItemListener {
		public void itemStateChanged( ItemEvent event ) {
			if ( event.getSource() == visibleButton ) {
				((Component)ms.getMovieComponent()).setVisible(true);
			}
			else if (event.getSource() == invisibleButton) {
				ms.stopPlayer();
				((Component)ms.getMovieComponent()).setVisible(false);
			}
			else if (event.getSource() == loopButton) {
				if (ms.isLooping()) {
					loopButton.setState(false);
					ms.setLooping(false);
				}
				else {
					loopButton.setState(true);
					ms.setLooping(true);
				}
			}
		}
	}
	
	/** Handle all the AWT button events here. */
	class Action implements ActionListener {
		public void actionPerformed  (ActionEvent event) {
			if ( event.getSource() == getMovieButton ) {
				doGetMovie();
			}
			else if (event.getSource() == stopButton) {
				ms.stopPlayer();
			}
			else if (event.getSource() == startButton) {
				ms.startPlayer();
			}
                }
	}
}