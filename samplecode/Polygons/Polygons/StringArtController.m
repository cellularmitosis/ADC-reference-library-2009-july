//
//  StringArtController.m
//  StringArtPath
//
//  Created by John C. Randolph on Thu May 02 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import "StringArtController.h"
#import "StringArtView.h"

@implementation StringArtController

- (void) awakeFromNib
  {
  [self updateUI];
  }
  
- (void) updateUI
  {
  [sidesSlider setIntValue:[stringArtView sides]];
  [sidesField setIntValue:[stringArtView sides]];
  
  [rotationSlider setFloatValue:[stringArtView rotation]];
  [rotationField setFloatValue:[stringArtView rotation]];
  
  [radiusSlider setFloatValue:[stringArtView radius]];
  [radiusField setFloatValue:[stringArtView radius]];

  [fgColorWell setColor:[stringArtView foregroundColor]];
  [bgColorWell setColor:[stringArtView backgroundColor]];
  }
  
@end
