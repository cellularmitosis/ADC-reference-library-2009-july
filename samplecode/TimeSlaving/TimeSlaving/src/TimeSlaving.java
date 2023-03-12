/*
	File:		TimeSlaving.java
	
	Description:	This demo program shows how the effects of using timing information to 
                        control the Scrolling Java Text object and how slaving its TimeBase 
                        to a movie alters the behaviour.

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
import java.awt.image.*;
import java.io.*;

import quicktime.qd.*;
import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.*;
import quicktime.std.movies.*;
import quicktime.io.*;
import quicktime.util.*;

import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.time.*;

public class TimeSlaving extends Frame implements QDConstants, StdQTConstants, WindowListener, Errors {		
	public static TimeSlaving pm;
	
	public static void main (String args[]) {
		pm = new TimeSlaving("QT in Java");
		pm.show();
		pm.toFront();
	}

	TimeSlaving (String title) {
		super (title);
		try { 
			QTSession.open();

			textCanv = new QTCanvas(QTCanvas.kInitialSize, 0.5f, 0.5f);
			textCanv.setBackground(Color.black);
			add("East", textCanv);
			addNotify();
			Insets insets = getInsets();
			
			addWindowListener(this);
			setUpWindow();
			setBounds (0, 0, (insets.left + insets.right + 500), (insets.top + insets.bottom + 250));
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	
	private void setUpWindow () throws QTException, IOException {		
		ScrollingText t = new ScrollingText();
		
		Dimension d = new Dimension (300, 150);
		TimeableID qid = new TimeableID (t, d);
		ti = new Timer (110, 1, qid);	//100 frames a second at rate == 1
		qid.setTimer(ti);
		
		cp = new ControlPanel(ti, this);
		add("North", cp);		
		cp.setDisplay();
		textCanv.setClient (qid, true);
	}
	
	QTCanvas textCanv;
	ControlPanel cp;
	Timer ti;
	
	public void windowOpened (WindowEvent ev) {
		try {
			ti.setActive (true);
			cp.setDisplay();
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	public void windowClosing (WindowEvent e) {
		QTSession.close();
		dispose();
	}

	public void windowIconified (WindowEvent e) {}
	public void windowDeiconified (WindowEvent e) {}		
	public void windowActivated (WindowEvent e) {}
	public void windowDeactivated (WindowEvent e) {}
	public void windowClosed (WindowEvent e) { 
		System.exit(0);
	}
}

