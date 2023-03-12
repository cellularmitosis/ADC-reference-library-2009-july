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
import quicktime.io.*;
import quicktime.app.image.*;
import quicktime.app.display.*;
import quicktime.app.QTFactory;
import quicktime.util.*;

import quicktime.app.anim.*;
import quicktime.app.actions.*;
import quicktime.std.image.*;

public final class SpriteDemoApp extends Frame {
	public static void main (String[] args) {
		try {
			QTSession.open();
			SpriteDemoApp win = new SpriteDemoApp ("QT in Java");
			win.show();
			win.toFront();
		} catch (Exception e) {
			QTSession.close();
			e.printStackTrace();
		}
	}
	
	MovingQTCanvas c1;
	MovingQTCanvas c2;
	boolean c1Active;
	
	SpriteDemoApp (String s) throws Exception {
		super(s);
		setResizable (false);
		setLayout(new BorderLayout());
		setBounds (0, 0, 400, 220);
		addWindowListener (new WindowAdapter() {
			public void windowClosing (WindowEvent we) {
				c1.cleanup();
				c2.cleanup();
				QTSession.close();
				dispose();
			}
			public void windowClosed (WindowEvent we) {
				System.exit(0);
			}
		});
					
		Compositor shipAnimation = new Compositor (new QDGraphics (new QDRect(200, 200)), 
													QDColor.yellow, 20, 1); 
		addSprites (shipAnimation);
		shipAnimation.getTimer().setRate(1);
		
		c1 = new MovingQTCanvas (shipAnimation);
		add("West", c1);
		c1.setClient (shipAnimation, true);
		
		c2 = new MovingQTCanvas (shipAnimation);
		add("East", c2);

		Button b = new Button ("Click Here To Make Sprites Jump");
		b.addActionListener (new ActionListener () {
			boolean c1Active = true;
			
			public void actionPerformed (ActionEvent event) {
				if (c1Active) {
					c1.removeClient();
					c2.doSetClient();
				} else {
					c2.removeClient();
					c1.doSetClient();
				}
				c1Active = ! c1Active;
			}
		});
		add("North", b);
	}

	// this is used to change the rates when the clients are changed
	static int r = 2;
	
	class MovingQTCanvas extends QTCanvas {
		
		MovingQTCanvas (Compositor anim) {
			client = anim;
		}
		
		Compositor client;
		
		void doSetClient() {
			try {
				setClient (client, true);
				if (r++ % 3 == 0)
					client.getTimer().setRate(0);
				else {
					float rt = client.getTimer().getRate();
					if (rt == 0)
						client.getTimer().setRate (1);
					else
						client.getTimer().setRate (-client.getTimer().getRate());
				}
			} catch (QTException e) {
				e.printStackTrace();
			}
		}
		
		public void cleanup () {
			client = null;
			removeClient();
		}
	}

	static void addSprites (Compositor sh) throws IOException, QTException {
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
