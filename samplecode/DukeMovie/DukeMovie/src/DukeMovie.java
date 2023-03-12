/*

File: DukeMovie.java

Abstract: Demo class for DukeMovie sample

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
import java.io.IOException;
import java.io.File;
import java.awt.event.*;

import quicktime.*;
import quicktime.app.view.*;
import quicktime.io.QTFile;
import quicktime.util.*;

import duke.*;
/**
 * Moving Movie Demo.
 */
public class DukeMovie extends Frame {
//_________________________ CLASS VARIABLES
	private static final Color bgColor = Color.lightGray; //tumbling duke gif's are light gray

//_________________________ CLASS METHODS
	public static void main(String args[]) {
		try {
			QTSession.open(); //start a QuickTime session
			Frame f = new DukeMovie("Biscotti = Java + QuickTime");
                        f.pack();
			f.show();
			f.toFront();
		}
		catch (Exception e) { 
			QTSession.close(); 
			System.exit(0); 
		}
	}

	DukeMovie (String frameTitle) throws QTException, IOException {
		super (frameTitle);

		setBackground(bgColor);
		setLayout(new BorderLayout());
                
		ms = new MovieScreen();
		Panel panel = new Panel();
                panel.add((Component)ms.getMovieComponent());
                add("Center", panel);
		
		// create, add and start tumbling duke, if the images are available
		try {
			File file = QTFactory.findAbsolutePath ("duke");		
			duke = new TumbleItem(file.getAbsolutePath());
		} catch (Exception e) {
			e.printStackTrace();
		}
		if (duke != null) {
			duke.setBackground(bgColor);
			add("North", duke);
		}
		
		// create our little control panel out of simple AWT objects
		cp = new ControlPanel(bgColor, ms, this);
		add("South", cp);

		//setBounds(0, 0, 625, 420);
		addWindowListener(new WindowAdapter () {
			public void windowOpened (WindowEvent ev) {
				if (duke != null) duke.start();
			}

			public void windowClosing (WindowEvent e) {
				if (duke != null) duke.stop();
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}

//_________________________ INSTANCE VARIABLES
	private MovieScreen ms;
	private TumbleItem  duke;
	private ControlPanel cp;
}