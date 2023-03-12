/*

File: SkyCreator.java

Abstract: This is a very simple demo that creates a random image
of a starry sky. It has several star images of different sizes that are 
placed randomly on the screen. The user can drag the stars to re-arrange 
them. This code was written to illustrate the functionallity of QuartzDebug.
There is a slow and a fast version. The slow version is commented out by 
default. The slow version repaints the whole window every time a star is 
moved. And the fast version only re-paints the  dirty region. You can use 
QuartzDebug to see the different beahvior between the two versions.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright ¬© 2001-2005 Apple Computer, Inc., All Rights Reserved

*/

import javax.swing.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.swing.*;
import java.util.*;


public class SkyCreator
{
    private final static int WIDTH = 1000;
    private final static int HEIGHT = 800;

    Random randomGenerator = new Random();

    public SkyCreator()
    {
	// create the frame and make it visible
	JFrame frame = new JFrame("SkyCreator");
	frame.getContentPane().add(new Sky());
	frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	frame.setBounds(50, 50, WIDTH, HEIGHT);
	frame.setVisible(true);
    }

    public static void main(String[] args)
    {
	new SkyCreator();
    }

    // The panel represent the sky. Its job is to paint a black background and draw the individual stars. 
    // It's also responsible for handling mouse events
    private class Sky extends Panel implements MouseMotionListener, MouseListener
    {
	// the heuristic for creating the sky is there are stars of for levels. Level_1 is the brightest and closes, 
	// while Level_4 is the most far away. There also more stars that are darker than brigher.
	private final static int NUM_STARS_LEVEL_1 = 1;
	private final static int NUM_STARS_LEVEL_2 = 2;
	private final static int NUM_STARS_LEVEL_3 = 20;
	private final static int NUM_STARS_LEVEL_4 = 1500;
	private final static int NUM_STARS_LEVEL_5 = 4500;
	private final static int NUM_STARS = NUM_STARS_LEVEL_1 + NUM_STARS_LEVEL_2 + NUM_STARS_LEVEL_3 + NUM_STARS_LEVEL_4 + NUM_STARS_LEVEL_5;
	private Star[] stars = new Star[NUM_STARS];
	private Star currentStar = null;

	public Sky()
	{
	    addMouseListener(this);
	    addMouseMotionListener(this);

	    ImageIcon icon = new ImageIcon(getClass().getResource("star1.png"));
	    Image star1 = icon.getImage();    

	    icon = new ImageIcon(getClass().getResource("star2.png"));
	    Image star2 = icon.getImage();

	    icon = new ImageIcon(getClass().getResource("star3.png"));
	    Image star3 = icon.getImage();

	    icon = new ImageIcon(getClass().getResource("star4.png"));
	    Image star4 = icon.getImage();
	    
	    icon = new ImageIcon(getClass().getResource("star5.png"));
	    Image star5 = icon.getImage();

	    // Add the stars in reverse order from farthest to closest. Since we use painters algorithm to paint them
	    // the one farthest need to be first in the array.
	    int i = 0;
	    for (int j = 0; j < NUM_STARS_LEVEL_5; j++)
	    {
		    stars[i++] = new Star(star5);
	    }
	    
	    for (int j = 0; j < NUM_STARS_LEVEL_4; j++)
	    {
		    stars[i++] = new Star(star4);
	    }	    

	    for (int j = 0; j < NUM_STARS_LEVEL_3; j++)
	    {
		    stars[i++] = new Star(star3);
	    }

	    for (int j = 0; j < NUM_STARS_LEVEL_2; j++)
	    {
		    stars[i++] = new Star(star2);
	    }

	    for (int j = 0; j < NUM_STARS_LEVEL_1; j++)
	    {
		    stars[i++] = new Star(star1);
	    }	    
	}

        public void paint(Graphics g)
        {
            // fill the background
            g.setColor(Color.black);
            g.fillRect(0, 0, getWidth(), getHeight());
			
	    Rectangle clip = g.getClipBounds();

	    // painters algorithm
	    for (int i = 0; i < stars.length; i++)
	    {
		// draw the stars
		stars[i].draw(g);
	    }
        }

        public void mousePressed(MouseEvent e)
        {
            Point p = e.getPoint();
	    for (int i = 0; i < NUM_STARS; i++)
	    {
		if (stars[i].hit(p))
		{
		    currentStar = stars[i];
		    currentStar.startDrag(p);
		}
	    }
        }
	
    // Slow version
//        public void mouseDragged(MouseEvent e)
//        {
//            if (currentStar != null)
//            {
//                currentStar.continueDrag(e.getPoint());
//                repaint();   /** INEFFICIENT - We only need to repaint the dirty region. Look at commented code for fast version **/
//            }
//        }

	// Fast version - also look at DragFrame.continueDrag()
        public void mouseDragged(MouseEvent e)
        {
            if (currentStar != null)
	    {
                Rectangle dirtyRegion = currentStar.continueDrag(e.getPoint());
                repaint(dirtyRegion.x, dirtyRegion.y, dirtyRegion.width, dirtyRegion.height);
            }
        }

        public void mouseReleased(MouseEvent e)
        {
            currentStar = null;
        }

        public void mouseEntered(MouseEvent e)
        {
            // do nothing
        }

        public void mouseExited(MouseEvent e)
        {
            // do nothing
        }


        public void mouseMoved(MouseEvent e)
        {
            // do nothing
        }

        public void mouseClicked(MouseEvent e)
        {
            // do nothing
        }
    }

    private class Star
    {
        private int dragX = 0;
        private int dragY = 0;
        private int x;
        private int y;
        private int width;
        private int height;
	
	private Image image;
		
	public Star(Image image)
	{
	    width = image.getWidth(null);
	    height = image.getHeight(null);
	    this.image = image;
	    
	    x = randomGenerator.nextInt(WIDTH);
	    y = randomGenerator.nextInt(HEIGHT);
	}

	public void draw(Graphics g)
	{
	    Rectangle clip = g.getClipBounds();
	    Rectangle myBounds = new Rectangle(x, y, width, height);
	    					    
	    if ((clip != null) && (!clip.intersects(myBounds)))
	    {
		    return;
	    }

	    g.drawImage(image, x, y, null);
	}


        public void startDrag(Point p)
        {
            dragX = p.x;
            dragY = p.y;
        }

	// Slow Version
//        public void continueDrag(Point p)
//        {
//            int deltaX = p.x - dragX;
//            int deltaY = p.y - dragY;
//            dragX = p.x;
//            dragY = p.y;
//
//            x = x + deltaX;
//            y = y + deltaY;
//        }
		
	// Fast version		
	public Rectangle continueDrag(Point p)
        {
            Rectangle oldBounds = new Rectangle(x, y, width, height);

            int deltaX = p.x - dragX;
            int deltaY = p.y - dragY;

            x = x + deltaX;
            y = y + deltaY;

            Rectangle newBounds = new Rectangle(x, y, width, height);
            Rectangle dirtyRegion = newBounds.union(oldBounds);

            dragX = p.x;
            dragY = p.y;

            return dirtyRegion;
        }


        public boolean hit(Point p)
        {
            if ((p.x >= x) && (p.y >= y) && (p.x <= x + width) && (p.y <= y + height))
            {
                return true;
            }

            return false;
        }


    }
}
