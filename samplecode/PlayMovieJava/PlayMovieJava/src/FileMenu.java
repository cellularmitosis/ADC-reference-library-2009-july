/*
	File:		FileMenu.java
	
	Description:	UI for access to functions in the PlayMovie sample.
        
	Author:		Apple Computer, Inc.

	Copyright: 	� Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
import java.applet.*;
import java.io.IOException;


import quicktime.*;
import quicktime.io.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.app.players.*;
import quicktime.app.display.*;


public class FileMenu {
	PlayMovie myPlayMovie;
		
	public FileMenu (PlayMovie src) {
		this.myPlayMovie = src;
		
		// make the menu bar up
		MenuBar menuBar = new MenuBar();
		Menu fileMenu = new Menu ("File");
		
		MenuItem openMenuItem = new MenuItem ("Open...");
		MenuItem openURLMenuItem = new MenuItem("Open URL...");
		MenuItem presentMovieMenuItem = new MenuItem ("Present Movie");
		MenuItem quitMenuItem = new MenuItem("Quit");
		
		fileMenu.add(openMenuItem);
		fileMenu.add(openURLMenuItem);
		fileMenu.addSeparator();
		fileMenu.add(presentMovieMenuItem);
		fileMenu.addSeparator();
		fileMenu.add(quitMenuItem);
			
		menuBar.add (fileMenu);
		myPlayMovie.setMenuBar (menuBar);
					
		openURLMenuItem.addActionListener (new ActionListener () {
			public void actionPerformed(ActionEvent event) {
				myPlayMovie.stopPlayer();
				
				// this will set the new movie in its action method if OK is chosen
				Open_URL d = new Open_URL (myPlayMovie);
				d.pack();
				d.show();
		 	}
		});
		openMenuItem.addActionListener (new ActionListener () {
			public void actionPerformed(ActionEvent event) {
				myPlayMovie.stopPlayer();
					
					// creat a movie through the file-open dialog of QT
				try {
					QTFile qtf = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
					myPlayMovie.createNewMovieFromURL ("file://" + qtf.getPath());
				} catch (QTException e) {
					if (e.errorCode() != Errors.userCanceledErr)
		 				e.printStackTrace();
		 		}
		 	}
		});
		quitMenuItem.addActionListener (new ActionListener () {
			public void actionPerformed(ActionEvent event) {
					// closes down QT and quits
				PlayMovie.goAway();
		 	}
		});
		presentMovieMenuItem.addActionListener (new ActionListener () {
			public void actionPerformed(ActionEvent event) {
				try {					
					if (myPlayMovie.getPlayer() == null) return;
					
					// present in full screen mode
					// use the current screen resolution and current movie
					FullScreenWindow w = new FullScreenWindow(new FullScreen(), myPlayMovie);
					MoviePlayer mp = new MoviePlayer (myPlayMovie.getMovie());
					QTCanvas c = new QTCanvas (QTCanvas.kPerformanceResize, 0.5F, 0.5F);
					w.add (c);
					w.setBackground (Color.black);
					
						//remove the movie from its current QTCanvas 
					myPlayMovie.getCanvas().removeClient();
						//put it into the new canvas of the FullScreenWindow
						//we restore this in the HideFSWindow's action
					c.setClient (mp, false);
					
						// show FSWindow and setup hide - it is invoke through a mousePressed action
						// so we set this as a listener for both the window and the canvas
					w.show();
					HideFSWindow hw = new HideFSWindow (w, myPlayMovie, c);
					w.addMouseListener (hw);							
					c.addMouseListener (hw);							
					
						// start the movie playing
					mp.setRate (1);
				} catch (QTException err) {
					err.printStackTrace();
				}
			}
		});			
	}
	
	static class HideFSWindow extends MouseAdapter {
		HideFSWindow (FullScreenWindow w, PlayMovie pm, QTCanvas c) {
			this.w = w;
			this.pm = pm;
			this.c = c;
		}
		
		private FullScreenWindow w;
		private PlayMovie pm;
		private QTCanvas c;
		
		public void mousePressed (MouseEvent me) {
			try {
					// this will stop the movie and reset it back to the previous window
				c.removeClient();
				pm.getCanvas().setClient (pm.getPlayer(), false);
			} catch (QTException e) {
				e.printStackTrace();
			} finally {
				w.hide();
			}
		}
	}
}			
