/*
 *  GLString.h

 *  Created November 3 2004.
 *  Copyright (c) 2004 Apple Computer Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple‚Äôs
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

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLContext.h>

@interface GLString : NSObject 
{
	CGLContextObj cgl_ctx; // current context at time of texture creation
	GLuint texID;			// Texture ID
	NSSize texSize;			// Texture size
	
	NSMutableAttributedString *str;
	NSBitmapImageRep	*bmpRep;
	
	// Color variables
	NSColor *textColor, *boxColor, *borderColor;
	NSFont *theFont;
	BOOL staticFrame; // default in NO
	NSSize marginSize, frameSize;
	
	NSMutableDictionary *attribs;
}

// Accessors
-(GLuint)texID;
-(NSSize)texSize;
-(NSColor*)textColor;
-(NSColor*)boxColor;
-(NSColor*)borderColor;
-(BOOL)staticFrame;
-(NSSize)frameSize;
-(NSSize)marginSize;

// 'Set' functions
-(void)setMargins:(NSSize)size;
-(void)useStaticFrame:(NSSize)size;
-(void)useDynamicFrame;
-(void)setTextColor:(NSColor*)color;
-(void)setBoxColor:(NSColor*)color;
-(void)setBorderColor:(NSColor*)color;

// Initialization and string adjustment
-(id)initWithString:(NSMutableString*)newString;
-(id)initWithAttributedString:(NSAttributedString*)newString;
-(id)initWithString:(NSMutableString*)newString textColor:(NSColor*)txColor boxColor:(NSColor*)bxColor borderColor:(NSColor*)brColor;
-(void)setString:(NSMutableString*)newString;
-(void)initTextAttributes;

// Texture generation and rendering
-(void)genTexture;
-(void)drawWithBounds:(NSRect)bounds;
-(void)drawAtPoint:(NSPoint)point;

// Clean up
-(void)dealloc;
-(void)deleteTexture;

@end
