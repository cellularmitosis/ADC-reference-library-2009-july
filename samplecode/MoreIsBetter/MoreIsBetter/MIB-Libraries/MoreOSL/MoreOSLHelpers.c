/*
	File:		MoreOSLHelpers.c

	Contains:	Property getters and setters for standard system objects.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreOSLHelpers.c,v $
Revision 1.6  2002/11/08 23:48:09         
Include our prototype early to flush out any missing dependencies. Adopt MoreAEDataModel. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.5  2001/11/07 15:54:13         
Tidy up headers, add CVS logs, update copyright.


         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>     20/3/00    Quinn   Tested and fleshed out the various supported properties.
         <2>      9/3/00    Quinn   Changes to support strings beyond Str255.
         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreOSLHelpers.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <AEDataModel.h>
	#include <AERegistry.h>
	#include <MacWindows.h>
	#include <Processes.h>
	#include <PLStringFuncs.h>
	#include <Resources.h>
#endif

// MIB Prototypes

#include "MoreMemory.h"
#include "MoreResources.h"
#include "MoreProcesses.h"
#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"
#include "MoreOSL.h"

/////////////////////////////////////////////////////////////////
	
extern pascal OSStatus MOSLCApplicationGetPropHelper(DescType propName, AEDesc *result)
	// See comment in header.
{
	OSStatus 			err;
	Str255 				tmpStr;
	Boolean 			tmpBool;
	VersRecHndl 		versH;
	OSType				tmpType;

	MoreAENullDesc(result);
	
	switch (propName) {
		case keyAEBestType:
			tmpType = cApplication;
			err = AECreateDesc(typeType, &tmpType, sizeof(tmpType), result);
			break;
		case pName:
			err = MoreProcGetProcessName(NULL, tmpStr);
			if (err == noErr) {
				err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], result);
			}
			break;
		case pIsFrontProcess:
			tmpBool = MoreProcIsProcessAtFront(NULL);
			err = AECreateDesc(typeBoolean, &tmpBool, sizeof(tmpBool), result);
			break;
		case pVersion:
			versH = (VersRecHndl) Get1Resource('vers', 1);
			err = MoreResError(versH);
			if (err == noErr) {
				PLstrcpy(tmpStr, (**versH).shortVersion);
				err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], result);
			}
			break;
		default:
			err = errAENoSuchObject;
			break;
	}
	
	return err;
}

static OSStatus IsWindowZoomed(WindowRef window, Point idealSize, Boolean *result)
	// Determines whether the window is currently zoomed using the
	// Mac OS 8.5 Window Manager routine IsWindowInStandardState.
{
	OSStatus 		 err;
	WindowAttributes winAttr;
	Rect			 junkRect;
	
	assert(window != NULL);
	assert(result != NULL);
	
	err = GetWindowAttributes(window, &winAttr);
	if (err == noErr) {
		if ((winAttr & kWindowFullZoomAttribute) != 0) {
			*result = IsWindowInStandardState(window, &idealSize, &junkRect);
		} else {
			*result = false;
		}
	}
	return err;
}

extern pascal OSStatus MOSLCWindowGetPropHelper(WindowRef window, DescType propName, Point idealSize, AEDesc *result)
	// See comment in header.
{
	OSStatus 		 err;
	Str255 			 tmpStr;
	WindowRef 		 thisWindow;
	Boolean 		 found;
	SInt32 			 index;
	Rect 			 bounds;
	WindowAttributes winAttr;
	WindowAttributes attrMask;
	WindowClass 	 winClass;
	Boolean 		 tmpBool;
	Boolean 		 tmpBoolValid;
	RgnHandle		 tmpRgn;

	MoreAENullDesc(result);

	tmpBoolValid = false;
	err = noErr;
	switch (propName) {
		case pName:
			GetWTitle(window, tmpStr);
			err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], result);
			break;
		case pIndex:
			thisWindow = FrontWindow();
			index = 1;
			found = false;
			do {
				if (thisWindow != NULL) {
					found = (thisWindow == window);
					if (!found) {
						thisWindow = GetNextWindow(thisWindow);
						index += 1;
					}
				}
			} while (!found && (thisWindow != NULL));
			if (found) {
				err = AECreateDesc(typeLongInteger, &index, sizeof(index), result);
			} else {
				err = paramErr;
			}
			break;
		case kAESetPosition:
			err = GetWindowBounds(window, kWindowStructureRgn, &bounds);
			if (err == noErr) {
				// In Pascal, Rect is a tagless case variant record that lets
				// you access the data as either four coordinates or two Points.
				// In C, we have no such luxury (defective language), so we
				// just sleaze it.
				err = AECreateDesc(typeQDPoint, &bounds, sizeof(Point), result);
			}
			break;
		case keyAEBounds:
			err = GetWindowBounds(window, kWindowStructureRgn, &bounds);
			if (err == noErr) {
				err = AECreateDesc(typeQDRectangle, &bounds, sizeof(bounds), result);
			}
			break;
		case pIsResizable:
		case pIsZoomable:
		case pHasCloseBox:
			err = GetWindowAttributes(window, &winAttr);
			if (err == noErr) {
				switch (propName) {
					case pIsResizable:
						attrMask = kWindowResizableAttribute;
						break;
					case pIsZoomable:
						attrMask = kWindowFullZoomAttribute;
						break;
					case pHasCloseBox:
						attrMask = kWindowCloseBoxAttribute;
						break;
					default:
						assert(false);
						break;
				}
				tmpBool = ((winAttr & attrMask) != 0);
				tmpBoolValid = true;
			}
			break;
		case pIsFloating:
			err = GetWindowClass(window, &winClass);
			if (err == noErr) {
				tmpBool = (winClass == kFloatingWindowClass);
				tmpBoolValid = true;
			}
			break;
		case pIsModal:
			err = GetWindowClass(window, &winClass);
			if (err == noErr) {
				tmpBool = ((winClass == kModalWindowClass)) || ((winClass == kMovableModalWindowClass));
				tmpBoolValid = true;
			}
			break;
		case pHasTitleBar:
			// We determine whether a window has a title bar by getting its
			// region and checking for the empty region.  Note that GetWindowRegion
			// may well return an error for a window with no title bar, so we
			// just treat that as false.
			tmpRgn = NewRgn();
			err = MoreMemError(tmpRgn);
			if (err == noErr) {
				tmpBool = (GetWindowRegion(window, kWindowTitleBarRgn, tmpRgn) == noErr)
							&& !EmptyRgn(tmpRgn);
				tmpBoolValid = true;
				DisposeRgn(tmpRgn);
			}
			break;
		case pIsZoomed:
			err = IsWindowZoomed(window, idealSize, &tmpBool);
			if (err == noErr) {
				tmpBoolValid = true;
			}
			break;
		case pVisible:
			tmpBool = IsWindowVisible(window);
			tmpBoolValid = true;
			break;
		case pIsFrontProcess:
			tmpBool = (FrontWindow() == window);
			tmpBoolValid = true;
			break;
		default:
			err = errAENoSuchObject;
			break;
	}

	// As numerous branches of the switch statement return a Boolean, we
	// factor out the AECreateDesc and set tmpBoolValid to indicate that
	// we should create the Boolean descriptor before we leave.
	
	if ((err == noErr) & tmpBoolValid) {
		err = AECreateDesc(typeBoolean, &tmpBool, sizeof(tmpBool), result);
	}
	
	return err;
}

extern pascal OSStatus MOSLCWindowSetPropHelper(WindowRef window, DescType propName, Point idealSize, const AEDesc *data)
	// See comment in header.
{
	OSStatus 	   err;
	OSStatus 	   junk;
	Str255 		   tmpStr;
	Boolean 	   tmpBool;
	Boolean 	   tmpBool2;
	UInt32 		   winAttr;
	WindowPartCode zoomPart;
	Rect		   bounds;
	Point		   pt;

	err = noErr;
	switch (propName) {
		case pName:
			err = MOSLCoerceObjDescToPtr(data, typePString, tmpStr, sizeof(Str255));
			if (err == noErr) {
				SetWTitle(window, tmpStr);
			}
			break;
		case kAESetPosition:
			err = MOSLCoerceObjDescToPtr(data, typeQDPoint, &pt, sizeof(pt));
			if (err == noErr) {			
				err = GetWindowBounds(window, kWindowStructureRgn, &bounds);
			}
			if (err == noErr) {
				OffsetRect(&bounds, -bounds.left, -bounds.top);
				OffsetRect(&bounds, pt.h, pt.v);
				err = SetWindowBounds(window, kWindowStructureRgn, &bounds);
			}
			break;
		case keyAEBounds:
			err = GetWindowBounds(window, kWindowStructureRgn, &bounds);
			if (err == noErr) {
				pt.v = bounds.bottom - bounds.top;
				pt.h = bounds.right  - bounds.left;
				err = MOSLCoerceObjDescToPtr(data, typeQDRectangle, &bounds, sizeof(bounds));
			}
			if (err == noErr) {
				err = GetWindowAttributes(window, &winAttr);
			}
			// You can only change the size of the window if the window
			// is resizable.
			if (err == noErr) {
				if ( ((winAttr & kWindowResizableAttribute) == 0)
						&& (   ((bounds.bottom - bounds.top)  != pt.v)
							|| ((bounds.right  - bounds.left) != pt.h) ) ) {
					err = paramErr;
				}
			}
			if (err == noErr) {
				err = SetWindowBounds(window, kWindowStructureRgn, &bounds);
			}
			break;
		case pIsZoomed:
			err = MOSLCoerceObjDescToPtr(data, typeBoolean, &tmpBool, sizeof(tmpBool));
			if (err == noErr) {
				err = IsWindowZoomed(window, idealSize, &tmpBool2);
			}
			if ((err == noErr) && (tmpBool != tmpBool2)) {
				err = GetWindowAttributes(window, &winAttr);
				if (err == noErr) {
					if ((winAttr & kWindowFullZoomAttribute) != 0) {
						if (tmpBool) {
							zoomPart = inZoomOut;
						} else {
							zoomPart = inZoomIn;
						}
						junk = ZoomWindowIdeal(window, zoomPart, &idealSize);
						assert(junk == noErr);
					} else {
						if (tmpBool) {
							err = errAENotModifiable;
						}
					}
				}
			}
			break;
		case pVisible:
			err = MOSLCoerceObjDescToPtr(data, typeBoolean, &tmpBool, sizeof(tmpBool));
			if (err == noErr) {
				if (tmpBool) {
					ShowWindow(window);
				} else {
					HideWindow(window);
				}
			}
			break;
		case pIsFrontProcess:
			err = MOSLCoerceObjDescToPtr(data, typeBoolean, &tmpBool, sizeof(tmpBool));
			if (err == noErr & tmpBool) {
				SelectWindow(window);
			}
			break;
		default:
			err = errAENoSuchObject;
			break;
	}
	
	return err;
}
