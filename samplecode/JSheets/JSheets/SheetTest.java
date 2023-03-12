/*

File: SheetTest.java

Abstract: Testcase for the Sheets-in-Java support class.  Receives source/destination filenames
	from the NSOpenPanel/NSSavePanel sheets that are shown, and displays the filenames in a 
	JOptionPane to demonstrate that the information was successfully passed from Cocoa.

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

import java.awt.Toolkit;
import java.awt.event.*;
import java.io.File;
import javax.swing.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import com.apple.eawt.*;

import apple.dts.samplecode.jsheets.event.*;

public class SheetTest extends JFrame implements ActionListener, OpenSheetListener, SaveSheetListener {
	
	protected JMenu fileMenu;
	protected JMenuBar jmb;
	protected JMenuItem openMI, saveMI;
	
	public static boolean ON_MAC_OS_X = false;
	public static boolean TIGER14_OR_LATER = false;
	
	private static Class sheetDelegateClass;
	private static Method openSheetMethod, saveSheetMethod;
	
	public SheetTest () {
		super("Sheets in Swing");

		jmb = new JMenuBar();
		fileMenu = new JMenu("File");
		openMI = new JMenuItem("Open...");
		openMI.addActionListener(this);
		openMI.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
		saveMI = new JMenuItem("Save...");
		saveMI.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
		saveMI.addActionListener(this);
		fileMenu.add(openMI);
		fileMenu.add(saveMI);
		jmb.add(fileMenu);
		setJMenuBar(jmb);
		
		setSize(320, 240);
	}
	
	// If the JSheetDelegate loaded successfully, we're on Mac OS X Tiger and can
	// make the calls to show sheets; otherwise show the standard JFileChooser
	public void actionPerformed(ActionEvent ae) {
		if (ae.getSource() == openMI) {			
			if (sheetDelegateClass != null && openSheetMethod != null) {
				Object[] args = { this, this };
				try {
					openSheetMethod.invoke(sheetDelegateClass, args);
					return;
				} catch (Exception ex) {
					ex.printStackTrace();
					// do not return; fall back to JFileChooser below
				}
			}
			
			JFileChooser jfc = new JFileChooser();
			jfc.setMultiSelectionEnabled(true);
			jfc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
			int result = jfc.showOpenDialog(this);
			if (result == JFileChooser.APPROVE_OPTION) {
				openFiles(jfc.getSelectedFiles());
			} else if (result == JFileChooser.CANCEL_OPTION) {
				dialogCancelled();
			}
		} else {
			if (sheetDelegateClass != null && saveSheetMethod != null) {
				Object[] args = { this, this };
				try {
					saveSheetMethod.invoke(sheetDelegateClass, args);
					return;
				} catch (Exception ex) {
					ex.printStackTrace();
					// do not return; fall back to JFileChooser below
				}
			}	
			
			JFileChooser jfc = new JFileChooser();
			int result = jfc.showSaveDialog(this);
			if (result == JFileChooser.APPROVE_OPTION) {
				saveFile(jfc.getSelectedFile());
			} else if (result == JFileChooser.CANCEL_OPTION) {
				dialogCancelled();
			}		
		}
	}
		
	public void openFiles(String[] filePaths) {
		String list = "";
		for (int i = 0; i < filePaths.length; i++) {
			list += filePaths[i] + "\n";
		}
		
		JOptionPane.showMessageDialog(this, list);
	}
	
	public void openFiles(File[] files) {
		String list = "";
		for (int i = 0; i < files.length; i++) {
			list += files[i].getAbsolutePath() + "\n";
		}
		
		JOptionPane.showMessageDialog(this, list);
	}
	
	public void saveFile(File newFile) {
		saveFile(newFile.getAbsolutePath());
	}
	
	public void saveFile(String filename) {
		JOptionPane.showMessageDialog(this, filename);	
	}
	
	public void dialogCancelled() {
		System.out.println("User cancelled open/save");
	}
	
	
	/*
	 * SheetListener methods for open/save/cancel
	 * Defer to pure-Java file handling above
	 */
	public void openSheetFinished(SheetEvent se) {
		openFiles(se.getFilenames());
	}
	
	public void saveSheetFinished(SheetEvent se) {
		saveFile(se.getFilename());
	}
	
	public void sheetCancelled() {
		dialogCancelled();
	}
	
	public static void main(String[] args) {
		ON_MAC_OS_X = System.getProperty("os.name").toLowerCase().startsWith("mac os x");
		float osVersion = Float.parseFloat(System.getProperty("os.version").substring(0, 4));
		float javaVersion = Float.parseFloat(System.getProperty("java.version").substring(0,3));
		TIGER14_OR_LATER = ON_MAC_OS_X && (osVersion >= 10.4f) && (javaVersion >= 1.4f);
		
		// Only load this class if on Mac OS X Tiger; this prevents the native library from 
		// being loaded on the wrong platforms
		// Also load the necessary methods for showing our sheets; these will also be called reflectively
		if (TIGER14_OR_LATER) {
			try {
				sheetDelegateClass = ClassLoader.getSystemClassLoader().loadClass("apple.dts.samplecode.jsheets.JSheetDelegate");
				Class[] defArgs = { java.awt.Frame.class, OpenSheetListener.class };
				openSheetMethod = sheetDelegateClass.getDeclaredMethod("showOpenSheet", defArgs);
				defArgs[1] = SaveSheetListener.class;
				saveSheetMethod = sheetDelegateClass.getDeclaredMethod("showSaveSheet", defArgs);
			} catch (Exception ex) {
				ex.printStackTrace();
			}
		}

		new SheetTest().setVisible(true);
	}
}