/*

File: TumbleItem.java

Abstract: Panel displaying an animation of Duke

Version: 1.2

© Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

package duke;

import java.awt.*;
import java.io.*;
/**
 * Based on the TumbleItem Applet by James Gosling.
 */
public class TumbleItem extends Panel implements Runnable {
//_______________________ CLASS METHODS
    public TumbleItem(String dir) throws Exception {
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
	private int      maxWidth   = 120;
	private Image    images[] = new Image[nimgs];
	private Image    offscreenImage;
	private Graphics offscreenGraphics;
    private boolean kickerStop = false;

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
                if (kickerStop)
                    break;

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
		    kickerStop = true;
		    kicker = null;
		}
    }

	/** So the layout manager respects our preferred size. */
	public Dimension getPreferredSize() { return new Dimension(5, 80); }
}
