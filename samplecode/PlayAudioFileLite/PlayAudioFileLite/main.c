/*
*	File:		main.c
* 
*	Contains:	Demonstrates how to play an audio file using the Default Output Audio Unit
*	
*	Version:	1.1
* 
*	Created:	2004-01-23
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include <unistd.h> //for usleep

AudioConverterRef converter;
void *gSourceBuffer;

AudioFileID *gSourceAudioFileID;
UInt64 gTotalPacketCount=0;
UInt64 gFileByteCount =0;
UInt32 gMaxPacketSize =0;
UInt64 gPacketOffset=0;

Boolean gIsPlaying = FALSE;


#define checkStatus( err) \
    if(err) {\
        printf("Error: %s ->  %s: %d\n", (char *)&err,__FILE__, __LINE__);\
        fflush(stdout);\
        return err; \
    }         
 
                                       
void PrintStreamDesc (AudioStreamBasicDescription *inDesc)
{
	if (!inDesc) {
		printf ("Can't print a NULL desc!\n");
		return;
	}
	
	printf ("- - - - - - - - - - - - - - - - - - - -\n");
	printf ("  Sample Rate:%f\n", inDesc->mSampleRate);
	printf ("  Format ID:%s\n", (char*)&inDesc->mFormatID);
	printf ("  Format Flags:%lX\n", inDesc->mFormatFlags);
	printf ("  Bytes per Packet:%ld\n", inDesc->mBytesPerPacket);
	printf ("  Frames per Packet:%ld\n", inDesc->mFramesPerPacket);
	printf ("  Bytes per Frame:%ld\n", inDesc->mBytesPerFrame);
	printf ("  Channels per Frame:%ld\n", inDesc->mChannelsPerFrame);
	printf ("  Bits per Channel:%ld\n", inDesc->mBitsPerChannel);
	printf ("- - - - - - - - - - - - - - - - - - - -\n");
}


OSStatus setupAudioUnit(AudioUnit *theOutputUnit){
        OSStatus err;

        //An Audio Unit is a OS component
        //The component description must be setup, then used to 
        //initialize an AudioUnit
	ComponentDescription desc;  


        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_DefaultOutput;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
                	
	Component comp = FindNextComponent(NULL, &desc);  //Finds an component that meets the desc spec's
		if (comp == NULL) exit (-1);
		
	err = OpenAComponent(comp, theOutputUnit);  //gains access to the services provided by the component
		if (err)  exit (-1);

	// Initialize AudioUnit 
	verify_noerr(AudioUnitInitialize(*theOutputUnit));


        return err;


}




OSStatus MatchAUFormats (AudioUnit *theUnit,AudioStreamBasicDescription *theDesc ,UInt32 theInputBus)
{
	UInt32 size = sizeof (AudioStreamBasicDescription);
        memset(theDesc, 0, size);
        Boolean             outWritable;                            
        
        //Gets the size of the Stream Format Property and if it is writable
        AudioUnitGetPropertyInfo(*theUnit,  
                                    kAudioUnitProperty_StreamFormat,
                                    kAudioUnitScope_Output, 
                                    0, 
                                    &size, 
                                    &outWritable);
        //Get the current stream format of the output
	OSStatus result = AudioUnitGetProperty (*theUnit,
							kAudioUnitProperty_StreamFormat,
							kAudioUnitScope_Output,
							0,
							theDesc,
							&size);
	checkStatus(result);	
        //Set the stream format of the output to match the input
	result = AudioUnitSetProperty (*theUnit,
							kAudioUnitProperty_StreamFormat,
							kAudioUnitScope_Input,
							theInputBus,
							theDesc,
							size);
                                                        
	
	return result;
}


OSStatus MakeAUConverter( AudioFileID *musicFileID,AudioConverterRef *conv,AudioStreamBasicDescription *inASBD, AudioStreamBasicDescription *outASBD){
    OSStatus err;
    
    
    //To Do: Add some error checking to make sure the formats are valid
    
    err = AudioConverterNew( inASBD,outASBD , conv);
    checkStatus(err);
    
    //Get Magic Cookie info(if exists)  and pass it to converter
    //Not all files have magic cooke information so this step might
    //not be necessary depending on your audio file type (AIFF, MP3, etc.)
    UInt32	magicCookieSize = 0;
    err = AudioFileGetPropertyInfo(	*musicFileID,
                                        kAudioFilePropertyMagicCookieData,
                                        &magicCookieSize,
                                        NULL); 
  
     if (err == noErr)
    {
        void 	*magicCookie = calloc (1, magicCookieSize);
        if (magicCookie) 
        {
            //Get Magic Cookie data from Audio File
            err = AudioFileGetProperty (	*musicFileID, 
                                            kAudioFilePropertyMagicCookieData, 
                                            &magicCookieSize, 
                                            magicCookie);       
                                        
            // Give the AudioConverter the magic cookie decompression params if there are any
            if (err == noErr)
            {
                err = AudioConverterSetProperty(	*conv, 
                                                    kAudioConverterDecompressionMagicCookie, 
                                                    magicCookieSize, 
                                                    magicCookie);
            }
             err = noErr;
            if (magicCookie) free(magicCookie);
        }
    }else //this is OK because some audio data doesn't need magic cookie data
        err = noErr;

    

    return err;
}

OSStatus getFileInfo(FSRef *fileRef, AudioFileID *fileID,  AudioStreamBasicDescription *fileASBD, const char *fileName){
    OSStatus err= noErr;
    UInt32 size;
    
    
     FSPathMakeRef ((const UInt8 *)fileName, fileRef, 0); //Obtain filesystem reference to the file
     err = AudioFileOpen(fileRef, fsRdPerm,0,fileID);   //Obtain AudioFileID
     
     
    gSourceAudioFileID = fileID;  //gloabal pointer to AudioFileID
     
     size = sizeof(AudioStreamBasicDescription);
     memset(fileASBD, 0, size);
     //Fetch the AudioStreamBasicDescription of the audio file.
     err = AudioFileGetProperty(*fileID, kAudioFilePropertyDataFormat, &size, fileASBD); 
     checkStatus(err);

    printf("File format is:\n");
    PrintStreamDesc(fileASBD);
  
    //Get total packet count, byte count, and max packet size
    //Theses values will be used later when grabbing data from the audio file
  
    size = sizeof(gTotalPacketCount);
    err = AudioFileGetProperty(*fileID, kAudioFilePropertyAudioDataPacketCount, &size, &gTotalPacketCount);
    checkStatus(err);
    
    size = sizeof(gFileByteCount);
    err = AudioFileGetProperty(*fileID, kAudioFilePropertyAudioDataByteCount, &size, &gFileByteCount);
    checkStatus(err);
    
    size = sizeof(gMaxPacketSize);
    err = AudioFileGetProperty(*fileID, kAudioFilePropertyMaximumPacketSize, &size, &gMaxPacketSize);
    checkStatus(err);

    return err;
}




OSStatus ACComplexInputProc 	(	AudioConverterRef                           inAudioConverter,
                                        UInt32                                      *ioNumberDataPackets,
                                        AudioBufferList                                     *ioData,
                                        AudioStreamPacketDescription	**outDataPacketDescription,
                                        void*                                               inUserData)
{
    OSStatus    err = noErr;
    
    
        //appGlobalsPtr		globals = (appGlobalsPtr) inUserData;
    UInt32          bytesReturned = 0;
    
    // initialize in case of failure
    ioData->mBuffers[0].mData = NULL;			
    ioData->mBuffers[0].mDataByteSize = 0;

    // if there are not enough packets to satisfy request, then read what's left
    if (gPacketOffset + *ioNumberDataPackets > gTotalPacketCount)
        *ioNumberDataPackets = gTotalPacketCount - gPacketOffset;
            
    // do nothing if there are no packets available
    if (*ioNumberDataPackets)
    {
        if (gSourceBuffer != NULL) {
            free(gSourceBuffer);
            gSourceBuffer = NULL;
        }
        
        gSourceBuffer = (void *) calloc (1, *ioNumberDataPackets * gMaxPacketSize);        
        
        //read the amount of data needed(ioNumberDataPackets) from AudioFile
        err = AudioFileReadPackets (*gSourceAudioFileID, false, &bytesReturned, NULL, gPacketOffset, 
                                    ioNumberDataPackets, gSourceBuffer);
        
        if(err){
              checkStatus(err);
              gIsPlaying =FALSE;   //end of data reached;
    
        }	
        
        gPacketOffset += *ioNumberDataPackets;	// keep track of where we want to read from next time
        
        ioData->mBuffers[0].mData = gSourceBuffer;		// tell the Audio Converter where it's source data is
        ioData->mBuffers[0].mDataByteSize = bytesReturned;		// tell the Audio Converter how much source data there is
    }	
    else
    {
        // there aren't any more packets to read at this time
        ioData->mBuffers[0].mData = NULL;			
        ioData->mBuffers[0].mDataByteSize = 0;
        gIsPlaying=FALSE;
    }	

    // it's not an error if we just read the remainder of the file
    if (err == eofErr && *ioNumberDataPackets)
        err = noErr;
    
    
    
    return err;   

}


OSStatus fileRenderProc(void 						*inRefCon, 
                                                                        AudioUnitRenderActionFlags		*inActionFlags,
                                                                        const AudioTimeStamp 			*inTimeStamp, 
                                                                        UInt32                                  inBusNumber,
                                                                        UInt32                                  inNumFrames, 
                                                                        AudioBufferList                         *ioData)
{
    OSStatus err= noErr;
    void *inInputDataProcUserData=NULL;
    AudioStreamPacketDescription* outPacketDescription =NULL;
    err = AudioConverterFillComplexBuffer(converter, ACComplexInputProc ,inInputDataProcUserData , &inNumFrames, ioData, outPacketDescription);

    /*Parameters for AudioConverterFillComplexBuffer()

    converter - the converter being used

    ACComplexInputProc() - input procedure to supply data to the Audio Converter

    inInputDataProcUserData - Used to hold any data that needs to be passed on.  Not needed in this example.
    
    inNumFrames - The amount of requested data.  On output, this
    number is the amount actually received.

    ioData - Buffer of the converted data recieved on return
    
    outPacketDescription - contains the format of the returned data.  Not used in this example.
    */

    checkStatus(err);

    return err;
}




OSStatus setupCallbacks(AudioUnit *theOutputUnit, AURenderCallbackStruct *renderCallback){  
OSStatus err= noErr;

    memset(renderCallback, 0, sizeof(AURenderCallbackStruct));
    
    renderCallback->inputProc = fileRenderProc;
    renderCallback->inputProcRefCon =0;
    
    //Sets the callback for the Audio Unit 
    err = AudioUnitSetProperty (*theOutputUnit, 
                            kAudioUnitProperty_SetRenderCallback, 
                            kAudioUnitScope_Input, 
                            0,
                            renderCallback, 
                            sizeof(AURenderCallbackStruct));

    checkStatus(err);

    return err;
}

void CleanUp(AudioUnit *theOutputUnit,AudioFileID *fileID){
        printf("finished playing\n");

        //Cleaning anything allocated.
        
        AudioFileClose(*fileID);
        
        if (gSourceBuffer != NULL) {
            free(gSourceBuffer);
            gSourceBuffer = NULL;
        }
        
        AudioConverterDispose(converter);
        AudioOutputUnitStop(*theOutputUnit);//you must stop the audio unit
        AudioUnitUninitialize (*theOutputUnit);
        CloseComponent(*theOutputUnit);
}

int main (int argc, const char * argv[]) {
    
    AudioUnit theOutputUnit;
    AURenderCallbackStruct  renderCallback;
    OSStatus err = noErr;
    
    AudioStreamBasicDescription fileASBD, outputASBD;
    AudioFileID musicFileID;
    FSRef fileRef;
    char *fileName ="/System/Library/Sounds/Submarine.aiff";  //give full path of any properly formatted audio file
  
    if(argc == 2)
        fileName = (char *)argv[1];
        
    
    err = setupAudioUnit(&theOutputUnit);
    checkStatus(err);    
        
    err = MatchAUFormats(&theOutputUnit,&outputASBD, 0); //"0" is the output bus, use "1" for the input bus
    checkStatus(err);
    
    err = getFileInfo(&fileRef, &musicFileID, &fileASBD, fileName);
    checkStatus(err);    
    
    err = MakeAUConverter(&musicFileID, &converter,&fileASBD, &outputASBD );
    checkStatus(err);
    
    err = setupCallbacks(&theOutputUnit, &renderCallback);
    checkStatus(err);
    
    printf("\n\nOutput format of Audio is:\n");
    PrintStreamDesc(&outputASBD);
    err =AudioOutputUnitStart(theOutputUnit);
    
    checkStatus(err);
    printf("\n\nPlay has started\n");
    
    gIsPlaying=TRUE;
    while (gIsPlaying) {
		usleep (250000);  //check every 1/4 of a second to see if audio is done playing
    }	

    
    CleanUp(&theOutputUnit, &musicFileID);
	
     
    return 0;
}
