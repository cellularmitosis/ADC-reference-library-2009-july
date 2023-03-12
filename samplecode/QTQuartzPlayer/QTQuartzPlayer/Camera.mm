/*

File: Camera.mm

Abstract: Implemtation of the Camera Class wrapping OpenGL camera positions

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

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

Revision History:
	<1>	08/08/2005  initial release

*/

#import "Camera.h"

@implementation Camera

// class method to create a Camera object
+(Camera*)camera
{
	return [[[self alloc] init] autorelease];
}

// initialize the Camara with the default coordinates
-(id)init
{
	[super init];	
	_start.z = _end.z = -kCameraDefaultDistance;
	return self;
}

// NSCoder protocal method called when camera data is being loaded back from disk
-(id)initWithCoder:(NSCoder*)coder
{
	[super init];
	_start.x = [coder decodeDoubleForKey:@"X"];
	_start.y = [coder decodeDoubleForKey:@"Y"];
	_start.z = [coder decodeDoubleForKey:@"Z"];
	_start.longitude = [coder decodeDoubleForKey:@"Longitude"];
	_start.latitude = [coder decodeDoubleForKey:@"Latitude"];
	_end = _start;
	return self;
}

// returns orientation values for the camera 
-(Orientation)orientationForTime:(const CVTimeStamp*)timeStamp
{
	double t = [self isAnimatedForTime:timeStamp]
	           ? double(timeStamp->hostTime - _startTime) / (_endTime - _startTime)
	           : timeStamp->hostTime < _startTime ? 0 : 1;
	Orientation orientation =
	{
		_start.x + (_end.x - _start.x) * t,
		_start.y + (_end.y - _start.y) * t,
		_start.z + (_end.z - _start.z) * t,
		_start.longitude + (_end.longitude - _start.longitude) * t,
		_start.latitude  + (_end.latitude - _start.latitude) * t,
	};
	return orientation;
}

// creates a new translated orientation
-(void)lookForTime:(const CVTimeStamp*)timeStamp
{
	Orientation orientation = [self orientationForTime:timeStamp];
	glTranslated(orientation.x, orientation.y, orientation.z);
	glRotated(orientation.longitude, 0, 1, 0);
	glRotated(orientation.latitude,  1, 0, 0);
}

// handles panning
-(void)panX:(double)x Y:(double)y Z:(double)z forTime:(const CVTimeStamp*)timeStamp
{
	_end = [self orientationForTime:timeStamp];
	_end.x += x;
	_end.y += y;
	_end.z = MIN(-1.5, _end.z + z);
	_endTime = timeStamp->hostTime;
}

// handles rotation
-(void)rotateX:(double)x Y:(double)y forTime:(const CVTimeStamp*)timeStamp
{
	_end = [self orientationForTime:timeStamp];
	_end.longitude += x;
	_end.latitude  += y;
	_endTime = timeStamp->hostTime;
}

// sets up camera orientations from a start time with a duration
-(void)animateTo:(Camera*)camera over:(double)duration
{
	CVTimeStamp now;
	now.hostTime = UInt64(AudioGetCurrentHostTime());	
	_start = [self orientationForTime:&now];
	_end = [camera orientationForTime:&now];
	_startTime = now.hostTime;
	_endTime = now.hostTime + UInt64(AudioGetHostClockFrequency() * duration);
}

// NSCoder protocal method for saving camera data to disk
-(void)encodeWithCoder:(NSCoder*)coder
{
	CVTimeStamp now;
	now.hostTime = UInt64(AudioGetCurrentHostTime());
	Orientation orientation = [self orientationForTime:&now];
	[coder encodeDouble:orientation.x forKey:@"X"]; 
	[coder encodeDouble:orientation.y forKey:@"Y"]; 
	[coder encodeDouble:orientation.z forKey:@"Z"]; 
	[coder encodeDouble:orientation.longitude forKey:@"Longitude"]; 
	[coder encodeDouble:orientation.latitude  forKey:@"Latitude"]; 
}

// returns true if we're animating
-(BOOL)isAnimatedForTime:(const CVTimeStamp*)timeStamp
{
	return timeStamp->hostTime >= _startTime && timeStamp->hostTime < _endTime;
}

@end
