//
//  StringArtController.h
//  StringArtPath
//
//  Created by John C. Randolph on Thu May 02 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

 
@interface StringArtController : NSObject 
  {
  IBOutlet id stringArtView;
  IBOutlet id sidesSlider;
  IBOutlet id sidesField;
  IBOutlet id rotationSlider;
  IBOutlet id rotationField;
  IBOutlet id radiusSlider;
  IBOutlet id radiusField;
  IBOutlet id fgColorWell;
  IBOutlet id bgColorWell;
  }

- (void) updateUI;


@end
