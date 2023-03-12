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

#ifndef TOMS_TRANSPOSE_H
#define TOMS_TRANSPOSE_H

#include "dist_fft_types.h"
    
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef DIST_FFT_USE_DOUBLE
    typedef double TOMS_el_type;
#else
    typedef float TOMS_el_type;
#endif

short TOMS_transpose_2d(TOMS_el_type * a,
                        int nx, int ny,
                        char *move,
                        int move_size);

short TOMS_transpose_2d_arbitrary(TOMS_el_type * a,
                                  int nx, int ny,
                                  int el_size,
                                  char *move,
                                  int move_size);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* TOMS_TRANSPOSE_H */
