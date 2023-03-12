/*
	File:		ViewerDemo.java
	
	Description:	Demo/presentation frame using the QuicKTime ImagePresenter.

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
import java.io.*;

import quicktime.*;
import quicktime.util.*;
import quicktime.app.QTFactory;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.qd.*;

/*
 This demo shows a simple slide show presentation where the SlideShow object defines
 the behaviour that moves the currently viewed image to the next or previous image
 based on mousePressed events.
*/
public class ViewerDemo extends Frame {
	public static void main (String[] args) {
		ViewerDemo vd = new ViewerDemo();
		vd.init();
		vd.show();
		vd.toFront();
	}
//____________________ INSTANCE METHODS

	public void init () {
// The frame, canvas and viewer are all the same size
		try {
			QTSession.open();

// add canvas to frame
			setLayout (new BorderLayout());
			QTCanvas canv = new QTCanvas ();
			add ("Center", canv);
			
// create image sequence		
			File file = QTFactory.findAbsolutePath ("images/Ship01.pct");		
			ImageDataSequence seq = ImageUtil.createSequence (file);
			ImageSequencer images = new ImageSequencer (seq);
			images.setLooping (ImageSequencer.kLoopForwards);		

// create our SlideShow
			SlideShow viewer = new SlideShow (images);

			addNotify();	//required so we can set the frame to the right size for insets
			Insets insets = getInsets();
			Dimension d = new Dimension (viewer.getDisplayBounds().getWidth(), viewer.getDisplayBounds().getHeight());
			setSize ((insets.left + insets.right + d.width), (insets.top + insets.bottom + d.height));

			addWindowListener(new WindowAdapter () {
				public void windowClosing (WindowEvent e) {
					QTSession.close();
					dispose();
				}
				public void windowClosed (WindowEvent e) {
					System.exit(0);
				}
			});
			canv.setClient (viewer, true);
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}	
	}
}

