/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;

import quicktime.*;
import quicktime.app.display.*;
import quicktime.app.anim.*;
import quicktime.app.time.*;
import quicktime.std.movies.MovieController;
import quicktime.app.players.QTPlayer;


/**
 * A simple panel of AWT objects to demonstrate how AWT can be used to control a
 * QuickTime movie.
 */
public class ControlPanel extends Panel implements Errors {
//_________________________ CLASS METHODS
	public ControlPanel (Timer c, QTPlayer m, DirectGroup dg, Compositor sh) {
		this.comp = c;
		this.mov = m;
		this.dg = dg;
		shTimer = sh.getTimer();
		dgTimer = dg.getTimer();
		
		
		GridBagLayout gb = new GridBagLayout();
		setLayout (gb);

		GridBagConstraints cons = new GridBagConstraints();
		cons.fill = GridBagConstraints.HORIZONTAL;
		cons.insets = new Insets (1, 1, 1, 1);		
		cons.weightx = 1.0;
		
		cons.gridy = 0;
		cons.gridx = 1;
		add (rateLabel, cons);
		
		cons.gridx = 2;
		add (rateField, cons);
		
		cons.gridy = 1;
		cons.gridx = 1;
		add (compLabel, cons);
		
		cons.gridx = 2;
		add (compField, cons);
				
		cons.gridy = 2;
		cons.gridx = 0;
   		add (movCb, cons);
		
		//sprites
		cons.gridy = 2;
		cons.gridx = 2;
		add (spriteLabel, cons);
		
		cons.gridx = 3;
		add (spriteField, cons);
		
		//directgroup controller
		rateField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					dgTimer.setRate (new Float (str).floatValue());
				}
			});
			
		//compositor controller (ripples and spaceships)				
		compField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					comp.setRate (new Float (str).floatValue());
				}
			});
		
		//movie looping controller
		movCb.addItemListener(new ItemListener() {
  			public void itemStateChanged (ItemEvent ev) {
				if (((Checkbox)ev.getItemSelectable()).getState()) {
					try {
						mov.getMovieController().setLooping(true);
					}catch(QTException ex) {
						throw new QTRuntimeException (ex);
					}
				}				
				else {
					try {
						mov.getMovieController().setLooping(false);
					}catch(QTException ex) {
						throw new QTRuntimeException (ex);
					}
				}	
			}
		});	
				
		//spaceships rate controller
		spriteField.addActionListener (new ActionListener () {
				public void actionPerformed (ActionEvent event) {
					String str = new String (event.getActionCommand());
					shTimer.setRate (new Float (str).floatValue());
				}
		});
	}

	public void setDisplay () throws QTException {
		rateField.setText (Float.toString (dgTimer.getRate()));
		spriteField.setText (Float.toString (shTimer.getRate()));
		compField.setText (Float.toString (comp.getRate()));
	}
	
//_________________________ INSTANCE VARIABLES
	private Label rateLabel = new Label ("Direct Group Rate:", Label.RIGHT);
	private TextField rateField = new TextField (5);
	
	private Label compLabel = new Label ("Compositor Rate:", Label.RIGHT);
	private TextField compField = new TextField (5);
	
	private Checkbox movCb = new Checkbox("Loop Movie");
	
	private Label spriteLabel = new Label ("Sprites Rate:", Label.RIGHT);
	private TextField spriteField = new TextField (5);


	private Timer comp;
	private QTPlayer mov;
	private Timer dgTimer;
	private Timer shTimer;
	private DirectGroup dg;
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
