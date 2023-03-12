/*
	File:		JavaText.java
	
	Description:	This demo program shows how to composit a presentation out of disparate media sources, 
                        applying compositing effects such as blend and transparency. Recording a movie from 
                        the activities of the Compositor is also shown.

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
import quicktime.app.image.*;
import java.util.*;

class JavaText implements Paintable {
	Font font = new Font ("Helvetica", Font.PLAIN, 32);
	int width, height;
	Rectangle[] r = new Rectangle[1];
	Random rand = new Random();	//use this for generating random text colour
	
	public void newSizeNotified (QTImageDrawer drawer, Dimension d) {
		width = d.width;
		height = d.height;
		r[0] = new Rectangle (width, height);
	}
	
	/**
	 * Paint on the graphics. The supplied component is the component from which
	 * the graphics object was derived or related to and is also the component
	 * that is the object that paint was called upon that has called this method.
	 * The Graphics object is what you should paint on.
	 * This maybe an on or off screen graphics.
	 * You should not cache this graphics object as it can be different
	 * between different calls.
	 * @param comp the component from which the Graphics object was derived or 
	 * related too.
	 * @param g the graphics to paint on.
	 */
	public Rectangle[] paint (Graphics g) {		
		Color c = new Color (255, Math.abs(rand.nextInt()) % 256, Math.abs(rand.nextInt()) % 256);
		System.out.println ("New TextColor:" + c);
		g.setColor (c);
		g.setFont (font);
		g.drawString ("Java Text", 5, 30);
		g.setColor (Color.red);
		g.drawRect (0, 0, width-1, height-1);
		g.drawRect (1, 1, width-3, height-3);
		g.drawRect (2, 2, width-5, height-5);
		return r;
	}
}
