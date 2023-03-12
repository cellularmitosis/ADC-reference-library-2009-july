/*

File: BlockedEventQueue.java

Abstract:  
	This is an example which INCORRECTLY handles actionEvents.
	It is designed as a simple example of a java tool which
	will lock up and show the spinning cursor, providing a case
	where developers can use practice with our debugging tools 
	to identify and isolate a problem.
	
	This example consists of two main parts: 
	
	1) a Panel which counts the number of times that it
	 is asked to paint itself
	 
	2) a Button which, when triggered, waits for the next
	 paint event on the Panel, and then changes its name.

Version: <1.0>

© Copyright 2005 Apple Computer, Inc. All rights reserved.

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
import java.util.*;
import java.awt.*;
import java.awt.event.*;

public class BlockedEventQueue extends Frame {
	
	// An object used for inter-thread communication
	Object paintWatcher = new Object();

	//
	//	A Panel that just "counts" each paint request
	//
	class PaintCounter extends Panel {
		int paintcount = 0;
		
		public void paint( Graphics g) {
			super.paint(g);
			paintcount++;
			
			// Paint a "count" variable in the frame
			FontMetrics fm = g.getFontMetrics();
			int ascent = fm.getAscent();
			g.drawString("Paint Count: "+paintcount, 10, ascent+10);
	
			// handle inter-thread communication
			synchronized( paintWatcher ) {
				paintWatcher.notify();
			}
		}
		
		public Dimension getMinimumSize() {
			return new Dimension(100,200);
		}
	}

	//
	//	A Button that just waits for the next paint to fire
	//  The button does not actually trigger a repaint;
	//  to do so, just resize the window manually
	//
	class WaitButton extends Button implements ActionListener {
		public WaitButton() {
			super( "Wait for paint" );
			// setFont( new Font( "Lucida Grande", Font.PLAIN, 30 ));
			addActionListener(this);
		}
		
		public void actionPerformed(ActionEvent e) {
			synchronized (paintWatcher) {
				try {
					paintWatcher.wait();
				}
				catch( InterruptedException ix ) {}
			} 
			setLabel("Painted!");
		}

//		public void actionPerformed(ActionEvent e) {
//			(new Thread() { 
//				public void run() {
//					synchronized (paintWatcher) {
//						try {
//							paintWatcher.wait();
//						}
//						catch( InterruptedException ix ) {}
//					} 
//					setLabel("Painted!");
//				}
//			}).start();
//		}

/*		
  Above actionPerformed offloads the waiting to another thread, allowing
  the event queue (and the repaint we are waiting on) to proceed
  Use this fix to remedy the problem caused by the above actionPerformed method
*/
		
	}

	//
	//	A frame for the example, with one button and a panel;
	//
	public BlockedEventQueue() {
		super("Test Frame");
		setFont( new Font( "Lucida Grande", Font.PLAIN, 30 ));
		Panel counter = new PaintCounter();
		Button waiter = new WaitButton();
		Panel  panel = new Panel();
		panel.setLayout( new GridLayout( 3 ,1) );
		
		// Add the button
		panel.add(waiter);

		// Add a tiny "spacer" panel
		panel.add(new Panel());
		
		// Add the counter panel
		panel.add(counter);

		add(panel);
		setSize(300,300);
		setVisible(true);
	}
	
	public static void main(String args[]) {
		new BlockedEventQueue();
	}
}
//<<<<<<< BlockedEventQueue.java

//	New actionPerformed offloads the waiting to another thread, allowing
//  the event queue (and the repaint we are waiting on) to proceed
//  Use this fix to remedy the problem caused by the above actionPerformed method
/*		
		public void actionPerformed(ActionEvent e) {
			(new Thread() { 
				public void run() {
					synchronized (paintWatcher) {
						try {
							paintWatcher.wait();
						}
						catch( InterruptedException ix ) {
						}
					} 
					setLabel("Painted!");
				}
			}).start();
		}
=======
>>>>>>> 1.4*/
