/*

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

*/

#import <Cocoa/Cocoa.h>
#import "GBuffer.h"

@implementation GBuffer

//private

- (BOOL) createCGContext
{
	if (cg_context)
		return YES;
	else
	{
		CGColorSpaceRef colorspace_ref = CGColorSpaceCreateDeviceRGB();
		
		if (!colorspace_ref)
			return NO;
		else
		{
			size_t buffer_rowbytes =  (size_t) buffer_size.width * (buffer_bitdepth >> 3);
		
			CGImageAlphaInfo alpha_info = kCGImageAlphaPremultipliedFirst;
		
			cg_context = CGBitmapContextCreate(buffer_ptr, (UInt32) buffer_size.width, (UInt32) buffer_size.height, 8, buffer_rowbytes, colorspace_ref, alpha_info);
		
			if (cg_context)
			{
				CGContextSetFillColorSpace(cg_context, colorspace_ref);
				CGContextSetStrokeColorSpace(cg_context, colorspace_ref);
			}
			
			CGColorSpaceRelease(colorspace_ref);
			colorspace_ref = NULL;
		}
	}
		
	return !!cg_context;
}

+ (id) create: (CGSize) size bitDepth: (UInt8) bitdepth
{
	GBuffer* buffer = [[[GBuffer alloc] init] autorelease];
	
	if (![buffer sizeTo: size bitDepth: bitdepth])
		return nil;
		
	if (![buffer createCGContext])
		return nil;
		
	return [buffer retain];
}

- (void) clear: (float) alpha
{
	CGContextSetRGBFillColor(cg_context, 0.0, 0.0, 0.0, alpha);
	CGContextFillRect(cg_context, CGRectMake(0,0, buffer_size.width, buffer_size.height));
}

- (BOOL) sizeTo: (CGSize) size bitDepth: (UInt8) bitdepth
{
	UInt32 buffer_rowbytes = (UInt32) size.width * (bitdepth>>3);
	UInt32 pixel_format = bitdepth == 16 ? k16BE555PixelFormat : k32ARGBPixelFormat;

	buffer_memory_size = (size_t) size.height * buffer_rowbytes;

	buffer_ptr = (Ptr) malloc(buffer_memory_size);
	
	if (!buffer_ptr)
	{
		NSLog(@"buffer malloc failed");
		return NO;
	}
	else
	{
		GWorldFlags flags = useTempMem+keepLocal;

		Rect bounds_rect = { 0,0, size.height,size.width };
	
		OSErr err = NewGWorldFromPtr(&gworld_ptr, pixel_format, &bounds_rect, NULL, NULL, flags, buffer_ptr, buffer_rowbytes);
	
		if (err != noErr || !gworld_ptr)
		{
			NSLog(@"NewGWorld failed %d, %1.f x %1.f", buffer_bitdepth, buffer_size.width, buffer_size.height);

			return NO;
		}
	
		pixmap_handle = GetGWorldPixMap(gworld_ptr);

		if (!pixmap_handle)
		{
			NSLog(@"GetGWorldPixMap");
	
			return NO;
		}
	
		LockPixels(pixmap_handle);
	}
	
	buffer_size = size;
	buffer_bitdepth = bitdepth;

	return YES;
}

- (void) blitRect:(CGRect) src_rect dstBuffer:(GBuffer*) dst_buffer dst:(CGPoint) dst
{
	Rect srcRect, dstRect;

	SetRect(&srcRect, 0,0, src_rect.size.width, src_rect.size.height);
	OffsetRect(&srcRect, src_rect.origin.x, src_rect.origin.y);
	SetRect(&dstRect, 0,0, src_rect.size.width, src_rect.size.height);
	OffsetRect(&dstRect, dst.x, dst.y);
	
	{
		const BitMap* src_bitmap = GetPortBitMapForCopyBits(gworld_ptr);
		const BitMap* dst_bitmap = GetPortBitMapForCopyBits(dst_buffer->gworld_ptr);
	
		CopyBits(src_bitmap, dst_bitmap, &srcRect, &dstRect, srcCopy, NULL);
	}	
} 	

- (void) renderText: (const char*) text font: (const char*) font size: (int) size
{
	CGContextSetRGBFillColor(cg_context, 0.0, 0.0, 0.0, 1.0);
	CGContextFillRect(cg_context, CGRectMake(0,0, buffer_size.width, buffer_size.height));

	CGContextSetRGBFillColor(cg_context, 1.0, 1.0, 1.0, 1.0);
	CGContextSelectFont(cg_context, font, size, kCGEncodingMacRoman);
	CGContextSetTextDrawingMode(cg_context, kCGTextFill);
	CGContextShowTextAtPoint(cg_context, 0, buffer_size.height*.25, text, strlen(text));
	CGContextFlush(cg_context);
}

- (void) renderGlyph: (CGGlyph) glyph typeface: (const char*) typeface size: (int) size offset: (CGPoint) offset  
{
	CGContextSetRGBFillColor(cg_context, 1.0, 1.0, 1.0, 1.0);
	CGContextSelectFont(cg_context, typeface, size, kCGEncodingMacRoman);
	CGContextSetTextDrawingMode(cg_context, kCGTextFill);
	CGContextShowGlyphsAtPoint(cg_context, offset.x, offset.y, &glyph, 1);
	CGContextFlush(cg_context);
}

@end
