/*
 Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dist_fft_twiddle.h"

#define DEBUG_ENDANGLE_PRINT		0

void twiddle_row_forward(dist_fft_storage data,
                         int local_row,
                         int local_row_start,
                         int dimension,
                         double two_pi_over_N)
{
    int col;
    int row = local_row + local_row_start;
    int index = local_row * dimension;
    int ij = 0;
    
    double twiddle_re1;
    double twiddle_im1;

    double twiddle_re2;
    double twiddle_im2;

    double data_re1;
    double data_im1;

    double data_re2;
    double data_im2;
    
	int		bigloopcount;
	int		bigloopindex;

    // always skip the first column
    index += 1;
    ij += row;
    
    // calculate the phase
    
    double theta = two_pi_over_N * ij;
    
    // incremental calculation of theta
        
    double cos_theta_plus_delta1;
    double sin_theta_plus_delta1;

    double cos_theta_plus_delta2;
    double sin_theta_plus_delta2;
    
    double delta = two_pi_over_N * row;
        
    double cos_theta1 = cos(theta);
    double sin_theta1 = sin(theta);	
	
    double cos_theta2 = cos(theta+delta);
    double sin_theta2 = sin(theta+delta);	

    double delta2 = 2*two_pi_over_N * row;
    double delta2_over_two = delta2 / 2.0;	
    double sin_delta2 = sin(delta2);    
    double sin_delta2_over_two = sin(delta2_over_two);
	
    double two_sin_squared_delta2_over_two = 2.0 * sin_delta2_over_two * sin_delta2_over_two;

    double a_2 = two_sin_squared_delta2_over_two;
    double b_2 = sin_delta2;    

	bigloopcount = (dimension-1)/2;
	
	col = 1;
	
    for (bigloopindex=0; bigloopindex<bigloopcount; bigloopindex++) {
        
        // do the twiddle -- comments show mathematical calculation
        
        // twiddle_re = cos(theta);
        twiddle_re1 = cos_theta1;        
        // twiddle_im = DIST_FFT_FORWARD * sin(theta);
        twiddle_im1 = DIST_FFT_FORWARD * sin_theta1;

        // twiddle_re = cos(theta);
        twiddle_re2 = cos_theta2;        
        // twiddle_im = DIST_FFT_FORWARD * sin(theta);
        twiddle_im2 = DIST_FFT_FORWARD * sin_theta2;
		
        
        data_re1 = c_re(data, index);
        data_im1 = c_im(data, index);

        data_re2 = c_re(data, index+1);
        data_im2 = c_im(data, index+1);

        
        c_re(data, index) = twiddle_re1 * data_re1 - twiddle_im1 * data_im1;
        c_im(data, index) = twiddle_re1 * data_im1 + twiddle_im1 * data_re1;

        c_re(data, index+1) = twiddle_re2 * data_re2 - twiddle_im2 * data_im2;
        c_im(data, index+1) = twiddle_re2 * data_im2 + twiddle_im2 * data_re2;
        
        // ij = col * row;
        ij += row;
        
        // theta = two_pi_over_N * ij;
        cos_theta_plus_delta1 = cos_theta1 - a_2 * cos_theta1 - b_2 * sin_theta1;
        sin_theta_plus_delta1 = sin_theta1 - a_2 * sin_theta1 + b_2 * cos_theta1;

        cos_theta1 = cos_theta_plus_delta1;
        sin_theta1 = sin_theta_plus_delta1;

        // theta = two_pi_over_N * ij;
        cos_theta_plus_delta2 = cos_theta2 - a_2 * cos_theta2 - b_2 * sin_theta2;
        sin_theta_plus_delta2 = sin_theta2 - a_2 * sin_theta2 + b_2 * cos_theta2;

        cos_theta2 = cos_theta_plus_delta2;
        sin_theta2 = sin_theta_plus_delta2;


                
        // index = col + local_row * dimension;
        index += 2;
		
		col += 2;
    }
	
	if (!(dimension & 1)) {
        
        // do the twiddle -- comments show mathematical calculation
        
        // twiddle_re = cos(theta);
        twiddle_re1 = cos_theta1;        
        // twiddle_im = DIST_FFT_FORWARD * sin(theta);
        twiddle_im1 = DIST_FFT_FORWARD * sin_theta1;
		        
        data_re1 = c_re(data, index);
        data_im1 = c_im(data, index);

        c_re(data, index) = twiddle_re1 * data_re1 - twiddle_im1 * data_im1;
        c_im(data, index) = twiddle_re1 * data_im1 + twiddle_im1 * data_re1;        
	}
	
#if DEBUG_ENDANGLE_PRINT	
	// print final angle:
	if (!(dimension & 1)) {
		double endangle = theta+(dimension-1)*delta;
		printf("NEW (even) angle:%.10f truecos:%.10f truesin:%.10f cos:%.10f sin:%.10f\n", endangle, cos(endangle), sin(endangle), twiddle_re1, twiddle_im1);
	} else {
		double endangle = theta+(dimension-1)*delta;
		printf("NEW (odd) angle:%.10f truecos:%.10f truesin:%.10f cos:%.10f sin:%.10f\n", endangle, cos(endangle), sin(endangle), twiddle_re2, twiddle_im2);
	}
#endif // DEBUG_ENDANGLE_PRINT	
}

void twiddle_row_inverse(dist_fft_storage data,
                          int local_row,
                          int local_row_start,
                          int dimension,
                          double two_pi_over_N)
{
    int col;
    int row = local_row + local_row_start;
    int index = local_row * dimension;
    int ij = 0;
    
    double twiddle_re1;
    double twiddle_im1;

    double twiddle_re2;
    double twiddle_im2;

    double data_re1;
    double data_im1;

    double data_re2;
    double data_im2;
    
	int		bigloopcount;
	int		bigloopindex;

    // always skip the first column
    index += 1;
    ij += row;
    
    // calculate the phase
    
    double theta = two_pi_over_N * ij;
    
    // incremental calculation of theta
        
    double cos_theta_plus_delta1;
    double sin_theta_plus_delta1;

    double cos_theta_plus_delta2;
    double sin_theta_plus_delta2;
    
    double delta = two_pi_over_N * row;
        
    double cos_theta1 = cos(theta);
    double sin_theta1 = sin(theta);	
	
    double cos_theta2 = cos(theta+delta);
    double sin_theta2 = sin(theta+delta);	

    double delta2 = 2*two_pi_over_N * row;
    double delta2_over_two = delta2 / 2.0;	
    double sin_delta2 = sin(delta2);    
    double sin_delta2_over_two = sin(delta2_over_two);
	
    double two_sin_squared_delta2_over_two = 2.0 * sin_delta2_over_two * sin_delta2_over_two;

    double a_2 = two_sin_squared_delta2_over_two;
    double b_2 = sin_delta2;    

	bigloopcount = (dimension-1)/2;
	
	col = 1;
	
    for (bigloopindex=0; bigloopindex<bigloopcount; bigloopindex++) {
        
        // do the twiddle -- comments show mathematical calculation
        
        // twiddle_re = cos(theta);
        twiddle_re1 = cos_theta1;        
        // twiddle_im = DIST_FFT_FORWARD * sin(theta);
        twiddle_im1 = DIST_FFT_FORWARD * sin_theta1;

        // twiddle_re = cos(theta);
        twiddle_re2 = cos_theta2;        
        // twiddle_im = DIST_FFT_FORWARD * sin(theta);
        twiddle_im2 = DIST_FFT_FORWARD * sin_theta2;
		
        
        data_re1 = c_re(data, index);
        data_im1 = c_im(data, index);

        data_re2 = c_re(data, index+1);
        data_im2 = c_im(data, index+1);

        
        c_re(data, index) = twiddle_re1 * data_re1 + twiddle_im1 * data_im1;
        c_im(data, index) = twiddle_re1 * data_im1 - twiddle_im1 * data_re1;

        c_re(data, index+1) = twiddle_re2 * data_re2 + twiddle_im2 * data_im2;
        c_im(data, index+1) = twiddle_re2 * data_im2 - twiddle_im2 * data_re2;
        
        // ij = col * row;
        ij += row;
        
        // theta = two_pi_over_N * ij;
        cos_theta_plus_delta1 = cos_theta1 - a_2 * cos_theta1 - b_2 * sin_theta1;
        sin_theta_plus_delta1 = sin_theta1 - a_2 * sin_theta1 + b_2 * cos_theta1;

        cos_theta1 = cos_theta_plus_delta1;
        sin_theta1 = sin_theta_plus_delta1;

        // theta = two_pi_over_N * ij;
        cos_theta_plus_delta2 = cos_theta2 - a_2 * cos_theta2 - b_2 * sin_theta2;
        sin_theta_plus_delta2 = sin_theta2 - a_2 * sin_theta2 + b_2 * cos_theta2;

        cos_theta2 = cos_theta_plus_delta2;
        sin_theta2 = sin_theta_plus_delta2;


                
        // index = col + local_row * dimension;
        index += 2;
		
		col += 2;
    }
	
	if (!(dimension & 1)) {
        
        // do the twiddle -- comments show mathematical calculation
        
        // twiddle_re = cos(theta);
        twiddle_re1 = cos_theta1;        
        // twiddle_im = DIST_FFT_FORWARD * sin(theta);
        twiddle_im1 = DIST_FFT_FORWARD * sin_theta1;
        
        data_re1 = c_re(data, index);
        data_im1 = c_im(data, index);

        c_re(data, index) = twiddle_re1 * data_re1 + twiddle_im1 * data_im1;
        c_im(data, index) = twiddle_re1 * data_im1 - twiddle_im1 * data_re1;        
	}
	
#if DEBUG_ENDANGLE_PRINT	
	// print final angle:
	if (!(dimension & 1)) {
		double endangle = theta+(dimension-1)*delta;
		printf("NEW (even) angle:%.10f truecos:%.10f truesin:%.10f cos:%.10f sin:%.10f\n", endangle, cos(endangle), sin(endangle), twiddle_re1, twiddle_im1);
	} else {
		double endangle = theta+(dimension-1)*delta;
		printf("NEW (odd) angle:%.10f truecos:%.10f truesin:%.10f cos:%.10f sin:%.10f\n", endangle, cos(endangle), sin(endangle), twiddle_re2, twiddle_im2);
	}
#endif // DEBUG_ENDANGLE_PRINT	
}

/*
void twiddle_row_forward(dist_fft_storage data,
                         int local_row,
                         int local_row_start,
                         int dimension,
                         double two_pi_over_N)
{
    int col;
    int row = local_row + local_row_start;
    
    // skip the first row
    if (row == 0) return;
    
    int index = local_row * dimension;
    int ij = 0;
    
    dist_fft_real twiddle_re;
    dist_fft_real twiddle_im;
    
    dist_fft_real data_re;
    dist_fft_real data_im;
    
    // always skip the first column

    index += 1;
    ij += row;
    
    // calculate the phase
    
    double theta = two_pi_over_N * ij;
    
    // incremental calculation of theta
    
    // double cos_theta = cos(theta);
    // double sin_theta = sin(theta);
    // 
    // double cos_theta_plus_delta;
    // double sin_theta_plus_delta;
    // 
    // double delta = two_pi_over_N * row;
    // double delta_over_two = delta / 2.0;
    // 
    // double sin_delta = sin(delta);    
    // double sin_delta_over_two = sin(delta_over_two);
    // 
    // double two_sin_squared_delta_over_two = 2.0 * sin_delta_over_two * sin_delta_over_two;
    // 
    // double a = two_sin_squared_delta_over_two;
    // double b = sin_delta;
    
    for (col=1; col<dimension; col++) {
        
        // do the twiddle -- comments show mathematical calculation
        
        twiddle_re = cos(theta);
        // twiddle_re = cos_theta;
        
        twiddle_im = DIST_FFT_FORWARD * sin(theta);
        // twiddle_im = DIST_FFT_FORWARD * sin_theta;
        
        data_re = c_re(data, index);
        data_im = c_im(data, index);
        
        c_re(data, index) = twiddle_re * data_re - twiddle_im * data_im;
        c_im(data, index) = twiddle_re * data_im + twiddle_im * data_re;
        
        // ij = col * row;
        ij += row;
        
        // theta = two_pi_over_N * ij;
        // cos_theta_plus_delta = cos_theta - a * cos_theta - b * sin_theta;
        // sin_theta_plus_delta = sin_theta - a * sin_theta + b * cos_theta;
        // cos_theta = cos_theta_plus_delta;
        // sin_theta = sin_theta_plus_delta;
                
        // index = col + local_row * dimension;
        index += 1;
    }
}

void twiddle_row_inverse(dist_fft_storage data,
                          int local_row,
                          int local_row_start,
                          int dimension,
                          double two_pi_over_N)
{
    int col;
    int row = local_row + local_row_start;
    
    // skip the first row
    if (row == 0) return;
    
    int index = local_row * dimension;
    int ij = 0;
    
    dist_fft_real twiddle_re;
    dist_fft_real twiddle_im;
    
    dist_fft_real data_re;
    dist_fft_real data_im;
    
    // always skip the first column
    
    index += 1;
    ij += row;
    
    // calculate the phase
    
    double theta = two_pi_over_N * ij;
    
    // incremental calculation of theta
    
    // double cos_theta = cos(theta);
    // double sin_theta = sin(theta);
    // 
    // double cos_theta_plus_delta;
    // double sin_theta_plus_delta;
    // 
    // double delta = two_pi_over_N * row;
    // double delta_over_two = delta / 2.0;
    // 
    // double sin_delta = sin(delta);    
    // double sin_delta_over_two = sin(delta_over_two);
    // 
    // double two_sin_squared_delta_over_two = 2.0 * sin_delta_over_two * sin_delta_over_two;
    // 
    // double a = two_sin_squared_delta_over_two;
    // double b = sin_delta;
        
    for (col=1; col<dimension; col++) {
        
        // do the twiddle -- comments show mathematical calculation
        
        twiddle_re = cos(theta);
        // twiddle_re = cos_theta;
        
        twiddle_im = DIST_FFT_FORWARD * sin(theta);
        // twiddle_im = DIST_FFT_FORWARD * sin_theta;
        
        data_re = c_re(data, index);
        data_im = c_im(data, index);
        
        c_re(data, index) = twiddle_re * data_re + twiddle_im * data_im;
        c_im(data, index) = twiddle_re * data_im - twiddle_im * data_re;
        
        // ij = col * row;
        ij += row;
        
        // theta = two_pi_over_N * ij;
        // cos_theta_plus_delta = cos_theta - a * cos_theta - b * sin_theta;
        // sin_theta_plus_delta = sin_theta - a * sin_theta + b * cos_theta;
        // cos_theta = cos_theta_plus_delta;
        // sin_theta = sin_theta_plus_delta;
        
        // index = col + local_row * dimension;
        index += 1;
    }
}
*/