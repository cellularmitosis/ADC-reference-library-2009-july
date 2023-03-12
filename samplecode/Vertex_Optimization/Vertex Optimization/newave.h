
#import <Cocoa/Cocoa.h>
#import <Foundation/NSObject.h>

#include <math.h>
#include <stdlib.h>

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

#define SQRTOFTWOINV (1.0f / 1.414213562f)

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define MAXGRID       180

typedef struct {
	GLfloat x, y, z;
} Vector;

typedef struct {
	GLfloat x, y, z, w;
} Vector4;

typedef struct {
	GLfloat s, t;
} TextureCoord;

typedef struct {
	Vector4 	position;
	Vector4 	normal;
        TextureCoord	texcoord;
} Vertex;

typedef struct {
        GLfloat	x, y, z, pad0;
        GLfloat	amp, freq, decay, pad1;
} Emitter;

@interface WaveOject : NSObject
{
	GLushort	elements[MAXGRID-1][MAXGRID*2];
	Vertex  	(*vn_ptr[2])[MAXGRID][MAXGRID];
	Vector4 	(*faceNorms[2])[MAXGRID][MAXGRID];
	GLfloat 	veloc[MAXGRID][MAXGRID];

	GLfloat		xPosDat[MAXGRID*MAXGRID + 16];
	GLfloat		yPosDat[MAXGRID*MAXGRID + 16];
        
	GLfloat		sim_time;
        
	GLfloat 	sphi;
	GLfloat 	stheta;
	GLfloat 	sdepth;
	
	GLint 		downX;
	GLint 		downY;
	
	GLint 		opt_level;
        
	GLint		fillmode;
	
	Emitter		emitters[4];
	GLuint		fences[2];
}

- (void)WaveMotion:(GLint) buffer;
- (GLuint)WaveDisplay:(GLint) buffer;

- (void)WaveSetDisplay:(GLint)level;

- (void)WaveReshape:(GLint)width:(GLint)height;
- (void)WaveMouseDown:(GLint)x:(GLint)y;
- (void)WaveMouseMotion:(GLint)x:(GLint)y;

- (void)WaveSetFillMode:(GLint)mode;

@end


