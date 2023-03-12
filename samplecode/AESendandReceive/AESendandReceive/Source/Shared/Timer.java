/*
	File:		Timer.java
	
	Description:	 * Sample showing how to send and receive AppleEvents using JDirect 3.

	Author:		LB

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

	* A simple timer class.
	* see TimerCallback

*/
package com.apple.dts.samplecode.aesendandreceive.shared;

public class Timer
{
	/**
	 * Instantiates a new Timer.
	 * param the amount of time for the timer to run before calling
	 * the callback (time in milliseconds).
	 * param the TimerCallback object to call when the time is up.
	 */
	public Timer(int sleepTime, TimerCallback callback)
	{
		this.sleepTime = sleepTime;
		this.callback = callback;
		runObj = new RunObj();
	}
	
	/**
	 * Starts the timer running.
	 * If the timer is currently running, it will be reset.
	 * see #stop
	 */
	public void start()
	{
		if (thread == null || !thread.isAlive())
		{
			thread = new Thread(runObj);
			thread.start();
		}
		else
		{
			thread.interrupt();
			thread = null;
			start();
		}
	}
	
	/**
	 * Interrupts the timer immediately.
	 * see #start
	 */
	public void stop()
	{
		thread.interrupt();
		thread = null;
	}
	
	/**
	 * Simply contains the body of the running thread
	 */
	class RunObj implements Runnable
	{
		public void run()
		{
			try
			{
				Thread.sleep(sleepTime);
				callback.timeIsUp();
			}
			catch (InterruptedException exc) { }
		}
	}

	protected int sleepTime;
	protected TimerCallback callback;
	protected Thread thread;
	protected RunObj runObj;
}