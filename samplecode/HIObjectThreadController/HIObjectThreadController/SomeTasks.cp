/*
	File:		SomeTasks.cp

	Contains:	Some Multi Processing Tasks and utility functions.

	Version:	1.0

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

	Copyright © 2003 Apple Computer, Inc., All Rights Reserved
*/

#include "SomeTasks.h"
#include "HIObjectThreadController.h"

/*****************************************************
*
* SendEventToUI(kind, params, iterator, result) 
*
* Purpose:  Posts an event to the main thread which is handling the User Interface
*
* Inputs:   kind      - the kind of event (in this sample code: kEventUpdateThreadUI or kEventTerminateThread)
*           params    - a pointer to the task params
*           iterator  - the current value of the iteration index
*           result    - the current value of the variable being calculated, in this sample code: pi
*
*/
void SendEventToUI(UInt32 kind, GeneralTaskWorkParamsPtr params, double iterator, double result)
	{
	params->iterator = iterator;
	params->result = result;

	EventRef theEvent;
	CreateEvent(NULL, kEventClassHIObjectThreadController, kind, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
	SetEventParameter(theEvent, kEventParamPostTarget, typeEventTargetRef, sizeof(params->threadControllerTarget), &params->threadControllerTarget);
	PostEventToQueue(GetMainEventQueue(), theEvent, kEventPriorityStandard);

	ReleaseEvent(theEvent);
	}

// pi = 4 ( 1 - 1/3 + 1/5 - 1/7 + 1/9 - ...)   
OSStatus CalculatingPiUsingLeibnizSimpleTask(void *p)
	{
	LeibnizTaskWorkParamsPtr params = (LeibnizTaskWorkParamsPtr)p;
	Boolean sign = params->sign;
	double denominator = params->general.iterator;
	double result = params->general.result;
	
	double lastDenominator = denominator + kSTEndIteration;
	double lastUIChange = denominator + kSTUIIteration;

	Duration updateInterval = 200 * durationMillisecond;
	AbsoluteTime nextTime = AddDurationToAbsolute(updateInterval, UpTime());
	
	for (; denominator < lastDenominator; )
		{
		if (sign)
			result += 4.0/denominator;
		else
			result -= 4.0/denominator;
		sign = ! sign;
		denominator += 2.0;
		
		// we don't want to submerge the main thread with too many events
		// so we check first if it's a good time to send one.
		// we also don't want to call too many APIs checking so we first
		// test if we did enough iterations.
		if (denominator > lastUIChange)
			{
			lastUIChange = denominator + kSTUIIteration;

			AbsoluteTime newTime = UpTime();
			if (S64Compare(UnsignedWideToUInt64(nextTime), UnsignedWideToUInt64(newTime)) < 0)
				{
				SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, denominator, result);
				nextTime = AddDurationToAbsolute(updateInterval, newTime);
				}
			}
		}
	
	// iteration is finished, we send the appropriate event to the main thread.
	SendEventToUI(kEventTerminateThread, (GeneralTaskWorkParamsPtr)params, denominator, result);
	
	return noErr;
	}

void * SetUpLeibniz(void)
	{
	LeibnizTaskWorkParamsPtr params = (LeibnizTaskWorkParamsPtr)malloc(sizeof(LeibnizTaskWorkParams));
	params->sign = true;
	params->general.iterator = 1.0;
	params->general.result = 0.0;
	return params;
	}

void TermLeibniz(void * p)
	{
	free(p);
	}

// pi = 2 ( (2/1) * (2/3) * (4/3) * (4/5) * (6/5) * (6/7) * (...) )
OSStatus CalculatingPiUsingWallisSimpleTask(void *p)
	{
	WallisTaskWorkParamsPtr params = (WallisTaskWorkParamsPtr)p;
	double numerator = params->general.iterator;
	double denominator = params->denominator;
	double result = params->general.result;
	
	double lastNumerator = numerator + kSTEndIteration;
	double lastUIChange = numerator + kSTUIIteration;

	AbsoluteTime interval, newTime, lastTime = UpTime();
	
	for (; numerator < lastNumerator; )
		{
		result *= numerator;
		result /= denominator;
		denominator += 2.0;
		result *= numerator;
		result /= denominator;
		numerator += 2.0;
		
		// we don't want to submerge the main thread with too many events
		// so we check first if it's a good time to send one.
		// we also don't want to call too many APIs checking so we first
		// test if we did enough iterations.		
		if (numerator > lastUIChange)
			{
			lastUIChange = numerator + kSTUIIteration;

			newTime = UpTime();
			interval = SubAbsoluteFromAbsolute(newTime, lastTime);
			Duration duration = AbsoluteToDuration(interval) / -1000;
			if (duration > 200)
				{
				lastTime = newTime;
				SendEventToUI(kEventUpdateThreadUI, (GeneralTaskWorkParamsPtr)params, numerator, result);
				}
			}
		}
	
	// iteration is finished, we send the appropriate event to the main thread.
	SendEventToUI(kEventTerminateThread, (GeneralTaskWorkParamsPtr)params, numerator, result);
	
	return noErr;
	}

void * SetUpWallis(void)
	{
	WallisTaskWorkParamsPtr params = (WallisTaskWorkParamsPtr)malloc(sizeof(WallisTaskWorkParams));
	params->general.iterator = 2.0;
	params->denominator = 1.0;
	params->general.result = 2.0;
	return params;
	}

void TermWallis(void * p)
	{
	free(p);
	}
