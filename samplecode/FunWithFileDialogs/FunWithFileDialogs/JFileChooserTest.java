/*
	File:		JFileChooserTest.java
	
	Description:Demonstrates behavior of the (Aqua) Swing JFileChooser dialog in
				Mac OS X.  Also illustrates available runtime options to handle
				Mac-specific conditions.

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

import java.awt.*;
import java.awt.event.*;
import java.io.File;
import javax.swing.*;

public class JFileChooserTest extends JPanel implements ActionListener {

	JButton showJFC;
	JCheckBox navigateApps, navigatePkgs, multiSelect;
	ButtonGroup fileSelectionMode;
	JRadioButton files, folders, both;
	JTextArea selectionDisplay;
	JLabel resultsCaption;
	JPanel controls, selectionMode, buttonPanel, resultsPanel;
	JFileChooser jfc;
	
	// Property names and values for Mac OS X JFileChooser properties
	final static String PKG		= "JFileChooser.packageIsTraversable";
	final static String APP		= "JFileChooser.appBundleIsTraversable";
	final static String ALWAYS	= "always";
	final static String NEVER	= "never";
	
	public JFileChooserTest() {
		setLayout(new GridLayout(2, 1));
		add(controls = new JPanel(new GridLayout(5,1)));
		
		// UI for Mac OS X-specific behavior:  Navigation of 
		// .app bundles and .pkg installer packages.
		controls.add(navigateApps = new JCheckBox("Navigate .apps", true));
		controls.add(navigatePkgs = new JCheckBox("Navigate .pkgs", true));
		
		// UI for standard Multiple File Selection option
		controls.add(multiSelect = new JCheckBox("Multiple File Selection"));
		
		// group of JRadioButtons to handle standard file navigation  
		controls.add(selectionMode = new JPanel(new FlowLayout()));
		selectionMode.add(files = new JRadioButton("Files Only"));
		selectionMode.add(folders = new JRadioButton("Folders Only"));
		selectionMode.add(both = new JRadioButton("Files and Folders", true));
		fileSelectionMode = new ButtonGroup();
		fileSelectionMode.add(files);
		fileSelectionMode.add(folders);
		fileSelectionMode.add(both);
		// Map JFileChooser nav options to the appropriate radio button.
		// These will be checked before showing the JFileChooser.
		files.setActionCommand(String.valueOf(JFileChooser.FILES_ONLY));
		folders.setActionCommand(String.valueOf(JFileChooser.DIRECTORIES_ONLY));
		both.setActionCommand(String.valueOf(JFileChooser.FILES_AND_DIRECTORIES));
		
		
		controls.add(buttonPanel = new JPanel());
		buttonPanel.add(showJFC = new JButton("Show JFileChooser"));
		
		add(resultsPanel = new JPanel(new BorderLayout()));
		resultsPanel.add(selectionDisplay = new JTextArea());
		resultsPanel.add(resultsCaption = new JLabel("Selected File(s):"), BorderLayout.NORTH);
		
		showJFC.addActionListener(this);

		//	add(options);
		
		jfc = new JFileChooser();
	}

	public void actionPerformed (ActionEvent e) {
		// Mac OS X .app/.pkg options are set using putClientProperty
		// See constants above for keys and values.
		// Note that the properties need only be set before *showing* the dialog;
		// it can be set and re-set at will on a single instance
		jfc.putClientProperty(APP, navigateApps.isSelected() ? ALWAYS : NEVER);
		jfc.putClientProperty(PKG, navigatePkgs.isSelected() ? ALWAYS : NEVER);
		
		// Check Multi-selection setting
		jfc.setMultiSelectionEnabled(multiSelect.isSelected());
		
		// Check the selected radio button for standard navigation options.
		try {
			jfc.setFileSelectionMode(Integer.parseInt(fileSelectionMode.getSelection().getActionCommand()));
		} catch (NumberFormatException ex) { /* allow default behavior */}
		
		
		// Display selected file(s) in the text area
		int result = jfc.showOpenDialog(this);
		if (result == JFileChooser.APPROVE_OPTION) {
			selectionDisplay.setText(null);
			if (jfc.isMultiSelectionEnabled()) {
				// List em all
				File[] selections = jfc.getSelectedFiles();
				for (int i=0; i<selections.length; i++) {
					selectionDisplay.append(selections[i].getName());
					selectionDisplay.append("\n");
				}
			} else {
				// Get the single selection.  Yes, the API is actually
				// different based on the multiselect bit.
				selectionDisplay.setText(jfc.getSelectedFile().getName());
			}
		}
	}
	
	public static void main(String[] args) {
		JFrame jf = new JFrame("JFileChooserTest");
		jf.getContentPane().add(new JFileChooserTest());
		jf.pack();
		jf.setVisible(true);
	}
}
