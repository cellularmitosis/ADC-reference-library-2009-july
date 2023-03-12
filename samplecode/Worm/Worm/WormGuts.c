/*
 
 File: WormGuts.c
 Abstract: The guts of the worm code
 
 Version: <1.0>
 
 © Copyright 2005 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
          PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
          OF USE, DATA, OR PROFITS; OR BUSINESS 
          INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
                                 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */

#include "WormGuts.h"
#include <stdlib.h>

GameState worm_guts(GamePosition *wormPositions, unsigned *wormLength, GameDirection *wormDirection, GameHeading *wormHeading, GamePosition *targetPosition, unsigned width, unsigned height, int noBounce) {
    int retval = kGameStateContinue;
    unsigned i;
    if (wormPositions[0].x == targetPosition->x && wormPositions[0].y == targetPosition->y) {	/* Check target */
        unsigned collisions;
        if (targetPosition->x >= 0 && targetPosition->y >= 0) retval = kGameStateScore;		/* Account for initial state */
        do {	/* Compute new location for target */
            targetPosition->x = random() % width;
            targetPosition->y = random() % height;
            for (i = 0, collisions = 0; collisions == 0 && i < *wormLength; i++) {
                if (targetPosition->x == wormPositions[i].x && targetPosition->y == wormPositions[i].y) collisions++;
            }
        } while (collisions > 0);
        (*wormLength)++;
    }
    for (i = *wormLength; i > 0; i--) {	/* Advance worm */
        wormPositions[i] = wormPositions[i-1];
    }
    *wormDirection = (*wormDirection + *wormHeading) % 4;
    wormPositions[0].x += (*wormDirection == kGameDirectionEast) - (*wormDirection == kGameDirectionWest);
    wormPositions[0].y += (*wormDirection == kGameDirectionSouth) - (*wormDirection == kGameDirectionNorth);
    if (*wormHeading != kGameHeadingStraight && (wormPositions[0].x < 0 || wormPositions[0].x >= width || wormPositions[0].y < 0 || wormPositions[0].y >= height)) {
        wormPositions[0].x -= (*wormDirection == kGameDirectionEast) - (*wormDirection == kGameDirectionWest);
        wormPositions[0].y -= (*wormDirection == kGameDirectionSouth) - (*wormDirection == kGameDirectionNorth);
        *wormDirection = (*wormDirection - *wormHeading) % 4;
        wormPositions[0].x += (*wormDirection == kGameDirectionEast) - (*wormDirection == kGameDirectionWest);
        wormPositions[0].y += (*wormDirection == kGameDirectionSouth) - (*wormDirection == kGameDirectionNorth);
    }
    *wormHeading = kGameHeadingStraight;
    if (wormPositions[0].x < 0 || wormPositions[0].x >= width) {	/* Hit the wall; turn or end game */
        if (noBounce) {	/* Done */
            retval = kGameStateCrash;	
        } else {	/* Turn */
            wormPositions[0].x = (wormPositions[0].x < 0) ? 0 : width - 1;
            *wormDirection = (wormPositions[0].y < height / 2) ? kGameDirectionSouth : kGameDirectionNorth;
            wormPositions[0].y += (*wormDirection == kGameDirectionSouth) - (*wormDirection == kGameDirectionNorth);
        }
    } else if (wormPositions[0].y < 0 || wormPositions[0].y >= height) {		/* Hit the wall; turn or end game */
        if (noBounce) {	/* Done */
            retval = kGameStateCrash;		
        } else {	/* Turn */
            wormPositions[0].y = (wormPositions[0].y < 0) ? 0 : height - 1;
            *wormDirection = (wormPositions[0].x < width / 2) ? kGameDirectionEast : kGameDirectionWest;
            wormPositions[0].x += (*wormDirection == kGameDirectionEast) - (*wormDirection == kGameDirectionWest);
        }
    }
    for (i = 1; retval != kGameStateCrash && i < *wormLength; i++) {
        if (wormPositions[0].x == wormPositions[i].x && wormPositions[0].y == wormPositions[i].y) retval = kGameStateCrash;
    }
    return retval;
}

