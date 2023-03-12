/*
	File:		SystemFunctions.java
	
	Description:	 * Sample showing how to send and receive AppleEvents using JDirect 3.

	Author:		EPJ

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
	* version 1.0

	* This class provides an interface for calling into SystemFramework using JDirect 3 for the functions
	* needed in this example. Each of the native methods is defined in the SystemFramework Library.
*/
package com.apple.dts.samplecode.aesendandreceive.shared;

import com.apple.mrj.jdirect.Linker;
import com.apple.mrj.macos.carbon.CarbonLock;

public class SystemFunctions implements com.apple.mrj.macos.frameworks.System {

    // From System/Library/Frameworks/Kernel.framework/Headers/mach/port.h
    public static final int MACH_PORT_NULL	= 0;
    
    public static final int MACH_PORT_RIGHT_SEND = 0;

    public static final int MACH_PORT_RIGHT_RECEIVE = 1;

    public static final int MACH_PORT_RIGHT_SEND_ONCE = 2;

    public static final int MACH_PORT_RIGHT_PORT_SET = 3;

    public static final int MACH_PORT_RIGHT_DEAD_NAME = 4;

    public static final int MACH_PORT_RIGHT_NUMBER = 5;

    // IOKit.framework/IOReturn.h
    public static final int KERN_SUCCESS = 0;
    
    static Object likage = new Linker(SystemFunctions.class);

   public static native int mach_port_allocate ( int task, int right, int [] name);

    public static native int mach_task_self();
    
    public static native int mach_port_deallocate(int task,int [] name);

}
