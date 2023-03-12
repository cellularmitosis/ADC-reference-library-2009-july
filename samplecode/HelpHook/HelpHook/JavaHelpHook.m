/*

File: JavaHelpHook.m

Abstract: JNI hook for calling NSApplication showHelp:
	Must be called on the main AppKit thread to prevent deadlocks with
	Java AWT.

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

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


#import <JavaVM/jni.h>
#import <Cocoa/Cocoa.h>

// The right way to call AppKit from JNI: put any drawing or event-based
// code on the main AppKit thread.  See NSObject docs for more details.
JNIEXPORT void JNICALL Java_apple_dts_samplecode_helphook_HelpHook_showHelp
  (JNIEnv *env, jclass clazz) {
	NSLog(@"Hello from JNI!");
	[[NSApplication sharedApplication] performSelectorOnMainThread:@selector(showHelp:) withObject:NULL waitUntilDone:NO];
}

// The wrong way to call AppKit from JNI: directly from an arbitrary Java thread.  The worst (and most likely)
// case is calling directly from the AWT Event Thread, in response to a Java event.  Explanation below.
JNIEXPORT void JNICALL Java_apple_dts_samplecode_helphook_HelpHook_badShowHelp
  (JNIEnv *env, jclass clazz) {
	/* 
	*	Calling this method in the sample as-is shows no visible difference.  However, removing the
	*	"SampleHelp" folder from your app bundle exposes it as a dangerous technique.
	*
	*   If there is no locatable help folder in your app, an alert dialog is brought up by AppKit.
	*   When this method is called directly from Java's actionPerformed, and the dialog appears, 
	*	the AWT queue is now blocked on it.  Clicks on buttons in the Frame are hung up in
	*	the queue and are not processed until the dialog (and the actionPerformed method waiting on it)
	*	returns.  Worse, resizing the Java frame causes a deadlock: live-resizing originates from AppKit,
	*	which on Mac OS X 10.3 sends blocking repaint requests to AWT, which is still blocked on showHelp.
	*
	*	AWT:actionPerformed --(blocked)--> badShowHelp --> AppKit dialog
	*	 ^
	*	 |
	*	 | (blocked)
	*	 |
	*	AWT:repaint <--(blocked)--	AppKit live resize
	*
	*	Note this is not a problem in the "good" showHelp method, because it correctly sends showHelp:
	*	to the main AppKit thread.  The dialog comes up and is processed normally by AppKit, 
	*	freeing up the AWT Event Queue in the process.  Button clicks, and more importantly live resize repaints
	*	can now be processed.
	*
	*			+----> showHelp --> AppKit dialog
	*			|
	*			| (performSelector... non-blocking)
	*			|
	*	AWT:actionPerformed (allowed to return... AWT events continue to process)
	*	 ^
	*	 |
	*	 | (queue is open.. not blocked)
	*	 |
	*	AWT:repaint <--(blocked)--	AppKit live resize
	*			
	*	This demonstrates the importance of using performSelectorOnMainThread when making AppKit calls from JNI, 
	*	and the more general rule of communicating between AppKit and AWT asynchronously at all times.
	*/
	
	[[NSApplication sharedApplication] showHelp:NULL];
}