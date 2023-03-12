/*

File: Scene.m

Abstract: Scene of Objects that will we sources and the listener of sound.

Version: <1.0>

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/
#import "Scene.h"
#include <math.h>
#define DEG2RAD(x) (0.0174532925 * (x))
#define RAD2DEG(x) (57.295779578 * (x))

#define	kSquareSize				500		// needs to be the size of the custom NIB object
#define kSourceCircleRadius		10.0
#define kListenerCircleRadius	20.0
#define kDefaultDistance		175.0
#define NUM_BUFFERS_SOURCES		4		// this test app has 4 Source Objects and 4 Buffer Objects

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Globals
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float gListenerPos[3] = {0.0,  0.0,  0.0};		// default position is centered
float gListenerDirection = 0;
float gSourcePos[NUM_BUFFERS_SOURCES][3]  = {	{kDefaultDistance, 0.0, -kDefaultDistance},
												{kDefaultDistance, 0.0, kDefaultDistance},
												{-kDefaultDistance , 0.0, -kDefaultDistance},
												{-kDefaultDistance , 0.0, kDefaultDistance}		}; 

char *	gSourceFile[NUM_BUFFERS_SOURCES];
ALuint	gBuffer[NUM_BUFFERS_SOURCES];
ALuint	gSource[NUM_BUFFERS_SOURCES];

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize OpenAL -Get an Audio Device and Set Current OpenAL Context
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void InitializeOpenAL()
{
	ALenum			error;
	ALCcontext		*newContext = NULL;
	ALCdevice		*newDevice = NULL;

	// Create a new OpenAL Device
	// Pass NULL to specify the system‚Äôs default output device
	newDevice = alcOpenDevice(NULL);
	if (newDevice != NULL)
	{
		// Create a new OpenAL Context
		// The new context will render to the OpenAL Device just created 
		newContext = alcCreateContext(newDevice, 0);
		if (newContext != NULL)
		{
			// Make the new context the Current OpenAL Context
			alcMakeContextCurrent(newContext);

			// Create some OpenAL Buffer Objects
			alGenBuffers(NUM_BUFFERS_SOURCES, gBuffer);
			if((error = alGetError()) != AL_NO_ERROR) {
				printf("Error Generating Buffers: ");
				exit(1);
			}

			// Create some OpenAL Source Objects
			alGenSources(NUM_BUFFERS_SOURCES, gSource);
			if(alGetError() != AL_NO_ERROR) 
			{
				printf("Error generating sources! \n");
				exit(1);
			}
		}
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void TeardownOpenAL()
{
    ALCcontext	*context = NULL;
    ALCdevice	*device = NULL;
	ALuint		returnedNames[NUM_BUFFERS_SOURCES];

	// Delete the Sources
    alDeleteSources(NUM_BUFFERS_SOURCES, returnedNames);
	// Delete the Buffers
    alDeleteBuffers(NUM_BUFFERS_SOURCES, returnedNames);
	
	//Get active context
    context = alcGetCurrentContext();
    //Get device for active context
    device = alcGetContextsDevice(context);
    //Release context
    alcDestroyContext(context);
    //Close device
    alcCloseDevice(device);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	InitializeBuffers() 
{
	ALenum  error = AL_NO_ERROR;
	ALenum  format;
	ALvoid* data;
	ALsizei size;
	ALsizei freq;
	UInt32	i;
	
	for (i = 0; i < NUM_BUFFERS_SOURCES; i ++)
	{	
		//Get the current path to the audio file (which is contained in the application bundle) 
		const char *fullPathToFile =[[[NSBundle mainBundle] pathForResource:[NSString stringWithCString:gSourceFile[i]] ofType:@"wav"]UTF8String];
		// get some audio data from a wave file
		alutLoadWAVFile(fullPathToFile, &format, &data, &size, &freq);
		if((error = alGetError()) != AL_NO_ERROR) {
			printf("error loading %s: ", gSourceFile[i]);
			exit(1);
		}
		
		// Attach Audio Data to OpenAL Buffer
		alBufferData(gBuffer[i], format, data, size, freq);
		
		// Release the audio data
		alutUnloadWAV(format, data, size, freq);
		
		if((error = alGetError()) != AL_NO_ERROR) {
			printf("error unloading %s: ", gSourceFile[i]);
		}	
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void InitializeSourcesAndPlay() 
{
	UInt32 i;
	ALenum error = AL_NO_ERROR;
	alGetError(); // Clear the error
    
	for (i = 0; i < NUM_BUFFERS_SOURCES; i++)
	{
		// attach OpenAL Buffer to OpenAL Source
		alSourcei(gSource[i], AL_BUFFER, gBuffer[i]);
		// Turn Looping ON
		alSourcei(gSource[i], AL_LOOPING, AL_TRUE);
		// Set Source Position
		alSourcefv(gSource[i], AL_POSITION, gSourcePos[i]);
		// Set Source Reference Distance
		alSourcef(gSource[i],AL_REFERENCE_DISTANCE, 5.0f);
		// Start Playing Sound
		alSourcePlay(gSource[i]);
	}
			
	if((error = alGetError()) != AL_NO_ERROR) {
		printf("Error attaching buffer to source");
		exit(1);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SCENE class
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation Scene

- (id) init
{
	if((self = [super init])) {
		//4 WAV files in the application bundle resources
		gSourceFile[0] = "sound_engine";
		gSourceFile[1] = "sound_monkey";
		gSourceFile[2] = "sound_bubbles";
		gSourceFile[3] = "sound_electric";
	
		mCurrentObject = -1;
		mCenterOffset = kSquareSize/2.0;
		
		[self initOpenAL];
		[self setListenerOrientation:0];
		[self setListenerGain:0.2];
	}
	return self;
}

//draws a simple filled circle using quick glu calls
static void drawCircle(GLdouble x, GLdouble y, GLdouble r, GLfloat red, GLfloat grn, GLfloat blu){
	glColor3f (red, grn, blu);
	GLUquadricObj* pix;
	pix = gluNewQuadric();
	gluQuadricDrawStyle(pix, GL_FILL);
	gluQuadricNormals(pix, GLU_SMOOTH);
	
	glPushMatrix();
	glTranslatef(x, y, (GLdouble) 0.0);
	gluDisk(pix, 0.0, r, 20, 1);
	glPopMatrix();
	
	gluDeleteQuadric(pix);
}

- (void) drawListener {
	
	//draw a triangle
	glPushMatrix();
		glTranslatef(gListenerPos[0] + mCenterOffset, -gListenerPos[2] + mCenterOffset, 0.0);
		//Rotate to show the listeners current orientation
		glRotatef(gListenerDirection, 0.0, 0.0, -1.0);
		glBegin(GL_TRIANGLES);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(-6.0, 0.0, 0.0);
			glVertex3f(6.0, 0.0, 0.0);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(0.0, 35.0, 0.0);
		glEnd();
	glPopMatrix();
	
	//minor adjustments to position circle
	drawCircle(gListenerPos[0]- .5 + mCenterOffset, -gListenerPos[2] - 1 + mCenterOffset, 20, 0 , 0, 1);
}

- (void) drawSources
{
	//draw sources as 4 red circles
	drawCircle(gSourcePos[0][0] + mCenterOffset, -gSourcePos[0][2] + mCenterOffset, kSourceCircleRadius, 1 , 0, 0);
	drawCircle(gSourcePos[1][0] + mCenterOffset, -gSourcePos[1][2] + mCenterOffset, kSourceCircleRadius, 1 , 0, 0);
	drawCircle(gSourcePos[2][0] + mCenterOffset, -gSourcePos[2][2] + mCenterOffset, kSourceCircleRadius, 1 , 0, 0);
	drawCircle(gSourcePos[3][0] + mCenterOffset, -gSourcePos[3][2] + mCenterOffset, kSourceCircleRadius, 1 , 0, 0);
}

-(void) resetCurrentObject
{
	//set current object to -1 to designate no object selected.
	mCurrentObject = -1;
}

-(int) selectCurrentObject:(NSPoint *)point
{
	//find the object (circle) that contains the point
	//return -1 if none are found
	
	if(mCurrentObject != -1)
	  return mCurrentObject;
	  
	if([self pointInCircle:point x:gSourcePos[0][0] + mCenterOffset y:-gSourcePos[0][2] + mCenterOffset r:kSourceCircleRadius]){
		mCurrentObject = 0;
	}else if	([self pointInCircle:point x:gSourcePos[1][0] + mCenterOffset y:-gSourcePos[1][2] + mCenterOffset r:kSourceCircleRadius])
	    mCurrentObject = 1;
	else if	([self pointInCircle:point x:gSourcePos[2][0] + mCenterOffset y:-gSourcePos[2][2] + mCenterOffset r:kSourceCircleRadius])
	    mCurrentObject = 2;
	else if	([self pointInCircle:point x:gSourcePos[3][0] + mCenterOffset y:-gSourcePos[3][2] + mCenterOffset r:kSourceCircleRadius])
	    mCurrentObject = 3;	
	else if	([self pointInCircle:point x:gListenerPos[0] + mCenterOffset y:-gListenerPos[2] + mCenterOffset r:kListenerCircleRadius])
	    mCurrentObject = 4;
				
	return mCurrentObject;
}

// need to detect if point is in a source circle
// if so, return the source pos ID
// if not, return -1
- (bool) pointInCircle:(NSPoint *)point x:(float)x  y:(float)y  r:(float)r
{
   float x1 =point->x;
   float y1 = point->y;

   float dist  =0;
   //calculate distance from the center of the circle to the point clcked
   dist =(((x1 - x )*(x1 - x ))   +   ((y1 - y)*(y1 - y)));
	//Here we can test this for each of the sources
	
	//if the distance is less than the radius (squared), then the point is inside the circle
	if (dist <= r*r)
	  return TRUE;
	else
	 return FALSE;	
}

-(void) setObjectPosition:(NSPoint *)point
{	
	if(mCurrentObject < 4){
		[self setSourcePosition:point];
	} 
	else if (mCurrentObject == 4 ) //listener
	{
		[self setListenerPosition:point];
	}
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize OpenAL Context, Buffers, Listener & Sources
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void) initOpenAL {
		
	InitializeOpenAL();
	atexit(TeardownOpenAL);
	alGetError();
	
	InitializeBuffers();
	InitializeSourcesAndPlay();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Orientation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void) setListenerOrientation: (float) angle
{
	ALenum  error = AL_NO_ERROR;
	float	rads = DEG2RAD(angle);
	float	orientation[6] = {	0.0, 0.0, -1.0,    // direction
								0.0, 1.0, 0.0	}; //up	
								 
	orientation[0] = cos(rads);
	orientation[1] = sin(rads);		// No Change to the Z vector
	gListenerDirection = RAD2DEG(atan2(orientation[1], orientation[0]));
	
	// Change OpenAL Listener's Orientation
	orientation[0] = sin(rads);
	orientation[1] = 0.0;			// No Change to the Y vector
	orientation[2] = -cos(rads);	

	alListenerfv(AL_ORIENTATION, orientation);
	if((error = alGetError()) != AL_NO_ERROR)
		printf("Error Setting Listener Orientation");
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)setSourcePosition:(NSPoint *)point
{
	gSourcePos[mCurrentObject][0] = point->x - mCenterOffset;
	gSourcePos[mCurrentObject][1] = 0;
	gSourcePos[mCurrentObject][2] = -point->y + mCenterOffset;	// top view only in this demo!
	
	alSourcefv(gSource[mCurrentObject], AL_POSITION, gSourcePos[mCurrentObject]);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)setListenerPosition:(NSPoint *)point
{
	gListenerPos[0] = point->x - mCenterOffset;
	gListenerPos[1] = 0;
	gListenerPos[2] = -point->y + mCenterOffset;				// top view only in this demo!
		
	alListenerfv(AL_POSITION, gListenerPos);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Pitch
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void) setSourcePitch:(int)inTag :(float)inPitch;
{
	alSourcef(gSource[inTag], AL_PITCH, inPitch);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Source Gain
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void) setSourceGain:(int)inTag :(float)inGain;
{
	alSourcef(gSource[inTag], AL_GAIN, inGain);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Listener Gain
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void) setListenerGain:(float)inGain;
{
	alListenerf(AL_GAIN, inGain);
}

@end