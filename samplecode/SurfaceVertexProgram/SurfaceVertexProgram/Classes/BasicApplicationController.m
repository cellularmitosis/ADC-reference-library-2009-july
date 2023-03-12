/*
 *  BasicApplicationController.m
 *
 *  Created by Michael Larson on Tue Mar 11 2003.
 *  Copyright (c) 2003 Apple Computer. All rights reserved.
 *
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
   ("Apple") in consideration of your agreement to the following terms, and your
   use, installation, modification or redistribution of this Apple software
   constitutes acceptance of these terms.  If you do not agree with these terms,
   please do not use, install, modify or redistribute this Apple software.

   In consideration of your agreement to abide by the following terms, and subject
   to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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

#import "BasicApplicationController.h"

typedef const OSType *		QTFrameTypeListPtr;
typedef const OSTypePtr 	TypeListPtr;

const OSType    		kApplicationSignature  	= FOUR_CHAR_CODE('oglv');
const ResType   		kOpenResourceType 	= FOUR_CHAR_CODE('open');
const StringPtr 		kApplicationName 	= "\pSurface Vertex Program";

@implementation 		BasicApplicationController

- (void) awakeFromNib
{
    // setup the timer
    m_timer = [NSTimer scheduledTimerWithTimeInterval: 0.01 target: self 
                selector:@selector(timerEventHandler) userInfo: 0 repeats: YES];

    // Init the rotation to zero
    m_rotx = 15;
    m_roty = 30;
    m_rotz = 15;

    m_scale = 1.0;
    m_aa = m_bb = m_cc = 1.0;
    m_dd = 0.0;

	m_energy = 1.0;
	
    m_SndPlayer = [SoundPlayer new];
    
    LissajousSurface = [self CreateLissajousSurface];
    
    [LissajousSurface attachToVertexArrayRange];
    
    m_AnalyzerBars	= NULL;
    m_VolumeLines	= NULL;
}

Handle CreateOpenHandle (OSType theApplicationSignature, short theNumTypes, TypeListPtr theTypeList)
{
    Handle   myHandle = NULL;
    
    // see if we have an 'open' resource...
    myHandle = Get1Resource('open', 128);
    if ( myHandle != NULL && ResError() == noErr ) {
        DetachResource( myHandle );
        return myHandle;
    } else {
        myHandle = NULL;
    }
    
    // nope, use the passed in types and dynamically create the NavTypeList
    if (theTypeList == NULL)
        return myHandle;
    
    if (theNumTypes > 0) {
        myHandle = NewHandle(sizeof(NavTypeList) + (theNumTypes * sizeof(OSType)));
        
        if (myHandle != NULL) {
            NavTypeListHandle  myOpenResHandle = (NavTypeListHandle)myHandle;
        
            (*myOpenResHandle)->componentSignature = theApplicationSignature;
            (*myOpenResHandle)->osTypeCount = theNumTypes;
            BlockMoveData(theTypeList, (*myOpenResHandle)->osType, theNumTypes * sizeof(OSType));
        }
    }
    
    return myHandle;
}

pascal void HandleNavEvent(NavEventCallbackMessage theCallBackSelector, NavCBRecPtr theCallBackParms, void *theCallBackUD)
{
#pragma unused(theCallBackUD)	
    if (theCallBackSelector == kNavCBEvent) {
            switch (theCallBackParms->eventData.eventDataParms.event->what) {
                    case updateEvt:
                        // Handle Update Event
                        break;
                    case nullEvent:
                        // Handle Null Event
                        break;
            }
    }
}

static OSErr GetOneFileWithPreview (short theNumTypes, TypeListPtr theTypeList, FSSpecPtr theFSSpecPtr, void *theFilterProc)
{
 
    NavReplyRecord  	myReply;
    NavDialogOptions 	myDialogOptions;
    NavTypeListHandle	myOpenList = NULL;
    NavEventUPP  	myEventUPP = NewNavEventUPP(HandleNavEvent);
    OSErr    		myErr = noErr;
    
    if (theFSSpecPtr == NULL)
        return(paramErr);
    
    // specify the options for the dialog box
    NavGetDefaultDialogOptions(&myDialogOptions);
    myDialogOptions.dialogOptionFlags -= kNavNoTypePopup;
    myDialogOptions.dialogOptionFlags -= kNavAllowMultipleFiles;
    myDialogOptions.dialogOptionFlags += kNavAllowPreviews;
    
    // create a handle to an 'open' resource
    myOpenList = (NavTypeListHandle)CreateOpenHandle(kApplicationSignature, theNumTypes, theTypeList);
    if (myOpenList != NULL)
        HLock((Handle)myOpenList);
    
    // prompt the user for a file
    if (NULL == theTypeList)
    {
        myErr = NavGetFile(NULL, &myReply, &myDialogOptions, NULL, NULL, NULL, NULL, NULL);
    }
    else
    {
        myErr = NavGetFile(NULL, &myReply, &myDialogOptions, myEventUPP, NULL, (NavObjectFilterUPP)theFilterProc, myOpenList, NULL);
    }
    
    if ((myErr == noErr) && myReply.validRecord)
    {
        AEKeyword  myKeyword;
        DescType  myActualType;
        Size   myActualSize = 0;
        
        // get the FSSpec for the selected file
        if (theFSSpecPtr != NULL)
        myErr = AEGetNthPtr(&(myReply.selection), 1, typeFSS, &myKeyword, &myActualType, theFSSpecPtr, sizeof(FSSpec), &myActualSize);
    
        NavDisposeReply(&myReply);
    }
 
    if (myOpenList != NULL)
    {
        HUnlock((Handle)myOpenList);
        DisposeHandle((Handle)myOpenList);
    }
    
    DisposeNavEventUPP(myEventUPP);
 
    return(myErr);
}

- (void) CloseSoundFile
{
}

- (OSErr) OpenSoundFile
{
    OSErr		myErr = noErr;
    
    // elicit a new sound file from the user
    myErr = GetOneFileWithPreview(1, NULL, &m_SndFileFSSpec, NULL);
    
    return(myErr);
}

- (void) menuFileOpen: (id) sender
{
    if(noErr == [self OpenSoundFile])
    {
        [m_SndPlayer PlaySound: &m_SndFileFSSpec];
    }
}

- (IBAction) sliderUpdateParams: (id) sender
{
    NSSlider	*slider;
    GLfloat	fValue;
    
    slider = sender;
    
    fValue = [slider floatValue];
    
    [slider setContinuous: YES];
    
    switch([slider tag])
    {
        case 0:	m_aa = fValue; break;
        case 1:	m_bb = fValue; break;
        case 2:	m_cc = fValue; break;
        case 3:	m_dd = fValue; break;
    }
    
    [self timerEventHandler];
}

- (VariableFormatVertex	*) CreateLissajousSurface
{
    VariableFormatVertex	*Surface = [VariableFormatVertex new];
    GLint			nVerticesRequired;
    GLint			vertexIndex;
    GLfloat			u, v, ustep, vstep;
    GLfloat			s, t, sstep, tstep;
    GLint			rows, cols;
    GLfloat			pi = 3.14159265359;
    
    vstep = ustep = pi / 40.0;
    sstep = tstep = 1.0 / 40.0;
    
    if(Surface)
    {    
        // Figure out how many vertices are required to complete the surface (uv)
        for(nVerticesRequired=0, rows=0, u=-pi;u<=pi+ustep; u+=ustep, rows++)
        {
            for(v=-pi, cols=0;v<=pi+vstep; v+=vstep, nVerticesRequired++, cols++)
            {
            }
        }
    
        [Surface initWithVertexType: kAttributeTexture0 vertexArrayOfCount: nVerticesRequired];

        [Surface setMeshRows: rows Cols: cols];
        
        // fill in the vertices
        for(vertexIndex=0, u=-pi, s=0;u<=pi+ustep; u+=ustep, s+=sstep)
        {
            for(v=-pi, t=0;v<=pi+vstep; v+=vstep, t+=tstep, vertexIndex++)
            {
                GLfloat	*pVertex = [Surface atVertex: vertexIndex];
				GLfloat *pST = [Surface Texture0: pVertex];
				
                pVertex[0] = u;
                pVertex[1] = v;
                pVertex[2] = 0;                
                pVertex[3] = 0;
				
				pST[0] = s;
				pST[1] = t;
            }
        }

        // Enable VertexArray Ranges
        [Surface attachToVertexArrayRange];
    }
    
    return Surface;
}

- (void) UpdateSurface: (UInt8 *) params
{
    GLint			i;
    GLfloat			fParams[8];
    GLfloat			energy = 1;
    struct timeval 	tp;
	struct timezone tzp;
	
    if(LissajousSurface)
    {
        if (params)
        {
            for(i=0; i<8; i++)
                fParams[i] = (float)params[i] / 256.0;

            for(energy=0,i=0; i<8; i++)
                energy += fParams[i];
                
            energy /= 8.0;
			
            m_energy = m_energy * 0.95 + energy * 0.1;
            
            gettimeofday(&tp, &tzp);
            fParams[0] = (float)tp.tv_usec / 1E6;
            fParams[1] = m_energy;
			
            [m_BasicOpenGLView loadEnvParameter: &fParams[0] atIndex: 1];
            [m_BasicOpenGLView loadEnvParameter: &fParams[4] atIndex: 2];
    
            for(i=0; i<8; i++)
                fParams[i] = (float)params[i] / 256.0;

            for(energy=0,i=0; i<8; i++)
                energy += fParams[i];
                
            energy /= 8.0;
            			
            m_aa = m_aa * 0.9 + ((fParams[0] + fParams[1]) / 2) * 0.1;
            m_bb = m_bb * 0.9 + ((fParams[2] + fParams[3]) / 2) * 0.1;
            m_cc = m_cc * 0.9 + ((fParams[4] + fParams[5]) / 2 + 1.0) * 0.1;
            m_dd = m_dd * 0.9 + ((fParams[6] + fParams[7]) / 2 * 2.0) * 0.1;
        }

        fParams[0] = m_aa;
        fParams[1] = m_bb;
        fParams[2] = m_cc;
        fParams[3] = m_dd;

        [m_BasicOpenGLView loadEnvParameter: fParams atIndex: 0];
    }
}

- (void) timerEventHandler
{
    UInt8	i, soundLevels[8];
    GLfloat	fSoundLevels[8];

    if ([m_SndPlayer IsMusicPlaying])
    {
            [m_SndPlayer GetSoundEqualizerBandLevels: soundLevels];
    }
	
    if (m_VolumeOpenGLView)
    {
        if (NULL == m_VolumeLines)
        {
            GLint	i;

            m_VolumeLines = [VariableFormatVertex new];
            
            // Alloc enough for 100 line segments
            [m_VolumeLines initWithVertexType: kAttributeColor vertexArrayOfCount: 50];

            for(i=0; i<50; i++)
            {
                GLfloat	*pVertex = [m_VolumeLines atVertex: i];

                pVertex[0] = 0.0;
                pVertex[1] = 0.0;
                pVertex[2] = 1.0;
                pVertex[3] = 1.0;
                
                pVertex = [m_VolumeLines Color: pVertex];

                pVertex[0] = 0.2;
                pVertex[1] = 0.0;
                pVertex[2] = 1.0;
            }
        }

        [m_VolumeOpenGLView frameBegin];

        if ([m_SndPlayer IsMusicPlaying])
        {
            GLfloat	energy;
            GLfloat	linePos[25];
            
            for(i=0; i<25; i++)
                linePos[i] = (float)i / 25.0;

            for(i=0; i<8; i++)
                fSoundLevels[i] = (float)soundLevels[i] / 256.0;

            for(energy=0, i=0; i<8; i++)
                energy += fSoundLevels[i];
            
            energy /= 16.0;
            
            for(i=49; i >= 2; i--)
            {                
                GLfloat	*pSrcVertex = [m_VolumeLines atVertex: i-2];
                GLfloat	*pDstVertex = [m_VolumeLines atVertex: i];
                
                pDstVertex[0] = linePos[i / 2 + 1];
                pDstVertex[1] = pSrcVertex[1];
            }
            
            {                
                GLfloat	*pVertex = [m_VolumeLines atVertex: 0];
                
                pVertex[1] = 0.5 - energy;

                pVertex = [m_VolumeLines atVertex: 1];
                
                pVertex[1] = 0.5 + energy;
            }
            
            [m_VolumeOpenGLView drawVertexArray: m_VolumeLines ofType: GL_LINES];
        }
        
        [m_VolumeOpenGLView frameEnd];
    }
    
    if (m_AnalyzerOpenGLView)
    {
        if (NULL == m_AnalyzerBars)
        {
            GLint	i;
            GLfloat	eightColors[][3] = {{0.5, 0.0, 0.0}, {1.0, 0.0, 0.0},
                                            {1.0, 0.5, 0.0}, {0.0, 1.0, 0.0},                                             
                                            {0.0, 0.5, 0.0}, {0.0, 1.0, 0.0}, 
                                            {0.0, 0.0, 0.5}, {0.0, 0.0, 1.0}};

            m_AnalyzerBars = [VariableFormatVertex new];
            
            // Alloc enough for 8 quads
            [m_AnalyzerBars initWithVertexType: kAttributeColor vertexArrayOfCount: 32];

            for(i=0; i<32; i++)
            {
                GLint barNumber	= i / 4;
                GLfloat	*pVertex = [m_AnalyzerBars atVertex: i];

                pVertex[0] = 0.0;
                pVertex[1] = 0.0;
                pVertex[2] = 1.0;
                pVertex[3] = 1.0;
                
                pVertex = [m_AnalyzerBars Color: pVertex];

                pVertex[0] = eightColors[barNumber][0];
                pVertex[1] = eightColors[barNumber][1];
                pVertex[2] = eightColors[barNumber][2];
            }
        }

        if ([m_SndPlayer IsMusicPlaying])
        {
			[m_AnalyzerOpenGLView frameBegin];
	
            for(i=0; i<8; i++)
                fSoundLevels[i] = (float)soundLevels[i] / 256.0;
            
            for(i=0; i<32; i++)
            {
                GLint corner 	= i & 0x3;
                GLint barNumber	= i / 4;
                
                GLfloat	*pVertex = [m_AnalyzerBars atVertex: i];

                switch(corner)
                {
                    case 0:
                        pVertex[0] = barNumber * 0.125;
                        pVertex[1] = 0;
                        pVertex[2] = 0;
                        break;
                        
                    case 1:
                        pVertex[0] = barNumber * 0.125;
                        pVertex[1] = fSoundLevels[barNumber];
                        pVertex[2] = 0;
                        break;
                        
                    case 2:
                        pVertex[0] = barNumber * 0.125 + 0.11;
                        pVertex[1] = fSoundLevels[barNumber];
                        pVertex[2] = 0;
                        break;
                        
                    case 3:
                        pVertex[0] = barNumber * 0.125 + 0.11;
                        pVertex[1] = 0;
                        pVertex[2] = 0;
                        break;
                }
            }

            [m_AnalyzerOpenGLView drawVertexArray: m_AnalyzerBars ofType: GL_QUADS];

			[m_AnalyzerOpenGLView frameEnd];
        }
    }

    if (m_BasicOpenGLView)
    {
        int	i;
        float   scale;

        [m_BasicOpenGLView frameBegin];

        if ([m_SndPlayer IsMusicPlaying])
        {
            [self UpdateSurface: soundLevels];
    
            for(scale=0, i=0; i<8; i++)
                scale += soundLevels[i];
        
            scale = scale / 1024;
            
            m_scale = m_scale * 0.9 + scale * 0.1;
        }
        else
        {
            [self UpdateSurface: NULL];
        }
        
        [m_BasicOpenGLView enableVertexPrograms];

        if ([m_BasicOpenGLView wireframeMode])
            [m_BasicOpenGLView drawVertexArray: LissajousSurface ofType: GL_LINE_STRIP];
        else
            [m_BasicOpenGLView drawVertexArray: LissajousSurface ofType: GL_QUAD_STRIP];            
        
        [m_BasicOpenGLView disableVertexPrograms];

        [m_BasicOpenGLView frameEnd];
    }
}

@end
