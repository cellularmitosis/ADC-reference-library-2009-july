/*
	File:		AESend.java
	
	Description:	Sample showing how to send and receive AppleEvents using JDirect 3.

	Author:		EPJ, LB, MH

	Copyright: 	© Copyright 1999 - 2002 Apple Computer, Inc. All rights reserved.
	
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
	* 4/15/1999 Shipped as 'AppleEvent Send and Receive' sample.
	* version 2.0
	* 4/2/2002  Updated for Mac OS X
	
	* This class contains the code needed to send an AppleEvent and
	* to extract any reply information from the AppleEvent.
	* The reply information is fired as an java.awt.event.ActionEvent
	* to all registered ActionListeners.
*/

package com.apple.dts.samplecode.aesendandreceive.aesend;

import java.awt.AWTEventMulticaster;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import com.apple.mrj.jdirect.PointerStruct;

import com.apple.dts.samplecode.aesendandreceive.shared.*;

public class AESend
{
    /**
        * The maximum length of the keyDirectObject parameter of the Apple Event
        */
    protected static final int kMaxTextSize	= 255;
    
    /**
        * The OSType (four character code) of the target application's signature.
        * 'TSnd' in this case.
        */
    protected static final byte[] kTestTargetSig = { (byte)'T',(byte)'R',(byte)'c',(byte)'v'}; //'TRcv'
    
    /**
        * The OSType (four character code) of the event class.
        * 'Test' in this case.
        */
    protected static final int kTestClass = 0x54657374; // 'Test'
    
    /**
        * The OSType (four character code) of the event ID.
        * 'TsID' in this case.
        */
    protected static final int kTestID = 0x54734944; // 'TsID'
    
    /**
        * The list of all registered ActionEvent listeners for this object.
        * see #addActionListener
        * see #removeActionListener
        * see #fireActionEvent
        */
    protected ActionListener actionListener = null;
    
    /**
        * The reply mach port used to send events back to AESend.
        */
    private int [] replyPort = { 0 };
    
    /**
        * Allocates the mach port for the AESend example.
        */
    public AESend() throws NativeException {
        int err = SystemFunctions.mach_port_allocate(SystemFunctions.mach_task_self(), SystemFunctions.MACH_PORT_RIGHT_RECEIVE, replyPort);
        if(err != SystemFunctions.KERN_SUCCESS)
            throw new NativeException("mach_port_allocate returned an error",err);
    }
    
    /**
        * Deallocates the mach port when we are done with it.
        */
    public void finalize() {
        if(replyPort[1] != 0)
            SystemFunctions.mach_port_deallocate(SystemFunctions.mach_task_self(),replyPort);
            replyPort[1] = 0;
    }
    
    /**
        * Sends a string to our target application via Apple Events.
        * param theText, the string to pass along to the target application.
        * exception if any problem occured while sending the AppleEvent.
        */
    public void sendAppleEvent(String theText) throws NativeException
    {
        AEAddressDescStruct targetAddrDesc = new AEAddressDescStruct();
        AppleEventStruct event = new AppleEventStruct();
        AppleEventStruct reply = new AppleEventStruct();
        short err;
        int[] actualType = new int[1];
        int[] actualSize = new int[1];
        byte[] replyText = new byte[kMaxTextSize];
        NativeException error = null;
    
        try
        {
            //Create a new descriptor using the target applications 4 character type
            err = AppServicesFunctions.AECreateDesc(AppServicesFunctions.typeApplSignature, kTestTargetSig, AppServicesFunctions.kSizeOfOSType, targetAddrDesc);
            ErrorHandler.checkError(err, "Error returned by AECreateDesc()");
    
            //Create the apple event with the class, event id, and address of the apple event target. 
            err = AppServicesFunctions.AECreateAppleEvent(kTestClass, kTestID, targetAddrDesc, (short)AppServicesFunctions.kAutoGenerateReturnID, AppServicesFunctions.kAnyTransactionID, event);
            ErrorHandler.checkError(err, "Error returned by AECreateAppleEvent()");
    
            if (theText != null && !theText.equals("") ) // if string is not null, and not empty
            {
                //Turn the String into a byte array
                byte[] theData = theText.getBytes();
                                                    
                //Add the string as a direct object parameter to the event
                err = AppServicesFunctions.AEPutParamPtr(event, AppServicesFunctions.keyDirectObject, AppServicesFunctions.typeChar, theData, theData.length);
                ErrorHandler.checkError(err, "Error returned by AEPutParamPtr()");
    
                //Add the reply port as an attribute to the event
                err = AppServicesFunctions.AEPutAttributePtr(event, AppServicesFunctions.typeReplyPortAttr, AppServicesFunctions.typeMachPort, replyPort, AppServicesFunctions.kSizeOfIntType);
    
                //Send the apple event with an empty reply event. Wait for reply, user interaction allowed
                    err = AppServicesFunctions.AESendMessage(event, reply, AppServicesFunctions.kAEWaitReply + AppServicesFunctions.kAECanInteract + AppServicesFunctions.kAECanSwitchLayer, AppServicesFunctions.kAEDefaultTimeout);
    
                if ( err == AppServicesFunctions.connectionInvalid )
                {
                        throw new NativeException("The target application is not running.", err);
                }
                
                ErrorHandler.checkError(err, "Error returned by AESend()");
                
                //Retrieve keyDirectObject parameter from reply structure
                err = AppServicesFunctions.AEGetParamPtr(reply, AppServicesFunctions.keyDirectObject, AppServicesFunctions.typeChar, actualType, replyText, kMaxTextSize, actualSize);
    
                ErrorHandler.checkError(err, "Error returned by AEGetParamPtr()");
    
                //Create a String from the byte array of returned data.
                String text = new String(replyText, 0, actualSize[0]);
                
                //Fire an ActionEvent with the reply data so registered ActionListeners can see the AppleEvent being received.
                fireActionEvent(text);
            }
        }
        catch (NativeException exc)
        {
                error = exc;
        }
        finally
        {
                AppServicesFunctions.AEDisposeDesc(targetAddrDesc);
                AppServicesFunctions.AEDisposeDesc(event);
                AppServicesFunctions.AEDisposeDesc(reply);
        }
        
        if (error != null)
                throw error;
    }
    
    /**
        * Adds the specified action listener to receive action events from this object.
        * param l the action listener
        */
        public void addActionListener(ActionListener l)
        {
                actionListener = AWTEventMulticaster.add(actionListener, l);
        }
    
    /**
        * Removes the specified action listener so it no longer receives
        * action events from this object.
        * param l the action listener
        */
        public void removeActionListener(ActionListener l)
        {
                actionListener = AWTEventMulticaster.remove(actionListener, l);
        }
    
    /**
        * Fire an action event to the listeners.
        * param the action command associated with the event.
        * In this case it will be the text from the AppleEvent.
        */
        protected void fireActionEvent(String message)
        {
                if (actionListener != null)
                        actionListener.actionPerformed(new ActionEvent(this, ActionEvent.ACTION_PERFORMED, message));
        }
}