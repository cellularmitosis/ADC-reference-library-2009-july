/*
	File:		SGDataProcDemo
	
	Description:<Description Here>

	Author:		TJ

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

*/

import java.awt.*;
import java.awt.event.*;

import quicktime.qd.*;
import quicktime.*;
import quicktime.util.*;


import quicktime.io.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.CodecComponent;
import quicktime.std.sg.*;
import quicktime.std.image.*;
import quicktime.app.sg.SGDrawer;
import quicktime.app.image.ImagePresenter;
import quicktime.std.movies.*;
import quicktime.app.display.QTCanvas;
import quicktime.std.StdQTException;
import quicktime.QTException;

public class SGDataProcDemo extends Frame implements WindowListener, SGDataProc, Errors {	

	static final int kWidth = 320;
	static final int kHeight = 240;
        
	private QTCanvas 		targetQTCanvas;
        private DSequence		myDSequence = null;
        private QDRect			targetRect = new QDRect(kWidth, kHeight);
        private	SGVideoChannel		sgVideo = null;
        
	public static void main (String args[]) {
	try {
		SGDataProcDemo sgDataProcDemo = new SGDataProcDemo("QTJava Sequence Grabber DataProc");
		sgDataProcDemo.show();
		sgDataProcDemo.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	SGDataProcDemo (String title) {
		super (title);
		try {		
			QTSession.open();		
                        System.out.println ("Make sure that you have connected a video capturing device or SoftVidig is installed");

                        // Create and configure the canvas
			targetQTCanvas = new QTCanvas(QTCanvas.kPerformanceResize, .5F ,.5F);
                        
                        targetQTCanvas.clientChanged (new Dimension (kWidth, kHeight));

			add ("Center", targetQTCanvas);

                        pack();

			Insets insets = getInsets();
			setBounds (0, 0, (insets.left + insets.right + kWidth), (insets.top + insets.bottom + kHeight));

			addNotify();
                        
			addWindowListener(this);

                        setResizable(false);

		} catch (QTException ee) {
			ee.printStackTrace();
			QTSession.close();
		}
	}

	public void windowClosing (WindowEvent e) {
		QTSession.close();
		dispose();
	}

        // These aren't of interest, but we needed to impliment them for the
        // WindowListener interface.
	public void windowIconified (WindowEvent e) {}	
	public void windowDeiconified (WindowEvent e) {}
	public void windowActivated (WindowEvent e) {}
	public void windowDeactivated (WindowEvent e) {}
	
	public void windowClosed (WindowEvent e) { 
		System.exit(0);
	}
	
	public void windowOpened (WindowEvent e) {
                
		try{	
                        // Configure the SequenceGrabber.
			SequenceGrabber sgGrabber = new SequenceGrabber();
			
                        // Call our execute method with the raw data
                        sgGrabber.setDataProc(this);

                        // Create the video channel for for the sequence grabber.
			sgVideo = new SGVideoChannel(sgGrabber);

			sgVideo.settingsDialog ();
		
			sgVideo.setBounds (targetRect);
                        
			sgVideo.setUsage ( StdQTConstants.seqGrabRecord );

                        // This demo doesn't use the preview, but it will record.
			sgGrabber.prepare(false,true);

                        // You need to create a SGDrawer for the video channel in order for the DataProc to be called.
                        SGDrawer sgDrawer = new SGDrawer(sgVideo);

                        // Don't make a Movie.
                        sgGrabber.setDataOutput(null,StdQTConstants.seqGrabDontMakeMovie);
                        
                        // Start the sequence grabber.
                        sgGrabber.startRecord();
                        
                        // Start tasking the SGDrawer so that our DataProc will be called.
                        sgDrawer.startTasking();

		} catch (QTException ee) {
			ee.printStackTrace();
			QTSession.close();
		}
	}
        
        // Sequence Graber DataProc
	public int execute(SGChannel chan, QTPointerRef dataToWrite, int offset, int chRefCon, int time, int writeType) {

            if(chan.getClass().getName().equals("quicktime.std.sg.SGVideoChannel") && writeType == StdQTConstants.seqGrabWriteAppend) {
                //Wrap the data with a RawEncodedImage Object
                RawEncodedImage encodedImage = RawEncodedImage.fromQTPointer(dataToWrite);
            
               try {
                    //Only the first time the DataProc's execute is called with an image we setup the Decompression sequence.
                    if(myDSequence == null) {
                       // targetGraphics is the QDGraphics port will will be decompressing the image to.
                        QDGraphics targetGraphics = targetQTCanvas.getPort();

                        // Get the imageDescription of the source image.
                        ImageDescription sgImageDescription = sgVideo.getImageDescription();

                        // sourceRect is the size of the image being captured.
                        QDRect 	sourceRect = sgImageDescription.getBounds();

                        // transformationMatix is the matrix that represents the trasformation of the image from the 
                        // image description from the sequence grabber source to the QuickDraw port.
                        Matrix transformationMatix = new Matrix();
                        transformationMatix.rect(sourceRect,targetGraphics.getPortRect());                       

                        //Setup the decompression sequence passing in the first image as a sample image.
                        myDSequence = new DSequence(sgImageDescription, encodedImage, targetGraphics, sourceRect, transformationMatix, null, 0, StdQTConstants.codecNormalQuality, CodecComponent.bestSpeedCodec);
                    }

                    //Decompress the image to the targetGraphics
                    myDSequence.decompressFrame(encodedImage,0);
               } catch(StdQTException e) {
                    e.printStackTrace();
                } catch(QTException e) {
                    e.printStackTrace();
                }
            }
            return 0;
	}
}
