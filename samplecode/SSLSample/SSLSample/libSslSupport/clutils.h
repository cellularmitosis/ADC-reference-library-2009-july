/*
	File:		clutils.h
        
        Contains:	common CL app-level routines, X version
        
	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                        ("Apple") in consideration of your agreement to the following terms, and your
                        use, installation, modification or redistribution of this Apple software
                        constitutes acceptance of these terms.  If you do not agree with these terms,
                        please do not use, install, modify or redistribute this Apple software.

                        In consideration of your agreement to abide by the following terms, and subject
                        to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
                11/4/02		1.0d1

*/


#ifndef	_CL_APP_UTILS_CLUTILS_H_
#define _CL_APP_UTILS_CLUTILS_H_

#include <Security/cssm.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern void * appMalloc (uint32 size, void *allocRef);
extern void appFree (void *mem_ptr, void *allocRef);
extern void * appRealloc (void *ptr, uint32 size, void *allocRef);
extern void * appCalloc (uint32 num, uint32 size, void *allocRef);

#define CSSM_MALLOC(size)			appMalloc(size, NULL)
#define CSSM_FREE(ptr)				appFree(ptr, NULL)
#define CSSM_CALLOC(num, size)		appCalloc(num, size, NULL)
#define CSSM_REALLOC(ptr, newSize)	appRealloc(ptr, newSize, NULL)

CSSM_CL_HANDLE clStartup();
void clShutdown(
	CSSM_CL_HANDLE clHand);

CSSM_TP_HANDLE tpStartup();
void tpShutdown(
	CSSM_TP_HANDLE tpHand);


CSSM_DATA_PTR intToDER(unsigned theInt);
uint32 DER_ToInt(const CSSM_DATA *DER_Data);

void printError(char *op, CSSM_RETURN err);
CSSM_BOOL appCompareCssmData(const CSSM_DATA *d1,
	const CSSM_DATA *d2);

#ifdef	__cplusplus
}
#endif

#endif	/* _CL_APP_UTILS_CLUTILS_H_ */