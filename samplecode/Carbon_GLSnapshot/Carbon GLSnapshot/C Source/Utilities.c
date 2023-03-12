//////////////////////////////////////////////////////////////////////////////////
/*
	File:		Utilities.c

	Project:	CarbonEvent Shell

	Contains:	Implementation of the utilities
	
	Author: 	Todd Previte
	
	Copyright:	2001 Apple Computer, Inc., All Rights Reserved

	Copyright:	(c) 2002 Apple Computer, Inc., All Rights Reserved
	
	Disclaimer:	

	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
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
      

*/
//////////////////////////////////////////////////////////////////////////////////

#include "Utilities.h"

// MemAlloc
// Simple memory allocation wrapper
void* MemAlloc(unsigned long memsize)
{
    if(memsize <= 0)
	return NULL;
    else
	return (malloc(memsize));
}

// MemDealloc
// Simple memory free wrapper
bool MemDealloc(void* memblock)
{
    if(memblock == NULL)
	return FALSE;
    else
    {
	free(memblock);
	return TRUE;
    }
}

// Swap2Byte
// Byte swapper for a 2-byte variable
void Swap2Byte(short* memblock)
{
    *memblock = ((*memblock >> 8) & 0x00FF) | ((*memblock << 8) & 0xFF00);
}

// Swap4Byte
// Byte swapper for a 4-byte variable
void Swap4Byte(long* longRef)
{
    *longRef =  ((*longRef >> 24) & 0x000000FF) | 
                ((*longRef >> 8)  & 0x0000FF00) |
                ((*longRef << 8)  & 0x00FF0000) |
		((*longRef << 24) & 0xFF000000);

}

// Swap4Byte
// Byte swapper for a 8-byte variable
void Swap8Byte(double memblock)
{
// This function is non-operational
/*
    char byteArray[8];
    byteArray[0] = ((memblock >> 56) & 0x00000000000000FF);
    byteArray[1] = ((memblock >> 48) & 0x000000000000FF00);
    byteArray[2] = ((memblock >> 40) & 0x0000000000FF0000);
    byteArray[3] = ((memblock >> 32) & 0x00000000FF000000);
    byteArray[4] = ((memblock << 8)  & 0x000000FF00000000);
    byteArray[5] = ((memblock << 40) & 0x0000FF0000000000);
    byteArray[6] = ((memblock << 48) & 0x00FF000000000000);
    byteArray[7] = ((memblock << 56) & 0xFF00000000000000);
*/
}

// ByteSwapShort
// Just another name for the 2-byte swap
void ByteSwapShort(short* shortRef)
{
    *shortRef = ((*shortRef >> 8) & 0x00FF) | ((*shortRef << 8) & 0xFF00);
}

// ByteSwapInt
// Another name for the 4-byte swap
void ByteSwapInt(int* intRef)
{
    *intRef =   ((*intRef >> 24) & 0x000000FF) | 
                ((*intRef >> 8)  & 0x0000FF00) |
                ((*intRef << 8)  & 0x00FF0000) |
		((*intRef << 24) & 0xFF000000);
}

// ByteSwapLong
// Another name for the 4-byte swap
void ByteSwapLong(long* longRef)
{
    *longRef =  ((*longRef >> 24) & 0x000000FF) | 
                ((*longRef >> 8)  & 0x0000FF00) |
                ((*longRef << 8)  & 0x00FF0000) |
		((*longRef << 24) & 0xFF000000);
}

// ByteSwapFloat
// Yet another name for the 4-byte swap
void ByteSwapFloat(float* floatRef)
{
    ByteSwapLong((long*) floatRef);
}

void RectToGLRect(GLRect* newGLRect, Rect* newRect)
{
	newGLRect->x 		= newRect->left;
	newGLRect->y 		= newRect->bottom;
	newGLRect->width 	= newRect->right - newRect->left;
	newGLRect->height 	= newRect->bottom - newRect->top;
	newGLRect->center.x	= newGLRect->width / 2;
	newGLRect->center.y	= newGLRect->height / 2;
}

void GLRectToRect(GLRect* newGLRect, Rect* newRect)
{
	newRect->left		= newGLRect->x;
	newRect->bottom 	= newGLRect->height;
	newRect->right		= newGLRect->width;
	newRect->top		= newGLRect->y;
}

char* CStringToPString(char *string)
{
    char *newString = MemAlloc(strlen(string) + 1);
    long i = 0;

    for(i = 0; i < strlen(string); i++)
	newString[i+1] = string[i];
    
    newString[0] = i;
    
    return newString;
}

char* PStringToCString(char *string)
{
    char *newString = malloc(string[0] + 1);
    long i = 0;
    
    for (i = 0; i < string[0]; i++)
	    newString[i] = string[i + 1];
    
    newString[string[0]] = '\0';
    
    return newString;
}

HFSUniStr255* GetAppPath(void)
{
    FSSpec fileSpec;
    HFSUniStr255 *uniFileName;
    FSRef processRef;
    FSCatalogInfo processInfo;
    ProcessSerialNumber psn = {0, kCurrentProcess};
    uniFileName = MemAlloc(sizeof(HFSUniStr255));
    GetProcessBundleLocation(&psn, &processRef);
    FSGetCatalogInfo(&processRef, kFSCatInfoNodeFlags, &processInfo, uniFileName, &fileSpec, NULL);

    return uniFileName;
}

void GetAppFSSpec(FSSpec *fileSpec)
{
    FSRef processRef;
    FSCatalogInfo processInfo;
    ProcessSerialNumber psn = {0, kCurrentProcess};
    GetProcessBundleLocation(&psn, &processRef);
    FSGetCatalogInfo(&processRef, kFSCatInfoNodeFlags, &processInfo, NULL, fileSpec, NULL);
}

void InvertGLImage(char *imageData, long imageSize, long rowBytes)
{
    long i, j;
    long numRows = 0;
    char *tBuffer = NULL;
    
    i = j = 0;
    
    tBuffer = (char*)malloc((size_t)imageSize);
    if (!tBuffer)
    {
            printf("Out of memory!");
            return;	// continue without flip
    }
    
    numRows = imageSize / rowBytes;
    
    // Copy rows into tmp buffer one at a time, reversing their order
    for (i = 0, j = imageSize - rowBytes; i < imageSize; i += rowBytes, j -= rowBytes)
        memcpy( &tBuffer[j], &imageData[i], (size_t)rowBytes );

    // Copy tmp buffer back into original buffer
    memcpy(imageData, tBuffer, (size_t)imageSize);
    free(tBuffer);
}

void ProcessImageArray(long imgCount, GWorldPtr *imgArray)
{
    // Here's where we'll actually export the images
    OSErr 			err 	= noErr;
    ComponentResult		cErr 	= 0;
    GWorldPtr 			img	= NULL;
    GraphicsExportComponent 	geComp 	= NULL;

    FSSpec 			imgFile;
    OSType			osFileType 	= kQTFileTypeJPEG;    
    char			cstrPath[] 	= "Snapshot";
    StringPtr			fileName 	= CStringToPString(cstrPath);
    int 			fileCount 	= 0;
    int				i = 0;
    
    // NOTE: Look in QuickTimeComponents.h for osFileType definitions
    err = OpenADefaultComponent(GraphicsExporterComponentType, osFileType, &geComp);
    if (err != noErr)
    {
	printf("ERROR: %i in OpenADefaultComponent()", err);
	return; // FIXME:
    }
    
    // Loop through the GWorlds and export them
    for(i = 0; i < imgCount; i++)
    {
	// Reset the file name
	fileName = CStringToPString(cstrPath);
	// create an FSSpec for the file containing the exported image    
	GetAppFSSpec(&imgFile);

	// Loop through to find the current file name
	// This routine is slow and brute force, but it works
	while(1)
	{
	    sprintf(cstrPath, "Snapshot%i.jpg", fileCount);
	    fileName = CStringToPString(cstrPath);
	    err = FSMakeFSSpec(imgFile.vRefNum, imgFile.parID, fileName, &imgFile);
	    if(err == noErr) // File exists. Increment and continue
	    {
		fileCount++;
		continue;
	    }
	    if(err == fnfErr) // File does not exist, so we're good to go.
	    {
		break;
	    }
	    if(err != noErr && err != fnfErr)
	    {
		printf("ERROR: %i in FSMakeFSSpec()", err);
		break;
	    }
	}

	// Get the next GWorld in the buffer
	img = imgArray[i];
	
	// Set the input GWorld for the exporter
	cErr = GraphicsExportSetInputGWorld(geComp, img);
	if (cErr != noErr)
	{
	    printf("ERROR: %i in GraphicsExportSetInputGWorld()", cErr);
	    return; // FIXME:
	}
    
	// Set the output file to our FSSpec
	cErr = GraphicsExportSetOutputFile(geComp, &imgFile);
	if (cErr != noErr)
	{
	    printf("ERROR: %i in GraphicsExportSetOutputFile()", cErr);
	    return; // FIXME:
	}
    
	// Set the compression quality (needed for JPEG, not necessarily for other formats)
	cErr = GraphicsExportSetCompressionQuality(geComp, codecLosslessQuality);
	if (cErr != noErr)
	{
	    printf("ERROR: %i in GraphicsExportSetCompressionQuality()", cErr);
	    return; // FIXME:
	}
    
	// Export it
	cErr = GraphicsExportDoExport(geComp, NULL);
	if (cErr != noErr)
	{
	    printf("ERROR: %i in GraphicsExportDoExport()", cErr);
	    return; // FIXME:
	}
    }

    // finally, close the component
    if (geComp != NULL)
	    CloseComponent(geComp);
}

OSStatus MPProcessImageAsync(void* params)
{
    if(MPTaskIsPreemptive(MPCurrentTaskID()))
	ProcessImageArray(imageCount, gwBuffer);
    else
    {
	MPTaskID tMPTaskID;
	MPCreateTask( MPProcessImageAsync,
			NULL,
			16384,
			kInvalidID,
			NULL,
			NULL,
			0,
			&tMPTaskID);
    }
    return noErr;
}

