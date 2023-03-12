/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

import quicktime.qd.*;
import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.*;
import quicktime.std.movies.*;
import quicktime.io.*;
import quicktime.util.*;

import quicktime.app.QTFactory;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.players.MoviePlayer;
import quicktime.std.image.GraphicsMode;

import cdf.*;
/** QuickTime Graphics Importer and Codec demo */
public class JavaQTAnimation extends Frame implements QDConstants, StdQTConstants, WindowListener {		
	public static int kWidth = 480;
	static int kHeight = 250;
	public static void main (String args[]) {
		JavaQTAnimation pm = new JavaQTAnimation("Java top, QuickTime bottom");
		
	}

	JavaQTAnimation (String title) {
		super (title);
		try { 
			QTSession.open();

			Insets insets = getInsets();
			setBounds (0, 0, (insets.left + insets.right + kWidth), (insets.top + insets.bottom + kHeight));
			
			addWindowListener(this);
			File file = QTFactory.findAbsolutePath ("duke");		
			duke = new JavaDuke(file.getAbsolutePath());
			add("North", duke);
			show();
			toFront();
			
			dukeFrame = new CaptureDukeFrame (this, "Capture Duke", kWidth);				
			dukeFrame.showDuke();

		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	public QTCanvas myQTCanvas;
	private CaptureDukeFrame dukeFrame;
	private JavaDuke duke;
	
	public void windowOpened (WindowEvent e) {

		try {	
			if (duke != null)
				duke.start();
			setResizable(false);	
		} catch (Exception ex) {
			ex.printStackTrace();
			QTSession.close();
		}
	}


	public void windowClosing (WindowEvent e) {
		dukeFrame.canv.removeClient();
		QTSession.close();
		dispose();
	}

	public void windowIconified (WindowEvent e) {
	}
	
	public void windowDeiconified (WindowEvent e) {
	}
		
	public void windowActivated (WindowEvent e) {}
	public void windowDeactivated (WindowEvent e) {}
	public void windowClosed (WindowEvent e) { 
		System.exit(0);
	}
}


