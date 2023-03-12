/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.qd.*;
import quicktime.*;
import quicktime.io.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.std.image.*;
import quicktime.app.QTFactory;
import quicktime.app.display.*;
import quicktime.app.players.*;
import quicktime.app.image.*;
import quicktime.app.actions.*;
import quicktime.app.anim.*;

public class MediaPresenter extends Frame implements StdQTConstants {	

	public static void main (String args[]) {
		try {
			QTSession.open();
			MediaPresenter gd = new MediaPresenter("QT in Java");
			
			gd.pack();
			gd.show();
			gd.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	
	MediaPresenter (String title) throws Exception {
		super (title);
		
		setBackground (Color.white);
		
		QTCanvas myQTCanvas = new QTCanvas ();
		add ("Center", myQTCanvas);
		add ("North", new ControlPanel (this));
		
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});

		int kWidth = 300, kHeight = 300;
		QDDimension d = new QDDimension (kWidth, kHeight);
		groupDrawer = new DirectGroup (d, QDColor.white);
			
		myQTCanvas.setClient (groupDrawer, true);
		
		groupDrawer.getTimer().setRate (1);
	}
	
	DirectGroup groupDrawer;
	QTDrawable myDrawer;
	
	void stopPlayer () throws QTException {
		if (myDrawer instanceof Playable)
			((Playable)myDrawer).setRate(0);
	}
	
	void openFromURL (String url) throws QTException {
		QTDrawable d = QTFactory.makeDrawable (url);

		groupDrawer.removeMember (myDrawer);
		myDrawer = null;
		
		groupDrawer.addMember (d);
		myDrawer = d;
	}
	
	private void redrawDrawer () throws QTException {
		groupDrawer.memberChanged (myDrawer);

		if (myDrawer instanceof QTPlayer) {
			((QTPlayer)myDrawer).getMovieController().movieChanged();
		}	
		groupDrawer.redraw (null);
	}
		
	boolean hasDrawer () { return myDrawer != null; }
	
	void reset () throws QTException {
		myDrawer.setMatrix (new Matrix());
		redrawDrawer ();
	}
	
	void setScaling (float scale) throws QTException {
		Matrix mat = myDrawer.getMatrix();
		mat.scale (scale, scale, 0, 0);
		myDrawer.setMatrix (mat);
		redrawDrawer ();
	}		
		
	void rotateDrawer () throws QTException {
		Matrix mat = myDrawer.getMatrix();
		
		// rotate 90 degress and keep origin at 0,0 of group
		// this rotation bounds stuff ONLY works currently
		// with 90 degree rotations - there are some bounds stuff
		// on the matrix class which WILL be added to this code
		// so that any rotation can be done and the result positioned at top / left
		float rx = 0;
		float ry = 0;
		mat.rotate (90, rx, ry);
	
		if (myDrawer instanceof QTPlayer) {
			myDrawer.setMatrix (mat);
			myDrawer.setLocation (0, 0);
		} else {
			QDDimension d = myDrawer.getOriginalSize();
			if (mat.getSx() == 0) {
				if (mat.getB() > 0) {
					float xLoc = mat.getC() * -d.getHeight();
					mat.setTx (xLoc);
					mat.setTy (0);
				} else {
					float yLoc = mat.getB() * -d.getWidth();
					mat.setTy (yLoc);
					mat.setTx (0);
				}
			} else {
				if (mat.getSx () > 0)
					mat.setTx (0);
				else
					mat.setTx (d.getWidth() * Math.abs (mat.getSx()));
				if (mat.getSy () > 0)
					mat.setTy (0);
				else
					mat.setTy (d.getHeight()* Math.abs (mat.getSy()));
			}
			myDrawer.setMatrix (mat);
		}
		
		redrawDrawer ();
	}
}


class ControlPanel extends Panel {
	ControlPanel (MediaPresenter mp) {
		myPresenter = mp;
		
		openButton.addActionListener (new ActionListener () {
			public void actionPerformed(ActionEvent event) {					
					// creat a movie through the file-open dialog of QT
				try {
					myPresenter.stopPlayer();
					QTFile qtf = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
					myPresenter.openFromURL ("file://" + qtf.getPath());
					enableButtons ();
				} catch (QTException e) {
					if (e.errorCode() != Errors.userCanceledErr)
		 				e.printStackTrace();
		 		}
		 	}
		});
		rotateButton.addActionListener (new ActionListener () {
			public void actionPerformed(ActionEvent event) {					
				try {
					myPresenter.rotateDrawer ();
				} catch (QTException e) {
						// with rotated members of a direct group
						// a QuickDraw bug will occassionally throw
						// this error - it can be ignored
					if (e.errorCode() != Errors.rgnTooBigErr)
						e.printStackTrace();
		 		}
		 	}
		});
		scaleField.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				float f = Float.valueOf (new String (event.getActionCommand())).floatValue();
				try {
					myPresenter.setScaling (f);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
		});
		resetButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try {
					myPresenter.reset ();
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
		});


		add (openButton);	
		add (rotateButton);
		add (scaleLabel);
		add (scaleField);
		add (resetButton);
		
		enableButtons();
	}
	
	void enableButtons () {
		rotateButton.setEnabled (myPresenter.hasDrawer());
		scaleField.setEnabled (myPresenter.hasDrawer());
		resetButton.setEnabled (myPresenter.hasDrawer());
	}

	public Dimension getPreferredSize () {
		return new Dimension (80, 80);
	}
	
	MediaPresenter myPresenter;
	
	Button openButton = new Button ("Open File...");
	Button rotateButton = new Button ("Rotate");
	Button resetButton = new Button ("Reset");
	Label scaleLabel = new Label ("Scale");
	TextField scaleField = new TextField (8);
}