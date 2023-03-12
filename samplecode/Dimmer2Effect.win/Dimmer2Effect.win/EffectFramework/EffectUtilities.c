/*
	File:		EffectUtilities.c
	
	Description: Tween utilities to help you write effect components

	Author:		Tom Dowdy, QuickTime Engineering

	Copyright: 	© Copyright 1997-2002 Apple Computer, Inc. All rights reserved.
	
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
		<1>	 	05/10/02	era		updated for X
*/

#if (__APPLE_CC__ || __MACH__)
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#elif TARGET_API_MAC_CARBON
	#include <Carbon.h>
	#include <QuickTime.h>
#else
    #include <ConditionalMacros.h>
	#include <ImageCodec.h>
	#include <NumberFormatting.h>
#endif

#include "EffectUtilities.h"
#include "BltMacros.h"

// ----------------------------------------------------------------------------------------
static int LocalMemCmp(Ptr a, Ptr b, Size length)
{
	while (length--) {
		if (*a++ != *b++)
			return(1);
	}
		
	return(0);

} // LocalMemCmp

// ----------------------------------------------------------------------------------------
OSErr CreateTweenRecord(
			TweenGlobals *glob, 			// input: our globals
			TweenContainerRecord *pTween, 	// input/output: tween record to initialize
			OSType parameterType,			// input: where to find parameter (type)
			long parameterID,				// input: where to find parameter (ID)
			Size parameterSize, 			// input: size of parameters
			OSType dataType,				// input: type of tween, 0 for none
			void *startValue,				// input: pointer or start value of default (for long size, just the data)
			void *endValue,					// input: pointer or end value of default (for long size, just the data)
			TimeValue duration)				// input: duration of the tween
{
	OSErr anErr = noErr;

	// all done?
	if (pTween->readyToGo)
		return(noErr);
		
	/* If we don't have the data handle, make one */
	if (pTween->tweenData == nil) {
		pTween->tweenData = NewHandleClear(parameterSize);
		anErr = MemError();
		if (anErr == noErr) {
			if (parameterSize == sizeof(unsigned long))
				((unsigned long*)(*(pTween->tweenData)))[0] = (unsigned long) EndianU32_NtoB(startValue);
			else
				BlockMoveData((Ptr) startValue, *(pTween->tweenData), parameterSize);
		}
	}
		
	/* If we don't have the tweeners set up yet, do so here */
	if ((anErr == noErr) && (pTween->tween == nil)) {
		QTAtom	atom;
		
		// find the parameter atom, if it exists
		atom = QTFindChildByID(glob->parameters, kParentAtomIsContainer, parameterType, parameterID, nil);
		if ((atom) && (QTCountChildrenOfType(glob->parameters, atom, 0) > 0)) {
			anErr = QTNewTween(&pTween->tween, glob->parameters, atom, duration);
			glob->atLeastOneTweener = true;
		} else {
			// okay, we either have a constant value, or need to default
			if (atom) {
				anErr = QTCopyAtomDataToHandle(glob->parameters, atom, pTween->tweenData);
				#if TARGET_RT_LITTLE_ENDIAN
				switch (dataType) {
					case kParameterTypeDataLong:
					case kParameterTypeDataFixed:
						**(long**)(pTween->tweenData) = EndianS32_BtoN(**(long**)(pTween->tweenData));
						break;
					case kParameterTypeDataRGBValue:
						MyEndianRGBColor(**(RGBColor**)pTween->tweenData);
						break;
					case kParameterTypeDataDouble:
						MyEndianQTFloatDouble(**pTween->tweenData);
						break;
				}
				#endif
			} else {
				if (dataType != 0) {
					QTAtom	tweenAtom;
					Boolean	valuesAreSame = false;
					
					if (glob->defaultParameters == nil)
						anErr = QTNewAtomContainer(&glob->defaultParameters);
					
					anErr = QTInsertChild(glob->defaultParameters, kParentAtomIsContainer, kTweenEntry, 0, 0,
													0, nil, &tweenAtom);

					if (anErr == noErr) {
						dataType = EndianU32_NtoB(dataType);
						QTInsertChild(glob->defaultParameters, tweenAtom, kTweenType, 0, 0,
													sizeof(dataType), &dataType, nil);
					}
					
					if (anErr == noErr) {
						SetHandleSize(pTween->tweenData, parameterSize * 2);
						anErr = MemError();
					}

					if (anErr == noErr) {
						HLock(pTween->tweenData);
						if (parameterSize == sizeof(unsigned long)) {
							((unsigned long*)(*(pTween->tweenData)))[0] = (unsigned long) EndianU32_NtoB(startValue);
							((unsigned long*)(*(pTween->tweenData)))[1] = (unsigned long) EndianU32_NtoB(endValue);
							
							if (startValue == endValue)
								valuesAreSame = true;
						} else {
							BlockMoveData((Ptr) startValue, *(pTween->tweenData), parameterSize);
							BlockMoveData((Ptr) endValue, (*(pTween->tweenData))+parameterSize, parameterSize);
							
							if (LocalMemCmp(startValue, endValue, parameterSize) == 0)
								valuesAreSame = true;
						}
						anErr = QTInsertChild(glob->defaultParameters, tweenAtom, kTweenData, 1, 0,
												parameterSize*2, *(pTween->tweenData), nil);
						HUnlock(pTween->tweenData);
						SetHandleSize(pTween->tweenData, parameterSize);
					}
					if (anErr == noErr) {
						if (valuesAreSame) {
							// flip back to native format, so effect can just read from it
							((unsigned long*)(*(pTween->tweenData)))[0] = (unsigned long) startValue;
						} else {
							anErr = QTNewTween(&pTween->tween, glob->defaultParameters, tweenAtom, duration);
							glob->atLeastOneTweener = true;
						}
					}
				// we need to create a default tween
				} else {
					// note that we store in native format, so that effect can just read from it
					if (parameterSize == sizeof(unsigned long))
						((unsigned long*)(*(pTween->tweenData)))[0] = (unsigned long) startValue;
					else
						BlockMoveData((Ptr) startValue, *(pTween->tweenData), parameterSize);
				} // we don't have a tween, just a default single value
					
			} // if we default the value
			
		} // if we don't have existing tween
			
	} // need to make the tween
			
	if (anErr == noErr)
		pTween->readyToGo = true;
		
	return(anErr);
	
} // CreateTweenRecord

// ----------------------------------------------------------------------------------------
void DisposeTweenRecord(TweenContainerRecord *pTween)
{
	if (pTween->tween) {
		QTDisposeTween(pTween->tween);
		pTween->tween = nil;
	}
	
	if (pTween->tweenData) {
		DisposeHandle(pTween->tweenData);
		pTween->tweenData = nil;
	}

	pTween->readyToGo = false;
	
} // DisposeTweenRecord

// ----------------------------------------------------------------------------------------
OSErr InitializeTweenGlobals(TweenGlobals *pTweenGlobals, CodecDecompressParams *p)
{
	return PtrToHand(p->data, (Handle*)&(pTweenGlobals->parameters), p->bufferSize);
	
} // InitializeTweenGlobals

// ----------------------------------------------------------------------------------------
void DisposeTweenGlobals(TweenGlobals *pTweenGlobals)
{
	if (pTweenGlobals->defaultParameters) {
		QTDisposeAtomContainer(pTweenGlobals->defaultParameters);
		pTweenGlobals->defaultParameters = nil;
	}
	
	pTweenGlobals->atLeastOneTweener = false;
	
	if (pTweenGlobals->parameters) {
		DisposeHandle((Handle)pTweenGlobals->parameters);
		pTweenGlobals->parameters = nil;
	}
	
} // DisposeTweenGlobals