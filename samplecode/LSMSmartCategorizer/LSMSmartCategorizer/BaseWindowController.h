/*
 File: BaseWindowController.h
 
 Abstract: Base class of all window controller.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, 
 Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple,
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
 
 Copyright ¬© 2007 Apple Inc., All Rights Reserved
 
 */

#import <Cocoa/Cocoa.h>
@class CategoryDataInfo;

/*! 
 * @abstract Super class to all window controllers used by this app.
 *
 * Both EvalWindowController and TrainingWindowController share some common 
 * controls and functionalities. So we put those in this super class.
 */
@interface BaseWindowController : NSObject {
	IBOutlet NSTextField* trainStatusText; /**< status text field. */
	IBOutlet NSTextView * logTextView; /**< log text view. */
	IBOutlet NSOutlineView* outlineView; /**< outline view that display categorized data */
	CategoryDataInfo* topLevelDataInfo; /**< Outline view data source. */
	
	//
	// These two variables are used by the availability bind of several controls,
	// such as buttons and progress indicators.
	//
	// busy: indicate if there some task is running, some buttons are disabled during tasks.
	// cancelEnabled: indicate if the cancel button is enabled. The cancellation of
	//                some tasks is very hard to implement. For those tasks, even
	//                cancel button is diabled.
	//
	BOOL busy; 
	BOOL cancelEnabled;
}

/*!
 * @abstract Set busy.
 */
- (void)setBusy:(BOOL)yn;

/*!
 * @abstract Set cancelEnabled.
 */
- (void)setCancelEnabled:(BOOL)yn;

/*!
 * @abstract Return whether the root of outline view data source contains any category.
 */
- (BOOL)hasCategories;
@end

/*!
 * @abstract Private routines.
 */
@interface BaseWindowController(Private)
/*!
 * @abstract Set the user interface (UI) to busy that does not allow cancellation.
 * @input statusText Test to be displayed on status text field.
 */
- (void) setUIAllBusy:(NSString*)statusText;

/*!
 * @abstract Set the user interface (UI) to busy that allows cancellation.
 * @input statusText Test to be displayed on status text field.
 */
- (void) setUICancellableBusy:(NSString*)statusText;

/*!
 * @abstract Set the user interface (UI) to idle.
 */
- (void) setUIIdle;

/*!
 * @abstract Append msg to log textView.
 */
- (void) log:(NSString*) msg;

/*!
 * @abstract Make the outline view to reload data source.
 */
- (void) reloadOutlineView;
@end
