/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
package cdf;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import quicktime.app.*;
import quicktime.app.anim.*;
import quicktime.app.actions.BounceAction;
import quicktime.app.actions.NextImageAction;
import quicktime.app.actions.SimpleActionList;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.std.image.*;
import quicktime.*;
import quicktime.qd.*;



public class CaptureDukeFrame {
	public QTCanvas canv;
	ImageDataSequence ids;
	QTImageDrawer qid;
	MultipleImagePainter dd;
	//public DukeAnim da;
	int width;
	private Frame showAfter;
	private Frame addTo;
	private Button b;
	
	//from duke anim
	private TwoDSprite sprite;
//	private ImageDataSequence images;
	
	private NextImageAction ni;
	private BounceAction ba;
	private SimpleActionList al;
	private Compositor comp;
	
	public CaptureDukeFrame (Frame f, String title, int w) {

		addTo = f;
		width = w;
		canv = new QTCanvas(QTCanvas.kInitialSize, 0.5F, 0.5F);
		addTo.add("Center", canv);
					
		
		addTo.addNotify();
		b = new Button ("QT Capture");
		b.addActionListener (new ActionListener() {
			public void actionPerformed (ActionEvent ae) {
				try {
					dd.setCurrentFrame(0);
					qid.redraw(null);
					ImagePresenter id = qid.toImagePresenter();
					ImageDescription desc = id.getDescription();
					ids = new ImageDataSequence (desc);
					ids.append (id.getImage());
					
					for (int i = 1; i < dd.getNumberOfFrames(); i++) {
						dd.setCurrentFrame(i);
						qid.redraw(null);
						id = qid.toImagePresenter();
						ids.append (id.getImage());
					}
					dd.setCurrentFrame(0);
					//da = new DukeAnim (ids, new Dimension (width, 80), QDColor.white);
					if (ids == null) {
						QTSession.close();
						System.exit(0);
					}
					canv.removeClient();
					disableButton();
					
					//create compositor
					comp = new Compositor (new QDGraphics (new QDRect(new Dimension (width, 80))), QDColor.white, 35, 1);
					al = new SimpleActionList();
					Matrix matrix1 = new Matrix();
					matrix1.setTx(20);
					matrix1.setTy(0);
					//sprite.setMatrix (matrix1);
					sprite = new TwoDSprite(ids, 1, matrix1, true, 1);
					
					ImageSequencer seq = new ImageSequencer (ids);
					seq.setLooping (ImageSequencer.kLoopPalindrome);		
					ni = new NextImageAction (10, 1, seq, sprite);
					al.addMember(ni);
					//ba = new BounceAction (1, 0, comp, 35, 1, sprite); 
					ba = new BounceAction (10, 0, comp, sprite, 1.0F, 0); 
					al.addMember(ba);
					comp.addController(al);
					comp.addMember(sprite);
			//			getTimer().setTicklish(al);
					comp.getTimer().setRate(1);
					canv.setClient (comp, true);
					
					//canv.setClient (da, true);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
		});
		
		addTo.add ("South", b);
	}
	
	private void disableButton() {
		b.setEnabled(false);
	}
	
	
	public void showDuke() {
		try {			
			File file = QTFactory.findAbsolutePath ("duke");
			Image[] images = getImages (file.getAbsolutePath(), 17);
			int width = images[0].getWidth(addTo);
			int height = images[0].getHeight(addTo);
			
			dd = new MultipleImagePainter(images);
			qid = new QTImageDrawer (dd, new Dimension (width, height), Redrawable.kMultiFrame);
			qid.setRedrawing(true);
			
			canv.setClient (qid, true);
		} catch (Exception ex) {
			ex.printStackTrace();
			QTSession.close();
		}
	}

			
	Image[] getImages (String dir, int nimgs) throws Exception {
		MediaTracker tracker = new MediaTracker (addTo);
		
		File fDir = new File(dir);
		if (fDir.isDirectory() == false)
			throw new FileNotFoundException (dir);
		if (dir.charAt (dir.length() - 1) != File.separatorChar)
			dir += File.separatorChar;

		// import gif images
		Image[] images = new Image[nimgs];
		for (int i=0; i < nimgs; i++) {		
			images[i] = Toolkit.getDefaultToolkit().getImage(dir + "T" + i + ".gif");
			tracker.addImage(images[i], i);
		}
		
		// wait for them all to finish importing
		try { tracker.waitForAll(); }
		catch (InterruptedException e) { }
		
		if (tracker.isErrorAny()) throw new Exception ("Error in Media Tracker");

		// make sure images are prepared so the draw properly
		for (int i = 0; i < nimgs; i++)
			addTo.prepareImage (images[i], addTo);

		return images;
	}
}
