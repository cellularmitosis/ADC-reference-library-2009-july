/*

File: AppController.m

Abstract:	Demonstrates how to enumerate audio devices attached
			to the system and how to handle device notifications

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import <CoreAudio/CoreAudio.h>
#import "AppController.h"


static OSStatus GetAudioDevices( Ptr * devices, UInt16 * devicesAvailable )
{
    OSStatus	err = noErr;
    UInt32 		outSize;
    Boolean		outWritable;
    
    // find out how many audio devices there are, if any
    err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &outSize, &outWritable);	
    if ( err != noErr ) 
		return err;
   
    // calculate the number of device available
	*devicesAvailable = outSize / sizeof(AudioDeviceID);						
    if ( *devicesAvailable < 1 )
	{
		fprintf( stderr, "No devices\n" );
		return err;
	}
    
    // make space for the devices we are about to get
    *devices = (Ptr) malloc(outSize);		
    	
    memset( *devices, 0, outSize );			
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &outSize, (void *) *devices);	
    if (err != noErr )
      return err;

    return err;
}

OSStatus AHPropertyListenerProc(AudioHardwarePropertyID inPropertyID, void *inClientData)
{
	AppController *app = (AppController*)inClientData;
    switch (inPropertyID)
	{
/*
 * These are the other types of notifications we might receive, however, they are beyond
 * the scope of this sample and we ignore them.
 *
        case kAudioHardwarePropertyDefaultInputDevice:
			fprintf(stderr, "AHPropertyListenerProc: default input device changed\n");
        break;
			
        case kAudioHardwarePropertyDefaultOutputDevice:
			fprintf(stderr, "AHPropertyListenerProc: default output device changed\n");
		break;
			
        case kAudioHardwarePropertyDefaultSystemOutputDevice:
			fprintf(stderr, "AHPropertyListenerProc: default system output device changed\n");
		break;
*/
        case kAudioHardwarePropertyDevices:
			[app performSelectorOnMainThread:@selector(updateDeviceList) withObject:nil waitUntilDone:NO];
		break;
			
		default:
			fprintf(stderr, "AHPropertyListenerProc: unknown message\n");
		break;
    }
	return noErr;
}

@implementation AppController

-(void) awakeFromNib
{
	// create empty array to hold device info
	deviceArray = [[NSMutableArray alloc] init];
	if(!deviceArray)
		return;

	// generate initial device list
	[self updateDeviceList];
	
	// install device notification callback
	AudioHardwareAddPropertyListener(kAudioHardwarePropertyDevices, AHPropertyListenerProc, self );
}

- (void)updateDeviceList
{
    OSStatus	err = noErr;
    UInt32 		outSize = 0;
	UInt32      theNumberInputChannels  = 0;
	UInt32      theNumberOutputChannels = 0;
	UInt32      theIndex = 0;
    UInt16		devicesAvailable = 0;
	UInt16		loopCount = 0;
    AudioDeviceID	*devices = NULL;
	AudioBufferList *theBufferList = NULL;
	CFNumberRef		tempNumberRef = NULL;
	CFStringRef		tempStringRef = NULL;
	
	// clear out any current entries in device array
	[deviceArray removeAllObjects];
	
	// fetch a pointer to the list of available devices
	if(GetAudioDevices((Ptr*)&devices, &devicesAvailable) != noErr)
		return;
	
	// iterate over each device gathering information
	for(loopCount = 0; loopCount < devicesAvailable; loopCount++)
	{
		CFMutableDictionaryRef theDict = NULL;
		UInt16 deviceID = devices[loopCount];
		
		// create dictionary to hold device info
		theDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if ( theDict == NULL )
		{
			fprintf(stderr, "Dictionary Creation Failed\n" );
			return;
		}

		// save device id
		tempNumberRef = CFNumberCreate(kCFAllocatorDefault,kCFNumberShortType,&deviceID);
		if(tempNumberRef)
		{
			CFDictionarySetValue(theDict, CFSTR("id"), tempNumberRef);
			CFRelease(tempNumberRef);
		}
		
		// get device name
		outSize = sizeof(CFStringRef);
		err = AudioDeviceGetProperty( devices[loopCount], 0, 0, kAudioDevicePropertyDeviceNameCFString, &outSize, &tempStringRef);
		if(tempStringRef)
		{
			CFDictionarySetValue(theDict, CFSTR("name"), tempStringRef);
			CFRelease(tempStringRef);
		}
		
		// get number of input channels
		outSize = 0;
		theNumberInputChannels = 0;
		err = AudioDeviceGetPropertyInfo( devices[loopCount], 0, 1, kAudioDevicePropertyStreamConfiguration, &outSize, NULL);
		if((err == noErr) && (outSize != 0))
		{
			theBufferList = (AudioBufferList*)malloc(outSize);
			if(theBufferList != NULL)
			{
				// get the input stream configuration
				err = AudioDeviceGetProperty( devices[loopCount], 0, 1, kAudioDevicePropertyStreamConfiguration, &outSize, 
                                                        theBufferList);
				if(err == noErr)
				{
					// count the total number of input channels in the stream
					for(theIndex = 0; theIndex < theBufferList->mNumberBuffers; ++theIndex)
                            theNumberInputChannels += theBufferList->mBuffers[theIndex].mNumberChannels;
				}
				free(theBufferList);
				tempNumberRef = CFNumberCreate(kCFAllocatorDefault,kCFNumberLongType,&theNumberInputChannels);
				if(tempNumberRef)
					CFDictionarySetValue(theDict, CFSTR("ich"), tempNumberRef);
			}
		}

		// get number of output channels
		outSize = 0;
		theNumberOutputChannels = 0;
		err = AudioDeviceGetPropertyInfo( devices[loopCount], 0, 0, kAudioDevicePropertyStreamConfiguration, &outSize, NULL);
		if((err == noErr) && (outSize != 0))
		{
			theBufferList = (AudioBufferList*)malloc(outSize);
			if(theBufferList != NULL)
			{
				// get the input stream configuration
				err = AudioDeviceGetProperty( devices[loopCount], 0, 0, kAudioDevicePropertyStreamConfiguration, &outSize, 
                                                        theBufferList);
				if(err == noErr)
				{
					// count the total number of output channels in the stream
					for(theIndex = 0; theIndex < theBufferList->mNumberBuffers; ++theIndex)
                            theNumberOutputChannels += theBufferList->mBuffers[theIndex].mNumberChannels;
				}
				free(theBufferList);
				tempNumberRef = CFNumberCreate(kCFAllocatorDefault,kCFNumberLongType,&theNumberOutputChannels);
				if(tempNumberRef)
					CFDictionarySetValue(theDict, CFSTR("och"), tempNumberRef);
			}
		}
		
		[deviceArray addObject:(NSDictionary*)theDict];
		CFRelease(theDict);
	}
	[myTable reloadData];
}

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [deviceArray count];
}

- (id)tableView:(NSTableView *)aTableView
      objectValueForTableColumn:(NSTableColumn *)aTableColumn
      row:(int)rowIndex
{
	NSDictionary *deviceDict = NULL;
	
	deviceDict = [deviceArray objectAtIndex:rowIndex];
	return [deviceDict objectForKey:[aTableColumn identifier]];
}

@end
