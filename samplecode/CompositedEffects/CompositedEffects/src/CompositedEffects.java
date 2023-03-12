/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;

import quicktime.qd.*;
import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.*;
import quicktime.std.movies.*;
import quicktime.io.*;
import quicktime.util.*;

import quicktime.app.QTFactory;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.players.*;
import quicktime.std.image.GraphicsMode;
import quicktime.app.time.*;
import quicktime.app.anim.*;
import quicktime.app.actions.*;

public class CompositedEffects extends Frame implements QDConstants, StdQTConstants {		
	private static boolean isWin = QTSession.isCurrentOS (QTSession.kWin32);

	public static void main (String args[]) {
		try { 
			QTSession.open();
			CompositedEffects pm = new CompositedEffects("QT in Java");
			pm.pack();
			pm.show();
			pm.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	int kWidth = 350;
	int kHeight = 250;
		
	CompositedEffects (String title) throws Exception {
		super (title);
		
		setBackground (Color.lightGray);
		
		QTCanvas myQTCanvas = new QTCanvas(QTCanvas.kInitialSize, 0.5f, 0.5f);
		add("Center", myQTCanvas);

// set the Background Color to white so that the Java text will appear transparent
// white is a color that provides a reliable transparent background for different pixel depths.
		myQTCanvas.setBackground (Color.white);
		
		Dimension d = new Dimension (kWidth, kHeight);
		QDRect r = new QDRect(d);
		QDGraphics gw = new QDGraphics (r);

// this is the compositor which will contain the bgPict, effect, text and a member compositor
		Compositor comp = new Compositor (gw, QDColor.green, new QDGraphics (r), 10, 1);
		
// add the background picture to the Comp -> load it into memory so it draws quicker
		QTFile backgroundFile = new QTFile (QTFactory.findAbsolutePath("pics/water.pct"));
		ImagePresenter background = makeImagePresenter (backgroundFile, r);
		comp.addMember (background, Layerable.kBackMostLayer);

// add the effect in front of the background pict
		CompositableEffect e = new CompositableEffect ();
		AtomContainer effectSample = new AtomContainer();
		effectSample.insertChild (new Atom(kParentAtomIsContainer), kEffectWhatAtom, 1, 0, EndianOrder.flipNativeToBigEndian32(kWaterRippleCodecType));
		e.setEffect (effectSample);
		e.setDisplayBounds (new QDRect (0, kHeight - 100, kWidth, 100));
		comp.addMember (e, 2);

// add the Text in front of the pict and ripples
// set its transparency (to the bgColor of the QTCanvas) so that only the text is seen
		Paintable jt = new JavaText ();
		QTImageDrawer qid = new QTImageDrawer (jt, new Dimension (110, 22), Redrawable.kSingleFrame);
		qid.setGraphicsMode (new GraphicsMode (transparent, QDColor.white));
		qid.setLocation (200, 20);
		comp.addMember (qid, 1);

// add the contained Compositor - yellow is bgColor which is then NOT drawn
// add a Dragger so that member of this compositor can be dragged around
// when any modifier key is pressed when the mousePressed event is generated
		Compositor sh = new Compositor (new QDGraphics (new QDRect(160, 160)), QDColor.yellow, 8, 1); 
		addSprites (sh);
		sh.setLocation (190, 90);
		sh.setGraphicsMode (new GraphicsMode (transparent, QDColor.yellow));
		sh.getTimer().setRate(1);
		comp.addMember (sh, 1);
		sh.addController(new SWController (new Dragger (MouseResponder.kAnyModifiersMask, MouseResponder.kAnyModifiers), true));
	
// add a Dragger to the main Compositor to enable dragging of all its members around
		comp.addController(new SWController (new Dragger (MouseResponder.kNoModifiersMask), true));
		
// make a DirectGroup as the top level container space 
		DirectGroup dg = new DirectGroup (d, QDColor.white);

// add	the Compositor to the DirectGroup parent
		dg.addMember (comp, 2);

// make a movie and add it in front of the Composited image
// resizing the movie to make it a little smaller
		QTDrawable mov = QTFactory.makeDrawable (new QTFile (QTFactory.findAbsolutePath ("jumps.mov")));
		mov.setDisplayBounds (new QDRect(20, 20, 120, 106));
		dg.addMember (mov, 1);

// Set the DirectGroup as the client of the QTCanas				
		myQTCanvas.setClient (dg, true);
		
// set the rates of the compositor and parent DirectGroup so you see
// it "playing" when it is first shown
		comp.getTimer().setRate(1);
		dg.getTimer().setRate(1);

// add the control panel to control the rates of the 
//	DirectGroup
//		-> Its Compositor member
//			-> The Compositor's Compositor 
// the movie can be controlled directly by the user
		ControlPanel cp = new ControlPanel(comp.getTimer(), (QTPlayer)mov, dg, sh);
		add (cp, "North");
		cp.setDisplay();

// add a WindowListener to close the program down
		addWindowListener (new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}

	private Movie makeMovie (QTFile f) throws IOException, QTException {
		OpenMovieFile movieFile = OpenMovieFile.asRead(f);
		Movie m = Movie.fromFile (movieFile);
		m.getTimeBase().setFlags (loopTimeBase);	
		return m;
	}
	
	private ImagePresenter makeImagePresenter (QTFile file, QDRect size) throws Exception {
		GraphicsImporterDrawer if1 = new GraphicsImporterDrawer (file);
		if1.setDisplayBounds (size);
		return ImagePresenter.fromGraphicsImporterDrawer (if1);
	}	

// makes the Sprites for the child Compositor 	
	void addSprites (Compositor sd) throws IOException, QTException {
		File matchFile = QTFactory.findAbsolutePath ("images/Ship01.pct");	//this file must exist in the directory!!!	
		ImageDataSequence isp = ImageUtil.createSequence (matchFile);
		ImageDataSequence seq = ImageUtil.makeTransparent (isp, QDColor.blue);

// Build Sprites		
		Matrix matrix1 = new Matrix();
		matrix1.setTx(20);
		matrix1.setTy(20);
		matrix1.setSx(0.8F);
		matrix1.setSy(0.8F);
		TwoDSprite s1 = new TwoDSprite(seq, 4, matrix1, true, 1);
		sd.addMember (s1);
		
		Matrix matrix2 = new Matrix();	
		matrix2.setTx(4);
		matrix2.setTy(4);
		TwoDSprite s2 = new TwoDSprite(seq, 1, matrix2, true, 10);
		sd.addMember (s2);

// Build ActionList
		SimpleActionList al = new SimpleActionList();
		ImageSequencer is = new ImageSequencer (seq);
		is.setLooping (ImageSequencer.kLoopForwards);
		ImageSequencer is2 = new ImageSequencer (seq);
		is2.setLooping (ImageSequencer.kLoopForwards);
		al.addMember (new NextImageAction (20, 1, is2, s2));
		al.addMember (new BounceAction (20, 1, sd, s2, 4, 3));
		al.addMember (new NextImageAction (7, 1, is, s1));
		al.addMember (new BounceAction (5, 1, sd, s1, 3, 2));
		sd.addController(al);
	}
}
