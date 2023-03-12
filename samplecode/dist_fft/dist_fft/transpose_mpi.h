/*
 * Copyright (c) 1997-1999, 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Changes made by Advanced Computation Group July 2004
 * Copyright (c) 2004 Apple Computer, Inc
 */

#ifndef TRANSPOSE_MPI_H
#define TRANSPOSE_MPI_H

#include "dist_fft_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef DIST_FFT_USE_DOUBLE
    typedef float TRANSPOSE_EL_TYPE;
#else
    typedef double TRANSPOSE_EL_TYPE;
#endif

typedef struct {
    int block_num, dest_pe, send_size, recv_size;
} transpose_mpi_exchange;

typedef struct {
    MPI_Comm comm;
    int n_pes, my_pe;
    
    int nx,ny,local_nx,local_ny;
    
    transpose_mpi_exchange *exchange;
    int num_steps, send_block_size, recv_block_size;
    
    MPI_Datatype el_type;
    
    MPI_Request request[2];
    
    int *perm_block_dest;
    int num_perm_blocks, perm_block_size;
    
    int all_blocks_equal;
    int *send_block_sizes, *send_block_offsets;
    int *recv_block_sizes, *recv_block_offsets;
    
    char *move;
    int move_size;
} transpose_mpi_plan_struct;

typedef transpose_mpi_plan_struct *transpose_mpi_plan;

extern void transpose_mpi_die(const char *error_string);

extern void transpose_mpi_get_local_size(int n, int my_pe, int n_pes,
                                         int *local_n, int *local_start);
extern int transpose_mpi_get_local_storage_size(int nx, int ny,
                                                int my_pe, int n_pes);

extern transpose_mpi_plan transpose_mpi_create_plan(int nx, int ny,
                                                    MPI_Comm comm);
extern void transpose_mpi_destroy_plan(transpose_mpi_plan p);

extern void transpose_mpi(transpose_mpi_plan p, int el_size,
                          TRANSPOSE_EL_TYPE *local_data,
                          TRANSPOSE_EL_TYPE *work);

typedef enum { BEFORE_TRANSPOSE, AFTER_TRANSPOSE } transpose_in_place_which;

typedef enum { TRANSPOSE_SYNC, TRANSPOSE_ASYNC } transpose_sync_type;

extern void transpose_in_place_local(transpose_mpi_plan p,
                                     int el_size, TRANSPOSE_EL_TYPE *local_data,
                                     transpose_in_place_which which);

extern TRANSPOSE_EL_TYPE *transpose_allocate_send_buf(transpose_mpi_plan p,
                                                      int el_size);
extern void transpose_get_send_block(transpose_mpi_plan p, int step,
                                     int *block_y_start, int *block_ny);
extern void transpose_start_exchange_step(transpose_mpi_plan p,
                                          int el_size,
                                          TRANSPOSE_EL_TYPE *local_data,
                                          TRANSPOSE_EL_TYPE *send_buf,
                                          int step,
                                          transpose_sync_type sync_type);
extern void transpose_finish_exchange_step(transpose_mpi_plan p, int step);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* TRANSPOSE_MPI_H */
