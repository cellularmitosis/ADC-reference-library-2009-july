/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.io.*;
/**
 * Based on the JavaDuke Applet by James Gosling.
 */
public class JavaDuke extends Panel implements Runnable {
//_______________________ CLASS METHODS
    public JavaDuke(String dir) throws Exception {
    	super();

		// used to monitor the image importing process
		MediaTracker tracker = new MediaTracker(this);
		
		// import gif images
		File fDir = new File(dir);
		if (fDir.isDirectory() == false)
			throw new FileNotFoundException (dir);
		if (dir.charAt (dir.length() - 1) != File.separatorChar)
			dir += File.separatorChar;
		for (int i=0; i < nimgs; i++) {		
			images[i] = Toolkit.getDefaultToolkit().getImage(dir + "T" + i + ".gif");
			tracker.addImage(images[i], i);
		}
		
		// wait for them all to finish importing
		try { tracker.waitForAll(); }
		catch (InterruptedException e) { }
		
		// we do nothing about no duke images
		if (tracker.isErrorAny()) throw new Exception ("Duke:Error in Media Tracker");
   }

//_______________________ INSTANCE VARIABLES
	private int      loopslot   = -1;   // The current loop slot.
	private Thread   kicker     = null; // The thread animating the images.
	private int      pause      = 100;  // The length of the pause between revs.
	private int      offset     = -57;
	private int      xOffset;
	private int      speed      = 100;
	private int      nimgs      = 17;
	private int      maxWidth   = 130;
	private Image    images[] = new Image[nimgs];
	private Image    offscreenImage;
	private Graphics offscreenGraphics;

//_______________________ INSTANCE METHODS
    /** Run the image loop. This method is called by class Thread. */
    public void run() {
		Thread.currentThread().setPriority(Thread.NORM_PRIORITY-1);

		// create offscreen image and graphics objects
		offscreenImage    = createImage(images[0].getWidth(this), images[0].getHeight(this));
		offscreenGraphics = offscreenImage.getGraphics();

		// take care of positioning the images across the screen
		Dimension d = getSize();
		if (nimgs > 1) {
		    if (offset < 0) xOffset = d.width - maxWidth;
		    while (kicker != null) {
				d = getSize();
				if (++loopslot >= nimgs) {
				    loopslot = 0;
				    xOffset += offset;
				    if (xOffset < 0) xOffset = d.width - maxWidth;
				    else if (xOffset + maxWidth > d.width) xOffset = 0;
				}
				repaint();
				try { Thread.sleep(speed + ((loopslot == nimgs - 1) ? pause : 0)); }
				catch (InterruptedException e) { break;	}
		    }
		}
    }

	/** Should not get called because we are using MediaTracker.waitForAll(). */
    public boolean imageUpdate(Image img, int flags, int x, int y, int w, int h) {
		if ((flags & (SOMEBITS|FRAMEBITS|ALLBITS)) != 0) {
		    if ((images != null) && (loopslot < nimgs) && (images[loopslot] == img)) {
				repaint();
		    }
		}
		return (flags & (ALLBITS|ERROR)) == 0;
    }

    /** Paint the current frame using our offscreen graphics to reduce flicker. */
    public void paint(Graphics g) {
		if (loopslot < 0) return;
		if ((images != null) && (loopslot < nimgs) && (images[loopslot] != null)) {
			offscreenGraphics.drawImage(images[loopslot], 0, 0, this);
		    g.drawImage(offscreenImage, xOffset, 0, this);
		}
    }

    /** Start the animation by forking an animation thread. */
    public void start() {
		if (kicker == null) {
		    kicker = new Thread(this);
			kicker.start();
		}
    }

    /** Stop the animation. The thread will exit because kicker is set to null. */
    public void stop() {
		if (kicker != null) {
		    kicker.stop();
		    kicker = null;
		}
    }

	/** So the layout manager respects our preferred size. */
	public Dimension getPreferredSize() { return new Dimension(5, 80); }
}
