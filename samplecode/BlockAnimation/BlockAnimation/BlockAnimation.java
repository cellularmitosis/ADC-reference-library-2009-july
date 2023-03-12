/*

File: BlockAnimation.java

Abstract:  

        PLEASE NOTE THAT THIS IS DEEPLY FLAWED CODE!
    
        This is an example which INCORRECTLY handles
        animation. It is designed as a simple example which
        developers can use with our debugging tools
        to identify and isolate a problem.

        This example consists of two main parts: 

        1) a JFrame with lots of JPanels inside
         
        2) many JPanels which periodically change
           their background color and repaint

        The primary flaw is that this example creates one 
		timer thread per JPanel (500+ threads) rather
		than juts the single timer thread it needs.

Version: <1.0>

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
Copyright © 2006 Apple Computer, Inc., All Rights Reserved

*/

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.util.Random;
import java.util.TimerTask;

import javax.swing.JFrame;
import javax.swing.JPanel;

public class BlockAnimation {
	
	// FIELDS
	
	private static final boolean USE_TIMERS = true;	// Whether to use timers
	private static final int TIMER_BASE = 50;		// Base for how often timers execute <millis> : [NEEDS SOME FIXING]
	private static final int VARIANCE = 100;		// Variance ammount in timer execution <millis>
	private static final int SIDE_LENGTH = 24;		// Panels per side of the display Frame 
	private static final Dimension SQUARE  = new Dimension(12, 12);		// Dimension of animated panel
	private static final Random rand  = new Random();
	
	// CONSTRUCTORS
	
	// BlockAmination - creates a Swing frame, fills it with panels that change color, and sets it visible.
	public BlockAnimation() {
		// set up the JFrame
		JFrame f = new JFrame("Block Animation");
		f.setUndecorated(true);
		f.getContentPane().setLayout(new GridLayout(SIDE_LENGTH,SIDE_LENGTH,0,0));

		// add panels
		for (int i = 0; i < SIDE_LENGTH*SIDE_LENGTH; i++) {
			f.getContentPane().add(new AnimatedPanel(USE_TIMERS));
		}
		
		// make the frame visible.
		f.pack();
		f.setVisible(true);                
	}
	
	// METHODS
	
	// randomColor - generates a Color with a random RGB value.
	public static Color randomColor() {
		int r = 128 + rand.nextInt( 128);
		int g = 128 + rand.nextInt( 128);
		int b = 128 + rand.nextInt( 128);
		Color c = new Color( r, g, b );
		return( c );
	}
	
	// main - creates a BlockAnimation object.
	//        The main() method does not return.
	public static void main (String[] args) {
		new BlockAnimation();
	}
	
	// INNER CLASSES
		
	// AnimatedPanel - a panel that changed color either by continiously calling paint()
	//                 or with a Timer (if isTimed == true).
	private class AnimatedPanel extends JPanel {
		private java.util.Timer timer; 
	
		public AnimatedPanel(boolean isTimed) {
			if (isTimed) {
				timer = new java.util.Timer();
				int phase = rand.nextInt(VARIANCE); // start panel animation out of phase, so we avoid a "sweep" affect
				int drift = rand.nextInt(VARIANCE); // each panel updates at a slightly different rate, cuz it looks better
				timer.scheduleAtFixedRate(new PanelTimerTask(this), phase, TIMER_BASE + drift);
			} 
		}
		
		public void paint(Graphics g) {
			// fill with a random color
			g.setColor(randomColor());
			g.fillRect(0, 0, SQUARE.width, SQUARE.height);

            // continuously repaint if there is no timer
			if (timer == null) {
				repaint();
			}
		}
		
		public Dimension getPreferredSize() {
			return SQUARE;
		}
	}
	
	// PanelTimerTask - a TimerTask that calls repaint() on its associated AnimatedPanel.
	private class PanelTimerTask extends TimerTask {
		private JPanel panel;
		
		public PanelTimerTask(AnimatedPanel panel) {
			this.panel = panel;
		}

		public void run() {
			panel.repaint();
		}
	}
}
