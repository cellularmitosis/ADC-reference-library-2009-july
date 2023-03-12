/*

File: JSheetDelegate.java

Abstract: JNI integration class for showing sheets in AWT/Swing

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

package apple.dts.samplecode.jsheets;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.util.ArrayList;
import javax.swing.*;

import com.apple.eawt.*;

import apple.dts.samplecode.jsheets.event.*;

public class JSheetDelegate {

	final static int OPEN_PANEL = 0;
	final static int SAVE_PANEL = 1;
	
	static {
		System.loadLibrary("SheetDelegate");
		initMethodIDs();
	}
		
	/*
	 * Sheet display methods; modeled after JFileChooser save/open dialogs
	 * 
	 * Because of threading rules between AWT and AppKit we cannot block like JFileChooser;
	 * pass in a listener to receive status callbacks when the sheet is dismissed
	 * 
	 * This class can be modified to show other types of sheets using Java constants
	 * and corresponding enum on the native side
	 */
	public static void showOpenSheet (Frame parent, OpenSheetListener osl) {
		showSheet(OPEN_PANEL, parent, osl);
	}
	
	public static void showSaveSheet (Frame parent, SaveSheetListener ssl) {
		showSheet(SAVE_PANEL, parent, ssl);
	}
	
	private static void showSheet(int sheetType, Frame parent, SheetListener sl) {
		if (parent == null || sl == null) {
			throw new IllegalArgumentException ("Need a valid parent Frame and SheetListener");
		}
		nativeShowSheet(sheetType, parent, sl);
	}
	
	/*
	 * Callbacks for firing SheetEvents on the AWT Event Queue
	 * Marked as private; Java code should not call these methods
	 */
	// Called from openPanelDidEnd: delegate method in Cocoa if user clicked Open
	private static void fireOpenSheetFinished(final OpenSheetListener osl, final String[] filenames) {
		// This callback occurs on the main AppKit thread
		// Changing the deadlock flag to true will fire the AWT events directly from that thread,
		// resulting in a deadlock: AppKit is waiting for us to return but our listener is blocked
		// showing a JOptionPane, which needs AppKit resources to draw itself
		boolean deadlock = false;
		if (deadlock) {
			osl.openSheetFinished(new SheetEvent(filenames)); // BAD! Don't call AWT directly from the AppKit thread
		} else {
			// GOOD! Get onto the AWT Event Queue before firing / processing event-based AWT functionality
			EventQueue.invokeLater(new Runnable() {
				public void run() {
					osl.openSheetFinished(new SheetEvent(filenames));
				}
			});
		}
	}
	
	// Called from savePanelDidEnd: delegate method in Cocoa if user clicked Save
	private static void fireSaveSheetFinished(final SaveSheetListener ssl, final String filename) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				ssl.saveSheetFinished(new SheetEvent(filename));
			}
		});
	}
	
	// Called from open/savePanelDidEnd: if user clicked Cancel
	private static void fireSheetCancelled(final SheetListener sl) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				sl.sheetCancelled();
			}
		});
	}

	private static native void initMethodIDs();	
	private static native void nativeShowSheet(int sheetType, Component c, SheetListener sl);
	
	private JSheetDelegate() {}
}