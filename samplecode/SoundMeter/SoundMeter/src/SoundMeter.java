/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.io.IOException;

import quicktime.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.app.display.*;
import quicktime.app.players.*;
import quicktime.app.time.*;


public class SoundMeter extends Frame implements Errors {

	public static void main(String args[]) {

		try {
			QTSession.open();
			SoundMeter movieWindow = new SoundMeter("QT in Java");
			movieWindow.show();
			movieWindow.toFront();
			movieWindow.openMovie (movieWindow.getMovie());	
		} catch (Exception ex) {
			ex.printStackTrace();
			QTSession.close();
		}
	}

	private QTDrawable myPlayer;
	private QTCanvas myQTCanvas;	
	private AudioMediaHandler audioMediaHandler;
	private Track theTrack;
	private Timer timer;
	
	public SoundMeter (String title) throws QTException {
		super (title);
		myQTCanvas = new QTCanvas();
					 			
		add(myQTCanvas);
		Button b = new Button ("Get Movie");
		b.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent ae) {
				openMovie (getMovie());
			}
		});
		add (b, "North");
							
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				try {
					timer.setActive(false);
				} catch (QTException ex) {}
				QTSession.close();
				System.exit(0);	
			}
			
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}
	
	private QTFile getMovie () {
		try {
			return QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
		} catch (QTException e) {}
		return null;
	}
	
	public void openMovie (QTFile qtf) {		
			if (qtf == null) 
				return;
		try {
			Movie mov = Movie.fromFile (OpenMovieFile.asRead (qtf));
			
			// cleanup
			if (myPlayer != null) {
				myQTCanvas.removeClient();
				if (timer != null)
					timer.setActive (false);
			}
			
			myPlayer = new QTPlayer (new MovieController (mov));
			myQTCanvas.setClient (myPlayer, true);
		
			int trackCount = mov.getTrackCount();		
			
			for( int i = 1; i <= trackCount; i++) {
				theTrack = mov.getTrack(i);
				Media media = Media.fromTrack (theTrack);;
				MediaHandler handler = media.getHandler();
				
				
				if (handler instanceof AudioMediaHandler) {
					int numBands = 52;
					int[] freq = new int [numBands];
					freq[0] = 250;
					for (int k = 1; k < numBands; k++) 
						freq[k] = freq[k-1] + 500;
					
					/*freq[0] = 50;
					freq[1] = 125;
					freq[2] = 200;
					freq[3] = 400;
					freq[4] = 800;
					freq[5] = 1000;
					freq[6] = 2000;
					freq[7] = 4000;
					freq[8] = 8000;
					freq[9] = 12000;
					freq[10] = 18000;
					freq[11] = 26000;*/
					
					audioMediaHandler = (AudioMediaHandler)handler;
					//set up the timer to display the freq info
					timer = new Timer (1, 1, new StatusPrinter (audioMediaHandler, numBands, freq), mov) ;

					MediaEQSpectrumBands eqm = new MediaEQSpectrumBands (numBands);
/*					eqm.setFrequency (0, 55);
					eqm.setFrequency (1, 110);
					eqm.setFrequency (2, 220);
					eqm.setFrequency (3, 440);
					eqm.setFrequency (4, 880);
					eqm.setFrequency (5, 1760);
					eqm.setFrequency (6, 3520);
					eqm.setFrequency (7, 7040);
					eqm.setFrequency (8, 14080);
*/
					for (int j = 0; j < numBands; j++) 
						eqm.setFrequency (j, freq[j]);

				// this sets up the eq
				// NOTE: MUST call set first or eq won't be enabled correctly				
					audioMediaHandler.setSoundEqualizerBands (eqm);
					System.out.println (audioMediaHandler.getSoundEqualizerBands(numBands));
					
					audioMediaHandler.setSoundLevelMeteringEnabled (true);
					System.out.println ("Metering enabled:" + audioMediaHandler.getSoundLevelMeteringEnabled());
					
			//		int[] bt = audioMediaHandler.getSoundBassAndTreble();
			//		System.out.println ("Balance :" + audioMediaHandler.getBalance () + " Bass :" + bt[0] +  " Treble : " + bt[1] + " TrackVolume : " + theTrack.getVolume() + "\n");
					
					timer.setActive (true);			
					pack();
				}
			}
					System.out.println ("MOVIE:" + qtf.getName());
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	// example of how to do this
	private void setBassTreble() throws QTException {
		System.out.println ("Set Balance, Bass, Treble, TrackVolume");
		theTrack.setVolume(0.1F);
		audioMediaHandler.setSoundBassAndTreble(10, 0);
		audioMediaHandler.setBalance (0.5F);
	}	
}
