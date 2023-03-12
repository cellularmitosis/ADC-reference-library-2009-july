/*
	File:		ControlPanel.java
	
	Description:	UI controls for altering the visible ScrollingText object.

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

import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.*;
import quicktime.app.time.*;
import quicktime.io.*;
import quicktime.std.movies.*;
import quicktime.std.*;
import quicktime.std.image.*;
import quicktime.app.*;
import quicktime.app.players.*;
import quicktime.app.display.*;
/**
 * A simple panel of AWT objects to demonstrate how AWT can be used to control a
 * QuickTime movie.
 */
public class ControlPanel extends Panel implements Errors, StdQTConstants {
private String strx;
//_________________________ CLASS METHODS
	public ControlPanel (Timer ti, Frame m) {
		this.t = ti;
		this.f = m;
		
		GridBagLayout gb = new GridBagLayout();
		setLayout (gb);
		setFont (new Font("Helvetica", Font.BOLD, 10));

		GridBagConstraints cons = new GridBagConstraints();
		cons.fill = GridBagConstraints.HORIZONTAL;
		cons.gridheight = 1;		
		cons.insets = new Insets (2, 2, 2, 2);		
		cons.gridwidth = 2;
		cons.gridy = 0;
		cons.gridx = 0;
		add (rateLabel, cons);
		
		cons.gridx = 2;
		add (rateField, cons);

		cons.gridx = 4;
		add (makeMovie, cons);
		
		cons.gridy = 1;
		cons.gridx = 0;
		add (scaleLabel, cons);
		
		cons.gridx = 2;
		add (scaleField, cons);

		cons.gridx = 4;
		add (periodLabel, cons);
		
		cons.gridx = 6;
		add (periodField, cons);
		
		
		
		rateField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					strx = new String (event.getActionCommand());
					t.setRate (new Float (strx).floatValue());
					
				}
			});
		scaleField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					try {
						t.rescheduleTickle (new Integer (str).intValue(), t.getPeriod());
					} catch (QTException e) {
						e.printStackTrace();
					}
				}
			});	
		periodField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					try {
						t.rescheduleTickle (t.getScale(), new Integer (str).intValue());
					} catch (QTException e) {
						e.printStackTrace();
					}
				}
			});	
		makeMovie.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {						
					try {
						QTCanvas movCanv = new QTCanvas(QTCanvas.kInitialSize, 0.5f, 0.5f);
						f.add ("West", movCanv);
						File fl = QTFactory.findAbsolutePath ("jumps.mov");
						OpenMovieFile movieFile = OpenMovieFile.asRead(new QTFile(fl));
						Movie mov = Movie.fromFile (movieFile);						
						t.getTimeBase().setMasterTimeBase(mov.getTimeBase(), null);
						MovieController mc = new MovieController (mov);
						mc.setLooping (true);
						p = new QTPlayer (mc);
						movCanv.setClient (p, true);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			});	
	}

	QTPlayer p;
	Frame f;
	
	public void setDisplay () throws QTException {
		rateField.setText (Float.toString (t.getRate()));
		scaleField.setText (Integer.toString (t.getScale()));
		periodField.setText (Integer.toString (t.getPeriod()));
	}
	
//_________________________ INSTANCE VARIABLES
	private Label rateLabel = new Label ("Playback Rate:", Label.RIGHT);
	private TextField rateField = new TextField (8);

	private Label scaleLabel = new Label ("Scale:", Label.RIGHT);
	private TextField scaleField = new TextField (8);

	private Label periodLabel = new Label ("Period:", Label.RIGHT);
	private TextField periodField = new TextField (8);
	
	private Button makeMovie = new Button ("Make Movie");
	Timer t;

//_________________________ INSTANCE METHODS
	/**
	 * @return a Dimension object which defines the minimum size
	 */	 
	public Dimension getMinimumSize() { return new Dimension (0, 80); }

	/**
	 * @return a Dimension object which defines the preferred size
	 */	 
	public Dimension getPreferredSize() { return getMinimumSize(); }
}
