/*

File: MyQTMovie.m

Abstract: Code to manage movie playback notifications during movie
			playback

Version: <1.0>

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Computer,
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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "MyQTMovie.h"
#import "QTMovieExtensions.h"


@implementation MyQTMovie

- (id)init 
{
    self = [super init];
    if (self==nil) return nil;
    
	playbackSelector			= nil;
	movieStoppedSelector		= nil;
	movieDidEndSelector			= nil;
	mcActionFilterCallBack		= nil;
	movieDidEndTargetObject		= nil;
	playbackTargetObject		= nil;
	movieStoppedTargetObject	= nil;
	    
    return self; 
}

-(void)dealloc
{
	MovieController mc = [self quickTimeMovieController];
	if (mc)
	{
		// remove existing registered callback
		MCSetActionFilterWithRefCon(mc,nil,0);
	}
	
	if (mcActionFilterCallBack)
	{
		// toss UPP for callback
		DisposeMCActionFilterWithRefConUPP(mcActionFilterCallBack);
	}

	// remove all notification associations
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc removeObserver:self];

	[super dealloc];
}

#pragma mark *** Movie Callback routines ***

	// removes the movie-end, movie rate-change notifications as
	// well as the movie controller action filter
-(void)removeAllMovieCallbacks
{
	[self removeMovieDidEndNotification];
	[self removeMovieRateChangeNotification];
	[self removeMCActionFilter];
}


#pragma mark *** movieDidEnd ***

	// install the QTMovieDidEndNotification notification
-(void)installMovieDidEndNotification
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(movieDidEnd:) 
            name:@"QTMovieDidEndNotification" object:nil];
}

	// specify the selector to be called when the QTMovieDidEndNotification
	// notification arrives
-(void)setMovieDidEndNotificationCallback:(SEL)aSelector withObject:(id)anObject
{
	if (!movieDidEndSelector || !movieDidEndTargetObject)
	{
		// install the notification
		[self installMovieDidEndNotification];
	}
	
	[self setMovieDidEndSelector:aSelector withObject:anObject];
}

	// delete the QTMovieDidEndNotification notification
-(void)removeMovieDidEndNotification
{
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc removeObserver:self name:@"QTMovieDidEndNotification" object:nil];
	
	movieDidEndSelector = nil;
	movieDidEndTargetObject = nil;
}

	// call the selector specified for movie-end
-(void)callMovieDidEndSelector
{
	if (movieDidEndTargetObject && movieDidEndSelector)
	{
		[movieDidEndTargetObject performSelector:movieDidEndSelector withObject:self];
	}
}

	// set the selector to be called for for movie-end notifications
-(void)setMovieDidEndSelector:(SEL)aSelector withObject:(id)anObject
{
	movieDidEndSelector = aSelector;
	movieDidEndTargetObject = anObject;
}

	// movieDidEnd is called for QTMovieDidEndNotification notifications.
	// this, in turn, will call the specified selector
- (void)movieDidEnd:(NSNotification *)notification
{
	[self callMovieDidEndSelector];
}


#pragma mark *** rateChange ***

	// install the QTMovieRateDidChangeNotification notification
-(void)installRateChangeNotification
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(movieRateChanged:) 
            name:@"QTMovieRateDidChangeNotification" object:nil];
}

	// specify the selector to be called when the QTMovieRateDidChangeNotification
	// notification arrives
-(void)setMovieRateChangeNotificationCallback:(SEL)aSelector withObject:(id)anObject
{
	if (!movieStoppedSelector || !movieStoppedTargetObject)
	{
		[self installRateChangeNotification];
	}
	
	[self setMovieStoppedSelector:aSelector withObject:anObject];
}

	// remove the QTMovieRateDidChangeNotification notification
-(void)removeMovieRateChangeNotification
{
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc removeObserver:self name:@"QTMovieRateDidChangeNotification" object:nil];
	
	movieStoppedSelector = nil;
	movieStoppedTargetObject = nil;
}

	// call the selector specified for movie stopped 
	// (QTMovieRateDidChangeNotification) notifications
-(void)callMovieStoppedSelector
{
	if (movieStoppedTargetObject && movieStoppedSelector)
	{
		[movieStoppedTargetObject performSelector:movieStoppedSelector withObject:self];
	}
}

	// set the selector to be called for movie stopped
	// notifications
-(void)setMovieStoppedSelector:(SEL)aSelector withObject:(id)anObject
{
	movieStoppedSelector = aSelector;
	movieStoppedTargetObject = anObject;
}

	// movieRateChanged is called for QTMovieRateDidChangeNotification
	// notifications -- then the user specified routine is called
- (void)movieRateChanged:(NSNotification *)notification
{
	NSDictionary *userInfo = [notification userInfo];
	
	NSNumber *newRate = [userInfo objectForKey:QTMovieRateDidChangeNotificationParameter];

	if ([newRate intValue] == 0)
	{
		[self callMovieStoppedSelector];
	}	
}



#pragma mark *** playback notification ***

	// set an action filter function for the movie controller
-(void)installMCActionFilter
{
	if (!mcActionFilterCallBack)
	{
		mcActionFilterCallBack =  NewMCActionFilterWithRefConUPP(&MyMCActionFilterCallBack);
	}
	
	if (!mcActionFilterCallBack) goto bail;
	
	ComponentResult	cr = MCSetActionFilterWithRefCon([self quickTimeMovieController],
										 mcActionFilterCallBack,
										 (long)self);
	if (cr != noErr)
	{
		DisposeMCActionFilterWithRefConUPP(mcActionFilterCallBack);
		mcActionFilterCallBack = nil;
	}
	
bail:
	return;
}

	// specify the selector to be called during playback
-(void)setMoviePlaybackNotificationCallback:(SEL)aSelector withObject:(id)anObject
{
	if (!playbackTargetObject || !playbackSelector)
	{
		[self installMCActionFilter];
	}
	
	[self setPlaybackSelector:aSelector withObject:anObject];
}

	// remove our movie controller action filter
-(void)removeMCActionFilter
{
	// remove existing callback by passing nil for the
	// callback parameter
	MCSetActionFilterWithRefCon([self quickTimeMovieController],
													 nil,
													 (long)self);
	
	DisposeMCActionFilterWithRefConUPP(mcActionFilterCallBack);
	mcActionFilterCallBack = nil;

	playbackTargetObject = nil;
	playbackSelector = nil;
}

	// call the specified playback selector
-(void)callPlaybackSelector
{
	if (playbackTargetObject && playbackSelector)
	{
		[playbackTargetObject performSelector:playbackSelector withObject:self];
	}
}

	// set the selector to be called during playback
-(void)setPlaybackSelector:(SEL)aSelector withObject:(id)anObject
{
	playbackSelector = aSelector;
	playbackTargetObject = anObject;
}


@end


 // our movie controller action filter function
pascal Boolean MyMCActionFilterCallBack (MovieController    mc,
										 short              action,
										 void               *params,
										 long               refCon )
{
#pragma unused(mc, params)

	Boolean        isHandled = false;      // false => allow controller to process the action
  
	switch(action)
	{
		case mcActionIdle:
			// refCon = our Controller object
			// Call the playback selector
			[(MyQTMovie *)refCon callPlaybackSelector];
	     break;

		 default:
		 break;
	}
	
	return (isHandled);
}