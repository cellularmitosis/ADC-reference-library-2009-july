/*
 
 File: ExpandableViewController.h
 
 Version: 1.0
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 

#import <Cocoa/Cocoa.h>

@interface ExpandableViewController : NSObject
{
    IBOutlet NSView *_bankTransactionView;
    IBOutlet NSView *_stockTransactionView;
    IBOutlet NSView *_upperTableScrollView;
    IBOutlet NSView *_middleBoxView;
    NSView *_currentView;
    id _transactionController;
    BOOL _expanded;
}

@end
