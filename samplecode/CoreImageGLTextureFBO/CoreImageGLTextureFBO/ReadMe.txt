Core Image to OpenGL Texture. 

This sample shows how to render with Core Image into an
OpenGL accelerated offscreen area known as an FBO (framebuffer object). 
This technique provides the benefit of leveraging Core Image 
and use its results as OpenGL textures. 

Here we use a series of CIHueAdjust, CIGammaAdjust, and CIGloom filters 
to adjust the picture of a Rose, which seems to float on top of a reflective surface. 

The code is all contained in MyOpenGLView.m.

The runtime flow of the project is the following:

- (id)initWithFrame:(NSRect)frameRect

 A first attempt is made for an antialised pixel format, if that 
 should fail, a less demanding pixel format is selected and the 
 OpenGL context is created. Then we synchronized the buffer swaps
 to the vertical refresh of the display to avoid tearing. 

- (void) prepareOpenGL

 Here the code starts dispatching to setup the CIImage, CIFilter,
 CIContext, the FBO, clear color, blending, and texturing. 
 
- (void)setupImageWithCIFilter

 Creates the CIImage from Rose.jpg and CIFilter with CIExposureAdjust.
 
- (BOOL)createCIContext

 Creates the CIContext based on the OpenGL context. 
 
- (void)setFBO

 Make sure the framebuffer extenstion is supported on the current renderer.
 The Framebuffer Object is created, and a texture with the same dimensions
 as the CIImage is attached as the color destination of the FBO. 
 
- (void)drawRect:(NSRect)theRect

 drawRect gets called to render. 
 
 The CIFilter gets the exposureValue assigned, which is controlled
 by the user via the NSSlider.
 
 It calls, 
 
- (void)renderCoreImageToFBO method:
makes the framebuffer object (FBO) the destination for rendering,
sets up an orthographic screen aligned 2D mode for OpenGL,
it asks the CIContext to draw the image and
sets the destination for rendering the default framebuffer. 
  
At this point the result of the Core Image process is stored
in the OpenGL texture object FBOTextureId. 
 
- (void) renderScene
 sets up an perspective for the 3D look,
 renders a quad with the texture upside down and decreasing
 in intensity as the reflection, renders a quad without the 
 texture to simulate a reflective surface, and finally
 renders another quad with the texture on top of the 
 reflective surface. 
 
 Finally, the OpenGL context is flushed to get the contents to
 the screen. 
 
- (void) dealloc

 Delete the texture and the framebuffer object (FBO).
 
 
References: 

OpenGL Programming Guide for Mac OS X <http://developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/> 
Core Image Programming Guide <http://developer.apple.com/documentation/GraphicsImaging/Conceptual/CoreImaging/index.html>
Core Image Filter Reference <http://developer.apple.com/documentation/GraphicsImaging/Reference/CoreImageFilterReference/index.html>



 