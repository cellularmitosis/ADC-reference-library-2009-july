/*

File: MovieTextFinder.java

Abstract: Performs a text search while playing a QuickTime movie

Version: 1.2

© Copyright 2006 Apple Computer, Inc. All rights reserved.

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

import java.awt.*;
import java.io.*;
import javax.swing.*;
import java.awt.event.*;
import java.applet.*;
import java.io.IOException;

import quicktime.*;
import quicktime.app.view.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.std.clocks.*;
import quicktime.std.movies.media.*;
import quicktime.util.*;


/**
 * MovieTextFinder class
 *
 * This class contains code to display and play a quicktime movie in a JFrame.
 * First a QTCanvas is created. The QTCanvas will be use for displaying our
 * QuickTime movie. Next, a movie file is opened and a MovieController
 * object created for the movie. The MovieController object is used to control/play
 * the movie. Finally, a QTDrawable object is created for the
 * MovieController, and the QTDrawable object is set as the drawing client for the
 * QTCanvas.
 */
public class MovieTextFinder extends JFrame implements Errors {	

	JTextField searchTextField;
	JCheckBox wrapSearch;
	JCheckBox caseSensitiveSearch;
	JRadioButton searchForwardButton;
	JRadioButton searchBackwardButton;

	Movie theMovie;
        MovieController theMovieController;
        QTComponent qtComponent = null;
		
	static final private String theTextMovieName = "TextOnly.mov";
	
		/* this flag will determine which technique we
		 use to perform our search */
	static final private boolean kUseMovieSearchTextMethod = true;
	
		// current offset into search text 
	private int currentOffset = 0;
	
	private JDialog errorDialog;
	
	public static void main (String args[]) {
		try {
			QTSession.open();
			// make a window and show it - we only have one window/one movie at a time
			MovieTextFinder pm = new MovieTextFinder("QT in Java");
			pm.show();
			pm.toFront();
		} catch (QTException e) {
			// at this point we close down QT if an exception is generated because it means
			// there was a problem with the initialization of QT>
			e.printStackTrace();
			QTSession.close ();
		}
	}

	/**
	 * MovieTextFinder constructor
	 *
	 * This class creates a QTCanvas which will be used to display QuickTime content. Next, a 
	 * movie file is opened. This movie contains a text track which we'll use to demonstrate
	 * how to search for individual text strings. Finally, we add the QTCanvas as a component
	 * to the JFrame container.
	 */
	public MovieTextFinder (String title) {
		super (title);

		new FileMenu (this);

		setResizable(false);

		JPanel p1 = new JPanel();
		
		p1.setLayout (new BoxLayout(p1, BoxLayout.Y_AXIS));
		p1.setBorder(BorderFactory.createEmptyBorder(20,20,20,20));

		searchForwardButton = new JRadioButton("Search Forward");
		searchForwardButton.setSelected(true);
		searchBackwardButton = new JRadioButton("Search Backward");
		ButtonGroup buttonGroup = new ButtonGroup();
		buttonGroup.add(searchForwardButton);
		buttonGroup.add(searchBackwardButton);
		
		JPanel radioPanel = new JPanel();
		
		radioPanel.setLayout(new BoxLayout(radioPanel,BoxLayout.Y_AXIS));
		radioPanel.add(searchForwardButton);
		radioPanel.add(searchBackwardButton);
		
		p1.add(radioPanel);

		wrapSearch = new JCheckBox("wrap search");
		caseSensitiveSearch = new JCheckBox("case sensitive");

		p1.add(wrapSearch);
		p1.add(caseSensitiveSearch);

		try {
				/* let's open and display our text movie which is
					located in our project folder */
			createNewMovieFromURL("file://" + QTFactory.findAbsolutePath (theTextMovieName));
		}
		catch (IOException ioe) {
			showErrorDialog(ioe.toString());
			System.exit(0);
		}

        Container C = getContentPane();
        C.setLayout (new FlowLayout(FlowLayout.LEFT));

		JPanel p3 = new JPanel();
        p3.setLayout (new GridLayout(5,1));
		p3.setBorder(BorderFactory.createEmptyBorder(20,20,20,20));
		
		JLabel jl = new JLabel("  Enter Search Text");
		p3.add(jl);

		searchTextField = new JTextField(10);
		searchTextField.setMaximumSize(new Dimension(250,20));
		searchTextField.select(0,0);
		p3.add(searchTextField);

		JButton searchButton = new JButton("       Search      ");
		searchButton.setDefaultCapable(true);

		p3.add(Box.createRigidArea(new Dimension(0,20)));
		p3.add(searchButton);
		
		// trap <return>, <enter> key presses so we can perform our search
		addKeyListener( new KeyAdapter() {
			public void keyReleased(KeyEvent e) {
					// <enter> key pressed? 
				if (e.getKeyCode() == e.VK_ENTER) {
					DoSearch();		// search for current text 
				}
			}
		} );

		searchButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				DoSearch();
			}
		} );

		Container container = getContentPane();
        container.setLayout (new FlowLayout(FlowLayout.LEFT));
		container.add(p1, "West");

		try {	
			JPanel p0 = new JPanel();
			p0.setLayout (new FlowLayout(FlowLayout.LEFT));
			p0.setBorder(BorderFactory.createEmptyBorder(20,20,20,20));
                        
			qtComponent = QTFactory.makeQTComponent(theMovieController);
			p0.add((Component)qtComponent);

			container.add(p0, "Center");		
		}
		catch (QTException e) {
			e.printStackTrace();
		}

		container.add(p3, "East");
		/* set focus on search text box, so user
			can type search text without having
			to first tab to it */
		searchTextField.requestFocus();

		pack();
		
		addWindowListener(new WindowAdapter () {

			public void windowClosing (WindowEvent e) {
				goAway();
			}
			
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});			
	}

	/*
	 * This internal method is called to perform a text search in the movie 
	 */
	private void DoSearch() {
		String theText = searchTextField.getText();
		try {
			
			/* set the search features */
			int searchFlags = quicktime.std.StdQTConstants.findTextUseOffset;

				/* backward search? */
			if (searchBackwardButton.isSelected())
				searchFlags |= quicktime.std.StdQTConstants.findTextReverseSearch;
				
				/* wrap search? */
			if (wrapSearch.isSelected())
				searchFlags |= quicktime.std.StdQTConstants.findTextWrapAround;
			
				/* case sensitive search? */
			if (caseSensitiveSearch.isSelected())
				searchFlags |= quicktime.std.StdQTConstants.findTextCaseSensitive;
	
			findTextInMovieTextTrack (theText, searchFlags);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}

	}
	
	/**
	 *
	 * createNewMovieFromURL method
	 *
	 * This class creates a movie and it's associated movie controller from
	 * a url. It then creates a QTPlayer object which is used to display/play 
	 * and control the movie. 
	 *
	 * @param theURL string containing the URL of the movie to open
	 */
	public void createNewMovieFromURL (String theURL) {
		try {
			// create the DataRef that contains the information about where the movie is
			DataRef urlMovie = new DataRef(theURL);
			
			// create the movie 
			theMovie = Movie.fromDataRef (urlMovie,StdQTConstants.newMovieActive);
			currentOffset = 0;
			theMovieController = new MovieController(theMovie);
                        if (qtComponent != null)
                            qtComponent.setMovieController(theMovieController);

			// this will set the size of the enclosing frame to the size of the incoming movie
			pack();

		} catch (QTException err) {
			err.printStackTrace();
		}
	}
		
	/**
	 * Gets the movie to be searched
	 * 
	 * @return the Movie
	 */
	public Movie getMovie () throws QTException {
		return theMovie;
	}
	
	/** 
	 * This method tears down the QTSession object and quits the application
	 */
	public static void goAway () {
		QTSession.close();
		System.exit(0);	
	}	

	/** 
	 * This method stops the movie (if applicable) from playing
	 */
	void stopPlayer () {
		try {
			if (theMovie != null)
				theMovie.setRate(0);
		} catch (QTException err) {
			err.printStackTrace();
		}
	}
	
	/** 
	 * FindTextInMovieTextTrack method
	 *
	 * This method shows how to use both the quicktime.std.movies.searchText method and the 
	 * quicktime.std.movies.media.TextMediaHandler.findNextText method to search for text in 
	 * a movie text track.
	 * @param searchText the string to search for
	 * @param theFlags search flags- forward, backward, etc.
	 * @return true if the text was found
	 */
	boolean findTextInMovieTextTrack (String searchText, int theFlags) {
			// was the text found? 
		boolean textFound = false;
		
			// we will search in enabled text tracks only 
		theFlags |= StdQTConstants.searchTextEnabledTracksOnly;
		
		if ( kUseMovieSearchTextMethod) {
			//////////
			//
			// METHOD ONE: Use the quicktime.std.movies.searchText method, your one-stop, 
			//				find-the-text-and-do-the-right-thing function.
			//
			//////////
			
			try {
					// get the current movie time - we will start searching
					//	from this point on
				QTPointer searchTextPtr = new QTPointer(searchText.getBytes());	
				SearchResult sResult = theMovie.searchText(searchTextPtr,		// the text to search for
															theFlags,				// flags 					
															theMovie.getTime(),	// current movie time 		
															currentOffset 			// searchOffset 			 
															);
				if (sResult.getFoundTrack() != null) {
					textFound = true;	// found it!!!! 
					currentOffset = sResult.getOffset();	// save off current offset into text 
				}
			}
			catch (QTException e) {
				int theError = e.errorCode();
				if (theError == quicktime.Errors.movieTextNotFoundErr) {
					showErrorDialog("Text not found!");
				}
				else {
					e.printStackTrace();
				}
			}
		} else {
			//////////
			//
			// METHOD TWO: Use quicktime.std.movies.media.TextMediaHandler.findNextText method. Here, 
			//				you need to explicitly go to the found text and select it. 
			//
			//////////

					// the text media handler for our text track 
				MediaHandler trackMediaHandler = null;

				try {					
						// get the first enabled text track from the movie. If there is
						//	more than one, the first one found will be used 
					Track theTextTrack = theMovie.getIndTrackType (1,
																StdQTConstants.textMediaType, 
																StdQTConstants.movieTrackMediaType | 
																StdQTConstants.movieTrackEnabledOnly);
						
					if (theTextTrack != null) {
						// load the entire text track into RAM 
						theTextTrack.loadIntoRam (0, theTextTrack.getDuration(), 0);

						// get the text media handler for the track - we'll use
						//	this media handler to demonstrate how to perform a
						//	text search 
						Media trackMedia 	= Media.getTrackMedia (theTextTrack);
						trackMediaHandler 	= trackMedia.getHandler();
					}
					else {
						showErrorDialog ("Movie does not contain a text track!");
						return false;
					}
			
					// take the generic media handler gotten from our text
					//	track and cast it to a TextMediaHandler, since we know
					//	we are dealing with text media 
					TextMediaHandler textMediaHdlr = (TextMediaHandler)trackMediaHandler;
				
						// we'll need to make a QTPointer object from our search text 
						//	and pass this to the findNextText method below 
					QTPointer searchTextPtr = new QTPointer (searchText.getBytes());	
					
						// search for the specified text
					FoundTextInfo fTextInfo = textMediaHdlr.findNextText (searchTextPtr,			// the text to search for 
																		theFlags,				// search flags 
																		theMovie.getTime()	// time to begin searching from 
																		);	
					if (fTextInfo.time != -1) {
						textFound = true;	// found it!!!! 

							// save current offset into search text 
						currentOffset = fTextInfo.offset;
						
							// go to the time in the movie where the
							//	search text was found 
						theMovie.setTimeValue(fTextInfo.time);
	
							// highlight the text 
						textMediaHdlr.hiliteTextSample(theMovie.getTime(),
														fTextInfo.offset,
														fTextInfo.offset+searchText.length(),
														new QDColor(0x8000,0x8000,0x8000)	// use grey color hilight 
														);
					}
					else 
						showErrorDialog("Text not found!");
				}
				catch (QTException e) {
					int theError = e.errorCode();
					if (theError == quicktime.Errors.movieTextNotFoundErr) 
						showErrorDialog("Text not found!");
					else 
						e.printStackTrace();
				}
			}
		
			// tell the caller whether or not text was found 
		return textFound;
	}
	
	/**
	 * Displays an error dialog reporting any problems encountered
	 * 
	 * @param theError the error string
 	 */
	private void showErrorDialog(String theError) {
		errorDialog = new JDialog();
		errorDialog.setModal(true);
		errorDialog.setResizable(false);
		Container c = errorDialog.getContentPane();
		c.setLayout(new BoxLayout(c,BoxLayout.Y_AXIS));
		
		JPanel jp = new JPanel();
		jp.setLayout(new BoxLayout(jp,BoxLayout.Y_AXIS));
		jp.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));				

		JLabel jl = new JLabel(theError);
		jl.setMaximumSize(new Dimension(270,20));
		
		jp.add(jl);
		
		JPanel jp2 = new JPanel();
		jp2.setLayout(new BoxLayout(jp2,BoxLayout.Y_AXIS));
		jp2.setBorder(BorderFactory.createEmptyBorder(5,125,15,10));

		JButton jb = new JButton("OK");
		jb.setMaximumSize(new Dimension(70,20));

		jp2.add(jb);
		
		errorDialog.getContentPane().add(jp, "South");
		errorDialog.getContentPane().add(jp2, "South");
		jb.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				errorDialog.dispose();
			}
		});
		errorDialog.setBounds(0,0,300,100);
		errorDialog.show();
	}
}