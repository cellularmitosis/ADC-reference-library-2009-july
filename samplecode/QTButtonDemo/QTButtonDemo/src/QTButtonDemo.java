/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.*;

import quicktime.app.*;
import quicktime.app.actions.*;
import quicktime.app.anim.*;
import quicktime.app.display.*;
import quicktime.app.event.*;
import quicktime.app.image.*;
import quicktime.app.players.*;
import quicktime.app.ui.*;

import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.image.*;
import quicktime.std.movies.*;
import quicktime.util.*;


public class QTButtonDemo extends Frame {

	public static void main(String args[]) {
		try {
			QTSession.open();
		
			Frame window = new QTButtonDemo();
			window.addWindowListener(new WindowAdapter() {
				public void windowClosing (WindowEvent e) {
					QTSession.close();
					((Frame)e.getSource()).dispose();
				}

				public void windowClosed (WindowEvent e) {
					System.exit(0);
				}
			});
			window.pack();
			window.show();
		} catch (QTException e) {
			if (e.errorCode() == Errors.userCanceledErr) {
				QTSession.close();
				System.exit(0);
			}
			e.printStackTrace();
			QTSession.close();
		}
	}	
	
	QTButtonDemo() throws QTException {
		super("QTButtonDemo");
			
// get movie
		QTFile movieFile = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
		OpenMovieFile openedFile = OpenMovieFile.asRead (movieFile);
		Movie mov = Movie.fromFile (openedFile);
		mov.getTimeBase().setFlags (StdQTConstants.loopTimeBase);
		final MoviePresenter moviePresenter = new MoviePresenter (mov);

		QDRect size = moviePresenter.getDisplayBounds();
		
// make buttons
		// create the images using the QDGraphics, you can use the above calls to use actual image files
		QDRect r20 = new QDRect (20, 20);
		
		ImagePresenter redImage = makeImagePresenter (r20, QDColor.red);
		ImagePresenter greenImage = makeImagePresenter (r20, QDColor.green);
		ImagePresenter blueImage = makeImagePresenter (r20, QDColor.blue);
		ImagePresenter cyanImage = makeImagePresenter (r20, QDColor.cyan);
		
// create a release button which fires on mouse release
// this also has a rollover image - an image that changes when the user rolls over this button
		ReleaseButton relButton = new  ReleaseButton (redImage, greenImage, blueImage, cyanImage);
		relButton.setLabel ("Release Button");
		relButton.setLocation (0, size.getHeight());
// set an action to set the rate of the movie
		relButton.addActionListener(new QTActionListener() {
			public void actionPerformed(QTActionEvent e) {
				try {
					if (moviePresenter.getRate() == 0) {	
						moviePresenter.setRate (1);
					} else
						moviePresenter.setRate (0);
				}catch (QTException ex) {
					ex.printStackTrace();
				}	
			}
		});
		//creates a press action button that fires when the mouse is on the button and the mouse button is pressed.
		PressActionButton pressButton = new PressActionButton (blueImage,redImage, greenImage);
		pressButton.setLabel ("Press Button");
		pressButton.setLocation (size.getWidth() - 20, size.getHeight());
		
//action will change the time of the movie and print out the action event
		pressButton.addActionListener (new QTActionListener() {
			public void actionPerformed (QTActionEvent e) {
				try {
					moviePresenter.setTime (moviePresenter.getTime() - 10);
				} catch( QTException ex) {
					ex.printStackTrace();
				}
			}
		});
					
// create a Compositor to hold the movie and the two buttons	
		Compositor comp = new Compositor (new QDGraphics (new QDRect(size.getWidth(), size.getHeight() + 20)), QDColor.yellow, 10, 1);
		comp.getTimer().setRate(1);
		
//add the presenters and the buttons to the compositor
		comp.addMember(moviePresenter);
		comp.addMember(relButton);
		comp.addMember(pressButton);
		
		QTMouseTargetController buttonController = new QTMouseTargetController (false);
		comp.addController (buttonController);
		buttonController.addMember(pressButton);
		buttonController.addMember(relButton);
		
		buttonController.addQTMouseListener (new ButtonActivator () );
					
// create the canvas and display it in the Frame
		QTCanvas myQTCanvas = new QTCanvas (QTCanvas.kPerformanceResize, 0.5f, 0.5f);
		myQTCanvas.setClient (comp, true);
		add("Center", myQTCanvas);
	}

	//create a raw-encoded image and its description from the QDGraphics and create a ImagePresenter to be used
	//as the image for the buttons
	private static ImagePresenter makeImagePresenter (QDRect size, QDColor col) throws QTException {
		QDGraphics g = new QDGraphics (QDConstants.k4IndexedPixelFormat, size);
		g.setBackColor (col);
		g.eraseRect (null);
		PixMap pm = g.getPixMap();
		EncodedImage ei = pm.getPixelData();
		ImageDescription id = new ImageDescription (pm);
		return ImagePresenter.fromQTImage (ei, id);
	}
}
