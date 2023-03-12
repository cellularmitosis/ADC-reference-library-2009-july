/*

File: JavaColorWell.java

Abstract:  CocoaComponent extension allowing NSColorWell
	to be embedded in Java containers.  Maintains a set 
	of listeners to communicate color changes: note the
	use of invokeLater to ensure listener notification occurs
	on the AWT Event Queue.

Version: 1.0

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

package apple.dts.samplecode.cwcocoacomponent;

import java.awt.*;
import java.util.ArrayList;
import java.util.Iterator;
import apple.dts.samplecode.cwcocoacomponent.event.*;

public class JavaColorWell extends com.apple.eawt.CocoaComponent {

	static {
		System.loadLibrary("JavaColorWell");
		// initialize any JNI callbacks at classload; jmethodIDs remain valid
		initMethodIDs();
	}
	
	// Constants for CocoaComponent sendMessage calls 
	// Matched to the enum in JavaColorWell.m
	final static int CLOSE_PANEL	= 0;
	final static int DEACTIVATE		= 1;
	final static int REMOVE_NOTIFY	= 2;
	
	// Constants for max/min/preferred size
	private static Dimension DEFAULT_SIZE	= new Dimension(52, 24);	// Default NSColorWell size in Interface Builder
	private static Dimension MAX_SIZE		= new Dimension(104, 48);
	
	// ColorSelectionListener storage
	private ArrayList listeners = new ArrayList();
	
	public JavaColorWell() { }
	
	public JavaColorWell(ColorSelectionListener csl) {
		addColorSelectionListener(csl);
	}
		
	// This is a callback function for the underlying NSColorWell extension
	// Calling this method from Java will NOT sync the colorwell
	private void cocoaColorChange(final float r, final float g, final float b, final float a) {
		Iterator i = listeners.iterator();
		final Color color = new Color(r, g, b, a);
		while (i.hasNext()) {
			final ColorSelectionEvent cse = new ColorSelectionEvent (color, this);
			final ColorSelectionListener csl = (ColorSelectionListener)i.next();
			EventQueue.invokeLater(new Runnable() {
				public void run() {
					csl.colorSelectionChange(cse);
				}
			});
		}
	}
	
	// Tell the peer we've been removed from the hierarchy in case cleanup is needed
	// CocoaComponent implementation will release the peer itself
	public void removeNotify() {
		sendMessage(REMOVE_NOTIFY, null);
		super.removeNotify();
	}
	
	/*
	 * Straightforward listener support
	 */
	public void addColorSelectionListener(ColorSelectionListener csl) {
		if (listeners.contains(csl)) return;
		listeners.add(csl);
	}
	
	public void removeColorSelectionListener(ColorSelectionListener csl) {
		listeners.remove(csl);
	}

	/*
	 * CocoaComponent implements
	 */
	public Dimension getPreferredSize() {
		return DEFAULT_SIZE;
	}
	
	public Dimension getMinimumSize() {
		return DEFAULT_SIZE;
	}
	
	public Dimension getMaximumSize() {
		return MAX_SIZE;
	}
	
	/*
	 * Native methods
	 */
	public static native void initMethodIDs();
	// createNSViewLong is strongly recommended in CocoaComponent extensions;
	// as of Mac OS X Tiger the 32-bit (int) createNSView is deprecated
	public native long createNSViewLong();
	
	public int createNSView() {
		return (int) createNSViewLong();
	}
}