/*
	File:		vrInteraction.java
	
	Description:	Shows the introspection of a Movie and the presentation of mixer
                        channels to edit the playback volumes and balance of audio media.
        
	Author:		Apple Computer, Inc.

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
import java.io.IOException;

import quicktime.*;
import quicktime.io.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.util.*;
import quicktime.vr.*;

import quicktime.app.display.QTCanvas;
import quicktime.app.players.QTPlayer;

public class vrInteraction extends Frame implements Errors {

	public static void main (String args[]) {
		try {
			System.out.println ("Open a VR Movie...");
			QTSession.open (QTSession.kInitVR);
			
			// QTVR interaction methods are new in QTJava 501
			vrInteraction.checkQTJavaVersion(5,0,1);
				
			vrInteraction pm = new vrInteraction("vrInteraction");
			pm.pack();
			pm.show();
			pm.toFront();
			
		} catch (QTException e) {
			if (e.errorCode() == userCanceledErr) {
				QTSession.close();
				System.exit(0);
			}
			e.printStackTrace();
			QTSession.close();
		}
	}
	
	static final double  kHotSpotZoomInFactor = 0.75; // factor to zoom in on hotspot click
	
	/**
	 * vrInteraction
	 */
	vrInteraction (String title) throws QTException {
		super (title);

		QTFile qtf = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
		
		OpenMovieFile movieFile = OpenMovieFile.asRead(qtf);
		Movie m = Movie.fromFile (movieFile);
		MovieController mc = new MovieController (m);

		// Note it is good practice to explicitly setKeysEnabled and not rely
		// on the default keysEnabled value of the movie controller
		mc.setKeysEnabled (true); // set arrow keys / shift etc to control VR
		
		Track t = m.getQTVRTrack (1);
		if (t == null) {
			System.out.println(qtf.getName() + " is not a QTVR movie!");
		} else { //setup VR callbacks
			try {
				theVR = new QTVRInstance (t, mc);
				System.out.println(
					"========== Install QTVR Callbacks ============");
				new TriggerHotSpotInterceptor(theVR);
				new MouseOverHotSpot(theVR);
				System.out.println(
					"========= Initial Settings =======");
				System.out.println(
					"** PanTiltSpeed= " 
					+ theVR.getInteractionPanTiltSpeed());
				System.out.println(
					"** FieldOfView= "
					+ theVR.getFieldOfView());
				System.out.println(
					"========= Hotspot Dynamic Behaviors =====");
				System.out.println(
					"** On hotspot enter: pan/tilt speed => [2,5,8]");
				System.out.println(
					"   speed = ((hotspotID %3) *3) +2 = [slow,normal,fast]");
				System.out.println(
					"** On hotspot leave: reset pan/tilt speed and FOV");
				System.out.println(
					"** When zoom-in on link hotspot: trigger node change");
				System.out.println(
					"** On click in non 'link': zoom-in by "
					+ kHotSpotZoomInFactor);
				System.out.println(
					"======================================================");

			} catch (QTVRException e) {
				e.printStackTrace();
			}
		}
		
		QTPlayer myQTPlayer = new QTPlayer (mc);
		QTCanvas myQTCanvas = new QTCanvas();
		add(myQTCanvas);
		myQTCanvas.setClient (myQTPlayer, true);		

		addWindowListener(
			new WindowAdapter () {
				public void windowClosing (WindowEvent we) {
					if (theVR != null) {
						try {
							theVR.removeInterceptProc(QTVRConstants.kQTVRTriggerHotSpotSelector);
						} catch (Exception e) {
							e.printStackTrace();
						}
					}
					QTSession.close();
					dispose();
				}
				
				public void windowClosed (WindowEvent we) {
					System.exit(0);
				}
			}
		);
	}

	// work around for Java 1.3 IllegalStateException : cannot dispose input context
	public void dispose() {
		if (QTSession.getJavaVersion() != 0x10001) {
			try {
				super.dispose();
			} catch (java.lang.IllegalStateException e) {
				System.exit(0);
			}
		} else {
			super.dispose();
		}
	}

	QTVRInstance theVR;	// package scope for windowClosing()
	float hsEnterFOV;	// save the FOV when entering a hotspot, for later restore
	
	class TriggerHotSpotInterceptor implements QTVRInterceptor {
		TriggerHotSpotInterceptor(QTVRInstance vr) {
			try {
				System.out.println(
					"*** installInterceptProc(TriggerHotSpotInterceptor) ***");
				vr.installInterceptProc(QTVRConstants.kQTVRTriggerHotSpotSelector, this, 0);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		public boolean execute (QTVRInstance vr, QTVRInterceptRecord qtvrMsg) {
			int selector = qtvrMsg.getSelector();
			switch (selector) {
				case QTVRConstants.kQTVRTriggerHotSpotSelector:
					triggerHotSpotIntercept(vr, qtvrMsg);
					break;
				default:
					System.out.println(
						"InterceptTriggerHotSpot : " + Integer.toHexString(selector));
					break;
			}
			return false;	// allow normal QTVR processing
		}
		
		void triggerHotSpotIntercept(QTVRInstance vr, QTVRInterceptRecord qtvrMsg) {
			float fov = hsEnterFOV;
			boolean	setFOV = false;
			try {
				int hsID = qtvrMsg.getHotSpotID();
				if (vr.getHotSpotType(hsID) != QTVRConstants.kQTVRHotSpotLinkType) {
					setFOV = true;	// set this flag if we fail to set the FOV (constraint reached)
					fov = (float)(vr.getFieldOfView() * kHotSpotZoomInFactor) ;
					System.out.println(
						"InterceptTriggerHotSpot : trigger hotspot selector"
						+ " hs #" + hsID 
						+ " FOV=> " + fov);
					vr.setFieldOfView(fov);
				}
			} catch (QTException qte) {
				qte.printStackTrace(); 
				try {
					if (setFOV) {// failed to set the FOV (constraint reached)
						vr.setFieldOfView(hsEnterFOV);
						System.out.println(
							"InterceptTriggerHotSpot : reset FOV");
					}
				} catch (QTException qte2) {
					qte2.printStackTrace(); 
				}
			}
		}
	}
	
	// QTVR default values
	static final int	kDefaultPanTiltSpeed = 5;
	static final int	kDefaultClickHysteresis = 2;
	static final int	kDefaultClickTimeOut = 30;

	class MouseOverHotSpot implements QTVRMouseOverHotSpot {
		MouseOverHotSpot(QTVRInstance vr) {
			try {
				System.out.println(
					"*** setMouseOverHotSpotProc(MouseOverHotSpot) *** ");
				vr.setMouseOverHotSpotProc(this, 0);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		public int execute (QTVRInstance vr, int hotSpotID, int flags) {
			int err = 0;
			try {
				switch (flags) {
					case QTVRConstants.kQTVRHotSpotEnter:
						err = hotSpotEnter(vr, hotSpotID);
						break;
					case QTVRConstants.kQTVRHotSpotWithin:
						err = hotSpotWithin(vr, hotSpotID);
						break;
					case QTVRConstants.kQTVRHotSpotLeave:
						err = hotSpotLeave(vr, hotSpotID);
						break;
				}
			} catch (QTVRException e) {
				e.printStackTrace();
			}
			return err;	// continue
		}
		
		// hotSpotEnter
		int hotSpotEnter(QTVRInstance vr, int hotSpotID) throws QTVRException {
			int err = 0;
			hsEnterFOV = vr.getFieldOfView();  // save the entering FOV
			int speed = 5;
			switch (hotSpotID % 3) {
				case 0:		speed = 2;	break;
				case 1:		speed = 5;	break;
				case 2:		speed = 8;	break;
			}
			
			System.out.println("Enter hotSpot #" + hotSpotID
				+ ":" 
				+ " speed=> " + speed
				+ " FOV= " + hsEnterFOV
			);
			vr.setInteractionPanTiltSpeed(speed);
			return err;
		}

		// hotSpotWithin : 'link' type auto trigger hotspot if FOV -> < enterFOV / 2
		// hotSpotWithin : else if FOV -> < enterFOV / 2, FOV -> hsEnterFOV
		int hotSpotWithin(QTVRInstance vr, int hotSpotID) throws QTVRException {
			int err = noErr;
			float FOV = vr.getFieldOfView();
			if (FOV < (hsEnterFOV*0.75)) {
				switch (vr.getHotSpotType(hotSpotID)) {
					case QTVRConstants.kQTVRHotSpotLinkType:
						System.out.println(
							" auto trigger hotspot on zoom in");
						vr.setFieldOfView(hsEnterFOV); // better user experience
						vr.triggerHotSpot (hotSpotID, null, null);
						err = userCanceledErr;
						break;
					case QTVRConstants.kQTVRHotSpotURLType:
						break;
					default:
						break;
				}
			}
			return err;
		}

		// hotSpotLeave : reset the FOV, speed
		int hotSpotLeave(QTVRInstance vr, int hotSpotID) throws QTVRException {
			int err = 0;
			int panTiltSpeed = kDefaultPanTiltSpeed;
			System.out.println(
				" Leave hotSpot #" + hotSpotID
				+ ": Reset... "
				+ " speed=> " + panTiltSpeed
				+ " FOV=> " + hsEnterFOV
			);
			vr.setFieldOfView(hsEnterFOV); // reset the FOV to saved value when hotspot entered
			vr.setInteractionPanTiltSpeed(panTiltSpeed);
			return err;
		}
	}			
	
	/**
	 * Utility to check QTJava version.
	 */
	static void checkQTJavaVersion(int version, int subVersion, int qualifyingSubVersion) throws QTException {
		// express current build and the required versions of QTJava as comparable values
		int build = (QTBuild.getVersion() <<16) + (QTBuild.getSubVersion() <<8) + QTBuild.getQualifyingSubVersion();
		int require = (version <<16) + (subVersion <<8) + qualifyingSubVersion;
		if (build < require) {
			throw new QTException(
				"You need QTJava " 
				+ version + "." +
				+ subVersion + "."
				+  qualifyingSubVersion + " or better");
		}
	}
}
