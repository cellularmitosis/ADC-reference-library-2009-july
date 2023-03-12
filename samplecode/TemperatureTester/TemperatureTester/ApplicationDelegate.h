/*
    ApplicationDelegate.h
    Copyright (c) 2003, Apple Computer, Inc., all rights reserved.
        
    A simple application delegate.   Initializes the application by
    registering the custom value transformers with NSValueTransformer
    and ensures that reasonable default data is presented to the user
    upon first launch.
*/

@interface ApplicationDelegate : NSObject
{
    IBOutlet NSFormCell *rankineTemperatureField;
    IBOutlet NSUserDefaultsController *sharedUserDefaultsController;
}
@end
