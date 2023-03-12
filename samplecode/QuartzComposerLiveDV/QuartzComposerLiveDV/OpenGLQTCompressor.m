/*

File: OpenGLQTCompressor.m

Abstract: Implements the OpenGLQTCompressor class.

Version: 1.0

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

#import <OpenGL/CGLMacro.h>

#import "OpenGLQTCompressor.h"

@implementation OpenGLQTCompressor

+ (void) initialize
{
	//Make sure QuickTime is initialized
	EnterMovies();
}

- (id) init
{
	//Make sure client goes through designated initializer
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}

- (id) initWithOpenGLContext:(NSOpenGLContext*)context pixelsWide:(unsigned)width pixelsHigh:(unsigned)height codec:(CodecType)codec quality:(CodecQ)quality flipVertically:(BOOL)flip useTextures:(BOOL)textures
{
	//IMPORTANT: We use the macros provided by <OpenGL/CGLMacro.h> which provide better performances and allows us not to bother with making sure the current context is valid
	CGLContextObj					cgl_ctx = [context CGLContextObj];
	GLint							save1,
									save2,
									save3,
									save4;
	OSErr							theError;
	
	//Check parameters
	if((context == nil) || ((width == 0) || (height == 0))) {
		[self release];
		return nil;
	}
	
	if(self = [super init]) {
		//Makes sure the OpenGL context stays around
		_glContext = [context retain];
		
		//Compute frame bounds
		_frameBounds.left = 0;
		_frameBounds.top = 0;
		_frameBounds.right = width;
		_frameBounds.bottom = height;
		
		//Create memory buffers - Make sure the buffer is paged-aligned and rowbytes is a multiple of 64 for performance reasons
		_bufferRowBytes = (_frameBounds.right * 4 + 63) & ~63;
		if(flip)
		_flippedFrameBuffer = valloc(_frameBounds.bottom * _bufferRowBytes);
		if(textures) {
			_frameBuffers[0] = valloc(_frameBounds.bottom * _bufferRowBytes);
			_frameBuffers[1] = valloc(_frameBounds.bottom * _bufferRowBytes);
		}
		else
		_frameBuffer = valloc(_frameBounds.bottom * _bufferRowBytes);
		if((flip && !_flippedFrameBuffer) || (textures && (!_frameBuffers[0] || !_frameBuffers[1])) || (!textures && !_frameBuffer)) {
			NSLog(@"Memory allocation failed");
			[self release];
			return nil;
		}
		
		//Create OpenGL textures - For extra safety, save & restore OpenGL states that are changed
		if(textures) {
			//Create and configure first texture
			glGenTextures(1, &_textureNames[0]);
			glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE_EXT, &save1);
			glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _textureNames[0]);
			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE);
			glGetIntegerv(GL_UNPACK_ALIGNMENT, &save2);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glGetIntegerv(GL_UNPACK_ROW_LENGTH, &save3);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, _bufferRowBytes / 4);
			glGetIntegerv(GL_UNPACK_CLIENT_STORAGE_APPLE, &save4);
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
			glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, _frameBounds.right, _frameBounds.bottom, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _frameBuffers[0]);
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, save4);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, save3);
			glPixelStorei(GL_UNPACK_ALIGNMENT, save2);
			glBindTexture(GL_TEXTURE_RECTANGLE_EXT, save1);
			
			//Create and configure second texture
			glGenTextures(1, &_textureNames[1]);
			glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE_EXT, &save1);
			glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _textureNames[1]);
			glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE);
			glGetIntegerv(GL_UNPACK_ALIGNMENT, &save2);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glGetIntegerv(GL_UNPACK_ROW_LENGTH, &save3);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, _bufferRowBytes / 4);
			glGetIntegerv(GL_UNPACK_CLIENT_STORAGE_APPLE, &save4);
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
			glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, _frameBounds.right, _frameBounds.bottom, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _frameBuffers[1]);
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, save4);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, save3);
			glPixelStorei(GL_UNPACK_ALIGNMENT, save2);
			glBindTexture(GL_TEXTURE_RECTANGLE_EXT, save1);
			
			//Check for OpenGL errors
			theError = glGetError();
			if(theError) {
				NSLog(@"OpenGL texture creation failed (error 0x%04X)", theError);
				[self release];
				return nil;
			}
		}
		
		//Create QuickTime compression sequence
		_compressorPixMap = NewPixMap();
		if(_compressorPixMap == NULL) {
			NSLog(@"Memory allocation failed");
			[self release];
			return nil;
		}
		(**_compressorPixMap).baseAddr = _flippedFrameBuffer;
		(**_compressorPixMap).rowBytes = _bufferRowBytes;
		(**_compressorPixMap).bounds = _frameBounds;
		(**_compressorPixMap).pixelType = RGBDirect;
		(**_compressorPixMap).pixelSize = 32;
		(**_compressorPixMap).cmpCount = 4;
		(**_compressorPixMap).cmpSize = 8;
		(**_compressorPixMap).pixelFormat = k32ARGBPixelFormat;
		_compressorImageDescription = (ImageDescriptionHandle) NewHandleClear(sizeof(ImageDescription));
		theError = CompressSequenceBegin(&_compressorImageSequence, _compressorPixMap, 0, &_frameBounds, NULL, 32, codec, bestFidelityCodec, quality, codecMinQuality, 1, NULL, 0, _compressorImageDescription);
		if(theError == noErr)
		theError = GetMaxCompressionSize(_compressorPixMap, &_frameBounds, 32, quality, codec, bestFidelityCodec, &_compressorBufferSize);
		if(theError == noErr) {
			_compressorBuffer = malloc(_compressorBufferSize);
			if(_compressorBuffer == NULL) {
				NSLog(@"Memory allocation failed");
				[self release];
				return nil;
			}
		}
		if(theError) {
			NSLog(@"Compression sequence creation failed (error %i)", theError);
			[self release];
			return nil;
		}
		
		//If we use textures, we need to skip the first frame which will not contain anything
		_skipFirstFrame = textures;
	}
	
	return self;
}

- (ImageDescriptionHandle) framesImageDescription
{
	return _compressorImageDescription;
}

- (void) dealloc
{
	//IMPORTANT: We use the macros provided by <OpenGL/CGLMacro.h> which provide better performances and allows us not to bother with making sure the current context is valid
	CGLContextObj					cgl_ctx = [_glContext CGLContextObj];
	
	//Destroy resources
	if(_compressorImageSequence)
	CDSequenceEnd(_compressorImageSequence);
	if(_compressorPixMap)
	DisposePixMap(_compressorPixMap);
	if(_compressorImageDescription)
	DisposeHandle((Handle)_compressorImageDescription);
	if(_compressorBuffer)
	free(_compressorBuffer);
	if(_frameBuffer)
	free(_frameBuffer);
	if(_flippedFrameBuffer)
	free(_flippedFrameBuffer);
	if(_frameBuffers[0])
	free(_frameBuffers[0]);
	if(_textureNames[0])
	glDeleteTextures(1, &_textureNames[0]);
	if(_frameBuffers[1])
	free(_frameBuffers[1]);
	if(_textureNames[1])
	glDeleteTextures(1, &_textureNames[1]);
	
	//Release context
	[_glContext release];
	
	[super dealloc];
}

- (NSData*) compressFrame:(BOOL)noCopy
{
	//IMPORTANT: We use the macros provided by <OpenGL/CGLMacro.h> which provide better performances and allows us not to bother with making sure the current context is valid
	CGLContextObj					cgl_ctx = [_glContext CGLContextObj];
	long							size = _compressorBufferSize;
	GLint							save1,
									save2,
									save3;
	unsigned char*					src;
    unsigned char*					dst;
	unsigned						i;
	UInt8							similarity;
	OSErr							theError;
	
	//Get image from OpenGL context
	if(_frameBuffer) {
		//Read OpenGL context pixels directly
		glGetIntegerv(GL_PACK_ALIGNMENT, &save1);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glGetIntegerv(GL_PACK_ROW_LENGTH, &save2);
		glPixelStorei(GL_PACK_ROW_LENGTH, _bufferRowBytes / 4);
		glReadPixels(0, 0, _frameBounds.right, _frameBounds.bottom, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _frameBuffer);
		glPixelStorei(GL_PACK_ROW_LENGTH, save2);
		glPixelStorei(GL_PACK_ALIGNMENT, save1);
	}
	else {
		//Copy OpenGL context pixels to non-current texture
		glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE_EXT, &save1);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _textureNames[1 - _currentTextureIndex]);
		glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 0, 0, _frameBounds.right, _frameBounds.bottom);
		
		//Read pixels from current texture
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _textureNames[_currentTextureIndex]);
		glGetIntegerv(GL_PACK_ALIGNMENT, &save2);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glGetIntegerv(GL_PACK_ROW_LENGTH, &save3);
		glPixelStorei(GL_PACK_ROW_LENGTH, _bufferRowBytes / 4);
		glGetTexImage(GL_TEXTURE_RECTANGLE_EXT, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _frameBuffers[_currentTextureIndex]);
		glPixelStorei(GL_PACK_ROW_LENGTH, save3);
		glPixelStorei(GL_PACK_ALIGNMENT, save2);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, save1);
	}
	
	//Check for OpenGL errors
	theError = glGetError();
	if(theError) {
		NSLog(@"OpenGL pixels read failed (error 0x%04X)", theError);
		return nil;
	}
	
	//Flip image vertically - OpenGL copies pixels upside-down
	if(_flippedFrameBuffer) {
		if(_frameBuffer) {
			for(i = 0; i < _frameBounds.bottom; ++i) {
				src = _frameBuffer + _bufferRowBytes * i;
				dst = _flippedFrameBuffer + _bufferRowBytes * (_frameBounds.bottom - 1 - i);
				bcopy(src, dst, _frameBounds.right * 4);
			}
		}
		else {
			for(i = 0; i < _frameBounds.bottom; ++i) {
				src = _frameBuffers[_currentTextureIndex] + _bufferRowBytes * i;
				dst = _flippedFrameBuffer + _bufferRowBytes * (_frameBounds.bottom - 1 - i);
				bcopy(src, dst, _frameBounds.right * 4);
			}
		}
	}
	
	//Update pixmap base address if necessary
	if(!_flippedFrameBuffer)
	(**_compressorPixMap).baseAddr = (_frameBuffer ? _frameBuffer : _frameBuffers[_currentTextureIndex]);
	
	//Toggle current texture index if necesary
	if(!_frameBuffer)
	_currentTextureIndex = 1 - _currentTextureIndex;
	
	//Compress image as data
	if(_skipFirstFrame) {
		size = 0;
		_skipFirstFrame = NO;
	}
	else {
		theError = CompressSequenceFrame(_compressorImageSequence, _compressorPixMap, &_frameBounds, codecFlagUpdatePrevious, _compressorBuffer, &size, &similarity, NULL);
		if(theError) {
			NSLog(@"CompressSequenceFrame() failed with error %i", theError);
			return nil;
		}
	}
	
	return (noCopy ? [NSData dataWithBytesNoCopy:_compressorBuffer length:size freeWhenDone:NO] : [NSData dataWithBytes:_compressorBuffer length:size]);
}

@end
