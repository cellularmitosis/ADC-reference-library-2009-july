/*

File: ImportExport.java

Abstract: Programmatically exports a movie with user-customized settings.

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
import java.awt.event.*;
import java.applet.*;
import java.io.IOException;

import quicktime.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.clocks.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.std.qtcomponents.*;
import quicktime.util.*;

import quicktime.app.view.*;

public class ImportExport extends Frame implements StdQTConstants, IOConstants,  Errors {	
	public static void main (String args[]) {
		try { 
			QTSession.open();
			ImportExport ie = new ImportExport("Import Export");
			ie.pack();
			ie.show();
			ie.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	ImportExport (String title)	throws Exception {
		super (title);

		Button importBtn = new Button ("Import Media");
		importBtn.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent ae) {
				importMedia();
			}
		});
		
		add (importBtn, "North");
		Button refMovieButton = new Button ("Reference Movie");
		refMovieButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent ae) {
				makeReferenceMovie();
			}
		});
		add (refMovieButton, "Center");

		Button exportButton = new Button ("Export Movie");
		exportButton.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent ae) {
				exportMovie();
			}
		});
		add (exportButton, "South");

		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});	   
	}

	
	QTComponent component;
	void importMedia () {
        QTFile importFile = null;
		try {
			FileDialog importFileDialog = new FileDialog (this, "Choose Movie to Import...", FileDialog.LOAD);
			importFileDialog.show();
			if (importFileDialog.getFile() == null)
				return;
            importFile = new QTFile (importFileDialog.getDirectory() + importFileDialog.getFile());
			
				// Import ANY media into QuickTime
				// Media files are first opened by QuickTime using OpenMovieFile.asRead.  A new Movie is created 
				// using Movie.fromFile which imports the media from the file. A MovieController
				// is then created using the Movie, and a new QTComponent is created using QTFactory.makeQTComponent.
				// The returned QTComponent can be added to a Java AWT frame.
                if (component != null) {
                    component.setMovieController(new MovieController(Movie.fromFile (OpenMovieFile.asRead (importFile))));
                }else {
                    displayMovie(Movie.fromFile (OpenMovieFile.asRead (importFile)));
                }
				//resizes the frame to the size of the new QTCanvas and buttons.
				// the QTCanvas' size will be the size of the media
			pack();
		} catch (QTException err) {
			if (err.errorCode() == userCanceledErr) return;
			err.printStackTrace();
		}
	}
	
	void displayMovie (Movie m) throws QTException {
        component = QTFactory.makeQTComponent (new MovieController (m));
        add ((Component)component, "East");
		pack();
        show();
	}
			
	void makeReferenceMovie () {
		try{
			FileDialog openFileDialog = new FileDialog (this, "Choose Movie to Reference...", FileDialog.LOAD);
			openFileDialog.show();
			if (openFileDialog.getFile() == null)
				return;
			QTFile movieFile = new QTFile (openFileDialog.getDirectory() + openFileDialog.getFile());

			FileDialog saveFileDialog = new FileDialog (this, "New Movie to create...", FileDialog.SAVE);
			saveFileDialog.show();
			if (saveFileDialog.getFile() == null) {
				return;
			}
			
			makeReferenceMovie (movieFile, saveFileDialog.getDirectory() + saveFileDialog.getFile());			
		} catch (QTException err) {
			if (err.errorCode() == userCanceledErr) return;
			err.printStackTrace();
		}
	}
		
		//makes a new movie that reference the data in an existing movie
	void makeReferenceMovie (QTFile movieFile, String outputPath) throws QTException {		
		// Create the movie object from the original movie
		Movie theMovie = Movie.fromFile (OpenMovieFile.asRead (movieFile));
		
        if (component != null) {
            component.setMovieController(new MovieController(theMovie));
        } else {
            displayMovie (theMovie);
        }
		
		QTFile outputMovie = new QTFile (outputPath);
		
		boolean creatingReferenceMovie = true;
			// this leaves data in existing movieFile
		if (creatingReferenceMovie) {
				//shortcut movies are movies that just contain a reference
				//to another movie. They are only available in QT 4 or above
			boolean createShortCutMovie = false;
			if (createShortCutMovie && QTSession.getQTMajorVersion() > 3) {
					//make a Data ref out of a URL that references the movie
				DataRef targetDataRef = new DataRef ("file://" + movieFile.getPath());
					//make the very small short cut movie
				outputMovie.createShortcutMovieFile (kMoviePlayer, smSystemScript, createMovieFileDeleteCurFile, targetDataRef);
			} else {
					// this create's a "medium" cut movie - same principle as the shortcut movie
					// it is a movie that gets its media data from another movie
				// create the new movie file, deleting existing file if there
				outputMovie.createMovieFile (kMoviePlayer, createMovieFileDeleteCurFile);
				
				// add a reference to all of the media in the existing media file
				// The outputMovie file now contains a reference and dependency on the existing media file
				OpenMovieFile outStream = OpenMovieFile.asWrite (outputMovie);
				theMovie.addResource (outStream, movieInDataForkResID, outputMovie.getName());
				outStream.close();
			}
		} else {
			// this will copy data to the new file
			
		//if the application wanted to remove this dependency it could flatten the existing movie
		// to the new movie file instead of the addResource calls that were made above
		// the createMovieFile call would not be required in this case
			int movieFlattenFlags = 0; //see docs to specify options here
			theMovie.flatten (movieFlattenFlags, 
									outputMovie, 
									kMoviePlayer,
									smSystemScript, 
									createMovieFileDeleteCurFile, 
									movieInDataForkResID, 
									outputMovie.getName());
		}
	}	

	void exportMovie () {
		try{
			FileDialog efd = new FileDialog (this, "Choose Movie to Export...", FileDialog.LOAD);
			efd.show();
			if (efd.getFile() == null)
				return;
			QTFile movieFile = new QTFile (efd.getDirectory() + efd.getFile());
			
			exportMovie (movieFile);
		
		} catch (QTException err) {
			err.printStackTrace();
		}
	}
	
		//export (to a movie) the incoming movie
		//user dialog allows user to customise media formats and tracks that are exported
	void exportMovie (QTFile movieFile) throws QTException {		
		// Create the movie object from the original movie
		Movie theMovie = Movie.fromFile (OpenMovieFile.asRead (movieFile));
		
        if (component != null) {
            component.setMovieController(new MovieController(theMovie));
        } else {
            displayMovie (theMovie);
        }
		
			//we do this in a different thread because exporting can take some time
			// and the event thread should not be blocked for so long
		new Thread (new Runner (theMovie)).start();
	}
	
	static class Runner implements Runnable {
		Runner (Movie mov) {
			theInputMovie = mov;
		}
		
		Movie theInputMovie;
		
		public void run () {
			try {
					//this determines both the exporter type, the resulting file type.
					// thus one could specify this to be AIFF and the resulting file will be an AIFF file
					// - in this case the result will be a movie.
				int exportType = kQTFileTypeMovie;
				
					//an application can alternatively configure exporter through setting
					// up the exporter in code to conform to the format appropriate
				
				boolean useMovieConvertFile = true;		
				if (useMovieConvertFile) {
						//set the progress proc of the movie so that it gives the user feedback
					theInputMovie.setProgressProc();
						//do the export of the movie - all tracks as specified by user in dialog
						// with the showUser... flag QT itself will display a dialog to the user
						// to save the exported file. The Name of the file in that dialog is the name we specify
						// in this case "Export Movie"
					theInputMovie.convertToFile (new QTFile ("Export Movie"), exportType, kMoviePlayer, IOConstants.smSystemScript, showUserSettingsDialog);
				} else {
					FileDialog exportFileDialog = new FileDialog (new Frame(), "Export Movie to...", FileDialog.SAVE);
					exportFileDialog.show();
					if (exportFileDialog.getFile() == null)
						return;
					QTFile outFile = new QTFile (exportFileDialog.getDirectory() + exportFileDialog.getFile());
					
					// Create a movie exporter so we can customise its settings
					// this could also be used in the convertToFile version, but
					// if we don't have custom settings then we allow the convertToFile
					// to create the exporter for us - based on the exportType we pass to it.
					MovieExporter theMovieExp = new MovieExporter (exportType);

					// Set export settings from the user.
					theMovieExp.doUserDialog (theInputMovie, null, 0, theInputMovie.getDuration());
					
					//this returns a dupFNErr on windows and is also more work for the application
						// create the output file but don't open it
					outFile.createMovieFile (kMoviePlayer, createMovieFileDeleteCurFile);
						//set a progress proc - we just print out to System.out
					theMovieExp.setProgressProc (new MovieProgress () {
					 	public int execute (Movie mov, int message, int whatOperation, float percentDone) {
							System.out.println (mov + ",message=" + Integer.toHexString(message) + "," + ",what=" + Integer.toHexString(whatOperation) + ",%=" + percentDone);
							return 0;
						}
					});
						// do the export of the movie
					theMovieExp.toFile (outFile, theInputMovie, null, 0, theInputMovie.getDuration());
				}
			} catch (QTException e) {
				e.printStackTrace();
			}
		}
	}
}