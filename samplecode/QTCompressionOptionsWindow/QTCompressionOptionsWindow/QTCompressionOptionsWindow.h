//
//  QTCompressionOptionsWindow.h
//
//  A very simple window wrapper around the QTCompressionOptions object
//  which can be used to bring up a UI listing the current compression
//  options for use with QTCaptureFileOutput -compressionOptionsForConnection

/*
 
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
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.
 
 */

// QTCompressionOptionsWindow is a simple class that demonstrates how easy it is to manage the preset QTCapture media
// compression options encapsulated in a QTCompressionOptions object.

// The QTCompressionOptions class represents a set of compression options for a particular type of media.
// QTCompressionOptions objects are used to describe compression options for different kinds of media.
// Compression options are created from presets keyed by a named identifier such as @"QTCompressionOptions120SizeH264Video"
// or @"QTCompressionOptionsHighQualityAACAudio" but may also be returned dynamically using the
// +compressionOptionsIdentifiersForMediaType method.
 
#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

@interface QTCompressionOptionsWindow : NSWindowController {
@private
    IBOutlet NSArrayController  *mOptionsArray;
    NSString                    *mMediaType;
    
    IBOutlet id                  delegate;
}

// designated initializer
- (id)initWithMediaType:(NSString *)inMediaType;

// compression options of this media type are being managed by the controller
- (NSString *)mediaType;

// manage compression options for the media type passed in, should only be
// QTMediaTypeVideo or QTMediaTypeSound, nil is equivalent to QTMediaTypeVideo
- (OSStatus)setMediaType:(NSString *)inMediaType;

// returns the selected QTCompressionOptions instance that may be used with -setCompressionOptions:forConnection:
- (QTCompressionOptions *)compressionOptions;

// closes the window and notifies the client of the class via delegation
- (IBAction)closeWindow:(id)sender;

// manages controller delegate
- (id)delegate;
- (void)setDelegate:(id)value;

@end

// a client of this class should implement this delegate method
// which is called when the compression options window is closed
// the client may ask the sender for the selected QTCompressionOptions instance
@interface NSObject (QTCompressionOptionsWindowDelegate)

- (void)compressionOptionsWindowDidClose:(id)sender;

@end
