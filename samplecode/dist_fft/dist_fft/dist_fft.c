/*
 Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dist_fft.h"
#include "dist_fft_twiddle.h"
#include "dist_fft_prefetch.h"
#include "TOMS_transpose.h"

#if 0
#define DIST_FFT_DEBUG_PLAN_PRINT_DATA(msg, plan,data) \
{ \
    int __rank; \
    for (__rank = 0; __rank < (plan)->comm_size; __rank++) { \
        MPI_Barrier((plan)->comm); \
        if (__rank == (plan)->comm_rank) { \
            print_data((msg), (plan), (data)); \
        } \
    } \
} \
MPI_Barrier(plan->comm);
#else
#define DIST_FFT_DEBUG_PLAN_PRINT_DATA(msg, plan,data)
#endif

#define DIST_FFT_DEBUG_NO_DISTRIBUTED_TRANSPOSE 0

dist_fft_plan dist_fft_create_plan(dist_fft_plan_type type,
                                   dist_fft_dimension dimension,
                                   dist_fft_direction direction,
                                   dist_fft_flags flags,
                                   MPI_Comm comm)
{
    // verify type and dimension
    dist_fft_dimension log2_dimension = (int)log2((double)(dimension));
    
    if ((1 << log2_dimension) != dimension) return NULL;
    
    if (type == DIST_FFT_1D_TYPE) {
        
        if (log2_dimension < 4 || log2_dimension > 60) return NULL;
    }
    else if (type == DIST_FFT_2D_TYPE) {
        
        if (log2_dimension < 2 || log2_dimension > 30) return NULL;
    }
    else if (type == DIST_FFT_3D_TYPE) {
        
        if (log2_dimension < 2 || log2_dimension > 20)  return NULL;
    }
    else {
        
        return NULL;
    }
        
    dist_fft_plan plan = malloc(sizeof(dist_fft_plan_data));
    
    if (plan != NULL) {
        
        // copy parameters 
        {
            plan->type = type;
            plan->dimension = dimension;
            plan->direction = direction;
            plan->flags = flags;
            plan->comm = comm;
        }
        
        // calculate length
        {
            dist_fft_dimension length = plan->dimension;
            
            if (plan->type == DIST_FFT_2D_TYPE) {
                
                length = length * length;
            }
            else if (plan->type == DIST_FFT_3D_TYPE) {
                
                length = length * length * length;
            }
            
            plan->length = length;
        }
        
        // calculate virtual dimension
        {
            if (plan->type == DIST_FFT_1D_TYPE) {
                
                plan->virtual_dimension = sqrt(plan->dimension);
            }
            else {
                
                plan->virtual_dimension = plan->dimension;
            }
        }
        
        // calculate distributed transpose element size
        {
            if (plan->type == DIST_FFT_3D_TYPE) {
                
                plan->transpose_element_size = plan->dimension;
            }
            else {
                
                plan->transpose_element_size = 1;
            }
            
#ifdef DIST_FFT_USE_INTERLEAVED_COMPLEX
            plan->transpose_element_size *= 2;
#endif
        }
        
        // determine communicator rank and size
        {
            int comm_rank, comm_size;

            MPI_Comm_rank(plan->comm,&comm_rank);
            MPI_Comm_size(plan->comm,&comm_size);
            
            plan->comm_rank = comm_rank;
            plan->comm_size = comm_size;
        }
        
        // calculate log2(virtual_dimension)
        {
            plan->log2_virtual_dimension = (int)log2((double)(plan->virtual_dimension));
        }
        
        // calculate dimension^2
        {
            plan->virtual_dimension_squared = plan->virtual_dimension * plan->virtual_dimension;
        }
        
        // determine local sizes
        {
            int local_dimension, local_start, local_virtual_dimension,local_virtual_start;
            size_t local_storage_size;

            transpose_mpi_get_local_size(plan->virtual_dimension,
                                         plan->comm_rank,
                                         plan->comm_size,
                                         &local_virtual_dimension,
                                         &local_virtual_start);
            
            local_dimension = local_virtual_dimension;
            local_start = local_virtual_start;
            
            local_storage_size = transpose_mpi_get_local_storage_size(plan->virtual_dimension,
                                                                      plan->virtual_dimension,
                                                                      plan->comm_rank,
                                                                      plan->comm_size);
            
            if (plan->type == DIST_FFT_1D_TYPE) {
                
                local_dimension *= plan->virtual_dimension;
                local_start *= plan->virtual_dimension;
            }
            
            if (plan->type == DIST_FFT_3D_TYPE) {
                
                local_storage_size *= plan->virtual_dimension;
            }
            
            plan->local_dimension = local_dimension;
            plan->local_start = local_start;
            plan->local_virtual_dimension = local_virtual_dimension;
            plan->local_virtual_start = local_virtual_start;
            plan->local_storage_size = local_storage_size;
        }
        
        // create a transpose plan
        {
            transpose_mpi_plan transpose_plan;

            transpose_plan = transpose_mpi_create_plan(plan->virtual_dimension,plan->virtual_dimension,plan->comm);
            
            plan->transpose_plan = transpose_plan;
        }
        
        // create vDSP FFT setup
        {
            DIST_FFT_FFTSetup fft_setup = DIST_FFT_create_fftsetup((int)log2((double)(plan->virtual_dimension)), FFT_RADIX2);
            
            plan->fft_setup = fft_setup;
        }
        
        // create vDSP buffer
        {
            DIST_FFT_DSPSplitComplex fft_buffer;
            
            fft_buffer.realp = malloc(16*1024);
            fft_buffer.imagp = malloc(16*1024);
            
            plan->fft_buffer = fft_buffer;
            
            plan->useBuffer = 0;
        }
        
        // create move buffer
        {
            plan->move = malloc(plan->virtual_dimension);
        }
        
        // set verbosity
        {
            plan->verbose = plan->flags & DIST_FFT_VERBOSE != 0;
        }
        
        // compute sin to preload libm
        {
            plan->sin_value = sin(1.0);
            plan->cos_value = cos(1.0);
        }
            
    }    
    
    return plan;
}

void dist_fft_destroy_plan(dist_fft_plan plan)
{
    /*
    if (plan->type == DIST_FFT_1D_TYPE) {
        
        dist_fft_destroy_twiddle(plan->twiddle);
    }
    */
    
    transpose_mpi_destroy_plan(plan->transpose_plan);
    
    DIST_FFT_destroy_fftsetup(plan->fft_setup);
    
    free(plan->fft_buffer.realp);
    free(plan->fft_buffer.imagp);
    
    free(plan->move);
    
    free(plan);
}

void dist_fft_local_dimensions(dist_fft_plan plan,
                               dist_fft_dimension *local_dimension,
                               dist_fft_dimension *local_start,
                               size_t *local_storage_size)
{
    *local_dimension = plan->local_dimension;
    *local_start = plan->local_start;
    *local_storage_size = plan->local_storage_size;
}

void dist_fft_1d_execute(dist_fft_plan plan,
                         dist_fft_storage data,
                         dist_fft_storage workspace)
{
    dist_fft_real *data_re = NULL;
    dist_fft_real *data_im = NULL;
    dist_fft_real *workspace_re = NULL;
    dist_fft_real *workspace_im = NULL;
    
    if (data != NULL) {
        
        data_re = &(c_re(data, 0));
        data_im = &(c_im(data, 0));
    }
    
    if (workspace != NULL) {
        
        workspace_re = &(c_re(workspace, 0));
        workspace_im = &(c_im(workspace, 0));
    }

#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
    int stride = 1;
#else
    int stride = 2;
#endif
    
    double two_pi_over_N = DIST_FFT_2_PI / (double) plan->dimension;
    
    double ts=0,te=0,tdelta=0;
    
    // first transpose the virtual matrix
    if ((plan->flags & DIST_FFT_COLUMN_INPUT) == 0) {
    
        if (plan->verbose) {
        
            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        size_t el_size = plan->transpose_element_size * sizeof(dist_fft_real) / sizeof(TRANSPOSE_EL_TYPE);
        
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_im), (TRANSPOSE_EL_TYPE *)workspace_im);
        
#else
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
#endif

        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transpose time: %f\n", tdelta);
        }
        
        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after first transpose:\n\n", plan, data);    
    }

    // next transform the 2nd dimension
    {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        int row;
        
        DIST_FFT_DSPSplitComplex splitData;
        
        FFTDirection direction = (plan->direction == DIST_FFT_FORWARD ? FFT_FORWARD : FFT_INVERSE);
        
        int offset = 0;
        
        for (row=0; row<plan->local_virtual_dimension; row++) {
            
            prefetch_row(data, row, plan->virtual_dimension);

            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
            
            DIST_FFT_fft_zip(plan->fft_setup, &splitData, stride, plan->log2_virtual_dimension, direction);
            
            DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transform:\n\n", plan, data);

            offset += stride * plan->virtual_dimension;
        }
        
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transform time: %f\n", tdelta);
        }

        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transform:\n\n", plan, data);
    }
    

    // now transpose the virtual matrix again
    if (!DIST_FFT_DEBUG_NO_DISTRIBUTED_TRANSPOSE) {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        size_t el_size = plan->transpose_element_size * (sizeof(dist_fft_real) / sizeof(TRANSPOSE_EL_TYPE));
        
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_im), (TRANSPOSE_EL_TYPE *)workspace_im);
        
#else
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
#endif
    
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transpose time: %f\n", tdelta);
        }

        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transpose:\n\n", plan, data);
    }
    
    // now twiddle, and then transform the 2nd dimension
    {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }

        int row;
        
        DIST_FFT_DSPSplitComplex splitData;
        
        FFTDirection direction = (plan->direction == DIST_FFT_FORWARD ? FFT_FORWARD : FFT_INVERSE);
        
        int offset = 0;
        
        for (row=0; row<plan->local_virtual_dimension; row++) {
            
            if (plan->direction == DIST_FFT_INVERSE) {
                
                twiddle_row_inverse(data, row, plan->local_start, plan->virtual_dimension, two_pi_over_N);
            }
            else if (plan->direction == DIST_FFT_FORWARD) {
                
                twiddle_row_forward(data, row, plan->local_start, plan->virtual_dimension, two_pi_over_N);
            }
            
            DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after twiddle:\n\n", plan, data);

            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
            
            DIST_FFT_fft_zip(plan->fft_setup, &splitData, stride, plan->log2_virtual_dimension, direction);
            
            DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transform:\n\n", plan, data);

            offset += stride * plan->virtual_dimension;
        }
        
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    twiddle+transform time: %f\n", tdelta);
        }
    }

    // now transpose the virtual matrix one more time
    if ((plan->flags & DIST_FFT_COLUMN_OUTPUT) == 0) {
    
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        size_t el_size = plan->transpose_element_size * (sizeof(dist_fft_real) / sizeof(TRANSPOSE_EL_TYPE));
        
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_im), (TRANSPOSE_EL_TYPE *)workspace_im);
        
#else
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
#endif
    
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transpose time: %f\n", tdelta);
        }    
    }
    
    if (plan->verbose && plan->comm_rank == 0) {
        
        printf("\n");
    }
}

void dist_fft_2d_execute(dist_fft_plan plan,
                         dist_fft_storage data,
                         dist_fft_storage workspace)
{
    dist_fft_real *data_re = NULL;
    dist_fft_real *data_im = NULL;
    dist_fft_real *workspace_re = NULL;
    dist_fft_real *workspace_im = NULL;
    
    if (data != NULL) {
        
        data_re = &(c_re(data, 0));
        data_im = &(c_im(data, 0));
    }
    
    if (workspace != NULL) {
        
        workspace_re = &(c_re(workspace, 0));
        workspace_im = &(c_im(workspace, 0));
    }
    
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
    int stride = 1;
#else
    int stride = 2;
#endif
    
    double ts=0,te=0,tdelta=0;
    
    // first transform the 2nd dimension
    {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        int row;
        
        DIST_FFT_DSPSplitComplex splitData;
        
        FFTDirection direction = (plan->direction == DIST_FFT_FORWARD ? FFT_FORWARD : FFT_INVERSE);
        
        int offset = 0;
                
        for (row=0; row<plan->local_virtual_dimension; row++) {
            
            prefetch_row(data, row, plan->virtual_dimension);

            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
            
            if (0 && plan->useBuffer) {
                
                DIST_FFT_fft_zipt(plan->fft_setup, &splitData, stride, &(plan->fft_buffer), plan->log2_virtual_dimension, direction);
            }
            else {
                
                DIST_FFT_fft_zip(plan->fft_setup, &splitData, stride, plan->log2_virtual_dimension, direction);
            }
            
            offset += stride * plan->virtual_dimension;
        }
        
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transform time: %f\n", tdelta);
        }
    }
    
    // now transpose the 1st and 2nd dimensions
    if (!DIST_FFT_DEBUG_NO_DISTRIBUTED_TRANSPOSE) {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        size_t el_size = plan->transpose_element_size * (sizeof(dist_fft_real) / sizeof(TRANSPOSE_EL_TYPE));
        
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_im), (TRANSPOSE_EL_TYPE *)workspace_im);
        
#else
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
#endif
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transpose time: %f\n", tdelta);
        }
    }
    
    // next transform the 2nd (previously 1st) dimension
    {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }

        int row;
        
        DIST_FFT_DSPSplitComplex splitData;
        
        FFTDirection direction = (plan->direction == DIST_FFT_FORWARD ? FFT_FORWARD : FFT_INVERSE);
        
        int offset = 0;
        
        for (row=0; row<plan->local_virtual_dimension; row++) {
            
            prefetch_row(data, row, plan->virtual_dimension);

            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
            
            if (0 && plan->useBuffer) {
                
                DIST_FFT_fft_zipt(plan->fft_setup, &splitData, stride, &(plan->fft_buffer), plan->log2_virtual_dimension, direction);
            }
            else {
                
                DIST_FFT_fft_zip(plan->fft_setup, &splitData, stride, plan->log2_virtual_dimension, direction);
            }
            
            offset += stride * plan->virtual_dimension;
        }
        
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transform time: %f\n", tdelta);
        }
    }
    
    // finally transpose back 2nd and 1st dimensions
    if ( ( (plan->flags & DIST_FFT_COLUMN_INPUT) == 0) ==
         ( (plan->flags & DIST_FFT_COLUMN_OUTPUT) == 0) ) {
          
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        size_t el_size = plan->transpose_element_size * (sizeof(dist_fft_real) / sizeof(TRANSPOSE_EL_TYPE));
        
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_im), (TRANSPOSE_EL_TYPE *)workspace_im);
        
#else
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
#endif
        
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->verbose && plan->comm_rank == 0) {
            
            printf("    transpose time: %f\n", tdelta);
        }
    }
    
    if (plan->verbose && plan->comm_rank == 0) {
        
        printf("\n");
    }
}    

void dist_fft_3d_execute(dist_fft_plan plan,
                         dist_fft_storage data,
                         dist_fft_storage workspace)
{
    dist_fft_real *data_re = NULL;
    dist_fft_real *data_im = NULL;
    dist_fft_real *workspace_re = NULL;
    dist_fft_real *workspace_im = NULL;
    
    if (data != NULL) {
        
        data_re = &(c_re(data, 0));
        data_im = &(c_im(data, 0));
    }
    
    if (workspace != NULL) {
        
        workspace_re = &(c_re(workspace, 0));
        workspace_im = &(c_im(workspace, 0));
    }
    
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
    int stride = 1;
#else
    int stride = 2;
#endif
    
    char *move = plan->move;
            
    double ts=0,te=0,tdelta=0;
    
    // first transform the 3rd dimension then transpose the 2nd and 3rd dimension
    {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        int slab;
        
        DIST_FFT_DSPSplitComplex splitData;
        DIST_FFT_DSPSplitComplex rowSplitData;
                
        FFTDirection direction = (plan->direction == DIST_FFT_FORWARD ? FFT_FORWARD : FFT_INVERSE);
        
        int slabOffset = 0;

        for (slab=0; slab<plan->local_virtual_dimension; slab++) {
            
            int offset = slabOffset;
            
            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
                            
            int row;
            
            for (row=0; row<plan->virtual_dimension; row++) {
                
                rowSplitData.realp = (data_re + offset);
                rowSplitData.imagp = (data_im + offset);
                
                // do the transform
                if (0 && plan->useBuffer) {
                    
                    DIST_FFT_fft_zipt(plan->fft_setup, &rowSplitData, stride, &(plan->fft_buffer), plan->log2_virtual_dimension, direction);
                }
                else {
                    
                    DIST_FFT_fft_zip(plan->fft_setup, &rowSplitData, stride, plan->log2_virtual_dimension, direction);
                }
                
                offset += stride * plan->virtual_dimension;
            }
            
            // do the transpose
            if (stride != 1) {
            
                TOMS_transpose_2d_arbitrary(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, stride, move, plan->virtual_dimension);
            }
            else {
                
                TOMS_transpose_2d(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
                
                TOMS_transpose_2d(splitData.imagp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
            }
                            
            slabOffset += stride * plan->virtual_dimension_squared;
        }

        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transform+transpose:\n\n", plan, data);    
        
        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transform+transpose time: %f\n", tdelta);
        }
    }
    
    // now transpose the 1st and 2nd dimensions
    if (!DIST_FFT_DEBUG_NO_DISTRIBUTED_TRANSPOSE) {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
        
        size_t el_size = plan->transpose_element_size * sizeof(dist_fft_real) / sizeof(TRANSPOSE_EL_TYPE);
        
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_im), (TRANSPOSE_EL_TYPE *)workspace_im);
        
#else
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
#endif
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transpose:\n\n", plan, data);    

        if (plan->verbose && plan->comm_rank == 0) {
            
            printf("    transpose time: %f\n", tdelta);
        }
    }
    
    // next transform the 3rd dimension, then transpose the 2nd and 3rd dimension,
    // then transform the 3rd dimension
    {
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
            
        int slab;
        
        DIST_FFT_DSPSplitComplex splitData;
        DIST_FFT_DSPSplitComplex rowSplitData;
                
        FFTDirection direction = (plan->direction == DIST_FFT_FORWARD ? FFT_FORWARD : FFT_INVERSE);
        
        int slabOffset = 0;

        for (slab=0; slab<plan->local_virtual_dimension; slab++) {
            
            int offset = slabOffset;
            
            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
                            
            int row;
            
            for (row=0; row<plan->virtual_dimension; row++) {
                
                rowSplitData.realp = (data_re + offset);
                rowSplitData.imagp = (data_im + offset);
                
                if (0 && plan->useBuffer) {
                    
                    DIST_FFT_fft_zipt(plan->fft_setup, &rowSplitData, stride, &(plan->fft_buffer), plan->log2_virtual_dimension, direction);
                }
                else {
                    
                    DIST_FFT_fft_zip(plan->fft_setup, &rowSplitData, stride, plan->log2_virtual_dimension, direction);
                }
                
                offset += stride * plan->virtual_dimension;
            }
            
            if (stride != 1) {
                
                TOMS_transpose_2d_arbitrary(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, stride, move, plan->virtual_dimension);
            }
            else {
                
                TOMS_transpose_2d(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
                
                TOMS_transpose_2d(splitData.imagp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
            }
            
            offset = slabOffset;
            
            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
            
            for (row=0; row<plan->virtual_dimension; row++) {
                
                rowSplitData.realp = (data_re + offset);
                rowSplitData.imagp = (data_im + offset);
                
                if (0 && plan->useBuffer) {
                    
                    DIST_FFT_fft_zipt(plan->fft_setup, &rowSplitData, stride, &(plan->fft_buffer), plan->log2_virtual_dimension, direction);
                }
                else {
                    
                    DIST_FFT_fft_zip(plan->fft_setup, &rowSplitData, stride, plan->log2_virtual_dimension, direction);
                }
                
                offset += stride * plan->virtual_dimension;
            }
            
            slabOffset += stride * plan->virtual_dimension_squared;
        }
        
        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transform+transpose+transform:\n\n", plan, data);    
        
        if (plan->verbose && plan->comm_rank == 0) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
            
            printf("    transform+transpose+transform time: %f\n", tdelta);
        }
    }
    
    // finally transpose back 3rd, 2nd and 1st dimensions
    if ( ( (plan->flags & DIST_FFT_COLUMN_INPUT) == 0) ==
         ( (plan->flags & DIST_FFT_COLUMN_OUTPUT) == 0) ) {
          
        if (plan->verbose) {

            // MPI_Barrier(plan->comm);
            ts = MPI_Wtime();
        }
            
        int slab;
        
        DIST_FFT_DSPSplitComplex splitData;
                
        // transpose the 2nd and 3rd dimension
        // (z, y, x) -> (z, x, y)
        
        int slabOffset = 0;
        
        for (slab=0; slab<plan->local_virtual_dimension; slab++) {
            
            int offset = slabOffset;
            
            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
                                        
            if (stride != 1) {
                
                TOMS_transpose_2d_arbitrary(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, stride, move, plan->virtual_dimension);
            }
            else {
                
                TOMS_transpose_2d(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
                
                TOMS_transpose_2d(splitData.imagp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
            }
            
            slabOffset += stride * plan->virtual_dimension_squared;
        }
        
        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transpose1:\n\n", plan, data);    
        
        // transpose the 1st and 2nd dimensions
        // (z, x, y) -> (x, z, y)
        
        size_t el_size = plan->transpose_element_size * sizeof(dist_fft_real) / sizeof(TRANSPOSE_EL_TYPE);
        
#ifndef DIST_FFT_USE_INTERLEAVED_COMPLEX
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_im), (TRANSPOSE_EL_TYPE *)workspace_im);
        
#else
        transpose_mpi(plan->transpose_plan, el_size, (TRANSPOSE_EL_TYPE *)(data_re), (TRANSPOSE_EL_TYPE *)workspace_re);
        
#endif
        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transpose2:\n\n", plan, data);    
        
        // transpose the 2nd and 3rd dimensions
        // (x, z, y) -> (x, y, z)

        slabOffset = 0;

        for (slab=0; slab<plan->local_virtual_dimension; slab++) {
            
            int offset = slabOffset;
            
            splitData.realp = (data_re + offset);
            splitData.imagp = (data_im + offset);
                                        
            if (stride != 1) {
                
                TOMS_transpose_2d_arbitrary(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, stride, move, plan->virtual_dimension);
            }
            else {
                
                TOMS_transpose_2d(splitData.realp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
                
                TOMS_transpose_2d(splitData.imagp, plan->virtual_dimension, plan->virtual_dimension, move, plan->virtual_dimension);
            }
        }

        DIST_FFT_DEBUG_PLAN_PRINT_DATA("*** after transpose3:\n\n", plan, data);    
        
        if (plan->verbose) {
        
            te = MPI_Wtime();
            tdelta = te - ts;
        }
        
        if (plan->verbose && plan->verbose && plan->comm_rank == 0) {
            
            printf("    transpose+transpose+transpose time: %f\n", tdelta);
        }
    }
    
    if (plan->verbose && plan->comm_rank == 0) {
        
        printf("\n");
    }
}

void dist_fft_execute(dist_fft_plan plan,
                      dist_fft_storage data,
                      dist_fft_storage workspace)
{
    switch (plan->type) {
        
        case DIST_FFT_3D_TYPE:
            
            dist_fft_3d_execute(plan, data, workspace);
            break;
            
        case DIST_FFT_2D_TYPE:
            
            dist_fft_2d_execute(plan, data, workspace);
            break;
            
        case DIST_FFT_1D_TYPE:
            
            dist_fft_1d_execute(plan, data, workspace);
            break;
            
        default:
            
            break;
    }
}

void dist_fft_set_data(dist_fft_plan plan, dist_fft_storage data, dist_fft_set_data_callback callback)
{
    dist_fft_dimension x = 0, y = 0, z = 0;
    
    dist_fft_dimension nx=0, ny=0, nz=0;
        
    if (plan->type == DIST_FFT_1D_TYPE) {
    
        nx = plan->dimension;
        ny = nz = 0;
    }
    else if (plan->type == DIST_FFT_2D_TYPE) {
    
        nx = ny = plan->virtual_dimension;
        nz = 0;
    }
    else if (plan->type == DIST_FFT_3D_TYPE) {
    
        nx = ny = nz = plan->virtual_dimension;
    }

    dist_fft_dimension index, local_index;
    
    dist_fft_real re, im;
    
    if (plan->type == DIST_FFT_3D_TYPE) {
    
        dist_fft_dimension start = plan->local_start * ny * nz;
        dist_fft_dimension limit = (plan->local_dimension + plan->local_start) * ny * nz;
    
        for (index=start; index<limit; index++) {
        
            if (plan->flags & DIST_FFT_COLUMN_INPUT) {
            
                x = index % nx;
                y = (index / nx) % ny;
                z = index / (nx * ny);
            }
            else {
            
                x = index / (nz * ny);
                y = (index / nz) % ny;
                z = index % nz;
            }
            
            callback(x, nx, y, ny, z, nz, &re, &im);
            
            local_index = index - start;

            c_re(data, local_index) = re;
            c_im(data, local_index) = im;
        }
    }
    else if (plan->type == DIST_FFT_2D_TYPE) {
        
        dist_fft_dimension start = plan->local_start * ny;
        dist_fft_dimension limit = (plan->local_dimension + plan->local_start) * ny;
    
        for (index=start; index<limit; index++) {
        
            if (plan->flags & DIST_FFT_COLUMN_INPUT) {
            
                x = index % nx;
                y = index / nx;
            }
            else {
            
                x = index / ny;
                y = index % ny;
            }
            
            callback(x, nx, y, ny, 0, 0, &re, &im);
            
            local_index = index - start;

            c_re(data, local_index) = re;
            c_im(data, local_index) = im;
        }
    }
    else if (plan->type == DIST_FFT_1D_TYPE) {
        
        dist_fft_dimension start = plan->local_start;
        dist_fft_dimension limit = plan->local_dimension + plan->local_start;
    
        for (index=start; index<limit; index++) {
        
            if (plan->flags & DIST_FFT_COLUMN_INPUT) {
            
                x = (index / plan->virtual_dimension) +
                    (index % plan->virtual_dimension) * plan->virtual_dimension;
            }
            else {
            
                x = index;
            }
            
            callback(x, nx, y, ny, 0, 0, &re, &im);
            
            local_index = index - start;

            c_re(data, local_index) = re;
            c_im(data, local_index) = im;
        }
    }
}

void dist_fft_warmup_workspace(dist_fft_plan plan, dist_fft_storage workspace)
{
    int i;
    
    dist_fft_dimension limit = plan->local_storage_size;
    
    for (i=0; i<limit; i++) {
    
        c_re(workspace, i) = 0.0;
        c_im(workspace, i) = 0.0;
    }
}
