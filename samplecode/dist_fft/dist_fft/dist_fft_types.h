/*
 Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DIST_FFT_TYPES_H
#define DIST_FFT_TYPES_H

#include <stdio.h>
#include <stdlib.h>

#include <Accelerate/Accelerate.h>

#include "mpi.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
// the following options are off by default
// #define DIST_FFT_USE_INTERLEAVED_COMPLEX
// #define DIST_FFT_USE_DOUBLE

#ifndef DIST_FFT_USE_DOUBLE
    
typedef float dist_fft_real;

#define DIST_FFT_DSPSplitComplex DSPSplitComplex
#define DIST_FFT_FFTSetup FFTSetup
#define DIST_FFT_create_fftsetup create_fftsetup
#define DIST_FFT_destroy_fftsetup destroy_fftsetup
#define DIST_FFT_fft_zip fft_zip
#define DIST_FFT_fft_zipt fft_zipt
#define DIST_FFT_fft2d_zip fft2d_zip
#define DIST_FFT_fft2d_zipt fft2d_zipt
#define DIST_FFT_mtrans mtrans
    
#else
    
typedef double dist_fft_real;
    
#define DIST_FFT_DSPSplitComplex DSPDoubleSplitComplex
#define DIST_FFT_FFTSetup FFTSetupD
#define DIST_FFT_create_fftsetup create_fftsetupD
#define DIST_FFT_destroy_fftsetup destroy_fftsetupD
#define DIST_FFT_fft_zip fft_zipD
#define DIST_FFT_fft_zipt fft_ziptD
#define DIST_FFT_fft2d_zip fft2d_zipD
#define DIST_FFT_fft2d_zipt fft2d_ziptD
#define DIST_FFT_mtrans mtransD

#endif /* DIST_FFT_USE_DOUBLE */

#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX

typedef struct {
	dist_fft_real *re, *im;
} dist_fft_complex;
	
#define c_re(c, index)  ((c)->re[index])
#define c_im(c, index)  ((c)->im[index])
	
#define DIST_FFT_MALLOC_DATA(__storage,__size) \
{ \
    (__storage) = malloc(sizeof(dist_fft_complex)); \
    \
    if ((__storage) != NULL) {\
        \
        (__storage)->re = malloc(__size*sizeof(dist_fft_real)); \
        (__storage)->im = malloc(__size*sizeof(dist_fft_real)); \
    } \
}

#define DIST_FFT_MALLOC_WORKSPACE(__storage,__size) \
{ \
    (__storage) = malloc(sizeof(dist_fft_complex)); \
    \
    if ((__storage) != NULL) {\
        \
        (__storage)->re = malloc(__size*sizeof(dist_fft_real)); \
        (__storage)->im = (__storage)->re; \
    } \
}

#define DIST_FFT_FREE_DATA(__storage) \
{ \
    free((__storage)->re); \
    free((__storage)->im); \
    \
    free((__storage)); \
}

#define DIST_FFT_FREE_WORKSPACE(__storage) \
{ \
    free((__storage)->re); \
    \
    free((__storage)); \
}

#else
	
typedef struct {
	dist_fft_real re, im;
} dist_fft_complex;

#define c_re(c, index)  ((c[index]).re)
#define c_im(c, index)  ((c[index]).im)
	
#define DIST_FFT_MALLOC_DATA(__storage,__size) \
{ \
    (__storage) = malloc(sizeof(dist_fft_complex)*__size+16); \
}

#define DIST_FFT_MALLOC_WORKSPACE(__storage,__size) \
{ \
    (__storage) = malloc(sizeof(dist_fft_complex)*__size+16); \
}

#define DIST_FFT_FREE_DATA(__storage) \
{ \
    free((__storage)); \
}

#define DIST_FFT_FREE_WORKSPACE(__storage) \
{ \
    free((__storage)); \
}

#endif /* DIST_FFT_USE_SPLIT_COMPLEX */

#define DIST_FFT_2_PI 6.2831853071795864769252867665590057683943388

typedef dist_fft_complex *dist_fft_storage;

typedef enum {
    DIST_FFT_1D_TYPE, DIST_FFT_2D_TYPE, DIST_FFT_3D_TYPE
} dist_fft_plan_type;

typedef long long dist_fft_dimension;

typedef enum {
    DIST_FFT_FORWARD = -1, DIST_FFT_INVERSE = 1
} dist_fft_direction;

typedef enum {
    DIST_FFT_ROW_INPUT = 0,
    DIST_FFT_ROW_OUTPUT = 0,
    DIST_FFT_COLUMN_INPUT = 1 << 1,
    DIST_FFT_COLUMN_OUTPUT = 1 << 2,
    DIST_FFT_VERBOSE = 1 << 31
} dist_fft_flags;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* DIST_FFT_TYPES_H */
    