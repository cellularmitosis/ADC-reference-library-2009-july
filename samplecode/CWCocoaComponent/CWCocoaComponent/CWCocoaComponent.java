/*

File: CWCocoaComponent.java

Abstract:  Testcase for the NSColorWell CocoaComponent example.

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
import javax.swing.*;
import java.awt.event.*;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

import apple.dts.samplecode.cwcocoacomponent.event.*;

public class CWCocoaComponent extends JFrame implements ActionListener {
    
    protected JLabel centerLabel;
    protected JPanel textPanel;
    protected Component buttonOrWell;
    
    public static boolean ON_MAC_OS_X, TIGER14_OR_LATER;
    
    public CWCocoaComponent() {
        super("Java Frame");

	getContentPane().setLayout(new BorderLayout());
	
	centerLabel = new JLabel("This JLabel Could Use Some Color...", JLabel.CENTER);
	centerLabel.setFont(centerLabel.getFont().deriveFont(36f));

        textPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 15, 15));
	
	// Use reflection to install our JavaColorWell; no direct references are made to it
	// so the class is not even loaded on non-Mac systems, since it loads a native lib
	// built for Mac OS X as well as referencing com.apple.eawt.CocoaComponent
	// JavaColorWell requires Mac OS X Tiger
	if (TIGER14_OR_LATER) try {
	    
	    Class colorWellClass = ClassLoader.getSystemClassLoader().loadClass("apple.dts.samplecode.cwcocoacomponent.JavaColorWell");
	    Class[] params = { ColorSelectionListener.class };
	    Constructor cnst = colorWellClass.getConstructor(params);
	    final JLabel finalCenterLabel = centerLabel;
	    Object args[] = { new ColorSelectionListener() {
		public void colorSelectionChange(ColorSelectionEvent cse) {
		    // Change the label to the new color from the native NSColorWell/ColorPanel
		    finalCenterLabel.setForeground(cse.getColor());
		}
	    } };	
	    buttonOrWell = (JavaColorWell)cnst.newInstance(args);
	} catch (Exception ex) {
	    buttonOrWell = null;
	}
	
	if (buttonOrWell == null) { // Didn't create the colorwell; we're not on Mac OS X Tiger
	    JButton jb = new JButton("Colors...");
	    jb.addActionListener(this);
	    buttonOrWell = jb;
	}

	textPanel.add(buttonOrWell);
	
	getContentPane().add(textPanel, BorderLayout.NORTH);
	getContentPane().add(centerLabel);

        setSize(800, 200);
    }
    
    // This listener is called on non-Tiger systems to show a JColorChooser in the absence of native integration 
    public void actionPerformed(ActionEvent ae) {
	java.awt.Color newColor = JColorChooser.showDialog(this, "Choose a new Label Color", centerLabel.getForeground());
	if (newColor != null) {
	    centerLabel.setForeground(newColor);
	}
    }
    
    public static void main(String args[]) {
	// Version / OS detection to make sure it's safe to load the ColorWell
	ON_MAC_OS_X = System.getProperty("os.name").toLowerCase().startsWith("mac os x");
	float osVersion = Float.parseFloat(System.getProperty("os.version").substring(0, 4));
	float javaVersion = Float.parseFloat(System.getProperty("java.version").substring(0,3));
	TIGER14_OR_LATER = ON_MAC_OS_X && (osVersion >= 10.4f) && (javaVersion >= 1.4f);
	
	// Turn off CocoaComponent compatibility in the code to save time on stage
	System.setProperty("com.apple.eawt.CocoaComponent.CompatibilityMode", "false");
	
        new CWCocoaComponent().setVisible(true);
    }
}