/*
 Copyright 2004 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dist_fft_twiddle.h"

// this function walks through data loading every fourth element
// the purpose is to cache the data for a subsequent FFT
dist_fft_real prefetch_row(dist_fft_storage data,
                           int local_row,
                           int dimension)
{
    // prefetching only speeds things up for very large dimensions
    if (dimension < (1 << 13)) return 0;
    
    int index = local_row * dimension;
    
    int index1 = index;
    int index2 = index+4;
    int index3 = index+8;
    int index4 = index+12;
    
    double data_re1;
    double data_im1;

    double data_re2;
    double data_im2;

    double data_re3;
    double data_im3;

    double data_re4;
    double data_im4;

	int bigloopcount;
	int bigloopindex;
            
    dist_fft_real sum_re1 = 0, sum_im1 = 0, sum_re2 = 0, sum_im2 = 0;
    dist_fft_real sum_re3 = 0, sum_im3 = 0, sum_re4 = 0, sum_im4 = 0;
    
	bigloopcount = (dimension)/16;
	
    for (bigloopindex=0; bigloopindex<bigloopcount; bigloopindex++) {
        
        data_re1 = c_re(data, index1);
        data_im1 = c_im(data, index1);
        
        data_re2 = c_re(data, index2);
        data_im2 = c_im(data, index2);
        
        data_re3 = c_re(data, index3);
        data_im3 = c_im(data, index3);
        
        data_re4 = c_re(data, index4);
        data_im4 = c_im(data, index4);
        
        sum_re1 += data_re1;
        sum_im1 += data_im1;
                
        sum_re2 += data_re2;
        sum_im2 += data_im2;
                
        sum_re3 += data_re3;
        sum_im3 += data_im3;
                
        sum_re4 += data_re4;
        sum_im4 += data_im4;
                
        index1 += 16;
        index2 += 16;
        index3 += 16;
        index4 += 16;
    }
    
    return sum_re1 * sum_im1 + sum_re2 * sum_im2 + sum_re3 * sum_im3 + sum_re4 * sum_im4;
}
