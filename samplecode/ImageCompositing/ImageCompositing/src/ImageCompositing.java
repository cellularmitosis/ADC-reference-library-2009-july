/*
	File:		ImageCompositing.java
	
	Description:	This demo program shows how to composit a presentation out of disparate media sources, 
                        applying compositing effects such as blend and transparency. Recording a movie from 
                        the activities of the Compositor is also shown.

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
import quicktime.app.actions.*;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.event.*;
import quicktime.app.players.*;
import quicktime.std.image.GraphicsMode;
import quicktime.app.anim.*;

public class ImageCompositing extends Frame implements QDConstants, StdQTConstants, Errors, Runnable {		
//	public static ImageCompositing pm;
	
	private static boolean profiling = true, profileStop = false;
	private Thread profileThread;
	
	
	
	private Compositor comp;
	
	public static void main (String args[]) {
		try {
			QTSession.open();
			ImageCompositing compWindow = new ImageCompositing("QT in Java");
			compWindow.pack();
			compWindow.show();
			compWindow.toFront();
			
			if (profiling) {
				compWindow.profileThread = new Thread(compWindow);
				compWindow.profileThread.start();
			}
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	
//only used if profiling
	public void run () {
		comp.resetStatistics();
		for (;;) {
			try {
				Thread.sleep (5000);
			} catch (Exception e) {}
			
			float stat = comp.getStatistics();
			if (stat != 0)
				System.out.println ("Frames a second:" + stat);
			comp.resetStatistics();
            if (profileStop = true)
                break;
		}
	}
			
	ImageCompositing (String title) throws Exception {
		super (title);
		QTCanvas myQTCanvas = new QTCanvas(QTCanvas.kPerformanceResize, 0.5f, 0.5f);
		add("Center", myQTCanvas);
		
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				// cleanup profile thread if profiling
				if (profileThread != null)
					profileStop = true;
					
				QTSession.close();
				dispose();
			}
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});


		QDGraphics gw = new QDGraphics (new QDRect(340, 240));
		if (profiling)
			comp = new ProfileCompositor (gw, QDColor.lightGray, 10, 1);	
		else
			comp = new Compositor (gw, QDColor.lightGray, 10, 1);	
			
		myQTCanvas.setClient (comp, true);
		setupCompositor (comp);
	}
	
	
	private void setupCompositor (Compositor comp) throws Exception {
		JavaTextDrawer jtDrawer = new JavaTextDrawer (new JavaText(), new Dimension (150, 38), true);
			// apply a tinge of pink to the java text that is drawn
		jtDrawer.setGraphicsMode (new GraphicsMode (blend, QDColor.gray));		
		comp.addMember (jtDrawer, 1);
		
		Compositor sh = new Compositor (new QDGraphics (new QDRect(200, 200)), 
													/*new QDColor(0xDFFF, 0xDFFF, 0xFFFF)*/QDColor.white, 20, 1); 
		addSprites (sh);
		sh.setLocation (10, 10);
		sh.getTimer().setRate(1);
		comp.addMember (sh, 3);
		
		Movie m = makeMovie (new QTFile (QTFactory.findAbsolutePath ("jumps.mov")));
		MoviePresenter md = new MoviePresenter (m);
		md.setLocation (120, 120);
		md.setGraphicsMode (new GraphicsMode (blend, QDColor.magenta));
		md.setRate(1);
		comp.addMember (md, 2);
		
		ImagePresenter id = makeImagePresenter (new QTFile (QTFactory.findAbsolutePath ("pics/house.jpg")),
								new QDRect (100, 100));
		id.setLocation (20, 20);
		id.setGraphicsMode (new GraphicsMode (blend, QDColor.yellow));
		comp.addMember (id, 2);

		SimpleActionList al2 = new SimpleActionList ();
		al2.addMember (new BounceAction (2, 1, comp, comp.getTransformable(jtDrawer), 1, 1));
		al2.addMember (new BounceAction (10, 1, comp, comp.getTransformable(id), 1, 1));
		al2.addMember (new BounceAction (10, 1, comp, comp.getTransformable(md), -1, -1));
		comp.addController (al2); 
		
		QTMouseTargetController mouseCtrl = new QTMouseTargetController( true );
		comp.addController(mouseCtrl);
			
			// the drag action activates all the time (regardless of any/no modifiers keys pressed
			// as we don't override the matchModifierFilter call
		if (profiling) {
			mouseCtrl.addQTMouseListener (new DragAction (new TranslateMatrix()) {
					// we use this to profile the dragging around of Sprites

					// this is only really useful if the rate of the compositor is zero
					// then the only rendering work that a compositor does is
					// in response to user interaction - in this case mouseDragging
				int profileCount = 0;
				long startTime, stopTime;
				boolean isProfiling;

				public void mouseDragged (QTMouseEvent e) {
					if (((Compositor)e.getSource()).getTimer().getRate() == 0)
						return;
						
					isProfiling = profileCount++ % 40 == 0;
					if (isProfiling)
						startTime = System.currentTimeMillis();	
					
					super.mouseDragged (e);
					
					if (isProfiling) {
						stopTime = System.currentTimeMillis();
						System.out.println ("mouseDragged:" + (stopTime - startTime) + " msecs");
					}
				}
			});
		} else
			mouseCtrl.addQTMouseListener( new DragAction (new TranslateMatrix()));
		
		mouseCtrl.addQTMouseListener (new MouseButtonAdapter () {
			
			//only activates when kJavaAltMask modifiers are pressed
			public boolean matchModifierFilter (int mods) {
				if ((mods & 0xF) == QTConstants.kJavaAltMask)
					return true;
				return false;
			}

			public void mouseClicked (QTMouseEvent event) {
				super.mouseClicked(event);
				
				Object target = event.getTarget();
				try {
					if (event.getSource() instanceof QTDisplaySpace && target instanceof TwoDSprite ) {
						int layer = ((QTDisplaySpace)event.getSource()).getBackLayer();
						((TwoDSprite)target).setLayer (layer+1);
					}
				}
				catch (QTException e)
				{
					QTRuntimeException.handleOrThrow (new QTRuntimeException(e), this, "mousePressed");
				}
			}
		});
			
		comp.getTimer().setRate(1);
		
		ControlPanel cp = new ControlPanel (this, comp, jtDrawer);
		add("North", cp);		
		cp.setDisplay();
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

	void addSprites (Compositor sd) throws IOException, QTException {
		File matchFile = QTFactory.findAbsolutePath ("images/Ship01.pct");	//this file must exist in the directory!!!	
		ImageDataSequence isp = ImageUtil.createSequence (matchFile);
		ImageDataSequence seq = ImageUtil.makeTransparent (isp, QDColor.blue);
		
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

		SimpleActionList al = new SimpleActionList();
		ImageSequencer is = new ImageSequencer (seq);
		is.setLooping (ImageSequencer.kLoopForwards);
		al.addMember (new NextImageAction (20, 1, is, s1));
		al.addMember (new BounceAction (20, 1, sd, s1, 3, 2));
		ImageSequencer is2 = new ImageSequencer (seq);
		is2.setLooping (ImageSequencer.kLoopForwards);
		al.addMember (new NextImageAction (15, 1, is2, s2));
		al.addMember (new BounceAction (40, 1, sd, s2, 4, 3));
		sd.addController (al);
	}
}
