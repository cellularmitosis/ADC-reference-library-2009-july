/*
*	File:		 ProcessFloatSamples.cpp
*
*	Contains:	 A filter that implements a lowpass filter with with a 400 Hz cutoff.
*	
*	Version:	1.0
*
*	Created:	11-23-2004
*
*   © Copyright 2004 Apple Computer, Inc. All rights reserved.
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
*				consideration of your agreement to the following terms, and your use, installation, modification
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do
*				not agree with these terms, please do not use, install, modify or redistribute this Apple
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms,
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you
*				redistribute the Apple Software in its entirety and without modifications, you must retain this
*				notice and the following text and disclaimers in all such redistributions of the Apple Software.
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to
*				endorse or promote products derived from the Apple Software without specific prior written
*				permission from Apple.  Except as expressly stated in this notice, no other rights or
*				licenses, express or implied, are granted by Apple herein, including but not limited to any
*				patent rights that may be infringed by your derivative works or by other works in which the
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include "ProcessFloatSamples.h"

/* processSamples - implements a lowpass filter with with a 400 Hz cutoff by using the coefficients in setProcessingParameters() 
   in the following difference equation:

	y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] - a1 * y[n-1] - a2 * y[n-2]

   Also saves states for subsequent processing.
*/

void processSamples (float * dataBuffer, UInt32 numSampleFrames, UInt32 numChannels, ParamStructPtr inParamStructPtr, StateStructPtr inStateStructPtr) 
{
	UInt32			index;
	UInt32			numSamplesToProcess;
	float			x_n;
	float			y_n;
	float			b0, b1, b2, a1, a2;
	float			xstate1l, xstate2l, ystate1l, ystate2l;
	float			xstate1r, xstate2r, ystate1r, ystate2r;
	
	numSamplesToProcess = numSampleFrames*numChannels;
	
	// set up local copy of parameters
	b0 = inParamStructPtr->b0;
	b1 = inParamStructPtr->b1;
	b2 = inParamStructPtr->b2;
	a1 = inParamStructPtr->a1;
	a2 = inParamStructPtr->a2;
	
	// set up local copy of state variables
	xstate1l = inStateStructPtr->xstate1l;
	xstate2l = inStateStructPtr->xstate2l;
	ystate1l = inStateStructPtr->ystate1l;
	ystate2l = inStateStructPtr->ystate2l;
	xstate1r = inStateStructPtr->xstate1r;
	xstate2r = inStateStructPtr->xstate2r;
	ystate1r = inStateStructPtr->ystate1r;
	ystate2r = inStateStructPtr->ystate2r;
	
	// process interleaved floating point data
	for (index = 0; index < numSamplesToProcess; index++) 
	{
		x_n = *(dataBuffer);
		// iMic is only stereo or mono; check to maintain correct state variables
		if (    (2 == numChannels)
			 && (1 == (index % 2)))
		{
			// stereo right channel case
			y_n = b0 * x_n + b1 * xstate1r + b2 * xstate2r - a1 * ystate1r - a2 * ystate2r;
			// update state variables
			xstate2r = xstate1r;
			xstate1r = x_n;
			ystate2r = ystate1r;
			ystate1r = y_n;
			// set output data
			*(dataBuffer++) = y_n;
		}
		else
		{
			// left channel or mono case
			y_n = b0 * x_n + b1 * xstate1l + b2 * xstate2l - a1 * ystate1l - a2 * ystate2l;
			// update state variables
			xstate2l = xstate1l;
			xstate1l = x_n;
			ystate2l = ystate1l;
			ystate1l = y_n;
			// set output data
			*(dataBuffer++) = y_n;
		}
		
	}
	
	// save states to structure
	inStateStructPtr->xstate1l = xstate1l;
	inStateStructPtr->xstate2l = xstate2l;
	inStateStructPtr->ystate1l = ystate1l;
	inStateStructPtr->ystate2l = ystate2l;
	inStateStructPtr->xstate1r = xstate1r;
	inStateStructPtr->xstate2r = xstate2r;
	inStateStructPtr->ystate1r = ystate1r;
	inStateStructPtr->ystate2r = ystate2r;
	
} // processSamples

void resetProcessingState (StateStructPtr inStateStructPtr) 
{
	// reset state variables
	inStateStructPtr->xstate1l = 0.0f;
	inStateStructPtr->xstate2l = 0.0f;
	inStateStructPtr->ystate1l = 0.0f;
	inStateStructPtr->ystate2l = 0.0f;
	inStateStructPtr->xstate1r = 0.0f;
	inStateStructPtr->xstate2r = 0.0f;
	inStateStructPtr->ystate1r = 0.0f;
	inStateStructPtr->ystate2r = 0.0f;

} // resetProcessingState

void setProcessingParameters (ParamStructPtr inParamStructPtr, UInt32 inSampleRate, UInt32 inNumChannels) 
{
	// set up our parameters for 400 Hz cutoff lowpass filter
	inParamStructPtr->b0 = 0.000777753436f;
	inParamStructPtr->b1 = 0.001555506871f;
	inParamStructPtr->b2 = 0.000777753436f;
	inParamStructPtr->a1 = -1.913115889852f;
	inParamStructPtr->a2 = 0.916226903594f;

} // setProcessingParameters
