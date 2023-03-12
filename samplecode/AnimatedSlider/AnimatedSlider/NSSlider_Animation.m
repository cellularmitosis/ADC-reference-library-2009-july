//
//  NSSlider_Animation.m
//  AnimatedSlider
//
//  Created by jcr on Tue Nov 06 2001.
// Copyright 2002, Apple Computer, Inc.  See the legal notice at the end of this file.

#import "NSSlider_Animation.h"

float easeFunction(float t);

const float sliderAnimationInterval = 1/10;  // This is basically the "frame rate" for animating sliders.
const float sliderAnimationSeconds = 1; 	// This is how long sliders should take to get where they're going.  A second seemed like a good start.

static NSString *endValueKey = @"endValue", *startValueKey = @"startValue", *startDateKey = @"startDate";

static NSMutableDictionary *animationTimersDictionary;  //This is where we keep track of timers, for when we need to remove them.

@implementation NSSlider (SliderAnimation)
  // When a method is very short and very obvious, I like to make it a one-liner.  These are just covers for sending the messages along to the cell.
- (IBAction) animateToFloatValueFrom:sender { [[self cell] animateToFloatValueFrom: sender];}
- (void) animateToFloatValue:(float) newValue { [[self cell] animateToFloatValue: newValue];}

@end

@interface NSSliderCell (SliderAnimationPrivate)  

- (void) doAnimationStep: sender;  // This should only ever be sent by the NSTimer created in -animateToFloatValue:.
- (void) stopAnimating;   // Used to clean up any stray timers.

@end

@implementation NSSliderCell (SliderAnimation) 
/* In an NSSlider, like most NSControl subclasses, an NSActionCell object does the real work.  An NSSlider is a view which contains an NSSliderCell.  Since we're putting this code into the cell class, and not the view, this will work just as well for a matrix of slider cells as it does for a single slider.*/
	
+ (NSMutableDictionary *) animationTimersDictionary	
	{
	if (!animationTimersDictionary)   // Create the dictionary if we haven't already.
		animationTimersDictionary = [[NSMutableDictionary dictionary] retain];
	return animationTimersDictionary;
	}
	
- (IBAction) animateToFloatValueFrom:sender // Action method 
  { 
  [self animateToFloatValue:[sender floatValue]];   
  }
  
- (void) animateToFloatValue:(float) newValue
	{
	NSTimer 
		*timer;
		
	NSDictionary 
		*animationInfo;   // Everything we need to know while animating..
	
	[self stopAnimating];  // This ensures that we don't get multiple timers trying to move the slider at the same time..
	
	  /*   NSTimer has a "userInfo" method that can hand us back whatever information we want to associate with a timer.  In this case, we'll use a dictionary  to hold multiple parameters.  To do the math for our spiffy ease-in/ease-out motion smoothing, at each step in the animation we need to know what value we started with, what value we're aiming for, and when we started to animate. */
	animationInfo =   
		[NSDictionary dictionaryWithObjectsAndKeys:  
			[NSNumber numberWithFloat:newValue], endValueKey,
			[NSNumber numberWithFloat:[self floatValue]], startValueKey,
			[NSDate date], startDateKey, nil];
		
   timer = // Make a new timer..
	  [NSTimer scheduledTimerWithTimeInterval:sliderAnimationInterval // how often it fires
			target:self   // what object it will message when it fires
			selector:@selector(doAnimationStep:) // What method will it invoke when it fires
			userInfo: animationInfo  // The NSDictionary we just built above..
			repeats:YES];  
	[[[self class] animationTimersDictionary] setObject:timer forKey:[self description]];  // Store a reference to this timer, so that -stopAnimating can find it.
	}

@end 

@implementation NSSliderCell (SliderAnimationPrivate)
 
- (void) doAnimationStep:sender
  {
  id
	 parms = [sender userInfo];  // This the dictionary that we created above in -animateToFloatValue:
	 
  id
	 controlView = [self controlView];  // The view that this cell lives in..
	 
	float 
		t, startValue, endValue, currentValue;

  endValue = [[parms valueForKey:endValueKey] floatValue];
  startValue = [[parms valueForKey:startValueKey] floatValue];

  t = MIN(1.0 , fabs([[parms valueForKey:startDateKey] timeIntervalSinceNow]) / sliderAnimationSeconds);  //Looking for a value from zero to one..
  t = easeFunction(t);  // comment this out to make it linear.

  currentValue = startValue + (t * (endValue - startValue));
  [self setFloatValue: currentValue];
  
  if ([controlView isContinuous])  // When the slider moves, the target should get a message..
    [controlView sendAction:[self action] to:[self target]];
	
  if (fabs(currentValue - endValue) <= 0.01)  // allow a bit of "servo jitter" to allow for floating-point messiness
	 {  
	 [self stopAnimating];
	 [self setFloatValue:endValue];
	 [controlView sendAction:[self action] to:[self target]];  // Make it act just like the user did the moving.
	 }
  }

- (void) stopAnimating  //This method removes any NSTimer that's trying to move the slider.
  {
	NSString 
		*dictKey = [self description];
		
	NSMutableDictionary 
		*timers = [[self class] animationTimersDictionary];
		
	NSTimer 
		*oldTimer;
	
	if (oldTimer = [timers objectForKey:dictKey])
	  {
		[oldTimer invalidate];  // If there already was a timer, clean it up.
		[timers removeObjectForKey:dictKey]; 
		}
	}
  
@end

float easeFunction(float t)  // This function implements a sinusoidal ease-in/ease-out for t = 0 to 1.0.  T is scaled to represent the interval of one full period of the sine function, and transposed to lie above the X axis.
	{
   return (sin((t * M_PI) - M_PI_2) + 1.0 ) / 2.0;
	} 

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
