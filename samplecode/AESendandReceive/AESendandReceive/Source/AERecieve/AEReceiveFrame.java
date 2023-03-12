/*
	File:		AEReceiveFrame.java
	
	Description:	 * Sample showing how to send and receive AppleEvents using JDirect 3.

	Author:		EPJ, LB

	Copyright: 	© Copyright 1999 - 2002 Apple Computer, Inc. All rights reserved.
	
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
	* version 1.0
	* 4/15/1999 Shipped as 'AppleEvent Send and Receive' sample.
	* version 2.0
	* 4/2/2002  Updated to Swing

	* This class contains this sample's user interface for receiving AppleEvents.
*/
package com.apple.dts.samplecode.aesendandreceive.aereceive;

import java.awt.Color;
import java.awt.Font;
import java.awt.Insets;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JFrame;

import com.apple.dts.samplecode.aesendandreceive.shared.*;

public class AEReceiveFrame extends JFrame
{
	public AEReceiveFrame()
	{
		//{{INIT_CONTROLS
		GridBagLayout gridBagLayout;
		gridBagLayout = new GridBagLayout();
		getContentPane().setLayout(gridBagLayout);
		setVisible(false);
		setSize(466,90);
		setBackground(new Color(-1052689));
		label1 = new javax.swing.JLabel("Message:");
		label1.setBounds(5,0,50,21);
		label1.setFont(new Font("SansSerif", Font.BOLD, 9));
		GridBagConstraints gbc;
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 1;
		gbc.gridwidth = 2;
		gbc.anchor = GridBagConstraints.WEST;
		gbc.fill = GridBagConstraints.NONE;
		gbc.insets = new Insets(0,5,0,0);
		((GridBagLayout)getContentPane().getLayout()).setConstraints(label1, gbc);
		getContentPane().add(label1);
		label2 = new javax.swing.JLabel("Status:");
		label2.setBounds(5,65,40,21);
		label2.setFont(new Font("SansSerif", Font.BOLD, 9));
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 5;
		gbc.anchor = GridBagConstraints.WEST;
		gbc.fill = GridBagConstraints.NONE;
		gbc.insets = new Insets(0,5,0,0);
		((GridBagLayout)getContentPane().getLayout()).setConstraints(label2, gbc);
		getContentPane().add(label2);
		divider1 = new Divider();
		divider1.setBounds(5,64,434,1);
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 4;
		gbc.gridwidth = 2;
		gbc.weightx = 0.5;
		gbc.anchor = GridBagConstraints.WEST;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.insets = new Insets(0,5,0,5);
		((GridBagLayout)getContentPane().getLayout()).setConstraints(divider1, gbc);
		getContentPane().add(divider1);
		statusLabel = new javax.swing.JLabel("Initializiing...");
		statusLabel.setBounds(50,65,394,21);
		statusLabel.setFont(new Font("SansSerif", Font.PLAIN, 9));
		gbc = new GridBagConstraints();
		gbc.gridx = 2;
		gbc.gridy = 5;
		gbc.weightx = 0.5;
		gbc.anchor = GridBagConstraints.WEST;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.insets = new Insets(0,5,0,0);
		((GridBagLayout)getContentPane().getLayout()).setConstraints(statusLabel, gbc);
		getContentPane().add(statusLabel);
		messageLabel = new javax.swing.JLabel("");
		messageLabel.setBounds(10,21,434,21);
		messageLabel.setFont(new Font("SansSerif", Font.PLAIN, 9));
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 2;
		gbc.gridwidth = 2;
		gbc.weightx = 0.5;
		gbc.anchor = GridBagConstraints.WEST;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.insets = new Insets(0,10,0,0);
		((GridBagLayout)getContentPane().getLayout()).setConstraints(messageLabel, gbc);
		getContentPane().add(messageLabel);
		panel1 = new javax.swing.JPanel();
		panel1.setLayout(null);
		panel1.setBounds(0,42,444,22);
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 3;
		gbc.gridwidth = 2;
		gbc.weightx = 0.5;
		gbc.weighty = 0.5;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(0,0,0,0);
		((GridBagLayout)getContentPane().getLayout()).setConstraints(panel1, gbc);
		getContentPane().add(panel1);
		setTitle("AEReceive");
		//}}

		//{{INIT_MENUS
		//}}

		//{{REGISTER_LISTENERS
		SymWindow aSymWindow = new SymWindow();
		this.addWindowListener(aSymWindow);
		//}}

		setLocation(50, 180);

		//Create a new timer to handle status messages.
		timer = new Timer(kEraseSleep, new TimerCallback()
				{
					public void timeIsUp()
					{
						isErase = false;
						try
						{
							setStatus("Idle");
						}
						finally
						{
							isErase = true;
						}
					}
				});
		
		isErase = true;
		
		//Create a new AEReceive object so we can register out AppleEvent handler
		aeReceive = new AEReceive();
		//Register a new instance of our ActionListener with the AEReceive object
		//so we get notified when an AppleEvent is received.
		aeReceive.addActionListener(new Action());
	}
	
	/**
	 * The entry point to our application
	 */	
	public static void main(String[] args)
	{
		(new AEReceiveFrame()).setVisible(true);
	}

	/**
	 * Gets called by the framework when this component is added to the component hierarchy.
	 * Overriden here to install our AppleEvent handler.
	 */
	public void addNotify()
	{
		super.addNotify();
		
		setStatus("Installing AppleEvent handler...");
		try
		{
			aeReceive.installAEHandler();
		}
		catch (NativeException exc)
		{
			setStatus("Failed to install AppleEvent handler. (#" + exc.getErrNum() + ")");
			return;
		}

		setStatus("AppleEvent handler successfully installed.");
	}

	/**
	 * Set the text of the status label
	 * param the message to use.
	 */
	protected void setStatus(String message)
	{
		statusLabel.setText(message);
		if (isErase)
			timer.start();
	}
	
	/**
	 * Set the text of the message label
	 * param the message to use.
	 */
	protected void setMessage(String message)
	{
		messageLabel.setText(message);
	}

	//{{DECLARE_CONTROLS
	javax.swing.JLabel label1;
	javax.swing.JLabel label2;
	Divider divider1;
	javax.swing.JLabel statusLabel;
	javax.swing.JLabel messageLabel;
	javax.swing.JPanel panel1;
	//}}

	//{{DECLARE_MENUS
	//}}

	class SymWindow extends java.awt.event.WindowAdapter
	{
		public void windowClosing(java.awt.event.WindowEvent event)
		{
			Object object = event.getSource();
			if (object == AEReceiveFrame.this)
				AEReceiveFrame_WindowClosing(event);
		}
	}
	
	void AEReceiveFrame_WindowClosing(java.awt.event.WindowEvent event)
	{
		setVisible(false);		 // hide the AEReceiveFrame
		dispose();			// free the system resources
		System.exit(0);		// close the application
	}

	/**
	 * An inner class designed to listen for ActionEvents
	 * from the AEReceive class.
	 */	
	class Action implements ActionListener
	{
		public void actionPerformed(ActionEvent event)
		{
			setMessage(event.getActionCommand());
			setStatus("Received AppleEvent.");
		}
	}

	/**
	 * The time in milliseconds to wait before removing the previous status message.
	 */
	protected static final int kEraseSleep = 1250;
	/**
	 * Our AEReceive object which allows us to register the AppleEvent Handler
	 */
	protected AEReceive aeReceive;
	/**
	 * An internal flag to prevent recursion in the status message handling code.
	 */
	protected boolean isErase;
	/**
	 * A timer to keep track of displaying the status message.
	 */
	protected Timer timer;
}