/*
 
 File: MyDocument.h
 
 Version: 1.0
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 


#import <Cocoa/Cocoa.h>

#define kSpendDocumentType @"iSpend Data Format"
#define kSpendExtension @"spend"

@interface MyDocument : NSDocument
{
    float _openingBalance;
    NSMutableArray *_transactions;
    NSMutableArray *_categories;
    NSMutableArray *_accountTypes;
    id _transactionController;
}

- (NSArray *)transactions;
- (void)setTransactions:(NSMutableArray *)transactions;
- (void)setCategories:(NSMutableArray *)categories;
- (void)setAccountTypes:(NSMutableArray *)accountTypes;

- (void)addObserversForKeyPathsInTransactions:(NSArray *)newTransactions;
- (void)removeObserversForKeyPathsInTransactions:(NSArray *)oldTransactions;
@end

@interface MyDocument(Toolbar)
- (void)setupToolbarForWindow:(NSWindow *)theWindow;
@end

