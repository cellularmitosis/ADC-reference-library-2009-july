/*
 
 File: SoundRecord.java
 
 Abstract: Records audio input using the QuickTime sequence grabber
 
 Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright © 2006 Apple Computer, Inc., All Rights Reserved
 
 */ 

import java.awt.*;
import java.awt.event.*;

import quicktime.*;
import quicktime.util.QTUtils;
import quicktime.io.*;

import quicktime.std.StdQTConstants;
import quicktime.std.sg.*;
import quicktime.std.movies.*;

public class SoundRecord extends Frame implements StdQTConstants, Runnable, Errors {	
	
	public static void main (String args[]) {
		SoundRecord sr = new SoundRecord ("Sound Recording");
		sr.show();
		sr.toFront();
	}

	
	void createSGObject() throws QTException {
			mGrabber = new SequenceGrabber();					
			mAudio = new SGSoundChannel (mGrabber);
			
			SGDeviceList dl = mAudio.getDeviceList (0);
			SGDeviceName name = dl.getDeviceName (0);
			
			System.out.println (dl + "" +  name);
			System.out.println (mAudio.getInputDriver());
			
			mAudio.setUsage (seqGrabRecord);
			t = new Thread (this,"SoundRecord Idle Method");			
	}
	
	
	SoundRecord (String title) {
		super (title);
		
		try {
			QTSession.open();
            System.out.println ("Make sure you have connected an Input device or have a built in audio input device.");
			createSGObject();
			
		} catch (Exception ee) {
			ee.printStackTrace();
			QTSession.close();
		}

		setLayout(new GridLayout(1, 3, 2, 2));
		add (startButton);
		add (settingsButton);

		startButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try{

					createMovie();
					
					if (mGrabber == null)
						createSGObject();
						
					mGrabber.setDataOutput (recordFile, seqGrabToDisk);
					mGrabber.prepare(true, true);
					mGrabber.startRecord();
					t.start();
					recording = true;
					recordNotice = new Frame("Recording");
					
					recordNotice.setLayout(new GridLayout(1, 3, 2, 2));
					recordNotice.add (stopButton);

			stopButton.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					if (recording) {
						try {
							tStop = true;
							mGrabber.stop();	//stop record
							recordNotice.hide();
							recordNotice = null;
							recording = false;

							mGrabber.disposeChannel (mAudio);
							mAudio = null;
							
							mGrabber.release();
							mGrabber = null;
						  	showMovie();
				    	} catch (QTException ee) {
							ee.printStackTrace();
					    }
					}
				}
			});					
					
					recordNotice.setBounds (40, 40, 200, 100);
					recordNotice.show();
					recordNotice.toFront();
				} catch (QTException ee){
					ee.printStackTrace();
				}	
			}
		});
		settingsButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try{
					if (mAudio == null) {
						createSGObject();
					}

					if (recording == false)
						mAudio.settingsDialog ();
				} catch (QTException ee){
					ee.printStackTrace();
				}	
			}
		});
		
//		t = new Thread (this);
		
		addNotify();
		Insets insets = getInsets();
		setBounds (0, 0, (insets.left + insets.right + 220), (insets.top + insets.bottom + 16));
		addWindowListener (new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				try {
				 if (mGrabber != null) 
					mGrabber.stop();	//stop preview
				
					if (recording) 
						tStop = true;
					
					if (mAudio != null)
						mGrabber.disposeChannel (mAudio);
						
				} catch (QTException ee) {
					ee.printStackTrace();
				}
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}
	
	private SGSoundChannel mAudio;
	private Button startButton = new Button("Record");
	private Button stopButton  = new Button("Stop");
	private Button settingsButton = new Button("Settings");
	private SequenceGrabber mGrabber;
	private Thread t;
	private boolean recording = false;
	private QTFile recordFile;
	private Frame recordNotice;
	private boolean tStop = false;
    	
	void createMovie () throws QTException {
	// We need to run the GC here to clean up any objects that
	// have been allocated. The Play movie frame does
	// not deallocate QT objects within that frame until the 
	// frame is disposed of completely 
		QTUtils.reclaimMemory();
		FileDialog fd = new FileDialog (this, "Save Movie As...", FileDialog.SAVE);
		fd.show();
		if(fd.getFile() == null)
			throw new QTIOException (userCanceledErr, "");
		recordFile = new QTFile(fd.getDirectory() + fd.getFile());
	}
	
	void showMovie () {
		PlayMovie pm = new PlayMovie(recordFile);
		pm.show();
		pm.toFront();
	}
	
	public void run () {
		try {
			for (;;) {
				mGrabber.idle();
				Thread.sleep(20);
                if (tStop == true)
                    break;
			}
		} catch (Exception e) {
			 e.printStackTrace();
	    }
	}
}
