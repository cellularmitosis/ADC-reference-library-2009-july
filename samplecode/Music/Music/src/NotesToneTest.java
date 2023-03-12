/*
 
 File: NotesToneTest.java
 
 Abstract: Demonstration of the QTJ musical instruments package
 
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
import quicktime.*;
import quicktime.std.music.*;
import quicktime.std.StdQTException;
import quicktime.util.QTUtils;

public class NotesToneTest implements Errors {
	public static void main (String[] args) {
		try {
			System.out.println ("Starting QTMA Test");
			QTSession.open();
			
			// A NoteAllocator is required for using NoteChannels
			// However QTJava provides a default NA:
			//	NoteAllocator na = NoteAllocator.getDefault();
			// and this is used for the TD/NR and NC constructors if the
			// application does not provide one.
			
			System.out.println ("Pick Instrument:");
			ToneDescription toneDesc = new ToneDescription ();
				//print out an uninitialized ToneDescription
			System.out.println (toneDesc);
			
			try {
				// Have the user choose an instrument and print out the choice
				toneDesc.pickInstrument (NoteAllocator.getDefault(), "Choose an Instrument...", 0);
				System.out.println (toneDesc);
			} catch (StdQTException e) {
				if (e.errorCode() != userCanceledErr) return;
			}
				
				// Make a Tone Description using GMIDI inst. no. 25
			System.out.println ("StuffTone:25");
			toneDesc = new ToneDescription (25);
			System.out.println (toneDesc);

				// Play a note (60) at maximum velocity (127) for 2000msecs then turn it off.
			System.out.println ("PlayNote:Inst,25, Note,60");
			NoteChannel noteChan = new NoteChannel(new NoteRequest(toneDesc));
			noteChan.playNote (60, 127);
				try { Thread.sleep (2000); } 
				catch (InterruptedException e) { e.printStackTrace(); }
			noteChan.playNote (60, 0);
		}
		catch (QTException qte) {
			qte.printStackTrace();
		}
		finally {
			System.out.println ("Finished QTMA Test");
			QTSession.close();
		}
	}
}
