/*
	File:		GroupDrawing.java
	
	Description:	This demo program shows how to select group QuickTime drawing capable objects into the same 
                        display space of the QTCanvas.

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
import quicktime.app.event.*;
import quicktime.app.anim.*;
import quicktime.app.spaces.*;

public class GroupDrawing extends Frame implements StdQTConstants {	

	public static void main (String args[]) {
		try {
			QTSession.open();
			GroupDrawing frame = new GroupDrawing("QT in Java");
			
			frame.pack();
			frame.show();
			frame.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	
	private QTMouseTargetController mouseTargetCntrl;
	
	GroupDrawing (String title) throws Exception {
		super (title);

		QTCanvas myQTCanvas = new QTCanvas(QTCanvas.kAspectResize, 0, 0);
		add("Center", myQTCanvas);
		
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});

			// setupGroup
		int kWidth = 300, kHeight = 300;
		DirectGroup drawer = new DirectGroup(new QDDimension (kWidth, kHeight), QDColor.gray);
	
		QTPlayer qtPlayer = makePlayer (new QTFile (QTFactory.findAbsolutePath ("jumps.mov")));
		drawer.addMember (qtPlayer, 1, 0.1F, 0.2F);
		
		Compositor shipAnimation = new Compositor (new QDGraphics (new QDRect(200, 200)), 
													QDColor.lightGray, 20, 1); 
		addSprites (shipAnimation);
		shipAnimation.setLocation (kWidth - 160, kHeight - 160);
		drawer.addMember (shipAnimation, 2);
		
		QTFile imageFile = new QTFile (QTFactory.findAbsolutePath ("pics/house.jpg"));
		GraphicsImporterDrawer gid1 = new GraphicsImporterDrawer (imageFile);
		gid1.setDisplayBounds (new QDRect (kWidth, kHeight));
		// we use an ImagePresenter for this member as the dragging is much quicker
		ImagePresenter ip = ImagePresenter.fromGraphicsImporterDrawer (gid1);
		drawer.addMember (ip, 3);
		
		/*Dragger dragger = new Dragger (MouseResponder.kNoModifiersMask);
		dragger.setConstrained(Dragger.kConstrainNone);
		GroupController controller1 = new GroupController (dragger, false);
		controller1.addMember (qtPlayer);
		controller1.addMember (shipAnimation);
		//drawer.addController (controller1);
		*/
	
	 	QTMouseTargetController qtmc = new QTMouseTargetController(false);
	 	qtmc.addMember(qtPlayer);
	 	qtmc.addMember(shipAnimation);
		drawer.addController (qtmc);
		TransformMatrix transMatrix = new TranslateMatrix();
		transMatrix.setConstraint(TransformMatrix.kConstraintSpecifiedAll);
		qtmc.addQTMouseListener(new DragAction (transMatrix) {

			//only activates when NO modifiers are pressed
			public boolean matchModifierFilter (int mods) {
				if ((mods & 0xF) == 0)
					return true;
				return false;
			}
		    public void mouseDragged(QTMouseEvent event) {	
		    	super.mouseDragged (event);
		    }
		});	

		/*Dragger dragger2 = new Dragger (MouseResponder.kNoModifiersMask);
		GroupController controller2 = new GroupController (dragger2, false);
		controller2.addMember (ip);		
		drawer.addController (controller2);*/
		
	 	QTMouseTargetController qtmc2 = new QTMouseTargetController(false);
	 	qtmc2.addMember(ip);
		drawer.addController (qtmc2);
		qtmc2.addQTMouseListener(new DragAction (new TranslateMatrix()) {
			//only activates when NO modifiers are pressed
			public boolean matchModifierFilter (int mods) {
				if ((mods & 0xF) == 0)
					return true;
				return false;
			}

			public void mouseDragged (QTMouseEvent event) {
		    	super.mouseDragged (event);
			}
		
		});

		
		
	/*	LayerChanger mouseTargetCntrl = new LayerChanger (InputEvent.ALT_MASK);
		GroupController controller3 = new GroupController (mouseTargetCntrl, true);
		drawer.addController (controller3);*/

		mouseTargetCntrl = new QTMouseTargetController(true);
		drawer.addController (mouseTargetCntrl);
		mouseTargetCntrl.addQTMouseListener(new DragAction (new TranslateMatrix()) {
			
			//only activates when kJavaAltMask modifiers are pressed
			public boolean matchModifierFilter (int mods) {
				if ((mods & 0xF) == QTConstants.kJavaAltMask)
					return true;
				return false;
			}

			public void mouseClicked (QTMouseEvent event) {
				super.mouseClicked(event);
				
				Object target = event.getTarget();
				Object source = event.getSource();
				GroupDrawable groupdrawer = (GroupDrawable)mouseTargetCntrl.getSpace();
					int layer = groupdrawer.getBackLayer();	
				
				if ( source instanceof DirectGroup && target instanceof QTDrawable && // make sure source and target are correct types
					 ((DirectGroup)source).isAppropriate( target ))
				{
					try
					{
						((DirectGroup)source).setMemberLayer ((QTDrawable)target, layer+1 );
					}
					catch (QTException e)
					{
						QTRuntimeException.handleOrThrow (new QTRuntimeException(e), this, "mousePressed");
					}
				}
			}

		});


		myQTCanvas.setClient (drawer, true);
		
		shipAnimation.getTimer().setRate(1);
		drawer.getTimer().setRate (1);
	}
	
		// use a file input stream to make dragging faster as the movie is comletetly loaded into memory
		// this is not required for any other reason
	private static QTPlayer makePlayer (QTFile f) throws IOException, QTException {
		QTDrawable d = QTFactory.makeDrawable (new FileInputStream(f), kDataRefFileExtensionTag, "mov");
		Movie m = ((QTPlayer)d).getMovieController().getMovie();
		m.getTimeBase().setFlags (loopTimeBase);	
		return (QTPlayer)d;
	}
	
	private static ImagePresenter makeImagePresenter (QTFile file, QDRect size) throws Exception {
		GraphicsImporterDrawer gid = new GraphicsImporterDrawer (file);
		gid.setDisplayBounds (size);
		return ImagePresenter.fromGraphicsImporterDrawer (gid);
	}	

	private static void addSprites (Compositor sh) throws IOException, QTException {
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
		sh.addMember (s1);
		
		Matrix matrix2 = new Matrix();	
		matrix2.setTx(4);
		matrix2.setTy(4);
		TwoDSprite s2 = new TwoDSprite(seq, 1, matrix2, true, 10);
		sh.addMember (s2);
		
		// This needs to be a 32bit QDGraphics so the blend mode will 
		// be applied correctly to this sprite
		File shipFile = QTFactory.findAbsolutePath ("images/Ship10.pct");
		GraphicsImporterDrawer ip = new GraphicsImporterDrawer (new QTFile(shipFile));
		QDRect r = new QDRect (ip.getDescription().getWidth(), ip.getDescription().getHeight());
		ImageSpec si = ImageUtil.makeTransparent (ip, QDColor.blue, new QDGraphics (QDGraphics.kDefaultPixelFormat, r));
		Matrix matrix3 = new Matrix();	
		matrix3.setTx(50);
		matrix3.setTy(50);
		TwoDSprite s3 = new TwoDSprite(si, matrix3, true, 8, new GraphicsMode (QDConstants.blend, QDColor.green));
		sh.addMember(s3);
		
// Add Dragger (DEPRECATED CODE)
/*		Dragger dragger = new Dragger (InputEvent.SHIFT_MASK);
		SWController controller = new SWController (dragger, true);
		sh.addController (controller);*/
		
		QTMouseTargetController shipController = new QTMouseTargetController( true );
		shipController.addQTMouseListener( new DragAction(new TranslateMatrix()) {
		//only activates when the shift modifier is pressed
			public boolean matchModifierFilter (int mods) {
				if ((mods & 0xF) == QTConstants.kJavaShiftMask)
					return true;
				return false;
			}
		});
		sh.addController( shipController ); 

// Add Controllers
// Build ActionList
		SimpleActionList al = new SimpleActionList();
		ImageSequencer is = new ImageSequencer (seq);
		is.setLooping (ImageSequencer.kLoopForwards);
		al.addMember (new NextImageAction (10, 1, is, s1));
		al.addMember (new BounceAction (5, 1, sh, s1, 3, 2));
		ImageSequencer is2 = new ImageSequencer (seq);
		is2.setLooping (ImageSequencer.kLoopForwards);
		al.addMember (new NextImageAction (20, 1, is2, s2));
		al.addMember (new BounceAction (20, 1, sh, s2, 4, 3));
		sh.addController(al);

	}
}
