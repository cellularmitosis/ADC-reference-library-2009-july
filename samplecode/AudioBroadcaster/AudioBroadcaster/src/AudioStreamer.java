/*
 * QuickTime for Java SDK Sample Code
 *
 * Usage subject to restrictions in SDK License Agreement
 * Copyright: © 2000 Apple Computer, Inc.
 */

import java.awt.*;
import java.awt.event.*;
import quicktime.*;
import quicktime.io.QTFile;
import quicktime.streaming.*;

/* This example demonstrates how to use the QuickTime 5 broadcasting API to broadcast audio without
   the use of a QTCanvas object. This makes it useful in an AWT or swing context */
public class AudioStreamer extends Frame {	
	/* Constructor- loads an sdp file, creates and starts the broadcast */
	public AudioStreamer( String s ) {
		super( s );
		setResizable(true);
		setBounds (0,0,400,200);
		
		setLayout (new FlowLayout());
		
		startBtn = new Button ("Start Broadcast");
		startBtn.setEnabled (false);
		configureBtn = new Button( "Configure Broadcast" );
		add(startBtn);
		add(configureBtn);
		
		ButtonListener listener = new ButtonListener();
		startBtn.addActionListener (listener);
		configureBtn.addActionListener (listener);
        theDialog = new FileDialog(this, "Choose an SDP File for the Broadcast", FileDialog.LOAD);
		
		addWindowListener( new WindowAdapter() 		
        {
            public void windowClosing( WindowEvent we )
            {
                broadcaster.stopBroadcast();
				dispose();
				QTSession.close();                					
            }
            public void windowClosed( WindowEvent we )
            {
                System.exit( 0 );		
            }
        });
	}
	
	/* main entry point of application */
	public static void main (String[] args) {
		try {
			QTSession.open();
			AudioStreamer app = new AudioStreamer( "Audio Streamer Test" );
			app.show();
			app.toFront();
		}
		catch (Exception e) {
			QTSession.close();	
			e.printStackTrace();
		}
	}
	
	/** Handles the start/stop and configure broadcast buttons */	
	public class ButtonListener implements ActionListener {
		public void actionPerformed (ActionEvent e) {	
			if (e.getSource() == startBtn)				// stop button
				if ( playing ) {
					broadcaster.stopBroadcast();
					startBtn.setLabel( "Start Broadcast" );				
				}										// start button
				else {
					broadcaster.startBroadcast();
					startBtn.setLabel( "Stop Broadcast" );
				}
			else if (e.getSource() == configureBtn) {	// configure button
				startBtn.setLabel( "Start Broadcast" );
				startBtn.setEnabled (false);
				theDialog.show();
                if (theDialog.getFile() == null) {
                    System.out.println ("Need to select valid .sdp file. Quitting...");
                    QTSession.close();
                    System.exit(1);
                }
				try {
					broadcaster = new Broadcaster( new QTFile( theDialog.getDirectory() + theDialog.getFile() ));
					broadcaster.prepareBroadcast();
				} catch (QTException ex) {
					ex.printStackTrace();
				}
				startBtn.setEnabled (true);
			}
		}
	}

	/* --- private data members --- */
	private boolean playing = false;
	private FileDialog 	theDialog;
	private Broadcaster broadcaster;
	private Button startBtn, configureBtn;
}