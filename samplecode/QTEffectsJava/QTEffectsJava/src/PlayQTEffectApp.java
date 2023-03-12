/*
	File:		PlayQTEffectApp.java
	
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

import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.std.image.*;

import quicktime.app.*;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.players.*;

import quicktime.util.*;

import qteffect.*;

public class PlayQTEffectApp extends Frame implements QDConstants, StdQTConstants, Errors {
	
    public static void main (String args[]) {
		try {
        	PlayQTEffectApp pe = new PlayQTEffectApp(
        		PlayQTEffectApp.isFilterExampleFlag ? "QT Filter Effects" : "QT Transition Effects");
        	pe.show();
        	pe.toFront();
       	} catch (Exception e) {
       		e.printStackTrace();
       	}
    }

	/**
	 *	-- flags control the example modes
	 *		demonstrates effects filter on an image
	 *		demonstrates effects filter on a movie
	 *		demonstrates effects transition between two images
	 *		demonstrates effects transition between two movies
	 *		demonstrates effects transition between a movie and an image
	 *		demonstrates effects transition between an image and a movie
	 */
	 
	// demonstrates a transition between two sources (true) or filtering of a single source (false)
	public	static final  boolean 	 isFilterExampleFlag = true;

		// -- options for isFilterExampleFlag is true
		// selects filtering of a movie (true) or an image (false)
	private	static final  boolean 	 isFilterSourceMovie = false;

		// -- options for isFilterExampleFlag is false
		// selects use of a movie (true) or an image (false) as the source of the transition
	private	static final  boolean 	 isTransitionSourceMovie = false;
		// selects use of a movie (true) or an image (false) as the destination of the transition
	private	static final  boolean 	 isTransitionDestMovie = false; 

	// -- default media from QTJavaDemos/media
	private	static final String 	defaultImageSrcA = "pics/house.jpg";
	private	static final String 	defaultImageSrcB = "pics/icons.jpg";
	private	static final String 	defaultMovieSrcA = "jumps.mov";
	private	static final String 	defaultMovieSrcB = "Ships.mov";

		// substitute the names of your images and movies here with valid partial path from ../media/
	private	static final String 	myImageSrcA = defaultImageSrcA;
	private	static final String 	myImageSrcB = defaultImageSrcB;
	private	static final String 	myMovieSrcA = defaultMovieSrcA;
	private	static final String 	myMovieSrcB = defaultMovieSrcB;
	
	// -- QTJava 5 ParameterDialog.showParameterDialog() or QTJava 4 ParameterDialog.showParameterDialog()
	public static final boolean 	useEnhancedParameterDialog = true;
		// options for useEnhancedParameterDialog is true
	public static final boolean 	useModalParameterDialog = false;
	public static final boolean 	usePreviewPictures = true;
	static QTCanvas qtCanvas;

    PlayQTEffectApp (String name) throws Exception {
        super (name);
        addNotify();
        Insets ins = getInsets();
        setBounds (0, 0, (400 + ins.left + ins.right), (400 + ins.top + ins.bottom));
        QTSession.open();

        qtCanvas = new QTCanvas(QTCanvas.kInitialSize, 0.5f, 0.5f);
        add("Center", qtCanvas); 

// Sources/Dests for Transitions or Filters can be movies, still images, compositors

		if (isFilterExampleFlag) {
//	Example Code for creating a single source filter
			QTFilter filter = new QTFilter(quicktime.app.image.Redrawable.kMultiFrame);
	   		/// setup effect (emboss)
	        filter.setEffect(createFilterEffect(kEmbossImageFilterType, 11));
	        	// apply filter to movie
		   if (isFilterSourceMovie) {
		        File fileMovie = QTFactory.findAbsolutePath (myMovieSrcA);
		        QTFile qtfMovie = new QTFile (fileMovie.getAbsolutePath());
		        OpenMovieFile openedFile = OpenMovieFile.asRead(qtfMovie);
		        Movie mov = Movie.fromFile(openedFile);
		      	mov.getTimeBase().setFlags( StdQTConstants.loopTimeBase);	
		        MoviePresenter player = new MoviePresenter(mov);
		        player.setRate(1);
		        filter.setSourceImage(player);
			} else {
		       	File fileImage = QTFactory.findAbsolutePath (myImageSrcA);
		        QTFile qtfImage = new QTFile (fileImage.getAbsolutePath());
		        Compositable image = new GraphicsImporterDrawer (qtfImage);
		        filter.setSourceImage(image);
			}

			// have to set source BEFORE setting filter as the client of the canvas
			// unless you supply a QDDimension to the filter constructor

	        FilterPanel fp = new FilterPanel (filter, qtCanvas);
	        add ("North", fp); 
			qtCanvas.setClient (filter, true);
			
		} else {
		
//	Example Code for creating a transition
	        QTTransition transition = new QTTransition ();
	        transition.setEffect (createSMPTEEffect (kEffectWipe, kRandomWipeTransitionType)); 
		
			if (isTransitionSourceMovie) {
				 // substitute the name of your movie here with valid partial path from ../media/
		        File fileSrcMovie = QTFactory.findAbsolutePath (myMovieSrcA);
		        QTFile qtfSrcMovie = new QTFile (fileSrcMovie.getAbsolutePath());
		        OpenMovieFile openedSrcMovie = OpenMovieFile.asRead(qtfSrcMovie);
		        Movie movSrc = Movie.fromFile(openedSrcMovie);
		      	movSrc.getTimeBase().setFlags( StdQTConstants.loopTimeBase);	
		        MoviePresenter playerSrc = new MoviePresenter(movSrc);
		        playerSrc.setRate(1);
		        transition.setSourceImage (playerSrc);
			} else {	// transition source is Image
		       	File fileSrcImage = QTFactory.findAbsolutePath (myImageSrcA);
		        QTFile qtfSrcImage = new QTFile (fileSrcImage.getAbsolutePath());
		        Compositable imageSrc = new GraphicsImporterDrawer (qtfSrcImage);
				transition.setSourceImage (imageSrc);
			}
			if (isTransitionDestMovie) {
				 // substitute the name of your movie here with valid partial path from ../media/
		        File fileDstMovie = QTFactory.findAbsolutePath (myMovieSrcB);
		        QTFile qtfDstMovie = new QTFile (fileDstMovie.getAbsolutePath());
		        OpenMovieFile openedDstMovie = OpenMovieFile.asRead(qtfDstMovie);
		        Movie movDst = Movie.fromFile(openedDstMovie);
		      	movDst.getTimeBase().setFlags( StdQTConstants.loopTimeBase);	
		        MoviePresenter playerDst = new MoviePresenter(movDst);
		        playerDst.setRate(1);
		        transition.setDestinationImage (playerDst);
			} else {
		       	File fileDstImage = QTFactory.findAbsolutePath (myImageSrcB);
		        QTFile qtfDstImage = new QTFile (fileDstImage.getAbsolutePath());
		        Compositable imageDst = new GraphicsImporterDrawer (qtfDstImage);
		    	transition.setDestinationImage (imageDst);
			}

	        transition.setTime (2000);
	        transition.setProfiled(true);

			// have to set source BEFORE setting filter as the client of the canvas
			// unless you supply a QDDimension to the filter constructor

	        TransitionPanel tp = new TransitionPanel (transition, qtCanvas);
	        add ("North", tp); 
			qtCanvas.setClient (transition, true);
		}

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


    public AtomContainer createSMPTEEffect (int effectType, int effectNumber) throws QTException {
        AtomContainer effectSample = new AtomContainer();
    // We are using SMPTE Effects so set the what atom to smpt
        effectSample.insertChild (Atom.kParentIsContainer, kEffectWhatAtom, 1, 0, EndianOrder.flipNativeToBigEndian32(kWipeTransitionType));
    // We are using SMPTE effect number 74	- start at 0%, stop at 100%
        effectSample.insertChild (Atom.kParentIsContainer, effectType, 1, 0, EndianOrder.flipNativeToBigEndian32(effectNumber));
        return effectSample;
    }


    public AtomContainer createFilterEffect (int effectType, int effectNumber) throws QTException {
        AtomContainer effectSample = new AtomContainer();
	    effectSample.insertChild(Atom.kParentIsContainer, kEffectWhatAtom, 1, 0, EndianOrder.flipNativeToBigEndian32(effectType));
	    effectSample.insertChild(Atom.kParentIsContainer, kParameterTypeDataEnum, 1, 0, EndianOrder.flipNativeToBigEndian32(effectNumber));
        return effectSample;
    }

	/**
	 * Effects Dialog
	 * QTJava 4 had only a rudimentary implementation of the Effects Parameters Dialog
	 * QTJava 5 extends this implementation with a choice of dialog title, preview pictures
	 * and re-editing an existing effect parameter container 
	 */
	static private EffectsList  sEffectsList = null; // for efficiency, generate the list once

	// QTJava 4 ParameterDialog.showParameterDialog
	
    public static void showDialog (QTEffect ef) throws QTException {
    	if (sEffectsList == null) {
    		if (isFilterExampleFlag) {
    			sEffectsList = new EffectsList (1, 1, 0);
    		} else {
    			sEffectsList = new EffectsList (2, 2, 0);
    		}
    	}
        AtomContainer effectSample = ParameterDialog.showParameterDialog (sEffectsList, 0);
        ef.setEffect (effectSample);
    }

	// QTJava 5 ParameterDialog.showParameterDialog
	
    public static void showEffectDialog (QTEffect ef) throws QTException, IOException {
    	if (sEffectsList == null) {
    		if (isFilterExampleFlag) {
    			sEffectsList = new EffectsList (1, 1, 0);
    		} else {
    			sEffectsList = new EffectsList (2, 2, 0);
    		}
		}
		    			
		int		pdOptions = 0;
		if (PlayQTEffectApp.useModalParameterDialog) pdOptions |= pdOptionsModalDialogBox;
		if (false) pdOptions |= pdOptionsCollectOneValue;
		if (false) pdOptions |= pdOptionsAllowOptionalInterpolations;
		
		String title = null;
		if (true)
			title = "Edit effect parameters...";
			
		Pict[] picts = null;
		if (PlayQTEffectApp.usePreviewPictures) {
			picts = new Pict[2];
			picts[0] = makeDialogPict(myImageSrcA);	// srcA
			picts[1] = makeDialogPict(myImageSrcB);	// srcB
		}
		
        AtomContainer fxSample = ParameterDialog.showParameterDialog (
        		sEffectsList, pdOptions, ef.getEffect(), title, picts);
        if (fxSample != null) {
	        ef.setEffect (fxSample);
            if (QTSession.isCurrentOS(QTSession.kMacOSX))
                qtCanvas.repaint();
        }
    }
	
	private static Pict makeDialogPict(String path) throws QTException, IOException {
       	File file = QTFactory.findAbsolutePath (path);
        QTFile qtf = new QTFile (file.getAbsolutePath());
        GraphicsImporter image = new GraphicsImporter (qtf);
        // Note: 128x96 is optimal size for dialog
        return image.getAsPicture();
	}
}
