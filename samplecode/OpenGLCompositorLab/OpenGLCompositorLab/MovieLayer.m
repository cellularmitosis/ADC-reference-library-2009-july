/*
	MovieLayer.m
	WWDC Open Compositor Lab

	Created by kdyke on Mon Oct 22 2001.

	Copyright:	Copyright © 2002-2003 Apple Computer, Inc., All Rights Reserved

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.
	
				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

#import "MovieLayer.h"
#import "Controller.h"
#import <QuickTime/QuickTimeComponents.h>
#import <OpenGL/gl.h>
#import <sys/time.h>
#import <stdlib.h>
#import <unistd.h>
#import "OpenGLCodecs.h"

@implementation MovieLayer

- (id)initWithFilename: (NSString *)filename
{
        NSURL  *movieURL;

	[super init];
        
        movieURL = [NSURL fileURLWithPath: filename];

        name = [[filename lastPathComponent] retain];
        
	movie = [[NSMovie alloc] initWithURL:movieURL byReference:YES];

	if(movie)
	{				
		qtMovie = [movie QTMovie];
		
		GetMovieBox(qtMovie,&movieRect);
		OffsetRect(&movieRect, -movieRect.left, -movieRect.top);
		
		SetMovieBox(qtMovie,&movieRect);
                SetMovieVolume(qtMovie, 0);
	
		SetMoviePlayHints(qtMovie, hintsHighQuality, hintsHighQuality);
		
		colorTextureWidth = movieRect.right;
		colorTextureHeight = movieRect.bottom;
		
		[self setRect:NSMakeRect(movieRect.left,movieRect.top,
		                       movieRect.right,movieRect.bottom)];
		[self updateLayerInfo];
		
		/* Create "custom" OpenGL GWorld to attach movie to. */
		QTNewGWorldFromPtr(&movieGWorld,
				   kOpenGLPixelFormat,
				   &movieRect,
				   NULL,
				   NULL,
				   0,
				   self, /* Magic */
				   32);
		
		SetMovieGWorld(qtMovie,(CGrafPtr)movieGWorld, NULL);		
		
		GoToBeginningOfMovie(qtMovie);
		UpdateMovie(qtMovie);
		MoviesTask (qtMovie, 0L); 
		
		StartMovie (qtMovie);
		SetMovieActive(qtMovie, true);
	}	
	else
	{
		[self release];
		return nil;
	}	

	return self;
}

- (void)dealloc
{
        if (alphaTextureData)
		free(alphaTextureData);
	[super dealloc];
}

- (void)heartbeat
{
	if(IsMovieDone(qtMovie)) {
		GoToBeginningOfMovie(qtMovie);
		StartMovie(qtMovie);
	}
	MoviesTask(qtMovie, 0);

}

@end
