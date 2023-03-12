/*

File: KeyBoardController.java

Abstract: Demonstrates use of AWT KeyEvents to control movie playback 

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
import java.io.*;

import quicktime.std.StdQTConstants;
import quicktime.*;
import quicktime.qd.*;
import quicktime.io.*;
import quicktime.std.image.*;
import quicktime.std.movies.*;

import quicktime.app.view.*;

public class KeyBoardController extends Frame implements StdQTConstants, QDConstants {
	
	public static void main(String args[]) {
		try { 
			QTSession.open();
			
			KeyBoardController te = new KeyBoardController("KeyBoard Controller");
			te.pack();
			te.show();
			te.toFront();	
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
		
	}
	
	private Movie moov;
	private QTComponent qtComponent;
	private float savedRate = 1;
	
	KeyBoardController(String title) throws Exception {
		super (title);
		
		addWindowListener(new WindowAdapter() {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}
			
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
		
		QTFile qtFile = new QTFile (QTFactory.findAbsolutePath ("jumps.mov"));		
		OpenMovieFile movieFile = OpenMovieFile.asRead(qtFile);
		moov = Movie.fromFile (movieFile);
		
		qtComponent = QTFactory.makeQTComponent(moov);
		add("Center", (Component) qtComponent);
		
		// if the window gains focus, pass it on to the qtComponent
		addFocusListener( new FocusAdapter() {
			public void focusGained(FocusEvent evt) {
				((Component)qtComponent).requestFocus();
			}
		});
		
		((Component)qtComponent).addKeyListener(new KeyAdapter() {
			public void keyPressed (KeyEvent e) {
				try {
				switch (e.getKeyCode()) {
					case KeyEvent.VK_SPACE:
						if (moov.getRate() != 0) 
							moov.setRate (0);
						else
							moov.setRate (savedRate);
						break;
					case KeyEvent.VK_UP:
						moov.setTimeValue (moov.getDuration());
						break;
					case KeyEvent.VK_DOWN:
						moov.setTimeValue (0);
						break;
					case KeyEvent.VK_LEFT:
						moov.setRate (-1);
						savedRate = -1;
						break;
					case KeyEvent.VK_RIGHT:
						moov.setRate (1);
						savedRate = 1;
						break;
					default:
						break;
				}
			} catch (QTException ee) {
				throw new QTRuntimeException (ee);
			}
		}
		public void keyReleased (KeyEvent e) {}
		public void keyTyped (KeyEvent e) {}
		});
		moov.setRate(1);
	}	
}	