/*
File: PluginWithJavaScript.m

Abstract: .

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

#import "PluginWithJavaScript.h"


@implementation PluginWithJavaScript


NSString* gKeysToObserve[] = {
	@"speed",
	@"curvature",
	@"ticks"
};
#define kNKeysToObserve (sizeof(gKeysToObserve)/sizeof(NSString*))



	/* initializer for our plug-in */
-(id) initWithArguments:(NSDictionary *)arguments {
	PDEBUG();
	if ((self = [super init]) != nil) {
			
			/* save the arguments */
		[self setPluginArguments: arguments];
		
			/* add some key value observers for some
			attributes of the SpeedometerView's properties.
			when the KVC setters are called for those
			values, our obeserver will be called.  See the comment above
			the -observeValueForKeyPath:ofObject:change:context:
			method below.*/
		int ix;
		for (ix=0; ix<kNKeysToObserve; ix++) {
			[self addObserver:self
				forKeyPath: gKeysToObserve[ix]
				options:(NSKeyValueObservingOptionNew |
					NSKeyValueObservingOptionOld)
				context:NULL];
		}
		
			/* default values for our properties */
		[self setTitle: @"Speedometer"];
		[self setCounter: [NSNumber numberWithUnsignedInt: 0]];
	}
	return self;
}


	/* deallocate plug-in */
- (void) dealloc {
	PDEBUG();
	
		/* release the arguments */
	[self setPluginArguments: nil];
	[self setTitle: nil];
	[self setCounter: nil];
		/* remove the key value observers for the speedometer
		view properties. */
	int ix;
	for (ix=0; ix<kNKeysToObserve; ix++) {
		[self removeObserver:self
			forKeyPath:gKeysToObserve[ix]];
	}
		/* deallocate superclass */
	[super dealloc];
}


	/* WebPlugInViewFactory message handler for vending plug-in instances */
+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments {
    PluginWithJavaScript *thePlugin;
	PDEBUG();
	thePlugin = [[[self alloc] initWithArguments:arguments] autorelease];
    return thePlugin;
}


	/* accessors for pluginArguments */
- (NSDictionary *)pluginArguments {
	PDEBUG();
    return [[pluginArguments retain] autorelease];
}

- (void)setPluginArguments:(NSDictionary *)value {
	PDEBUG();
    if (pluginArguments != value) {
        [pluginArguments release];
        pluginArguments = [value copy];
    }
}
	

- (NSString *)title {
	PDEBUG();
    return [[title retain] autorelease];
}

- (void)setTitle:(NSString *)value {
	PDEBUG();
    if (title != value) {
        [title release];
        title = [value copy];
    }
}

- (NSNumber *)counter {
	PDEBUG();
    return [[counter retain] autorelease];
}

- (void)setCounter:(NSNumber *)value {
	PDEBUG();
    if (counter != value) {
        [counter release];
        counter = [value copy];
    }
}



	
	
	/* Calling JavaScript in the containing page from our plug-in. */
	

	/* this method is our key-value observer.  Whenever it is called
	for one of the keys we have registered our self as the observer for,
	we will call through to the JavaScript on the containing page to
	provide a notification that the value has changed.
	
	In our -initWithArguments: method, we set ourself as the key value
	observer for the keys @"speed", @"curvature", @"ticks", and KVC accessors
	are defined for those keys on our SpeedometerView superclass.  Inside
	of the observer, we intercept calls to the accessors and call through to
	the JavaScript in the containing page to provide notifications about
	the changing values.
	
	The names of the JavaScript functions called is based on the name of the
	key.  For the keys @"speed", @"curvature", and @"ticks" we call JavaScript
	methods named @"speedValueChangedTo", @"curvatureValueChangedTo", and
	@"ticksValueChangedTo".
	*/ 
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
		change:(NSDictionary *)change context:(void *)context {
	PDEBUG(@"keyPath = '%@'", self, NSStringFromSelector(_cmd), keyPath);
  	
	id pluginContainer = [[self pluginArguments] objectForKey:WebPlugInContainerKey];
	if (pluginContainer) {
	
			/* retrieve a reference to the webview */
		WebView *myWebView = [[pluginContainer webFrame] webView];
		
			/* Formulate the name of the callback as 'NNNNNValueChangedTo'
			where NNNNN is the key name */
		NSString* callbackName = [NSString stringWithFormat:@"%@ValueChangedTo", keyPath ];
				
			/* create the list of parameters for our call back.  in this case, there
			is only one parameter. */
		NSArray* parameterList = [NSArray arrayWithObject: [change objectForKey:NSKeyValueChangeNewKey]];
			
			/* make a simple call through to JavaScript. */
		[[myWebView windowScriptObject]
			callWebScriptMethod:callbackName withArguments:parameterList];
	}
}




	/* Calling our plug-in from JavaScript in the containing page. */


- (id)objectForWebScript {
	PDEBUG();
    return self;
}

	

	/* the following two class methods allow us to export our 
	performOp:withOptions: method so it can be called from JavaScript
	using the name PerformOperation(opname,options). */

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector {
	PDEBUG(@"selector = '%@'", NSStringFromSelector(selector));
    if ( selector == @selector(getCurrentSpeed) ) {
        return NO;
    } else if ( selector == @selector(setCurrentSpeed:) ) {
        return NO;
    } else if ( selector == @selector(getCurrentCurvature) ) {
        return NO;
    } else if ( selector == @selector(setCurrentCurvature:) ) {
        return NO;
    } else if ( selector == @selector(getCurrentTicks) ) {
        return NO;
    } else if ( selector == @selector(setCurrentTicks:) ) {
        return NO;
    } else if ( selector == @selector(setSpeed:Ticks:AndCurvature:) ) {
        return NO;
    }
    return YES;
}

+ (NSString *)webScriptNameForSelector:(SEL)aSelector {
	PDEBUG(@"aSelector = '%@'", NSStringFromSelector(aSelector));
    if ( aSelector == @selector(setCurrentSpeed:) ) {
        return @"SetSpeed";
    } else if ( aSelector == @selector(getCurrentSpeed) ) {
        return @"GetSpeed";
    } else if ( aSelector == @selector(getCurrentCurvature) ) {
        return @"GetCurvature";
    } else if ( aSelector == @selector(setCurrentCurvature:) ) {
        return @"SetCurvature";
    }  else if ( aSelector == @selector(getCurrentTicks) ) {
        return @"GetTicks";
    } else if ( aSelector == @selector(setCurrentTicks:) ) {
        return @"SetTicks";
    } else if ( aSelector == @selector(setSpeed:Ticks:AndCurvature:) ) {
        return @"SetSpeedTicksAndCurve";
    } else {
		return nil;
	}
}

- (NSNumber*)getCurrentSpeed {
	PDEBUG();
	return [NSNumber numberWithFloat: [self speed]];
}

- (void)setCurrentSpeed: (NSNumber*) newSpeed {
	PDEBUG();
	[self setSpeed:[newSpeed floatValue]];
}

- (NSNumber*)getCurrentCurvature {
	PDEBUG();
	return [NSNumber numberWithFloat: [self curvature]];
}

- (void)setCurrentCurvature: (NSNumber*) newCurvature {
	PDEBUG();
	[self setCurvature:[newCurvature floatValue]];
}
- (NSNumber*)getCurrentTicks {
	PDEBUG();
	return [NSNumber numberWithInt: [self ticks]];
}

- (void)setCurrentTicks: (NSNumber*) newTicks {
	PDEBUG();
	[self setTicks:[newTicks intValue]];
}

- (void) setSpeed: (NSNumber*) newSpeed Ticks: (NSNumber*) newTicks
	AndCurvature: (NSNumber*) newCurvature {
	PDEBUG();
	[self setSpeed:[newSpeed floatValue]];
	[self setTicks:[newTicks intValue]];
	[self setCurvature:[newCurvature floatValue]];
}





	/* sharing instance variables between our class and JavaScripts running
	in the containing page */


	/* the following two class methods allow us to export our 
	timerTaskEnabled and timerTaskCounter instance variables so they
	can be accessed from JavaScript. */
	
+ (BOOL)isKeyExcludedFromWebScript:(const char *)property {
	PDEBUG(@"property = '%s'", self, NSStringFromSelector(_cmd), property);
    if (strcmp(property, "title") == 0) {
        return NO;
    } else if (strcmp(property, "counter") == 0) {
        return NO;
    }
    return YES;
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	PDEBUG(@"name = '%s'", self, NSStringFromSelector(_cmd), name);
    if (strcmp(name, "counter") == 0) {
	
        return @"total";
		
    } else {
	
		return nil;
		
	}
}




@end
