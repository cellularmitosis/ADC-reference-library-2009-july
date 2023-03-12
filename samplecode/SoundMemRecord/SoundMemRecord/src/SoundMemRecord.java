/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;

import quicktime.*;
import quicktime.sound.*;
import quicktime.util.*;

public class SoundMemRecord extends Frame {	
	private static int
		kOSType =  1 << 1;
	private static int sndBufferSize = 327680;
	
	public static void main (String args[]) {
            System.out.println ("Make sure you have connected an Input device or have a built in audio input device.");
		SoundMemRecord sr = new SoundMemRecord ("Sound Recording");
		sr.show();
		sr.toFront();
	}
		
		// the sndHndl is kept around as it must be cached while we're using it
	SndHandle sndHndl;
	SPBDevice sndDevice;
	SPB recorder;
	
	SoundMemRecord (String title) {
		super (title);
		try {
			QTSession.open();
			sndDevice = new SPBDevice (null, SoundConstants.siWritePermission);
			System.out.println ("OptionsDialog==" + sndDevice.hasOptionsDialog());
			System.out.println ("NumChannels=" + sndDevice.getChannelAvailable());
			System.out.println ("SampleSize=" + sndDevice.getSampleSize());
			System.out.println ("SampleRate=" + sndDevice.getSampleRate());
			printArray ("Names", sndDevice.getInputSourceNames());
			printArray("Compression", sndDevice.getCompressionAvailable(), kOSType);
			printArray("SampleSize", sndDevice.getSampleSizeAvailable(), 0);
			printArray("SampleRate", sndDevice.getSampleRateAvailable());
			sndDevice.setPlayThruOnOff (0);
			sndHndl = new SndHandle (sndDevice.getNumberChannels(), 
											sndDevice.getSampleRate(), 
											sndDevice.getSampleSize(), 
											sndDevice.getCompressionType(), 
											60);
			sndHndl.appendSoundBuffer (sndBufferSize);	//this is the size of our sound buffer


// Set up recording to reuse object for multiple recordings
	// we hold onto this variable so that it isn't finalized before we
	// finish recording
	// we also need to remove the completion proc as we shut this down - see closingWindow
			System.out.println ("Will Record:" + sndDevice.bytesToMilliseconds(sndBufferSize) + " msecs");
			recorder = new SPB(sndDevice, 0, sndDevice.bytesToMilliseconds(sndBufferSize), sndHndl.getSoundData());
	
	// We're going to record this block ASynchronously
	// and we're installing a completion proc to notify us when
	// the recording is finished				
			recorder.setCompletionProc (new SICompletion () {
				final SndHandle soundHdl = sndHndl;
				final SPBDevice device = sndDevice;
						
				public void execute (SPB paramBlock) {
					System.out.println ("FinishedRecording");
					try{					
			// set up sndHndl after recording is finished so we cam play it
						soundHdl.setupHeader (device.getNumberChannels(), 
											device.getSampleRate(), 
											device.getSampleSize(), 
											device.getCompressionType(), 
											60,
											paramBlock.getCount());
					} catch (QTException ee){
						ee.printStackTrace();
					}	
				}
			});

		} catch (Exception ee) {
			ee.printStackTrace();
			QTSession.close();
		}
		

		setLayout(new GridLayout(1, 3, 2, 2));

		startButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try{					
					System.out.println ("StartRecording");
					recorder.record (true);
				} catch (QTException ee){
					ee.printStackTrace();
				}	
			}
		});
		add (startButton);

		playRecordedButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try{
					if (sndHndl != null) {
		// As with all other callbacks the application is reponsible for 
		// disposing of the callback - in this case that means disposing the sound channel.
		// So if using a callback the last thing you should do 
		// is install a callback command on the channel and dispose it when that guy fires
		// see SoundAction ....
						SndChannel sndChan1 = new SndChannel (new SoundCallBack () {
							SndHandle theSnd = sndHndl;
							
							public void execute (SndChannel sc) {
								try {
										System.out.println ("In Execute:" + sc);
									if (theSnd != null) {
										SndInfo info = theSnd.parseSndHeader ();
										SoundComponentData data = info.sndData;
										
											//shows use of buffer command
											// this call will LOCK the handle - it must stay locked whilst the buffer command is active
										QTPointerRef sndData = theSnd.getSoundData();
										ExtScheduledSoundHeader buffer = new ExtScheduledSoundHeader (sndData,//data.getBuffer(),
																						data.getNumChannels(),
																						data.getSampleRate(),
																						data.getSampleSize(),
																						data.getFormat());
										SndCommand bufferCmd = new SndCommand (SoundConstants.bufferCmd);
										bufferCmd.setBuffer (buffer);
										sc.doCommand (bufferCmd);
										
										theSnd = null;
											// reschedule callback to dispose the SndChannel
											// do it this way so that we don't have to wait
											// until the sound is finished playing
										sc.doCommand (new SndCommand (SoundConstants.callBackCmd));
										System.out.println ("Replay the sound just for fun:" + sc);
									} else {
										System.out.println ("Finished Playing:" + sc);
										sc.disposeQTObject();//finished with SndChannel so we MUST dispose it
									}
									
								} catch (Exception e) {
									e.printStackTrace();
								}
							}
						});
		// this plays asynchronously
		// we issue a callback on the SndChannel which will 
		// fire AFTER the snd play has completed
						sndChan1.play (sndHndl);
						sndChan1.doCommand (new SndCommand (SoundConstants.callBackCmd));
						System.out.println ("finished scheduling play and callback:" + sndChan1);
					}
				} catch (Exception ee){
					ee.printStackTrace();
				}	
			}
		});
		add (playRecordedButton);
		
		playMyDataButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try{
					System.out.println ("Play constructed data");
					Thread.yield(); // make sure we get print feedback!!!
					
				// CONSTRUCT a Sound in memory and play it
						//use this constructor then do setup header with data size
					SndHandle tempSndHndl = new SndHandle ();

					byte[] media = new byte[64000];
					short val = -32768;

					for (int i = 0; i < media.length; i+=2) {
						if (val > 32750)
							val = -32768;
						val+=16;
						media[i] = (byte)((val | 0xFF00) >>> 8);
						media[i + 1] = (byte)val;	
					}

					// MUST do this first
					tempSndHndl.setupHeader (1, 11050, 16, SoundConstants.k16BitBigEndianFormat, 0, media.length);
						
						// either way works
					if (false) {
							// allocates the extra memory for the sound buffer in the SndHandle
						tempSndHndl.appendSoundBuffer (media.length);
						QTPointerRef qtpr=tempSndHndl.getSoundData();
						qtpr.copyFromArray (0,media,0,media.length);
					} else {
						QTPointerRef qtpr= new QTPointer (media.length, true);
						qtpr.copyFromArray (0,media,0,media.length);
							// sets the data directly
						tempSndHndl.setSoundData (qtpr);
					}
					
					Sound.play(tempSndHndl);

					System.out.println ("Finished Playing");
				} catch (Exception ee){
					ee.printStackTrace();
				}	
			}
		});
		add (playMyDataButton);

		pack();
		
		addWindowListener (new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				recorder.removeCompletionProc(); //clean this up as we installed it
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}
	
	private static void printArray (String prefix, int[] array, int printTypeFlag) {
		System.out.print (prefix + "=[");
		if (array.length == 0) {
			System.out.println ("]");
			return;
		}
		for (int i = 0; i < array.length - 1; i++) {
			if ((printTypeFlag & kOSType) != 0)
				System.out.print (QTUtils.fromOSType(array[i]) + ",");
			else
				System.out.print (array[i] + ",");
		}
		if ((printTypeFlag & kOSType) != 0)
			System.out.println (QTUtils.fromOSType(array[array.length-1]) + "]");
		else
			System.out.println (array[array.length-1] + "]");
	}
	
	private static void printArray (String prefix, float[] array) {
		System.out.print (prefix + "=[");
		if (array.length == 0) {
			System.out.println ("]");
			return;
		}
		for (int i = 0; i < array.length - 1; i++) {
			System.out.print (array[i] + "F,");
		}
		System.out.println (array[array.length-1] + "F]");
	}

	private static void printArray (String prefix, String[] array) {
		System.out.print (prefix + "=[");
		if (array.length == 0) {
			System.out.println ("]");
			return;
		}
		for (int i = 0; i < array.length - 1; i++) {
			System.out.print (array[i] + ",");
		}
		System.out.println (array[array.length-1] + "]");
	}

	private Button startButton = new Button("Record");
	private Button playRecordedButton = new Button("Play Recorded Data");
	private Button playMyDataButton = new Button("Play My Data");
	private boolean recording = false;
}
