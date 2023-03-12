/*

File: CreatePictFile.java

Abstract: Imports a PICT, JPEG or GIF file using a GraphicsImporter into a QTComponent.

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
import java.io.File;

import quicktime.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.app.view.*;
import quicktime.io.QTFile;
import quicktime.util.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.*;


public class CreatePictFile extends Frame implements StdQTConstants, QDDrawer {

	Pict myPict;
	CreatePictFile pictFile;
	
	public static void main(String args[]) {		
		try {
			QTSession.open();
			CreatePictFile pictFile = new CreatePictFile();
			pictFile.show();
			pictFile.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}
	
	CreatePictFile () throws Exception {
		super ("QT in Java");
		addWindowListener(new WindowAdapter () {
			public void windowClosed(WindowEvent inEvent) {
				System.exit(0);
			}
				
			public void windowClosing(WindowEvent inEvent) {
				QTSession.close();
				dispose();
			}
		});
					
		addNotify();
		Insets insets = getInsets();
		setBounds (0, 0, (insets.left + insets.right + 500), (insets.top + insets.bottom + 300));

		Button btn = new Button ("Click here to create a PICT File of window's contents");
		btn.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent event) {
				File f = null;
                try {
                    f = new File("results.pict");
					myPict.writeToFile(f);							
				} catch (Exception ex) { 
					ex.printStackTrace();
				}
                System.out.println ("Done - " + f.getAbsolutePath());						
			}
		});
		
		add("North", btn);
		
		int[] fileTypes = { kQTFileTypeGIF, kQTFileTypeJPEG, kQTFileTypePicture };
            QTFile qtf = null;
            try {
                qtf = QTFile.standardGetFilePreview(fileTypes);
			}catch (quicktime.io.QTIOException ex) {
                System.out.println ("UserCanceled : Application needs media file to run. Quitting....");
                QTSession.close();
                System.exit(1);
            }

		myImageFile = new GraphicsImporterDrawer (qtf);

		QDGraphics recordingGraphics = new QDGraphics (myImageFile.getDisplayBounds());
		myImageFile.setGWorld (recordingGraphics);
		
			//Note that the last 2 Parameters to OpenCPicParams represent the resolution in DPI - the default is 72
		OpenCPicParams param = new OpenCPicParams(myImageFile.getDisplayBounds());
			
			// whenever you draw to a QDGraphics from which
			// you are creating a PICT you MUST use the beginDraw
			// method of that QDGraphics. This will set and retain
			// that graphics object as the current graphics for the duration
			// of the draw call - see below
		myPict = Pict.open (recordingGraphics, param);
		
		recordingGraphics.beginDraw (this);
		
		myPict.close();
                
                GraphicsImporter gi = new GraphicsImporter(qtf);
                QTComponent qtComponent = QTFactory.makeQTComponent(gi);
                add("Center", (Component) qtComponent);
        }
	
	GraphicsImporterDrawer myImageFile;

	public void draw (QDGraphics g) throws QTException {
		myImageFile.redraw(null);
	}
}
