/*
	File:		TransitionPanel.java
	
	Description:	This demo program shows how to use QuickTime's visual effects architecture 
                        as applied to two source images. The effects are applied in realtime - 
                        controlled by the user settings in the window.

	Author:		Apple Computer, Inc.

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
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
            11/22/2002	md	new SampleCode revisions

*/

package qteffect;

import java.awt.*;
import java.awt.event.*;

import quicktime.*;
import quicktime.std.image.*;
import quicktime.app.image.QTTransition;
import quicktime.app.display.*;
import PlayQTEffectApp;
/**
 * A simple panel of AWT objects to demonstrate how AWT can be used to control a
 * QuickTime movie.
 */
public class TransitionPanel extends Panel implements Errors {
//_________________________ CLASS METHODS
	public TransitionPanel (QTTransition ef, QTCanvas c) throws QTException {
		this.transition = ef;
		final QTCanvas cv = c;
		
		GridBagLayout gb = new GridBagLayout();
		setLayout (gb);
		setFont (new Font("Helvetica", Font.BOLD, 10));

		GridBagConstraints cons = new GridBagConstraints();
		cons.fill = GridBagConstraints.HORIZONTAL;

		cons.gridheight = 1;		
		cons.insets = new Insets (2, 2, 2, 2);
		
		cons.gridy = 0;

		cons.gridwidth = 3;
		cons.gridx = 0;
		add (runEffectButton, cons);
		
		cons.gridx = 3;
		add (chooseEffectButton, cons);

		++cons.gridy;
		cons.gridwidth = 2;
		
		cons.gridx = 0;
		add (timeLabel, cons);
		
		cons.gridx = 2;
		add (timeField, cons);
		
		cons.gridx = 4;
		add (timeButton, cons);

		++cons.gridy;
		cons.gridwidth = 2;
		
		cons.gridx = 0;
		add (frameLabel, cons);
		
		cons.gridx = 2;
		add (frameField, cons);
		
		cons.gridx = 4;
		add (frameButton, cons);

		++cons.gridy;
		cons.gridwidth = 2;
		
		cons.gridx = 0;
		add (fpsLabel, cons);

		cons.gridx = 2;
		add (fpsField, cons);
				
		timeField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					String output = "";
					for (int i = 0; i < str.length(); i++) {
						if (Character.isDigit (str.charAt (i)))
							output += str.charAt (i);
						else
							break;
					}
					try {
						transition.setTime (new Integer (output).intValue());
						timeField.setText (Integer.toString (transition.getTime()));
						frameField.setText (Integer.toString (transition.getFrames()));
					} catch (QTException e) {
						e.printStackTrace();
					}
				}
			});
		frameField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					String output = "";
					for (int i = 0; i < str.length(); i++) {
						if (Character.isDigit (str.charAt (i)))
							output += str.charAt (i);
						else
							break;
					}
					try {
						transition.setFrames (new Integer (output).intValue());
						timeField.setText (Integer.toString (transition.getTime()));
						frameField.setText (Integer.toString (transition.getFrames()));
					} catch (QTException e) {
						e.printStackTrace();
					}
				}
			});
		fpsField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					String output = "";
					for (int i = 0; i < str.length(); i++) {
						if (Character.isDigit (str.charAt (i)))
							output += str.charAt (i);
						else
							break;
					}
					try {
						transition.setFramesPerSecond (new Integer (output).intValue());
						frameField.setText (Integer.toString (transition.getFrames()));
					} catch (QTException e) {
						e.printStackTrace();
					}
				}
			});
		runEffectButton.addActionListener (new ActionListener () {				
				public void actionPerformed (ActionEvent event) {
					try {
						transition.doTransition();
						if (transition.isProfiled()) {
							String profileString = "Transition Profile:"
								+ "requestedDuration:" + transition.getTime()
								+ ",actualDuration:" + transition.profileDuration()
								+ ",requestedFrames:" + transition.getFrames()
								+ ",framesRendered=" + transition.profileFramesRendered()
								+ ",averageRenderTimePerFrame=" + (transition.profileDuration() / transition.profileFramesRendered());
							System.out.println (profileString);
						}
							
					} catch (QTException e) {
						if (e.errorCode() != userCanceledErr)
							e.printStackTrace();
					}
				}
			});

		chooseEffectButton.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					if (PlayQTEffectApp.useEnhancedParameterDialog) {
						try {
							PlayQTEffectApp.showEffectDialog (transition);
						} catch (QTException e) {
							if (e.errorCode() != userCanceledErr)
								e.printStackTrace();
						} catch (java.io.IOException ioe) {
							ioe.printStackTrace();
						}
					} else {
						try {
							PlayQTEffectApp.showDialog (transition);
						} catch (QTException e) {
							if (e.errorCode() != userCanceledErr)
								e.printStackTrace();
						}
					}
				}
			});

		frameButton.addItemListener (new ItemListener () {
				public void itemStateChanged (ItemEvent event) {
					transition.doTime (false);
				}
			});		
		timeButton.addItemListener (new ItemListener () {
				public void itemStateChanged (ItemEvent event) {
					transition.doTime (true);
				}
			});	
	
		timeField.setText (Integer.toString (transition.getTime()));
		frameField.setText (Integer.toString (transition.getFrames()));
		fpsField.setText (Integer.toString (transition.getFramesPerSecond()));
	}

//_________________________ INSTANCE VARIABLES
	private Button runEffectButton = new Button("Run Effect");
	
	private Button chooseEffectButton = new Button("Choose Effect");
	
	private Label timeLabel = new Label ("Effect Length (msecs):", Label.RIGHT);
	private TextField timeField = new TextField (8);

	private Label frameLabel = new Label ("Total Frames:", Label.RIGHT);
	private TextField frameField = new TextField (8);

	private Label fpsLabel = new Label ("Frames Per Second:", Label.RIGHT);
	private TextField fpsField = new TextField (8);
	
	private CheckboxGroup cbg = new CheckboxGroup ();
	private Checkbox timeButton = new Checkbox ("Time", cbg, true);
	private Checkbox frameButton = new Checkbox ("Frames", cbg, false);
	
	private QTTransition transition;
	
//_________________________ INSTANCE METHODS
	/**
	 * @return a Dimension object which defines the minimum size
	 */	 
	public Dimension getMinimumSize() { return new Dimension (0, 100); }

	/**
	 * @return a Dimension object which defines the preferred size
	 */	 
	public Dimension getPreferredSize() { return getMinimumSize(); }
}
