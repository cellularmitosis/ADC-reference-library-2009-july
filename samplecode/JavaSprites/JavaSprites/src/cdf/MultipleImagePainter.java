/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
package cdf;

import java.awt.*;
import quicktime.app.image.*;

class MultipleImagePainter implements Paintable {
	MultipleImagePainter (Image[] ims) {
		images = ims;
	}
	
	int width, height;
	Image[] images;
	int loopslot;
	Rectangle[] r = new Rectangle[1];
	/**
	 * Returns the number of images
	 */
	public int getNumberOfFrames () { return images.length; }
	
	public void setCurrentFrame (int frame) {
		loopslot = frame;
		if (loopslot < 0) loopslot = 0;
		if (loopslot >= getNumberOfFrames()) loopslot = getNumberOfFrames() - 1;
	}
	
	/** 
	 * Sets the current frame - zero based
	 */
	public Image getCurrentFrame () {
		return images[loopslot];
    }

	/**
	 * The Parent object of the Paintable tell the paintable object the size of its available
	 * drawing surface. Any drawing done outside of these bounds (originating at 0,0) will
	 * be clipped.
	 */
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
		g.drawImage (getCurrentFrame(), 0, 0, null);
		return r;
	}
}
