/*
 * QuickTime for Java SDK Sample Code
 *
 * Usage subject to restrictions in SDK License Agreement
 * Copyright: © 2000 Apple Computer, Inc.
 */
 
import java.awt.*;
import java.awt.event.*;
import quicktime.*;
import quicktime.app.display.QTCanvas;
import quicktime.app.sg.*;
import quicktime.io.QTFile;
import quicktime.qd.*;

/** This example demonstrates how to use the QuickTime 5 broadcasting API to do
  * audio and video broadcasting from a Java Application
  */
public class BroadcastDrawer extends Frame {
	static public int WIDTH = 500;		// constants defining the height and width of the main window
	static public int HEIGHT= 300;
	
	/* constructor- creates the QTCanvas and creates some AWT components for controlling the broadcast */
	public BroadcastDrawer( String s ){
		super(s);
		setResizable(true);
		setBounds( 0, 0, WIDTH, HEIGHT);
		theCanvas = new QTCanvas (QTCanvas.kFreeResize, 0.5F, 0.5F);
		
		/* creating and intializing AWT components */
		setLayout( new BorderLayout() );
		add (theCanvas, "Center");
		startBtn = new Button ("Start Broadcast");
		ButtonListener listener = new ButtonListener();
		startBtn.addActionListener (listener);
		Panel statusPanel = new Panel();
		add (statusPanel, "South");
		statusPanel.add( startBtn );
		statusPanel.setBackground( Color.lightGray );
		Label timeBaseLabel = new Label( "Time base:" );
		timeBaseLabel.setLocation( 10, 10 );
		Label rateLabel = new Label("Data rate:" );
		rateLabel.setLocation( 10, 50 );
		currTimeLabel = new Label( "00:00:00.00  " );
		currTimeLabel.setLocation( 80, 10 );
		currRateLabel = new Label("0       ");
		currRateLabel.setLocation( 80, 50 );
		statusPanel.add(timeBaseLabel);
		statusPanel.add(currTimeLabel);
		statusPanel.add(rateLabel);
		statusPanel.add(currRateLabel);

		addWindowListener( new WindowAdapter() 	//	handle quit when user closes main window
        {
            public void windowClosing( WindowEvent we )
            {
            	stopBroadcast();		
                QTSession.close();		
                dispose();					
            }
            public void windowClosed( WindowEvent we )
            {
                System.exit( 0 );		
            }
        });
	}
	
	/* Main entry point- open session, create instance of class, prepare for Broadcasting */
	public static void main (String[] args) {
		try {
			QTSession.open();
			BroadcastDrawer app = new BroadcastDrawer( "Broadcaster API Test" );
			app.show();
			app.toFront();
			app.prepareBroadcast();
		}
		catch (Exception e) {
			QTSession.close();	
			e.printStackTrace();
		}
	}	
		
   /** Prepare for broadcasting
	*  Display the file selection dialog to select an sdp file and show the settings dialog
	*/	
	public void prepareBroadcast() {
		theDialog.show();
		try {
			thePres= new SimplePres( new QTFile( theDialog.getDirectory() + theDialog.getFile()), theCanvas );
			setTitle(theDialog.getFile());
			theCanvas.setClient( thePres.pDrawer, true );

			if (drawer == null)
			{
				drawer = new StatDrawer(thePres);	// draws the rate and time information
				drawer.timeLabel = currTimeLabel;
				drawer.rateLabel = currRateLabel;
			}
			else
				drawer.setPres(thePres);
		}
		catch (QTException qte) {
			qte.printStackTrace();
		} 
	} 
	
	/**
	 * Stops the broadcast and all associated threads 
	 */
	public void stopBroadcast(){
		thePres.stopBroadcast();
		startBtn.setLabel("Start Broadcast");
		drawer.stopTasking();
		isPlaying = false;
		currRateLabel.setText("0          ");
	}
	
	/**
	 * Starts the broadcast and all associated threads
	 */
	public void startBroadcast() {
		thePres.startBroadcast();
		startBtn.setLabel("StopBroadcast");
		drawer.startTasking();
		isPlaying = true;
	}
	
	/**
	 * Respond to user interaction with the stop/start button
	 */
	public class ButtonListener implements ActionListener {
		public void actionPerformed (ActionEvent e) {
			if (isPlaying) 
				stopBroadcast( );	
			else 
				startBroadcast();
		}
	}
	
	/* --- private data members --- */
	private Button		startBtn;			// AWT start button
	private boolean 	isPlaying = false;	// internal state variable
	private Label 		currTimeLabel;		// current time label
	private Label 		currRateLabel;		// current data rate label
	private StatDrawer	drawer = null;		// statistics drawer
	private SimplePres  thePres;			// simple presenter object
	private QTCanvas 	theCanvas;			// QTCanvas
	private FileDialog 	theDialog = new FileDialog(this, "Choose an SDP File for the Broadcast", FileDialog.LOAD);
}