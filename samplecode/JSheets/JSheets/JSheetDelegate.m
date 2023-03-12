/*

File: JSheetDelegate.m

Abstract: Implementation file for the native side of the Sheets-in-Java example,
	using JAWT to attach a sheet to a Swing JFrame's NSWindow peer.
	See additional comments below, noting the asynchronous communication between
	AppKit and AWT.

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

#import "JSheetDelegate.h"

static JavaVM *jvm;
static jmethodID saveFinish_mID, openFinish_mID, cancel_mID;
static jclass jDelegateClass;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	jvm = vm;
	
	return JNI_VERSION_1_4;
}

// Cache all our callbacks at class-load time
// These may be invalidated if the Java class is unloaded, so we globalRef the class
// To get native signatures for Java methods:
//	% cd <build dir>
//	% javap -classpath JavaHeaders.jar -s apple.dts.samplecode.jsheets.JSheetDelegate
JNIEXPORT void JNICALL Java_apple_dts_samplecode_jsheets_JSheetDelegate_initMethodIDs (JNIEnv *env, jclass clazz) {
	jDelegateClass = (*env)->NewGlobalRef(env, clazz);
	saveFinish_mID = (*env)->GetStaticMethodID(env, clazz, "fireSaveSheetFinished", "(Lapple/dts/samplecode/jsheets/event/SaveSheetListener;Ljava/lang/String;)V");
	openFinish_mID = (*env)->GetStaticMethodID(env, clazz, "fireOpenSheetFinished", "(Lapple/dts/samplecode/jsheets/event/OpenSheetListener;[Ljava/lang/String;)V");
	cancel_mID = (*env)->GetStaticMethodID(env, clazz, "fireSheetCancelled", "(Lapple/dts/samplecode/jsheets/event/SheetListener;)V");	
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
 * Class:     apple_dts_samplecode_jsheets_JSheetDelegate
 * Method:    nativeShowSheet
 * Signature: (ILjava/awt/Component;Lapple/dts/samplecode/jsheets/event/SheetListener;)V
 */
JNIEXPORT void JNICALL Java_apple_dts_samplecode_jsheets_JSheetDelegate_nativeShowSheet 
 (JNIEnv *env, jclass caller, jint type, jobject parent, jobject listener) { 
	// Never assume an AutoreleasePool is in place, unless you are on the main AppKit thread
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	JSheetDelegate *jdel;

	// Take the parent component (passed via Java call) and get the parent NSWindow from it
	NSWindow *parentWindow = GetWindowFromComponent(parent, env);
	
	// It is extremely important to show the sheet from the main AppKit thread WITHOUT BLOCKING using performSelectorOnMainThread with a waitUntilDone value of NO
	switch (type) {
		case apple_dts_samplecode_jsheets_JSheetDelegate_OPEN_PANEL:
			jdel = [JSheetDelegate delegateWithListener:listener env:env];
			// Bump the retain count on the delegate until the sheet goes away (i.e. openPanelDidEnd:)
			[jdel retain];
			[jdel performSelectorOnMainThread:@selector(showOpenPanelForWindow:) withObject:parentWindow waitUntilDone:NO];

			// DON'T DO THIS; AppKit operations off the main AppKit thread will likely hang or crash Cocoa
//			[jdel showOpenPanelForWindow:parentWindow];
			break;
		case apple_dts_samplecode_jsheets_JSheetDelegate_SAVE_PANEL:
			jdel = [JSheetDelegate delegateWithListener:listener env:env];
			// Bump the retain count on the delegate until the sheet goes away (i.e. savePanelDidEnd:)
			[jdel retain];
			[jdel performSelectorOnMainThread:@selector(showSavePanelForWindow:) withObject:parentWindow waitUntilDone:NO];
			break;
	}
	
	[pool release];
}

// Given a Java component, return an NSWindow*
NSWindow * GetWindowFromComponent(jobject parent, JNIEnv *env) {
	JAWT awt;
	JAWT_DrawingSurface* ds;
	JAWT_DrawingSurfaceInfo* dsi;
	JAWT_MacOSXDrawingSurfaceInfo* dsi_mac;
	jboolean result;
	jint lock;

	// Get the AWT
	awt.version = JAWT_VERSION_1_4;
	result = JAWT_GetAWT(env, &awt);
	assert(result != JNI_FALSE);

	// Get the drawing surface
	ds = awt.GetDrawingSurface(env, parent);
	assert(ds != NULL);

	// Lock the drawing surface
	lock = ds->Lock(ds);
	assert((lock & JAWT_LOCK_ERROR) == 0);

	// Get the drawing surface info
	dsi = ds->GetDrawingSurfaceInfo(ds);

	// Get the platform-specific drawing info
	dsi_mac = (JAWT_MacOSXDrawingSurfaceInfo*)dsi->platformInfo;

	// Get the NSView corresponding to the component that was passed
	NSView *view = dsi_mac->cocoaViewRef;
	
	// Free the drawing surface info
	ds->FreeDrawingSurfaceInfo(dsi);
	// Unlock the drawing surface
	ds->Unlock(ds);

	// Free the drawing surface
	awt.FreeDrawingSurface(ds);
	
	// Get the view's parent window; this is what we need to show a sheet
	return [view window];
}


@implementation JSheetDelegate

+ (id) delegateWithListener:(jobject)listener env:(JNIEnv *)env {
	return [[[JSheetDelegate alloc] initWithListener:listener env:env] autorelease];
}

- (id) initWithListener:(jobject)listener env:(JNIEnv*)env {	
	self = [super init];
	
	// Obtain a global ref to the Java listener for this sheet's results.
	// This prevents the listener from being GC'd until we are done with it
	sheetListener = (*env)->NewGlobalRef(env, listener);
	if (sheetListener == NULL) {
		NSLog(@"No memory available for JSheetDelegate GlobalRef");
		return nil;
	}
	return self;
}

// Given a Java-derived NSWindow, show an NSOpenPanel as a sheet
// Register a response selector so Java can be called back with the resulting filename
- (void) showOpenPanelForWindow:(NSWindow *)parentWindow {
	NSOpenPanel *op = [NSOpenPanel openPanel];
	[op setAllowsMultipleSelection:YES];
	[op beginSheetForDirectory:NSHomeDirectory() file:nil types:nil modalForWindow:parentWindow modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

// Same as above, only using NSSavePanel
- (void) showSavePanelForWindow:(NSWindow *)parentWindow {
	NSSavePanel *sp;
	
	// create or get the shared instance of NSSavePanel
	sp = [NSSavePanel savePanel];

	// set up new attributes 
	[sp setRequiredFileType:@"txt"];

	// display the NSSavePanel
	[sp beginSheetForDirectory:NSHomeDirectory() file:nil modalForWindow:parentWindow modalDelegate:self didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

// Delegate selector for openPanel selection
// Calls back to Java with result status: either cancellation or the path for a file to open
- (void) openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
	JNIEnv *env = NULL;
	bool shouldDetach = false;

	// Find out if we actually need to attach the current thread to obtain a JNIEnv, 
	// or if one is already in place
	// This will determine whether DetachCurrentThread should be called later
	if (GetJNIEnv(&env, &shouldDetach) != JNI_OK) {
		NSLog(@"savePanelDidEnd: could not attach to JVM");
		return;
	}
	
	// If we have file results, translate them to Java strings and tell the Java JSheetDelegate 
	// class to notify our listener
	if (returnCode == NSOKButton) {				
		NSEnumerator *files = [[sheet filenames] objectEnumerator];
		
		// init a jobjectArray with [[sheet filenames] count]
		jclass stringClass = (*env)->FindClass(env, "java/lang/String");
		jobjectArray jpaths = (*env)->NewObjectArray(env, [[sheet filenames] count], stringClass, NULL);
		
		NSString *nextFile;
		int javaIndex = 0;
		while (nextFile = (NSString *)[files nextObject]) {
			jsize buflen = [nextFile length];
			jchar buffer[buflen];
			[nextFile getCharacters:(unichar *)buffer];
			// fill the jobjectArray
			jstring js = (*env)->NewString(env, buffer, buflen);
			if (js == NULL) {
				NSLog(@"openPanelDidEnd: could not create jstring");
				return;
			}
			(*env)->SetObjectArrayElement(env, jpaths, javaIndex++, js);
			(*env)->DeleteLocalRef(env, js);
		}
		
		// Tell Java to notify the SheetListener
		if (openFinish_mID != NULL) {
			(*env)->CallStaticVoidMethod(env, jDelegateClass, openFinish_mID, sheetListener, jpaths);
		} else NSLog(@"openPanelDidEnd: bad mID");
		
		(*env)->DeleteLocalRef(env, jpaths);
		(*env)->DeleteLocalRef(env, stringClass);
	} else if (cancel_mID != NULL) {
		// Cancellation callback; Java will invoke sheetCancelled on our listener
		(*env)->CallStaticVoidMethod(env, jDelegateClass, cancel_mID, sheetListener);
	}
	
	// We're done with the listener; release the global ref
	(*env)->DeleteGlobalRef(env, sheetListener);

	// IMPORTANT: if GetJNIEnv attached for us, we need to detach when done
	if (shouldDetach) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	// This delegate was was retained in nativeShowSheet; since this callback occurs on the AppKit thread,
	// which always has a pool in place, it can be autoreleased rather than released
	[self autorelease];
}

// Nearly identical to openPanelDidEnd, except savePanels only ever have a single filename 
- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
	JNIEnv *env;
	bool shouldDetach = false;

	if (GetJNIEnv(&env, &shouldDetach) != JNI_OK) {
		NSLog(@"savePanelDidEnd: could not attach to JVM");
		return;
	}
		
	// Call back to Java if the user clicked Save
	if (returnCode == NSOKButton) {
		if (saveFinish_mID != NULL) {
			jsize buflength = [[sheet filename] length];
			jchar buffer[buflength];
			[[sheet filename] getCharacters:(unichar *)buffer];
			jstring str = (*env)->NewString(env, buffer, buflength);
			
			(*env)->CallStaticVoidMethod(env, jDelegateClass, saveFinish_mID, sheetListener, str);
			(*env)->DeleteLocalRef(env, str);
		} else NSLog(@"savePanelDidEnd: null Java methodID");
	} else if (cancel_mID != NULL) {
		(*env)->CallStaticVoidMethod(env, jDelegateClass, cancel_mID, sheetListener);
	}
	
	(*env)->DeleteGlobalRef(env, sheetListener);

	if (shouldDetach) {
		(*jvm)->DetachCurrentThread(jvm);
	}
	[self autorelease];
}
@end