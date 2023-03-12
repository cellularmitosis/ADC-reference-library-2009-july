/*

File: VRMedia.java

Abstract: A custom GenericMedia subclass

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

package com.vr;

import quicktime.std.movies.media.*;
import quicktime.std.StdQTException;
import quicktime.std.movies.Track;
import quicktime.QTException;
import java.util.*;
/** Represents VR Media - this is an example of how to subclass GenericMedia
 * to return a Media object of a subclass that corresponds to the Media Type.
 */
public class VRMedia extends GenericMedia {
	public static void registerMediaType () {
		Media.addMediaType (vrMediaOSType, "com.vr.VRMedia");
	}
	
		// 0x5354706e is the OSType for a panorama media type
	private static final int vrMediaOSType = 0x5354706e;
//_________________________ CLASS METHODS
	public VRMedia (Integer v) throws QTException {
		super (v);
	}	//must have a default constructor for the makeMedia call
	/**
	* This constructor creates a media struct for the specified Track object.
	* <BR><BR><b>QuickTime::NewTrackMedia()</b><BR><BR>
	* @param itsTrack Specifies the Track object this media belongs to.
	* @param timeScale Specifies the time scale of the new media.
	* @param dataRef a DataRef object specifying the default data reference for this media.
	*/
	public VRMedia (Track itsTrack, int timeScale, DataRef dataRef) throws QTException {
		super (itsTrack, timeScale, dataRef, vrMediaOSType);
	}

	/**
	* This constructor creates a media struct for the specified Track object.
	* <BR><BR><b>QuickTime::NewTrackMedia()</b><BR><BR>
	* @param itsTrack Specifies the Track object this media belongs to.
	* @param timeScale Specifies the time scale of the new media.
	*/
	public VRMedia(Track itsTrack, int timeScale) throws QTException {
		this (itsTrack, timeScale, null);
	}
			
//_________________________ INSTANCE METHODS
}