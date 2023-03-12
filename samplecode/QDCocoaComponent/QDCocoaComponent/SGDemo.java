/*
	File:		SGDemo.java
	
	Description:	Main class for this sample.  This sample shows how to use an
                        com.apple.eawt.CocoaComponent to display a NSQuickDraw port.
                        This sample uses the QuickTime Sequence Grabber for its live
                        image source.  It also shows running the Carbon code on the
                        main thread to prevent re-entrancy and threading issues.

	Copyright: 	© Copyright 2003 - 2004 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Appleâ€™s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):

*/

package apple.dts;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class SGDemo extends JFrame implements ActionListener {
    /* The QDCanvas object targetCanvas is the canvas we */
    /* will be decompressing to */
    QDCanvas	targetCanvas;
    
    /* Pointer to the native object that will do the Sequence Grabbing */
    int		sqDemoObject = 0;
    
    /* Button used to start the SequenceGrabbing */
    JButton 	startButton;
    
    public SGDemo(String name) {
        super(name);
        
        getContentPane().setLayout(new BorderLayout());

        /* Build the control panel and add a start button */
        Panel controlPanel = new Panel();
        startButton = new JButton("Start");
        startButton.addActionListener(this);
        controlPanel.add(startButton);

        /* Create the Canvas we will be rendering to */
        targetCanvas = new QDCanvas();
        
        getContentPane().add(targetCanvas,BorderLayout.CENTER);
        getContentPane().add(controlPanel,BorderLayout.SOUTH);
        
        targetCanvas.setSize(320,240);
        pack();
        setVisible(true);
        
        /* Create our native object that will be used to do */
        /* the sequence grabbing */
        sqDemoObject = makeSGObject(targetCanvas.getView());
    }
    
    /* Return a pointer to the native object that will be */
    /* doing the sequence grabbing */
    public int getSGObject() {
        return sqDemoObject;
    }
    
    /* method used to create the native object that will be doing */
    /* the sequence grabbing.  It accepts a pointer to a native QDCanvas */
    /* that data will be rendered to */
    native int  makeSGObject(int view);
    
    /* method used to actually start the sequence grabber.  It accepts */
    /* an integer that is actually a pointer to the native Objective-C */
    /* object */
    public native void startSG(int sgObject);
    
    /* Called by the start button to start the sequence grabbing */
    public void actionPerformed(ActionEvent newEvent) {
            startButton.setEnabled(false);
            startSG(getSGObject());
    }	

    public static void main(String args[]) {
        SGDemo demo = new SGDemo("Target");
    }
}
