/*
    File:  AnimatingTabView.h

    Abstract:  An NSTabView subclass that animates tab switches using Core Image "Transition" Filters
     
    Version:  1.0

    Copyright:  � Copyright 2005-2006 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
*/

#import <Cocoa/Cocoa.h>

@class CIFilter;
@class CIImage;

// These are the transition styles that AnimatingTabView supports.  Each corresponds to one of Core Image's standard transition filters.

typedef enum {
    AnimatingTabViewCopyMachineTransitionStyle,
    AnimatingTabViewDisintegrateWithMaskTransitionStyle,
    AnimatingTabViewDissolveTransitionStyle,
    AnimatingTabViewFlashTransitionStyle,
    AnimatingTabViewModTransitionStyle,
    AnimatingTabViewPageCurlTransitionStyle,
    AnimatingTabViewRippleTransitionStyle,
    AnimatingTabViewSwipeTransitionStyle
} AnimatingTabViewTransitionStyle;

// "AnimatingTabView" is a subclass of NSTabView that animates tab switches using a Core Image transition filter.  The animation is triggered automatically whenever a tab switch occurs.  All an application has to do is use an instance of AnimatingTabView instead of an ordinary NSTabView.

@interface AnimatingTabView : NSTabView
{
    // Animation State
    int             transitionStyle;        // the style of transition to use; one of the AnimatingTabViewTransitionStyle values enumerated above
    CIFilter        *transitionFilter;      // the Core Image transition filter that will generate the animation frames
    CIImage         *inputShadingImage;     // an environment-map image that the transitionFilter may use in generating the transition effect
    CIImage         *inputMaskImage;        // a mask image that the transitionFilter may use in generating the transition effect
    NSRect          imageRect;              // the subrect of the AnimatingTabView where the animating image should be composited
    NSAnimation     *animation;             // the NSAnimation instance that will drive the transitionFilter's time input value
    BOOL            slowMotionDemo;         // YES if we play the animation slowly in "Demo mode"; NO for normal playback speed
}

//	Could be hooked up to popup, but we use a binding on the popup instead
//- (IBAction)transitionChange:(id)sender;

#pragma mark -
#pragma mark *** Accessors ***

- (AnimatingTabViewTransitionStyle)transitionStyle;
- (void)setTransitionStyle:(AnimatingTabViewTransitionStyle)newTransitionStyle;

@end
