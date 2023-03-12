/*

File: MovieEntry.m

Author: QuickTime DTS

Change History (most recent first):
        
      <1> 07/15/07 initial release
      
© Copyright 2007 Apple Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#import "MovieEntry.h"

// there's better ways to do this but to keep this sample simple
// we only perform 4 exports and set the max progress control value
// to 4 -- we use to this keep track of when we're done with this
// entry in the table
static float kMaxExportValue = 4.0;

@implementation MovieEntry

//---------------------------------------------------------- 
// init
//---------------------------------------------------------- 
- (id)init
{
	[super init];
    [self setImage:nil];
    [self setFullPath:nil];
    [self setProgressState:0.0];
    return self;
}

//---------------------------------------------------------- 
// dealloc
//---------------------------------------------------------- 
- (void)dealloc
{
    [self setFullPath:nil];
    [super dealloc];
}

//---------------------------------------------------------- 
// image 
//---------------------------------------------------------- 
- (NSImage *)image
{
	return [[mImage retain] autorelease]; 
}

- (void)setImage:(NSImage *)inImage
{
    if (mImage != inImage) {
    	[mImage release];
    	mImage = [inImage copy];
    }
}

//---------------------------------------------------------- 
//  fileName 
//---------------------------------------------------------- 
- (NSString *)fileName
{
    return [mFullPath lastPathComponent];
}

//---------------------------------------------------------- 
//  fullPath 
//---------------------------------------------------------- 
- (NSString *)fullPath
{
    return [[mFullPath retain] autorelease]; 
}

- (void)setFullPath:(NSString *)inFullPath
{
    if (mFullPath != inFullPath) {
        [mFullPath release];
        mFullPath = [inFullPath copy];
    }
}

//---------------------------------------------------------- 
//  progressState 
//---------------------------------------------------------- 
- (float)progressState
{
    return mProgressState;
}

- (void)setProgressState:(float)inProgressState
{
    mProgressState = inProgressState;
    
    if (mProgressState < kMaxExportValue) {
    	mDone = NO;
    } else {
    	mDone = YES;
    }
}

- (BOOL)done
{
	return mDone;
}

- (void)setDone:(BOOL)inDone
{
	mDone = inDone;
}

@end

// export item is used as a structure that
// holds references to the origial objects all the time
// therefore, we don't care about retain/release issues here
@implementation ExportItem

//---------------------------------------------------------- 
// init
//---------------------------------------------------------- 
- (id)init
{
	[super init];
    [self setMovie:nil];
    [self setEntry:nil];
    [self setDidSucceed:NO];
    return self;
}

//---------------------------------------------------------- 
//  movie 
//---------------------------------------------------------- 
- (QTMovie *)movie
{
    return mMovie; 
}

- (void)setMovie:(QTMovie *)inMovie
{
    mMovie = inMovie;
}

//---------------------------------------------------------- 
//  entry 
//---------------------------------------------------------- 
- (MovieEntry *)entry
{
    return mEntry;
}

- (void)setEntry:(MovieEntry *)inEntry
{
    mEntry = inEntry;
}

//---------------------------------------------------------- 
//  success 
//---------------------------------------------------------- 
- (BOOL)didSucceed
{
	return mDidSucceed;
}

- (void)setDidSucceed:(BOOL)inDidSucceed
{
	mDidSucceed = inDidSucceed;
}

@end