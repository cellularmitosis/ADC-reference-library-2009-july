/*
	File:		ControlPanel.java
	
	Description:	This demo program shows how to composit a presentation out of disparate media sources, 
                        applying compositing effects such as blend and transparency. Recording a movie from 
                        the activities of the Compositor is also shown.

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

import quicktime.*;
import quicktime.app.*;
import quicktime.app.display.*;
import quicktime.app.anim.*;
import quicktime.io.*;
import quicktime.std.movies.*;
import quicktime.std.*;
import quicktime.std.image.*;
/**
 * A simple panel of AWT objects to demonstrate how AWT can be used to control a
 * QuickTime movie.
 */
public class ControlPanel extends Panel implements Errors, StdQTConstants {
//_________________________ CLASS METHODS
	public ControlPanel (Frame fr, Compositor c, JavaTextDrawer t) throws QTException 
	{
		comp = c;
		jText = t;
		mainWindow = fr;
				
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
		add (scaleLabel, cons);
		
		cons.gridx = 6;
		add (scaleField, cons);
		
		cons.gridy = 1;
		cons.gridx = 0;	
		add (preflightCheck, cons);

		cons.gridx = 2;	
		add (recordMovieButton, cons);

		cons.gridx = 4;
		add (recordLabel, cons);
		
		cons.gridx = 6;
		add (recordField, cons);

		cons.gridy = 2;
		cons.gridx = 3;	
		add (changeColorButton, cons);
	
		//set recordmovie
		recMovie = new MovieRecording();
		recMovie.setCodec();

		changeColorButton.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					jText.doWork();
				}
			});


		rateField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					comp.getTimer().setRate (new Float (str).floatValue());
				}
			});
		scaleField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					try {
						comp.getTimer().rescheduleTickle(new Integer (str).intValue(), 1);
					} catch (QTException e) {
						e.printStackTrace();
					}
				}
			});	
		recordField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					numRecordFrames = new Integer (str).intValue();
				}
			});	
			
		recordMovieButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				try {
					FileDialog fd = new FileDialog (mainWindow, "Save Movie As...", FileDialog.SAVE);
					fd.show();
					if(fd.getFile() == null)
						throw new QTIOException (userCanceledErr, "");
					QTFile f = new QTFile(fd.getDirectory() + fd.getFile());
					Movie theMovie = Movie.createMovieFile (f,
										kMoviePlayer, 
										createMovieFileDeleteCurFile | createMovieFileDontCreateResFile);
					
					preflightCheck.setState(false); // recording disables preflight
				
					//Setup the record movie class
					recMovie.setMovie(theMovie, new CleanupMovie (f));
					recMovie.recordMode (numRecordFrames);
					comp.setRecordMovie (recMovie);
					
					System.out.println ("Start Recording");
					
				} catch (QTException e) {
					if (e.errorCode() != userCanceledErr)
						e.printStackTrace();
				}
			}
		});
		
		
		//for preflighting
		preflightCheck.addItemListener (new ItemListener () {
    			public void itemStateChanged (ItemEvent ev) {
					try {
						if (((Checkbox)ev.getItemSelectable()).getState()) {
							recMovie.setPreflighting(true);
							comp.setRecordMovie (recMovie);
						}				
						else {
							recMovie.setPreflighting(false);
						}	
					} catch (QTException e) {
						e.printStackTrace();
					}
				}
			});	
			
	}

		
	private class CleanupMovie implements RecordMovieCallback {
		CleanupMovie (QTFile f) {
			this.f = f;
		}
		
		private QTFile f;
		
		public void finish (Movie m) {
			try {
				OpenMovieFile outStream = OpenMovieFile.asWrite (f); 
				m.addResource (outStream, movieInDataForkResID, f.getName());
				outStream.close();
			} catch (QTException e) {
				e.printStackTrace();
			}
			System.out.println ("Finished Recording");
		}
	}
	
	private class MovieRecording extends RecordMovie {
		// we record at 2 frames a second
		int framesPerSecond = 10;

		MovieRecording() throws QTException {
			super();
		}
		
		public void setCodec() throws QTException{
			setCompressionSettings (framesPerSecond, 
					codecNormalQuality, 
					codecNormalQuality, 
					0, 
					kAnimationCodecType, 
					CodecComponent.bestSpeedCodec);	
					
		}			
			
	}
	
	
	public void setDisplay () throws QTException {
		rateField.setText (Float.toString (comp.getTimer().getRate()));
		scaleField.setText (Integer.toString (comp.getTimer().getScale()));
		recordField.setText (Integer.toString (numRecordFrames));
	}
	
//_________________________ INSTANCE VARIABLES
	private Label rateLabel = new Label ("Playback Rate:", Label.RIGHT);
	private TextField rateField = new TextField (8);

	private Button changeColorButton = new Button ("Text Colour");

	private Label scaleLabel = new Label ("Scale (fps):", Label.RIGHT);
	private TextField scaleField = new TextField (8);
		
	private Frame mainWindow;
	private Compositor comp;
	private JavaTextDrawer jText;
	private Button recordMovieButton = new Button("Record Movie");
	private Label recordLabel = new Label ("Record Frames:", Label.RIGHT);
	private TextField recordField = new TextField (8);
	private Checkbox preflightCheck = new Checkbox ("Preflight", false);
	
	private boolean selected = false;
	private MovieRecording recMovie;
	private int numRecordFrames = 10;
	int keyFrameRate = 0;
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
