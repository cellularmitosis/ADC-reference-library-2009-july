/*
 * 
    Copyright:	Copyright © 2003 Apple Computer, Inc., All Rights Reserved

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
 *
 */


#import "Controller.h"

@implementation Controller

- (IBAction)detailSlider:(id)sender
{
    [myOpenGLView setDetail:[sender intValue]];
    [trisInModel setIntValue:[myOpenGLView modelTriangleCount]];
}

- (IBAction)sizeSlider:(id)sender
{
    [myOpenGLView setSize:[sender intValue]];
}

- (IBAction)xSlider:(id)sender
{
    [myOpenGLView setX:[sender floatValue]];
}

- (IBAction)ySlider:(id)sender
{
    [myOpenGLView setY:[sender floatValue]];
}

- (IBAction)zSlider:(id)sender
{
    [myOpenGLView setZ:[sender floatValue]];
}

- (IBAction)textureBox:(id)sender
{
    int i;
    int values[kTextureUnitCount];
    for (i = 0; i< kTextureUnitCount; i++)
    {
        NSCell *cell = [sender cellAtRow:i column:0];
        values[i] = [cell intValue];
    }
    [myOpenGLView setTexturing:values];
}

- (IBAction)macrosBox:(id)sender
{
    [myOpenGLView setMacros:[sender intValue]];
}

- (IBAction)matrixBox:(id)sender
{
    [myOpenGLView setMatrix:[sender intValue]];
}

- (IBAction)normalBox:(id)sender
{
    [myOpenGLView setNormals:[sender intValue]];
}

- (IBAction)fogFactor:(id)sender
{
    [myOpenGLView setFogFactor:[sender intValue]];
}

- (IBAction)colorBox:(id)sender
{
    [myOpenGLView setColor:[sender intValue]];
}

- (IBAction)secColorBox:(id)sender
{
    [myOpenGLView setSecColor:[sender intValue]];
}

- (IBAction)autoRotateBox:(id)sender
{
    [myOpenGLView setRotate:[sender intValue]];
    
    if ([sender intValue])
    {
        [xSlider setEnabled:NO];
        [ySlider setEnabled:NO];
        [zSlider setEnabled:NO];
    }
    else
    {
        [xSlider setEnabled:YES];
        [ySlider setEnabled:YES];
        [zSlider setEnabled:YES];
        [myOpenGLView setX:[xSlider floatValue]];
        [myOpenGLView setY:[ySlider floatValue]];
        [myOpenGLView setZ:[zSlider floatValue]];
    }
}

- (IBAction)matrixClick:(id)sender
{
    NSCell *cell = [sender selectedCell];
    
    [myOpenGLView setDrawMethod:[cell tag]];
}


- (void) awakeFromNib
{
    timer = [NSTimer scheduledTimerWithTimeInterval: 0.05 target:self 
                selector:@selector(myUpdate) userInfo:0 repeats:YES];
}


- (void) myUpdate
{
    [tpsCounter setFloatValue:[myOpenGLView performance]];
    [trisInModel setIntValue:[myOpenGLView modelTriangleCount]];
    [myOpenGLView spinModel];
}

- (IBAction)showAboutBox:(id)sender
{
    [[AboutBox sharedInstance] showPanel:sender];
}


- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    int units = [myOpenGLView textureUnitCount];
    int i;
    for (i = units; i < kTextureUnitCount; i++)
    {
        NSCell *cell = [textureBoxes cellAtRow:i column:0];
        [cell setEnabled:NO];
    }
    
    if (GL_FALSE == gluCheckExtension("GL_APPLE_vertex_array_range", glGetString(GL_EXTENSIONS)))
    {
        for (i = 0; i < [drawingMatrix numberOfRows]; i++)
        {
            NSCell *cell = [drawingMatrix cellAtRow:i column:0];
            if ([cell tag]==kVertexArrayRange)
                [cell setEnabled:NO];
        }
    }
}

@end
