//
//	File:		main.h
//
//	Contains:	header file for <main.c>
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
#ifndef __MAIN__
#define __MAIN__

#include <CoreServices/CoreServices.h>
#include <Accelerate/Accelerate.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__VEC__)
	/* HasAltiVec returns true if the AltiVec is available for use */
extern Boolean HasAltiVec(void);

	/* TurnJavaModeOffOnG4 and RestoreJavaModeOnG4 can be used for
	turning off the processor's java mode on a G4.  These routines call through
	to the lower level TurnJavaModeOff and RestoreJavaMode routines, but only
	after testing for the presence of the altivec unit. */
extern void TurnJavaModeOffOnG4(void);
extern void RestoreJavaModeOnG4(void);
#endif

	/* routines used for timing.  To start the timer, call Start_Clock,
	and to stop the timer call Stop_Clock.  Stop_Clock will return
	the number of microseconds since Start_Clock was called, or
	-1.0f if that number cannot be calculated. */
extern void StartClock( void );
extern void StopClock( float *call_time );

	/* RunConvolutionSample calls the Convolution calculation
	example defined in the file Convolution.c  */
extern void RunConvolutionSample(void);

        /* RunOneDimFFT calls the one dimensional FFT calculation
	example defined in the file oneDimFFT.c  */
extern void RunOneDimFFT(void);

        /* RunTwoDimFFT calls the two dimensional FFT calculation
	example defined in the file twoDimFFT.c  */
extern void RunTwoDimFFT(void);

extern void Compare ( float *original, float *computed, long length );

extern void Dummy_ctoz( COMPLEX *C, SInt32 strideC, COMPLEX_SPLIT *Z, SInt32 strideZ, SInt32 size );
extern void Dummy_ztoc( COMPLEX_SPLIT *A, SInt32 strideZ, COMPLEX *C, SInt32 strideC, SInt32 size );

#ifdef __cplusplus
}
#endif

#endif
