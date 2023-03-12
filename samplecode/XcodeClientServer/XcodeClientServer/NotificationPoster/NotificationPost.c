/*
 File: NotificationPost.c
 
 Abstract: Implements the code to send out stock price change notifications
 using CFNotificationCenter.
 
 Copyright (c) 2000-2005, Apple Computer, Inc., all rights reserved.
*/
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import <CoreFoundation/CoreFoundation.h>

// This routine packages up the arguments and posts the notification
// The object is the stock symbol (so observers can choose what individual stocks
//   they want to track)
// The user dictionary contains the stock symbol and new price (and we can
//   put more info in there if needed)
// We also post with "postImmediately" set to FALSE, meaning observers which
//   have asked for coalesced or held notifications won't be bothered while
//   suspended

void SendNotification(CFStringRef stockSymbol, double price) {
    CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
    CFNumberRef priceNum = CFNumberCreate(NULL, kCFNumberDoubleType, &price);
    CFMutableDictionaryRef stockInfoDict = CFDictionaryCreateMutable(NULL, 2,
        &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(stockInfoDict, CFSTR("Stock Symbol"), stockSymbol);
    CFDictionaryAddValue(stockInfoDict, CFSTR("New Price"), priceNum);
    CFRelease(priceNum);

    CFNotificationCenterPostNotification(center, CFSTR("Stock Price Changed Notification"), stockSymbol, stockInfoDict, FALSE);
    
    CFRelease(stockInfoDict);
}

