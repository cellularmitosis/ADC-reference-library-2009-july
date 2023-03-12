

#import "AppController.h"
#import "JoystickView.h"


@implementation AppController

- (void)awakeFromNib
{
	anArray = [[NSMutableArray alloc] init];
	
	NSMutableDictionary *dict =
		[NSMutableDictionary dictionaryWithObject:[NSNumber numberWithInt:28]
										   forKey:@"angle"];
	[dict setObject:[NSNumber numberWithInt:7] forKey:@"offset"];
	
	[anArray addObject:dict];
	
	
	NSDictionary *options = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES]
										forKey:NSAllowsEditingMultipleValuesSelectionBindingOption];
	
	
	[arrayController bind:@"contentArray"
					 toObject:self
				  withKeyPath:@"anArray"
					  options:options];
	
	
	[joystick bind:@"angle"
		  toObject:arrayController
	   withKeyPath:@"selection.angle"
		   options:options];
	
	[joystick bind:@"offset"
		  toObject:arrayController
	   withKeyPath:@"selection.offset"
		   options:options];
}

- (void)dealloc
{
	[anArray release];	
	[super dealloc];
}





- (unsigned int)countOfAnArray 
{
    return [anArray count];
}

- (id)objectInAnArrayAtIndex:(unsigned int)index 
{
    return [anArray objectAtIndex:index];
}

- (void)insertObject:(id)anObject inAnArrayAtIndex:(unsigned int)index 
{
    [anArray insertObject:anObject atIndex:index];
}

- (void)removeObjectFromAnArrayAtIndex:(unsigned int)index 
{
    [anArray removeObjectAtIndex:index];
}

- (void)replaceObjectInAnArrayAtIndex:(unsigned int)index withObject:(id)anObject 
{
    [anArray replaceObjectAtIndex:index withObject:anObject];
}





@end
