/*
*	File:		ProcessFloatSamples.h
*
*	Contains:	This file provides the interface to a simple filter that implements a lowpass filter with with a 400 Hz cutoff.  
*				Good references for the implementation of filters include:
*				 -Discrete-Time Signal Processing, Schafer
*				 -Introduction to Signal Processing, Orfanidis
*				 -Understanding Digital Signal Processing, Lyons
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

#include <IOKit/IOTypes.h>

// _ParamStruct - coefficients for filter

typedef struct _ParamStruct 
{
	float		b0;
	float		b1;
	float		b2;
	float		a1;
	float		a2;
} ParamStruct, *ParamStructPtr;

// _StateStruct - state variables for filter

typedef struct _StateStruct 
{
	float		xstate1l;
	float		xstate2l;
	float		ystate1l;
	float		ystate2l;
	float		xstate1r;
	float		xstate2r;
	float		ystate1r;
	float		ystate2r;
} StateStruct, *StateStructPtr;

extern "C" 

{

void processSamples (float * dataBuffer, UInt32 numSampleFrames, UInt32 numChannels, ParamStructPtr inParamStructPtr, StateStructPtr inStateStructPtr);
void resetProcessingState (StateStructPtr inStateStructPtr);
void setProcessingParameters (ParamStructPtr inParamStructPtr, UInt32 inSampleRate, UInt32 inNumChannels);

};
