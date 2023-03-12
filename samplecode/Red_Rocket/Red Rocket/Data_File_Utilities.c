/*
 *  Data_File_Utilities.c
 *  Red Rocket
 *
 *  Created by ggs on Sat May 19 2001.

	Copyright:	Copyright © 2001 Apple Computer, Inc., All Rights Reserved

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
 *
 */

#include <Carbon/Carbon.h>

#include "Data_File_Utilities.h"

// ==================================
// public

char * ReadDataFile (long * pBufferSize, FSSpec * pfsspecData)
{
	NavReplyRecord replyNav;
	NavTypeListHandle hTypeList = (NavTypeListHandle) NewHandleClear (sizeof (NavTypeList) + sizeof (OSType) * 1);
	AEKeyword theKeyword;
	DescType actualType;
	Size actualSize;
	OSStatus err = noErr;
    short fileRef;
    char * pFileBuffer;

	HLock ((Handle) hTypeList);
	(**hTypeList).osTypeCount = 2;
	(**hTypeList).osType[0] = 'TEXT';
	(**hTypeList).osType[1] = '????';

    // select sprite file
	err = NavChooseFile (NULL, &replyNav, NULL, NULL, NULL, NULL, hTypeList, NULL); 
	if ((err == noErr) && (replyNav.validRecord))
		err = AEGetNthPtr (&(replyNav.selection), 1, typeFSS, &theKeyword, &actualType,
						   pfsspecData, sizeof (FSSpec), &actualSize);
	NavDisposeReply (&replyNav);
	if (err != noErr)
		return false;
    FSpOpenDF(pfsspecData, fsRdPerm, &fileRef);
    GetEOF(fileRef, pBufferSize);
    pFileBuffer = (char *) NewPtrClear (*pBufferSize);
    FSRead (fileRef, pBufferSize, pFileBuffer);
    FSClose (fileRef);
	return pFileBuffer;
}

// ---------------------------------

void DisposeDataBuffer (char ** ppBuffer)
{
	DisposePtr ((Ptr)*ppBuffer);
	*ppBuffer = NULL;
}

// ---------------------------------

long GetLongFromData (char * pBuffer, long * pIndex, long bufferSize)
{
    char cstr [255];
    short j = 0;
    long num;
    
    long index = *pIndex;
    
    // dump comment
    while ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
    {
	while ((pBuffer[index] != '\r') && (index < bufferSize))
	    index++;
	index++;
    }
    while ((pBuffer[index] != '\r') && (index < bufferSize))
    {
	if ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
	{
	    while ((pBuffer[index] != '\r') && (index < bufferSize))
		index++;
	}
	else
	    cstr [j++] = pBuffer[index++];
    }
    index++;
    cstr [j] = 0;
    num = atoi (cstr);

    *pIndex = index;
    
    return num;
}

// ---------------------------------

void GetCStrFromData (char * pBuffer, long * pIndex, long bufferSize, char * cstr)
{
    short j = 0;
    
    long index = *pIndex;
    
    // dump comment
    while ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
    {
	while ((pBuffer[index] != '\r') && (index < bufferSize))
	    index++;
	index++;
    }
    while ((pBuffer[index] != '\r') && (index < bufferSize))
    {
	if ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
	{
	    while ((pBuffer[index] != '\r') && (index < bufferSize))
		index++;
	}
	else
	    cstr [j++] = pBuffer[index++];
    }
    index++;
    cstr [j] = 0;

    *pIndex = index;
}

// ---------------------------------

float GetFloatFromData (char * pBuffer, long * pIndex, long bufferSize)
{
    char cstr [256];
    short j = 0;
    float num;
	
    long index = *pIndex;
    
    // dump comment
    while ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
    {
	while ((pBuffer[index] != '\r') && (index < bufferSize))
	    index++;
	index++;
    }
    while ((pBuffer[index] != '\r') && (index < bufferSize))
    {
	if ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
	{
	    while ((pBuffer[index] != '\r') && (index < bufferSize))
		index++;
	}
	else
	    cstr [j++] = pBuffer[index++];
    }
    index++;
    cstr [j] = 0;
    num = atof (cstr);

    *pIndex = index;
    
    return num;
}

// ---------------------------------

Boolean GetBooleanFromData (char * pBuffer, long * pIndex, long bufferSize)
{
    char cstr [255];
    short j = 0;
    long num;
    
    long index = *pIndex;

    // dump comment
    while ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
    {
	while ((pBuffer[index] != '\r') && (index < bufferSize))
	    index++;
	index++;
    }
    while ((pBuffer[index] != '\r') && (index < bufferSize))
    {
	if ((pBuffer[index] == '/') && (pBuffer[index + 1] == '/')) // comment skip rest of line
	{
	    while ((pBuffer[index] != '\r') && (index < bufferSize))
		index++;
	}
	else
	    cstr [j++] = pBuffer[index++];
    }
    index++;
    cstr [j] = 0;
    num = atoi (cstr);

    *pIndex = index;
    if (num != 0)
	return true;
    else
	return false;
}

