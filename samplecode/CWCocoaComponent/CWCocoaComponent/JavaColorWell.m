/*

File: JavaColorWell.m

Abstract: Implementation file for the native side of the 
	NSColorWell-in-Java example.  See comments below for 
	explanation of how the NSColorWell communicates color 
	changes back to Java.

Version: 1.1

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

#import "JavaColorWell.h"

static JavaVM *jvm;
static jmethodID colorChange_mID;

// Cache the JavaVM when this library is loaded; it never changes (only one JVM per process)
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	jvm = vm;
	return JNI_VERSION_1_4;
}

// jmethodIDs never go stale for the life of the class, so fetch them on classload and keep them around
// It is necessary to ensure the object bearing the method call is valid at call time
// From the build directory: javap -classpath JavaHeaders.jar -s apple.dts.samplecode.cwcocoacomponent.JavaColorWell
JNIEXPORT void JNICALL Java_apple_dts_samplecode_cwcocoacomponent_JavaColorWell_initMethodIDs (JNIEnv *env, jclass clazz) {
	// This potentially throws NoSuchMethodError, which we let propagate since the app is useless without it
	colorChange_mID = (*env)->GetMethodID(env, clazz, "cocoaColorChange", "(FFFF)V");
}

// Determines whether the current thread is already attached to the VM,
// and tells the caller if it needs to later DetachCurrentThread 
//
// CALL THIS ONCE WITHIN A FUNCTION SCOPE and use a local boolean
// for mustDetach; if you do not, the first call might attach, setting 
// mustDetach to true, but the second will misleadingly set mustDetach 
// to false, leaving a dangling JNIEnv
jint GetJNIEnv(JNIEnv **env, bool *mustDetach) {
	jint getEnvErr = JNI_OK;
	*mustDetach = false;
	if (jvm) {
		getEnvErr = (*jvm)->GetEnv(jvm, (void **)env, JNI_VERSION_1_4);
		if (getEnvErr == JNI_EDETACHED) {
			getEnvErr = (*jvm)->AttachCurrentThread(jvm, (void **)env, NULL);
			if (getEnvErr == JNI_OK) {
				*mustDetach = true;
			}
		}
	}
	return getEnvErr;
}

/*
 * Class:     apple_dts_samplecode_cwcocoacomponent_JavaColorWell
 * Method:    createNSViewLong
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_apple_dts_samplecode_cwcocoacomponent_JavaColorWell_createNSViewLong (JNIEnv *env, jobject caller) {
	return (jlong)[JavaColorWell colorWellWithCaller:caller env:env];
}

@implementation JavaColorWell

+ (id) colorWellWithCaller:(jobject) caller env:(JNIEnv *)env {
	return [[[JavaColorWell alloc] initWithCaller:caller env:env] autorelease];
}

// Obtain a JNI global reference to the enclosing Java object
// This will be used later to fire AWT events after a color change
- (id) initWithCaller:(jobject)caller env:(JNIEnv *)env {
	self = [super init];
	javaPeer = (*env)->NewGlobalRef(env, caller);
	
	return self;
}

// Override [NSColorWell activate] to show the opacity slider
- (void) activate:(BOOL)exclusive {
	[[NSColorPanel sharedColorPanel] setShowsAlpha:YES];
	[super activate:exclusive];
}

// Override [NSColorWell setColor] to fire a notification to Java when this ColorWell's color changes
// This is lighter-weight than observing a notification, and avoids the complexity of whether
// or not this ColorWell is active
- (void) setColor:(NSColor *)color {
	[self panelColorChange:color];
	[super setColor:color];
}

// This selector is performed by the observer on the sharedColorPanel whenever the selected color changes
// Call back to the enclosing Java instance so AWT events can be fired to respond to the color change
- (void) panelColorChange:(NSColor *)newColor {
	// The JavaVM can be cached; JNIEnvs are thread-specific and should be re-fetched every time
	JNIEnv *env = NULL;
	bool shouldDetach = false;
	
	if (GetJNIEnv(&env, &shouldDetach) != 0) {
		NSLog(@"panelColorChange: could not attach to JVM");
		return;
	}
	
            if (javaPeer != NULL && colorChange_mID != NULL) {
		// Convert the selected color to an RGB space that can be easily understood by Java
		NSColor *rgbColor = [newColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
		if (rgbColor == nil) return;
	
		float r, g, b, a;
		[rgbColor getRed:&r green:&g blue:&b alpha:&a];
		
		// Call back into Java with the new color properties; the Java peer will fire ColorSelectionEvents to all its listeners		
		(*env)->CallVoidMethod(env, javaPeer, colorChange_mID, r, g, b, a);
		
		// IMPORTANT: if GetJNIEnv attached for us, we need to detach when done
		if (shouldDetach) {
			(*jvm)->DetachCurrentThread(jvm);
		}
            }

}

// Native target of the CocoaComponent.sendMessage method; see JavaColorWell.java
- (void) awtMessage:(jint)messageID message:(jobject)message env:(JNIEnv *)env { 
	switch (messageID) {
		// Delete the globalRef to the Java peer when the component is removed from its container
		case apple_dts_samplecode_cwcocoacomponent_JavaColorWell_REMOVE_NOTIFY:
			if (javaPeer != NULL) {
				(*env)->DeleteGlobalRef(env, javaPeer);
			}
			break;
		case apple_dts_samplecode_cwcocoacomponent_JavaColorWell_DEACTIVATE:
			// In case we need to deactivate the ColorWell directly
			// This is not called anywhere in the demo app
			[self deactivate];
			break;
		case apple_dts_samplecode_cwcocoacomponent_JavaColorWell_CLOSE_PANEL:
			// Close the NSColorPanel from Java
			// This could be a static method since the NSColorPanel is shared,
			// but sendMessage is more convenient
			[[NSColorPanel sharedColorPanel] close];
			break;
		default:
			break;
	}
}

@end
