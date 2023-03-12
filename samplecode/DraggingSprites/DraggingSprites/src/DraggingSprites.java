/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;
import java.io.IOException;
import java.io.File;

import quicktime.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.io.*;
import quicktime.std.image.*;
import quicktime.util.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.app.actions.*;
import quicktime.app.anim.*;
import quicktime.app.audio.*;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.QTFactory;

import ds.actions.*;

public final class DraggingSprites extends Frame implements StdQTConstants {
	static final int 
				kWidth = 400,
				kHeight = 300;
	
	// MIDI note numbers for drum kit instrument
	static final int
		kSnareDrum = 40,
		kClosedHiHat = 44,
		kOpenHiHat = 46,
		kCrashCymbal = 49,
		kRideCymbal = 51;

	public static void main (String[] args) {
		try {
			QTSession.open();
			new DraggingSprites().show();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	
//____________________ INSTANCE VARIABLES	
	private Compositor sd;
	private QTCanvas canv;
	
	private boolean performance = true;
	
//____________________ INSTANCE METHODS	
	DraggingSprites () throws QTException, IOException {
		super ("Sprite App");
		addNotify();
		Insets insets = getInsets();
		setBounds (0, 0, (insets.left + insets.right + kWidth), (insets.top + insets.bottom + kHeight));

		try {
			QTSession.open();

			setLayout (new BorderLayout());
			canv = new QTCanvas (QTCanvas.kInitialSize, 0.5F, 0.5F);
			add ("Center", canv);
			Dimension size = new Dimension(kWidth, kHeight);
			QDGraphics gw = new QDGraphics (new QDRect(size));
			sd = new Compositor (gw, QDColor.red, 20, 1);	
			File matchFile = QTFactory.findAbsolutePath ("images/Ship01.pct");	//this file must exist in the directory!!!	
			ImageDataSequence is = ImageUtil.createSequence (matchFile);
			createSprites (sd, ImageUtil.makeTransparent (is, QDColor.blue), size);
			sd.getTimer().setRate (2);

			canv.setClient (sd, true);
		} catch (Exception e) {
			e.printStackTrace();
			throw new QTRuntimeException (e);
		}

		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) {
				System.exit(0);
			}
		});
	}
				
	void createSprites (Compositor sd, ImageDataSequence seq, Dimension size) throws Exception {
		// This is the instrument that will be used to play all of the notes
		// of the different actions -> we use a polyphony of 10
		// which is a reasonable estimate of the maximum number of notes
		// that will be sounding at a given time
		final NoteChannelControl nc = new NoteChannelControl (StdQTConstants.kFirstDrumkit, 10);

		Matrix matrix = new Matrix();
		matrix.setTx(size.width / 4);
		matrix.setTy(size.height / 2);
		TwoDSprite s1 = new TwoDSprite(seq, matrix, true, 1);
		sd.addMember (s1);
		
        // paint a sprite and set a hot spot region for it
		// the green region is the hot spot
		// we use a 16 bit BigEndian pixel format as we have no blending on this sprite
		matrix = new Matrix();
		matrix.setTx(20);
		matrix.setTy(20);
		QDRect r40 = new QDRect (40, 40);
		QDRect r8 = new QDRect (1, 1, 8, 8);
		QDGraphics y = new QDGraphics (QDConstants.k16BE555PixelFormat, r40);
		y.setBackColor (QDColor.yellow);
		y.eraseRect (null);
		y.setForeColor (QDColor.green);
		y.paintRect (r8);
		EncodedImage ei = RawEncodedImage.fromPixMap (y.getPixMap());
		ImageDescription id = new ImageDescription (y.getPixMap());
		ImageDataSequence idsy = new ImageDataSequence (id);
		idsy.addMember (ei);
		if ((QTSession.isCurrentOS(QTSession.kWin32) && QTSession.getQTMajorVersion() == 3) == false)	//doesn't work on QT3.0.2 on Win
			idsy = ImageUtil.makeTransparent (idsy, QDColor.black, new QDGraphics (QDConstants.k16BE555PixelFormat, r40), new Region (r8));
		TwoDSprite s2 = new TwoDSprite(idsy, matrix, true, 10);
		sd.addMember (s2);

		matrix = new Matrix();
		matrix.setTx(size.width / 2);
		matrix.setTy(size.height / 2);
		TwoDSprite s3 = new TwoDSprite(seq, 15, matrix, true, 10);
		sd.addMember (s3);
		ImageSequencer is3 = new ImageSequencer (seq);
		is3.setCurrentFrame (15);	//keep in sync with sprite's current image
		is3.setLooping (ImageSequencer.kLoopForwards);

		SimpleActionList al = new SimpleActionList();
		ImageSequencer is = new ImageSequencer (seq);
		is.setLooping (ImageSequencer.kLoopForwards);
		al.addMember (new NextImageAction (15, 1, is, s1));
		BounceAction b1 = new BounceAction (20, 1, sd, s1, 1, 2);
		b1.setActionable (new Actionable () {
			public void trigger () {
				try {
					nc.playNoteFor (kCrashCymbal, 127, 20);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
		});
		al.addMember (b1);
					// 360 degrees at a scale of 20
		al.addMember (new RotateAction (360, 20, sd, s3));
					// 360/ (1/2)number of images changes at a scale of 20
		al.addMember (new NextImageAction ((360 / seq.size() / 2), 20, is3, s3));
		al.addMember (new BounceAction (1, 10, sd, s3, 1, 1)); // a slow drift
		sd.addController(al);

	//all mouse downs with NO modifiers pressed will drag all sprites
		Dragger dragger = new Dragger (MouseResponder.kNoModifiersMask);
		dragger.setActionable (new Actionable () {
			public void trigger () {
				try {
					nc.playNoteFor (kRideCymbal, 127, 80);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
		});
		SWController ct = new SWController (dragger, true);
		sd.addController (ct);
		
	//only shift key down when mouse pressed will allow sprite number 2 & 3 to be scaled when dragged
		MouseController controller = new SWController (new Scaler (400, InputEvent.SHIFT_MASK), false);
		controller.addMember (s3);
		controller.addMember (s2);
		sd.addController (controller);

	//only option/alt key will allow sprite number 3 to be skewed when dragged
		controller = new SWController (new Skewer (200, 200, InputEvent.ALT_MASK), false);
		controller.addMember (s2);
		sd.addController (controller);
		
		// this responder responds to rollover type events on sprites when NO modifiers are pressed
		final GenericResponder gr = GenericResponder.asRolloverListener (MouseResponder.kNoModifiersMask, MouseResponder.kModifiersExactMatch);
		// add a listener to play notes when rolled over 
		// 
		gr.addQTMouseMotionListener (new QTMouseMotionAdapter () {			
			// keep the notes playing even when we are being dragged
			// however this is NOT enough to properly handle drag events
			// - this is done be the Dragger-Controller pairing
			public void mouseDragged (MouseEvent event) {
				try {
					nc.playNoteFor (kClosedHiHat, 127, 80);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}

			public void mouseMoved (MouseEvent event) {
				try {
					nc.playNoteFor (kClosedHiHat, 127, 80);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
			
			public void mouseExitedTarget (MouseEvent event) {
				try {
					nc.playNoteFor (kOpenHiHat, 127, 80);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}

			public void mouseEnteredTarget (MouseEvent event) {
				try {
					nc.playNoteFor (kSnareDrum, 127, 80);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
		});
		// another listener that prints out the target on entry and the space on exit of a rollover occurance
		gr.addQTMouseMotionListener (new QTMouseMotionAdapter () {
			GenericResponder g = gr;
			GraphicsMode savedGM;
			GraphicsMode setGM = new GraphicsMode (QDConstants.blend, QDColor.lightGray);
			
			public void mouseEnteredTarget (MouseEvent event) {
				try {
					if (g.getTarget() instanceof TwoDSprite) {	// just for sanity
						TwoDSprite sprite = (TwoDSprite)g.getTarget();
						savedGM = sprite.getGraphicsMode();
						sprite.setGraphicsMode (setGM);
					}
				} catch (QTException e) {
					throw new QTRuntimeException (e);
				}
			}

			public void mouseExitedTarget (MouseEvent event) {
				try {
					if (g.getTarget() instanceof TwoDSprite) {	// just for sanity
						TwoDSprite sprite = (TwoDSprite)g.getTarget();
						sprite.setGraphicsMode (savedGM);
					}
				} catch (QTException e) {
					throw new QTRuntimeException (e);
				}
			}
		});
		SWController ctr = new SWController (gr, true);
		sd.addController (ctr);
	}
}
