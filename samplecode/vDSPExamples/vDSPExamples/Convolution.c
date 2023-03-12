//
//	File:		Convolution.C
//
//	Contains:	A sample program to illustrate the usage of convolution and correlation
//				functions.  This program also times the functions using the microsecond
//				timer.  
//
//	Copyright:  Copyright (c) 2007 Apple Inc., All Rights Reserved
//
//	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by 
//				Apple Inc. ("Apple") in consideration of your agreement to the
//				following terms, and your use, installation, modification or
//				redistribution of this Apple software constitutes acceptance of these
//				terms.  If you do not agree with these terms, please do not use,
//				install, modify or redistribute this Apple software.
//
//				In consideration of your agreement to abide by the following terms, and
//				subject to these terms, Apple grants you a personal, non-exclusive
//				license, under Apple's copyrights in this original Apple software (the
//				"Apple Software"), to use, reproduce, modify and redistribute the Apple
//				Software, with or without modifications, in source and/or binary forms;
//				provided that if you redistribute the Apple Software in its entirety and
//				without modifications, you must retain this notice and the following
//				text and disclaimers in all such redistributions of the Apple Software. 
//				Neither the name, trademarks, service marks or logos of Apple Inc. 
//				may be used to endorse or promote products derived from the Apple
//				Software without specific prior written permission from Apple.  Except
//				as expressly stated in this notice, no other rights or licenses, express
//				or implied, are granted by Apple herein, including but not limited to
//				any patent rights that may be infringed by your derivative works or by
//				other works in which the Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
//				MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
//				THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
//				FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
//				OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
//				OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//				SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//				INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
//				MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
//				AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
//				STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
//				POSSIBILITY OF SUCH DAMAGE.
//
#include <stdio.h>
#include <stdlib.h>
#include <CoreServices/CoreServices.h>
#include <Accelerate/Accelerate.h>

#include "main.h"
#include "javamode.h"

#define MAX_LOOP_NUM 1000    // Number of iterations used in the timing loop

static void ConvTiming( void );
static void Dummy_Conv( float *signal, SInt32 signalStride, float  *filter, SInt32 filterStride, float  *result, SInt32 resultStride, SInt32 resultLength, SInt32 filterLength );

void RunConvolutionSample(void)
{
	ConvTiming();     
}

/*******************************************************************************
*     Convolution.                                                             *
********************************************************************************
*                                                                              *
*     This function performs at 3.69 gigaflops for a convolution of size       *
*     ( 2048 x 256 ) on a 500mhz vector enabled processor.                     *
*                                                                              *
*******************************************************************************/

static void ConvTiming( )
{
	float  *signal, *filter, *result;
	SInt32 signalStride, filterStride, resultStride;
	UInt32 lenSignal, filterLength, resultLength;
	UInt32 i;
	
	filterLength = 256;
	resultLength = 2048;
	lenSignal = ( ( filterLength + 3 ) & 0xFFFFFFFC ) + resultLength;
	
	signalStride = filterStride = resultStride = 1;
	
	printf("\nConvolution ( resultLength = %d, filterLength = %d )\n", (unsigned int)resultLength, (unsigned int)filterLength);
	
	// Allocate memory for the input operands and check its availability.
	signal = ( float* ) malloc ( lenSignal * sizeof ( float ) );
	filter = ( float* ) malloc ( filterLength * sizeof ( float ) );
	result = ( float* ) malloc ( resultLength * sizeof ( float ) );
	
	if( signal == NULL || filter == NULL || result == NULL )      
	{
		printf ( "\nmalloc failed to allocate memory for the convolution sample.\n");
		exit ( 0 );
	}      
	
	// Set the input signal of length "lenSignal" to [1,...,1].
	for( i = 0; i < lenSignal; i++ )
		signal[i] = 1.0;
	
	// Set the filter of length "filterLength" to [1,...,1].
	for( i = 0; i < filterLength; i++ )
		filter[i] = 1.0;
	
	// Correlation.
	conv ( signal, signalStride, filter, filterStride, result, resultStride, resultLength, filterLength );
	
	// Carry out a convolution.
	filterStride = -1;
	conv ( signal, signalStride, filter + filterLength - 1, filterStride, result, resultStride, resultLength, filterLength );
	
	// Timing section for the convolution.
	{
		float time, overheadTime;
		float GFlops;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4();
#endif
		
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			conv ( signal, signalStride, filter, filterStride, result, resultStride, resultLength, filterLength );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the convolution (very minimal impact).
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_Conv ( signal, signalStride, filter, filterStride, result, resultStride, resultLength, filterLength );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		GFlops = ( 2.0f * filterLength - 1.0f ) * resultLength / ( time * 1000.0f );
		
		printf("Time for a ( %d x %d ) Convolution is %4.4f µsecs or %4.4f GFlops\n", (unsigned int)resultLength, (unsigned int)filterLength, time, GFlops );
	}
	
	// Free allocated memory.
	free ( signal );
	free ( filter );
	free ( result );
}      

// Dummy functions that are used to measure the overhead time for the function call.
void Dummy_Conv ( float *signal, SInt32 signalStride, float *filter, SInt32 filterStride, float *result, SInt32 resultStride, SInt32 resultLength, SInt32 filterLength )
{
#pragma unused(  signal, signalStride, filter, filterStride, result, resultStride, resultLength, filterLength  )
}  

