/*
 *  drawInfo.m
 *
 *  Created by ggs on Wed Mar 12 2003.
 *  Copyright (c) 2003 Apple Computer Inc. All rights reserved.

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
#import <Foundation/Foundation.h>
#import "glcheck.h"
#import "StringTexture.h"
#import "drawInfo.h"

NSMutableArray *capsTextures;

void initCapsTexture (GLCaps * displayCaps, CGDisplayCount numDisplays)
{
	short index;
	[capsTextures release];
	capsTextures = NULL;
	capsTextures = [NSMutableArray arrayWithCapacity: numDisplays];
	[capsTextures retain];
	
	// draw info
    NSMutableDictionary *bold12Attribs = [NSMutableDictionary dictionary];
    [bold12Attribs setObject: [NSFont fontWithName: @"Helvetica-Bold" size: 12.0f] forKey: NSFontAttributeName];
    [bold12Attribs setObject: [NSColor whiteColor] forKey: NSForegroundColorAttributeName];
 
    NSMutableDictionary *bold9Attribs = [NSMutableDictionary dictionary];
    [bold9Attribs setObject: [NSFont fontWithName: @"Helvetica-Bold" size: 9.0f] forKey: NSFontAttributeName];
    [bold9Attribs setObject: [NSColor whiteColor] forKey: NSForegroundColorAttributeName];
 
	NSMutableDictionary *normal9Attribs = [NSMutableDictionary dictionary];
    [normal9Attribs setObject: [NSFont fontWithName: @"Helvetica" size: 9.0f] forKey: NSFontAttributeName];
    [normal9Attribs setObject: [NSColor whiteColor] forKey: NSForegroundColorAttributeName];

	for (index = 0; index < numDisplays; index++) {
		NSMutableAttributedString * outString, * appendString;
		StringTexture *capsTexture;
		
		// draw caps string
		outString = [[[NSMutableAttributedString alloc] initWithString:@"OpenGL Capabilities:" attributes:bold12Attribs] autorelease];
		
		appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithFormat:@"\n  Max VRAM- %ld MB (%ld MB free)", displayCaps[index].deviceVRAM / 1024 / 1024, displayCaps[index].deviceTextureRAM / 1024 / 1024] attributes:normal9Attribs];
		[outString appendAttributedString:appendString];
		[appendString release];
	
		appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithFormat:@"\n  Max Texture Size- 1D/2D: %ld, 3D: %ld, Cube: %ld, Rect: %ld (%ld texture units)", displayCaps[index].maxTextureSize, displayCaps[index].max3DTextureSize, displayCaps[index].maxCubeMapTextureSize, displayCaps[index].maxRectTextureSize, displayCaps[index].textureUnits] attributes:normal9Attribs];
		[outString appendAttributedString:appendString];
		[appendString release];

		appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n Features:"] attributes:bold9Attribs];
		[outString appendAttributedString:appendString];
		[appendString release];

		if (displayCaps[index].fSpecularVector) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Specular Vector (GL_APPLE_specular_vector)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTransformHint) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Transform Hint (GL_APPLE_transform_hint)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPackedPixels) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Packed Pixels (GL_APPLE_packed_pixels or OpenGL 1.2+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fClientStorage) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Client Storage (GL_APPLE_client_storage)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fYCbCr) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  YCbCr Textures (GL_APPLE_ycbcr_422)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTextureRange) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Range (AGP Texturing) (GL_APPLE_texture_range)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fFence) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Fence (GL_APPLE_fence)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fVAR) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Vertex Array Range (GL_APPLE_vertex_array_range)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fVAO) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Vertex Array Object (GL_APPLE_vertex_array_object)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fElementArray) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Element Array (GL_APPLE_element_array)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fVPEvals) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Vertex Program Evaluators (GL_APPLE_vertex_program_evaluators)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fFloatPixels) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Floating Point Pixels (GL_APPLE_vertex_program_evaluators)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fFlushRenderer) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Flush Renderer (GL_APPLE_flush_render)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPixelBuffer) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Pixel Buffers (GL_APPLE_pixel_buffer)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fImaging) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Imaging Subset (GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTransposeMatrix) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Transpose Matrix (GL_ARB_transpose_matrix or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fMultitexture) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Multitexture (GL_ARB_multitexture or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexEnvAdd) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Env Add (GL_ARB_texture_env_add, GL_EXT_texture_env_add or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexEnvCombine) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Env Combine (GL_ARB_texture_env_combine or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexEnvDot3) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Env Dot3 (GL_ARB_texture_env_dot3 or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexEnvCrossbar) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Env Crossbar (GL_ARB_texture_env_crossbar or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexCubeMap) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Env Cube Map (GL_ARB_texture_cube_map or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexCompress) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Compression (GL_ARB_texture_compression or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fMultisample) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Multisample (Anti-aliasing) (GL_ARB_multisample or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexBorderClamp) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Border Clamp (GL_ARB_texture_border_clamp or OpenGL 1.3+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPointParam) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Point Parameters (GL_ARB_point_parameters or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPointParam) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Point Parameters (GL_ARB_point_parameters or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fVertexProg) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Vertex Program (GL_ARB_vertex_program)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fFragmentProg) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Fragment Program (GL_ARB_fragment_program)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexMirrorRepeat) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Mirrored Repeat (GL_ARB_texture_mirrored_repeat or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fDepthTex) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Depth Texture (GL_ARB_depth_texture or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fShadow) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Shadow Support (GL_ARB_shadow or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fShadowAmbient) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Shadow Ambient (GL_ARB_shadow_ambient)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fVertexBlend) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Vertex Blend (GL_ARB_vertex_blend)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fWindowPos) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Window Position (GL_ARB_window_pos or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTex3D) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  3D Texturing (GL_EXT_texture3D or OpenGL 1.2+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fClipVolHint) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Clip Volume Hint (GL_EXT_clip_volume_hint)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fRescaleNorm) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Rescale Normal (GL_EXT_rescale_normal or OpenGL 1.2+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBlendColor) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Blend Color (GL_EXT_blend_color or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBlendMinMax) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Blend Min/Max (GL_EXT_blend_minmax or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBlendSub) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Blend Subtract (GL_EXT_blend_subtract or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fCVA) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Compiled Vertex Array (GL_EXT_compiled_vertex_array)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexLODBias) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Level Of Detail Bias (GL_EXT_texture_lod_bias or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fABGR) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  ABGR Texture Support (GL_EXT_abgr)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBGRA) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  BGRA Texture Support (GL_EXT_bgra or OpenGL 1.2+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexFilterAniso) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Anisotropic Texture Filtering (GL_EXT_texture_filter_anisotropic)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPaletteTex) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Paletted Textures (GL_EXT_paletted_texture)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fShareTexPalette) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Shared Texture Palette (GL_EXT_shared_texture_palette)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fSecColor) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Secondary Color (GL_EXT_secondary_color or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexCompressS3TC) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Compression S3TC (GL_EXT_texture_compression_s3tc)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexRect) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Rectangle (GL_EXT_texture_rectangle)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fFogCoord) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Fog Coordinate (GL_EXT_fog_coord)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fDrawRangeElements) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Draw Range Elements (GL_EXT_draw_range_elements)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fStencilWrap) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Stencil Wrap (GL_EXT_stencil_wrap or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBlendFuncSep) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Separate Blend Function (GL_EXT_blend_func_separate or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fMultiDrawArrays) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Multi-Draw Arrays (GL_EXT_multi_draw_arrays or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fShadowFunc) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Shadow Function (GL_EXT_shadow_funcs)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fStencil2Side) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  2-Sided Stencil (GL_EXT_stencil_two_side)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fColorSubtable) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Color Subtable ( GL_EXT_color_subtable or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fConvolution) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Convolution ( GL_EXT_convolution or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fHistogram) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Histogram ( GL_EXT_histogram or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fColorTable) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Color Table ( GL_SGI_color_table or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fColorMatrix) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Color Matrix ( GL_SGI_color_matrix or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexEdgeClamp) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Edge Clamp (GL_SGIS_texture_edge_clamp or OpenGL 1.2+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexLOD) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Level Of Detail (GL_SGIS_texture_lod or OpenGL 1.2+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPointCull) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Point Culling (GL_ATI_point_cull_mode)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexMirrorOnce) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Mirror Once (GL_ATI_texture_mirror_once)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPNtriangles) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  PN Triangles (GL_ATI_pn_triangles or GL_ATIX_pn_triangles)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTextFragShader) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Text Fragment Shader (GL_ATI_text_fragment_shader)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBlendEqSep) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Separate Blend Equations (GL_ATI_blend_equation_separate)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBlendWeightMinMax) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Blend Weighted Min/Max (GL_ATI_blend_weighted_minmax)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fCombine3) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Env Combine 3 (GL_ATI_texture_env_combine3)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fSepStencil) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Separate Stencil (GL_ATI_separate_stencil)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fArrayRevComps4Byte) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Reverse 4 Byte Array Components (GL_ATI_array_rev_comps_in_4_bytes)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fPointSprite) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Point Sprites (GL_NV_point_sprite)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fRegCombiners) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Register Combiners (GL_NV_register_combiners)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fRegCombiners2) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Register Combiners 2 (GL_NV_register_combiners2)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexEnvCombine4) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Env Combine 4 (GL_NV_texture_env_combine4)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fBlendSquare) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Blend Square (GL_NV_blend_square or OpenGL 1.4+)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fFogDist) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Eye Radial Fog Distance (GL_NV_fog_distance)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fMultisampleFilterHint) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Multi-Sample Filter Hint (GL_NV_multisample_filter_hint)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexGenReflect) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  TexGen Reflection (GL_NV_texgen_reflection)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexShader) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Shader (GL_NV_texture_shader)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexShader2) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Shader 2 (GL_NV_texture_shader2)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fTexShader3) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Texture Shader 3 (GL_NV_texture_shader3)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fDepthClamp) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Depth Clamp (GL_NV_depth_clamp)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fLightMaxExp) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Light Max Exponent (GL_NV_light_max_exponent)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fConvBorderModes) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Convolution Border Modes (GL_HP_convolution_border_modes or GL_ARB_imaging)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}
		if (displayCaps[index].fRasterPosClip) {
			appendString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithString:@"\n  Raster Position Clipping (GL_IBM_rasterpos_clip)"] attributes:normal9Attribs];
			[outString appendAttributedString:appendString];
			[appendString release];
		}

		capsTexture = [[StringTexture alloc] initWithAttributedString:outString withTextColor:[NSColor colorWithDeviceRed:1.0f green:1.0f blue:1.0f alpha:1.0f] withBoxColor:[NSColor colorWithDeviceRed:0.4f green:0.4f blue:0.0f alpha:0.4f] withBorderColor:[NSColor colorWithDeviceRed:0.8f green:0.8f blue:0.0f alpha:0.8f]];
		[capsTextures addObject:capsTexture];
		[capsTexture release];
	}
}

// get NSString with caps for this renderer
void drawCaps (GLCaps * displayCaps, CGDisplayCount numDisplays, long renderer, GLfloat width) // view width for drawing location
{ // we are already in an orthographic per pixel projection
	short i;
	// match display in caps list
	for (i = 0; i < numDisplays; i++) {
		if (renderer == displayCaps[i].rendererID) {
			StringTexture *capsTexture = [capsTextures objectAtIndex:i];
			[capsTexture drawAtPoint:NSMakePoint (width - 10.0f - [capsTexture frameSize].width, 10.0f)];
			break;
		}
	}
}
	/*
		return outString;		
	*/
