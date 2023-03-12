/*
 Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dist_fft_transpose.h"

#define TRANSPOSE_4x4_VEC(in, out) \
{\
	vector float	temp1, temp2, temp3, temp4;\
    \
    temp1 = vec_mergeh(in##1, in##3);\
    temp2 = vec_mergeh(in##2, in##4);\
    temp3 = vec_mergel(in##1, in##3);\
    temp4 = vec_mergel(in##2, in##4);\
    out##1 = vec_mergeh(temp1, temp2);\
    out##2 = vec_mergel(temp1, temp2);\
    out##3 = vec_mergeh(temp3, temp4);\
    out##4 = vec_mergel(temp3, temp4);\
}


void dist_fft_transpose_square(float *p, int length, int rowbytes)
{
	vector float	vInLeft1, vInLeft2, vInLeft3, vInLeft4;
	vector float	vInTop1, vInTop2, vInTop3, vInTop4;
	vector float	vOutLeft1, vOutLeft2, vOutLeft3, vOutLeft4;
	vector float	vOutTop1, vOutTop2, vOutTop3, vOutTop4;
    
	vector float	*pInLeft;
	vector float	*pInTop;
	
	int				i, j;
	
	for (i=0; i<length/4; i++) {

		////////////////////////////////////////////////////////////////////////////////
		// set pointer to point to the beginning of row 4i
		////////////////////////////////////////////////////////////////////////////////
		pInLeft = (vector float*)(p + 4*i*(rowbytes/sizeof(float)));
		
		////////////////////////////////////////////////////////////////////////////////
		// set pointers to point to the first elements of column 4i
		////////////////////////////////////////////////////////////////////////////////
		pInTop = (vector float*)(p + 4*i);
        
		for (j=0; j<i; j++) {
            
			vInLeft1 = vec_lvx(0*rowbytes, pInLeft);
			vInLeft2 = vec_lvx(1*rowbytes, pInLeft);
			vInLeft3 = vec_lvx(2*rowbytes, pInLeft);
			vInLeft4 = vec_lvx(3*rowbytes, pInLeft);
            
			vInTop1 = vec_lvx(0*rowbytes, pInTop);
			vInTop2 = vec_lvx(1*rowbytes, pInTop);
			vInTop3 = vec_lvx(2*rowbytes, pInTop);
			vInTop4 = vec_lvx(3*rowbytes, pInTop);
			
			TRANSPOSE_4x4_VEC(vInLeft, vOutTop);
			TRANSPOSE_4x4_VEC(vInTop, vOutLeft);
			
			vec_st(vOutTop1, 0*rowbytes, pInTop);
			vec_st(vOutTop2, 1*rowbytes, pInTop);
			vec_st(vOutTop3, 2*rowbytes, pInTop);
			vec_st(vOutTop4, 3*rowbytes, pInTop);
            
			vec_st(vOutLeft1, 0*rowbytes, pInLeft);
			vec_st(vOutLeft2, 1*rowbytes, pInLeft);
			vec_st(vOutLeft3, 2*rowbytes, pInLeft);
			vec_st(vOutLeft4, 3*rowbytes, pInLeft);	
			
			// advance left in by one vector
			pInLeft += 1;
			
			// advance top down by 4 rows
			pInTop += 4*rowbytes/sizeof(vector float);
		}
        
		vInLeft1 = vec_lvx(0*rowbytes, pInLeft);
		vInLeft2 = vec_lvx(1*rowbytes, pInLeft);
		vInLeft3 = vec_lvx(2*rowbytes, pInLeft);
		vInLeft4 = vec_lvx(3*rowbytes, pInLeft);
		
		TRANSPOSE_4x4_VEC(vInLeft, vOutLeft);
        
		vec_st(vOutLeft1, 0*rowbytes, pInLeft);
		vec_st(vOutLeft2, 1*rowbytes, pInLeft);
		vec_st(vOutLeft3, 2*rowbytes, pInLeft);
		vec_st(vOutLeft4, 3*rowbytes, pInLeft);	
	}
}

void dist_fft_transpose_2d(dist_fft_real *a, int a_stride, dist_fft_real *b, int b_stride, dist_fft_dimension nx, dist_fft_dimension ny)
{
    // nx and ny are swapped
    DIST_FFT_mtrans(a, a_stride, b, b_stride, ny, nx);
}
