/*
	File:		SGCapture.java
	
	Description:	Container window for SequenceGrabber functionality.

	Author:		Apple Computer, Inc.

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

import quicktime.std.*;
import quicktime.qd.*;
import quicktime.*;
import quicktime.util.QTUtils;

import quicktime.std.StdQTConstants;
import quicktime.std.sg.*;

import quicktime.app.sg.SGDrawer;
import quicktime.util.*;
import quicktime.app.display.QTCanvas;
import quicktime.std.comp.*;
import quicktime.std.comp.Component;
import quicktime.app.image.*;

import MySGController.*;

public class SGCapture extends Frame implements WindowListener, StdQTConstants {	

	static final int kWidth = 320;
	static final int kHeight = 240;
	public QDGraphics mGraphics = null;
	public QTCanvas 		myQTCanvas;
	public SGControlPanel 	mControlPanel;
	
	public static void main (String args[]) {
	try {
			QTSession.open();		
			SGCapture pm = new SGCapture("QT in Java");
			pm.pack();
			pm.show();
			pm.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	SGCapture (String title) {
		super (title);
		try {		
			myQTCanvas = new QTCanvas(QTCanvas.kPerformanceResize, 0.5F, 0.5F);
			myQTCanvas.setMaximumSize (new Dimension (640, 480));
			myQTCanvas.setPreferredSize (new Dimension (kWidth, kHeight));
			myQTCanvas.setSize (new Dimension (kWidth, kHeight));
			
			add ("Center", myQTCanvas);	
			
	//Create a control panel with 3 buttons that perform PowerOn,Start and Stop recording
			mControlPanel = new SGControlPanel(myQTCanvas);
			add("South", mControlPanel);		
			addWindowListener(this);
		} catch (Exception ee) {
			ee.printStackTrace();
			QTSession.close();
		}
	}

	
	public void windowClosing (WindowEvent e) {
		QTSession.close();
		dispose();
	}


	public void windowIconified (WindowEvent e) {
		mControlPanel.doAction(seqGrabPause);
	}
	
	public void windowDeiconified (WindowEvent e) {
		mControlPanel.doAction(seqGrabUnpause);
	
	}
	
	public void windowClosed (WindowEvent e) { 
		System.exit(0);
	}
	
	public void windowOpened (WindowEvent e) { 

	}
	
	public void windowActivated (WindowEvent e) {}
	public void windowDeactivated (WindowEvent e) {}
}
