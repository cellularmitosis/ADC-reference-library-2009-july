/*
 
 File: JavaMapping_AppDelegate.m
 
 Abstract: Delegate for handling events from JavaFrameView and NSTableView.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
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
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
 */

#import "JavaMapping_AppDelegate.h"
#import <JavaVM/jni.h>

// Save JNI values to minimize callbacks.
static jclass sJavaMappingClass = nil;

@implementation JavaMapping_AppDelegate

#pragma mark -
#pragma mark Java-related methods 
#pragma mark

- (JNIEnv *)getJNIEnv
{
    static JNIEnv* sAppKitEnv = NULL;
    static JavaVM *sJVM = NULL;
	
	if (sAppKitEnv == NULL) {		
		if (sJVM == NULL) {
			JavaVM *createdJVMs;
			jint outCreatedVMCount;
			if (JNI_GetCreatedJavaVMs(&createdJVMs, sizeof(createdJVMs), &outCreatedVMCount) == 0) {
				if (outCreatedVMCount > 0) {
					sJVM = createdJVMs;
				}
			}
		}
		
		// If we didn't find one, we're stuck.
		if (sJVM != NULL) {
			(*sJVM)->AttachCurrentThreadAsDaemon(sJVM, (void **)&sAppKitEnv, nil);
		}
	}
	
	return sAppKitEnv;
}

- (void)javaFrameView:(JavaFrameView *)sender didCreateJavaFrame:(jobject)javaFrame withJNIEnv:(JNIEnv *)inEnv;
{
	// Call our Java method to set up this Java frame. javaFrame is always an instance of java.awt.Frame.
	if (sJavaMappingClass == NULL) {
		sJavaMappingClass = (*inEnv)->FindClass(inEnv, "JavaMappingSupport");
	}
	
	if (sJavaMappingClass) {
		jmethodID setupFrameMethodID = (*inEnv)->GetStaticMethodID(inEnv, sJavaMappingClass, "setupFrame", "(Ljava/awt/Frame;)Ljava/lang/Object;");
		
		if (setupFrameMethodID) {
			jobject frameSupport = (*inEnv)->CallStaticObjectMethod(inEnv, sJavaMappingClass, setupFrameMethodID, javaFrame);
			
			if (frameSupport) {
				mapViewer = (*inEnv)->NewGlobalRef(inEnv, frameSupport);
			}
		}
	}
	
	if ((*inEnv)->ExceptionOccurred(inEnv)) {
		(*inEnv)->ExceptionDescribe(inEnv);
	}
}

- (void)javaFrameFinishedCreation:(NSNotification *)notification
{
	// **** Add code to turn on any UI elements
	// Turn on any UI elements that would interact with the Java frame.
	[zoomSliderVisible setEnabled:YES];
	[zoomButtonsVisible setEnabled:YES];
	[returnHomeButton setEnabled:YES];
	javaFrameReady = YES;
}

- (IBAction)toggleZoomSlider:(id)sender
{
	// Since we can't get this action until javaFrameFinishedCreation is called,
	// there's no need to check for the boolean state
	JNIEnv *theEnv = [self getJNIEnv];
	static jmethodID toggleZoomSliderMethod = NULL;
	
	if (theEnv) {
		if (!toggleZoomSliderMethod) {
			toggleZoomSliderMethod = (*theEnv)->GetMethodID(theEnv, sJavaMappingClass, "toggleZoomSlider", "(Z)V");
		}
		
		if (toggleZoomSliderMethod) {
			(*theEnv)->CallVoidMethod(theEnv, mapViewer, toggleZoomSliderMethod, [zoomSliderVisible state] == 1);
		}
	}
}

- (IBAction)toggleZoomButtons:(id)sender
{
	// Since we can't get this action until javaFrameFinishedCreation is called,
	// there's no need to check for the boolean state
	JNIEnv *theEnv = [self getJNIEnv];
	static jmethodID toggleZoomButtonsMethod = NULL;
	
	if (theEnv) {
		if (!toggleZoomButtonsMethod) {
			toggleZoomButtonsMethod = (*theEnv)->GetMethodID(theEnv, sJavaMappingClass, "toggleZoomButtons", "(Z)V");
		}
		
		if (toggleZoomButtonsMethod) {
			(*theEnv)->CallVoidMethod(theEnv, mapViewer, toggleZoomButtonsMethod, [zoomButtonsVisible state] == 1);
		}
	}
}

- (IBAction)returnHome:(id)sender
{
	// Since we can't get this action until javaFrameFinishedCreation is called,
	// there's no need to check for the boolean state
	JNIEnv *theEnv = [self getJNIEnv];
	static jmethodID returnHomeMethod = NULL;
	
	if (theEnv) {
		if (!returnHomeMethod) {
			returnHomeMethod = (*theEnv)->GetMethodID(theEnv, sJavaMappingClass, "returnHome", "()V");
		}
		
		if (returnHomeMethod) {
			(*theEnv)->CallVoidMethod(theEnv, mapViewer, returnHomeMethod);
		}
	}
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	if (javaFrameReady) {
		JNIEnv *theEnv = [self getJNIEnv];
		
		static jmethodID setNewCenterPointMethod = NULL;
		
		if (theEnv) {
			if (!setNewCenterPointMethod) {
				setNewCenterPointMethod = (*theEnv)->GetMethodID(theEnv, sJavaMappingClass, "setNewCenterPoint", "(DD)V");
			}
			
			if (setNewCenterPointMethod) {
				int selectedRow = [dataTable selectedRow];
				double longitude = [dataSource longitudeForRow:selectedRow];
				double latitude = [dataSource latitudeForRow:selectedRow];
				(*theEnv)->CallVoidMethod(theEnv, mapViewer, setNewCenterPointMethod, latitude, longitude);
			}
		}
	}
}

#pragma mark -
#pragma mark Application setup methods 
#pragma mark

- (void)awakeFromNib
{	
	dataSource = [[GPSDataSource alloc] init];
	[dataTable setDataSource:dataSource];
	
	// **** Add creation of VM arguments here.
	NSString *myJarFile = [[NSBundle mainBundle] pathForResource:@"JavaMappingSupport" ofType:@"jar"];
	NSString *swingxwsJar = [[NSBundle mainBundle] pathForResource:@"swingx-ws" ofType:@"jar"];
	NSString *swingxJar = [[NSBundle mainBundle] pathForResource:@"swingx" ofType:@"jar"];
	NSString *fullClasspath = [NSString stringWithFormat:@"-Djava.class.path=%@:%@:%@", myJarFile, swingxwsJar, swingxJar];
	NSArray *vmArguments = [NSArray arrayWithObject:fullClasspath];
	[JavaFrameView setJavaStartupOptions:vmArguments];
	
	// **** Add Java object creation notification here.
	// **** We set up the delegate relationship to the JavaFrameView in the nib, so no need to do it here.
	// Register for Java object creation notification.
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(javaFrameFinishedCreation:) name:JavaFrameViewDidCreateJavaFrame object:javaView];
}

/**
    Implementation of dealloc, to release the retained variables.
 */
 
- (void) dealloc {
	[dataSource release];
    [super dealloc];
}


@end
