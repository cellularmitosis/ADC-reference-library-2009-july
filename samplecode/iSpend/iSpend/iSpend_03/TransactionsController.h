/*
 
 File: TransactionsController.h
 
 Version: 1.0
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 

#import <Cocoa/Cocoa.h>

@interface TransactionsController : NSArrayController
{
    id _document;
    id _transactionTable;
    NSString *_observedKeyPath;
    BOOL _pendingArrangement;
}
@end

@interface TransactionsController(Sorting)
- (void)scheduleRearrangeObjects;
- (void)updateObservationForOldTransactions:(NSArray *)oldTransactions newTransactions:(NSArray *)newTransactions;
- (void)removeSortObserversForTransactions:(NSArray *)transactions sortDescriptors:(NSArray *)sortDescriptors;
- (void)addSortObserversForTransactions:(NSArray *)transactions sortDescriptors:(NSArray *)sortDescriptors;
@end
