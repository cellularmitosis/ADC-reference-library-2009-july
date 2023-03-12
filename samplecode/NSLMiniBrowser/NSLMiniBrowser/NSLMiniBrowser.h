/*
	File:		NSLMiniBrowser.h
	
	Description:	This sample code demonstrates the basic usage of the NSL API for finding network
                        services using SLP, NBP, and Directory Services.
        
	Author:		MK

	Copyright:	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
                5/21/01		1.0d1


*/

#include <Carbon.h>
#include <CFString.h>

#define kAppName			"NSL MiniBrowser"
#define kAppVersion			"1.0d1"
#define	kMaxNeighborhoods		32
#define kMaxServicesPerNeighborhood	1024
#define kMaxStringLength		128
#define	kDataBrowserRowHeight		18
#define kLookupBufferSize		1024
#define kMaxTypeNameLength		64
#define kNameKey			"NAME"
#define kAppleTalkKey			":/at/"


enum
{
    kServicesCountText = 1,
    kServicesTypeText = 2,
    kServicesTypePopup = 3,
    kUserPaneControl = 4,
    kDeregisterButton = 5,
    kDeregisterText = 6,
    kRegisterButton = 7,
    kRegisterText = 8,
    kRegisterInfoText = 9,
    kAddressStaticText = 10,
    kAddressEditText = 11,
    kNeighborhoodStaticText = 12,
    kNeighborhoodEditText = 13,
    kCancelRegisterButton = 14,
    kRegisterAddressButton = 15,
    kAboutWindowIcon = 16,
    kAboutAppNameText = 17,
    kAboutVersionText = 18,
};



enum
{
    kNSLSample = 'nsls',
    kNameColumn = 'SERV',
    kShowRegisterEvent = 'REGI',
    kDeregisterEvent = 'DERG',
    kEventParamItemID = 'ITEM',
    kEventParamName = 'NAME',
    kAddItemToBrowser = 9999,
    kServiceLookup = 1,
    kNeighborhoodLookup = 2,
    kDefaultNeighborhoods = 0xffffffff
};



typedef struct TimerInfo
{
    UInt32 refCount;
    EventLoopTimerRef timerRef;
    
} TimerInfo;



typedef struct LookupInfo
{
    UInt16 sTime;
    Boolean sLookupActive;
    NSLRequestRef sRequestRef;
    NSLServicesList sServiceList;
    NSLTypedDataPtr sRequestData;
    NSLNeighborhood sNeighborhood;
    UInt8 * sBufPtr;
    
    UInt16 nTime;
    Boolean nLookupActive;
    NSLRequestRef nRequestRef;
    NSLServicesList nServiceList;
    NSLNeighborhood nNeighborhood;
    UInt8 * nBufPtr;
    
} LookupInfo;



typedef struct NeighborhoodInfo
{
    UInt32 neighborhoodCount;
    DataBrowserItemID neighborhoodSize[kMaxNeighborhoods];
    Boolean isNeighborhoodOpen[kMaxNeighborhoods];
    Boolean isDefaultNeighborhood[kMaxNeighborhoods];
    Boolean isNeighborhoodVisible[kMaxNeighborhoods];
    DataBrowserItemID parentID[kMaxNeighborhoods];
    
} NeighborhoodInfo;