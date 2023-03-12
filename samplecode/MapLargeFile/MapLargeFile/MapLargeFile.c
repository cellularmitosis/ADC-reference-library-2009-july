/*
    File:       MapLargeFile.c
    
    Description: Shows a technique for accessing files > 4GB in size.

    Copyright:  © Copyright 2003 Apple Computer, Inc. All rights reserved.
    
    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple's
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple Computer, Inc. may be used to endorse or promote products derived from the
                Apple Software without specific prior written permission from Apple.  Except as
                expressly stated in this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any patent rights that
                may be infringed by your derivative works or by other works in which the Apple
                Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
                
    Change History (most recent first):

*/

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/mman.h>


struct timeval lastTime, thisTime;


/* 
	This sample is set up to map a 6GB file into memory in 256MB chunks.
*/

#define kFileSize (0x180000000ULL)  // size of file we expect to map = 6GB
#define kBufferSize (0x10000000UL)  // we will map it in 256MB chunks


int main( int argc, const char *argv[] )
{
    int					fd, cnt, loops;
    unsigned long long	*buffer, sum;
    off_t            	offset;
    
    if ( argc != 2 )
    {
        fprintf( stderr, "usage: %s filename\n", argv[0] );
        return -1;
    } 
    
    // open the file
    fd = open( argv[1], O_RDONLY, 0 );
    if ( fd == 0 )
    {
        fprintf( stderr, "%s: failed to open %s for read\n", argv[0], argv[1] );
        return -1;
    }
    
    sum = 0;
    offset = 0;
    loops = 9;	// loop through the file 10 times
	
    gettimeofday( &lastTime, NULL );
    
    while ( 1 )	// loop for ever, loop exit is handled below
    {        
        // map in a kBufferSize segment of the file
        buffer = mmap( 0, kBufferSize, PROT_READ, MAP_FILE, fd, offset );
        
        if ( buffer == MAP_FAILED )
        {
            fprintf( stderr, "%s: failed to mmap file at offset 0x%llx\n", argv[0], offset );
            break;
        }
        
        // touch every byte in this segment of the file.
        // this 'warms up' the pages in memory so that subsequent accesses will be much faster

        for ( cnt = 0; cnt < kBufferSize /  sizeof(sum); cnt += 16 )
        {
            sum += buffer[cnt + 0];
            sum += buffer[cnt + 1];
            sum += buffer[cnt + 2];
            sum += buffer[cnt + 3];
            sum += buffer[cnt + 4];
            sum += buffer[cnt + 5];
            sum += buffer[cnt + 6];
            sum += buffer[cnt + 7];
            sum += buffer[cnt + 8];
            sum += buffer[cnt + 9];
            sum += buffer[cnt + 10];
            sum += buffer[cnt + 11];
            sum += buffer[cnt + 12];
            sum += buffer[cnt + 13];
            sum += buffer[cnt + 14];
            sum += buffer[cnt + 15];
        }

        // unmap the current segment of the file
        munmap( buffer, kBufferSize );
        
        // increment the offset into the file
        offset += kBufferSize;  
        
        if ( offset >= kFileSize )
        {
			offset = 0;   // reset the offset if we're at the end of the file
        
            gettimeofday( &thisTime, NULL );
        
            printf( "Time = %u seconds, sum = %llx\n", thisTime.tv_sec - lastTime.tv_sec, sum );
            
            if ( loops-- == 0 ) break;	// exit if we've done all loops.

            lastTime = thisTime;
        }
    }
  
  close( fd );	// all done, close the file
  
  return 0;
}
