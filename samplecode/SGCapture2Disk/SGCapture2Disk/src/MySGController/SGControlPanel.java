/*
	File:		SGControlPanel.java
	
	Description:	UI controls and record logic for the SGCapture2Disk sample.

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

package MySGController;
 
import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.io.*;
import quicktime.util.*;
import quicktime.*;
import quicktime.app.*;
import quicktime.std.*;
import quicktime.std.sg.*;

import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.std.StdQTConstants;
import quicktime.app.display.QTCanvas;
import quicktime.app.image.*;
import quicktime.qd.*;
import quicktime.app.sg.SGDrawer;

import MyPlayMovie.PlayMovie;


public class SGControlPanel extends Panel implements Runnable, Errors, StdQTConstants {

//_________________________ INSTANCE VARIABLES
	public 	Button 			powerOn 	  = new Button("Power");	
	public 	Button 			recordButton  = new Button("Record");
	public 	Button 			stopButton    = new Button("Stop");
	
	public	QDGraphics 		theGraphics;
	public  SequenceGrabber mGrabber = null;
	public 	QTCanvas 		theQTCanvas;
	public  SGVideoChannel 	mVideo;
	public  SGDrawer		mDrawer;
	public  QTFile			mFile;
	public  boolean  		isPowerOn = false;

	static final int kWidth = 320;
	static final int kHeight = 240;

//_________________________ CLASS METHODS
	
	public void showMovie() {
		PlayMovie pm = new PlayMovie("Captured Movie",mFile);
		pm.show();
		pm.toFront();
	}


	public void createNewMovie() {
		try {
				Container c = getParent();
				for (;;) {
					if (c instanceof Frame) break;
					c = c.getParent();
				}


				FileDialog fd = new FileDialog ((Frame) c, "Save Movie As...", FileDialog.SAVE);
				fd.show();
				
				if(fd.getFile() == null)
					throw new QTIOException (userCanceledErr, "");
					
				mFile = new QTFile(fd.getDirectory() + fd.getFile());
				
				}
				catch (QTException qte) {
					System.out.println("Error in makeMovie: ");
					qte.printStackTrace(); 
				}
	}
	
	
// See the Comments below under the recordButton action listner to see why this run method is here.
// Basically this is here as a workaround for using the SequenceGrabber on Windows Systems.

	public void run () {
		try {
			Thread.sleep(10);
			mDrawer.startTasking();
		} catch (Exception e) {
			 e.printStackTrace();
	    }
	}


	public void powerUp() throws QTException {
			
			QDRect bounds = new QDRect(kWidth, kHeight);
					
			mGrabber = new SequenceGrabber();
							
			mVideo = new SGVideoChannel(mGrabber);
			//Bring up the sequence Grabber settings Dialog and make sure 
			//you are receiving a clear image in the preview dialog.
			mVideo.settingsDialog ();

			mVideo.setBounds (new QDRect(kWidth, kHeight));
			mVideo.setUsage (seqGrabPreview | seqGrabRecord | seqGrabPlayDuringRecord);
			
			mGrabber.prepare(true,true);				
			mDrawer = new SGDrawer(mVideo);
			theQTCanvas.setClient(mDrawer,true);	//Uncomment to have the sequence grabber draw in the window					
			mGrabber.startPreview();	
			
			isPowerOn = true;
	}
	
	
	public void doAction(int what) {
	   try {
			mGrabber.pause(what);
		}
		
		catch (QTException e) {
			e.printStackTrace();
		}
		
	}
	
	public SGControlPanel(QTCanvas myQTCanvas) throws QTException {	

		theQTCanvas = myQTCanvas;
				
		setLayout(new FlowLayout());
		setBackground(getBackground());
		setLayout(new FlowLayout());
		

		add (powerOn);
		add (recordButton);
		add (stopButton);
		
		QTRuntimeException.registerHandler (new Handler());


		powerOn.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {

				try {
					powerUp();
				} catch (QTException err) {
					err.printStackTrace();
				}
				System.out.println("Powered UP");
			}
		});
		
		recordButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try{
				
					if (isPowerOn == false) {
						System.out.println("You Must first Click The Power button");
						return;
					}
				
				  	mDrawer.stopTasking();
				  	mGrabber.stop();
					createNewMovie();
				
					if (mFile != null) {
						mGrabber.setDataOutput(mFile,seqGrabToDisk);
						mGrabber.startRecord();
						
						//This code of spawning a new thread is here as a workaround for MS Windows 98 Systems.
						//It appears as though a race condition could occur where the setDataOutput does not fully complete
						//execution before the task object for the sequence grabber would try to idle it.
						//As a result of the race condition a -9402 exception (cantDoThatInCurrentMode) would be thrown.
						//-9402 in some instances means that we are trying to set a parameter during a record operation
						//This is a harmless error, but because a runtime exception gets thrown, the task object
						//for the SGDrawer (which is responsible for calling SGIdle() internally )  would terminate the objects
						//tasker. Starting a new thread which simply starts the SGDrawer object idling seems to fix this problem.
						//
						new Thread (SGControlPanel.this).start();			
					}
				} catch (QTException ee){
					ee.printStackTrace();
				}	
			}
		});		

		stopButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try {
				
					if (isPowerOn == false) {
						System.out.println("You Must first Click the Power button");
						return;
					}
				  	mDrawer.stopTasking();
					mGrabber.stop();
				System.out.println("*** BEFORE START PREVIEW ***");
				  	mGrabber.startPreview();
				  	mDrawer.startTasking();
				  	
				  	showMovie();

		    	} catch (QTException ee) {
					ee.printStackTrace();
			    }
			}
		});		

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
