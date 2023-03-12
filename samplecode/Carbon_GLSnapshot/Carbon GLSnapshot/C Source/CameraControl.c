//////////////////////////////////////////////////////////////////////////////////
/*
	File:		CameraControl.c

	Project:	CarbonEvent Shell

	Contains:	Implementation for the camera controls functionality
	
	Author: 	Todd Previte
	
	Copyright:	2001 Apple Computer, Inc., All Rights Reserved

	Copyright:	(c) 2002 Apple Computer, Inc., All Rights Reserved
	
	Disclaimer:	

	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
	("Apple") in consideration of your agreement to the following terms, and your
	use, installation, modification or redistribution of this Apple software
	constitutes acceptance of these terms.  If you do not agree with these terms,
	please do not use, install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and subject
	to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
//////////////////////////////////////////////////////////////////////////////////

#include "CameraControl.h"

// Camera Controls

void cameraInitialize(GLCamera *theCam, WindowPtr winPtr)
{
    Rect winPort;
    GLdouble viewparms[] = {-0.5, 0.5, 		// Left Right
			    -0.5, 0.5, 		// Bottom Top
			    0.5, 100.0};	// zNear zFar
    GLdouble *viewVol = theCam->viewVolume;
    
    GetWindowBounds(winPtr, kWindowContentRgn, &winPort);
    theCam->w = winPort.right - winPort.left;
    theCam->h = winPort.bottom - winPort.top;

    theCam->aspectRatio = theCam->h / theCam->w;
    theCam->fov = 3.141517/2; // pi/2
    theCam->hFrac = tan(theCam->fov * 0.5);
    theCam->vFrac = tan(theCam->fov * 0.5 * theCam->aspectRatio);

    glViewport(0, 0, (GLsizei) theCam->w, (GLsizei) theCam->h);

    cameraSetViewParameters(theCam, viewparms);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    cameraMove(theCam, 0.0, 0.0, -4.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#if 1
//USE_PERSPECTIVE
   glFrustum(viewVol[0], viewVol[1], viewVol[2], viewVol[3], viewVol[4], viewVol[5]);
#else
    glOrtho(viewVol[0], viewVol[1], viewVol[2], viewVol[3], viewVol[4], viewVol[5]);
#endif
    glMatrixMode(GL_MODELVIEW);
}

void cameraSetPosition(GLCamera *theCam, double newX, double newY, double newZ)
{
    theCam->camPosition.x += newX;
    theCam->camPosition.y += newY;
    theCam->camPosition.z += newZ;
}

void cameraSetViewParameters(GLCamera *theCam, GLdouble* newView)
{
    int i = 0;
    for(i = 0; i< 6;i++)
	theCam->viewVolume[i] = newView[i];
}

void cameraViewTarget(GLCamera *theCam, Point3D target)
{
#pragma unused (theCam, target)

}

/*
void cameraRotate(GLCamera *theCam, double angle, double x, double y, double z)
{
    glMatrixMode(GL_PROJECTION);
    //glMatrixMode(GL_PROJECTION);
    //glRotated(angle, camPosition.x, camPosition.y, camPosition.z);
    if(x != 0)
	glRotated(angle, 0.0, 1.0, 0.0);
    if(y != 0)
	glRotated(angle, 1.0, 0.0, 0.0);
    // glRotated(angle, x, y, z);
   //  AngleFromOrigin(x, y, z);
   glMatrixMode(GL_MODELVIEW);
}
*/

void cameraRotate(GLCamera *theCam, short dV, short dH)
{
    // So we've got the h and v change of the mouse.
    // We need to apply that to this rotation
    glMatrixMode(GL_MODELVIEW);

    glRotated(1.0 * dV, 1.0, 0.0, 0.0);
    glRotated(1.0 * dH, 0.0, 1.0, 0.0);
    //if(dV != 0)
	//glRotated(1.0 * dV, theCam->camPosition.x, 0.0, 0.0);
    //if(dH != 0)
	//glRotated(1.0 * dH, 0.0, theCam->camPosition.x, 0.0);

    glMatrixMode(GL_MODELVIEW);
}

void cameraOrbit(GLCamera *theCam, Point3D target)
{

}

void cameraMove(GLCamera *theCam, double x, double y, double z)
{
    glMatrixMode(GL_MODELVIEW);
    glTranslated(x, y, z);
    cameraSetPosition(theCam, x, y, z);
}

double cameraAngleFromOrigin(GLCamera *theCam)
{
    theCam->camAngle = atan(theCam->camPosition.y/theCam->camPosition.x);
    theCam->fov = atan(((theCam->viewVolume[1] - theCam->viewVolume[0])/2)/theCam->viewVolume[4]);
    return theCam->camAngle;
}

double cameraViewAngleFromPoint(GLCamera *theCam, Point3D target)
{
    // FIXME
    return 0.0;
}
