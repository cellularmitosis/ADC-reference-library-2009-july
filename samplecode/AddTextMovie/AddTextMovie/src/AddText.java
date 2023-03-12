/*

File: AddText.java

Abstract: Shows how to add a text track to a Movie.

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

import quicktime.*;
import quicktime.io.*;
import quicktime.std.image.*;
import quicktime.std.movies.media.*;
import quicktime.std.movies.*;
import quicktime.std.*;
import quicktime.std.music.*;
import quicktime.qd.*;
import quicktime.util.QTPointer;


import java.awt.*;

public class AddText extends Frame {

	public static void main (String args[]) {
		new AddText();
	}
	
	public AddText() {
		try {
			QTSession.open();     
			QTFile qtf = null;
			
			// Provide a standard file open dialogue for our user to select an input movie.
			try {
				qtf = QTFile.standardGetFilePreview (QTFile.kStandardQTFileTypes);
			} catch (quicktime.io.QTIOException e) {
				  // catch a userCanceledErr and just exit the program
				if (e.errorCode() != Errors.userCanceledErr)
				  e.printStackTrace();
				else
				  System.out.println ("UserCanceled : Application needs media file to run. Quitting....");
				QTSession.close();
				System.exit(1);
            }

			DataRef urlMovie = new DataRef ("file://" + qtf.getPath());
			Movie m = Movie.fromDataRef (urlMovie,StdQTConstants.newMovieActive);
    
			float	textTrackHeight = 32;

			QDRect	movieBounds = m.getNaturalBoundsRect();
			float	movieWidth  = movieBounds.getWidthF();
			float	movieHeight = movieBounds.getHeightF();

			// Construct a new movie track that spans the width of our movie
			Track	textTrack = m.addTrack (movieWidth, textTrackHeight, 0 /* no volume */);

		  	Matrix	textTrackMatrix = textTrack.getMatrix();
			
			// Position the text track along the bottom of the Movie
			textTrackMatrix.translate (0, movieHeight - textTrackHeight);
			textTrack.setMatrix (textTrackMatrix);

			textTrack.setEnabled (true);

			int			movieTimeScale = m.getTimeScale();
			TextMedia	textMedia = new TextMedia (textTrack, movieTimeScale);

	  		QDRect		textBounds = new QDRect (movieWidth, movieHeight);

			// Commence an editing session on the empty textMedia
			textMedia.beginEdits();

			TimeInfo sampleTime = new TimeInfo (0, m.getDuration());
		
			String text = new String ("Adding a text track is easy with\rQuickTime for Java!");
			TextMediaHandler textMediaHandler = textMedia.getTextHandler();
			QTPointer textPointer = new QTPointer ( text.length() + 1, true );
			
			// Copy the contents of our Java String to a C style string for QuickTime.
			textPointer.copyFromArray ( 0, text.getBytes(), 0, text.length() );
			textMediaHandler.addTextSample (
				textPointer,
				QDFont.getFNum ( "Times" ), 
				10, 
				0,
				QDColor.black,
				QDColor.white,
				QDConstants.teCenter,
				textBounds,
				StdQTConstants.dfClipToTextBox | StdQTConstants.dfKeyedText,
				0,0,0,
				null,
				sampleTime.duration );
			
			textMedia.endEdits();

			textTrack.insertMedia (sampleTime.time, 0, sampleTime.duration, 1 );
			OpenMovieFile outStream = OpenMovieFile.asWrite (qtf); 

			m.updateResource (outStream, StdQTConstants.movieInDataForkResID, qtf.getName());
        } catch (QTException e) {
			e.printStackTrace();
            QTSession.close();
            System.exit(0);
		}
        
        QTSession.close();
        System.out.println ("Done.");
        System.exit(0);
    }
}