/*
 * quicktime.app: Sample Code for Initial Seeding
 *
 * © 1997 Copyright, Apple Computer
 * All rights reserved
 */

import java.applet.Applet;
import java.awt.*;

import quicktime.*;
import quicktime.io.QTFile;

import quicktime.app.display.*;
import quicktime.app.image.ImageDrawer;	// for exceptions

public class QTTestApplet extends Applet {	
	private boolean initDone = false;
	
	public void init () {
		try {
			if (QTSession.isInitialized() == false)
				QTSession.open();
			initDone = true;
		} catch (NoClassDefFoundError er) {
			add (new Label ("Can't Find QTJava classes"), "North");
			add (new Label ("Check install and try again"), "South");
		} catch (SecurityException se) {
				// this is thrown by MRJ trying to find QTSession class
			add (new Label ("Can't Find QTJava classes"), "North");
			add (new Label ("Check install and try again"), "South");
		} catch (Exception e) {
				// do a dynamic test for QTException 
				//so the QTException class is not loaded unless
				// an unknown exception is thrown by the runtime
			if (e instanceof ClassNotFoundException || e instanceof java.io.FileNotFoundException) {
				add (new Label ("Can't Find QTJava classes"), "North");
				add (new Label ("Check install and try again"), "South");
			} else if (e instanceof QTException) {
				add (new Label ("Problem with QuickTime install"), "North");
				if (((QTException)e).errorCode() == -2093)
					add (new Label ("QuickTime must be installed"), "South");			
				else
					add (new Label (e.getMessage()), "South");			
			}
		}
	}
	
	// we create an inner class here so that the class loader
	// does NOT try to load QT classes until we know that 
	// everything is OK	
	public void start () {
		if (initDone)
			new DoQT();
	}	
	
	class DoQT {
		DoQT () {
			try {				
				setLayout (new BorderLayout());
				
				QTCanvas myQTCanvas = new QTCanvas (QTCanvas.kInitialSize, 0.5F, 0.5F);
				add (myQTCanvas, "Center");	
				myQTCanvas.setClient (ImageDrawer.getQTLogo(), true);

				add (new Label ("QuickTime for Java"), "North");
				add (new Label ("Installed successfully"), "South");
			} catch (Throwable e) {
				if (e instanceof ClassNotFoundException) {
					add (new Label ("Can't Find QTJava classes"), "North");
					add (new Label ("Check install and try again"), "South");
				} else {
					System.out.println (e);
					e.printStackTrace();
				}
			}		
		}
	}
	
	public void stop () {}
	
	public void destroy () {
		if (initDone)
			QTSession.close();
		initDone = false;
	}
}
