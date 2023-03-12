/*
    File:		ShadeRect.c
    
    Version:	1.0

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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "ShadeRect.h"

// -----------------------------------------------------------------------------
//	types
// -----------------------------------------------------------------------------
//
typedef struct
{
	float	start;
	float	range;
} ShadeData;

typedef struct
{
	CGRGB	start;
	CGRGB	range;
} ColorShadeData;

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
void GrayscaleGradientEvaluate(
	void*				info,
	const float*		in,
	float*				out );
void ColorGradientEvaluate(
	void*				info,
	const float*		in,
	float*				out );

// -----------------------------------------------------------------------------
//	GrayscaleGradientEvaluate
// -----------------------------------------------------------------------------
//
void GrayscaleGradientEvaluate(
	void*				inInfo,
	const float*		in,
	float*				out )
{
	ShadeData*			data = (ShadeData*) inInfo;
	
	out[ 0 ] = out[ 1 ] = out[ 2 ] = data->start + in[ 0 ] * data->range;
	
	// always 100% alpha  CGShading alpha isn't working in Jaguar right now
	// anyway, so if it does start working, the behavior won't change.
	out[ 3 ] = 1;
}
	
// -----------------------------------------------------------------------------
//	ShadeRect
// -----------------------------------------------------------------------------
//
OSStatus ShadeRect(
	float				inStartShade,
	float				inEndShade,
	const HIRect*		inRect,
	CGContextRef		inContext )
{
	OSStatus			err = noErr;
	CGColorSpaceRef		colorSpace;
	CGFunctionCallbacks	callbacks = { 0, GrayscaleGradientEvaluate, NULL };
	CGFunctionRef		function;
	CGShadingRef		shading;
	ShadeData			data;
	
	// Warning: this stuff is sitting on the stack.  Be careful if you move
	// the shading code around.
	data.start = inStartShade;
	data.range = inEndShade - inStartShade;
	
	CGContextSaveGState( inContext );
	CGContextClipToRect( inContext, *inRect );

	colorSpace = CGColorSpaceCreateDeviceRGB();
	require_action( colorSpace != NULL, CantCreateColorSpace, err = memFullErr );

	function = CGFunctionCreate(
			&data,			// info
			1,				// domainDimension
			NULL,			// input domain NULL == no range clipping
			4,				// rangeDimension,
			NULL,			// output domain NULL == no range clipping
			&callbacks );	// CGFunctionCallbacks
	require_action( function != NULL, CantCreateFunction, err = memFullErr );

	shading = CGShadingCreateAxial(
			colorSpace,
			inRect->origin,	// start
			CGPointMake( CGRectGetMinX( *inRect ), CGRectGetMaxY( *inRect ) ),	// end
			function,
			false,			// extendStart
			false );		// extendEnd
	require_action( colorSpace != NULL, CantCreateShading, err = memFullErr );

	CGContextDrawShading( inContext, shading);

CantCreateFunction:
	CGShadingRelease( shading );

CantCreateShading:
	CGColorSpaceRelease( colorSpace );

CantCreateColorSpace:
	CGContextRestoreGState( inContext );
	
	return err;
}

// -----------------------------------------------------------------------------
//	ColorGradientEvaluate
// -----------------------------------------------------------------------------
//
void ColorGradientEvaluate(
	void*				inInfo,
	const float*		in,
	float*				out )
{
	ColorShadeData*		data = (ColorShadeData*) inInfo;
	
	out[ 0 ] = data->start.red + in[ 0 ] * data->range.red;
	out[ 1 ] = data->start.green + in[ 0 ] * data->range.green;
	out[ 2 ] = data->start.blue + in[ 0 ] * data->range.blue;
	
	// always 100% alpha  CGShading alpha isn't working in Jaguar right now
	// anyway, so if it does start working, the behavior won't change.
	out[ 3 ] = 1;
}
	
// -----------------------------------------------------------------------------
//	ShadeRectColor
// -----------------------------------------------------------------------------
//
OSStatus ShadeRectColor(
	const CGRGB*		inStartColor,
	const CGRGB*		inEndColor,
	const HIRect*		inRect,
	CGContextRef		inContext )
{
	OSStatus			err = noErr;
	CGColorSpaceRef		colorSpace;
	CGFunctionCallbacks	callbacks = { 0, ColorGradientEvaluate, NULL };
	CGFunctionRef		function;
	CGShadingRef		shading;
	ColorShadeData		data;
	
	// Warning: this stuff is sitting on the stack.  Be careful if you move
	// the shading code around.
	data.start = *inStartColor;
	data.range.red = inEndColor->red - inStartColor->red;
	data.range.green = inEndColor->green - inStartColor->green;
	data.range.blue = inEndColor->blue - inStartColor->blue;
	
	CGContextSaveGState( inContext );
	CGContextClipToRect( inContext, *inRect );

	colorSpace = CGColorSpaceCreateDeviceRGB();
	require_action( colorSpace != NULL, CantCreateColorSpace, err = memFullErr );

	function = CGFunctionCreate(
			&data,			// info
			1,				// domainDimension
			NULL,			// input domain NULL == no range clipping
			4,				// rangeDimension,
			NULL,			// output domain NULL == no range clipping
			&callbacks );	// CGFunctionCallbacks
	require_action( function != NULL, CantCreateFunction, err = memFullErr );

	shading = CGShadingCreateAxial(
			colorSpace,
			inRect->origin,	// start
			CGPointMake( CGRectGetMinX( *inRect ), CGRectGetMaxY( *inRect ) ),	// end
			function,
			false,			// extendStart
			false );		// extendEnd
	require_action( colorSpace != NULL, CantCreateShading, err = memFullErr );

	CGContextDrawShading( inContext, shading);

CantCreateFunction:
	CGShadingRelease( shading );

CantCreateShading:
	CGColorSpaceRelease( colorSpace );

CantCreateColorSpace:
	CGContextRestoreGState( inContext );
	
	return err;
}