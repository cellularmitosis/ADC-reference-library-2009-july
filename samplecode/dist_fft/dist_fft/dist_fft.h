/*
 Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DIST_FFT_H
#define DIST_FFT_H

#include "dist_fft_types.h"
#include "transpose_mpi.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    dist_fft_plan_type type;
    dist_fft_dimension dimension;
    dist_fft_direction direction;
    dist_fft_flags flags;
    dist_fft_dimension length;
    MPI_Comm comm;
    int comm_rank;
    int comm_size;
    dist_fft_dimension transpose_element_size;
    dist_fft_dimension virtual_dimension;
    dist_fft_dimension log2_virtual_dimension;
    dist_fft_dimension virtual_dimension_squared;
    dist_fft_dimension local_dimension;
    dist_fft_dimension local_start;
    dist_fft_dimension local_virtual_dimension;
    dist_fft_dimension local_virtual_start;
    size_t local_storage_size;
    transpose_mpi_plan transpose_plan;
    DIST_FFT_FFTSetup fft_setup;
    DIST_FFT_DSPSplitComplex fft_buffer;
    int useBuffer;
    char *move;
    int verbose;
    double sin_value;
    double cos_value;
} dist_fft_plan_data;

typedef dist_fft_plan_data *dist_fft_plan;

typedef void (* dist_fft_set_data_callback)(dist_fft_dimension x, dist_fft_dimension nx, dist_fft_dimension y, dist_fft_dimension ny, dist_fft_dimension z, dist_fft_dimension nz, dist_fft_real *re, dist_fft_real *im);

dist_fft_plan dist_fft_create_plan(dist_fft_plan_type type,
                                   dist_fft_dimension dimension,
                                   dist_fft_direction direction,
                                   dist_fft_flags flags,
                                   MPI_Comm comm);

extern void dist_fft_destroy_plan(dist_fft_plan plan);

extern void dist_fft_local_dimensions(dist_fft_plan plan,
                                      dist_fft_dimension *local_n,
                                      dist_fft_dimension *local_start,
                                      size_t *local_storage_size);

extern void dist_fft_execute(dist_fft_plan plan,
                             dist_fft_storage data,
                             dist_fft_storage workspace);

void dist_fft_set_data(dist_fft_plan plan, dist_fft_storage data, dist_fft_set_data_callback callback);

void dist_fft_warmup_workspace(dist_fft_plan plan, dist_fft_storage workspace);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* DIST_FFT_H */
