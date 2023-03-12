/*
 Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "test_main.h"

#ifndef DIST_FFT_TEST_VERBOSE
#define DIST_FFT_TEST_VERBOSE 0
#endif

#define START_TIMING(__i) \
{ \
    double __iterations = __i; \
    double __ts, __te, __tdelta, __tavg; \
    \
    __ts = MPI_Wtime(); \
    \
    for (; __i>0; __i--) {

#define STOP_TIMING(__i, __message, __comm_rank) \
    }\
    \
    __te = MPI_Wtime(); \
    \
    __tdelta = (__te - __ts); \
    __tavg = __tdelta / (__iterations * 2); \
    \
    if (__comm_rank == 0) { \
    \
        printf(__message, __tdelta, __tavg); \
    } \
}
    
#ifndef DIST_FFT_USE_IN_PLACE_TRANSPOSE
static int useWorkspace = 1;
#else
static int useWorkspace = 0;
#endif

static dist_fft_dimension log2_dimension = 6;

static int timing_count = 1;


void print_data(char *msg, dist_fft_plan plan, dist_fft_storage data)
{
    int i, j, k;
    
    char *complex_format_string = "(%1.3f,%1.3f) ";
    
    switch (plan->type) {
        
        case DIST_FFT_3D_TYPE:
        {             
            if (plan->virtual_dimension > 8) {
                
                return;
            }
            
            if (plan->comm_rank == 0) {
                
                printf(msg);
            }
            
            for (i=0; i<plan->local_virtual_dimension; i++) {

                for (j=0; j<plan->virtual_dimension; j++) {
                
                    for (k=0; k<plan->virtual_dimension; k++) {
                        
                        int index = k + (j + i*plan->virtual_dimension)*plan->virtual_dimension;
                        
                        float realp = c_re(data, index);
                        float imagp = c_im(data, index);
                        
                        printf(complex_format_string, realp, imagp);
                    }
                    
                    printf("\n");
                }
                
                printf("\n");
            }
            
            break;
        }    
        case DIST_FFT_2D_TYPE:
        {
            if (plan->virtual_dimension > 8) {
                
                return;
            }
            
            if (plan->comm_rank == 0) {
                
                printf(msg);
            }
            
            for (i=0; i<plan->local_virtual_dimension; i++) {
                
                for (j=0; j<plan->virtual_dimension; j++) {
                    
                    int index = j + i*plan->virtual_dimension;
                    
                    float realp = c_re(data, index);
                    float imagp = c_im(data, index);
                    
                    printf(complex_format_string, realp, imagp);
                }
                
                printf("\n");
            }
            
            printf("\n");
            
            break;
        }    
        case DIST_FFT_1D_TYPE:
        {
            if (plan->virtual_dimension > 8) {
                
                return;
            }
            
            if (plan->comm_rank == 0) {
                
                printf(msg);
            }
            
            for (i=0; i<plan->local_virtual_dimension; i++) {
                
                for (j=0; j<plan->virtual_dimension; j++) {
                    
                    int index = j + i*plan->virtual_dimension;
                    
                    float realp = c_re(data, index);
                    float imagp = c_im(data, index);
                    
                    printf(complex_format_string, realp, imagp);
                }
                
                printf("\n");
            }
            
            printf("\n");
            
            break;
        }    
        default:
        {    
            break;
        }
    }
}

void richard_test_data_callback(dist_fft_dimension x, dist_fft_dimension nx, dist_fft_dimension y, dist_fft_dimension ny, dist_fft_dimension z, dist_fft_dimension nz, dist_fft_real *re, dist_fft_real *im)
{
    *re = 0.0;
    *im = 0.0;
    
    double d_x, d_nx, d_y, d_ny, d_z, d_nz;
    
    d_x = x; d_y = y; d_z = z; d_nx = nx; d_ny = ny; d_nz = nz;
    
    *re = (d_x/d_nx)*(d_x/d_nx) + (d_y/d_ny)*(d_y/d_ny) + (d_z/d_nz)*(d_z/d_nz);
}

void test_data_callback(dist_fft_dimension x, dist_fft_dimension nx, dist_fft_dimension y, dist_fft_dimension ny, dist_fft_dimension z, dist_fft_dimension nz, dist_fft_real *re, dist_fft_real *im)
{
    *re = 0.0;
    *im = 0.0;
        
    // complex wave
    if (1) {
    
        double amp = 1.0;
        
        double theta = DIST_FFT_2_PI * (double)(x) * 2.0 / (double)nx;
        
        *re +=  amp * cos(theta);
        *im += -1 * amp * sin(theta);
    }
    
    // real square waves
    if (1) {
     
        int lump;
        
        lump = x % 4 < 2 ? 1 : -1;
        
        *re += lump;

        if (ny > 0) {
        
            lump = y % 4 < 2 ? 1 : -1;
                
            *re += lump;
        }
        
        if (nz > 0) {
        
            lump = z % 4 < 2 ? 1 : -1;
        
            *re += lump;
        }
    }
    
    // 8
    if (0) {
        
        if (x % 8 == 0) {
            
            *re += 1000.0;
            *im += 0.0;
        }
        else if (x % 8 == 1) {
            
            *re += 707.107;
            *im += 0.0;
        }
        else if (x % 8 == 2) {
            
            *re += 0.0;
            *im += 0.0;
        }
        else if (x % 8 == 3) {
            
            *re += -707.107;
            *im += 0.0;
        }
        else if (x % 8 == 4) {
            
            *re += -1000.0;
            *im += 0.0;
        }
        else if (x % 8 == 5) {
            
            *re += -707.107;
            *im += 0.0;
        }
        else if (x % 8 == 6) {
            
            *re += 0.0;
            *im += 0.0;
        }
        else {
            
            *re += 707.107;
            *im += 0.0;
        }
    }
    
    // 4
    if (0) {
        
        if (x % 4 == 0) {
            
            *re += 0.0;
            *im += 0.0;
        }
        else if (x % 4 == 1) {
            
            *re += 100.0;
            *im += 0.0;
        }
        else if (x % 4 == 2) {
            
            *re += 0.0;
            *im += 0.0;
        }
        else {
            
            *re += -100.0;
            *im += 0.0;
        }
    }
    
    // 2
    if (0) {
        
        if (x % 2 == 0) {
            
            *re += -10.0;
            *im += 0.0;
        }
        else {
            
            *re += 10.0;
            *im += 0.0;
        }
    }
    
    // 1
    if (0) {
        
        *re += 1.0;
        *im += 1.0;
    }
}

void do_forward_inverse_test(dist_fft_plan_type type,
                          dist_fft_dimension dimension,
                          dist_fft_flags forwardFlags,
                          dist_fft_flags inverseFlags,
                          int timing_count,
                          MPI_Comm comm)
{
    double ts,te,tdelta;

    MPI_Barrier(comm);
    
    ts = MPI_Wtime();

    int comm_rank, comm_size, rank;
    
    MPI_Comm_rank(comm,&comm_rank);
    MPI_Comm_size(comm,&comm_size);
    
    dist_fft_plan planForward, planInverse;
    
    planForward = dist_fft_create_plan(type, dimension, DIST_FFT_FORWARD, forwardFlags, comm);
    
    if (planForward == NULL) return;
    
    planInverse = dist_fft_create_plan(type, dimension, DIST_FFT_INVERSE, inverseFlags, comm);
    
    if (planInverse == NULL) return;

    MPI_Barrier(comm);
    
    te = MPI_Wtime();
    
    tdelta = te - ts;

    if (comm_rank == 0) {
    
        printf("  plan creation time: %f\n", tdelta);
    }

    MPI_Barrier(comm);
    
    ts = MPI_Wtime();

    dist_fft_dimension local_dimension,local_start;
    size_t local_storage_size;
    
    dist_fft_storage data = NULL;
    dist_fft_storage workspace = NULL;
    
    dist_fft_local_dimensions(planForward, &local_dimension, &local_start, &local_storage_size);
    
    DIST_FFT_MALLOC_DATA(data, local_storage_size);
    
    if (useWorkspace) {
        
        DIST_FFT_MALLOC_WORKSPACE(workspace, local_storage_size);
    }
    
    MPI_Barrier(comm);
    
    te = MPI_Wtime();
    
    tdelta = te - ts;

    if (DIST_FFT_TEST_VERBOSE && comm_rank == 0) {
    
        printf("  memory allocation time: %f\n", tdelta);
    }

    MPI_Barrier(comm);
    
    ts = MPI_Wtime();

    // initialize the data
    if (1)
    {
        dist_fft_set_data(planForward, data, test_data_callback);
        
        MPI_Barrier(comm);
        
        te = MPI_Wtime();
        
        tdelta = te - ts;

        if (comm_rank == 0) {
        
            printf("  data initialization time: %f\n", tdelta);
        }

        MPI_Barrier(comm);
        
        ts = MPI_Wtime();
    }
    else {
    
        int index;
        
        for (index = 0; index < local_storage_size; index++) {
        
            c_re(data, index) = 0.0;
            c_im(data, index) = 0.0;
        }
        
        MPI_Barrier(comm);
        
        te = MPI_Wtime();
        
        tdelta = te - ts;

        if (comm_rank == 0) {
        
            printf("  data warmup time: %f\n", tdelta);
        }

        MPI_Barrier(comm);
        
        ts = MPI_Wtime();
    }
    
    // warmup the workspace
    if (1 && workspace != NULL)
    {
        dist_fft_warmup_workspace(planForward, workspace);

        MPI_Barrier(comm);
    
        te = MPI_Wtime();
    
        tdelta = te - ts;

        if (comm_rank == 0) {
    
            printf("  workspace warmup time: %f\n\n", tdelta);
        }

        MPI_Barrier(comm);
    
        ts = MPI_Wtime();
    }

    if (timing_count < 1) timing_count = 10;
    
    // run the plain-vanilla 1d fft for comparison sake
    if (0 && type == DIST_FFT_1D_TYPE) {
        
        printf("straight 1d fft:\n\n");
        
        DIST_FFT_DSPSplitComplex splitData;
        
        splitData.realp = &(c_re(data, 0));
        splitData.imagp = &(c_im(data, 0));
        
        DIST_FFT_FFTSetup setup = DIST_FFT_create_fftsetup(log2((double)dimension), FFT_RADIX2);

        print_data("1d before:\n\n", planForward, data);

        DIST_FFT_fft_zip(setup, &splitData, 1, log2((double)dimension), FFT_FORWARD);
        
        print_data("1d middle:\n\n", planForward, data);
        
        DIST_FFT_fft_zip(setup, &splitData, 1, log2((double)dimension), FFT_INVERSE);
        
        double aaa = 1.0 / (double)local_storage_size;
        int zzz;
        
        for (zzz=0; zzz<local_storage_size; zzz++) {
            
            c_re(data, zzz) *= aaa;
            c_im(data, zzz) *= aaa;
        }
        
        print_data("1d after:\n\n", planForward, data);
        
        DIST_FFT_destroy_fftsetup(setup);
    }    
    
    START_TIMING(timing_count);
    
    if (DIST_FFT_TEST_VERBOSE) {
        
        for (rank = 0; rank < comm_size; rank++) {
            MPI_Barrier(comm);
            if (rank == comm_rank) {
                char *message = NULL;
                if ((planForward->flags & DIST_FFT_COLUMN_INPUT) == 0) {
                    message = "BEFORE (row order):\n\n";
                } else {
                    message = "BEFORE (column order):\n\n";
                }
                print_data(message, planForward, data);
            }
            if (comm_rank == 0) printf("(0,0,0) = %.3f\n\n", c_re(data, 0));
        }
        MPI_Barrier(comm);
    }
    
    // execute the plans
    {
        dist_fft_execute(planForward, data, workspace);
        
        if (DIST_FFT_TEST_VERBOSE) {
            
            for (rank = 0; rank < comm_size; rank++) {
                MPI_Barrier(comm);
                if (rank == comm_rank) {
                    char *message = NULL;
                    if ((planForward->flags & DIST_FFT_COLUMN_OUTPUT) == 0) {
                        message = "MIDDLE (row order):\n\n";
                    } else {
                        message = "MIDDLE (column order):\n\n";
                    }
                    print_data(message, planForward, data);
                }
                if (comm_rank == 0) printf("(0,0,0) = %.3f\n\n", c_re(data, 0));
            }
            MPI_Barrier(comm);
        }
        
        dist_fft_execute(planInverse, data, workspace);
    }
    
    // normalize
    {
        if (DIST_FFT_TEST_VERBOSE) {
            
            double one_over_length = 1.0 / (double)planInverse->length;
            int index;
            
            for (index=0; index<local_storage_size; index++) {
                
                c_re(data, index) *= one_over_length;
                c_im(data, index) *= one_over_length;
            }
        }
    }
    
    if (DIST_FFT_TEST_VERBOSE) {
        
        for (rank = 0; rank < comm_size; rank++) {
            MPI_Barrier(comm);
            if (rank == comm_rank) {
                char *message = NULL;
                if ((planInverse->flags & DIST_FFT_COLUMN_OUTPUT) == 0) {
                    message = "AFTER (row order, normalized):\n\n";
                } else {
                    message = "AFTER (column order, normalized):\n\n";
                }
                print_data(message, planInverse, data);
                if (comm_rank == 0) printf("(0,0,0) = %.3f\n\n", c_re(data, 0));
            }
        }
        MPI_Barrier(comm);
    }
    
    STOP_TIMING(timing_count, "  total transformation time: %f\n  average transformation time: %f\n\n", comm_rank);
    
    DIST_FFT_FREE_DATA(data);
    
    if (useWorkspace) {
        
        DIST_FFT_FREE_WORKSPACE(workspace);
    }
    
    dist_fft_destroy_plan(planForward);
    dist_fft_destroy_plan(planInverse);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);
    
    int comm_size;
    
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    
    if (argc > 1) {
        
        int new_log2_dimension = atoi(argv[1]);
        
        if (new_log2_dimension > 1) log2_dimension = new_log2_dimension;
    }
    
    if (argc > 2) {
    
        int new_timing_count = atoi(argv[2]);
        
        if (new_timing_count > 0) timing_count = new_timing_count;
    }

#ifdef DIST_FFT_USE_INTERLEAVED_COMPLEX
    char *complex_numbers = "interleaved";
#else
    char *complex_numbers = "split";
#endif
    
    TEST_MAIN_LOG1("log2(dimension) = %lld\n", log2_dimension);
    TEST_MAIN_LOG1("complex numbers are %s\n", complex_numbers);
    TEST_MAIN_LOG1("real numbers are %s\n", sizeof(dist_fft_real) == 8 ? "doubles" : "floats");
    TEST_MAIN_LOG1("transposes will be %s\n", useWorkspace ? "out of place" : "in place");
    TEST_MAIN_LOG2("running in %d process%s\n\n", comm_size, comm_size == 1 ? "" : "es");
    
    dist_fft_dimension dimension_1d = 1LL << ((log2_dimension/2LL)*2LL);
    dist_fft_dimension dimension_2d = 1LL << (log2_dimension/2LL);
    dist_fft_dimension dimension_3d = 1LL << (log2_dimension/3LL);
        
    dist_fft_flags default_flags = DIST_FFT_TEST_VERBOSE ? DIST_FFT_VERBOSE : 0;
    
    if (1) {
    
        TEST_MAIN_LOG3("testing %d forward and inverse 1D (%lld) FFT%s:\n\n", timing_count, dimension_1d, timing_count == 1 ? "" : "s");
        
        dist_fft_flags forward_1d_flags = default_flags | DIST_FFT_COLUMN_INPUT | DIST_FFT_COLUMN_OUTPUT;
        dist_fft_flags inverse_1d_flags = default_flags | DIST_FFT_COLUMN_OUTPUT | DIST_FFT_COLUMN_INPUT;
        
        do_forward_inverse_test(DIST_FFT_1D_TYPE, dimension_1d, forward_1d_flags, inverse_1d_flags, timing_count, MPI_COMM_WORLD);
        
        TEST_MAIN_LOG4("testing %d forward and inverse 2D (%lldx%lld) FFT%s:\n\n", timing_count, dimension_2d, dimension_2d, timing_count == 1 ? "" : "s");

        dist_fft_flags forward_2d_flags = default_flags | DIST_FFT_COLUMN_INPUT;
        dist_fft_flags inverse_2d_flags = default_flags | DIST_FFT_COLUMN_OUTPUT;

        do_forward_inverse_test(DIST_FFT_2D_TYPE, dimension_2d, forward_2d_flags, inverse_2d_flags, timing_count, MPI_COMM_WORLD);

        TEST_MAIN_LOG5("testing %d forward and inverse 3D (%lldx%lldx%lld) FFT%s:\n\n", timing_count, dimension_3d, dimension_3d, dimension_3d, timing_count == 1 ? "" : "s");

        dist_fft_flags forward_3d_flags = default_flags | DIST_FFT_COLUMN_INPUT;
        dist_fft_flags inverse_3d_flags = default_flags | DIST_FFT_COLUMN_OUTPUT;

        do_forward_inverse_test(DIST_FFT_3D_TYPE, dimension_3d, forward_3d_flags, inverse_3d_flags, timing_count, MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    
    exit(0);
    return 0;
}
