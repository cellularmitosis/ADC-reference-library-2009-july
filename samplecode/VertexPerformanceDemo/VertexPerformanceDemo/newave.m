
#import "newave.h"

#import "Textures.h"

#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/OpenGL.h>

@implementation WaveOject

- (id)init
{
    GLint 			i, j, k;
    unsigned int	byteStride;
    GLuint			textureId;
	GLuint	w, h;
    
    [super init];
    
    sphi      = 90.0f;
    stheta    = 45.0f;
    sdepth    = 5.0f / 4.0f * (GLfloat) MAXGRID;
    
    downX     = 0;
    downY     = 0;
    
    sim_time  = 0;
	
    vn_ptr[0] = (void *)((GLuint)malloc(sizeof(*vn_ptr[0]) + 0xf) & ~0xf);
    vn_ptr[1] = (void *)((GLuint)malloc(sizeof(*vn_ptr[1]) + 0xf) & ~0xf);
    faceNorms[0] = (void *)((GLuint)malloc(sizeof(*faceNorms[0]) + 0xf) & ~0xf);
    faceNorms[1] = (void *)((GLuint)malloc(sizeof(*faceNorms[1]) + 0xf) & ~0xf);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
            
    // Load our texture
    {
        NSImage * image = [NSImage imageNamed:@"macosxlogo"];
        
        textureId = 0;

        glGenTextures(1, &textureId);
        
        TextureFromNSImage(image, &textureId, &w, &h);
    
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, textureId);
        
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glEnable(GL_TEXTURE_RECTANGLE_EXT);                
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(64.0f, 5.0f / 4.0f, (GLfloat) MAXGRID / 10.0f, (GLfloat) MAXGRID * 3.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
    glTranslatef(0.0f,0.0f,-sdepth);
    glRotatef(-stheta, 1.0f, 0.0f, 0.0f);
    glRotatef(sphi, 0.0f, 0.0f, 1.0f);
    glTranslatef(-(GLfloat)((MAXGRID+1)/2-1), -(GLfloat)((MAXGRID+1)/2-1), 0.0f);
    
    // Generate fences for vertex array ranges
    glGenFencesAPPLE(2, fences);

    // Compute byteStride from one vertex to another for vertex arrays
    byteStride = sizeof(Vertex);
	
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// (Slow)
	CGLEnable(CGLGetCurrentContext(), kCGLCPSwapInterval);
    
    for (i = 0; i < MAXGRID - 1; i++)
    {
        k = 0;
        for (j = 0; j < MAXGRID; j++)
        {
            elements[i][k++] =     i * MAXGRID + j;
            elements[i][k++] = (i+1) * MAXGRID + j;
        }
    }
    
    for(k=0; k<NUMBUFFERS; k++)
    {
		// (Fast) glBindBuffer(GL_ARRAY_BUFFER_ARB, k+1);
		// (Fast) glVertexPointer(3, GL_FLOAT, byteStride, 0);
		// (Fast) glTexCoordPointer (2, GL_FLOAT, byteStride, sizeof(Vector4));
	
		// (Slow)
		glVertexPointer(3, GL_FLOAT, byteStride, &(*vn_ptr[k])[0][0].position.x);
		// (Slow)
		glTexCoordPointer (2, GL_FLOAT, byteStride, &(*vn_ptr[k])[0][0].texcoord.s);
	
        for(i = 0; i < MAXGRID; i++)
        {
            GLfloat t = (GLfloat) w * (GLfloat) (i) / (GLfloat)(MAXGRID-1);
             
            for(j = 0; j < MAXGRID; j++)
            {
                (*vn_ptr[k])[i][j].position.x = (GLfloat) i;
                (*vn_ptr[k])[i][j].position.y = (GLfloat) j;
                (*vn_ptr[k])[i][j].position.z = 0.0;
                
                (*vn_ptr[k])[i][j].texcoord.s = (GLfloat) h * (GLfloat) (j) / (GLfloat)(MAXGRID-1);
                (*vn_ptr[k])[i][j].texcoord.t = t;
            }
        }
		
		// (Fast) glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(*vn_ptr[k]), (*vn_ptr[k]), GL_DYNAMIC_DRAW_ARB);
    }
    
    return self;
}

- (void) WaveMotion:(GLint) buffer
{
    GLint	i, k, j;
    Vertex	(*vbuffer)[MAXGRID][MAXGRID];
	vector float 	zero		= (vector float)(0.0,          0.0,          0.0,          0.0);
	vector float 	epsilon		= (vector float)(5.0e-6, 	   5.0e-6,       5.0e-6,       5.0e-6);
	vector float 	ptOne		= (vector float)(0.1,          0.1,          0.1,          0.1);
	vector float 	half		= (vector float)(0.5,          0.5,          0.5,          0.5);
	vector float 	pt375		= (vector float)(0.375,        0.375,        0.375,        0.375);
	vector float 	one			= (vector float)(1.0,          1.0,          1.0,          1.0);
	vector float 	infinite	= (vector float)(5.0e10,       5.0e10, 	     5.0e10,       5.0e10);
	vector float 	_2Pi		= (vector float)(6.2831853071, 6.2831853071, 6.2831853071, 6.2831853071);
	vector float 	inv2Pi		= (vector float)(0.1591549439, 0.1591549439, 0.1591549439, 0.1591549439);
	vector float 	invFac[20];
	vector float 	emitterX[4], emitterY[4], emitterAmp[4], emitterFreq[4], emitterDecay[4];
	vector float 	sim_timeVec;
	register GLfloat		fact, *pTemp;
	register GLfloat		*xPos;
	register GLfloat		*yPos;
    
	// (Fast) glBindBuffer(GL_ARRAY_BUFFER_ARB, buffer + 1);
	// (Fast) vbuffer = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
	
	// (Slow) 
	vbuffer = (*vn_ptr[buffer]);

    sim_time += 0.5;
    
    emitters[0].x		= 0;
    emitters[0].y		= 0;
    emitters[0].z		= 0;
    emitters[0].amp		= 5;
    emitters[0].freq	= 1.2;
    emitters[0].decay	= 20;

    emitters[1].x		= 100;
    emitters[1].y		= 60;
    emitters[1].z		= 0;
    emitters[1].amp		= 8;
    emitters[1].freq	= 0.8;
    emitters[1].decay	= 40;

    emitters[2].x		= 200;
    emitters[2].y		= 150;
    emitters[2].z		= 0;
    emitters[2].amp		= 3;
    emitters[2].freq	= 0.3;
    emitters[2].decay	= 10;

    emitters[3].x		= 0;
    emitters[3].y		= 50;
    emitters[3].z		= 0;
    emitters[3].amp		= 2;
    emitters[3].freq	= 2.0;
    emitters[3].decay	= 20;
	
	// Align x and y pos arrays to 16 bytes
	xPos = (GLfloat *)(((GLuint)&xPosDat[0] + 0xf) & ~0xf);
	yPos = (GLfloat *)(((GLuint)&yPosDat[0] + 0xf) & ~0xf);
	
	// Load the sim time vector
	pTemp = (GLfloat *)&sim_timeVec;
	*pTemp = sim_time;
	vec_st(vec_splat(vec_ld(0, &sim_timeVec), 0), 0, &sim_timeVec);

	// Fill in the emitter vectors
	for(k=0; k<3; k++)
	{   
		pTemp = (GLfloat *)&emitterX[k];
		*pTemp = emitters[k].x;
		vec_st(vec_splat(vec_ld(0, &emitterX[k]), 0), 0, &emitterX[k]);

		pTemp = (GLfloat *)&emitterY[k];
		*pTemp = emitters[k].y;
		vec_st(vec_splat(vec_ld(0, &emitterY[k]), 0), 0, &emitterY[k]);

		pTemp = (GLfloat *)&emitterAmp[k];
		*pTemp = emitters[k].amp;
		vec_st(vec_splat(vec_ld(0, &emitterAmp[k]), 0), 0, &emitterAmp[k]);

		pTemp = (GLfloat *)&emitterFreq[k];
		*pTemp = emitters[k].freq;
		vec_st(vec_splat(vec_ld(0, &emitterFreq[k]), 0), 0, &emitterFreq[k]);

		pTemp = (GLfloat *)&emitterDecay[k];
		*pTemp = emitters[k].decay;
		vec_st(vec_splat(vec_ld(0, &emitterDecay[k]), 0), 0, &emitterDecay[k]);
	}

	// Compute our power basis
	for(i=1, fact=1; i<17; i++)
	{
		fact = fact * i;
		
		// pos inv fac
		pTemp = (GLfloat *)&invFac[i];
		*pTemp = 1.0 / fact;
		vec_st(vec_splat(vec_ld(0, &invFac[i]), 0), 0, &invFac[i]);
	}

	// Copy xy grid to something we can use
	for(i=0; i<MAXGRID; i++)
	{
		register	unsigned int ii = i * MAXGRID;

		for(j=0; j<MAXGRID; j++)
		{
			xPos[ii + j] = (*vbuffer)[i][j].position.x;
			yPos[ii + j] = (*vbuffer)[i][j].position.y;
		}
	}

	for(i=0; i<MAXGRID; i++)
	{
		register vector float xVec, yVec;
		register vector float tempVec, temp_xVec, temp_yVec, distVec, cosVec, expVec;
		register vector float vecPow2, vecPow4, vecPow6, vecPow8;
		register vector float vecPow10, vecPow12, vecPow14, vecPow16;
		register vector float vecPowN;
		register vector float t0, t1, t2;
		register vector float invFac2, invFac4, invFac6, invFac8, invFac10;
		register vector float y0;
		register vector float invDistVec;
		register vector float temp_zVec;
		register unsigned int ii = i * MAXGRID;
		vector float zVec;
		
		// 3 fps... we just ran out of registers...
		invFac2		= invFac[2];
		invFac4		= invFac[4];
		invFac6		= invFac[6];
		invFac8		= invFac[8];
		invFac10	= invFac[10];
		
		for(j=0; j<MAXGRID; j+=4)
		{
			xVec 		= vec_ld(0, (vector float *)&xPos[ii + j]);
			yVec 		= vec_ld(0, (vector float *)&yPos[ii + j]);
			temp_zVec 	= vec_sub(xVec, xVec);	// avoid a zero load
			
			for(k=0; k<3; k++)
			{        
				temp_xVec = vec_sub(emitterX[k], xVec);
				temp_yVec = vec_sub(emitterY[k], yVec);
				
				// dist = sqrt(x*x + y*y) * 0.1;
				distVec = vec_madd(temp_yVec, temp_yVec, vec_madd(temp_xVec, temp_xVec, zero));
				
				// single second order newton rhapson iteration
				distVec = vec_max(epsilon, distVec);

				y0 = vec_rsqrte(distVec);
				t0 = vec_madd(y0, y0, zero);
				t1 = vec_madd(y0, half, zero);
				t2 = vec_madd(y0, pt375, zero);
				t0 = vec_nmsub(distVec, t0, one);
				invDistVec = vec_madd(t0, vec_madd(t2, t0, t1), y0);

				// compute the distVec
				invDistVec = vec_max(epsilon, invDistVec);
				y0 = vec_re(invDistVec);
				distVec = vec_madd(y0, vec_nmsub(y0, invDistVec, one), y0);
				distVec = vec_madd(distVec, ptOne, zero);
										
				// z += amp * cos(freq * dist + sim_time) / exp(decay / dist);
				// compute freq * dist + sim_time
				tempVec = vec_madd(emitterFreq[k], distVec, sim_timeVec);

				// bound tempVec to 0 <-> 2pi
				// force expression evaluator to make this all one expression
				tempVec = vec_nmsub(vec_trunc(vec_madd(tempVec, inv2Pi, zero)), _2Pi, tempVec);
				
				// compute cos(freq * dist + sim_time)
				// Cos = sum (-1)^n*x^2n/2n!
				// compute sum of products for cos(freq * dist + sim_time)
				// remove some of the dependent loads
				vecPow2=vec_madd(tempVec, tempVec, zero);		// x^2
				vecPow6=vec_madd(vecPow2, tempVec, zero);		// x^3...
				vecPow4=vec_madd(vecPow2, vecPow2, zero);		// x^4

				vecPow6=vec_madd(vecPow6, vecPow6, zero);		// x^6

				vecPow8=vec_madd(vecPow4, vecPow4, zero);		// x^8

				vecPow10=vec_madd(vecPow6, vecPow4, zero);		// x^10

				vecPow12=vec_madd(vecPow6, vecPow6, zero);		// x^12

				vecPow14=vec_madd(vecPow8, vecPow6, zero);		// x^14

				vecPow16=vec_madd(vecPow8, vecPow8, zero);		// x^16
				
				// force the evaluator to madd this whole thingy adds 5-10fps...lol
				cosVec   = vec_madd( vecPow16, invFac[16], vec_nmsub(vecPow14, invFac[14], vec_madd( vecPow12, invFac[12], vec_nmsub(vecPow10, invFac10, vec_madd(  vecPow8,  invFac8, vec_nmsub( vecPow6,  invFac6, vec_madd(  vecPow4,  invFac4, vec_nmsub( vecPow2,  invFac2, one))))))));	// n = 8

				// amp * cos(freq * dist + sim_time)
				cosVec   = vec_madd(cosVec, emitterAmp[k], zero);
				
				// compute decay / dist
				tempVec = vec_madd(emitterDecay[k], invDistVec, zero);
				
				// power series expansion for exp(x)
				// this adds 5 fps maybe 8-10
				vecPowN	= vec_madd( tempVec, tempVec, zero);		// x^2
				expVec  = vec_madd( vecPowN, invFac[2], vec_add( tempVec, one));	// n = 2

				vecPowN	= vec_madd( vecPowN, tempVec, zero);		// x^3
				expVec  = vec_madd( vecPowN, invFac[3], expVec);	// n = 3

				vecPowN	= vec_madd( vecPowN, tempVec, zero);		// x^4
				expVec  = vec_madd( vecPowN, invFac[4], expVec);	// n = 4

				vecPowN	= vec_madd( vecPowN, tempVec, zero);		// x^5
				expVec	= vec_min(infinite, vec_madd( vecPowN, invFac[5], expVec));

				// compute the invExpVec
				y0 = vec_re(expVec);
				// compute the rest of the invExpVec and accum onto z.... 2 fps...more
				temp_zVec = vec_madd(cosVec, vec_madd(y0, vec_nmsub(y0, expVec, one), y0), temp_zVec);
			}
			
			// dump vector into interleaved array
			zVec = temp_zVec;		// single store vs. 3 load / stores

			pTemp = (float *)&zVec;
			(*vbuffer)[i][  j].position.z = pTemp[0];
			(*vbuffer)[i][j+1].position.z = pTemp[1];
			(*vbuffer)[i][j+2].position.z = pTemp[2];
			(*vbuffer)[i][j+3].position.z = pTemp[3];
		}
	}
	
	// (Fast) glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
}

- (GLuint)WaveDisplay:(GLint) buffer
{
	GLint i;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// (Fast) glBindBuffer(GL_ARRAY_BUFFER_ARB, buffer + 1);

	for(i = 0; i < (MAXGRID - 1); i++)
		glDrawElements(GL_TRIANGLE_STRIP, 2 * MAXGRID, GL_UNSIGNED_SHORT, elements[i]);
		
	// (Slow)
	glFinish();
	
	return (((MAXGRID - 1)) * ((MAXGRID - 1)) * 2);
}

- (void)WaveReshape:(GLint)width:(GLint)height
{
    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(64.0, (GLfloat)width/(GLfloat)height, (GLfloat) MAXGRID / 10.0f, (GLfloat) MAXGRID * 3.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
    
    glTranslatef(0.0,0.0,-sdepth);
    glRotatef(-stheta, 1.0, 0.0, 0.0);
    glRotatef(sphi, 0.0, 0.0, 1.0);
    glTranslatef(-(GLfloat)((MAXGRID+1)/2-1), -(GLfloat)((MAXGRID+1)/2-1), 0.0);
}

- (void)WaveMouseDown:(GLint)x:(GLint)y
{
    downX = x;
    downY = y;
}

- (void)WaveMouseMotion:(GLint)x:(GLint)y
{
    sphi   += (GLfloat) (x - downX) / 2.0f;
    stheta -= (GLfloat) (downY - y) / 2.0f;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
    
    glTranslatef(0.0,0.0,-sdepth);
    glRotatef(-stheta, 1.0, 0.0, 0.0);
    glRotatef(sphi, 0.0, 0.0, 1.0);
    glTranslatef(-(GLfloat)((MAXGRID+1)/2-1), -(GLfloat)((MAXGRID+1)/2-1), 0.0);
    
    downX = x;
    downY = y;
}

@end
