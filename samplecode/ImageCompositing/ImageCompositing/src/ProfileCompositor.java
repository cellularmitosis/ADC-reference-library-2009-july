/*
	File:		ProfileCompositor.java
	
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

import quicktime.app.anim.*;
import quicktime.qd.*;
import quicktime.*;
import java.util.*;
// We use this class to profile just the tickling times (incl. blit) of the compositor

// we could expand this profiling to include the time between each tickle call, the number of tickle calls a second, etc.
public class ProfileCompositor extends Compositor {
	public ProfileCompositor (QDGraphics gw, QDColor bgColor, int scale, int period) throws QTException {
		super (gw, bgColor, scale, period);
	}
	
	
	int profileCount = 0;
	long startTime, stopTime;
	boolean isProfiling;
	Vector tickleTimes = new Vector();
	long previousTime;
	
		//the tickle method will invoke the idle method to blit
		// as such we can get an idea of the blit time as well
		//by overiding the idle method and profiling it when we
		// are profiling the tickle method
	public boolean tickle (float er, int time) throws QTException {
		//profile the tickle method and print it every 80 times
		isProfiling = profileCount++ % 80 == 0;
		startTime = System.currentTimeMillis();	
		
		boolean ret = super.tickle (er, time);
		
		if (isProfiling) {
			stopTime = System.currentTimeMillis();
			System.out.print (",tickle:" + (stopTime - startTime) + " msecs");
			if (tickleTimes.size() != 0) {
				Enumeration iter = tickleTimes.elements();
				Integer min = new Integer (1000);
				Integer max = new Integer (0);
				int total = 0;
				
				while (iter.hasMoreElements()) {
					Integer el = (Integer)iter.nextElement();
					if (el.intValue() > max.intValue())
						max = el;
					if (el.intValue() < min.intValue())
						min = el;
					total += el.intValue();
				}
				System.out.print (" * * * min=" + min.intValue() + ",max=" + max.intValue() + ",AVG=" + (total / tickleTimes.size()));// + "," + tickleTimes);
			}
			System.out.println("");
			tickleTimes = new Vector();
		}
		tickleTimes.addElement (new Integer ((int)(startTime - previousTime)));
		previousTime = startTime;
		return ret;
	}
	
	long idleStartTime, idleStopTime;
	
	protected void idle () throws QTException {
		if (isProfiling)
			idleStartTime = System.currentTimeMillis();
		
		super.idle();
		
		if (isProfiling) {
			idleStopTime = System.currentTimeMillis();
			System.out.print ("SpriteWorldIdle:" + (idleStopTime - idleStartTime) + " msecs");
		}
	}
}