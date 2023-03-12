/*
	File:		AppServicesFunctions.java
	
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
	* version 1.01
	* 4/2/2002 Shipped as 'AEFramework AppleEvent Send and Receive' sample.

	* This class provides an interface for calling into AEFramework using JDirect 3 for the functions
	* needed in this example. Each of the native methods is defined in the AEFramework Library.
*/
package com.apple.dts.samplecode.aesendandreceive.shared;

import com.apple.mrj.jdirect.Linker;
import com.apple.mrj.macos.carbon.CarbonLock;

public class AppServicesFunctions implements com.apple.mrj.macos.frameworks.ApplicationServices {

    // Implicit Values for Mac OS
    public static final int kSizeOfIntType = 4; // 4 Bytes
    public static final int kSizeOfOSType = 4; // 4 Bytes
        
    // From MacTypes.h
    public static final int noErr = 0;
    
    // From MacErrors.h
    public static final int connectionInvalid = -609;

    // From AEDataModel.h
    public static final int kAutoGenerateReturnID = -1;
    
    public static final int kAnyTransactionID = 0;

    public static final int typeApplSignature = 0x7369676e; // 'sign'
                
    public static final int typeChar = 0x54455854; // 'TEXT'
        
    public static final int typeMachPort = 0x706f7274; // 'port'

    public static final int typeReplyPortAttr = 0x72657070; // 'repp'

    public static final int kAEDefaultTimeout = -1;   /* timeout value determined by AEM */
        
    public static final int kAENormalPriority = 0x00000000; /* post message at the end of the event queue */
 
    public static final int kAEWaitReply = 0x00000003; /* sender wants a reply and will wait */
    
    public static final int kAENeverInteract = 0x00000010; /* server should not interact with user */
    
    public static final int kAECanInteract = 0x00000020; /* server may try to interact with user */
    
    public static final int kAECanSwitchLayer = 0x00000040; /* interaction may switch layer */

    // From AppleEvents.h
    public static final int keyDirectObject = 0x2d2d2d2d; // '----'
    
    static Object likage = new Linker(AppServicesFunctions.class);
    
    public static short AECreateAppleEvent ( int theAEEventClass, int theAEEventID, AEAddressDescStruct target, short returnID, int transactionID, AppleEventStruct result) {
	    return AECreateAppleEvent ( theAEEventClass, theAEEventID, target.getByteArray(), returnID, transactionID, result.getByteArray());
    }

    public static short AEPutParamPtr(AppleEventStruct event, int AEKeyWord, int type, byte [] dataPtr, int size) {
	return AEPutParamPtr(event.getByteArray(), AEKeyWord, type, dataPtr, size);
    }
    
    public static short  AEGetParamPtr(AppleEventStruct event, int AEKeyWord, int desiredType, int [] actualType, byte [] dataPtr, int maxSize, int [] actualSize) {
	return AEGetParamPtr(event.getByteArray(),AEKeyWord,desiredType,actualType,dataPtr,maxSize,actualSize);
    }
    
    public static short AECreateDesc(int i, byte [] abyte0, int j, AEAddressDescStruct abyte1) {
	return AECreateDesc(i, abyte0, j, abyte1.getByteArray());
    }

    public static short AEDisposeDesc(AEAddressDescStruct AEDesc) {
	return AEDisposeDesc(AEDesc.getByteArray());
    }

    public static short AEDisposeDesc(AppleEventStruct AEDesc) {
	return AEDisposeDesc(AEDesc.getByteArray());
    }
    
    public static short AEInstallEventHandler(int eventClass, int id, AEEventHandlerClosureUPP handler, int refcon, boolean sysHandler) {
	return AEInstallEventHandler(eventClass,id,handler.getProc(),refcon,sysHandler);
    }
    
    public static short AEPutAttributePtr(AppleEventStruct event, int AEKeyword, int desiredType, int [] dataPtr, int dataSize) {
            return AEPutAttributePtr(event.getByteArray(), AEKeyword, desiredType, dataPtr, dataSize);
    }

    public static short AESendMessage(AppleEventStruct event, AppleEventStruct reply, int sendMode,
	int timeOutInTicks) {
        return AESendMessage(event.getByteArray(), reply.getByteArray(), sendMode, timeOutInTicks);
   }

    private static native short AECreateAppleEvent ( int theAEEventClass, int theAEEventID, byte [] target, short returnID, int transactionID, byte [] result);
           
    private static native short AEPutParamPtr(byte [] event, int AEKeyWord, int type, byte [] dataPtr, int size);

    private static native short AEGetParamPtr(byte [] event, int AEKeyWord, int desiredType, int [] actualType, byte [] dataPtr, int maxSize, int [] actualSize);
   
    private static native short AECreateDesc(int i, byte [] abyte0, int j, byte [] abyte1);

    private static native short AEDisposeDesc(byte [] AEDesc);
    
    private static native short AEInstallEventHandler(int eventClass, int id, int handler, int refcon, boolean sysHandler);
    
    private static native short AEPutAttributePtr(byte [] event, int AEKeyword, int desiredType, int [] dataPtr, int dataSize);

    private static native short AESendMessage(byte [] event, byte [] reply, int sendMode, int timeOutInTicks);
}
