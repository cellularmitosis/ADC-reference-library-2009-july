/*
	File:		TransitionEffect.java
	
	Description:	This demo shows how to use the QuickTime effects architecture 
                        applied to a character in an animation scene.

	Author:		Apple Computer, Inc.

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
 
import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.std.StdQTConstants;
import quicktime.*;
import quicktime.qd.*;
import quicktime.io.*;
import quicktime.std.image.*;
import quicktime.std.movies.*;
import quicktime.util.*;

import quicktime.app.QTFactory;
import quicktime.app.time.*;
import quicktime.app.image.*;
import quicktime.app.display.*;
import quicktime.app.anim.*;
import quicktime.app.players.*;
import quicktime.app.spaces.*;
import quicktime.app.actions.*;


public class TransitionEffect extends Frame implements StdQTConstants, QDConstants {
	
	public static void main(String args[]) {
		try { 
			QTSession.open();
			
			TransitionEffect te = new TransitionEffect("Transition Effect");
			te.pack();
			te.show();
			te.toFront();	
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	
	}
	
	

	TransitionEffect(String title) throws Exception {
		super (title);

		QTCanvas myQTCanvas = new QTCanvas(QTCanvas.kInitialSize, 0.5f, 0.5f);
		add("Center", myQTCanvas);
		
		Dimension d = new Dimension (300, 300);
		addWindowListener(new WindowAdapter() {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}
			
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});

		QDGraphics gw = new QDGraphics (new QDRect(d.width, d.height));
		Compositor comp = new Compositor (gw, QDColor.black, 20, 1);

		ImagePresenter idb = makeImagePresenter (new QTFile (QTFactory.findAbsolutePath ("pics/stars.jpg")),
									new QDRect(300, 220));
		idb.setLocation (0, 0);
		comp.addMember (idb, 2);

		ImagePresenter id = makeImagePresenter (new QTFile (QTFactory.findAbsolutePath ("pics/water.pct")),
									new QDRect(300, 80));
		id.setLocation (0, 220);
		comp.addMember (id, 4);
		
		CompositableEffect ce = new CompositableEffect ();
		AtomContainer effectSample = new AtomContainer(); 
		effectSample.insertChild (new Atom(kParentAtomIsContainer), 
						kEffectWhatAtom, 
						1, 
						0,
						EndianOrder.flipNativeToBigEndian32(kWaterRippleCodecType));
		ce.setEffect (effectSample);
		ce.setDisplayBounds (new QDRect(0, 220, 300, 80));
		comp.addMember (ce, 3);
		
		Fader fader = new Fader();		
		QTEffectPresenter efp = fader.makePresenter();
		efp.setGraphicsMode (new GraphicsMode (blend, QDColor.gray));
		efp.setLocation(80, 80);
		comp.addMember (efp, 1);

		comp.addController(new TransitionControl (20, 1, fader.getTransition()));

		myQTCanvas.setClient (comp, true);
		comp.getTimer().setRate(1);
	}
	
	private ImagePresenter makeImagePresenter (QTFile file, QDRect size) throws Exception {
		GraphicsImporterDrawer if1 = new GraphicsImporterDrawer (file);
		if1.setDisplayBounds (size);
		return ImagePresenter.fromGraphicsImporterDrawer (if1);
	}
}

class Fader implements StdQTConstants {
	Fader() throws Exception {
		File file = QTFactory.findAbsolutePath ("pics/Ship.pct");
		QTFile f = new QTFile (file.getAbsolutePath());

		QDGraphics g = new QDGraphics (new QDRect (78, 29));
		g.setBackColor (QDColor.black);
		g.eraseRect(null);
		ImagePresenter srcImage = ImagePresenter.fromGWorld(g);
		Compositable destImage = new GraphicsImporterDrawer (f);
		
		ef = new QTTransition ();
		ef.setRedrawing(true);
		ef.setSourceImage (srcImage);
		ef.setDestinationImage (destImage);
		ef.setDisplayBounds (new QDRect(78, 29));
		ef.setEffect (createFadeEffect (kEffectBlendMode, kCrossFadeTransitionType));
		ef.setFrames(60);
		ef.setCurrentFrame(0);
	}
	
	private QTTransition ef;
	
	public QTEffectPresenter makePresenter() throws QTException {
		QTEffectPresenter efPresenter = new QTEffectPresenter (ef);
		return efPresenter;
	}	
	
	public QTTransition getTransition () {
		return ef;
	}
	
	AtomContainer createFadeEffect (int effectType, int effectNumber) throws QTException {
		AtomContainer effectSample = new AtomContainer();
		effectSample.insertChild (new Atom(kParentAtomIsContainer), 
								kEffectWhatAtom, 
								1,
								0, 
								EndianOrder.flipNativeToBigEndian32(kCrossFadeTransitionType));
		effectSample.insertChild (new Atom(kParentAtomIsContainer), 
								effectType, 
								1, 
								0, 
								EndianOrder.flipNativeToBigEndian32(effectNumber));
		return effectSample;
	}
}	

class TransitionControl extends PeriodicAction implements TicklishController {

	TransitionControl (int scale, int period, QTTransition t) {
		super (scale , period);
		this.t = t;			
	}
	
	
	QTTransition t;
	boolean waiting = false;
	int startWaitTime = 0;
	int waitForMsecs = 4000;
	
	public void addedToSpace (Space s) {}

	public void removedFromSpace () {}
	
	protected boolean constraintReached () {
		return false;
	}
	
	public void timeChanged (int tm) throws QTException {
		if (waiting)
			startWaitTime = tm;
		super.timeChanged(tm);
	}
	
	protected void doAction (float er, int tm) throws QTException {
		if (waiting) {
			if ((er > 0 && ((startWaitTime + waitForMsecs) <= tm))
				|| (er < 0 && ((startWaitTime - waitForMsecs) >= tm))) {
				
				waiting = false;
				t.setRedrawing(true);
			} else
				return;
		}
		int curr_frm = t.getCurrentFrame();
		curr_frm++;
		t.setCurrentFrame(curr_frm);
		if (curr_frm > t.getFrames()) {
			curr_frm = 0;
			t.setRedrawing(false);
			t.setCurrentFrame(curr_frm);	
			t.swapImages();
			waiting = true;
			startWaitTime = tm;
		}
	}	
}	
