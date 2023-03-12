/*
 
 File: ImageFileDemo.java
 
 Abstract: Uses a GraphicsImporter to import and display images of various formats in a resizable window.
 
 Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright © 2006 Apple Computer, Inc., All Rights Reserved
 
 */
 
import java.awt.*;
import java.awt.event.*;

import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.GraphicsImporter;
import quicktime.io.QTFile;

import quicktime.app.view.*;

/* QuickTime Graphics Importer and Codec demo */
public class ImageFileDemo extends Frame implements StdQTConstants {		
	
	public static void main (String args[]) {
		/* Provide a standard file open dialogue for our user to select an input movie. */
		try {
			QTSession.open();

			int[] fileTypes = { kQTFileTypeGIF, kQTFileTypeJPEG, kQTFileTypePicture };
			QTFile qtf = QTFile.standardGetFilePreview (fileTypes);

			ImageFileDemo ifd = new ImageFileDemo (qtf);
			ifd.pack();
			ifd.show();
			ifd.toFront();
		} catch (QTException e) {
			if (e.errorCode() != Errors.userCanceledErr)
				e.printStackTrace();
			else
				System.out.println ("UserCanceled : Application needs media file to run. Quitting....");
            QTSession.close();
            System.exit(1);
		}
	}

	ImageFileDemo (QTFile qtf) throws QTException {
		super (qtf.getName());
		/* Construct a GraphicsImporter object.
		 * GraphicsImporter objects are able to manage a
		 * range of different still image file formats */
		GraphicsImporter gi = new GraphicsImporter(qtf);
		
		/* Construct a displayable component able to render the image */
		QTComponent qtComponent = QTFactory.makeQTComponent(gi);

		add ((Component) qtComponent, "Center");

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
}
