/*

File: FrameLiveDVExporter.m

Abstract: Implements the FrameLiveDVExporter class.

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

#import "FrameLiveDVExporter.h"

@implementation FrameLiveDVExporter

+ (NSSize) sizeForFormat:(DVFormat)format
{
	switch(format) {
		case kDVFormat_NTSC: return NSMakeSize(720, 480);
		case kDVFormat_PAL: return NSMakeSize(720, 576);
	}
	
	return NSZeroSize;
}

+ (float) framerateForFormat:(DVFormat)format
{
	switch(format) {
		case kDVFormat_NTSC: return 29.97;
		case kDVFormat_PAL: return 25.00;
	}
	
	return 0.0;
}

- (id) initWithCodec:(CodecType)codec pixelsWide:(unsigned)width pixelsHigh:(unsigned)height options:(ICMCompressionSessionOptionsRef)options
{
	//Make sure client goes through designated initializer
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}

- (id) initWithDVFormat:(DVFormat)format progressive:(BOOL)progressive wideScreen:(BOOL)wide
{
	ICMCompressionSessionOptionsRef	options = [[self class] defaultOptions];
	CodecType						codec = (format == kDVFormat_NTSC ? kDVCNTSCCodecType : kDVCPALCodecType);
	NSSize							size = [[self class] sizeForFormat:format];
	Rect							bounds = {0, 0, size.height, size.width};
	Boolean							boolean = true;
	CodecQ							quality = codecHighQuality;
	GWorldPtr						displayGWorld;
	OSErr							theError;
	Handle							settings;
	ComponentInstance				component;
	
	//Check parameters
	if((format != kDVFormat_NTSC) && (format != kDVFormat_PAL)) {
		[self release];
		return nil;
	}
	
	//Customize compression options
	ICMCompressionSessionOptionsSetProperty(options, kQTPropertyClass_ICMCompressionSessionOptions, kICMCompressionSessionOptionsPropertyID_AllowAsyncCompletion, sizeof(Boolean), &boolean);
	ICMCompressionSessionOptionsSetProperty(options, kQTPropertyClass_ICMCompressionSessionOptions, kICMCompressionSessionOptionsPropertyID_Quality, sizeof(CodecQ), &quality);
	
	//Retrieve extra options directly from DV compressor
	theError = OpenADefaultComponent(compressorComponentType, codec, &component);
	if(theError) {
		NSLog(@"DV component opening failed (error %i)", theError);
		[self release];
		return nil;
	}
	boolean = progressive;
	QTSetComponentProperty(component, kQTPropertyClass_DVCompressor, kDVCompressorPropertyID_ProgressiveScan, sizeof(Boolean), &boolean);
	boolean = wide;
	QTSetComponentProperty(component, kQTPropertyClass_DVCompressor, kDVCompressorPropertyID_AspectRatio16x9, sizeof(Boolean), &boolean);
	settings = NewHandle(0);
	if(settings == NULL) {
		NSLog(@"Memory allocation failed");
		CloseComponent(component);
		[self release];
		return nil;
	}
	theError = ImageCodecGetSettings(component, settings);
	CloseComponent(component);
	if(theError) {
		NSLog(@"ImageCodecGetSettings() failed with error %i", theError);
		DisposeHandle(settings);
		[self release];
		return nil;
	}
	ICMCompressionSessionOptionsSetProperty(options, kQTPropertyClass_ICMCompressionSessionOptions, kICMCompressionSessionOptionsPropertyID_CompressorSettings, sizeof(Handle), &settings);
	
	//Initialize super class
	if(self = [super initWithCodec:codec pixelsWide:size.width pixelsHigh:size.height options:options]) {
		//We don't need the settings anymore
		DisposeHandle(settings);
		
		//Create FireWire DV video out QuickTime component
		_displayImageDescription = (ImageDescriptionHandle) NewHandleClear(sizeof(ImageDescription));
		if(_displayImageDescription == NULL) {
			NSLog(@"Memory allocation failed");
			[self release];
			return nil;
		}
		(**_displayImageDescription).idSize = sizeof(ImageDescription);
		(**_displayImageDescription).cType = codec;
		(**_displayImageDescription).width = size.width;
		(**_displayImageDescription).height = size.height;
		(**_displayImageDescription).spatialQuality = codecNormalQuality;
		(**_displayImageDescription).depth = 24;
		(**_displayImageDescription).hRes = 72 << 16;
		(**_displayImageDescription).vRes = 72 << 16;
		(**_displayImageDescription).clutID = -1;
		theError = OpenADefaultComponent(QTVideoOutputComponentType, 'fire', &_displayComponent);
		if(theError == noErr)
		QTVideoOutputSetClientName(_displayComponent, (ConstStr255Param)"DV Exporter");
		theError = QTVideoOutputSetDisplayMode(_displayComponent, (format == kDVFormat_NTSC ? 1 : 2)); //NOTE: 1 = NTSC and 2 = PAL
		if(theError == noErr)
		theError = QTVideoOutputBegin(_displayComponent);
		if(theError == noErr)
		theError = QTVideoOutputGetGWorld(_displayComponent, &displayGWorld);
		if(theError == noErr)
		theError = DecompressSequenceBeginS(&_displayImageSequence, _displayImageDescription, nil, 0, displayGWorld, NULL, &bounds, nil, srcCopy, nil, 0, codecNormalQuality, anyCodec);
		if(theError) {
			NSLog(@"Video out component creation failed (error %i)", theError);
			[self release];
			return nil;
		}
	}
	else
	DisposeHandle(settings);
	
	return self; 
}

- (void) dealloc
{
	//Destroy resources
	if(_displayImageSequence)
	CDSequenceEnd(_displayImageSequence);
	if(_displayComponent) {
		QTVideoOutputEnd(_displayComponent);
		CloseComponent(_displayComponent);
	}
	if(_displayImageDescription)
	DisposeHandle((Handle)_displayImageDescription);
	
	[super dealloc];
}

- (void) doneCompressingFrame:(ICMEncodedFrameRef)frame
{
	CodecFlags						inFlags = 0;
    CodecFlags						outFlags = 0;
	OSErr							theError;
	
	//Send compressed frame data to FireWire DV
	theError = DecompressSequenceFrameS(_displayImageSequence, (Ptr)ICMEncodedFrameGetDataPtr(frame), ICMEncodedFrameGetDataSize(frame), inFlags, &outFlags, nil);
	if(theError)
	NSLog(@"DecompressSequenceFrameS() failed with error %i", theError);
	
	[super doneCompressingFrame:frame];
}

- (BOOL) exportFrame:(CVPixelBufferRef)frame
{
	return [super compressFrame:frame timeStamp:NAN duration:NAN];
}

@end
