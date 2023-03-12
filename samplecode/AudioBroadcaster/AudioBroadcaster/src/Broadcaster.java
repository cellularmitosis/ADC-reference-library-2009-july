/*
 * QuickTime for Java SDK Sample Code
 *
 * Usage subject to restrictions in SDK License Agreement
 * Copyright: © 2000 Apple Computer, Inc.
 */
 
import quicktime.streaming.*;
import quicktime.app.time.*;
import quicktime.*;
import quicktime.io.*;

/* The Broadcaster class can create and prepare a presentation from a QTFile representation of an SDP file
   Additionally, this class also creates a task thread that is responsible for taking care of idling tasks for
   the presentation object */
public class Broadcaster extends Tasking {
	private static TaskThread idleTasker = new TaskThread ("Broadcaster Idle Tasker", 20);
	public static final int kDefaultPresTimeScale = 600; // default time scale of the presenter
	
//_________________________ CLASS METHODS
	public Broadcaster( QTFile file ) throws QTException {
		try {
			MediaParams medParams = new MediaParams();
			
			PresParams presParams = new PresParams( kDefaultPresTimeScale, QTSConstants.kQTSSendMediaFlag | 
					QTSConstants.kQTSAutoModeFlag | QTSConstants.kQTSDontShowStatusFlag, medParams );
			pres = Presentation.fromFile( file, presParams );
		} catch ( QTException e ) {
			e.printStackTrace();
		}
	 	setDefaultTasker (idleTasker);
	 }
	 
//_________________________ INSTANCE VARIABLES
	private Presentation pres;

//_________________________ INSTANCE METHODS
	/** This returns the PresentationDrawer's Presentation that is displayed by this drawable.
	 * @return an Presentation
	 */
	public Presentation getPresentation () { return pres; }

	/**
	 * This method is called by the specified object when the instance of 
	 * the class that implements this interface is added to the object
	 * that is the source of the interest.
	 * @param interest the object that is to be the source of interest for the
	 * the object that implements this interface.
	 */
	public void addedTo (Object interest) {}
	
	/**
	 * This method is called by the specified object when the instance of 
	 * the class that implements this interface is removed from the object
	 * that is the source of the interest.
	 * @param interest the object that was the source of interest for the
	 * the object that implements this interface.
	 */
	public void removedFrom (Object interest) {}
	
	/**
	 *  Displays a settings dialog and prerolls the broadcast
	 */
	public void prepareBroadcast( ) {
		try {
			SettingsDialog dialog = new SettingsDialog(pres);
			pres.preroll();
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * This method starts the broadcast
	 */
	public void startBroadcast( ) { 
		try {
			pres.start();
			startTasking();
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * This method stops the broadcast
	 */
	public void stopBroadcast() {
		try {
			pres.stop();
			stopTasking();
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * This method performs idle processing for the Presentation and will be automatically
	 * called if this object is added to the TaskThread object.
	 * @see quicktime.util.TaskThread
     */
	public synchronized final void task() throws QTException {
		pres.idle(null);
	}
	
	
	/** Returns a String representation of this object */
	public String toString () {
		return getClass().getName() + "[" + pres + "]";
	}
}