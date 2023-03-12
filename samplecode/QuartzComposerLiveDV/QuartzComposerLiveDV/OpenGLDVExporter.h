/*

File: OpenGLDVExporter.h

Abstract: Declares the interface for the OpenGLDVExporter class (a subclass
of OpenGLQTCompressor) which allows to grab frames from an OpenGL context
and send them as a real-time DV stream over the FireWire port.
The OpenGLDVExporter is initialized with an OpenGL context to read from
and the DV format to use (NTSC or PAL). Refer to the OpenGLQTCompressor.h
file for a description of the "flipVertically" and "useTextures" parameters.
The Frame grabbing and DV exporting from the OpenGL context is simply
performed by calling -exportFrame:. The method +sizeForFormat returns the
size of the frames for a given DV format and +framerateForFormat returns
the recommended framerate at which -exportFrame: should be called.

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

#import <Cocoa/Cocoa.h>
#import <QuickTime/QuickTime.h>

#import "OpenGLQTCompressor.h"

typedef enum {
	kDVFormat_NTSC,
	kDVFormat_PAL
} DVFormat;

@interface OpenGLDVExporter : OpenGLQTCompressor
{
	ImageSequence			_displayImageSequence;
	PixMapHandle			_displayPixMap;
	ImageDescriptionHandle	_displayImageDescription;
	ComponentInstance		_displayComponent;
}
+ (NSSize) sizeForFormat:(DVFormat)format;
+ (float) framerateForFormat:(DVFormat)format;

- (id) initWithOpenGLContext:(NSOpenGLContext*)context dvFormat:(DVFormat)format flipVertically:(BOOL)flip useTextures:(BOOL)textures;
- (BOOL) exportFrame;
@end
