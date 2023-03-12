/*
	File:		QTtoJavaImage.java
	
	Description:	This demo program shows the usage of the GraphicsImporterDrawer is used 
                        to produce pixels to create a java.awt.Image.
        
	Author:		Apple Computer, Inc.

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.image.*;
import quicktime.app.image.*;

/*
	This sample code shows how to create a java.awt.Image out 
	of some image that QuickTime produces.
	
	The source of the QuickTime image could come from any one of:
		(1) An image file in a format that Java doesn't directly support but QT does
		(2) Recording the drawing actions of a QDGraphics into a Pict -> this can be
		written out to a file or presented by an ImagePresenter class to the QTIMageProducer directly
		(3) Using the services of QuickTime's SequenceGrabbing component. A SequenceGrabber
		can be used to capture just an individual frame from a video source (the SGCapture shows basic
		usage of the SequenceGrabber and the QT documentation has more details on these components)

	In this code the user is prompted to open an image file (one of 20+ formats that QuickTime's
	GraphicsImporter can import)
	
	The program then uses the QTImageProducer to create a java.awt.Image which is then drawn
	in the paint method of the Frame
*/
public class QTtoJavaImage extends Frame {	
	
	public static void main (String args[]) {
		try {
			QTSession.open ();			
			QTtoJavaImage window = new QTtoJavaImage("QT in Java");
				// this will lay out and resize the Frame to the size of the selected movie
			window.pack();
			window.show();
			window.toFront();
		} catch (QTException e) {
				// catch a userCanceledErr and just exit the program
			if (e.errorCode() == Errors.userCanceledErr) {
				QTSession.close();
				System.exit(0);
			}
				// some other error occured - print out a stack trace
				// and close the QTSession
			e.printStackTrace();
			QTSession.close();
		}
	}

	QTtoJavaImage (String title) throws QTException {
		super (title);
			
			// prompt the user to select an image file
		QTFile imageFile = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
		
			// import the image into QuickTime
		GraphicsImporter myGraphicsImporter = new GraphicsImporter (imageFile);
		
			//Create a GraphicsImporterDrawer which uses the GraphicsImporter to draw
			//this object produces pixels for the QTImageProducer
		GraphicsImporterDrawer myDrawer = new GraphicsImporterDrawer (myGraphicsImporter);
			
			//Create a java.awt.Image from the pixels supplied to it by the QTImageProducer
		QDRect r = myDrawer.getDisplayBounds();
			// this is the size of the image - this will become the size of the frame
		imageSize = new Dimension (r.getWidth(), r.getHeight());
		QTImageProducer qtProducer = new QTImageProducer (myDrawer, imageSize);
		javaImage = Toolkit.getDefaultToolkit().createImage(qtProducer);

			// add a Window Listener to this frame 
			// that will close down the QTSession, dispose of the Frame
			// which will close the window - where we exit
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
	
	Image javaImage = null;
	Dimension imageSize;
	
	public void paint (Graphics g) {
		Insets i = getInsets();
		Dimension d = getSize();
		int width = d.width - i.left - i.right;
		int height = d.height - i.top - i.bottom;
			//make sure image is scaled correctly to fill the entire visible area of the frame
		g.drawImage (javaImage, i.left, i.top, width, height, this);
	}
	
		//this returns the size of the image - so the pack will correctly resize the frame
	public Dimension getPreferredSize () {
		return imageSize;
	}
}





