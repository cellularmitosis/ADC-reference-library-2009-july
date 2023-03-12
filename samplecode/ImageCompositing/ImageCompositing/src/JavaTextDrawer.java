/*
	File:		JavaTextDrawer.java
	
	Description:	This demo program shows how to composit a presentation out of disparate media sources, 
                        applying compositing effects such as blend and transparency. Recording a movie from 
                        the activities of the Compositor is also shown.

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

import java.util.*;

import quicktime.std.StdQTConstants;
import quicktime.std.image.GraphicsImporter;
import quicktime.app.*;
import quicktime.app.players.MoviePlayer;
import quicktime.std.image.GraphicsMode;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;

import quicktime.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.io.*;
import quicktime.std.image.*;
import quicktime.util.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.app.actions.*;
import quicktime.app.anim.*;
import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.players.QTPlayer;
import quicktime.app.actions.*;

import quicktime.app.spaces.*;
import quicktime.util.QTUtils;

import java.io.*;

import quicktime.std.StdQTConstants;
import quicktime.std.image.GraphicsImporter;
import quicktime.std.movies.*;
import quicktime.io.*;
import quicktime.util.*;

public class JavaTextDrawer implements Compositable, Listener, Notifier {
	
	public JavaTextDrawer (Paintable renderer, Dimension size, boolean makeTransparent) {
		fSize		= size;
		fRenderer 	= renderer;
		fMakeTransparent = makeTransparent;
	}
	
	private ImageDataSequence 	ids;
	private NotifyListener 		fListener;
	private Paintable			fRenderer;
	private GraphicsMode		fGraphicsMode;
	private Dimension			fSize;
	private boolean				fMakeTransparent;
	private Object				fInterest;
		
	public void setGraphicsMode(GraphicsMode mode) {
		fGraphicsMode = mode;	
	}
	
	public GraphicsMode getGraphicsMode() {
		return fGraphicsMode;
	}
	
	// need to make sure we don't get a null pointer exception at this stage
	// this is OK to return NUL BECAUSE we are a notifier
	// normally sprites expect valid description and image data
	public EncodedImage getImage() {
		return (ids != null ? ids.getImage() : null);
	}
	
	public ImageDescription getDescription() {
		return (ids != null ? ids.getDescription() : null);
	}
	
	public void removedFrom(Object interest) {
		fInterest = null;
	}
	
	public void addedTo(Object interest) {
		try {
			fInterest = interest;
			
			this.doWork();
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public void doWork() {
		try {
					// if we don't have an interest registered then qid can't draw its image
			if (fInterest != null) {
					// qid is only kept as a local as we only need it to produce
					// a single frame then we throw it away
				QTImageDrawer qid = new QTImageDrawer (fRenderer, fSize, Redrawable.kSingleFrame);			
					//this will capture the result of the Java drawing
				qid.addedTo (fInterest);
					
					// get the image data from the java drawing in
					// a QT format
				ids = new ImageDataSequence (qid.getDescription());
				ids.addMember (qid.getImage()); 
				
				if (fMakeTransparent) {
					ids = ImageUtil.makeTransparent (ids, QDColor.white);				
				}
				
				if (fListener != null)
					fListener.notifyComplete();
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	// this method will be called by the Compositor when it
	// detects that a Notifier has been added to it
	public boolean addNotifyListener (NotifyListener nl) {
		//we set ourselves as the Notifier for this listener
		// if the listener doesn't like us it returns false
		// otherwise we can assume that the listener is able
		// to deal with us		
		if (nl.setNotifier (this) == false)
			return false;
		
		fListener = nl;
		
		if (ids != null) {
			nl.notifyComplete();
		}
		
		return true;
	}
}