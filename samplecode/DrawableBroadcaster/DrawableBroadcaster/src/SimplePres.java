/*
 * QuickTime for Java SDK Sample Code
 *
 * Usage subject to restrictions in SDK License Agreement
 * Copyright: © 2000 Apple Computer, Inc.
 */
 
import quicktime.io.*;
import quicktime.app.display.*;
import quicktime.app.sg.*;
import quicktime.streaming.*;
import quicktime.*;
import quicktime.std.*;
import quicktime.qd.*;
import quicktime.app.anim.*;

/* SimplePres- this class uses a simple approach to creating a Broadcaster (Presentation) from a specified QT File 
 * object.  It also uses the utility class Presentation Drawer to handle drawing operations within the QTCanvas */
public class SimplePres {
	public static final int kDefaultPresTimeScale = 600;	// the default time scale of the Presentation
	
	/* Constructor- creates a new presentation object from a file and a new Presentation drawer */ 
	SimplePres (QTFile file, QTCanvas canvas) throws QTException {
		newPresFromFile (file);
		pDrawer = new PresentationDrawer(pres);
		QDGraphics gw = new QDGraphics( new QDRect( BroadcastDrawer.WIDTH, BroadcastDrawer.HEIGHT ));
		showSettings();	// shows the settings dialog
	}

	/* Creates a new presentation object from a QTFile using default settings for the MediaParams and PresParams */
	public void newPresFromFile( QTFile f ) {
		try {
			MediaParams medParams = new MediaParams();
		
			PresParams presParams = new PresParams( kDefaultPresTimeScale, QTSConstants.kQTSSendMediaFlag | 
					QTSConstants.kQTSAutoModeFlag | QTSConstants.kQTSDontShowStatusFlag, medParams );
			pres = Presentation.fromFile( f, presParams );	
		}
		catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	/** Shows the settings dialog that allows you to specify the input source, codecs, and packetizer 
	  * for the broadcast and prerolls the broadcast
	  */
	public void showSettings( ) {
		try {
			SettingsDialog dialog = new SettingsDialog(pres);
			pres.preroll();
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	/* starts the broadcast and the drawing task */
	public void startBroadcast() {
		try {
			pres.start();
			pDrawer.startTasking();
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	/* stops the broadcast and the drawing task */
	public void stopBroadcast() {
		try {
			pres.stop();
			pDrawer.stopTasking();
		} catch (QTException e) {
			e.printStackTrace();
		}
	}

/* ---- Protected data members ---- */	
	PresentationDrawer 	pDrawer = null;
	Presentation 		pres = null;
}