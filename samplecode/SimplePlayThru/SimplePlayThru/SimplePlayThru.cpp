/*
 
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
 
 Copyright © 2004 Apple Computer, Inc., All Rights Reserved
 
 */ 


#include "SimplePlayThru.h"
#pragma mark ---Public Methods---

SimplePlayThru::SimplePlayThru()
:mInputDevice(0),
mOutputDevice(0)
{
	OSStatus err = noErr;
	err =Init();
    if(err)
		exit(1);	
}

SimplePlayThru::~SimplePlayThru()
{
	DisposeAUGraph(mGraph);
}

OSStatus SimplePlayThru::Init()
{	
    OSStatus err = noErr;
	ComponentDescription desc;
	
	//There are several different types of Audio Units.
	//Some audio units serve as Outputs, Mixers, or DSP
	//units. See AUComponent.h for listing
	desc.componentType = kAudioUnitType_Output;
	
	//Every Component has a subType, which will give a clearer picture
	//of what this components function will be.
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	
	//all Audio Units in AUComponent.h must use 
	//"kAudioUnitManufacturer_Apple" as the Manufacturer
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	//Make a Audio Unit Graph
    err = NewAUGraph(&mGraph);  
	
	//////////////////////////
	///MAKE NODES
	//This creates a node in the graph that is an AudioUnit, using
	//the supplied ComponentDescription to find and open that unit
	
    err = AUGraphNewNode(mGraph, &desc, 0, NULL, &mInputNode);
	checkErr(err);	
	
	//////////////////////////
    //MAKE Connections
    //Connects the Input Node's output bus (1) to the Output Nodes input bus (0)
    //Input (AUHAL)--->Output (AUHAL)
	err = AUGraphConnectNodeInput(	mGraph,mInputNode, 1, mInputNode, 0);
	checkErr(err);
    
	//////////////////////////
    //Opens the Graph, AudioUnits are opened but not initialized    
    err = AUGraphOpen(mGraph);
	checkErr(err);
	
	err = SetupAUHAL();
	checkErr(err);
	
	//Finally init the graph and it's single node
	err = AUGraphInitialize(mGraph);  
	checkErr(err);
		
	return err;	
}

//Checks to see if the play thru device is valid for this sample code
//a valid device has both input and output for the "simple" case
Boolean SimplePlayThru::DeviceCheck()
{
	Boolean valid = false;
	OSStatus err =noErr;
	
	UInt32 size;
	UInt32 hasIO;	
	
	if((mInputDevice ==0) || (mOutputDevice==0)){
		size = sizeof(AudioDeviceID);
		
		err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
									   &size,  
									   &mInputDevice);
		err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
									   &size,  
									   &mOutputDevice);
	}
	
	//Must be the same device!
	if(	mOutputDevice == mInputDevice)
	{
		//check to see if device has IO in both directions (in/out)
		hasIO=0;
		size = sizeof(hasIO);
		err =AudioUnitGetProperty(HALUnit,
								  kAudioOutputUnitProperty_HasIO,
								  kAudioUnitScope_Input,
								  1, // input element
								  &hasIO,
								  &size);
		if(hasIO)				 
			err = AudioUnitGetProperty(HALUnit,
									   kAudioOutputUnitProperty_HasIO,
									   kAudioUnitScope_Output,
									   0, // input element
									   &hasIO,
									   &size);
		
		if(hasIO)
			valid = true;
		
	}
	return valid;
}


char* SimplePlayThru::GetDeviceName(Boolean IsInput)
{
	OSStatus err =noErr;
	char *name;
	UInt32 size;
	
	AudioDeviceID Device = (IsInput == true) ? mInputDevice :mOutputDevice ;
	
	if(Device ==0){
		size = sizeof(AudioDeviceID);
		if(IsInput)
			err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
										   &size,  
										   &mInputDevice);
		else
			err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
										   &size,  
										   &mOutputDevice);
		checkErr(err); 
		
		Device = (IsInput == true) ? mInputDevice :mOutputDevice ;
	}
	
	AudioDeviceGetPropertyInfo(	Device,0,IsInput,kAudioDevicePropertyDeviceName,&size, NULL);
	name = (char *)malloc(size);
	
	err= AudioDeviceGetProperty(Device, 0, IsInput, kAudioDevicePropertyDeviceName, &size,name);
	
	
	return name;
}
#pragma mark -
#pragma mark --- Operation---

OSStatus SimplePlayThru::Start()
{
	
	OSStatus err =AUGraphStart (mGraph);
	return err;
	
}

OSStatus SimplePlayThru::Stop()
{
	
	OSStatus err= noErr;
	
	if(IsRunning())
		err =AUGraphStop(mGraph);
	return err;
	
}
Boolean SimplePlayThru::IsRunning()
{		
	OSStatus err= noErr;
	Boolean running=false;
	
	err = AUGraphIsRunning(mGraph,&running);
	checkErr(err);
	
	return running;
}
#pragma mark -
#pragma mark --Private methods---
//Set up the AUHAL for simple play thru
OSStatus SimplePlayThru::SetupAUHAL()
{
	OSStatus err= noErr;
	UInt32 enableIO;
	
	
	//Get Audio Unit from AUGraph node
	err = AUGraphGetNodeInfo(mGraph, mInputNode, NULL, NULL, NULL, &HALUnit  );   	
	
	///////////////
	//ENABLE IO (INPUT & OUTPUT)
	//You must enable the Audio Unit (AUHAL) for input and output for the same  device.
	
	//When using AudioUnitSetProperty the 4th parameter in the method
	//refer to an AudioUnitElement.  When using an AudioOutputUnit
	//for input the element will be '1' and the output element will be '0'.	
	
	//Enable input on the AUHAL
	enableIO = 1;
	err = AudioUnitSetProperty(HALUnit,
								kAudioOutputUnitProperty_EnableIO,
								kAudioUnitScope_Input,
								1, // input element
								&enableIO,
								sizeof(enableIO));
	checkErr(err);
	
	
	//Enable Output on the AUHAL
	enableIO = 1;
	err = AudioUnitSetProperty(HALUnit,
							  kAudioOutputUnitProperty_EnableIO,
							  kAudioUnitScope_Output,
							  0,   //output element
							  &enableIO,
							  sizeof(enableIO));
	
	return err;
}
