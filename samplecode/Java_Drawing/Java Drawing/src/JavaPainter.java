/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.io.*;
import quicktime.app.image.*;

class JavaPainter implements Paintable {
	
	JavaPainter (Frame f, String iName) {
		this.f = f;
		this.iName = iName;
	}
	
	Frame f;
	String iName;
	Image im;
	Font font = new Font ("Helvetica", Font.PLAIN, 18);
	int width, height;
	Rectangle[] r = new Rectangle[1];
	
	void prepareImage () throws Exception {
		MediaTracker tracker = new MediaTracker (f);
		im = Toolkit.getDefaultToolkit().getImage(iName);
		tracker.addImage (im, 0);
		
		// wait for them all to finish importing
		try { tracker.waitForAll(); }
		catch (InterruptedException e) { }
		
		if (tracker.isErrorAny()) throw new Exception ("Error in Media Tracker");

		// make sure images are prepared so they draw properly
		f.prepareImage (im, f);
	}

	public void newSizeNotified (QTImageDrawer drawer, Dimension d) {
		width = d.width;
		height = d.height;
		r[0] = new Rectangle (width, height);
	}
	
	/**
	 * Paint on the graphics. The supplied component is the component from which
	 * the graphics object was derived or related to and is also the component
	 * that is the object that paint was called upon that has called this method.
	 * The Graphics object is what you should paint on.
	 * This maybe an on or off screen graphics.
	 * You should not cache this graphics object as it can be different
	 * between different calls.
	 * @param comp the component from which the Graphics object was derived or 
	 * related too.
	 * @param g the graphics to paint on.
	 */
	public Rectangle[] paint (Graphics g) {		
		g.drawImage (im, 0, 0, null);
		g.setColor (Color.red);
		g.setFont (font);
		g.drawString ("Hello! I'm the Duke.", 2, height - 2);
		return r;
	}
}
