
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
    const char 		*pCstring;
    GLuint			textureId;
    
    [super init];
    
    sphi      = 90.0f;
    stheta    = 45.0f;
    sdepth    = 5.0f / 4.0f * (GLfloat) MAXGRID;
    
    downX     = 0;
    downY     = 0;
    
    opt_level = 0;
    sim_time  = 0;
    
	fillmode  = 1;
	
    vn_ptr[0] = (void *)((GLuint)malloc(sizeof(*vn_ptr[0]) + 0xf) & ~0xf);
    vn_ptr[1] = (void *)((GLuint)malloc(sizeof(*vn_ptr[1]) + 0xf) & ~0xf);
    faceNorms[0] = (void *)((GLuint)malloc(sizeof(*faceNorms[0]) + 0xf) & ~0xf);
    faceNorms[1] = (void *)((GLuint)malloc(sizeof(*faceNorms[1]) + 0xf) & ~0xf);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
            
    // Load our texture
    {
        NSImage * image = [NSImage imageNamed:@"macosxlogo"];
        GLuint	w, h;
        
        textureId = 0;

        glGenTextures(1, &textureId);
        
        TextureFromNSImage(image, &textureId, &w, &h);
    
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glEnable(GL_TEXTURE_2D);                
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
    
    pCstring = [[NSString stringWithContentsOfFile: @"WaveSurface.vsh"] cString];
    
    if (pCstring)
    {
		int len = strlen(pCstring);
		char *pVPString = (char *)malloc(len + 1);

		strcpy(pVPString, pCstring);
		pVPString[len + 1] = 0;
		
        glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                            len, pVPString);

		if (GL_NO_ERROR != glGetError())
		{
			fprintf(stderr, "Loading Program returned %s\n", (unsigned char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
			
			return NULL;
		}
    }
    
    // Generate fences for vertex array ranges
    glGenFencesAPPLE(2, fences);

    // Compute byteStride from one vertex to another for vertex arrays
    byteStride = sizeof(Vertex);
	
    glBindVertexArrayAPPLE(2);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, byteStride, &(*vn_ptr[1])[0][0].position.x);
    glVertexArrayRangeAPPLE(sizeof(*vn_ptr[1]), (*vn_ptr[1]));
    glTexCoordPointer (2, GL_FLOAT, byteStride, &(*vn_ptr[1])[0][0].texcoord.s);
    
    glBindVertexArrayAPPLE(1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, byteStride, &(*vn_ptr[0])[0][0].position.x);
    glVertexArrayRangeAPPLE(sizeof(*vn_ptr[0]), (*vn_ptr[0]));
    glTexCoordPointer (2, GL_FLOAT, byteStride, &(*vn_ptr[0])[0][0].texcoord.s);
	
	CGLDisable(CGLGetCurrentContext(), kCGLCESwapLimit);
    
    for (i = 0; i < MAXGRID - 1; i++)
    {
        k = 0;
        for (j = 0; j < MAXGRID; j++)
        {
            elements[i][k++] =     i * MAXGRID + j;
            elements[i][k++] = (i+1) * MAXGRID + j;
        }
    }
    
    for(k=0; k<2; k++)
    {
        for(i = 0; i < MAXGRID; i++)
        {
            GLfloat s = (GLfloat) (i * 3) / (GLfloat)(MAXGRID-1);
             
            for(j = 0; j < MAXGRID; j++)
            {
                (*vn_ptr[k])[i][j].position.x = (GLfloat) i;
                (*vn_ptr[k])[i][j].position.y = (GLfloat) j;
                (*vn_ptr[k])[i][j].position.z = 0.0;
                
                (*vn_ptr[k])[i][j].texcoord.s = s;
                (*vn_ptr[k])[i][j].texcoord.t = (GLfloat) (j * 3) / (GLfloat)(MAXGRID-1);
            }
        }
    }
    
    return self;
}

- (void)WaveSetDisplay:(GLint)level
{
	GLint i;
	
    opt_level = level;

    glFinish();
	
	for(i = 1; i <= 2; i++)
	{
		glBindVertexArrayAPPLE(i);
		
		switch(level)
		{
			case 0:
			case 1:
			case 2:
				glDisableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
				glVertexArrayRangeAPPLE(0, 0);
			break;
			case 3:
			case 4:
				glVertexArrayParameteriAPPLE(GL_VERTEX_ARRAY_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
				glVertexArrayRangeAPPLE(sizeof(*vn_ptr[i-1]), (*vn_ptr[i-1]));
				glEnableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
			break;
			
			default:
				glVertexArrayParameteriAPPLE(GL_VERTEX_ARRAY_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
				// Really slow
				//glVertexArrayRangeAPPLE(sizeof(*vn_ptr[0]), (*vn_ptr[0]));
				// Medium speed
				glVertexArrayRangeAPPLE(sizeof(*vn_ptr[i-1]), (*vn_ptr[i-1]));
				// Really fast
				//glVertexArrayRangeAPPLE(sizeof(*vn_ptr[1]), (*vn_ptr[1]));
				glEnableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
			break;
		}
	}
	
    switch(level)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            glDisable(GL_VERTEX_PROGRAM_ARB);
        break;
        
        default:
			glEnable(GL_VERTEX_PROGRAM_ARB);
        break;
    }
}

- (void) WaveMotion:(GLint) buffer
{
    GLint	i, j, k;
    GLfloat	dist;
    GLfloat	x, y, z;
    Vertex	(*vbuffer)[MAXGRID][MAXGRID];
    
	// bug in buffer swap code someplace, hack to fix here
	if (5 == opt_level)
	{
		buffer = (buffer) ? 0 : 1;
	}

    vbuffer      = (*vn_ptr[buffer]);

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

    switch(opt_level)
    {
		case 5:			
			glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, sim_time, 0, 0, 0);
        break;
        
        case 4:
        {
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

//#define GOOD_ENOUGH             
    
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
#ifdef GOOD_ENOUGH
				register vector float t, t0, t1, t2, t3;
#else
				register vector float t0, t1, t2;
				register vector float invFac2, invFac4, invFac6, invFac8, invFac10;
#endif
				register vector float y0;
				register vector float invDistVec;
#ifdef GOOD_ENOUGH
				register vector float tempVecModPi;
#else
				register vector float temp_zVec;
#endif
                register unsigned int ii = i * MAXGRID;
				vector float zVec;
				
#ifndef GOOD_ENOUGH
				// 3 fps... we just ran out of registers...
				invFac2		= invFac[2];
				invFac4		= invFac[4];
				invFac6		= invFac[6];
				invFac8		= invFac[8];
				invFac10	= invFac[10];
#endif
				
                for(j=0; j<MAXGRID; j+=4)
                {
					xVec 		= vec_ld(0, (vector float *)&xPos[ii + j]);
					yVec 		= vec_ld(0, (vector float *)&yPos[ii + j]);
#ifdef GOOD_ENOUGH
					zVec		= zero;
#else
                    temp_zVec 	= vec_sub(xVec, xVec);	// avoid a zero load
#endif
                    
                    for(k=0; k<3; k++)
                    {        
                        temp_xVec = vec_sub(emitterX[k], xVec);
                        temp_yVec = vec_sub(emitterY[k], yVec);
                        
						// dist = sqrt(x*x + y*y) * 0.1;
#ifdef GOOD_ENOUGH
                        distVec = vec_madd(temp_yVec, temp_yVec, vec_madd(temp_xVec, temp_xVec, zero));
                        
                        // single second order newton rhapson iteration
                        distVec = vec_max(epsilon, distVec);
#else
                        distVec = vec_max(epsilon, vec_madd(temp_yVec, temp_yVec, vec_madd(temp_xVec, temp_xVec, zero)));
#endif
                        y0 = vec_rsqrte(distVec);
                        t0 = vec_madd(y0, y0, zero);
                        t1 = vec_madd(y0, half, zero);
                        t2 = vec_madd(y0, pt375, zero);
                        t0 = vec_nmsub(distVec, t0, one);
#ifdef GOOD_ENOUGH
                        t3 = vec_madd(t2, t0, t1);
                        invDistVec = vec_madd(t0, t3, y0);
#else
                        invDistVec = vec_madd(t0, vec_madd(t2, t0, t1), y0);
#endif

                        // compute the distVec
                        invDistVec = vec_max(epsilon, invDistVec);
                        y0 = vec_re(invDistVec);
#ifdef GOOD_ENOUGH
                        t = vec_nmsub(y0, invDistVec, one);
                        distVec = vec_madd(y0, t, y0);

						// dist = sqrt(x*x + y*y) * 0.1;
                        distVec = vec_madd(distVec, ptOne, zero);
#else
                        distVec = vec_madd(y0, vec_nmsub(y0, invDistVec, one), y0);
                        distVec = vec_madd(distVec, ptOne, zero);
#endif

                                                
						// z += amp * cos(freq * dist + sim_time) / exp(decay / dist);
                        // compute freq * dist + sim_time
                        tempVec = vec_madd(emitterFreq[k], distVec, sim_timeVec);

                        // bound tempVec to 0 <-> 2pi
#ifdef GOOD_ENOUGH
                        tempVecModPi = vec_madd(tempVec, inv2Pi, zero);
                        tempVecModPi = vec_trunc(tempVecModPi);
                        tempVec = vec_nmsub(tempVecModPi, _2Pi, tempVec);
#else
						// force expression evaluator to make this all one expression
                        tempVec = vec_nmsub(vec_trunc(vec_madd(tempVec, inv2Pi, zero)), _2Pi, tempVec);
#endif
                        
                        // compute cos(freq * dist + sim_time)
                        // Cos = sum (-1)^n*x^2n/2n!
                        // compute sum of products for cos(freq * dist + sim_time)
#ifdef GOOD_ENOUGH
                        vecPow2	 = vec_madd( tempVec, tempVec, zero);		// x^2
                        cosVec   = vec_nmsub( vecPow2,  invFac[2], one);	// n = 1

                        vecPow4  = vec_madd( vecPow2, vecPow2, zero);		// x^4
                        cosVec   = vec_madd(  vecPow4,  invFac[4], cosVec);	// n = 2

                        vecPow6  = vec_madd( vecPow4, vecPow2, zero);		// x^6
                        cosVec   = vec_nmsub( vecPow6,  invFac[6], cosVec);	// n = 3

                        vecPow8  = vec_madd( vecPow6, vecPow2, zero);		// x^8
                        cosVec   = vec_madd(  vecPow8,  invFac[8], cosVec);	// n = 4

                        vecPow10 = vec_madd( vecPow8, vecPow2, zero);		// x^10
                        cosVec   = vec_nmsub(vecPow10, invFac[10], cosVec);	// n = 5

                        vecPow12 = vec_madd(vecPow10, vecPow2, zero);		// x^12
                        cosVec   = vec_madd( vecPow12, invFac[12], cosVec);	// n = 6

                        vecPow14 = vec_madd(vecPow12, vecPow2, zero);		// x^14
                        cosVec   = vec_nmsub(vecPow14, invFac[14], cosVec);	// n = 7

                        vecPow16 = vec_madd(vecPow14, vecPow2, zero);		// x^16
                        cosVec   = vec_madd( vecPow16, invFac[16], cosVec);	// n = 8
#else
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
#endif
                        // amp * cos(freq * dist + sim_time)
                        cosVec   = vec_madd(cosVec, emitterAmp[k], zero);
                        
                        // compute decay / dist
                        tempVec = vec_madd(emitterDecay[k], invDistVec, zero);
						
                        // power series expansion for exp(x)
#ifdef GOOD_ENOUGH
                        expVec  = vec_add( tempVec, one);				// n = 0, 1

                        vecPowN	= vec_madd(  tempVec, tempVec, zero);		// x^2
                        expVec  = vec_madd( vecPowN, invFac[2], expVec);	// n = 2
                        
                        for(n=3; n<6; n++)
                        {
                            vecPowN	= vec_madd( vecPowN, tempVec, zero);	// x^n
                            expVec  = vec_madd( vecPowN, invFac[n], expVec);
                        }

                        // compute the invExpVec
                        expVec = vec_min(infinite, expVec);
#else
						// this adds 5 fps maybe 8-10
                        vecPowN	= vec_madd( tempVec, tempVec, zero);		// x^2
                        expVec  = vec_madd( vecPowN, invFac[2], vec_add( tempVec, one));	// n = 2

						vecPowN	= vec_madd( vecPowN, tempVec, zero);		// x^3
						expVec  = vec_madd( vecPowN, invFac[3], expVec);	// n = 3

						vecPowN	= vec_madd( vecPowN, tempVec, zero);		// x^4
						expVec  = vec_madd( vecPowN, invFac[4], expVec);	// n = 4

						vecPowN	= vec_madd( vecPowN, tempVec, zero);		// x^5
                        expVec	= vec_min(infinite, vec_madd( vecPowN, invFac[5], expVec));
#endif
                        // compute the invExpVec
                        y0 = vec_re(expVec);
#ifdef GOOD_ENOUGH
                        t = vec_nmsub(y0, expVec, one);
                        expVec = vec_madd(y0, t, y0);

                        // accum onto z
                        zVec = vec_madd(cosVec, expVec, zVec);
#else
						// compute the rest of the invExpVec and accum onto z.... 2 fps...more
                        temp_zVec = vec_madd(cosVec, vec_madd(y0, vec_nmsub(y0, expVec, one), y0), temp_zVec);
#endif
                    }
                    
                    // dump vector into interleaved array
#ifndef GOOD_ENOUGH
					zVec = temp_zVec;		// single store vs. 3 load / stores
#endif
                    pTemp = (float *)&zVec;
                    (*vbuffer)[i][  j].position.z = pTemp[0];
                    (*vbuffer)[i][j+1].position.z = pTemp[1];
                    (*vbuffer)[i][j+2].position.z = pTemp[2];
                    (*vbuffer)[i][j+3].position.z = pTemp[3];
                }
            }
        }
        break;
    
        default:
        for(i=0; i<MAXGRID; i++)
        {
            for(j=0; j<MAXGRID; j++)
            {
                z = 0;
                
                for(k=0; k<3; k++)
                {        
                    GLfloat	amp, freq, decay;
                    
                    x = emitters[k].x - (*vbuffer)[i][j].position.x;
                    y = emitters[k].y - (*vbuffer)[i][j].position.y;
                    
                    dist = sqrt(x*x + y*y);
                    
                    amp   = emitters[k].amp;
                    freq  = emitters[k].freq;
                    decay = emitters[k].decay;
                    
                    z += amp * cos(freq * dist * 0.1 + sim_time) / exp(decay / dist);
                }
                
                (*vbuffer)[i][j].position.z = z;
            }
        }
        break;
    }
}

- (GLuint)WaveDisplay:(GLint) buffer
{
	GLint i, j;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	switch(opt_level)
	{
		case 0:
			if (fillmode)
				glBegin(GL_QUADS);
			else
				glBegin(GL_POINTS);				
			for(i = 0; i < (MAXGRID - 1); i++)
			{
				for(j = 0; j < (MAXGRID - 1); j++)
				{
					glTexCoord2fv(&(*vn_ptr[buffer])[i][j].texcoord.s );
					glVertex3fv(  &(*vn_ptr[buffer])[i][j].position.x );

					glTexCoord2fv(&(*vn_ptr[buffer])[i+1][j].texcoord.s );
					glVertex3fv(  &(*vn_ptr[buffer])[i+1][j].position.x );

					glTexCoord2fv(&(*vn_ptr[buffer])[i+1][j+1].texcoord.s );
					glVertex3fv(  &(*vn_ptr[buffer])[i+1][j+1].position.x );

					glTexCoord2fv(&(*vn_ptr[buffer])[i][j+1].texcoord.s );
					glVertex3fv(  &(*vn_ptr[buffer])[i][j+1].position.x );
				}
			}
			glEnd();
		break;
		
		case 1:
			for(i = 0; i < (MAXGRID - 1); i++)
			{
				if (fillmode)
					glBegin(GL_QUAD_STRIP);
				else
					glBegin(GL_POINTS);				
				for(j = 0; j < MAXGRID; j++)
				{
					glTexCoord2fv(&(*vn_ptr[buffer])[i][j].texcoord.s );
					glVertex3fv(  &(*vn_ptr[buffer])[i][j].position.x );
                                        
					glTexCoord2fv(&(*vn_ptr[buffer])[i+1][j].texcoord.s );
					glVertex3fv(  &(*vn_ptr[buffer])[i+1][j].position.x );
				}
				glEnd();
			}
		break;
		
		case 2:
			if (fillmode)
			{
				for(i = 0; i < (MAXGRID - 1); i++)
					glDrawElements(GL_QUAD_STRIP, 2 * MAXGRID, GL_UNSIGNED_SHORT, elements[i]);
			}
			else
			{
				for(i = 0; i < (MAXGRID - 1); i++)
					glDrawElements(GL_POINTS, 2 * MAXGRID, GL_UNSIGNED_SHORT, elements[i]);
			}
		break;
		case 3:
		case 4:
			glBindVertexArrayAPPLE(buffer + 1);
			
			glFlushVertexArrayRangeAPPLE(sizeof(*vn_ptr[0]), (*vn_ptr[buffer]));

			if (fillmode)
			{
				for(i = 0; i < (MAXGRID - 1); i++)
					glDrawElements(GL_QUAD_STRIP, 2 * MAXGRID, GL_UNSIGNED_SHORT, elements[i]);
			}
			else
			{
				for(i = 0; i < (MAXGRID - 1); i++)
					glDrawElements(GL_POINTS, 2 * MAXGRID, GL_UNSIGNED_SHORT, elements[i]);
			}
		break;
                
		default:
			glBindVertexArrayAPPLE(buffer + 1);
			
			if (fillmode)
			{
				for(i = 0; i < (MAXGRID - 1); i++)
					glDrawElements(GL_QUAD_STRIP, 2 * MAXGRID, GL_UNSIGNED_SHORT, elements[i]);
			}
			else
			{
				for(i = 0; i < (MAXGRID - 1); i++)
					glDrawElements(GL_POINTS, 2 * MAXGRID, GL_UNSIGNED_SHORT, elements[i]);
			}
		break;
	}
	
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

- (void)WaveSetFillMode:(GLint)mode
{
	fillmode = mode;

}

@end
