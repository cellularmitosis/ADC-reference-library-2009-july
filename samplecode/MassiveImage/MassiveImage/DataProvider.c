/*

File: DataProvider.c

Abstract: Implementation of the custom CGDataProvider

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2007 Apple Inc., All Rights Reserved

*/

#include "DataProvider.h"

size_t MyGetBytesAtOffset(void *info, void *buffer, size_t offset, size_t count);
void MyReleaseInfo(void * info);

static CGDataProviderDirectAccessCallbacks callbacks = 
{
	NULL, // CGDataProviderGetBytePointerCallback
	NULL, // CGDataProviderReleaseBytePointerCallback
	&MyGetBytesAtOffset, // CGDataProviderGetBytesAtOffsetCallback
	&MyReleaseInfo // CGDataProviderReleaseInfoCallback
};

typedef struct
{
	// necessary for filling out the image data
	size_t width, height; 
	// Deference to UI - These allow us to provide and communicate with some form of UI.
	void *uiContext;
	DataProviderProgressCallback uiProgressCallback;
	DataProviderCancelCallback uiCancelCallback;
} DataProviderInfo;

// Simple inlines to make sure that we only call the callbacks if they were specified.
void inline SafeProgressCallback(DataProviderInfo * data, float progress)
{
	if(data->uiProgressCallback != NULL)
		(data->uiProgressCallback)(data->uiContext, progress);
}

bool inline SafeCancelCallback(DataProviderInfo *data)
{
	if(data->uiCancelCallback != NULL)
		return (data->uiCancelCallback)(data->uiContext);
	else
		return false;
}

// Callback to obtain bytes for the data provider.
// If you don't provide the number of bytes requested (given by count) then
// Quartz will cancel the request.
size_t MyGetBytesAtOffset(void * info, void * buffer, size_t offset, size_t count)
{
	DataProviderInfo * data = (DataProviderInfo *)info;
	size_t provided = count;
	
	// Deference to UI - check for cancel
	if(!SafeCancelCallback(data))
	{
		float percent = (float)(offset + count) / (float)(data->width * data->height * kBytesPerPixel);

		// Determine where in the Image CG is asking for data from
		size_t rowbytes = data->width * kBytesPerPixel;
		size_t y = offset / rowbytes, x = offset - y * rowbytes;
		size_t i;
		
		// This sample uses 32-bit pixels in XRGB format - you will want to update this loop
		// for whatever image format you choose to use.
		UInt8 *pixelData = (UInt8*)buffer;
		for(i = 0; i < count; ++x)
		{
			// This is just going to fill the image with a repeating pattern.
			// Nothing particularly special, but you would replace it with
			// whatever code would be required to fill in the image appropriately.
			// X
			pixelData[i++] = 0;
			// R
			pixelData[i++] = y & 255;
			// G
			pixelData[i++] = x & 255;
			// B
			pixelData[i++] = lrintf(percent * 255);
		}
		
		// Deference to UI - we're just signallying the controller
		SafeProgressCallback(data, percent * 100.0f);
	}
	else
	{
		// On cancel return 0 signaling that the Data Provider has no more data
		// which will in turn signal Image IO to fail trying to save the image
		provided = 0;
	}
	return provided;
}

void MyReleaseInfo(void * info)
{
	// info block was malloc()'d, so free() it.
	free(info);
}

CGDataProviderRef CreateDataProvider(size_t width, size_t height, void * context, DataProviderProgressCallback progressCallback, DataProviderCancelCallback cancelCallback)
{
	DataProviderInfo *info = (DataProviderInfo*)malloc(sizeof(DataProviderInfo));
	info->width = width;
	info->height = height;
	info->uiContext = context;
	info->uiProgressCallback = progressCallback;
	info->uiCancelCallback = cancelCallback;
	return CGDataProviderCreateDirectAccess(info, width * height * kBytesPerPixel, &callbacks);
}