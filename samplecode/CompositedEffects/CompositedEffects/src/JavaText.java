/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import quicktime.app.image.*;

class JavaText implements Paintable {
	Font font = new Font ("Helvetica", Font.PLAIN, 24);
	int width, height;
	Rectangle[] r = new Rectangle[1];
	
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
		g.setColor (Color.magenta);
		g.setFont (font);
		g.drawString ("Java Text", 2, 20);
		return r;
	}
}
