//
//	File:		twoDimFFT.c
//
//	Contains:	A sample program to illustrate the usage of 2-d FFT functions of real
//				and complex number.  This program also times the functions using the
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

// function prototypes
static void Dummy_fft2d_zip   ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, unsigned log2nc, unsigned log2nr, long flag );
static void Dummy_fft2d_zrip ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, unsigned log2nc, unsigned log2nr, long flag );
static void Dummy_fft2d_zop   ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, COMPLEX_SPLIT *B, long resultRowStride, long resultColumnStride, unsigned log2nc, unsigned log2nr, long flag );
static void Dummy_fft2d_zrop ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, COMPLEX_SPLIT *B, long resultRowStride,  long resultColumnStride, unsigned log2nc, unsigned log2nr, long flag );

static void Complex2dFFTUsageAndTiming( void );
static void Complex2dFFTUsageAndTimingOutOfPlace( void );
static void Real2dFFTUsageAndTiming( void );
static void Real2dFFTUsageAndTimingOutOfPlace ( void );

// called by main.c and that call eventually runs all functions in this file.
void RunTwoDimFFT(void)
{
	Complex2dFFTUsageAndTiming( );
	Complex2dFFTUsageAndTimingOutOfPlace( );
	Real2dFFTUsageAndTiming( );
	Real2dFFTUsageAndTimingOutOfPlace( );
}      

// For the difference between in-place and out-place way, please refer to oneDimFFT.c

/*******************************************************************************
*     Complex Two Dimensional FFT.    ( In-Place way )                         *
*******************************************************************************/

static void Complex2dFFTUsageAndTiming()
{
	COMPLEX_SPLIT originalValue, A;
	FFTSetup      setup;
	UInt32        log2nr, log2nc;
	UInt32        n;
	SInt32        rowStride, columnStride;
	UInt32        i;
	float         scale;
	
	// Set the size of 2d FFT.
	log2nr = N / 2;      // (2^(N/2)) * (2^(N/2)) = 2^n  To allocate n nodes, you specify row and column as a half of it.
	log2nc = N / 2;
	n = 1 << ( log2nr + log2nc );    // n = 2^N
	
	rowStride = 1; 
	columnStride = 0;      // choose 0 for the default value, which is nc * stridInRow  or ( 1 << log2nc )
	
	printf ( "\n2D complex FFT of length log2 ( %d x %d ) = ( %d x %d )\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc );
	
	// Allocate memory for the input operands and check its availability.
	originalValue.realp = ( float* ) malloc ( n * sizeof ( float) );
	originalValue.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	A.realp    = ( float* ) malloc ( n * sizeof ( float ) );
	A.imagp    = ( float* ) malloc ( n * sizeof ( float ) );
	
	if ( originalValue.realp == NULL || originalValue.imagp == NULL || A.realp == NULL || A.imagp == NULL  )
	{
		printf ( "\nmalloc failed to allocate memory for the 2D FFT section of the test.\n");
		exit ( 0 );
	}
	
	// Set the input vector of length n: [(1+j1),...,(1+j1)], where j^2 = -1.
	for ( i = 0; i < n; i++ )
	{
		originalValue.realp[i] = ( float ) (i+1);
		originalValue.imagp[i] = 0.0;
	}      
	
	memcpy ( A.realp, originalValue.realp, ( n * sizeof ( float ) ) ); 
	memcpy ( A.imagp, originalValue.imagp, ( n * sizeof ( float ) ) ); 
	
	// Set up the required memory for the FFT routines and check its availability.
	setup = create_fftsetup ( MAX ( log2nr, log2nc ), FFT_RADIX2 );
	if( setup == NULL )
	{
		printf ( "\nFFT_Setup failed to allocate enough memory.\n" );
		exit(0);
	}    
	
	// Carry out a Forward and Inverse 2d FFT transform, check for errors.
	fft2d_zip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_FORWARD );
	fft2d_zip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_INVERSE );      
	
	// Verify correctness of the results.
	scale = 1.0 / (float) n;     // make constants to scale the result.
	
	// scale the result.
	vsmul( A.realp, 1, &scale, A.realp, 1, n );
	vsmul( A.imagp, 1, &scale, A.imagp, 1, n );
	
	// compare for accuracy report
	Compare ( originalValue.realp, A.realp, n );
	Compare ( originalValue.imagp, A.imagp, n );
	
	// Timing section for the 2d FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4();
#endif
		
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft2d_zip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif 
		
		// Measure and take off the calling overhead of the 2d FFT (very minimal impact).
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft2d_zip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf("\n2D complex FFT of length log2 ( %d x %d ) = ( %d x %d ) is %4.4f µsecs\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc, time );
	}
	
	// Free allocated memory.
	destroy_fftsetup ( setup );
	free ( originalValue.realp );
	free ( originalValue.imagp );
	free ( A.realp );
	free ( A.imagp );
}

/**************************************************************
*        Complex Two Dimensional FFT.    ( Out-Of-Place way ) *
**************************************************************/

static void Complex2dFFTUsageAndTimingOutOfPlace()
{
	COMPLEX_SPLIT originalValue, A, result;
	FFTSetup      setup;
	UInt32        log2nr, log2nc;
	UInt32        n;
	SInt32        rowStride, columnStride;
	SInt32        resultRowStride, resultColumnStride;
	UInt32        i;
	float         scale;
	
	// Set the size of 2d FFT.
	log2nr = N / 2; 
	log2nc = N / 2;
	n = 1 << ( log2nr + log2nc );
	
	rowStride = 1; 
	columnStride = 0;      // choose 0 for the default value, which is nc * stridInRow  or ( 1 << log2nc )
	
	resultRowStride = 1;
	resultColumnStride = 0;
	
	printf ( "\n2D complex FFT out-of-place of length log2 ( %d x %d ) = ( %d x %d )\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc );
	
	// Allocate memory for the input operands and check its availability.
	originalValue.realp = ( float* ) malloc ( n * sizeof ( float) );
	originalValue.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	A.realp    = ( float* ) malloc ( n * sizeof ( float ) );
	A.imagp    = ( float* ) malloc ( n * sizeof ( float ) );
	result.realp = ( float* ) malloc ( n * sizeof ( float ) );
	result.imagp = ( float* ) malloc ( n * sizeof ( float ) );
	
	if ( originalValue.realp == NULL || originalValue.imagp == NULL || A.realp == NULL || A.imagp == NULL || result.realp == NULL || result.imagp == NULL )
	{
		printf ( "\nmalloc failed to allocate memory for the 2D FFT section of the test.\n");
		exit ( 0 );
	}
	
	// Set the input vector of length n: [(1+j1),...,(1+j1)], where j^2 = -1.
	for ( i = 0; i < n; i++ )
	{
		originalValue.realp[i] = ( float ) (i+1);
		originalValue.imagp[i] = 0.0;
	}      
	
	memcpy ( A.realp, originalValue.realp, ( n * sizeof ( float ) ) ); 
	memcpy ( A.imagp, originalValue.imagp, ( n * sizeof ( float ) ) ); 
	
	// Set up the required memory for the FFT routines and check its availability.
	setup = create_fftsetup ( MAX ( log2nr, log2nc ), FFT_RADIX2 );
	if( setup == NULL )
	{
		printf ( "\nFFT_Setup failed to allocate enough memory.\n" );
		exit(0);
	}    
	
	// Carry out a Forward and Inverse 2d FFT transform, check for errors.
	fft2d_zop ( setup, &A, rowStride, columnStride, &result, resultRowStride, resultColumnStride, log2nc, log2nr, FFT_FORWARD );
	fft2d_zop ( setup, &result, resultRowStride, resultColumnStride, &result, resultRowStride, resultColumnStride, log2nc, log2nr, FFT_INVERSE );      
	
	// Verify correctness of the results.
	scale = 1.0 / (float) n; 
	
	vsmul( result.realp, 1, &scale, result.realp, 1, n );
	vsmul( result.imagp, 1, &scale, result.imagp, 1, n );
	
	Compare ( originalValue.realp, result.realp, n );
	Compare ( originalValue.imagp, result.imagp, n );
	
	// Timing section for the 2d FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4();
#endif
		
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft2d_zop ( setup, &A, rowStride, columnStride, &result, resultRowStride, resultColumnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the 2d FFT (very minimal impact).
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft2d_zop ( setup, &A, rowStride, columnStride, &result, resultRowStride, resultColumnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf("\n2D complex FFT out-of-place of length log2 ( %d x %d ) = ( %d x %d ) is %4.4f µsecs\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc, time );
	}
	
	// Free allocated memory.
	destroy_fftsetup ( setup );
	free ( originalValue.realp );
	free ( originalValue.imagp );
	free ( A.realp );
	free ( A.imagp );
	free ( result.realp );
	free ( result.imagp );
}

/*******************************************************************************
*     Real Two Dimensional FFT.   ( In-Place way )                             *
*******************************************************************************/

static void Real2dFFTUsageAndTiming()
{
	COMPLEX_SPLIT A;
	FFTSetup      setup;
	UInt32        log2nr, log2nc;
	UInt32        n, nOver2;
	SInt32        rowStride, columnStride;
	UInt32        i;
	float         scale;
	float         *originalReal, *obtainedReal;
	
	// Set the size of 2d FFT.
	log2nr = N / 2; 
	log2nc = N / 2;
	n = 1 << ( log2nr + log2nc );
	nOver2 = n/2;      // Since one pair of real numbers are considered as one complex number,
					   // if you have n real numbers, then it equivalent to n/2 complex number.
	
	rowStride = 1; 
	columnStride = 0;      // choose 0 for the default value, which is nc * stridInRow  or ( 1 << log2nc )
	
	printf ( "\n2D real FFT of length log2 ( %d x %d ) = ( %d x %d )\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc );
	
	// Allocate memory for the input operands and check its availability.
	originalReal = ( float* ) malloc ( n * sizeof ( float ) );
	obtainedReal = ( float* ) malloc ( n * sizeof ( float ) );
	A.realp    = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	A.imagp    = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	
	if ( originalReal == NULL || obtainedReal == NULL || A.realp == NULL || A.imagp == NULL  )
	{
		printf ( "\nmalloc failed to allocate memory for the 2D FFT section of the test.\n");
		exit ( 0 );
	}
	
	// Set the input vector of length n: [(1+j1),...,(1+j1)], where j^2 = -1.
	for ( i = 0; i < n; i++ )
	{
		originalReal[i] = ( float )i;
	}      
	
	//treat originalRealInput as interleaved complex number. change the format to split 
	//so that we can pass it to fft functions
	ctoz( (COMPLEX*) originalReal, 2, &A, 1, nOver2 );
	
	// Set up the required memory for the FFT routines and check its availability.
	setup = create_fftsetup ( MAX ( log2nr, log2nc ), FFT_RADIX2 );
	if( setup == NULL )
	{
		printf ( "\nFFT_Setup failed to allocate enough memory.\n" );
		exit(0);
	}    
	
	// Carry out a Forward and Inverse 2d FFT transform, check for errors.
	fft2d_zrip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_FORWARD );
	fft2d_zrip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_INVERSE );      
	
	// Verify correctness of the results.
	scale = 1.0 / (float) (2*n);    // get scale factor
	
	// scale the result
	vsmul( A.realp, 1, &scale, A.realp, 1, nOver2 );
	vsmul( A.imagp, 1, &scale, A.imagp, 1, nOver2 );
	
	// The output signal is now in a split real form.  Use the function
	// ztoc to get a split real vector.
	ztoc ( &A, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
	
	// Check for accuracy by looking at the inverse transform results.
	Compare ( originalReal, obtainedReal, n );
	
	// Timing section for the 2d FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4();
#endif
		
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft2d_zrip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the 2d FFT (very minimal impact).
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft2d_zrip ( setup, &A, rowStride, columnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf("\n2D real FFT of length log2 ( %d x %d ) = ( %d x %d ) is %4.4f µsecs\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc, time );
	}
	
	// Free allocated memory.
	destroy_fftsetup ( setup );
	free ( originalReal );
	free ( obtainedReal );
	free ( A.realp );
	free ( A.imagp );
}

/*******************************************************************************
*     Real Two Dimensional FFT. ( Out-Of-Place )                               *
*******************************************************************************/

static void Real2dFFTUsageAndTimingOutOfPlace()
{
	COMPLEX_SPLIT A, result1, result2;
	FFTSetup      setup;
	UInt32        log2nr, log2nc;
	UInt32        n, nOver2;
	SInt32        rowStride, columnStride, result1RowStride, result1ColumnStride, result2RowStride, result2ColumnStride;
	UInt32        i;
	float         scale;
	float         *originalReal, *obtainedReal;
	
	// Set the size of 2d FFT.
	log2nr = N / 2; 
	log2nc = N / 2;
	n = 1 << ( log2nr + log2nc );
	nOver2 = n/2;
	
	rowStride = 1; 
	columnStride = 0;      // choose 0 for the default value, which is nc * stridInRow  or ( 1 << log2nc )
	result1RowStride = 1;
	result1ColumnStride = 0;
	result2RowStride = 1;
	result2ColumnStride = 0;
	
	printf ( "\n2D real FFT out-of-place of length log2 ( %d x %d ) = ( %d x %d )\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc );
	
	// Allocate memory for the input operands and check its availability.
	originalReal = ( float* ) malloc ( n * sizeof ( float ) );
	obtainedReal = ( float* ) malloc ( n * sizeof ( float ) );
	A.realp    = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	A.imagp    = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	result1.realp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	result1.imagp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	result2.realp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	result2.imagp = ( float* ) malloc ( nOver2 * sizeof ( float ) );
	
	
	if ( originalReal == NULL || obtainedReal == NULL || A.realp == NULL || A.imagp == NULL  )
	{
		printf ( "\nmalloc failed to allocate memory for the 2D FFT section of the test.\n");
		exit ( 0 );
	}
	
	// Set the input vector of length n: [(1+j1),...,(1+j1)], where j^2 = -1.
	for ( i = 0; i < n; i++ )
	{
		originalReal[i] = ( float )i;
	}      
	
	//treat originalRealInput as interleaved complex number. change the format to split 
	//so that we can pass it to fft functions
	ctoz( (COMPLEX*) originalReal, 2, &A, 1, nOver2 );
	
	// Set up the required memory for the FFT routines and check its availability.
	setup = create_fftsetup ( MAX ( log2nr, log2nc ), FFT_RADIX2 );
	if( setup == NULL )
	{
		printf ( "\nFFT_Setup failed to allocate enough memory.\n" );
		exit(0);
	}    
	
	// Carry out a Forward and Inverse 2d FFT transform, check for errors.
	fft2d_zrop ( setup, &A, rowStride, columnStride, &result1, result1RowStride, result1ColumnStride, log2nc, log2nr, FFT_FORWARD );
	fft2d_zrop ( setup, &result1, result1RowStride, result1ColumnStride, &result2, result2RowStride, result2ColumnStride, log2nc, log2nr, 
				 FFT_INVERSE );         
	
	// Verify correctness of the results.
	scale = 1.0 / (float) (2*n); 
	
	vsmul( result2.realp, 1, &scale, result2.realp, 1, nOver2 );
	vsmul( result2.imagp, 1, &scale, result2.imagp, 1, nOver2 );
	
	// The output signal is now in a split real form.  Use the function
	// ztoc to get a split real vector.
	ztoc ( &result2, 1, ( COMPLEX * ) obtainedReal, 2, nOver2 );
	
	// Check for accuracy by looking at the inverse transform results.
	Compare ( originalReal, obtainedReal, n );
	
	// Timing section for the 2d FFT.
	{
		float time, overheadTime;
		
#if defined(__VEC__)
		// Turn Java mode off.  Otherwise, there is an extra cycle added to the vfpu.
		// WARNING:  Java mode has to be treated with care.  Some algorithms may be
		// sensitive to flush to zero and may need proper IEEE-754 denormal handling.
		TurnJavaModeOffOnG4();
#endif
		
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			fft2d_zrop ( setup, &A, rowStride, columnStride, &result1, result1RowStride, result1ColumnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &time );
		
#if defined(__VEC__)
		// Restore Java mode.
		RestoreJavaModeOnG4();
#endif
		
		// Measure and take off the calling overhead of the 2d FFT (very minimal impact).
		StartClock();
		for ( i = 0; i < MAX_LOOP_NUM; i++ )
			Dummy_fft2d_zrop ( setup, &A, rowStride, columnStride, &result1, result1RowStride, result1ColumnStride, log2nc, log2nr, FFT_FORWARD );
		StopClock ( &overheadTime );
		
		time -= overheadTime;   
		time /= MAX_LOOP_NUM;
		
		printf("\n2D real FFT out-of-place of length log2 ( %d x %d ) = ( %d x %d ) is %4.4f µsecs\n", 1 << log2nr, 1 << log2nc, (unsigned int)log2nr, (unsigned int)log2nc, time );
	}
	
	// Free allocated memory.
	destroy_fftsetup ( setup );
	free ( originalReal );
	free ( obtainedReal );
	free ( A.realp );
	free ( A.imagp );
	free ( result1.realp );
	free ( result1.imagp );
	free ( result2.realp );
	free ( result2.imagp );
}

void Compare ( float *original, float *computed, long length )
{
	long  i, absErrorIndex, relErrorIndex, maxIndex;
	float absoluteError, relativeError, absTmp, relTmp, maximum, average, noise, signal;
	
	relativeError = 0.0f;
	absoluteError = 0.0f;
	relErrorIndex = 0;
	absErrorIndex = 0;
	maxIndex      = 0;
	maximum       = 0.0f;
	average       = 0.0f;
	signal        = 0.0f;
	noise         = 0.0f;
	
	for ( i = 0; i < length; i++ ) 
	{
		absTmp = fabs ( ( original[i] - computed[i] ) ); // |original[i] - computed[i]|
		relTmp = absTmp / fabs ( original[i] );          // |(original[i] - computed[i])/original[i]|
		
		if ( absTmp > absoluteError ) 
		{
			absoluteError = absTmp;
			absErrorIndex = i;
		}
		
		if ( relTmp > relativeError ) 
		{
			relativeError = relTmp;
			relErrorIndex = i;
		}
		
		if ( computed[i] > maximum )
		{
			maximum = original[i];
			maxIndex = i;
		}
		
		average += computed[i];      
		signal  += computed[i] * computed[i];
		noise   += absTmp * absTmp;
	}
	average /= length;
	
	printf ( "    Acuracy Report:\n");
	printf ( "            The absolute error %13.6e is at node %3d;\n", absoluteError, (unsigned int)absErrorIndex );
	if ( relativeError != HUGE_VAL )
		printf ( "            The relative error %13.6e is at node %3d;\n", relativeError, (unsigned int)relErrorIndex );
	else
		printf ( "            The relative error is meaningless;\n" );
	printf ( "            The maximum value  %13.6e is at node %3d;\n", maximum, (unsigned int)maxIndex );
	printf ( "            The average value is %13.6e;\n", average );
	printf ( "            The signal/noise  is %13.6e.\n", signal/noise );
}

// Dummy functions to measure the overhead time for the function call
static void Dummy_fft2d_zip ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, unsigned log2nc, unsigned log2nr, long flag )
{
#pragma unused( setup )
#pragma unused( A )
#pragma unused( flag )
#pragma unused( log2nr )
#pragma unused( log2nc )
#pragma unused( columnStride )
	rowStride = 1;
}      

static void Dummy_fft2d_zrip ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, unsigned log2nc, unsigned log2nr, long flag )
{
#pragma unused( setup )
#pragma unused( A )
#pragma unused( flag )
#pragma unused( log2nr )
#pragma unused( log2nc )
#pragma unused( columnStride )
	rowStride = 1;
}

static void Dummy_fft2d_zop ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, COMPLEX_SPLIT *B, long resultRowStride,  long resultColumnStride, unsigned log2nc, unsigned log2nr, long flag )
{
#pragma unused( setup )
#pragma unused( A )
#pragma unused( flag )
#pragma unused( log2nr )
#pragma unused( log2nc )
#pragma unused( columnStride )
#pragma unused( B )
#pragma unused( resultColumnStride )
	rowStride = 1;
	resultRowStride = 1;
}

static void Dummy_fft2d_zrop ( FFTSetup setup, COMPLEX_SPLIT *A, long rowStride, long columnStride, COMPLEX_SPLIT *B, long resultRowStride,  long resultColumnStride, unsigned log2nc, unsigned log2nr, long flag )
{
#pragma unused( setup )
#pragma unused( A )
#pragma unused( flag )
#pragma unused( log2nr )
#pragma unused( log2nc )
#pragma unused( columnStride )
#pragma unused( B )
#pragma unused( resultColumnStride )
	rowStride = 1;
	resultRowStride = 1;
}

void Dummy_ctoz ( COMPLEX *C, SInt32 strideC, COMPLEX_SPLIT *Z, SInt32 strideZ, SInt32 size )
{
#pragma unused( C )
#pragma unused( strideC )
#pragma unused( Z )
#pragma unused( strideZ )
	size = 1;
}

void Dummy_ztoc ( COMPLEX_SPLIT *A, SInt32 strideZ, COMPLEX *C, SInt32 strideC, SInt32 size )
{
#pragma unused ( A )
#pragma unused ( strideZ )
#pragma unused ( C )
#pragma unused ( strideC )
	size = 1;
}

