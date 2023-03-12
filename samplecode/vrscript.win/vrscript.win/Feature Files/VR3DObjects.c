//////////
//
//	File:		VR3DObjects.c
//
//	Contains:	QuickDraw 3D support for QuickTime VR movies.
//
//	Written by:	Tim Monroe
//				Parts modeled on BoxMoov code by Nick Thompson, Rick Evans, and Robert Dierkes
//				and on code in QD3D book (by yours truly!).
//
//	Copyright:	© 1996-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <37>	 	03/28/00	rtm		changed ((**myPixMap).rowBytes & 0x3fff) to QTGetPixMapHandleRowBytes(myPixMap)
//	   <36>	 	07/29/99	rtm		set polyhedronAttributeSet to NULL in VR3DObjects_EnlistRectangle; now
//									the Set3DObjColor command works on rectangles as well as other objects
//	   <35>	 	09/28/98	rtm		added Q3InteractiveRenderer_ calls to VR3DObjects_CreateView
//	   <34>	 	04/22/98	rtm		used GetGWorldPixMap instead of gw->portPixMap in VR3DObjects_PrescreenRoutine
//	   <33>	 	03/18/98	rtm		minor cleanup to make things a little more structured
//	   <32>	 	03/09/98	rtm		removed VR3DObjects_GetEmbeddedPicture; moved code into VR3DTexture.c from
//									VR3DObjects_LoadEmbeddedMovie and VR3DObjects_LoopEmbeddedMovie
//	   <31>	 	02/19/98	rtm		changed default chroma key color to prevent transparency problems
//	   <30>	 	01/28/98	rtm		changed NewGWorld calls to QTNewGWorld
//	   <29>	 	01/27/98	rtm		fixed pixel depth problems: QD3D's GWorlds should all be 32-bit
//	   <28>	 	10/13/97	rtm		added VR3DObjects_CreateDefaultGroup call to VR3DObjects_GetModelFromFile
//									to provide default lighting for 3DMF files
//	   <27>	 	10/03/97	rtm		removed UPDATE_GWORLD_BROKEN symbol
//	   <26>	 	09/23/97	rtm		added endian adjustment to VR3DObjects_GetEmbeddedPicture
//	   <25>	 	09/15/97	rtm		added UPDATE_GWORLD_BROKEN symbol; should be removed later
//									when UpdateGWorld is fixed
//	   <24>	 	09/12/97	rtm		added Windows support to VR3DObjects_GetModelFromFile
//	   <23>	 	07/17/97	rtm		added VR3DObjects_DoIdle
//	   <22>	 	05/05/97	rtm		more work on geometry specifications;
//									added Set calls for styles, colors, etc.
//	   <21>	 	05/04/97	rtm		reworked geometry specifications; added Enlist_ functions
//	   <20>	 	05/02/97	rtm		made sound in texture movies directional
//	   <19>	 	04/10/97	rtm		added call to Q3View_Sync to prevent tearing
//	   <18>	 	04/02/97	rtm		copied file for integration with VRScript; removed support for object nodes
//	   <17>	 	02/21/97	rtm		added support for multi-node, mixed-type movies;
//									fixed window resizing for object nodes
//	   <16>	 	02/10/97	rtm		fixed up-vector; fixed pan/tilt granularity in object nodes
//									(now QD3D camera doesn't update until object view changes)
//	   <15>	 	02/04/97	rtm		begun implementing support for object nodes
//	   <14>	 	01/24/97	rtm		slight adjustment to camera-setting algorithm
//	   <13>	 	01/23/97	rtm		reworked camera-setting algorithm; now it works fine!
//									(replaced VR3DObjects_RotateCamera[XY] with VR3DObjects_SetCamera)
//	   <12>	 	01/20/97	rtm		various small tweaks; nothing major 
//	   <11>	 	01/17/97	rtm		further work on tilting; still unable to fix it entirely 
//	   <10>	 	01/16/97	rtm		added VR3DObjects_SetCameraAspectRatio 
//									and VR3DObjects_UpdateDrawContext to handle window resizing
//	   <9>	 	01/14/97	rtm		added polyhedron object to VR3DObjects_CreateModel; added static texture support
//	   <8>	 	01/13/97	rtm		added some object types to VR3DObjects_CreateModel
//	   <7>	 	01/10/97	rtm		added VR3DObjects_MCActionFilterProc (later merged into ApplicationMCActionFilterProc)
//	   <6>	 	01/09/97	rtm		fixed jittering on pan/tilt/zoom; fixed VR3DObjects_RotateCameraX for correct tilting
//	   <5>	 	01/03/97	rtm		added animation (that is, rotation) support for 3D objects
//	   <4>	 	12/23/96	rtm		further work on 3D overlay; fixed some jitters
//	   <3>	 	12/20/96	rtm		added zooming support; implemented 3D overlay instead of 3D test window
//	   <2>	 	12/16/96	rtm		added 3D test window, intercept procedure
//									added QuickTime movie texture mapping support
//	   <1>	 	12/13/96	rtm		first file
//	   
//
// This file provides functions to display 3D objects at specific locations in a panorama.
//
// Our strategy: QuickDraw 3D is a "slave" to VR: whenever the VR environment changes,
// we change the QuickDraw 3D camera accordingly and render a new image to our view.
// All this occurs in the prescreen buffer imaging complete procedure.
//
//////////

// TODO:
//	+ add lighting calls (create new light, move light, set light color/intensity etc.)

#if QD3D_AVAIL

//////////
//
// header files
//	   
//////////

#include "VR3DObjects.h"


//////////
//
// global variables
//	   
//////////

extern Boolean			gHasQuickDraw3D;							// is QuickDraw 3D available?
extern Boolean			gHasQuickDraw3D15;							// is QuickDraw 3D version 1.5 (or greater) available?


//////////
//
// constants
//	   
//////////

const TQ3Point3D		kCameraOrigin = {0.0, 0.0, 0.0};
const TQ3Point3D		kPointOfInterest = {0.0, 0.0, -1000.0};
const TQ3ColorRGB		k3DObjColor = {0.5, 0.5, 0.5};				// default 3D object color
const RGBColor			k3DChromaColor = {0x1111, 0x2222, 0x3333};	// the default chroma key color


//////////
//
// VR3DObjects_Init
// Initialize for QuickDraw 3D.
//
//////////

void VR3DObjects_Init (void)
{
	// make sure that QuickDraw 3D is available
	gHasQuickDraw3D = VR3DObjects_IsQD3DAvailable() ? true : false;
	
	// now perform any initialization
	if (gHasQuickDraw3D) {
		unsigned long		myMajor, myMinor;
		
		Q3Initialize();
		
		// see which version of QD3D is installed; version 1.5 provides some useful new geometries
		Q3GetVersion(&myMajor, &myMinor);
		gHasQuickDraw3D15 = ((myMajor >= 0x0001) && (myMinor >= 0x0005));
	}
}


//////////
//
// VR3DObjects_Stop
// Clean up for QuickDraw 3D.
//
//////////

void VR3DObjects_Stop (void)
{
	// perform any QD3D-related shutdown operations
	if (gHasQuickDraw3D)
		Q3Exit();
}


//////////
//
// VR3DObjects_IsQD3DAvailable
// Is QuickDraw 3D available in the present operating environment?
//
//////////

Boolean VR3DObjects_IsQD3DAvailable (void)
{
#if TARGET_OS_MAC
	return((Ptr)Q3Initialize != (Ptr)kUnresolvedCFragSymbolAddress);
#endif

#if TARGET_OS_WIN32
	return(true);
#endif

	// the following code should work cross-platform, but currently it returns -5551 (gestaltUndefSelectorErr) on Windows
//	Boolean 	myQD3DAvail = false;
//	long		myAttrs;
//	OSErr 		myErr = noErr;
//
//	myErr = Gestalt(gestaltQD3D, &myAttrs);
//	if (myErr == noErr)
//		if (myAttrs & (1L << gestaltQD3DPresent))
//			myQD3DAvail = true;
//
//	return(myQD3DAvail);
}


//////////
//
// VR3DObjects_InitWindowData
// Initialize the window-specific 3D data.
//
//////////

void VR3DObjects_InitWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData;
	Rect				myRect;
	OSErr				myErr = noErr;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
	
		// lock the application data handle
		HLock((Handle)myAppData);
		
		// get the size of the destination window
		if ((**theWindowObject).fMovie != NULL)
			GetMovieBox((**theWindowObject).fMovie, &myRect);
				
		// create an offscreen GWorld that is the size of the window area
		// (this GWorld is our draw context buffer)
		myErr = QTNewGWorld(&(**myAppData).fQD3DDCGWorld, kOffscreenPixelType, &myRect, NULL, NULL, kICMTempThenAppMemory);
		if (myErr != noErr)
			goto bail;

		// create a new view attached to the offscreen GWorld
		(**myAppData).fView = VR3DObjects_CreateView((**myAppData).fQD3DDCGWorld);
		
		// set the FOV vertical orientation flag to ON
		(**myAppData).fQD3DFOVIsVert = true;

		// set the default 3D chroma key color
		(**myAppData).fQD3DKeyColor = k3DChromaColor;

		// initialize QD3D camera's aspect ratio								 
		VR3DObjects_SetCameraAspectRatio(theWindowObject);								 

		// initialize QD3D camera's point-of-interest and FOV
		VR3DObjects_SetCamera(theWindowObject);
		
bail:
		// unlock the application data handle
		HUnlock((Handle)myAppData);
	}
}


//////////
//
// VR3DObjects_DumpWindowData
// Dump the window-specific 3D data.
//
//////////

void VR3DObjects_DumpWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
	
		// dispose of the view object
		if ((**myAppData).fView)
			Q3Object_Dispose((**myAppData).fView);

		// dispose of GWorld used by this window object
		if ((**myAppData).fQD3DDCGWorld != NULL)
			DisposeGWorld((**myAppData).fQD3DDCGWorld);
	}
}


//////////
//
// VR3DObjects_DoIdle
// Do any 3D-related processing that can or should occur at idle time.
// Returns true if the caller should call QTVRUpdate, false otherwise.
//
//////////

Boolean VR3DObjects_DoIdle (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	VRScript3DObjPtr		myPointer;
	Boolean					myNeedUpdate = false;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return(myNeedUpdate);
	
	// if any currently visible 3D object is animated or has a movie texture,
	// we need to update window
	myPointer = (VRScript3DObjPtr)(**myAppData).fListArray[kVREntry_QD3DObject];
	while ((myPointer != NULL) && !myNeedUpdate) {
		if (myPointer->fModelIsVisible)
			if ((myPointer->fModelIsAnimated) || (myPointer->fTextureIsMovie))
				myNeedUpdate = true;
		
		myPointer = myPointer->fNextEntry;
	}

	return(myNeedUpdate);
}
		
				
//////////
//
// VR3DObjects_EnlistBox
// Add a box to the list of 3D objects.
//
//////////

void VR3DObjects_EnlistBox (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize, UInt32 theOptions)
{
	TQ3BoxData				myBoxData;
	TQ3GeometryObject		myObject;
	TQ3GroupObject			myGroup;
	
	myBoxData.boxAttributeSet = NULL;
	myBoxData.faceAttributeSet = NULL;
	Q3Point3D_Set(&myBoxData.origin, theXSize / -2.0, theYSize / -2.0, theZSize / -2.0);
	Q3Vector3D_Set(&myBoxData.orientation, 0.0, theYSize, 0.0);
	Q3Vector3D_Set(&myBoxData.majorAxis, 0.0, 0.0, theZSize);	
	Q3Vector3D_Set(&myBoxData.minorAxis, theXSize, 0.0, 0.0);	
	myObject = Q3Box_New(&myBoxData);	
	if (myObject != NULL) {
		myGroup = VR3DObjects_CreateDefaultGroup(myObject);
		if (myGroup != NULL)
			VRScript_Enlist3DObject(theWindowObject, myGroup, theEntryID, theX, theY, theZ, theOptions);
	}
}


//////////
//
// VR3DObjects_EnlistCone
// Add a cone to the list of 3D objects.
//
//////////

void VR3DObjects_EnlistCone (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, float theMajRad, float theMinRad, float theHeight, UInt32 theOptions)
{
	TQ3ConeData				myConeData;
	TQ3GeometryObject		myObject;
	TQ3GroupObject			myGroup;
	
	myConeData.coneAttributeSet = NULL;
	myConeData.interiorAttributeSet = NULL;
	myConeData.faceAttributeSet = NULL;
	myConeData.bottomAttributeSet = NULL;
	myConeData.caps = kQ3EndCapMaskBottom;
	Q3Point3D_Set(&myConeData.origin, 0.0, theHeight / -2.0, 0.0);
	Q3Vector3D_Set(&myConeData.orientation, 0.0, theHeight, 0.0);
	Q3Vector3D_Set(&myConeData.majorRadius, 0.0, 0.0, theMajRad);	
	Q3Vector3D_Set(&myConeData.minorRadius, theMinRad, 0.0, 0.0);	
	myObject = Q3Cone_New(&myConeData);

	if (myObject != NULL) {
		myGroup = VR3DObjects_CreateDefaultGroup(myObject);
		if (myGroup != NULL) {
			VR3DObjects_SetSubdivisionStyle(myGroup, 20);	// set a finer subdivision style
			VRScript_Enlist3DObject(theWindowObject, myGroup, theEntryID, theX, theY, theZ, theOptions);
		}
	}
}


//////////
//
// VR3DObjects_EnlistCylinder
// Add a cylinder to the list of 3D objects.
//
//////////

void VR3DObjects_EnlistCylinder (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, float theMajRad, float theMinRad, float theHeight, UInt32 theOptions)
{
	TQ3CylinderData			myCylinderData;
	TQ3GeometryObject		myObject;
	TQ3GroupObject			myGroup;
	
	myCylinderData.cylinderAttributeSet = NULL;
	myCylinderData.interiorAttributeSet = NULL;
	myCylinderData.topAttributeSet = NULL;
	myCylinderData.faceAttributeSet = NULL;
	myCylinderData.bottomAttributeSet = NULL;
	myCylinderData.caps = kQ3EndCapMaskTop | kQ3EndCapMaskBottom;
	Q3Point3D_Set(&myCylinderData.origin, 0.0, theHeight / -2.0, 0.0);
	Q3Vector3D_Set(&myCylinderData.orientation, 0.0, theHeight, 0.0);
	Q3Vector3D_Set(&myCylinderData.majorRadius, 0.0, 0.0, theMajRad);	
	Q3Vector3D_Set(&myCylinderData.minorRadius, theMinRad, 0.0, 0.0);	
	myObject = Q3Cylinder_New(&myCylinderData);
	
	if (myObject != NULL) {
		myGroup = VR3DObjects_CreateDefaultGroup(myObject);
		if (myGroup != NULL) {
			VR3DObjects_SetSubdivisionStyle(myGroup, 20);	// set a finer subdivision style
			VRScript_Enlist3DObject(theWindowObject, myGroup, theEntryID, theX, theY, theZ, theOptions);
		}
	}
}


//////////
//
// VR3DObjects_EnlistEllipsoid
// Add an ellipsoid to the list of 3D objects.
//
//////////

void VR3DObjects_EnlistEllipsoid (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, float theMajRad, float theMinRad, float theHeight, UInt32 theOptions)
{
	TQ3EllipsoidData		myEllipsoidData;
	TQ3GeometryObject		myObject;
	TQ3GroupObject			myGroup;
	
	myEllipsoidData.ellipsoidAttributeSet = NULL;
	myEllipsoidData.interiorAttributeSet = NULL;
	myEllipsoidData.caps = 0L;
	Q3Point3D_Set(&myEllipsoidData.origin, 0.0, 0.0, 0.0);
	Q3Vector3D_Set(&myEllipsoidData.orientation, 0.0, theHeight, 0.0);
	Q3Vector3D_Set(&myEllipsoidData.majorRadius, 0.0, 0.0, theMajRad);	
	Q3Vector3D_Set(&myEllipsoidData.minorRadius, theMinRad, 0.0, 0.0);	
	myObject = Q3Ellipsoid_New(&myEllipsoidData);

	if (myObject != NULL) {
		myGroup = VR3DObjects_CreateDefaultGroup(myObject);
		if (myGroup != NULL) {
			VR3DObjects_SetSubdivisionStyle(myGroup, 20);	// set a finer subdivision style
			VRScript_Enlist3DObject(theWindowObject, myGroup, theEntryID, theX, theY, theZ, theOptions);
		}
	}
}


//////////
//
// VR3DObjects_EnlistTorus
// Add a torus to the list of 3D objects.
//
//////////

void VR3DObjects_EnlistTorus (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, float theMajRad, float theMinRad, float theHeight, float theRatio, UInt32 theOptions)
{
	TQ3TorusData			myTorusData;
	TQ3GeometryObject		myObject;
	TQ3GroupObject			myGroup;
	
	myTorusData.torusAttributeSet = NULL;
	myTorusData.interiorAttributeSet = NULL;
	myTorusData.caps = 0L;
	myTorusData.ratio = theRatio;
	Q3Point3D_Set(&myTorusData.origin, 0.0, 0.0, 0.0);
	Q3Vector3D_Set(&myTorusData.orientation, 0.0, theHeight, 0.0);
	Q3Vector3D_Set(&myTorusData.majorRadius, 0.0, 0.0, theMajRad);	
	Q3Vector3D_Set(&myTorusData.minorRadius, theMinRad, 0.0, 0.0);	
	myObject = Q3Torus_New(&myTorusData);

	if (myObject != NULL) {
		myGroup = VR3DObjects_CreateDefaultGroup(myObject);
		if (myGroup != NULL) {
			VR3DObjects_SetSubdivisionStyle(myGroup, 20);	// set a finer subdivision style
			VRScript_Enlist3DObject(theWindowObject, myGroup, theEntryID, theX, theY, theZ, theOptions);
		}
	}
}


//////////
//
// VR3DObjects_EnlistRectangle
// Add a rectangle to the list of 3D objects.
// This is intended to serve as a "screen" for texture-mapped movies or pictures.
//
//////////

void VR3DObjects_EnlistRectangle (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ,
									float theX1, float theY1, float theZ1, 
									float theX2, float theY2, float theZ2, 
									float theX3, float theY3, float theZ3, 
									float theX4, float theY4, float theZ4, UInt32 theOptions)
{
	TQ3PolyhedronData			myPolyData;
	static TQ3Vertex3D			myVertices[4];
	static TQ3Param2D			myUVParams[4] = {
									{0.0, 0.0},
									{0.0, 1.0},
									{1.0, 1.0},
									{1.0, 0.0}
								};
	
	TQ3PolyhedronTriangleData	myTriangles[2] = {
									{ { 0, 1, 3 }, kQ3PolyhedronEdge01 | kQ3PolyhedronEdge20, NULL },
									{ { 1, 2, 3 }, kQ3PolyhedronEdge01 | kQ3PolyhedronEdge12, NULL }
								};

	unsigned long				myIndex;
	TQ3GeometryObject			myObject;
	TQ3GroupObject				myGroup;
	
	// set up vertices, edges, and triangular faces
	Q3Point3D_Set(&myVertices[0].point, theX1, theY1, theZ1);
	Q3Point3D_Set(&myVertices[1].point, theX2, theY2, theZ2);
	Q3Point3D_Set(&myVertices[2].point, theX3, theY3, theZ3);
	Q3Point3D_Set(&myVertices[3].point, theX4, theY4, theZ4);
	myVertices[0].attributeSet = NULL;
	myVertices[1].attributeSet = NULL;
	myVertices[2].attributeSet = NULL;
	myVertices[3].attributeSet = NULL;
	myPolyData.numVertices = 4;
	myPolyData.vertices = myVertices;	
	myPolyData.numEdges = 0;
	myPolyData.edges = NULL;
	myPolyData.numTriangles	= 2;
	myPolyData.triangles = myTriangles;
	
	// add uv parameterization to the polyhedron
	for (myIndex = 0; myIndex < myPolyData.numVertices; myIndex++) {					
		myVertices[myIndex].attributeSet = Q3AttributeSet_New();
		Q3AttributeSet_Add(myVertices[myIndex].attributeSet, kQ3AttributeTypeSurfaceUV, &myUVParams[myIndex]);
	}
	
	myPolyData.polyhedronAttributeSet = NULL;		// inherit the attribute set from the current state
	myObject = Q3Polyhedron_New(&myPolyData);

	if (myObject != NULL) {
		myGroup = VR3DObjects_CreateDefaultGroup(myObject);
		if (myGroup != NULL)
			VRScript_Enlist3DObject(theWindowObject, myGroup, theEntryID, theX, theY, theZ, theOptions);
	}
}


//////////
//
// VR3DObjects_Enlist3DMFFile
// Open a 3DMF file and add it to the list of 3D objects.
//
//////////

void VR3DObjects_Enlist3DMFFile (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, UInt32 theOptions, char *thePathName)
{
	TQ3GroupObject			myGroup;
	
	myGroup = VR3DObjects_GetModelFromFile(thePathName);
	if (myGroup != NULL) 
		VRScript_Enlist3DObject(theWindowObject, myGroup, theEntryID, theX, theY, theZ, theOptions);
}


//////////
//
// VR3DObjects_CreateDefaultGroup
// Return a default model containing the specified object.
//
//////////

TQ3GroupObject VR3DObjects_CreateDefaultGroup (TQ3GeometryObject theObject)
{
	TQ3GroupObject			myGroup;
	
	// create a new display group
	myGroup = Q3DisplayGroup_New();
	if (myGroup != NULL) {
		TQ3ShaderObject			myIlluminationShader;
		
		// create an illumination shader	
		myIlluminationShader = Q3PhongIllumination_New();
		if (myIlluminationShader != NULL) {
			TQ3AttributeSet			myAttributeSet;

			// add the illumination shader to our display group	
			Q3Group_AddObject(myGroup, myIlluminationShader);
			
			// add the default diffuse color to the group
			myAttributeSet = Q3AttributeSet_New();
			Q3AttributeSet_Add(myAttributeSet, kQ3AttributeTypeDiffuseColor, &k3DObjColor);
			Q3Group_AddObject(myGroup, myAttributeSet);

			// add the geometry object to the group
			Q3Group_AddObject(myGroup, theObject);
			
			// dispose of the objects we've added to the group
			if (myIlluminationShader != NULL) 
				Q3Object_Dispose(myIlluminationShader);	
		}
	}
	
	if (theObject != NULL) 
		Q3Object_Dispose(theObject);
	
	return(myGroup);
}


//////////
//
// VR3DObjects_CreateView
// Return a view object for the QD3D objects in our model.
//
//////////

TQ3ViewObject VR3DObjects_CreateView (GWorldPtr theGWorld)
{
	TQ3Status				myStatus;
	TQ3ViewObject			myView;
	TQ3DrawContextObject	myDrawContext;
	TQ3RendererObject		myRenderer;
	TQ3CameraObject			myCamera;
	TQ3GroupObject			myLights;
	
	// create a new view object
	myView = Q3View_New();
	if (myView == NULL)
		goto bail;
	
	// create and set draw context
	myDrawContext = VR3DObjects_CreateDrawContext(theGWorld);
	if (myDrawContext == NULL)
		goto bail;
		
	myStatus = Q3View_SetDrawContext(myView, myDrawContext);
	if (myStatus == kQ3Failure)
		goto bail;

	Q3Object_Dispose(myDrawContext);
	
	// create and set renderer
	myRenderer = Q3Renderer_NewFromType(kQ3RendererTypeInteractive);
	if (myRenderer == NULL) {
		myRenderer = Q3Renderer_NewFromType(kQ3RendererTypeWireFrame);
	} else {
		// these two lines allow us to use the best possible renderer,
		// including hardware if it is installed.
		Q3InteractiveRenderer_SetDoubleBufferBypass(myRenderer, kQ3True);						
		Q3InteractiveRenderer_SetPreferences(myRenderer, kQAVendor_BestChoice, 0);
	}
		
	if (myRenderer == NULL) 
		goto bail;
	
	myStatus = Q3View_SetRenderer(myView, myRenderer);
	if (myStatus == kQ3Failure)
		goto bail;
		
	Q3Object_Dispose(myRenderer);
	
	// create and set camera
	myCamera = VR3DObjects_CreateCamera(theGWorld);
	if (myCamera == NULL)
		goto bail;
		
	myStatus = Q3View_SetCamera(myView, myCamera);
	if (myStatus == kQ3Failure)
		goto bail;

	Q3Object_Dispose(myCamera);
	
	// create and set lights
	myLights = VR3DObjects_CreateLights();
	if (myLights == NULL)
		goto bail;
		
	myStatus = Q3View_SetLightGroup(myView, myLights);
	if (myStatus == kQ3Failure)
		goto bail;

	Q3Object_Dispose(myLights);
		
	return(myView);
	
bail:
	// if any of the above creations failed, don't return a view
	return(NULL);
}


//////////
//
// VR3DObjects_CreateLights
// Return a light group object for the QD3D objects in our model.
// For displaying embedded movies, we need just the ambient light.
//
//////////

TQ3GroupObject VR3DObjects_CreateLights (void)
{
	TQ3GroupPosition			myGroupPosition;
	TQ3GroupObject				myLightList;
	TQ3LightData				myLightData;
	TQ3PointLightData			myPointLightData;
	TQ3DirectionalLightData		myDirectionalLightData;
	TQ3LightObject				myAmbientLight, myPointLight, myFillLight;
	TQ3Point3D					myPointLocation =	{-10.0, 0.0, 10.0};
	TQ3Vector3D					myFillDirection =	{10.0, 0.0, 10.0};
	TQ3ColorRGB					myWhiteLight =		{1.0F, 1.0F, 1.0F};
	
	// create the ambient light
	myLightData.isOn = kQ3True;
	myLightData.color = myWhiteLight;
	myLightData.brightness = 1.0F;
	myAmbientLight = Q3AmbientLight_New(&myLightData);
	if (myAmbientLight == NULL)
		goto bail;
	
	// create a point light
	myLightData.brightness = 1.0F;
	myPointLightData.lightData = myLightData;
	myPointLightData.castsShadows = kQ3False;
	myPointLightData.attenuation = kQ3AttenuationTypeNone;
	myPointLightData.location = myPointLocation;
	myPointLight = Q3PointLight_New(&myPointLightData);
	if (myPointLight == NULL)
		goto bail;

	// create a directional light for fill
	myLightData.brightness = 0.2F;
	myDirectionalLightData.lightData = myLightData;
	myDirectionalLightData.castsShadows = kQ3False;
	myDirectionalLightData.direction = myFillDirection;
	myFillLight = Q3DirectionalLight_New(&myDirectionalLightData);
	if (myFillLight == NULL)
		goto bail;

	// create a light group and add each of the lights to the group
	myLightList = Q3LightGroup_New();
	if (myLightList == NULL)
		goto bail;
	myGroupPosition = Q3Group_AddObject(myLightList, myAmbientLight);
	if (myGroupPosition == 0)
		goto bail;
	myGroupPosition = Q3Group_AddObject(myLightList, myPointLight);
	if (myGroupPosition == 0)
		goto bail;
	myGroupPosition = Q3Group_AddObject(myLightList, myFillLight);
	if (myGroupPosition == 0)
		goto bail;

	Q3Object_Dispose(myAmbientLight);
	Q3Object_Dispose(myPointLight);
	Q3Object_Dispose(myFillLight);

	return(myLightList);
	
bail:
	// if any of the above creations failed, don't return a light group
	return(NULL);
}


//////////
//
// VR3DObjects_CreateDrawContext
// Return a draw context object for the QD3D objects in our model.
//
//////////

TQ3DrawContextObject VR3DObjects_CreateDrawContext (GWorldPtr theGWorld)
{
	TQ3DrawContextObject		myDrawContext;
	TQ3PixmapDrawContextData	myPixMapDrawContextData;
	TQ3DrawContextData			myDrawContextData;
	PixMapHandle 				myPixMap;
	Rect						myRect;
	TQ3ColorARGB				myClearColor;
	float						myFactor = 0xffff;
	
	if (theGWorld == NULL)
		goto bail;
		
	// set the background color;
	// note that RGBColor is defined in the range 0-65535,
	// while TQ3ColorARGB is defined in the range 0.0-1.0; hence the division....
	myClearColor.a = 0.0;
	myClearColor.r = k3DChromaColor.red / myFactor;
	myClearColor.g = k3DChromaColor.green / myFactor;
	myClearColor.b = k3DChromaColor.blue / myFactor;
	
	// fill in draw context data
	myDrawContextData.clearImageMethod = kQ3ClearMethodWithColor;
	myDrawContextData.clearImageColor = myClearColor;
	myDrawContextData.paneState = kQ3False;
	myDrawContextData.maskState = kQ3False;
	myDrawContextData.doubleBufferState = kQ3False;
 
	myPixMapDrawContextData.drawContextData = myDrawContextData;
	
	// the pixmap must remain locked in memory for as long as it exists
	myPixMap = GetGWorldPixMap(theGWorld);
	LockPixels(myPixMap);

	myRect = theGWorld->portRect;
	
	myPixMapDrawContextData.pixmap.width = myRect.right - myRect.left;
	myPixMapDrawContextData.pixmap.height = myRect.bottom - myRect.top;
	myPixMapDrawContextData.pixmap.rowBytes = (unsigned long)QTGetPixMapHandleRowBytes(myPixMap)
	myPixMapDrawContextData.pixmap.pixelType = kQ3PixelTypeRGB32;
	myPixMapDrawContextData.pixmap.pixelSize = 32;
	myPixMapDrawContextData.pixmap.bitOrder = kQ3EndianBig;
	myPixMapDrawContextData.pixmap.byteOrder = kQ3EndianBig;
	myPixMapDrawContextData.pixmap.image = GetPixBaseAddr(myPixMap);
	
	// create draw context and return it
	myDrawContext = Q3PixmapDrawContext_New(&myPixMapDrawContextData);

bail:
	return(myDrawContext);
}


//////////
//
// VR3DObjects_CreateCamera
// Return a camera object for the QD3D objects in our model.
// Note that some of the initial values are wrong and will soon be reset (in InitApplicationWindowObject).
//
//////////

TQ3CameraObject VR3DObjects_CreateCamera (CGrafPtr thePort)
{
	TQ3CameraObject					myCamera;
	TQ3ViewAngleAspectCameraData	myCameraData;
	
	TQ3Point3D 						myFrom 		= kCameraOrigin;
	TQ3Point3D 						myTo 		= kPointOfInterest;
	TQ3Vector3D 					myUp 		= {0.0, 1.0, 0.0};

	float 							myFOV		= 1.00;				// initial FOV really depends on QTVR instance, so we reset this later
	float 							myHither	= (float)0.01;
	float 							myYon 		= 1000.0;
	
	myCameraData.cameraData.placement.cameraLocation = myFrom;
	myCameraData.cameraData.placement.pointOfInterest = myTo;
	myCameraData.cameraData.placement.upVector = myUp;

	myCameraData.cameraData.range.hither = myHither;
	myCameraData.cameraData.range.yon = myYon;

	// the default camera view port
	myCameraData.cameraData.viewPort.origin.x = -1.0F;
	myCameraData.cameraData.viewPort.origin.y = 1.0F;
	myCameraData.cameraData.viewPort.width = 2.0F;
	myCameraData.cameraData.viewPort.height = 2.0F;
	
	// some default FOV and aspect ratio values
	myCameraData.fov = myFOV;
	myCameraData.aspectRatioXToY =
		(float)(thePort->portRect.right - thePort->portRect.left) / 
		(float)(thePort->portRect.bottom - thePort->portRect.top);
		
	myCamera = Q3ViewAngleAspectCamera_New(&myCameraData);

	return(myCamera);
}


//////////
//
// VR3DObjects_GetModelAttributeSet
// Get the attribute set of a 3D model.
//
//////////

TQ3AttributeSet VR3DObjects_GetModelAttributeSet (VRScript3DObjPtr thePointer)
{
	TQ3AttributeSet		myAttributeSet = NULL;
	
	if ((thePointer != NULL) && (thePointer->fModel != NULL)) {
		TQ3GroupPosition	myPosition;
		
		// get the position of the attribute set in the group
		Q3Group_GetFirstPositionOfType(thePointer->fModel, kQ3SetTypeAttribute, &myPosition);
		if (myPosition != 0)		// get the attribute set
			Q3Group_GetPositionObject(thePointer->fModel, myPosition, &myAttributeSet);
	}
	
	return(myAttributeSet);
}


//////////
//
// VR3DObjects_SetSubdivisionStyle
// Create a subdivision style and add it to the specified group.
//
//////////

void VR3DObjects_SetSubdivisionStyle (TQ3GroupObject theGroup, short theNumDivisions)
{
	TQ3SubdivisionStyleData		myStyleData;
	TQ3StyleObject				myStyleObject;
	TQ3GroupPosition			myPosition;
	
	// get the position of the geometry object in the group
	Q3Group_GetFirstPositionOfType(theGroup, kQ3ShapeTypeGeometry, &myPosition);
	
	// set a subdivision style
	myStyleData.method = kQ3SubdivisionMethodConstant;
	myStyleData.c1 = theNumDivisions;
	myStyleData.c2 = theNumDivisions;
	myStyleObject = Q3SubdivisionStyle_New(&myStyleData);
	Q3Group_AddObjectBefore(theGroup, myPosition, myStyleObject);
	Q3Object_Dispose(myStyleObject);
}


//////////
//
// VR3DObjects_SetColor
// Set the diffuse color of a 3D object.
//
//////////

void VR3DObjects_SetColor (WindowObject theWindowObject, UInt32 theEntryID, float theRed, float theGreen, float theBlue, UInt32 theOptions)
{
#pragma unused(theOptions)
	VRScript3DObjPtr	myPointer;
	TQ3AttributeSet		myAttributeSet;
	
	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) {
	
		myAttributeSet = VR3DObjects_GetModelAttributeSet(myPointer);
		if (myAttributeSet != NULL) {
			TQ3ColorRGB				myColor;
			
			// remove the existing diffuse color attribute
			Q3AttributeSet_Clear(myAttributeSet, kQ3AttributeTypeDiffuseColor);
			Q3ColorRGB_Set(&myColor, theRed, theGreen, theBlue);
			Q3AttributeSet_Add(myAttributeSet, kQ3AttributeTypeDiffuseColor, &myColor);
		}
		
		Q3Object_Dispose(myAttributeSet);
	}
}


//////////
//
// VR3DObjects_SetTransparency
// Set the transparency level of a 3D object.
//
//////////

void VR3DObjects_SetTransparency (WindowObject theWindowObject, UInt32 theEntryID, float theRed, float theGreen, float theBlue, UInt32 theOptions)
{
#pragma unused(theOptions)
	VRScript3DObjPtr	myPointer;
	TQ3AttributeSet		myAttributeSet;
	
	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) {
	
		myAttributeSet = VR3DObjects_GetModelAttributeSet(myPointer);
		if (myAttributeSet != NULL) {
			TQ3ColorRGB				myColor;
			
			// remove the existing transparency color attribute
			Q3AttributeSet_Clear(myAttributeSet, kQ3AttributeTypeTransparencyColor);
			Q3ColorRGB_Set(&myColor, theRed, theGreen, theBlue);
			Q3AttributeSet_Add(myAttributeSet, kQ3AttributeTypeTransparencyColor, &myColor);
		}
		
		Q3Object_Dispose(myAttributeSet);
	}
}


//////////
//
// VR3DObjects_SetInterpolation
// Set the interpolation style of a 3D object.
//
//////////

void VR3DObjects_SetInterpolation (WindowObject theWindowObject, UInt32 theEntryID, UInt32 theStyle, UInt32 theOptions)
{
#pragma unused(theOptions)
	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) 
		Q3InterpolationStyle_Set(myPointer->fInterpolation, (TQ3InterpolationStyle)theStyle);
}


//////////
//
// VR3DObjects_SetBackfacing
// Set the backfacing style of a 3D object.
//
//////////

void VR3DObjects_SetBackfacing (WindowObject theWindowObject, UInt32 theEntryID, UInt32 theStyle, UInt32 theOptions)
{
#pragma unused(theOptions)
	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) 
		Q3BackfacingStyle_Set(myPointer->fBackFacing, (TQ3BackfacingStyle)theStyle);
}


//////////
//
// VR3DObjects_SetFill
// Set the fill style of a 3D object.
//
//////////

void VR3DObjects_SetFill (WindowObject theWindowObject, UInt32 theEntryID, UInt32 theStyle, UInt32 theOptions)
{
#pragma unused(theOptions)
	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) 
		Q3FillStyle_Set(myPointer->fFillStyle, (TQ3FillStyle)theStyle);
}


//////////
//
// VR3DObjects_SetLocation
// Set the location of a 3D object.
//
//////////

void VR3DObjects_SetLocation (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, UInt32 theOptions)
{
	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) {
		if (theOptions == kVRValue_Relative) {
			myPointer->fGroupCenter.x -= theX;
			myPointer->fGroupCenter.y += theY;
			myPointer->fGroupCenter.z -= theZ;
		} else {
			myPointer->fGroupCenter.x = -1 * theX;
			myPointer->fGroupCenter.y = theY;
			myPointer->fGroupCenter.z = -1 * theZ;
		}
	}
}


//////////
//
// VR3DObjects_SetRotation
// Set the rotation factors of a 3D object.
//
//////////

void VR3DObjects_SetRotation (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, UInt32 theOptions)
{
	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) {
		if (theOptions == kVRValue_Relative) {
			myPointer->fRotateFactors.x += theX;
			myPointer->fRotateFactors.y += theY;
			myPointer->fRotateFactors.z += theZ;
		} else {
			myPointer->fRotateFactors.x = theX;
			myPointer->fRotateFactors.y = theY;
			myPointer->fRotateFactors.z = theZ;
		}
	}
}


//////////
//
// VR3DObjects_SetRotationState
// Set the rotation state (on or off) of a 3D object.
//
//////////

void VR3DObjects_SetRotationState (WindowObject theWindowObject, UInt32 theEntryID, Boolean theState, UInt32 theOptions)
{
#pragma unused(theOptions)
	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) {
		if (theState == kVRState_Toggle)
			theState = !(myPointer->fModelIsAnimated);
		myPointer->fModelIsAnimated = theState;
	}
}


//////////
//
// VR3DObjects_SetVisibleState
// Set the visibility state (on or off) of a 3D object.
//
//////////

void VR3DObjects_SetVisibleState (WindowObject theWindowObject, UInt32 theEntryID, Boolean theState, UInt32 theOptions)
{
#pragma unused(theOptions)
	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) {
		if (theState == kVRState_Toggle)
			theState = !(myPointer->fModelIsVisible);
		myPointer->fModelIsVisible = theState;
	}
}


//////////
//
// VR3DObjects_SetTexture
// Set the texture of a 3D object.
//
//////////

void VR3DObjects_SetTexture (WindowObject theWindowObject, UInt32 theEntryID, Boolean isMovie, UInt32 theOptions, char *thePathName)
{
#pragma unused(theOptions)

	VRScript3DObjPtr	myPointer;

	myPointer = (VRScript3DObjPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, theEntryID);
	if (myPointer != NULL) {
	
		// delete any existing texture structure, since we'll soon create another one	
		if (myPointer->fTexture != NULL) {
			VR3DTexture_Delete(myPointer->fTexture);
			myPointer->fTexture = NULL;
		}
			
		myPointer->fTextureIsMovie = isMovie;
		myPointer->fTexture = VR3DTexture_New(thePathName, isMovie);
		
		if (myPointer->fTexture != NULL)
			VR3DTexture_AddToGroup(myPointer->fTexture, myPointer->fModel);
	}
}


//////////
//
// VR3DObjects_AnimateModel
// Animate the QuickDraw 3D model.
//
//////////

void VR3DObjects_AnimateModel (VRScript3DObjPtr thePointer)
{
	TQ3Matrix4x4		myMatrix;
	TQ3Vector3D			myVector;
	
	if (thePointer != NULL) {		
		// rotate the object around the local x axis
		Q3Vector3D_Set(&myVector, 1.0, 0.0, 0.0);
		Q3Matrix4x4_SetRotateAboutAxis(&myMatrix, &(thePointer->fGroupCenter), &myVector, thePointer->fRotateFactors.x);
		Q3Matrix4x4_Multiply(&(thePointer->fRotation), &myMatrix, &(thePointer->fRotation));
		
		// rotate the object around the local y axis
		Q3Vector3D_Set(&myVector, 0.0, 1.0, 0.0);
		Q3Matrix4x4_SetRotateAboutAxis(&myMatrix, &(thePointer->fGroupCenter), &myVector, thePointer->fRotateFactors.y);
		Q3Matrix4x4_Multiply(&(thePointer->fRotation), &myMatrix, &(thePointer->fRotation));
		
		// rotate the object around the local z axis
		Q3Vector3D_Set(&myVector, 0.0, 0.0, 1.0);
		Q3Matrix4x4_SetRotateAboutAxis(&myMatrix, &(thePointer->fGroupCenter), &myVector, thePointer->fRotateFactors.z);
		Q3Matrix4x4_Multiply(&(thePointer->fRotation), &myMatrix, &(thePointer->fRotation));
	}
}


//////////
//
// VR3DObjects_DrawModel
// Draw any QuickDraw 3D models into the (offscreen GWorld) pixmap draw context.
// This routine is called only in the prescreen buffer imaging complete procedure.
//
//////////

TQ3Status VR3DObjects_DrawModel (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData;
	TQ3Status			myStatus = kQ3Failure;
	TQ3ViewObject		myView;
	VRScript3DObjPtr	myPointer;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);	
	if (myAppData == NULL)
		return(myStatus);
	
	myView = (**myAppData).fView;
	if (myView == NULL)
		return(myStatus);
	
	// if the camera needs to be updated, do so now
	if ((**myAppData).fViewHasChanged)
		VR3DObjects_SetCamera(theWindowObject);

	// our rendering loop
	myStatus = Q3View_StartRendering(myView);
	if (myStatus != kQ3Failure)
		do {
			// walk our linked list of 3D objects and render any visible objects,
			// after updating any object-specific properties 
			myPointer = (VRScript3DObjPtr)(**myAppData).fListArray[kVREntry_QD3DObject];
			while (myPointer != NULL) {
				if (myPointer->fModelIsVisible) {
					// if there is an active movie texture, advance the movie to the next frame
					if (myPointer->fTexture)
						if (myPointer->fTextureIsMovie)
							VR3DTexture_NextFrame(myPointer->fTexture);
					
					// if animation is active, animate the model
					if (myPointer->fModelIsAnimated)
						VR3DObjects_AnimateModel(myPointer);
						
					// submit the model
					Q3Push_Submit(myView);
					VR3DObjects_SubmitModel(myPointer, myView);
					Q3Pop_Submit(myView);
				}
				
				myPointer = myPointer->fNextEntry;
			}
		
		} while (Q3View_EndRendering(myView) == kQ3ViewStatusRetraverse);
		
	// wait until the rendering is completed...
	Q3View_Sync(myView);
	
	return(myStatus);
}


//////////
//
// VR3DObjects_SubmitModel
// Submit a QuickDraw 3D model for rendering, picking, bounding, or writing.
//
//////////

TQ3Status VR3DObjects_SubmitModel (VRScript3DObjPtr thePointer, TQ3ViewObject theView)
{
	TQ3Vector3D		myTranslate;
	TQ3Vector3D		myScale;

	if (thePointer != NULL) {
		myTranslate = *(TQ3Vector3D *) &(thePointer->fGroupCenter);
		myScale.x = myScale.y = myScale.z = thePointer->fGroupScale;

		Q3Style_Submit(thePointer->fInterpolation, theView);
		Q3Style_Submit(thePointer->fBackFacing, theView);
		Q3Style_Submit(thePointer->fFillStyle, theView);
		Q3MatrixTransform_Submit(&(thePointer->fRotation), theView);
		Q3TranslateTransform_Submit(&myTranslate, theView);
		Q3ScaleTransform_Submit(&myScale, theView);
		Q3DisplayGroup_Submit(thePointer->fModel, theView);
		return(kQ3Success);
	}
	
	return(kQ3Failure);
}


//////////
//
// VR3DObjects_GetModelFromFile
// Open a model contained in a 3DMF file.
//
//////////

TQ3GroupObject VR3DObjects_GetModelFromFile (char *thePathName)
{
	TQ3StorageObject	myStorage;
	TQ3GroupObject 		myModel;
	OSErr				myErr = noErr;
	
#if TARGET_OS_MAC
	FSSpec				myFSSpec;
	short				myRefNum;

	// for Macintosh, open the file and create a Mac storage object
	FSMakeFSSpec(0, 0L, c2pstr(thePathName), &myFSSpec);
	myErr = FSpOpenDF(&myFSSpec, fsRdPerm, &myRefNum);
	if (myErr == noErr)
		myStorage = Q3MacintoshStorage_New(myRefNum);
#endif

#if TARGET_OS_WIN32
	// for Windows, create a UNIX pathname storage object (Q3UnixPathStorage_New calls fopen internally)
	myStorage = Q3UnixPathStorage_New(thePathName);
#endif	
	
	if (myStorage != NULL) {
		TQ3FileObject		myFile;
		TQ3Status			myStatus;
		TQ3Object			myObject;
		TQ3Boolean			isEOF;
		
		// create a file object
		myFile = Q3File_New();
		if (myFile == NULL)
			return(NULL);
		
		// associate the storage with the file
		Q3File_SetStorage(myFile, myStorage);
		Q3Object_Dispose(myStorage);
	
		// read the drawable objects from the file object into a new group
		myModel = Q3DisplayGroup_New();
		if (myModel != NULL) {
			myStatus = Q3File_OpenRead(myFile, NULL);
			if (myStatus == kQ3Success) {
			
				do {
					myObject = Q3File_ReadObject(myFile);
					
					// if object read is not NULL, then process object
					if (myObject != NULL) {
						if (Q3Object_IsDrawable(myObject))
							Q3Group_AddObject(myModel, myObject);
						
						Q3Object_Dispose(myObject);
					}
					
					// check to see whether we've reached the end of file yet
					isEOF = Q3File_IsEndOfFile(myFile);
				} while (isEOF == kQ3False);
				
			}
		}
		
		// close the file object
		Q3File_Close(myFile);
	}
	
	// apply illumination shader to model
	myModel = VR3DObjects_CreateDefaultGroup(myModel);

#if TARGET_OS_MAC
	// close the file, since we've got the data we need
	FSClose(myRefNum);
#endif

#if TARGET_OS_WIN32
	// no action required for UNIX pathname storage
#endif

	return(myModel);
}


///////////
//
// VR3DObjects_PrescreenRoutine
// Draw the 3D objects into the current graphics world.
//
//////////

void VR3DObjects_PrescreenRoutine (QTVRInstance theInstance, WindowObject theWindowObject)
{
#pragma unused(theInstance)

	ApplicationDataHdl	myAppData;
	CGrafPtr			myGWorld;
	GDHandle			myGDevice;
	PixMapHandle		mySrcPixMap;
	PixMapHandle		myDstPixMap;
				
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	// render any objects into the pixmap draw context
	VR3DObjects_DrawModel(theWindowObject);
			
	// get the current graphics world
	// (on entry, the current graphics world is set to the prescreen buffer)
	GetGWorld(&myGWorld, &myGDevice);
	
	RGBBackColor(&(**myAppData).fQD3DKeyColor);
	OpColor(&(**myAppData).fQD3DKeyColor);
	
	mySrcPixMap = GetGWorldPixMap((**myAppData).fQD3DDCGWorld);
	myDstPixMap = GetGWorldPixMap(myGWorld);
	// we don't need to lock the source pixel map, since it was locked when the draw context was created
	LockPixels(myDstPixMap);
	
	// copy the rendered image to the current graphics world
	CopyBits((BitMapPtr)(*mySrcPixMap),
			 (BitMapPtr)(*myDstPixMap),
			 &(**myAppData).fQD3DDCGWorld->portRect, 
			 &myGWorld->portRect,
			 srcCopy | transparent, 
			 NULL);
			 
	UnlockPixels(myDstPixMap);
}


//////////
//
// VR3DObjects_SetCamera
// Set the FOV of the 3D camera associated with the specified window object;
// then set the point-of-interest and the location of the 3D camera.
//
//////////

void VR3DObjects_SetCamera (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	TQ3ViewObject			myView;
	TQ3CameraObject			myCamera;
	TQ3CameraPlacement		myCameraPos;
	QTVRInstance			myInstance;
	
	if (theWindowObject == NULL)
		return;
	
	// get the QTVR instance associated with the specified window
	myInstance = (**theWindowObject).fInstance;
	if (myInstance == NULL)
		return;
		
	// get the view object associated with the specified window
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);	
	if (myAppData == NULL)
		return;
	myView = (**myAppData).fView;

	// get the camera associated with the view object
	Q3View_GetCamera(myView, &myCamera);
	
	if (myCamera != NULL) {
		float				myFOV, myPan, myTilt;
		TQ3Point3D			myPoint;
		TQ3Vector3D			myUpVector;
		
		// set the camera's field of view
		myFOV = QTVRGetFieldOfView(myInstance);
		
		if ((**myAppData).fQD3DFOVIsVert) {
			Q3ViewAngleAspectCamera_SetFOV(myCamera, myFOV);
		} else {
			float			myRatio;
			
			Q3ViewAngleAspectCamera_GetAspectRatio(myCamera, &myRatio);
			Q3ViewAngleAspectCamera_SetFOV(myCamera, myFOV * myRatio);
		}

		// get the camera's current pan and tilt angles
		myPan = QTVRGetPanAngle(myInstance);
		myTilt = QTVRGetTiltAngle(myInstance);

		// calculate the new point-of-interest
		myPoint.x = sin(myPan) * cos(myTilt) * k3DObjectDistance;
		myPoint.y = sin(myTilt) * k3DObjectDistance;
		myPoint.z = cos(myPan) * cos(myTilt) * k3DObjectDistance;
		
		// calculate the new up vector of the camera
		myUpVector.x = -sin(myTilt) * sin(myPan);
		myUpVector.y = +cos(myTilt);
		myUpVector.z = -sin(myTilt) * cos(myPan);
		Q3Vector3D_Normalize(&myUpVector, &myUpVector);
		
		Q3Camera_GetPlacement(myCamera, &myCameraPos);
		myCameraPos.upVector = myUpVector;
		
		myCameraPos.pointOfInterest = myPoint;
		myCameraPos.cameraLocation = kCameraOrigin;
		Q3Camera_SetPlacement(myCamera, &myCameraPos);
		
		// update the QD3D camera
		Q3View_SetCamera(myView, myCamera);
		Q3Object_Dispose(myCamera);
	}
}


//////////
//
// VR3DObjects_SetCameraAspectRatio
// Adjust the aspect ratio of the QuickDraw 3D camera based on current window shape.
//
//////////

void VR3DObjects_SetCameraAspectRatio (WindowObject theWindowObject)
{
	ApplicationDataHdl				myAppData;
	TQ3ViewObject					myView;
	TQ3CameraObject					myCamera;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);	
	if (myAppData == NULL)
		return;
		
	// get the window's view and camera objects
	myView = (**myAppData).fView;
	Q3View_GetCamera(myView, &myCamera);

	// adjust the aspect ratio of the camera
	if (myCamera != NULL) {
		Rect						myRect;
		float						myRatio;
	
		// get the size of the specified window
		if ((**theWindowObject).fMovie != NULL)
			GetMovieBox((**theWindowObject).fMovie, &myRect);
	
		// calculate the aspect ratio of the movie rectangle
		myRatio = (float)(myRect.right - myRect.left) / (float)(myRect.bottom - myRect.top);

		// determine whether QD3D FOV is horizontal or vertical
		(**myAppData).fQD3DFOVIsVert = (myRatio >= 1.0);
				
		// adjust camera's aspect ratio
		Q3ViewAngleAspectCamera_SetAspectRatio(myCamera, myRatio);
		Q3Object_Dispose(myCamera);
	}
}


//////////
//
// VR3DObjects_UpdateDrawContext
// Delete current QD3D draw context and create a new one based on current window size.
//
//////////

void VR3DObjects_UpdateDrawContext (WindowObject theWindowObject)
{
	ApplicationDataHdl				myAppData;
	TQ3ViewObject					myView;
	TQ3DrawContextObject			myDrawContext;
	CGrafPtr						mySavedPort;
	GDHandle						mySavedDevice;
	Rect							myRect;
	OSErr							myErr = noErr;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);	
	if (myAppData == NULL)
		return;
		
	// lock the application data handle
	HLock((Handle)myAppData);
	
	// get the size of the movie in the specified window
	if ((**theWindowObject).fMovie != NULL)
		GetMovieBox((**theWindowObject).fMovie, &myRect);
	
	// get the current drawing environment
	GetGWorld(&mySavedPort, &mySavedDevice);

	// update the pixmap draw context GWorld: dispose of the existing one and then allocate a new one
	DisposeGWorld((**myAppData).fQD3DDCGWorld);
	(**myAppData).fQD3DDCGWorld = NULL;
	
	myErr = QTNewGWorld(&(**myAppData).fQD3DDCGWorld, kOffscreenPixelType, &myRect, NULL, NULL, kICMTempThenAppMemory);
	if (myErr != noErr)
		goto bail;

	// get the window's view and draw context objects
	myView = (**myAppData).fView;
	Q3View_GetDrawContext(myView, &myDrawContext);

	// associate the new GWorld with the QD3D draw context
	if (myDrawContext != NULL) {
		// get rid of existing draw context	
		Q3Object_Dispose(myDrawContext);		
	
		// create and set new draw context
		myDrawContext = VR3DObjects_CreateDrawContext((**myAppData).fQD3DDCGWorld);
		Q3View_SetDrawContext(myView, myDrawContext);
		Q3Object_Dispose(myDrawContext);
	}
	
bail:
	// restore the original drawing environment
	SetGWorld(mySavedPort, mySavedDevice);
	HUnlock((Handle)myAppData);
}


//////////
//
// VR3DObjects_DumpEntryMem
// Release any memory associated with the specified list entry.
//
//////////

void VR3DObjects_DumpEntryMem (VRScript3DObjPtr theEntry)
{	
	if (theEntry != NULL) {
		if (theEntry->fModel)
			Q3Object_Dispose(theEntry->fModel);
		if (theEntry->fInterpolation)
			Q3Object_Dispose(theEntry->fInterpolation);
		if (theEntry->fBackFacing)
			Q3Object_Dispose(theEntry->fBackFacing);
		if (theEntry->fFillStyle)
			Q3Object_Dispose(theEntry->fFillStyle);
		VR3DTexture_Delete(theEntry->fTexture);
	}
}

#endif	// QD3D_AVAIL
