/*
	File:		clutils.c
        
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


#include <stdlib.h>
#include <stdio.h>
#include <Security/cssm.h>
#include "clutils.h"
#include <Security/cssmapple.h>	
#include <string.h>
/*
 * Standard app-level memory functions required by CDSA.
 */
void * appMalloc (uint32 size, void *allocRef) {
	return( malloc(size) );
}
void appFree (void *mem_ptr, void *allocRef) {
	free(mem_ptr);
 	return;
}
void * appRealloc (void *ptr, uint32 size, void *allocRef) {
	return( realloc( ptr, size ) );
}
void * appCalloc (uint32 num, uint32 size, void *allocRef) {
	return( calloc( num, size ) );
}

static CSSM_API_MEMORY_FUNCS memFuncs = {
	appMalloc,
	appFree,
	appRealloc,
 	appCalloc,
 	NULL
 };

static CSSM_VERSION vers = {2, 0};
static const CSSM_GUID testGuid = { 0xFADE, 0, 0, { 1,2,3,4,5,6,7,0 }};

/*
 * Init CSSM; returns CSSM_FALSE on error. Reusable.
 */
static CSSM_BOOL cssmInitd = CSSM_FALSE;
CSSM_BOOL cssmStartup()
{
	CSSM_RETURN  crtn;
    CSSM_PVC_MODE pvcPolicy = CSSM_PVC_NONE;
	
	if(cssmInitd) {
		return CSSM_TRUE;
	}  
	crtn = CSSM_Init (&vers, 
		CSSM_PRIVILEGE_SCOPE_NONE,
		&testGuid,
		CSSM_KEY_HIERARCHY_NONE,
		&pvcPolicy,
		NULL /* reserved */);
	if(crtn != CSSM_OK) 
	{
		printError("CSSM_Init", crtn);
		return CSSM_FALSE;
	}
	else {
		cssmInitd = CSSM_TRUE;
		return CSSM_TRUE;
	}
}


/*
 * Init CSSM and establish a session with the Apple CL.
 */
CSSM_CL_HANDLE clStartup()
{
	CSSM_CL_HANDLE clHand;
	CSSM_RETURN crtn;
	
	if(cssmStartup() == CSSM_FALSE) {
		return 0;
	}
	crtn = CSSM_ModuleLoad(&gGuidAppleX509CL,
		CSSM_KEY_HIERARCHY_NONE,
		NULL,			// eventHandler
		NULL);			// AppNotifyCallbackCtx
	if(crtn) {
		printError("CSSM_ModuleLoad(AppleCL)", crtn);
		return 0;
	}
	crtn = CSSM_ModuleAttach (&gGuidAppleX509CL,
		&vers,
		&memFuncs,				// memFuncs
		0,						// SubserviceID
		CSSM_SERVICE_CL,		// SubserviceFlags - Where is this used?
		0,						// AttachFlags
		CSSM_KEY_HIERARCHY_NONE,
		NULL,					// FunctionTable
		0,						// NumFuncTable
		NULL,					// reserved
		&clHand);
	if(crtn) {
		printError("CSSM_ModuleAttach(AppleCL)", crtn);
		return 0;
	}
	else {
		return clHand;
	}
}

void clShutdown(
	CSSM_CL_HANDLE clHand)
{
	CSSM_RETURN crtn;
	
	crtn = CSSM_ModuleDetach(clHand);
	if(crtn) {
		printf("Error detaching from AppleCL\n");
		printError("CSSM_ModuleDetach", crtn);
		return;
	}
	crtn = CSSM_ModuleUnload(&gGuidAppleX509CL, NULL, NULL);
	if(crtn) {
		printf("Error unloading AppleCL\n");
		printError("CSSM_ModuleUnload", crtn);
	}
}

/*
 * Init CSSM and establish a session with the Apple TP.
 */
CSSM_TP_HANDLE tpStartup()
{
	CSSM_TP_HANDLE tpHand;
	CSSM_RETURN crtn;
	
	if(cssmStartup() == CSSM_FALSE) {
		return 0;
	}
	crtn = CSSM_ModuleLoad(&gGuidAppleX509TP,
		CSSM_KEY_HIERARCHY_NONE,
		NULL,			// eventHandler
		NULL);			// AppNotifyCallbackCtx
	if(crtn) {
		printError("CSSM_ModuleLoad(AppleTP)", crtn);
		return 0;
	}
	crtn = CSSM_ModuleAttach (&gGuidAppleX509TP,
		&vers,
		&memFuncs,				// memFuncs
		0,						// SubserviceID
		CSSM_SERVICE_TP,		// SubserviceFlags
		0,						// AttachFlags
		CSSM_KEY_HIERARCHY_NONE,
		NULL,					// FunctionTable
		0,						// NumFuncTable
		NULL,					// reserved
		&tpHand);
	if(crtn) {
		printError("CSSM_ModuleAttach(AppleTP)", crtn);
		return 0;
	}
	else {
		return tpHand;
	}
}

void tpShutdown(
	CSSM_TP_HANDLE tpHand)
{
	CSSM_RETURN crtn;
	
	crtn = CSSM_ModuleDetach(tpHand);
	if(crtn) {
		printf("Error detaching from AppleTP\n");
		printError("CSSM_ModuleDetach", crtn);
		return;
	}
	crtn = CSSM_ModuleUnload(&gGuidAppleX509TP, NULL, NULL);
	if(crtn) {
		printf("Error unloading AppleTP\n");
		printError("CSSM_ModuleUnload", crtn);
	}
}


/*
 * Cook up a CSSM_DATA with specified integer, DER style (minimum number of
 * bytes, big-endian).
 */
CSSM_DATA_PTR intToDER(unsigned theInt)
{
	CSSM_DATA_PTR DER_Data = (CSSM_DATA_PTR)CSSM_MALLOC(sizeof(CSSM_DATA));

	if(theInt < 0x100) {
		DER_Data->Length = 1;
		DER_Data->Data = (uint8 *)CSSM_MALLOC(1);
		DER_Data->Data[0] = (unsigned char)(theInt);
	}
	else if(theInt < 0x10000) {
		DER_Data->Length = 2;
		DER_Data->Data = (uint8 *)CSSM_MALLOC(2);
		DER_Data->Data[0] = (unsigned char)(theInt >> 8);
		DER_Data->Data[1] = (unsigned char)(theInt);
	}
	else if(theInt < 0x1000000) {
		DER_Data->Length = 3;
		DER_Data->Data = (uint8 *)CSSM_MALLOC(3);
		DER_Data->Data[0] = (unsigned char)(theInt >> 16);
		DER_Data->Data[1] = (unsigned char)(theInt >> 8);
		DER_Data->Data[2] = (unsigned char)(theInt);
	}
	else  {
		DER_Data->Length = 4;
		DER_Data->Data = (uint8 *)CSSM_MALLOC(4);
		DER_Data->Data[0] = (unsigned char)(theInt >> 24);
		DER_Data->Data[1] = (unsigned char)(theInt >> 16);
		DER_Data->Data[2] = (unsigned char)(theInt >> 8);
		DER_Data->Data[3] = (unsigned char)(theInt);
	}
	return DER_Data;
}

/*
 * Convert a CSSM_DATA_PTR, referring to a DER-encoded int, to a
 * uint32.
 */
uint32 DER_ToInt(const CSSM_DATA *DER_Data)
{
	uint32		rtn = 0;
	unsigned	i = 0;

	while(i < DER_Data->Length) {
		rtn |= DER_Data->Data[i];
		if(++i == DER_Data->Length) {
			break;
		}
		rtn <<= 8;
	}
	return rtn;
}

/*
 * Log CSSM error.
 */
void printError(char *op, CSSM_RETURN err)
{
	cssmPerror(op, err);
}

CSSM_BOOL appCompareCssmData(const CSSM_DATA *d1,
	const CSSM_DATA *d2)
{	
	if(d1->Length != d2->Length) {
		return CSSM_FALSE;
	}
	if(memcmp(d1->Data, d2->Data, d1->Length)) {
		return CSSM_FALSE;
	}
	return CSSM_TRUE;	
}


