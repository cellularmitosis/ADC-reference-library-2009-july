//
//	File:		oneDimFFT.c
//
//	Contains:	A sample program to illustrate the usage of 1-d FFT functions of real
//				and complex numbers.  This program also times the functions using the
//				microsecond timer.  
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
#include <string.h>
#include <CoreServices/CoreServices.h>
#include <Accelerate/Accelerate.h>

#include "javamode.h"
#include "main.h"

#define MAX_LOOP_NUM       10000  // Number of iterations used in the timing loop
#define kHasAltiVecMask    ( 1 << gestaltPowerPCHasVectorInstructions )  // used in looking for a g4
#define MAX(a,b)           ( (a>b)?a:b )
#define N                  10     // This is a power of 2 defining the length of the FFTs
#include <Carbon/Carbon.h>

// function prototype
static void Dummy_fft_zip( FFTSetup setup, COMPLEX_SPLIT  *C, long stride, unsigned long log2n, long flag );
static void Dummy_fft_zop( FFTSetup setup, COMPLEX_SPLIT  *C, long stride, COMPLEX_SPLIT *R, long resultStride, unsigned long log2n, long flag );
static void Dummy_fft_zrip( FFTSetup setup, COMPLEX_SPLIT *C, long stride, unsigned long log2n, long flag );
static void Dummy_fft_zrop( FFTSetup setup, COMPLEX_SPLIT *C, long stride, COMPLEX_SPLIT *R, long resultStride, unsigned long log2n, long flag );

static void RealFFTUsageAndTiming( void );
static void RealFFTUsageAndTimingOutOfPlace( void );
static void ComplexFFTUsageAndTiming( void );
static void ComplexFFTUsageAndTimingOutOfPlace( void );

// called by main.c to run all functions in this file.
void RunOneDimFFT(void)
{
	RealFFTUsageAndTiming( );
	RealFFTUsageAndTimingOutOfPlace( );
	ComplexFFTUsageAndTiming( );
	ComplexFFTUsageAndTimingOutOfPlace( );
}      

/*******************************************************************************
*     Real One Dimensional FFT ( In-Place way )                                *
*     ; In-place way puts the result in the input array rather than putting    * 
*       the result in the newly created array only for result which is the     *
*       way that out-of-place implemented.                                     *
********************************************************************************
*     The real FFT, unlike the complex FFT, may possibly have to use two       *
*     transformation functions, one before the FFT call and one after the FFT. *
*     This is if the input array is not in the even-odd split configuration.   *
*                                                                              *
*     A real array A = {A[0],...,A[n]} has to be transformed into an even-odd  *
*     array AEvenOdd = {A[0],A[2],...,A[n-1],A[1],A[3],...A[n]} via the call   *
*     "ctoz".                                                                  *
*                                                                              *
*     The result of the real FFT of AEvenOdd of dimension n is a complex       *
*     array of the dimension 2n, with a very special format:                   *
*                                                                              *
*        {[DC,0],C[1],C[2],...,C[n/2],[NY,0],Cc[n/2],...,Cc[2],Cc[1]}  where   *
*                                                                              *
*        1. DC and NY are the dc and nyquist components (real valued),         *
*        2. C is complex in a split representation,                            *
*        3. Cc is the complex conjugate of C in a split representation.        *
*                                                                              *
*     For an n size real array A, the results, which are complex, require 2n   *
*     spaces.  In order to fit the 2n size result into an n size input and     *
*     since the the complex conjugates are duplicate information, the real     *
*     FFT produces its results as follows:                                     *
*                                                                              *
*        {[DC,NY],C[1],C[2],...,C[n/2]}.                                       *
*                                                                              *
*     The timing for a length 1024 real FFT with the transformation functions  *
*     on a 500mhz machine is 14.9µs.  If the data structure of the calling     *
*     program renders the transformation functions unecessary. then the same   *
*     signal is processed in 11.3µs.                                           *
*                                                                              *
*******************************************************************************/

// the function that utilizes the one dimensional fft in-place methods for 
// real number input and measures the running time of those methods.
static void RealFFTUsageAndTiming( )
{
	COMPLEX_SPLIT A;
	FFTSetup      setupReal;
	UInt32        log2n;
	UInt32        n, nOver2;
	SInt32        stride;
	UInt32        i;
	float         *originalReal, *obtainedReal;
	float         scale;
	
	// Set the size of FFT.
	log2n = N;                     // N is 10.
	n = 1 << log2n;                // n is 2^10.
	
	stride = 1;
	nOver2 = n / 2;                // half of n as real part and imag part.
	
	printf ( "1D real FFT of length log2 ( %d ) = %d\n", (unsigned int)n, (unsigned int)log2n );
	
	// Allocate memory for the input operands and check its availability,
	// use the vector version to get 16-byte alignment.
	A.realp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	A.imagp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	originalReal = ( float* ) malloc ( n * sizeof ( float ) );
	obtainedReal = ( float* ) malloc ( n * sizeof ( float ) );
	
	if ( originalReal == NULL || A.realp == NULL || A.imagp == NULL || obtainedReal == NULL )
	{
		printf ( "\nmalloc failed to allocate memory for the real FFT section of the sample.\n" );
		exit ( 0 );
	}
	
	// Generate an input signal in the real domain.
	for ( i = 0; i < n; i++ )
		originalReal[i] = ( float ) ( i+1 );
	
	// Look at the real signal as an interleaved complex vector by casting it.
	// Then call the transformation function ctoz to get a split complex vector,
	// which for a real signal, divides into an even-odd configuration.
	ctoz ( ( COMPLEX * ) originalReal, 2, &A, 1, nOver2 );
	
	// Set up the required memory for the FFT routines and check its availability.
	setupReal = create_fftsetup ( log2n, FFT_RADIX2);
	if ( setupReal == NULL )
	{      
		printf ( "\nFFT_Setup failed to allocate enough memory for the real FFT.\n" );
		exit ( 0 );
	}
	
	// Carry out a Forward and Inverse FFT transform.
	fft_zrip ( setupReal, &A, stride, log2n, FFT_FORWARD );
	fft_zrip ( setupReal, &A, stride, log2n, FFT_INVERSE );
	
	// Verify correctness of the results, but first scale it by 2n.
	scale = (float)1.0/(2*n);
	
	vsmul( A.realp, 1, &scale, A.realp, 1, nOver2 );
	vsmul( A.imagp, 1, &scale, A.imagp, 1, nOver2 );
	
	// The output signal is now in a split real form.  Use the function
	// ztoc to get a split real vector.
	ztoc ( &A, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
	
	// Check for accuracy by looking at the inverse transform results.
	Compare( originalReal, obtainedReal, n );
	
	// Timing section for the real 1d FFT plus the translation functions.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4( );
#endif
		
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
		{
			ctoz ( ( COMPLEX * ) originalReal, 2, &A, 1, nOver2 );
			fft_zrip ( setupReal, &A, stride, log2n, FFT_FORWARD );
			ztoc ( &A, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
		}
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the FFT  and the 
		// translation functions ctoz nad ztoc (very minimal impact).
		
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
		{
			Dummy_ctoz ( ( COMPLEX * ) originalReal, 2, &A, 1, nOver2 );
			Dummy_fft_zrip ( setupReal, &A, stride, log2n, FFT_FORWARD );
			Dummy_ztoc ( &A, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
		}
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf ( "\nTime for 1D real FFT of length log2 ( %d ) = %d plus the translation functions is %4.4f µsecs.", (unsigned int)n, (unsigned int)log2n, time );
		
	}
	
	// Timing section for the real 1d FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4( );
#endif
		
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft_zrip ( setupReal, &A, stride, log2n, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the FFT and the 
		// translation functions ctoz and ztoc (very minimal impact).
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft_zrip ( setupReal, &A, stride, log2n, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf ( "\nTime for 1D real FFT of length log2 ( %d ) = %d only is %4.4f µsecs.\n", (unsigned int)n, (unsigned int)log2n, time );
	}
	
	// Free the allocated memory.
	destroy_fftsetup ( setupReal );
	free ( obtainedReal );
	free ( originalReal );
	free ( A.realp );
	free ( A.imagp );
}

/********************************************************************
*       Real One Dimensional FFT. ( Out-Of-Place way )              *
*       ; Out-of-place way puts the result in the newly created     *
*         array rather than modifying input array.                  *
********************************************************************/

// the function that utilizes the one dimensional fft out-of-place methods 
// for real number input and measures the running time of those methods.
static void RealFFTUsageAndTimingOutOfPlace ( )
{
	COMPLEX_SPLIT A, result1, result2;
	FFTSetup      setupReal;
	UInt32        log2n;
	UInt32        n, nOver2;
	SInt32        stride;
	SInt32        result1Stride, result2Stride;
	UInt32        i;
	float         *originalReal, *obtainedReal;
	float         scale;
	
	// Set the size of FFT.
	log2n = N;                     // N is 10.
	n = 1 << log2n;                // n is 2^10.
	
	stride = 1;
	result1Stride = 1;
	result2Stride = 1;
	nOver2 = n / 2;                // half of n as real part and imag part.
	
	printf ( "1D real FFT out-of-place of length log2 ( %d ) = %d\n", (unsigned int)n, (unsigned int)log2n );
	
	// Allocate memory for the input operands and check its availability,
	// use the vector version to get 16-byte alignment.
	A.realp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	A.imagp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	originalReal = ( float* ) malloc ( n * sizeof ( float ) );
	obtainedReal = ( float* ) malloc ( n * sizeof ( float ) );
	result1.realp = ( float* ) malloc ( nOver2 * sizeof ( float ) );                // storage for intermediate result
	result1.imagp = ( float* ) malloc ( nOver2 * sizeof ( float ) ); 
	result2.realp = ( float* ) malloc ( nOver2 * sizeof ( float ) );                // storage for final result
	result2.imagp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
    
	if ( originalReal == NULL || obtainedReal == NULL || A.realp == NULL || A.imagp == NULL || result1.realp == NULL || result1.imagp == NULL || result2.realp == NULL || result2.imagp == NULL )
	{
		printf ( "\nmalloc failed to allocate memory for the real FFT section of the sample.\n" );
		exit ( 0 );
	}
	
	// Generate an input signal in the real domain.
	for ( i = 0; i < n; i++ )
		originalReal[i] = ( float ) ( i+1 );
	
	// Look at the real signal as an interleaved complex vector by casting it.
	// Then call the transformation function ctoz to get a split complex vector,
	// which for a real signal, divides into an even-odd configuration.
	ctoz ( ( COMPLEX * ) originalReal, 2, &A, 1, nOver2 );
	
	// Set up the required memory for the FFT routines and check its availability.
	setupReal = create_fftsetup ( log2n, FFT_RADIX2);
	if ( setupReal == NULL )
	{      
		printf ( "\nFFT_Setup failed to allocate enough memory for the real FFT.\n" );
		exit ( 0 );
	}
	
	// Carry out a Forward and Inverse FFT transform. It's very important to remember that this is an out-of-place transform. 
	// Therefore, result1 will contain the intermediate result which is the result of forward transform and the result from the
	// inverse transform will be stored again in result1.
	fft_zrop ( setupReal, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
	fft_zrop ( setupReal, &result1, result1Stride, &result2, result2Stride, log2n, FFT_INVERSE );
	
	// Verify correctness of the results, but first scale it by 2n.
	scale = (float)1.0/(2*n);
	
	vsmul( result2.realp, 1, &scale, result2.realp, 1, nOver2 );
	vsmul( result2.imagp, 1, &scale, result2.imagp, 1, nOver2 );
	
	// The output signal is now in a split real form.  Use the function
	// ztoc to get a split real vector.
	ztoc ( &result2, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
	
	// Check for accuracy by looking at the inverse transform results.
	Compare( originalReal, obtainedReal, n );
	
	// Timing section for the real 1d FFT plus the translation functions.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4( );
#endif
		
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
		{
			ctoz ( ( COMPLEX * ) originalReal, 2, &A, 1, nOver2 );
			fft_zrop ( setupReal, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
			ztoc ( &result1, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
		}
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the FFT  and the 
		// translation functions ctoz nad ztoc (very minimal impact).
		
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
		{
			Dummy_ctoz ( ( COMPLEX * ) originalReal, 2, &A, 1, nOver2 );
			Dummy_fft_zrop ( setupReal, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
			Dummy_ztoc ( &result1, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
		}
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf ( "\nTime for 1D real FFT out-of-place of length log2 ( %d ) = %d plus the translation functions is %4.4f µsecs.", (unsigned int)n, (unsigned int)log2n, time );
		
	}
	
	// Timing section for the real 1d FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4( );
#endif
		
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft_zrop ( setupReal, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the FFT and the 
		// translation functions ctoz and ztoc (very minimal impact).
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft_zrop ( setupReal, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf ( "\nTime for 1D real FFT out-of-place of length log2 ( %d ) = %d only is %4.4f µsecs.\n", (unsigned int)n, (unsigned int)log2n, time );
	}
	
	// Free the allocated memory.
	destroy_fftsetup ( setupReal );
	free ( obtainedReal );
	free ( originalReal );
	free ( A.realp );
	free ( A.imagp );
	free ( result1.realp );
	free ( result1.imagp );
	free ( result2.realp );
	free ( result2.imagp );
}

/*******************************************************************************
*      Complex One Dimensional FFT.  ( In-Place way )                          *
*******************************************************************************/

// the function that utilizes the one dimensional fft methods for 
// complex number input and measures the running time of those methods.
static void ComplexFFTUsageAndTiming ( )
{
	COMPLEX_SPLIT originalValue, A;
	FFTSetup      setup;
	UInt32        log2n;
	UInt32        n;
	SInt32        stride;
	UInt32        i;
	float         scale;
	
	// Set the size of FFT.
	log2n = N;
	n = 1 << log2n;
	
	stride = 1;
	
	printf ( "1D complex FFT of length log2 ( %d ) = %d\n", (unsigned int)n, (unsigned int)log2n );
	
	// Allocate memory for the input operands and check its availability.
	originalValue.realp = ( float* ) malloc ( n * sizeof ( float ) );
	originalValue.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	A.realp = ( float* ) malloc ( n * sizeof ( float ) );
	A.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	
	if( originalValue.realp == NULL || originalValue.imagp == NULL || A.realp == NULL || A.imagp == NULL  )
	{
		printf( "\nmalloc failed to allocate memory for the FFT section of the test.\n");
		exit(0);
	}
	
	// Set the input vector of length n: [(1+j1),...,(1+j1)], where j^2 = -1.      
	for ( i = 0; i < n; i++ )
	{
		originalValue.realp[i] = ( float ) ( i + 1 );
		originalValue.imagp[i] = 0.0;
	}      
	
	memcpy ( A.realp, originalValue.realp, ( n * sizeof ( float ) ) ); 
	memcpy ( A.imagp, originalValue.imagp, ( n * sizeof ( float ) ) ); 
	
	// Set up the required memory for the FFT routines and check its availability.
	setup = create_fftsetup ( log2n, FFT_RADIX2 );
	if ( setup == NULL )
	{      
		printf ( "\nFFT_Setup failed to allocate enough memory.\n" );
		exit ( 0 );
	}
	
	// Carry out a Forward and Inverse FFT transform, check for errors.
	fft_zip ( setup, &A, stride, log2n, FFT_FORWARD );
	fft_zip ( setup, &A, stride, log2n, FFT_INVERSE );
	
	// Verify correctness of the results.
	scale = ( float ) 1.0 / n;
	
	vsmul( A.realp, 1, &scale, A.realp, 1, n );
	vsmul( A.imagp, 1, &scale, A.imagp, 1, n );
	
	Compare( originalValue.realp, A.realp, n );
	Compare( originalValue.imagp, A.imagp, n );
	
	// Timing section for the FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4( );
#endif
		
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft_zip ( setup, &A, stride, log2n, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the FFT (very minimal impact).
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft_zip ( setup, &A, stride, log2n, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf ( "\nTime for 1D complex FFT of length log2 ( %d ) = %d is %4.4f µsecs\n", (unsigned int)n, (unsigned int)log2n, time );
		
	}
	
	// Free allocated memory.
	destroy_fftsetup ( setup );
	free ( originalValue.realp );
	free ( originalValue.imagp );
	free ( A.realp );
	free ( A.imagp );
}

/************************************************************
*         Complex One Dimensional FFT.  ( Out-Of-Place Way) *
************************************************************/

// the function that utilizes the one dimensional fft methods for 
// real number input and measures the running time of those methods.      
static void ComplexFFTUsageAndTimingOutOfPlace ( )
{
	COMPLEX_SPLIT originalValue, A, result1;
	FFTSetup      setup;
	UInt32        log2n;
	UInt32        n;
	SInt32        stride;
	SInt32        result1Stride;
	UInt32        i;
	float         scale;
	
	// Set the size of FFT.
	log2n = 10;
	n = 1 << log2n;
	
	stride = 1;
	result1Stride = 1;
	
	printf ( "1D complex FFT out-of-place of length log2 ( %d ) = %d\n", (unsigned int)n, (unsigned int)log2n );
	
	// Allocate memory for the input operands and check its availability.
	originalValue.realp = ( float* ) malloc ( n * sizeof ( float ) );
	originalValue.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	A.realp = ( float* ) malloc ( n * sizeof ( float ) );
	A.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	result1.realp = ( float* ) malloc ( n * sizeof ( float ) );
	result1.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	
	if( originalValue.realp == NULL || originalValue.imagp == NULL || A.realp == NULL || A.imagp == NULL || result1.realp == NULL || result1.imagp == NULL )
	{
		printf( "\nmalloc failed to allocate memory for the FFT section of the test.\n");
		exit(0);
	}
	
	// Set the input vector of length n: [(1+j1),...,(1+j1)], where j^2 = -1.       
	for ( i = 0; i < n; i++ )
	{
		originalValue.realp[i] = ( float ) ( i + 1 );
		originalValue.imagp[i] = 0.0;
	}      
	
	memcpy ( A.realp, originalValue.realp, ( n * sizeof ( float ) ) ); 
	memcpy ( A.imagp, originalValue.imagp, ( n * sizeof ( float ) ) ); 
	
	// Set up the required memory for the FFT routines and check its availability.
	setup = create_fftsetup ( log2n, FFT_RADIX2 );
	if ( setup == NULL )
	{      
		printf ( "\nFFT_Setup failed to allocate enough memory.\n" );
		exit ( 0 );
	}
	
	// Carry out a Forward and Inverse FFT transform, check for errors.
	fft_zop ( setup, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
	fft_zop ( setup, &result1, result1Stride, &result1, result1Stride, log2n, FFT_INVERSE );
	
	// Verify correctness of the results.
	scale = ( float ) 1.0 / n;
	
	vsmul( result1.realp, 1, &scale, result1.realp, 1, n );
	vsmul( result1.imagp, 1, &scale, result1.imagp, 1, n );
	
	Compare( originalValue.realp, result1.realp, n );
	Compare( originalValue.imagp, result1.imagp, n );
	
	// Timing section for the FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4( );
#endif
		
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft_zop ( setup, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the FFT (very minimal impact).
		StartClock ( );
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft_zop ( setup, &A, stride, &result1, result1Stride, log2n, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf ( "\nTime for 1D complex FFT out-of-place of length log2 ( %d ) = %d is %4.4f µsecs\n", (unsigned int)n, (unsigned int)log2n, time );
		
	}
	
	// Free allocated memory.
	destroy_fftsetup ( setup );
	free ( originalValue.realp );
	free ( originalValue.imagp );
	free ( A.realp );
	free ( A.imagp );
	free ( result1.realp );
	free ( result1.imagp );
}


// Dummy functions to measure the overhead time for the function call.
void Dummy_fft_zip ( FFTSetup setup, COMPLEX_SPLIT *C, long K, unsigned long log2n, long flag )
{
#pragma unused( setup )
#pragma unused( C )
#pragma unused( flag )
#pragma unused( log2n )
	K = 1;
}     

void Dummy_fft_zop ( FFTSetup setup, COMPLEX_SPLIT *C, long K, COMPLEX_SPLIT *R, long L, unsigned long log2n, long flag )
{
#pragma unused( setup )
#pragma unused( C )
#pragma unused( flag )
#pragma unused( log2n )
#pragma unused( R )
	K = 1;
	L = 1;
}

void Dummy_fft_zrip ( FFTSetup setup, COMPLEX_SPLIT *C, long K, unsigned long log2n, long flag )
{
#pragma unused( setup )
#pragma unused( C )
#pragma unused( flag )
#pragma unused( log2n )
	K = 1;
}      

void Dummy_fft_zrop ( FFTSetup setup, COMPLEX_SPLIT *C, long K, COMPLEX_SPLIT *R, long L, unsigned long log2n, long flag)
{
#pragma unused( setup )
#pragma unused( C )
#pragma unused( flag )
#pragma unused( log2n )
#pragma unused( R )
	K = 1;
	L = 1;
}  

