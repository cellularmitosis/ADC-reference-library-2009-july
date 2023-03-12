/*

File: MovieCallbacks.java

Abstract: Allows Java code to be called at specified points during movie playback. 

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
import java.io.IOException;

import quicktime.*;
import quicktime.io.*;
import quicktime.std.movies.*;
import quicktime.app.view.*;
import quicktime.std.*;
import quicktime.vr.*;
import quicktime.util.*;

public class MovieCallbacks extends Frame implements Errors {	
	
	public static void main (String args[]) {
		try {
			System.out.println ("Open a Multi-node VR Movie...");
			QTSession.open (QTSession.kInitVR);
				//register handler for QTRuntimeExceptions
			QTRuntimeException.registerHandler (new Handler());
			
			MovieCallbacks pm = new MovieCallbacks("QT in Java");
			pm.pack();
			pm.show();
			pm.toFront();
		} catch (QTException e) {
			if (e.errorCode() != Errors.userCanceledErr)
				e.printStackTrace();
			else
				System.out.println ("UserCanceled : Application needs media file to run. Quitting....");
            QTSession.close();
            System.exit(1);
		}
	}
	
	QTVRInstance myQTVRInstance;
	Movie myMovie;
	MovieController myMovieController;
	
	MovieCallbacks (String title) throws QTException {
		super (title);
		
		QTFile qtFile = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);

		OpenMovieFile movieFile = OpenMovieFile.asRead(qtFile);
		myMovie = Movie.fromFile (movieFile);
		myMovieController = new MovieController (myMovie);
		myMovieController.setKeysEnabled (true);
		
		Track track = myMovie.getQTVRTrack (1);
		if (track != null) {	//setup VR callbacks
			myQTVRInstance = new QTVRInstance (track, myMovieController);
			myQTVRInstance.setEnteringNodeProc (new EnteringNode(), 0);
			myQTVRInstance.setLeavingNodeProc (new LeavingNode(), 0);
			myQTVRInstance.setMouseOverHotSpotProc (new HotSpot(), 0);
			Interceptor ip = new Interceptor();
			myQTVRInstance.installInterceptProc (QTVRConstants.kQTVRSetPanAngleSelector, ip, 0);
			myQTVRInstance.installInterceptProc (QTVRConstants.kQTVRSetTiltAngleSelector, ip, 0);
			myQTVRInstance.installInterceptProc (QTVRConstants.kQTVRSetFieldOfViewSelector, ip, 0);
			myQTVRInstance.installInterceptProc (QTVRConstants.kQTVRSetViewCenterSelector, ip, 0);
			myQTVRInstance.installInterceptProc (QTVRConstants.kQTVRTriggerHotSpotSelector, ip, 0);
			myQTVRInstance.installInterceptProc (QTVRConstants.kQTVRGetHotSpotTypeSelector, ip, 0);
		}
		
		// set up movie drawing callback
		myMovie.setDrawingCompleteProc (StdQTConstants.movieDrawingCallWhenChanged, new MovieDrawing());
		// set up action filter
		myMovieController.setActionFilter (new PMFilter(), false);	//don't do idle events
		
                QTComponent mComponent = QTFactory.makeQTComponent(myMovieController);
                add((Component) mComponent);
                
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				try {	// remove callbacks we instantiated
					if (myQTVRInstance != null) {
						myQTVRInstance.removeEnteringNodeProc();
						myQTVRInstance.removeLeavingNodeProc();
						myQTVRInstance.removeMouseOverHotSpotProc();
						
						myQTVRInstance.removeInterceptProc (QTVRConstants.kQTVRSetPanAngleSelector);
						myQTVRInstance.removeInterceptProc (QTVRConstants.kQTVRSetTiltAngleSelector);
						myQTVRInstance.removeInterceptProc (QTVRConstants.kQTVRSetFieldOfViewSelector);
						myQTVRInstance.removeInterceptProc (QTVRConstants.kQTVRSetViewCenterSelector);
						myQTVRInstance.removeInterceptProc (QTVRConstants.kQTVRTriggerHotSpotSelector);
						myQTVRInstance.removeInterceptProc (QTVRConstants.kQTVRGetHotSpotTypeSelector);
					}
					myMovie.removeDrawingCompleteProc();
					myMovieController.removeActionFilter();
				} catch (QTException ex) {}
					
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}
	
	static class MovieDrawing implements MovieDrawingComplete {
		public int execute (Movie m) {
			System.out.println ("drawing:" + m);
			return 0;
		}
		
		public int execute (Movie m, int action, StringHandle sh) {
			System.out.println (m + ",action=" + action);
			return 0;
		}
	}	

	static class EnteringNode implements QTVREnteringNode {
		public int execute (QTVRInstance vr, int nodeID) {
			System.out.println (vr + ",entering:" + nodeID);
			return 0;
		}
	}
	 	
	static class LeavingNode implements QTVRLeavingNode {
		public int execute (QTVRInstance vr, int fromNodeID, int toNodeID, boolean[] cancel) {
			System.out.println (vr + ",leaving:" + fromNodeID + ",entering:" + toNodeID);
			// no error and Don't cancel leaving node
				// cancel[0] = true; -> this would cancel leaving the fromNode
			return 0;
		}
	}
	
	static class HotSpot implements QTVRMouseOverHotSpot {
		public int execute (QTVRInstance vr, int hotSpotID, int flags) {
			System.out.println (vr + ",hotSpot:" + hotSpotID + ",flags=" + flags);
			return 0;
		}
	}
	
	static class Interceptor implements QTVRInterceptor {
		public boolean execute (QTVRInstance vr, QTVRInterceptRecord qtvrMsg) {
			System.out.println (vr + "," + qtvrMsg);
			return false;	//dont cancel default execution
		}
	}

	static class PMFilter extends ActionFilter {
		public boolean execute (MovieController mc, int action) { 
			System.out.println (mc + "," + "action:" + action);
			return false; 
		}

		public boolean execute (MovieController mc, int action, float value) {
			System.out.println (mc + "," + "action:" + action + ",value=" + value);
			return false; 
		}
	}

	//_________________________ Runtime Error Handling
	static class Handler implements QTRuntimeHandler {
		public void exceptionOccurred (QTRuntimeException e, Object eGenerator, String methodNameIfKnown, boolean unrecoverableFlag) {
			System.out.println (eGenerator + "," + methodNameIfKnown + ",unrecoverable=" + unrecoverableFlag);
			e.printStackTrace();
			throw e;	// we don't handle this exception - just print stack trace and throw it
		}
	}
}


