/*

File: VideoChannel.m

Abstract:   The VideoChannel handles one movie with its navigation and rendering.

Version: 1.1

© Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/ 

#import "VideoChannel.h"
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import "LiveVideoMixerController.h"

@implementation VideoChannel

//--------------------------------------------------------------------------------------------------

+ (id)createWithFilePath:(NSString*)theFilePath forView:(VideoMixView*)inView
{
    return [[[VideoChannel alloc] initWithFilePath:theFilePath forView:inView] autorelease];
}

//--------------------------------------------------------------------------------------------------

- (id)initWithFilePath:(NSString*)theFilePath forView:(VideoMixView*)inView;
{
    self = [super init];
    
    OSStatus		theError = noErr;
    Boolean			active = TRUE;
    UInt32			trackCount = 0;
    OSType			theTrackType;
    Track			theTrack = nil;
    Media			theMedia = nil;

    targetRect = NSMakeRect(0.0, 0.0, kVideoWidth, kVideoHeight);
    QTNewMoviePropertyElement newMovieProperties[] = {  {kQTPropertyClass_DataLocation, kQTDataLocationPropertyID_CFStringNativePath, sizeof(theFilePath), &theFilePath, 0},
							{kQTPropertyClass_NewMovieProperty, kQTNewMoviePropertyID_Active, sizeof(active), &active, 0},
							{kQTPropertyClass_Context, kQTContextPropertyID_VisualContext, sizeof(visualContext), &visualContext, 0},
							};
    
    theError = QTOpenGLTextureContextCreate(nil, [[inView sharedContext] CGLContextObj],
						[[NSOpenGLView defaultPixelFormat] CGLPixelFormatObj],
						nil,
						&visualContext);
    if(visualContext == NULL) 
    {
		NSLog(@"QTVisualContext creation failed with error:%d", theError);
		[self dealloc];
		return nil;
    }
    theError = NewMovieFromProperties(sizeof(newMovieProperties) / sizeof(newMovieProperties[0]), newMovieProperties, 0, nil, &channelMovie);
    
    if(theError)
    {
		NSLog(@"NewMovieFromProperties failed with %d", theError);
		[self dealloc];
		return nil;
    }
	
    // setup the movie
    GoToBeginningOfMovie(channelMovie);
    SetTimeBaseFlags(GetMovieTimeBase(channelMovie), loopTimeBase);
	SetMoviePlayHints(channelMovie, hintsLoop, hintsLoop);
    trackCount = GetMovieTrackCount(channelMovie);
    while(trackCount > 0)
    {
	theTrack = GetMovieIndTrack(channelMovie, trackCount);
	if(theTrack != nil)
	{
	    theMedia = GetTrackMedia(theTrack);
	    if(theMedia != nil)
	    {			    
		GetMediaHandlerDescription(theMedia, &theTrackType, 0, 0);
		if(theTrackType != VideoMediaType)
		{
		    SetTrackEnabled(theTrack, false);		// disable all non video tracks
		}
	    }
	}
	trackCount--;
    }

    return self;
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
	if(visualContext)
		QTVisualContextRelease(visualContext);
	if(channelMovie)
		DisposeMovie(channelMovie);
    [super dealloc];
}

//--------------------------------------------------------------------------------------------------

- (void)prerollMovie:(Fixed)rate
{
    GoToBeginningOfMovie(channelMovie);
    MoviesTask(channelMovie, 0);
    PrerollMovie(channelMovie, 0, rate);
}

//--------------------------------------------------------------------------------------------------

- (void)startMovie:(Fixed)rate usingMasterTimeBase:(TimeBase)masterTimeBase
{
    if (NULL != masterTimeBase) 
    {
	// set the movie to run from the master timebase unless it is the same as its own
	TimeBase selfTimeBase = [self timeBase];
	if (masterTimeBase != selfTimeBase)
		SetTimeBaseMasterTimeBase(selfTimeBase, masterTimeBase, nil);
    }
    SetMovieRate(channelMovie, rate);	// this starts the playback of the movie
}

//--------------------------------------------------------------------------------------------------

- (void)stopMovie
{
    StopMovie(channelMovie);
}

//--------------------------------------------------------------------------------------------------

- (TimeBase)timeBase
{
    return GetMovieTimeBase(channelMovie);
}

//--------------------------------------------------------------------------------------------------

- (void)_setCurrentTextureRef:(CVOpenGLTextureRef)inTextureRef
{
    CVOpenGLTextureRelease(currentTexture);
    currentTexture = inTextureRef;
    //get the clean aperture texture coordinates
    CVOpenGLTextureGetCleanTexCoords(currentTexture, lowerLeft, lowerRight, upperRight, upperLeft);
}

//--------------------------------------------------------------------------------------------------

- (BOOL)renderChannel:(const CVTimeStamp*)syncTimeStamp
{
    CVOpenGLTextureRef		newTextureRef = nil;
    
    QTVisualContextTask(visualContext);
    if(QTVisualContextIsNewImageAvailable(visualContext, syncTimeStamp))
    {
		QTVisualContextCopyImageForTime(visualContext, nil, syncTimeStamp, &newTextureRef);
		[self _setCurrentTextureRef:newTextureRef];
		return YES; // we got a frame from QT
    } else {
		//NSLog(@"No Frame ready %@", self);
    }
    return NO;	// no frame ready
}

//--------------------------------------------------------------------------------------------------

- (float)opacity
{
    return [channelOpacity floatValue] / 100.0;	// the slider ranges from zero to 100 while alpha is normalize (0.0 to 1.0)
}

//--------------------------------------------------------------------------------------------------

- (void)setTargetRect:(NSRect)inRect
{
    targetRect = inRect;
}

//--------------------------------------------------------------------------------------------------

- (NSRect)targetRect
{
    return targetRect;
}

//--------------------------------------------------------------------------------------------------

- (void)drawOutline:(NSRect)destRect

{
    glLoadIdentity();
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_EXT);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_EXT);
    glDisable(GL_TEXTURE_2D);
    glPushAttrib(GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glMatrixMode(GL_TEXTURE);
    glLineWidth(1);
    glColor4f(1, 0, 0, 1);
    glBegin(GL_QUADS);            
	    glVertex2f(destRect.origin.x, destRect.origin.y);
	    glVertex2f(destRect.origin.x + destRect.size.width, destRect.origin.y);
	    glVertex2f(destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
	    glVertex2f(destRect.origin.x, destRect.origin.y + destRect.size.height);
    glEnd();
    glPopAttrib();
}

//--------------------------------------------------------------------------------------------------

- (void)compositeChannelInRect:(NSRect)destRect;

{
    float				opacity = [self opacity];
    
    if(!channelShape)
    {
		glEnable(CVOpenGLTextureGetTarget(currentTexture));
		glBindTexture(CVOpenGLTextureGetTarget(currentTexture), CVOpenGLTextureGetName(currentTexture));
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glColor4f(1, 1, 1, opacity);
		glBegin(GL_QUADS);            
			glTexCoord2f(upperLeft[0], upperLeft[1]);
			glVertex2f  (destRect.origin.x, destRect.origin.y);
			glTexCoord2f(upperRight[0], upperRight[1]);
			glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y);
			glTexCoord2f(lowerRight[0], lowerRight[1]);
			glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
			glTexCoord2f(lowerLeft[0], lowerLeft[1]);
			glVertex2f  (destRect.origin.x, destRect.origin.y + destRect.size.height);
		glEnd();
    } else {
		// setup the matte texture as a mask in texture unit 1
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(kTextureTarget);
		glBindTexture(kTextureTarget, gTexNames[[channelShape intValue]]);	
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		// setup the image texture in texture unit 0
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(CVOpenGLTextureGetTarget(currentTexture));
		glBindTexture(CVOpenGLTextureGetTarget(currentTexture), CVOpenGLTextureGetName(currentTexture));
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		// render both textures together
		glColor4f(1, 1, 1, opacity);
		glBegin(GL_QUADS);            
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, upperLeft[0], upperLeft[1]);
			glVertex2f  ( destRect.origin.x, destRect.origin.y);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, kVideoWidth, 0);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, upperRight[0], upperRight[1]);
			glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, kVideoWidth, kVideoHeight);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, lowerRight[0], lowerRight[1]);
			glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, kVideoHeight);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, lowerLeft[0], lowerLeft[1]);
			glVertex2f  ( destRect.origin.x, destRect.origin.y + destRect.size.height);
		glEnd();	
		glActiveTextureARB(GL_TEXTURE1_ARB);    // make sure the texture unit 1 is now disabled 
		glDisable(GL_TEXTURE_RECTANGLE_EXT);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		if([channelShape intValue] == kSpeechBubble)
		{
			glEnable(kTextureTarget);
			glBindTexture(kTextureTarget, gTexNames[kSpeechBubbleTextureID]);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glColor4f(1, 1, 1, 1);
			glBegin(GL_QUADS);            
				glTexCoord2f( 0, 0);
				glVertex2f  ( destRect.origin.x, destRect.origin.y);
				glTexCoord2f( kVideoWidth, 0);
				glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y);
				glTexCoord2f( kVideoWidth, kVideoHeight);
				glVertex2f  ( destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
				glTexCoord2f( 0, kVideoHeight);
				glVertex2f  ( destRect.origin.x, destRect.origin.y + destRect.size.height);
			glEnd();
		}
    }
	
}

//--------------------------------------------------------------------------------------------------

- (void)compositeChannelThumbnailInRect:(NSRect)destRect
{
    glEnable(CVOpenGLTextureGetTarget(currentTexture));
    glBindTexture(CVOpenGLTextureGetTarget(currentTexture), CVOpenGLTextureGetName(currentTexture));
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);            
	    glTexCoord2f(upperLeft[0], upperLeft[1]);
	    glVertex2f  (destRect.origin.x, destRect.origin.y);
	    glTexCoord2f(upperRight[0], upperRight[1]);
	    glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y);
	    glTexCoord2f(lowerRight[0], lowerRight[1]);
	    glVertex2f  (destRect.origin.x + destRect.size.width, destRect.origin.y + destRect.size.height);
	    glTexCoord2f(lowerLeft[0], lowerLeft[1]);
	    glVertex2f  (destRect.origin.x, destRect.origin.y + destRect.size.height);
    glEnd();
}

//--------------------------------------------------------------------------------------------------

@end
