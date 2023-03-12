//
// File:       BoingView.m
//
// Abstract:   Main View 
//             This is where the OpenGL rendering happens.
//
// Version:    1.1 - Minor bugs and cosmetic changes.
//			   1.0 - Initial version.
//
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2001-2007 Apple Inc. All Rights Reserved.
//

#import "BoingView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/OpenGL.h>

@implementation BoingView

- (id)initWithFrame:(NSRect)frame
{
	NSOpenGLPixelFormatAttribute attribsNice[] = 
		{NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFANoRecovery,
		kCGLPFASampleBuffers, 1, kCGLPFASamples, 2,
		0};

	NSOpenGLPixelFormatAttribute attribsJaggy[] = 
		{NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFANoRecovery,
		0};
	
	NSOpenGLPixelFormat *fmt;

	/* Choose a pixel format */
	fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribsNice];
	
	if(fmt)
		doMultiSampleStuff = YES;
	else
		fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribsJaggy];
		
	self = [self initWithFrame:frame pixelFormat:fmt];
	
	[fmt release];
	
	r = 48.0f;
	
	xVelocity = 1.5f;
	yVelocity = 0.0f;
	xPos = r*2.0f;
	yPos = r*3.0f;
	return self;
}

- (BOOL)isOpaque
{
	return YES;
}

-(void)generateBoingData
{
	int x;
	int index = 0;
	
	float v1x, v1y, v1z;
	float v2x, v2y, v2z;
	float d;
	
	int theta, phi;
	
	float theta0, theta1;
	float phi0, phi1;
	
	Vertex vtx[4];
	
	float delta = M_PI / 8.0f;

	// 8 vertical segments
	for(theta = 0; theta < 8; theta++)
	{
		theta0 = theta*delta;
		theta1 = (theta+1)*delta;
		
		// 16 horizontal segments
		for(phi = 0; phi < 16; phi++)
		{
			phi0 = phi*delta;
			phi1 = (phi+1)*delta;
			
			// For now, generate 4 full points.			
			vtx[0].x = r * sin(theta0)*cos(phi0);
			vtx[0].y = r * cos(theta0);
			vtx[0].z = r * sin(theta0)*sin(phi0);

			vtx[1].x = r * sin(theta0)*cos(phi1);
			vtx[1].y = r * cos(theta0);
			vtx[1].z = r * sin(theta0)*sin(phi1);

			vtx[2].x = r * sin(theta1)*cos(phi1);
			vtx[2].y = r * cos(theta1);
			vtx[2].z = r * sin(theta1)*sin(phi1);

			vtx[3].x = r * sin(theta1)*cos(phi0);
			vtx[3].y = r * cos(theta1);
			vtx[3].z = r * sin(theta1)*sin(phi0);
			
			// Generate normal
			if(theta >= 4)
			{
				v1x = vtx[1].x - vtx[0].x;
				v1y = vtx[1].y - vtx[0].y;
				v1z = vtx[1].z - vtx[0].z;
	
				v2x = vtx[3].x - vtx[0].x;
				v2y = vtx[3].y - vtx[0].y;
				v2z = vtx[3].z - vtx[0].z;
			}
			else
			{
				v1x = vtx[0].x - vtx[3].x;
				v1y = vtx[0].y - vtx[3].y;
				v1z = vtx[0].z - vtx[3].z;
	
				v2x = vtx[2].x - vtx[3].x;
				v2y = vtx[2].y - vtx[3].y;
				v2z = vtx[2].z - vtx[3].z;
			}
			
			vtx[0].nx = (v1y * v2z) - (v2y * v1z);
			vtx[0].ny = (v1z * v2x) - (v2z * v1x);
			vtx[0].nz = (v1x * v2y) - (v2x * v1y);
			
			d = 1.0f/sqrt(vtx[0].nx*vtx[0].nx + 
				      vtx[0].ny*vtx[0].ny +
				      vtx[0].nz*vtx[0].nz);
			
			vtx[0].nx *= d;
			vtx[0].ny *= d;
			vtx[0].nz *= d;
			
			// Generate color			
			if((theta ^ phi) & 1)
			{
				vtx[0].r = 1.0f;
				vtx[0].g = 1.0f;
				vtx[0].b = 1.0f;
				vtx[0].a = 1.0f;			
			}
			else
			{
				vtx[0].r = 1.0f;
				vtx[0].g = 0.0f;
				vtx[0].b = 0.0f;
				vtx[0].a = 1.0f;
			}
			
			// Replicate vertex info
			for(x = 0; x < 4; x++)
			{
				vtx[x].nx = vtx[0].nx;
				vtx[x].ny = vtx[0].ny;
				vtx[x].nz = vtx[0].nz;
				vtx[x].r = vtx[0].r;
				vtx[x].g = vtx[0].g;
				vtx[x].b = vtx[0].b;
				vtx[x].a = vtx[0].a;
			}
			
			// Store vertices
			boingData[index++] = vtx[0];
			boingData[index++] = vtx[1];
			boingData[index++] = vtx[2];
			boingData[index++] = vtx[3];			
		}
	}
}

float light0Position[4]    = {-2.0, 2.0, 1.0, 0.0 };

float light0Ambient[4]     = { 0.2, 0.2, 0.2, 0.2 };
float light0Diffuse[4]     = { 1.0, 1.0, 1.0, 1.0 };
float light0Specular[4]    = { 1.0, 1.0, 1.0, 1.0 };

float materialShininess[4] = { 10.0, 0.0, 0.0, 0.0 };

float materialAmbient[4]   = { 1.0, 1.0, 1.0, 1.0 };
float materialDiffuse[4]   = { 1.0, 1.0, 1.0, 1.0 };
float materialSpecular[4]  = { 1.0, 1.0, 1.0, 1.0 };

float materialEmission[4]  = { 0.0, 0.0, 0.0, 0.0 };

- (void)drawRect:(NSRect)theRect
{
	int i;
	float lightModelAmbient[4];
	float lightSpecular[4];
	
	if(!didInit)
	{
		long opaque = NO;
		[[NSColor clearColor] set];
		NSRectFill([self bounds]);
		[[self openGLContext] setValues:&opaque forParameter:NSOpenGLCPSurfaceOpacity];
		[[self window] setOpaque:NO];
		[[self window] setAlphaValue:.999f];
		[[self window] setMovableByWindowBackground:YES];
		
		[self generateBoingData];
		didInit = YES;
		screenBounds = [[NSScreen mainScreen] frame];
		angleDelta = -2.5f;
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		[[self openGLContext] flushBuffer];	
		bgFade = 1.0f;
		lightFactor = 0.0f;
		scaleFactor = 1.0f;
		bounceInsideWindow = YES;
		if(doMultiSampleStuff)
		{
			glDisable(GL_MULTISAMPLE_ARB);			
			glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
		}
	}
	
	if(enableMultiSample)
		glEnable(GL_MULTISAMPLE_ARB);
		
	if(doTransparency == 2)
	{	
		bgFade -= 0.05f;
		if(bgFade < 0.0f)
			bgFade = 0.0f;
	}
	
	if(doLight)
	{
		lightFactor += 0.005f;
		if(lightFactor > 1.0f)
		{
			lightFactor = 1.0f;
			doLight = NO;
		}
	}

	if(doScale)
	{
		float oldr = r;
		scaleFactor += 0.025f;
		r = scaleFactor * 48.0f;
		yPos += r - oldr;
		
		if(scaleFactor > 2.0f)
		{
			scaleFactor = 2.0f;
			doScale = NO;
		}
		[self generateBoingData];
	}
		
	glViewport(0,0,[self bounds].size.width,[self bounds].size.height);
	
	glScissor(0,0,320*scaleFactor,200*scaleFactor);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.675f*bgFade,0.675f*bgFade,0.675f*bgFade,bgFade);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,[self bounds].size.width,0,[self bounds].size.height, 0.0f, 2000.0);
	
	glDisable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	lightModelAmbient[0] = 1.0f - lightFactor;
	lightModelAmbient[1] = 1.0f - lightFactor;
	lightModelAmbient[2] = 1.0f - lightFactor;
	lightModelAmbient[3] = 1.0f - lightFactor;	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);
	
	// Directional white light
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light0Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0Diffuse);
	lightSpecular[0] = lightFactor;
	lightSpecular[1] = lightFactor;
	lightSpecular[2] = lightFactor;
	lightSpecular[3] = lightFactor;
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	
	// Material properties
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission);
	
	
	// Reset transformation matrix and set light position.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, light0Position);	

	// I'm doing the scaling myself (rather than using OpenGL) just because
	// I want to get exact positioning of the lines.  There is probably a better
	// way to do this if I'd try harder with the math.  Hey, it's a demo. ;)
	if(bgFade > 0.0f)
	{
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
	
		glBegin(GL_LINES);	
		glColor4f(0.6275*bgFade, 0.0f, 0.6275 * bgFade, bgFade);
		for(i = 40; i <= 280; i+= 16)
		{
			glVertex3f(floor(i*scaleFactor)+0.5f,
			           floor(7*scaleFactor)+0.5f,-500.0f);
			glVertex3f(floor(i*scaleFactor)+0.5f,(200*scaleFactor)+0.5f,-500.0f);
	
			// Do stragglers along the bottom. Not exactly the same as
			// the original, but close enough.
			glVertex3f(floor(i*scaleFactor)+0.5f,
			           floor(7*scaleFactor)+0.5f,-500.0f);
			glVertex3f((i- 160)*scaleFactor*1.1 + 160.0f*scaleFactor,-0.5f,-500.0f);
		}
		for(i = 8; i <= 200; i+= 16)
		{
			glVertex3f(40.0f*scaleFactor,floor(i*scaleFactor)-0.5f,-500.0f);
			glVertex3f(280.0f*scaleFactor,floor(i*scaleFactor)-0.5f,-500.0f);
		}
		// Do final two horizontal lines
		glVertex3f(floor((40.0f-3.0f)*scaleFactor) + 0.5f ,
			   floor((7.0f-2.0f)*scaleFactor) + 0.5f,-500.0f);
		glVertex3f(floor((280.0f+3.0f)*scaleFactor) + 0.5f,
		           floor((7.0-2.0f)*scaleFactor) + 0.5f,-500.0f);
		glVertex3f(floor((40.0f-8.0f)*scaleFactor) + 0.5f,
		           floor((7.0f-5.0f)*scaleFactor) + 0.5f,-500.0f);
		glVertex3f(floor((280.0f+8.0f)*scaleFactor) + 0.5f,
			   floor((7.0f-5.0f)*scaleFactor) + 0.5f,-500.0f);
		glEnd();
	}

	glLoadIdentity();
	
	// Draw "shadow"
	glEnable(GL_CULL_FACE);
	if(bounceInsideWindow)
		glTranslatef(xPos+10, yPos-2, -800.0f);
	else
		glTranslatef(r+10, r-2, -800.0f);	
	glRotatef(-16.0f,0.0f,0.0f,1.0f);
	glRotatef(angle, 0.0f, 1.0f, 0.0f);
	glScalef(1.05f,1.05f,1.05f);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.4f);
	for(i = 0; i < 4 * 8 * 16; i++)
	{
		glNormal3fv(&boingData[i & ~3].nx);
		glVertex3fv(&boingData[i].x);
	}	
	glEnd();

	// Draw real boing
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);	
	glDisable(GL_BLEND);
	
	glLoadIdentity();

	if(bounceInsideWindow)
		glTranslatef(xPos, yPos, -100.0f);
	else
		glTranslatef(r, r, -100.0f);		
	glRotatef(-16.0f,0.0f,0.0f,1.0f);
	glRotatef(angle, 0.0f, 1.0f, 0.0f);
	
	glBegin(GL_QUADS);
	for(i = 0; i < 4 * 8 * 16; i++)
	{
		glColor4fv(&boingData[i].r);
		glNormal3fv(&boingData[i & ~3].nx);
		glVertex3fv(&boingData[i].x);
	}	
	glEnd();

	glDisable(GL_LIGHTING);
	
	angle += angleDelta;
	if(angle < 0.0f)
		angle += 360.0f;
	else if(angle > 360.0f)
		angle -= 360.0f;

	[[self openGLContext] flushBuffer];	
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	NSPoint origin;
	
	origin = [[self window] frame].origin;
	
	origin.x += [theEvent deltaX];
	origin.y -= [theEvent deltaY];
	
	[[self window] setFrameOrigin:origin];
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	angleDelta = 2.0f - angleDelta;
}

- (void)transition
{
	// Now we are going to switch to window movement mode for the bounce.
	// Move window origin such that when the ball is locked at r,r the ball doesn't
	// appear to move from it's current location.
	NSRect frame;
	
	bounceInsideWindow = NO;
	
	frame.origin = [[self window] frame].origin;
	
	frame.origin.x += xPos - r;
	frame.origin.y += yPos - r;
	frame.size.width = r*2 + 20;
	frame.size.height = r*2 + 20;
	
	// Convert xPos,yPos to screen coordinates
	xPos = frame.origin.x+r;
	yPos = frame.origin.y+r;
	
	// Move window
	[[self window] setFrame:frame display:YES];
	
	[[self openGLContext] update];
	[self display];
	[[self window] flushWindow];
	
}

- (void)animate
{
	// Do bouncy stuff
	if(bounceInsideWindow)
	{
		yVelocity -= 0.05f;

		xPos += xVelocity*scaleFactor;
		yPos += yVelocity*scaleFactor;		
		
		if(xPos < (r+10.0f))
		{
			if(doTransparency == 2)
				[self transition];
			else
			{
				xPos = r+10.f;
				xVelocity = -xVelocity;
				angleDelta = -angleDelta;
			}
		}
		else if(xPos > (310*scaleFactor-r))
		{
			if(doTransparency == 2)
				[self transition];
			else
			{
				xPos = 310*scaleFactor-r;
				xVelocity = -xVelocity;
				angleDelta = -angleDelta;
			}
		}
		if(yPos < r)
		{
			if(doTransparency < 2)
			{
				yPos = r;
				yVelocity = -yVelocity;
			}
			if(doTransparency == 1)
			{
				doTransparency = 2;
			}
			else if(doTransparency == 2)
			{
				[self transition];
			}
		}
		if(bounceInsideWindow)
			[self display];
	}
	else
	{
		NSRect frame;

		yVelocity -= 0.1f;

		xPos += xVelocity*scaleFactor;
		yPos += yVelocity*scaleFactor;		
		
		frame = [[self window] frame];
		
		if(xPos < (r+10.0f))
		{
			xPos = r+10.f;
			xVelocity = -xVelocity;
			angleDelta = -angleDelta;
		}
		else if(xPos > ((NSMaxX(screenBounds)-10)-r))
		{
			xPos = (NSMaxX(screenBounds)-10)-r;
			xVelocity = -xVelocity;
			angleDelta = -angleDelta;
		}
		if(yPos < r)
		{
			yPos = r;
			yVelocity = -yVelocity;
		}
		frame.origin.x = xPos - r;
		frame.origin.y = yPos - r;
		
		[self setNeedsDisplay:YES];
		[[self window] setFrameOrigin:frame.origin];
	}
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	return YES;
}

- (BOOL)resignFirstResponder
{
	return YES;
}

- (void)doEventForCharacter:(unichar)character downEvent:(BOOL)flag
{
	unsigned long bit;
	
	bit = 0;
	switch(character)
	{
		case 's':
			doScale = YES;
			break;
		case 'l':
			doLight = YES;
			break;
		case 't':
			doTransparency = 1;
			break;
		case 'm':
			enableMultiSample = YES;
			break;
	}
}

- (void)keyDown:(NSEvent *)theEvent
{
	NSString *characters;
	unsigned int characterIndex, characterCount;
	
	characters = [theEvent charactersIgnoringModifiers];
	characterCount = [characters length];

	for (characterIndex = 0; characterIndex < characterCount; characterIndex++) {
		[self doEventForCharacter:[characters characterAtIndex:characterIndex] downEvent:YES];
	}
}

@end
