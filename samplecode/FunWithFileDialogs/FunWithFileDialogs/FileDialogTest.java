/*
	File:		FileDialogTest.java
	
	Description:Demonstrates behavior of the AWT FileChooser dialog in Mac OS X.
				Also illustrates available runtime options to handle Mac-specific
				conditions.
				
				This sample is designed for Java 1.4.1 and later on Mac OS X.  
				
	Author:		md

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
			04282003	mdrance	Initial Submission

*/

package com.apple.dts.samplecode.funwithfiledialogs;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;


public class FileDialogTest extends JPanel implements ActionListener {

	FileDialog dialog;
	JButton open, save;
	JCheckBox fileDialogProps;
	JPanel resultsPanel, controls, buttons;
	JTextArea selectionDisplay;
	JLabel resultsCaption;
	
	// The only Mac-special property for AWT File Dialogs
	// Treats .apps and .pkgs as files or folders based on setting.
	final static String NAV_PACKAGES = "apple.awt.use-file-dialog-packages";
	
	public FileDialogTest( Frame owner) {
	
		dialog = new FileDialog(owner);
		
		setLayout(new GridLayout(2, 1));
		add(controls = new JPanel(new GridLayout(2, 1)));
		controls.add(fileDialogProps = new JCheckBox("Treat packages like files"));
		controls.add(buttons = new JPanel(new FlowLayout()));
		buttons.add(open = new JButton("Open"));
		buttons.add(save = new JButton("Save"));
		
		add(resultsPanel = new JPanel(new BorderLayout()));
		resultsPanel.add(selectionDisplay = new JTextArea());
		resultsPanel.add(resultsCaption = new JLabel("Selected File:"), BorderLayout.NORTH);
		
		open.addActionListener(this);
		save.addActionListener(this);	
		fileDialogProps.addActionListener(this);
	}
		
	public void actionPerformed(ActionEvent e) {
		if (e.getSource() == open) {
			dialog.setMode(FileDialog.LOAD);
		} else if (e.getSource() == save) { 
			dialog.setMode(FileDialog.SAVE);
		} else if (e.getSource() == fileDialogProps) {
			// Based on UI selection, make .pkgs and .apps navigable.
			// Note that the property needs only be set before *showing* the dialog;
			// it can be set and re-set at will on a single instance
			System.setProperty(NAV_PACKAGES, String.valueOf(fileDialogProps.isSelected()));
			return;
		}
		
		dialog.setVisible(true);
		selectionDisplay.setText(dialog.getFile());
	}	
	
	public static void main(String[] args) {	
		JFrame jf = new JFrame("FileDialogTest");
		jf.getContentPane().add(new FileDialogTest(jf));
		jf.pack();
		jf.setVisible(true);
	}		

}
	
	