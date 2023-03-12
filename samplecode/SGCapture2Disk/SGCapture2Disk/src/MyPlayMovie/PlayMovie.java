/*
	File:		PlayMovie.java
	
	Description:	Playback window for recorded material using the SGCapture2Disk sample.

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

package MyPlayMovie;

import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.io.IOException;

import quicktime.*;
import quicktime.io.*;
import quicktime.std.movies.*;
import quicktime.util.QTUtils;
import quicktime.app.display.QTCanvas;
import quicktime.app.players.QTPlayer;

// This example shows using QT without the layout manager
// The movie is automatically resized to the size of the window
// by intercepting the setBounds call on the window
public class PlayMovie extends Frame implements WindowListener {	

	public static void main (String args[]) {
		PlayMovie pm = new PlayMovie("QT in Java",null);
		pm.show();
		pm.toFront();
	}

	public PlayMovie (String title,QTFile qtf) {
		super (title);
		try { 
			if (qtf == null)
			{
				QTSession.open();
				qtf = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
			}

			myQTCanvas = new QTCanvas();
			add(myQTCanvas);
		
			OpenMovieFile movieFile = OpenMovieFile.asRead(qtf);
			MovieController mc = new MovieController (Movie.fromFile (movieFile));
			mc.setKeysEnabled (true);

			addNotify();
			Insets insets = getInsets();
			setBounds (0, 0, (insets.left + insets.right + mc.getBounds().getWidth()), (insets.top + insets.bottom + mc.getBounds().getHeight()));
			
			myQTPlayer = new QTPlayer (mc);
			myQTCanvas.setClient (myQTPlayer, true);

			addWindowListener(this);
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	private QTPlayer myQTPlayer;
	private QTCanvas myQTCanvas;
	
	public void windowClosing (WindowEvent e) {
		myQTCanvas.removeClient();
		QTSession.close();
		dispose();
	}

	public void windowIconified (WindowEvent e) {
		try { 
			myQTPlayer.setRate (0); 
		} catch (Exception ex) {}
		myQTPlayer.getTasker().stop();
	}
	
	public void windowDeiconified (WindowEvent e) {
		myQTPlayer.getTasker().start();
	}
	
	public void windowClosed (WindowEvent e) { 
		System.exit(0);
	}
	
	public void windowOpened (WindowEvent e) {
	}
	public void windowActivated (WindowEvent e) {}
	public void windowDeactivated (WindowEvent e) {}
}
