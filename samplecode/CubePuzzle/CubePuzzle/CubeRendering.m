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
#import <AppKit/AppKit.h>
#import "Cube.h"
#import <OpenGL/gl.h>

NSString *CubeDidChangeNotification = @"CubeDidChange";

/* 
   Notes to self:
   
   A lot of this code could be far more generalized if we changed the way we
   store cube state to be represented as an exploded set of cubes in space, where
   each cube hovers right over top of it's face color.  This effectively gives us
   unique 3D positions for every face.   The benefit of this is that we could code
   up fully generalized rotation and rendering functions for any size cube.  This
   could possibly also simplify some of the lower level primitives that check for
   proper orientation and such.
   
*/

/*

         +--+--+--+
         |T0|T1|T2|
         +--+--+--+
         |T3|T4|T5|
         +--+--+--+
         |T6|T7|T8|
+--+--+--+--+--+--+--+--+--+--+--+--+
|L0|L1|L2|F0|F1|F2|R0|R1|R2|P0|P1|P2|
+--+--+--+--+--+--+--+--+--+--+--+--+
|L3|L4|L5|F3|F4|F5|R3|R4|R5|P3|P4|P5|
+--+--+--+--+--+--+--+--+--+--+--+--+
|L6|L7|L8|F6|F7|F8|R6|R7|R8|P6|P7|P8|
+--+--+--+--+--+--+--+--+--+--+--+--+
         |B0|B1|B2|
	 +--+--+--+
         |B3|B4|B5|
	 +--+--+--+
	 |B6|B7|B8|
	 +--+--+--+
	 
*/

static unsigned char cubeColors[7][4] = 
{
	{ 0xff, 0x00, 0x00, 0xff},
	{ 0xff, 0xff, 0x00, 0xff},
	{ 0xff, 0x4f, 0x00, 0xff},
	{ 0x00, 0xef, 0x00, 0xff},
	{ 0xff, 0xff, 0xff, 0xff},
	{ 0x00, 0x00, 0xef, 0xff},
	{ 0x00, 0x00, 0x00, 0xff}
};

typedef struct _Vector
{
    float x;
    float y;
    float z;
} Vector;

// Some simple vector utility routines ripped out of VectorLib.h.
static inline Vector MakeVector(float _x, float _y, float _z)
{
  Vector v;
  v.x = _x; v.y = _y; v.z = _z;
  return v;
}

static inline Vector SumVectors(Vector _a, Vector _b)
{
  _a.x += _b.x;
  _a.y += _b.y;
  _a.z += _b.z;
  return _a;
}

// Increasing the number of segments results in nicer
// looking edges, but increases the number of polygons
// significantly.
#define SEGMENTS 3
#define RADIUS 0.05

static Vector polarToVector(float theta, float phi, float r)
{
	Vector v;
	float st;
	
	st = sin(theta);
	v.x = r * st * cos(phi);
	v.y = r * cos(theta);
	v.z = r * st * sin(phi);
	
	return v;
}

// TODO: Rewrite this whole thing to use vertex arrays for speed.
static void DrawCubeRounded(Vector _center, Vector _size,
	unsigned char left, unsigned char front, unsigned char right, 
	unsigned char back, unsigned char top, unsigned char bottom)
{
	float delta = M_PI_2 / SEGMENTS;
	float t, p;
	Vector v;
	
	// Do colored faces
	glBegin(GL_QUADS);

	// Front
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor4ubv(cubeColors[front]);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y - _size.y + RADIUS,
		   _center.z + _size.z);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y - _size.y + RADIUS,
		   _center.z + _size.z);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y + _size.y - RADIUS,
		   _center.z + _size.z);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y + _size.y - RADIUS,
		   _center.z + _size.z);	

	// Back
	glNormal3f(0.0f, 0.0f, -1.0f);
	glColor4ubv(cubeColors[back]);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y - _size.y + RADIUS,
		   _center.z - _size.z);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y - _size.y + RADIUS,
		   _center.z - _size.z);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y + _size.y - RADIUS,
		   _center.z - _size.z);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y + _size.y - RADIUS,
		   _center.z - _size.z);	

	// Top
	glNormal3f(0.0f, 1.0f, 0.0f);
	glColor4ubv(cubeColors[top]);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y + _size.y,
		   _center.z + _size.z - RADIUS);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y + _size.y,
		   _center.z + _size.z - RADIUS);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y + _size.y,
		   _center.z - _size.z + RADIUS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y + _size.y,
		   _center.z - _size.z + RADIUS);

	// Bottom
	glNormal3f(0.0f, -1.0f, 0.0f);
	glColor4ubv(cubeColors[bottom]);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y - _size.y,
		   _center.z - _size.z + RADIUS);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y - _size.y,
		   _center.z - _size.z + RADIUS);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(_center.x + _size.x - RADIUS,
		   _center.y - _size.y,
		   _center.z + _size.z - RADIUS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(_center.x - _size.x + RADIUS,
		   _center.y - _size.y,
		   _center.z + _size.z - RADIUS);

	// Left
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glColor4ubv(cubeColors[left]);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(_center.x - _size.x,
		   _center.y - _size.y + RADIUS,
		   _center.z - _size.z + RADIUS);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(_center.x - _size.x,
		   _center.y - _size.y + RADIUS,
		   _center.z + _size.z - RADIUS);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(_center.x - _size.x,
		   _center.y + _size.y - RADIUS,
		   _center.z + _size.z - RADIUS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(_center.x - _size.x,
		   _center.y + _size.y - RADIUS,
		   _center.z - _size.z + RADIUS);

	// Right
	glNormal3f(1.0f, 0.0f, 0.0f);
	glColor4ubv(cubeColors[right]);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(_center.x + _size.x,
		   _center.y - _size.y + RADIUS,
		   _center.z + _size.z - RADIUS);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(_center.x + _size.x,
		   _center.y - _size.y + RADIUS,
		   _center.z - _size.z + RADIUS);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(_center.x + _size.x,
		   _center.y + _size.y - RADIUS,
		   _center.z - _size.z + RADIUS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(_center.x + _size.x,
		   _center.y + _size.y - RADIUS,
		   _center.z + _size.z - RADIUS);
	
	glEnd();
	
	// Do rounded edges.  We "know" that we can use
	// a texture coordinate of 0.0f,0.0f to get black.
	// This avoids doing a state change on each cube.
	
	// Note: It's debatable whether separate strips
	// are more efficient than just doing one begin/end
	// with individual quads.
	
	glColor3f(0.0f,0.0f,0.0f);
	glTexCoord2f(0.0f, 0.0f);
//	glMaterialf(GL_FRONT, GL_SHININESS, 20.0f);
	
	// Draw 4 vertical rounded edges.
	
	// Right front.
	glBegin(GL_QUAD_STRIP);
	for(t = 0; t <= SEGMENTS; t++)
	{
		v = polarToVector(M_PI_2, t*delta,1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
					  _center.y - _size.y + RADIUS,
					  _center.z + _size.z - RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
					  _center.y + _size.y - RADIUS,
					  _center.z + _size.z - RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
	}
	glEnd();

	// Left front.
	glBegin(GL_QUAD_STRIP);
	for(t = SEGMENTS; t <= SEGMENTS*2; t++)
	{
		v = polarToVector(M_PI_2, t*delta,1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
					  _center.y - _size.y + RADIUS,
					  _center.z + _size.z - RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
		v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
					  _center.y + _size.y - RADIUS,
					  _center.z + _size.z - RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
	}
	glEnd();
	
	// Left rear.
	glBegin(GL_QUAD_STRIP);
	for(t = SEGMENTS*2; t <= SEGMENTS*3; t++)
	{
		v = polarToVector(M_PI_2, t*delta,1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
					  _center.y - _size.y + RADIUS,
					  _center.z - _size.z + RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
		v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
					  _center.y + _size.y - RADIUS,
					  _center.z - _size.z + RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
	}
	glEnd();

	// Right rear.
	glBegin(GL_QUAD_STRIP);
	for(t = SEGMENTS*3; t <= SEGMENTS*4; t++)
	{
		v = polarToVector(M_PI_2, t*delta,1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
					  _center.y - _size.y + RADIUS,
					  _center.z - _size.z + RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
					  _center.y + _size.y - RADIUS,
					  _center.z - _size.z + RADIUS),
					  polarToVector(M_PI_2, t*delta,RADIUS));
		glVertex3fv(&v.x);
	}
	glEnd();
	
	// Do rounded top edges.
	for(t = 0; t < SEGMENTS; t++)
	{
		glBegin(GL_QUAD_STRIP);
		// Right front top
		for(p = 0; p <= SEGMENTS; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y + _size.y - RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y + _size.y - RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		// Left front top
		for(p = SEGMENTS; p <= SEGMENTS*2; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						  _center.y + _size.y - RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						  _center.y + _size.y - RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		// Left top back
		for(p = SEGMENTS*2; p <= SEGMENTS*3; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						_center.y + _size.y - RADIUS,
						_center.z - _size.z + RADIUS),
						polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						_center.y + _size.y - RADIUS,
						_center.z - _size.z + RADIUS),
						polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		// Right top back
		for(p = SEGMENTS*3; p <= SEGMENTS*4; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y + _size.y - RADIUS,
						  _center.z - _size.z + RADIUS),
						  polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y + _size.y - RADIUS,
						  _center.z - _size.z + RADIUS),
						  polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		// Close right top edge.
		p = 0;
		v = polarToVector((t+1)*delta, p*delta, 1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						_center.y + _size.y - RADIUS,
						_center.z + _size.z - RADIUS),
						polarToVector((t+1)*delta, p*delta, RADIUS));
		glVertex3fv(&v.x);
		
		v = polarToVector(t*delta, p*delta, 1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						_center.y + _size.y - RADIUS,
						_center.z + _size.z - RADIUS),
						polarToVector(t*delta, p*delta, RADIUS));
		glVertex3fv(&v.x);
		
		glEnd();
	}
	
	// Do rounded bottom edges.
	for(t = SEGMENTS; t < SEGMENTS*2; t++)
	{
		glBegin(GL_QUAD_STRIP);
		for(p = 0; p <= SEGMENTS; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		for(p = SEGMENTS; p <= SEGMENTS*2; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z + _size.z - RADIUS),
						  polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		for(p = SEGMENTS*2; p <= SEGMENTS*3; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z - _size.z + RADIUS),
						  polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x - _size.x + RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z - _size.z + RADIUS),
						  polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		for(p = SEGMENTS*3; p <= SEGMENTS*4; p++)
		{
			v = polarToVector((t+1)*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z - _size.z + RADIUS),
						  polarToVector((t+1)*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
			
			v = polarToVector(t*delta, p*delta, 1.0f);
			glNormal3fv(&v.x);
			v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						  _center.y - _size.y + RADIUS,
						  _center.z - _size.z + RADIUS),
						  polarToVector(t*delta, p*delta, RADIUS));
			glVertex3fv(&v.x);
		}
		// Close right bottom edge.
		p = 0;
		v = polarToVector((t+1)*delta, p*delta, 1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						_center.y - _size.y + RADIUS,
						_center.z + _size.z - RADIUS),
						polarToVector((t+1)*delta, p*delta, RADIUS));
		glVertex3fv(&v.x);
		
		v = polarToVector(t*delta, p*delta, 1.0f);
		glNormal3fv(&v.x);
		v = SumVectors(MakeVector(_center.x + _size.x - RADIUS,
						_center.y - _size.y + RADIUS,
						_center.z + _size.z - RADIUS),
						polarToVector(t*delta, p*delta, RADIUS));
		glVertex3fv(&v.x);
		
		glEnd();
	}
}

/* 

  Things we need:

  Base cube renderer

  Cube renderer for each face rotation.  Or we could write
  a fully generalized renderer that could deal with any single
  face rotation at a time.  I'm not convinced that the latter
  is actually possible though since rotations around different
  faces requires different base translations.  Ugh.

*/

@interface Cube(CubeRenderingPrivate)
- (void)setState:(CubeState)newState;
- (void)animateRotationSelector:(SEL)drawSelector angle:(float)angle;

@end
  
@implementation Cube(CubeRendering)

// The 12 edge cubes are:

// BF, BL, BP, BR, FL, FR, FT, LP, LT, PR, PT and RT

// The 8 corner cubes are:

// BFL, BFR, BLP, BPR, FLT, FRT, LPT and PRT

static void drawFLT(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f, 1.0f, 1.0f),
			MakeVector(0.5f,0.5f, 0.5f),
			state->L2,state->F0,6,6,state->T6,6);
}

static void drawFT(CubeState *state)
{
	DrawCubeRounded(MakeVector(0.0f, 1.0f, 1.0f),
			MakeVector(0.5f, 0.5f, 0.5f),
		       6,state->F1,6,6,state->T7,6);
}

static void drawFRT(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f, 1.0f, 1.0f),
			MakeVector(0.5f, 0.5f, 0.5f),
		       6,state->F2,state->R0,6,state->T8,6);
}

static void drawFL(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f, 0.0f, 1.0f),
			MakeVector(0.5f, 0.5f, 0.5f),
		       state->L5,state->F3,6,6,6,6);
}

static void drawF(CubeState *state)
{
	DrawCubeRounded(MakeVector(0.0f, 0.0f, 1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,state->F4,6,6,6,6);
}

static void drawFR(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f,0.0f, 1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,state->F5,state->R3,6,6,6);
}

static void drawBFL(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f,-1.0f, 1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       state->L8,state->F6,6,6,6,state->B0);
}

static void drawBF(CubeState *state)
{
	DrawCubeRounded(MakeVector( 0.0f,-1.0f, 1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,state->F7,6,6,6,state->B1);
}

static void drawBFR(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f,-1.0f, 1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,state->F8,state->R6,6,6,state->B2);
}

static void drawLT(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f, 1.0f,0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       state->L1,6,6,6,state->T3,6);
}

static void drawT(CubeState *state)
{
	DrawCubeRounded(MakeVector(0.0f, 1.0f, 0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,6,6,state->T4,6);
}

static void drawRT(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f, 1.0f, 0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,state->R1,6,state->T5,6);
}

static void drawL(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f, 0.0f, 0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       state->L4,6,6,6,6,6);
}

static void drawR(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f, 0.0f, 0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,state->R4,6,6,6);
}

static void drawBL(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f,-1.0f, 0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       state->L7,6,6,6,6,state->B3);
}

static void drawB(CubeState *state)
{
	DrawCubeRounded(MakeVector( 0.0f,-1.0f, 0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,6,6,6,state->B4);
}

static void drawBR(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f,-1.0f, 0.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,state->R7,6,6,state->B5);
}

static void drawLPT(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f, 1.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       state->L0,6,6,state->P2,state->T0,6);
}

static void drawPT(CubeState *state)
{
	DrawCubeRounded(MakeVector(0.0f, 1.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,6,state->P1,state->T1,6);
}

static void drawPRT(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f, 1.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,state->R2,state->P0,state->T2,6);
}

static void drawLP(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f, 0.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       state->L3,6,6,state->P5,6,6);
}

static void drawP(CubeState *state)
{
	DrawCubeRounded(MakeVector( 0.0f, 0.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,6,state->P4,6,6);
}

static void drawRP(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f, 0.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,state->R5,state->P3,6,6);
}

static void drawBLP(CubeState *state)
{
	DrawCubeRounded(MakeVector(-1.0f,-1.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       state->L6,6,6,state->P8,6,state->B6);
}

static void drawBP(CubeState *state)
{
	DrawCubeRounded(MakeVector( 0.0f,-1.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,6,state->P7,6,state->B7);
}

static void drawBPR(CubeState *state)
{
	DrawCubeRounded(MakeVector( 1.0f,-1.0f,-1.0f),
		       MakeVector( 0.5f, 0.5f, 0.5f),
		       6,6,state->R8,state->P6,6,state->B8);
}

#define LEFT 1
#define RIGHT 2
#define BOTTOM 4
#define TOP 8
#define FRONT 16
#define POST 32

- (void)drawSimple:(unsigned)flags invert:(unsigned)invert
{
	// Assume correct texture is bound already.  We just have to draw the quads in the
	// appropriate colors.
	if(((flags & (FRONT|LEFT|TOP)) != 0) ^ invert)
		drawFLT(&animationState);
	if(((flags & (FRONT|TOP)) != 0) ^ invert)
		drawFT(&animationState);
	if(((flags & (FRONT|RIGHT|TOP)) != 0) ^ invert)
		drawFRT(&animationState);
	if(((flags & (FRONT|LEFT)) != 0) ^ invert)
		drawFL(&animationState);
	if(((flags & FRONT) != 0) ^ invert)
		drawF(&animationState);
	if(((flags & (FRONT|RIGHT)) != 0) ^ invert)
		drawFR(&animationState);
	if(((flags & (BOTTOM|FRONT|LEFT)) != 0) ^ invert)
		drawBFL(&animationState);
	if(((flags & (BOTTOM|FRONT)) != 0) ^ invert)
		drawBF(&animationState);
	if(((flags & (BOTTOM|FRONT|RIGHT)) != 0) ^ invert)
		drawBFR(&animationState);
	if(((flags & (LEFT|TOP)) != 0) ^ invert)
		drawLT(&animationState);
	if(((flags & TOP) != 0) ^ invert)
		drawT(&animationState);
	if(((flags & (RIGHT|TOP)) != 0) ^ invert)
		drawRT(&animationState);
	if(((flags & LEFT) != 0) ^ invert)
		drawL(&animationState);
	if(((flags & RIGHT) != 0) ^ invert)
		drawR(&animationState);
	if(((flags & (BOTTOM|LEFT)) != 0) ^ invert)
		drawBL(&animationState);
	if(((flags & BOTTOM) != 0) ^ invert)
		drawB(&animationState);
	if(((flags & (BOTTOM|RIGHT)) != 0) ^ invert)
		drawBR(&animationState);
	if(((flags & (LEFT|POST|TOP)) != 0) ^ invert)
		drawLPT(&animationState);
	if(((flags & (POST|TOP)) != 0) ^ invert)
		drawPT(&animationState);
	if(((flags & (POST|RIGHT|TOP)) != 0) ^ invert)
		drawPRT(&animationState);
	if(((flags & (LEFT|POST)) != 0) ^ invert)
		drawLP(&animationState);
	if(((flags & POST) != 0) ^ invert)
		drawP(&animationState);
	if(((flags & (RIGHT|POST)) != 0) ^ invert)
		drawRP(&animationState);
	if(((flags & (BOTTOM|LEFT|POST)) != 0) ^ invert)
		drawBLP(&animationState);
	if(((flags & (BOTTOM|POST)) != 0) ^ invert)
		drawBP(&animationState);
	if(((flags & (BOTTOM|POST|RIGHT)) != 0) ^ invert)
		drawBPR(&animationState);
}

- (void)drawXAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,1.0f,0.0f,0.0f);
	[self drawSimple:(LEFT|FRONT|RIGHT|BOTTOM|TOP|POST) invert:0];
	glPopMatrix();
}

- (void)drawYAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,0.0f,1.0f,0.0f);
	[self drawSimple:(LEFT|FRONT|RIGHT|BOTTOM|TOP|POST) invert:0];
	glPopMatrix();
}

- (void)drawZAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,0.0f,0.0f,1.0f);
	[self drawSimple:(LEFT|FRONT|RIGHT|BOTTOM|TOP|POST) invert:0];
	glPopMatrix();
}

- (void)drawLeftAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,-1.0f,0.0f,0.0f);
	[self drawSimple:LEFT invert:0];
	glPopMatrix();
	[self drawSimple:LEFT invert:1];
}

- (void)drawRightAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,1.0f,0.0f,0.0f);
	[self drawSimple:RIGHT invert:0];
	glPopMatrix();
	[self drawSimple:RIGHT invert:1];
}

- (void)drawTopAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,0.0f,1.0f,0.0f);
	[self drawSimple:TOP invert:0];
	glPopMatrix();
	[self drawSimple:TOP invert:1];
}

- (void)drawBottomAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,0.0f,-1.0f,0.0f);
	[self drawSimple:BOTTOM invert:0];
	glPopMatrix();
	[self drawSimple:BOTTOM invert:1];
}

- (void)drawFrontAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,0.0f,0.0f,1.0f);
	[self drawSimple:FRONT invert:0];
	glPopMatrix();
	[self drawSimple:FRONT invert:1];
}

- (void)drawBackAxis:(float)angle
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(-angle,0.0f,0.0f,-1.0f);
	[self drawSimple:POST invert:0];
	glPopMatrix();
	[self drawSimple:POST invert:1];
}

- (void)animateRotationSelector:(SEL)drawSelector angle:(float)angle
{	
	if(showAnimation)
	{
		renderInvocation = [[NSInvocation invocationWithMethodSignature:
			[self methodSignatureForSelector:drawSelector]] retain];
		
		[renderInvocation setTarget:self];
		[renderInvocation setSelector:drawSelector];
		currentAngle = 0.0f;
		angleTarget = angle;
		
		// Set up animation timer for completing this step.
		animationTimer = [[NSTimer timerWithTimeInterval:1.0f/60.0f
			target:self selector:@selector(stepAnimation:) userInfo:nil repeats:YES] retain];
		
		[[NSRunLoop currentRunLoop] addTimer:animationTimer forMode:NSDefaultRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer:animationTimer forMode:NSEventTrackingRunLoopMode];
	}
}

- (void)stepAnimation:(NSTimer *)timer
{
	BOOL done = NO;
	
	if(angleTarget > 0.0f)
	{
		currentAngle += [timer timeInterval] * 720.0f; // FIXME
		if(currentAngle >= angleTarget)
		{
			currentAngle = angleTarget;
			done = YES;
		}
	}
	else
	{
		currentAngle -= [timer timeInterval] * 720.0f; // FIXME
		if(currentAngle <= angleTarget)
		{
			currentAngle = angleTarget;
			done = YES;
		}
	}
	[renderInvocation setArgument:&currentAngle atIndex:2];
	[[NSNotificationCenter defaultCenter]
		postNotificationName:CubeDidChangeNotification
			object:self];
	if(done)
	{
	    [timer invalidate];
	    [timer release];
	    timer = nil;
	    [renderInvocation release];
	    renderInvocation = nil;
	    [self animationComplete];
	}
}

- (void)setShowsAnimation:(BOOL)flag
{
    showAnimation = flag;
}

- (BOOL)showsAnimation
{
    return showAnimation;
}

// The texture is built to have a black pixel border.  Combined with
// bilinear filtering, this gives the "stickers" a nice smooth edge
// (think of it as poor-man's antialiasing), and also lowers the amount
// of geometry required for the cubes.
#define FACE_TEX_SIZE 64
#define FACE_TEX_BORDER 3

// It actually looks better to not use mipmapped textures.  With mipmapping
// enabled, the faces turn really fuzzy at extreme angles.
#define USE_MIPMAPS 0

- (void)makeFaceTexture
{
	unsigned char *tex = calloc(FACE_TEX_SIZE*FACE_TEX_SIZE*sizeof(unsigned char)*4,1);
	unsigned x, y;
	#if USE_MIPMAPS
	float maxAniso;
	#endif
	
	for(y = 0; y < FACE_TEX_SIZE; y++)
	{
		for(x = 0; x < FACE_TEX_SIZE; x++)
		{
			if((y >= FACE_TEX_BORDER) && (y < FACE_TEX_SIZE-FACE_TEX_BORDER) &&
			   (x >= FACE_TEX_BORDER) && (x < FACE_TEX_SIZE-FACE_TEX_BORDER))
			{
				tex[(y*FACE_TEX_SIZE+x)*4+0] = 0xff;
				tex[(y*FACE_TEX_SIZE+x)*4+1] = 0xff;
				tex[(y*FACE_TEX_SIZE+x)*4+2] = 0xff;
				tex[(y*FACE_TEX_SIZE+x)*4+3] = 0xff;
			}
			else
			{
				tex[(y*FACE_TEX_SIZE+x)*4+0] = 0x00;
				tex[(y*FACE_TEX_SIZE+x)*4+1] = 0x00;
				tex[(y*FACE_TEX_SIZE+x)*4+2] = 0x00;
				tex[(y*FACE_TEX_SIZE+x)*4+3] = 0xff;
			}
		}
	}

	glGenTextures(1, &faceTexture);
	glBindTexture(GL_TEXTURE_2D, faceTexture);
	#if USE_MIPMAPS
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, FACE_TEX_SIZE, FACE_TEX_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FACE_TEX_SIZE, FACE_TEX_SIZE, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, tex);
	#endif
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	#if USE_MIPMAPS
	glGetTexParameterfv(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
	#endif
	free(tex);
}
					
- (void)draw
{
	float materialAmbient[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	float materialDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	float materialSpecular[4] = {0.6f, 0.6f, 0.6f, 1.0f};
	float materialEmission[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	// Set OpenGL material properties for the cube
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission);
	glMaterialf(GL_FRONT, GL_SHININESS, 20.0f);

	if(!faceTexture)
		[self makeFaceTexture];
		
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, faceTexture);
	
	if(renderInvocation)
		[renderInvocation invoke];
	else
		[self drawSimple:(LEFT|TOP|RIGHT|BOTTOM|FRONT|POST) invert:0];
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);
}

@end
