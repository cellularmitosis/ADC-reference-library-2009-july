/*

File: JavaQCView.java

Abstract:  CocoaComponent extension allowing integration of
	a Quartz Composer QCView into Java containers.  Includes
	a testcase in main().
	
	This sample depends on the Particle System composition found in
	/Developer/Examples/Quartz Composer/Motion Graphics Compositions
	which is installed with the Xcode 2.0 developer examples.

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

package apple.dts.samplecode.qccocoacomponent;

import java.awt.*;

// for main() unit test
import javax.swing.*;
import java.awt.event.*;
import java.io.File;

public class JavaQCView extends com.apple.eawt.CocoaComponent {
	
	static {
		System.loadLibrary("JavaQCView");
	}
	
	// Constants for sending messages to the underlying NSView
	// matches an enum defined in JavaQCView.m
	final static int LOAD_MESSAGE = 0;
	final static int START_MESSAGE = 1;
	
	// Instantiate the NSView on the native side and return it as a long
	public native long createNSViewLong();
	
	// Deprecated; just cast the correct createNSViewLong implementation
	public int createNSView() {
		return (int)createNSViewLong();
	}
	
	/*
	 * sendMessage wrapper methods
	 */ 
	public void loadComposition(String fullPath) {
		sendMessage(LOAD_MESSAGE, fullPath);
	}
	
	public void startRendering() {
		sendMessage(START_MESSAGE, null);
	}
	
	/*
	 *  Implementation of CocoaComponent abstracts
	 */
	final static Dimension MIN_SIZE		= new Dimension(200, 200);
	final static Dimension PREF_SIZE		= new Dimension(400, 400);
	final static Dimension MAX_SIZE		= new Dimension(600, 600);
	
	public Dimension getPreferredSize() {
		return PREF_SIZE;
	}
	
	public Dimension getMinimumSize () {
		return MIN_SIZE;
	}
	
	public Dimension getMaximumSize() {
		return MAX_SIZE;
	}
	
	
	public static void main(String[] args) {	
		// Turn off CocoaComponent compatibility in the code to save time on stage
		System.setProperty("com.apple.eawt.CocoaComponent.CompatibilityMode", "false");

		JFrame jf = new JFrame("QCCC Test");
		
		final JavaQCView view = new JavaQCView();
		jf.getContentPane().setLayout(new BorderLayout());
		jf.getContentPane().add(view);
		JPanel buttons = new JPanel();
		final JButton loadButton = new JButton("Load");
		final JButton playButton = new JButton("Play");
		
		ActionListener al = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (e.getSource() == loadButton) {
					view.loadComposition (File.separator + "Developer" + File.separator +
						"Examples" + File.separator + "Quartz Composer" + File.separator + 
						"Motion Graphics Compositions" + File.separator + "Particle System.qtz");
				} else {
					view.startRendering();
				}
			}
		};

		loadButton.addActionListener(al);
		playButton.addActionListener(al);
		buttons.add(loadButton);
		buttons.add(playButton);
		jf.getContentPane().add(buttons, BorderLayout.SOUTH);
		jf.setResizable(false);
		jf.pack();
		jf.setVisible(true);
	}
}