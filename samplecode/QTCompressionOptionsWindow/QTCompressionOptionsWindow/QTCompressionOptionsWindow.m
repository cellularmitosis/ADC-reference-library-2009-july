//
//  QTCompressionOptionsWindow.m
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

#import "QTCompressionOptionsWindow.h"

@implementation QTCompressionOptionsWindow

#pragma mark ---- initialization ----

// designated initializer
// call with QTMediaTypeVideo or QTMediaTypeSound to initalize
- (id)initWithMediaType:(NSString *)inMediaType
{
    // call NSWindowController designated initializer
    if (self = [super initWithWindow:nil]) {
        // inappropriate media types will bail on init
        if ([self setMediaType:inMediaType]) {
            [self release];
            return nil;
        }
    }
    
    return self;
}

#pragma mark ---- deallocation ----

- (void)dealloc
{
    [mMediaType release];
    
    [super dealloc];
}

#pragma mark ---- functions ----

// return the current media type being managed
- (NSString *)mediaType
{
    return [[mMediaType retain] autorelease];
}

// set the media type of the compression options you want managed
- (OSStatus)setMediaType:(NSString *)inMediaType
{
    // accept nil as a request for the default - the more common vide options
    if (nil == inMediaType) inMediaType = QTMediaTypeVideo; 
    
    // only accept Video or Audio since they are the only valid compression option media types at this time
    if ((NO == [inMediaType isEqualToString:QTMediaTypeVideo]) && (NO == [inMediaType isEqualToString:QTMediaTypeSound])) return invalidMedia;
 
    // only do the set up if the media type has changed
    if ([mMediaType isEqualToString:inMediaType]) return noErr;
    
    [mMediaType release];
    mMediaType = [inMediaType retain];
    
    // make sure the nib is actually loaded at this time, required for our connections to the array controller and so on
    if (![self isWindowLoaded]) { [self window]; }
    
    // make sure the array controller is empty
    [mOptionsArray removeObjects:[mOptionsArray arrangedObjects]];
    
    // load it up with the currently chosen compression options objects
    NSArray *optionsIdentifiers = [QTCompressionOptions compressionOptionsIdentifiersForMediaType:inMediaType];
    NSEnumerator *enumerator = [optionsIdentifiers objectEnumerator];
    
    UInt8 index;
    UInt8 count = [optionsIdentifiers count];
    for (index = 0; index < count; index++) {
        QTCompressionOptions *options = [QTCompressionOptions compressionOptionsWithIdentifier:[enumerator nextObject]];
        [mOptionsArray addObject:options];
    }
    
    [mOptionsArray setSelectionIndex:0];
    
    return noErr;
}

// return the currently selected compression options object
- (QTCompressionOptions *)compressionOptions
{
    return [[mOptionsArray selectedObjects] lastObject];
}

#pragma mark ---- actions ----

- (IBAction)closeWindow:(id)sender
{
    [self close];
}

#pragma mark ---- subclassing behaviors ----

// window loaded
- (void)windowDidLoad
{
    if (nil == mMediaType) {
        // if a client of this class fails to set the media type
        // but decides to show the window, set it here to the default
        [self setMediaType:QTMediaTypeVideo];
    }
}

// inform the client of this class that the window has been closed
// the client will then be able to ask for the currently selected
// compression options object
- (void)close
{
    [super close];
        
    if (nil != delegate && [delegate respondsToSelector:@selector(compressionOptionsWindowDidClose:)] ) {
    
        [delegate compressionOptionsWindowDidClose:self];
    }
}

// return the name of the nib file that stores the window
- (NSString *)windowNibName
{
    return @"QTCompressionOptionsWindow";
}

#pragma mark ---- delegate ----

// manage deletates
- (id)delegate {
    return delegate;
}

- (void)setDelegate:(id)inDelegate {
    delegate = inDelegate;
}

@end