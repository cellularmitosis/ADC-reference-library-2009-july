/*
	File:		SlideShow.java
	
	Description:	Flow logic for images in the slide show.  Extension of the 
                        QuickTime ImageViewer.

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

import java.awt.event.*;
import java.awt.*;

import quicktime.*;
import quicktime.app.image.*;
import quicktime.app.display.*;

public class SlideShow extends ImageViewer implements MouseListener {
//_________________________ CLASS METHODS
	public SlideShow (ImageSequencer images) throws QTException {
		super (images);
	}
//_________________________ INSTANCE METHODS
	/**
	 * This method is called when a Drawable object is added to a canvas.
	 * @param obj the QTCanvas that the drawable object is being added to.
	 */
	public void addedTo (Object obj) {
		((Component)obj).addMouseListener (this);
	}

	/**
	 * This method is called when a Drawable object is removed a canvas.
	 * @param obj the QTCanvas that the drawable object is being removed from.
	 */
	public void removedFrom (Object obj) {
		((Component)obj).removeMouseListener (this);
	}

// this would be a good place to put a transition
	/** Mouse pressed events are used to change the current image */
	public void mousePressed (MouseEvent ev) {
		if ((ev.getModifiers() & InputEvent.ALT_MASK) != 0) {
			getImages().setFramePrevious();
		} else {
			getImages().setFrameNext();
		}
		try {
			setImageData (getImages().getImage(), getImages().getDescription());
		} catch (QTException e) { e.printStackTrace(); }
	}

	public void mouseClicked (MouseEvent e) {}
	public void mouseEntered (MouseEvent e) {}
	public void mouseExited (MouseEvent e) {}
	public void mouseReleased (MouseEvent e) {}
}

