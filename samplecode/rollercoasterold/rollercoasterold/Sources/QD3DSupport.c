

/*
	File:		QD3DSupport.c
	
	Contains:	QD3D support routines
	
	Written by:	Scott Kuechle, based on original Gerbils code by Brian Greenstone

	Copyright:	© 1998 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<2>		9/28/98		rtm		made changes for Metrowerks compiler
		<1>		9/01/98		srk		first file


*/

/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/

#include "QD3DSupport.h"
#include "Utils.h"


/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
	static TQ3ViewObject 			QD3DSupport_NewView(WindowPtr theWindow);
	static TQ3DrawContextObject 	QD3DSupport_NewDrawContext(WindowPtr theWindow);
	static TQ3ShaderObject 			QD3DSupport_CreateShaderFromTexture(short grndTextureResourceID);
#else if TARGET_OS_WIN32
	static TQ3ViewObject 			QD3DSupport_NewView(HWND theWindow);
	static TQ3DrawContextObject 	QD3DSupport_NewDrawContext(HWND theWindow);
	static TQ3ShaderObject 			QD3DSupport_CreateShaderFromTexture(LPTSTR	grndTextureFilePath);
#endif

static TQ3GroupObject 			QD3DSupport_GroundInit(TQ3ShaderObject	groundShaderObject);
static TQ3CameraObject 			QD3DSupport_NewCamera(float docWidth, float docHeight);
static TQ3GroupObject			QD3DSupport_NewLights(void);
static OSErr 					QD3DSupport_QuickTimeInit (void);
static TQ3GroupObject 			QD3DSupport_NewTrackGroup(TQ3ShaderObject	theShader);



/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_NewView                         *
*                                                           *
*    PURPOSE:   Creates a new QD3D view object, along with  *
*               the accompanying draw context, light and    *
*               camera objects                              *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
	static TQ3ViewObject QD3DSupport_NewView(WindowPtr theWindow)
#else if TARGET_OS_WIN32
	static TQ3ViewObject QD3DSupport_NewView(HWND theWindow)
#endif
{
	TQ3Status				myStatus;
	TQ3ViewObject			myView;
	TQ3DrawContextObject	myDrawContext;
	TQ3RendererObject		myRenderer;
	TQ3GroupObject			myLights;
	TQ3CameraObject			myCamera;
#if TARGET_OS_WIN32
	RECT					clientRect;
	BOOL					success;
#endif


		myView = Q3View_New();
		if (myView == nil)
		{
			goto bail;
		}

		//	Create and set draw context.
		if ((myDrawContext = QD3DSupport_NewDrawContext(theWindow)) == nil )
		{
			goto bail;
		}
			
		if ((myStatus = Q3View_SetDrawContext(myView, myDrawContext)) == kQ3Failure )
		{
			goto bail;
		}

		Q3Object_Dispose( myDrawContext ) ;
		
		//	Create and set renderer.
		
		// this would use the Z-Buffer renderer
	#if 0

		myRenderer = Q3Renderer_NewFromType(kQ3RendererTypeWireFrame);
		if ((myStatus = Q3View_SetRenderer(myView, myRenderer)) == kQ3Failure ) {
			goto bail;
		}
		
	#else

		// this would use the interactive software renderer

		if ((myRenderer = Q3Renderer_NewFromType(kQ3RendererTypeInteractive)) != nil )
		{
			if ((myStatus = Q3View_SetRenderer(myView, myRenderer)) == kQ3Failure )
			{
				goto bail;
			}
			// these two lines set us up to use the best possible renderer,
			// including  hardware if it is installed.
			Q3InteractiveRenderer_SetDoubleBufferBypass (myRenderer, kQ3True);						
			Q3InteractiveRenderer_SetPreferences(myRenderer, kQAVendor_BestChoice, 0);

		}
		else
		{
			goto bail;
		}
	#endif

		Q3Object_Dispose( myRenderer ) ;
		
		//	Create and set camera.
		#if TARGET_OS_MAC
			if ( (myCamera = QD3DSupport_NewCamera((float) (theWindow->portRect.right - theWindow->portRect.left),
													(float) (theWindow->portRect.bottom - theWindow->portRect.top))) == nil )
			{
				goto bail;
			}
		#else if TARGET_OS_WIN32
			success = GetClientRect(theWindow, &clientRect);
			if (success)
			{
				if ( (myCamera = QD3DSupport_NewCamera((float)RECT_WIDTH(clientRect), (float)RECT_HEIGHT(clientRect))) == nil )
				{
					goto bail;
				}
			}
			else
			{
				goto bail;
			}
		#endif
			
		if ((myStatus = Q3View_SetCamera(myView, myCamera)) == kQ3Failure )
		{
			goto bail;
		}
		Q3Object_Dispose( myCamera ) ;
		
		//	Create and set lights.
		if ((myLights = QD3DSupport_NewLights()) == nil )
		{
			goto bail;
		}
			
		if ((myStatus = Q3View_SetLightGroup(myView, myLights)) == kQ3Failure )
		{
			goto bail;
		}
			
		Q3Object_Dispose(myLights);

		//	Done!!!
		return ( myView );
		
	bail:
		//	If any of the above failed, then don't return a view.
		return ( nil );
}


/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_NewDrawContext                  *
*                                                           *
*    PURPOSE:   Creates a new QD3D draw context object      *
*                                                           *
*                                                           *
*                                                           *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC

static TQ3DrawContextObject QD3DSupport_NewDrawContext(WindowPtr theWindow)
{
	TQ3DrawContextData		myDrawContextData;
	TQ3MacDrawContextData	myMacDrawContextData;
	TQ3ColorARGB			ClearColor;
	TQ3DrawContextObject	myDrawContext ;
	
		/*	Set the background color. */
		ClearColor.a = 1.0F;
		ClearColor.r = 0.0F;
		ClearColor.g = 0.0F;
		ClearColor.b = 1.0F;
		
		/*	Fill in draw context data. */
		myDrawContextData.clearImageMethod	= kQ3ClearMethodWithColor;
		myDrawContextData.clearImageColor	= ClearColor;
		myDrawContextData.paneState			= kQ3False;
		myDrawContextData.maskState			= kQ3False;
		myDrawContextData.doubleBufferState = kQ3True;
	 
		myMacDrawContextData.drawContextData = myDrawContextData;
		
		myMacDrawContextData.window = (CGrafPtr) theWindow;		// this is the window associated with the view
		myMacDrawContextData.library = kQ3Mac2DLibraryNone;
		myMacDrawContextData.viewPort = nil;
		myMacDrawContextData.grafPort = (CGrafPtr) theWindow;
		
		/*	Create draw context and return it, if it’s nil the caller must handle */
		myDrawContext = Q3MacDrawContext_New(&myMacDrawContextData) ;

		return myDrawContext ;
}


#else if TARGET_OS_WIN32

static TQ3DrawContextObject QD3DSupport_NewDrawContext(HWND theWindow)
{

	TQ3DrawContextData			myDrawContextData;
	TQ3Win32DCDrawContextData	myWin32DCDrawContextData;
	TQ3DrawContextObject		myDrawContext ;
	DWORD						wndClassStyle;
	TQ3ColorARGB				ClearColor;


		/*	Set the background color. */

		ClearColor.a = 1.0;
		ClearColor.r = 0.0;
		ClearColor.g = 0.0;
		ClearColor.b = 1.0;

		/*	Fill in draw context data. */

		myDrawContextData.clearImageMethod	= kQ3ClearMethodWithColor;
		myDrawContextData.clearImageColor	= ClearColor;
		myDrawContextData.paneState			= kQ3False;
		myDrawContextData.maskState			= kQ3False;
		myDrawContextData.doubleBufferState = kQ3True;

		myWin32DCDrawContextData.drawContextData = myDrawContextData;

		/* Assertion: window MUST be CS_OWNDC */

		wndClassStyle = GetClassLong( theWindow, GCL_STYLE );
		if( CS_OWNDC != ( wndClassStyle & CS_OWNDC ) )
		{
			return NULL;
		}

		/* set up the win32DCDrawContext */
		myWin32DCDrawContextData.hdc = GetDC( theWindow );

		/*	Create draw context and return it, if it's NULL the caller must handle */
		myDrawContext = Q3Win32DCDrawContext_New(&myWin32DCDrawContextData) ;

		ReleaseDC(theWindow, myWin32DCDrawContextData.hdc);

		return myDrawContext ;

}

#endif

/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_NewCamera                       *
*                                                           *
*    PURPOSE:   Creates our camera                          *
*                                                           *
*                                                           *
*************************************************************/

static TQ3CameraObject QD3DSupport_NewCamera(float 			docWidth,
											float 			docHeight)
{
	TQ3ViewAngleAspectCameraData	perspectiveData;
	TQ3CameraObject					camera;
	
	TQ3Point3D 					from 	= { 30.0F, 50.0F, -50.0F };
	TQ3Point3D 					to 		= { 30.0F, 0.0F, 0.0F };
	TQ3Vector3D 				up 		= { 0.0F, 1.0F, 0.0F };

	float 						fieldOfView = 1.2F;
	float 						hither 		= 0.2F;
	float 						yon 		= 200.0F;
	

		perspectiveData.cameraData.placement.cameraLocation 	= from;
		perspectiveData.cameraData.placement.pointOfInterest 	= to;
		perspectiveData.cameraData.placement.upVector 			= up;

		perspectiveData.cameraData.range.hither	= hither;
		perspectiveData.cameraData.range.yon 	= yon;

		perspectiveData.cameraData.viewPort.origin.x 	= 	-1.0F;
		perspectiveData.cameraData.viewPort.origin.y 	= 	1.0F;
		perspectiveData.cameraData.viewPort.width		= 	2.0F;
		perspectiveData.cameraData.viewPort.height 		= 	2.0F;
		
		perspectiveData.fov				= fieldOfView;
		perspectiveData.aspectRatioXToY	= docWidth / docHeight;

		camera = Q3ViewAngleAspectCamera_New(&perspectiveData);

		return camera ;
}


/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_NewLights                       *
*                                                           *
*    PURPOSE:   Create our lights                           *
*                                                           *
*                                                           *
*************************************************************/

static TQ3GroupObject QD3DSupport_NewLights()
{
	TQ3GroupPosition		myGroupPosition;
	TQ3GroupObject			myLightList;
	TQ3LightData			myLightData;
	TQ3LightObject			myAmbientLight = NULL;
	TQ3ColorRGB				WhiteLight = { 1.0F, 1.0F, 1.0F };
	
		/*	Set up light data for ambient light.  This light data will be used for point and fill
			light also. */

		myLightData.isOn = kQ3True;
		myLightData.color = WhiteLight;
		
		/*	Create ambient light. */
		myLightData.brightness = 0.8F;
		myAmbientLight = Q3AmbientLight_New(&myLightData);
		if ( myAmbientLight == nil )
		{
			goto bail;
		}

		/*	Create light group and add each of the lights into the group. */
		myLightList = Q3LightGroup_New();
		if ( myLightList == nil )
		{
			goto bail;
		}
		myGroupPosition = Q3Group_AddObject(myLightList, myAmbientLight);
		if ( myGroupPosition == 0 )
		{
			goto bail;
		}

		Q3Object_Dispose( myAmbientLight ) ;

		/*	Done! */
		return ( myLightList );
		
	bail:
		/*	If any of the above failed, then return nothing! */
		if (myAmbientLight != NULL)
		{
			Q3Object_Dispose( myAmbientLight ) ;
		}
		return ( nil );
}


/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_NewTrackGroup                   *
*                                                           *
*    PURPOSE:   Creates a new group for our track & its     *
*               associated shader object                    *
*                                                           *
*                                                           *
*************************************************************/

static TQ3GroupObject QD3DSupport_NewTrackGroup(TQ3ShaderObject	theShader)
{
	TQ3GroupObject	myGroup = NULL;	
			
		if ((myGroup = Q3OrderedDisplayGroup_New()) != NULL )
		{
		TQ3GroupPosition myGroupPosition;
		
				/* ADD TEXTURE SHADER OBJECT TO GROUP */
			myGroupPosition = Q3Group_AddObject(myGroup, theShader);
			if ( myGroupPosition == nil )
			{
				Utils_DisplayErrorMsg("Group_AddObject failed!");
			}
		}
						
		/*	Done! */
		return ( myGroup );
}



/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_GroundInit                      *
*                                                           *
*    PURPOSE:   Creates a new ground object and group       *
*                                                           *
*                                                           *
*************************************************************/

static TQ3GroupObject QD3DSupport_GroundInit(TQ3ShaderObject	groundShaderObject)
{
	long				i;
	TQ3GeometryObject	polygonObj = NULL;
	TQ3GroupPosition	myGroupPosition;
	TQ3GroupObject		groundGroup;
	TQ3PolygonData		polygonData;
	TQ3Vertex3D			vertices[kNumGrndTextureVertices] = {0,-3,GROUND_SIZE,nil,
															GROUND_SIZE,-3,GROUND_SIZE,nil,
															GROUND_SIZE,-3,-GROUND_SIZE,nil,
															0,-3,-GROUND_SIZE,nil};
	TQ3AttributeSet		attribs[kNumGrndTextureVertices] = {NULL, NULL, NULL, NULL};
	float				ambient = 1.0F;
	TQ3Param2D			uv[kNumGrndTextureVertices] = {0,0,1,0,1,1,0,1};



		if (groundShaderObject == NULL)

		{

			return NULL;

		}


		groundGroup =  Q3OrderedDisplayGroup_New();
		if (groundGroup == NULL)
		{
			return NULL;
		}
			
				/* ADD TEXTURE SHADER OBJECT TO GROUP */
		myGroupPosition = Q3Group_AddObject(groundGroup, groundShaderObject);
		if ( myGroupPosition == nil )
		{
			Utils_DisplayErrorMsg("Q3Group_AddObject failed!");
			goto outOfMem;
		}
		Q3Object_Dispose(groundShaderObject);


				/* CREATE VERTICES */
			
		for (i=0; i < kNumGrndTextureVertices; i++)
		{
			attribs[i] = Q3AttributeSet_New();	
			if( attribs[i] == NULL )
			{
				Utils_DisplayErrorMsg("Attribute set creation failed!");	
				goto outOfMem;
			}
			Q3AttributeSet_Add(attribs[i], kQ3AttributeTypeShadingUV, &uv[i]);
			vertices[i].attributeSet = attribs[i];
		}

				/* CREATE NEW POLYGON OBJECT */

		polygonData.numVertices = kNumGrndTextureVertices;
		polygonData.vertices = vertices;
		polygonData.polygonAttributeSet = nil;
		polygonObj = Q3Polygon_New(&polygonData);			
		
		if( polygonObj == NULL )
		{
			Utils_DisplayErrorMsg("Polygon_New failed!");	
			goto outOfMem;
		}
		
		for (i=0; i < kNumGrndTextureVertices; i++)
		{			
			Q3Object_Dispose(attribs[i]);
			attribs[i] = NULL;
		}

		myGroupPosition = Q3Group_AddObject(groundGroup, polygonObj);
		Q3Object_Dispose(polygonObj);
		if ( myGroupPosition == nil )
		{
			Utils_DisplayErrorMsg("Q3Group_AddObject failed!");	
			goto outOfMem;
		}
		
			/* Success! */
			
		return groundGroup;
		
			/* Error handling */
	outOfMem:
		if (groundGroup)
		{
			Q3Object_Dispose(groundGroup);
		}

		for (i=0; i < kNumGrndTextureVertices; i++)
		{			
			if( attribs[i] )
			{
				Q3Object_Dispose(attribs[i]);
			}
		}
		
		return NULL;
}


/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_InitDoc3DData                   *
*                                                           *
*    PURPOSE:   General initialization routine which        *
*               creates our QD3D view and builds our track  *
*               and ground objects                          *
*                                                           *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
	void QD3DSupport_InitDoc3DData( WindowPtr 	window,
									DocumentPtr theDocument )
#else if TARGET_OS_WIN32
	void QD3DSupport_InitDoc3DData( HWND 		window,
									DocumentPtr theDocument )
#endif
{
	TQ3Status 		status;
	TQ3ShaderObject	groundShaderObject;
	short			partCount;
	
	#if TARGET_OS_WIN32

		OSErr err;
		DWORD error;


	#endif


			/* Initialize QuickDraw 3D, open a connection to the QuickDraw 3D library */

		status = Q3Initialize();
		if ( status == kQ3Failure )
		{
			Utils_DisplayFatalErrorMsg("QD3D Initialization failure!");
		}
		
		#if TARGET_OS_WIN32

			err = QD3DSupport_QuickTimeInit ();

			if ( err != noErr )
			{
				Utils_DisplayFatalErrorMsg("QTML Initialization failure!");
			}
		#endif

		theDocument->fSplinePointsPtr 	= NULL;
		theDocument->fNumSplineNubs 	= 0;
		theDocument->fNumSplinePoints 	= 0;
		theDocument->fTrackIndex 		= 0;
		theDocument->fMainWindow 		= window;

			/* sets up the 3d data for the scene */
			/* Create view for QuickDraw 3D. */
		theDocument->fView = QD3DSupport_NewView( theDocument->fMainWindow ) ;
		if (theDocument->fView == NULL)
		{
			Utils_DisplayFatalErrorMsg("Failure creating a new view!");
		}
		
			/* get the view's camera for use later */
		Q3View_GetCamera(theDocument->fView, &theDocument->fCamera);
		
			/* the drawing styles: */
		theDocument->fInterpolation = Q3InterpolationStyle_New(kQ3InterpolationStyleVertex) ;
		theDocument->fBackFacing 	= Q3BackfacingStyle_New(kQ3BackfacingStyleBoth ) ;
		theDocument->fFillStyle 	= Q3FillStyle_New(kQ3FillStyleFilled ) ;


			/* get shaders for ground/track textures */
		#if TARGET_OS_MAC
		
			groundShaderObject = QD3DSupport_CreateShaderFromTexture(kGrndTextureResID);
			theDocument->fTrackShader = QD3DSupport_CreateShaderFromTexture(kTextureRezID);

		#else if TARGET_OS_WIN32
		
			error = Utils_Win32_BuildCurDirPath((Ptr)&theDocument->fGroundTextureFilePath, kGroundTextureFileName);
			if (!Utils_Win32_DoesFileExist((char *)&theDocument->fGroundTextureFilePath))

			{

				Utils_DisplayFatalErrorMsg("Failure loading ground texture file GroundTexture.pct!");

			}

			error = Utils_Win32_BuildCurDirPath((Ptr)&theDocument->fTrackTextureFilePath, kTrackTextureFileName);
			if (!Utils_Win32_DoesFileExist((char *)&theDocument->fGroundTextureFilePath))

			{

				Utils_DisplayFatalErrorMsg("Failure loading track texture file MetalTrack.pct!");

			}

			
			groundShaderObject = QD3DSupport_CreateShaderFromTexture((Ptr)&theDocument->fGroundTextureFilePath);
			theDocument->fTrackShader = QD3DSupport_CreateShaderFromTexture((Ptr)&theDocument->fTrackTextureFilePath);

		#endif
		
			/* create ground/track groups */
			
		theDocument->fGroundGroup = QD3DSupport_GroundInit(groundShaderObject);
		theDocument->fTrackGroup = QD3DSupport_NewTrackGroup (theDocument->fTrackShader);
		
			/* load track parts */

		#if TARGET_OS_WIN32

			Track_LoadPartsFromFile((PartType *)&theDocument->fPartsList, &partCount);

		#elif TARGET_OS_MAC

			Track_LoadPartsFromRez((PartType *)&theDocument->fPartsList, &partCount);

		#endif

			/* build track from track parts */
		if (partCount > 0)
		{
			Track_MakeRandomTrack((TrackSectionType *)&theDocument->fTrackSectionList, MAX_TRACK_SECTIONS);
			Track_CreateMasterNubList((TrackSectionType *)&theDocument->fTrackSectionList,
									partCount,
									(PartType *)&theDocument->fPartsList,
									(NubEntryType *)&theDocument->fNubArray,
									&theDocument->fNumSplineNubs);	/* on output, new spline count */

			Track_CalcSplineCurve(&theDocument->fSplinePointsPtr,
								MAX_SPLINE_POINTS,
								(NubEntryType *)&theDocument->fNubArray,
								theDocument->fNumSplineNubs,
								&theDocument->fNumSplinePoints,
								kTrackSubDivFactor);

			Track_BuildCoasterGeometry_Mesh(kSkipValue,
											theDocument->fTrackGroup,
											theDocument->fNumSplinePoints,
											theDocument->fSplinePointsPtr);
		}

}
/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_DisposeDoc3DData                *
*                                                           *
*    PURPOSE:   Dispose of our QD3D objects                 *
*                                                           *
*                                                           *
*************************************************************/

void QD3DSupport_DisposeDoc3DData( DocumentPtr theDocument)
{
	TQ3Status status;

		Q3Object_Dispose(theDocument->fView) ;				/* the view for the scene */
		Q3Object_Dispose(theDocument->fCamera) ;			/* the camera for the scene */
		Q3Object_Dispose(theDocument->fTrackGroup) ;
		Q3Object_Dispose(theDocument->fGroundGroup) ;
		Q3Object_Dispose(theDocument->fInterpolation) ;		/* interpolation style used when rendering */
		Q3Object_Dispose(theDocument->fBackFacing) ;		/* whether to draw shapes that face away from the camera */
		Q3Object_Dispose(theDocument->fFillStyle) ;			/* whether drawn as solid filled object or decomposed to components */
		Q3Object_Dispose(theDocument->fTrackShader) ;			/* whether drawn as solid filled object or decomposed to components */

		status = Q3Exit();
		if ( status == kQ3Failure )
		{
			Utils_DisplayErrorMsg("QD3D Exit returned failure");
		}

}

/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_DocDraw3DData                   *
*                                                           *
*    PURPOSE:   Draws our track and ground group objects    *
*                                                           *
*                                                           *
*************************************************************/

TQ3Status QD3DSupport_DocDraw3DData( DocumentPtr theDocument )
{

	Q3View_StartRendering(theDocument->fView );
	do
	{		
		Q3Style_Submit( theDocument->fInterpolation, theDocument->fView );
		Q3Style_Submit( theDocument->fBackFacing, theDocument->fView );
		Q3Style_Submit( theDocument->fFillStyle, theDocument->fView );
		Q3DisplayGroup_Submit( theDocument->fTrackGroup, theDocument->fView );
		Q3DisplayGroup_Submit( theDocument->fGroundGroup, theDocument->fView );

	} while (Q3View_EndRendering(theDocument->fView) == kQ3ViewStatusRetraverse );

	Track_MoveCamera(theDocument->fCamera,
					theDocument->fSplinePointsPtr,
					theDocument->fNumSplinePoints,
					&theDocument->fTrackIndex);

	return kQ3Success ;
}


/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_QuickTimeInit                   *
*                                                           *
*    PURPOSE:   Initalize QuickTime                         *
*                                                           *
*                                                           *
*************************************************************/

static OSErr QD3DSupport_QuickTimeInit (void)
{
	#if TARGET_OS_WIN32
		OSErr err;

			err = InitializeQTML(0L);
			if (err != noErr)
			{

				return err;

			}
	#endif

	return ( EnterMovies () );
}



/************************************************************
*                                                           *
*    FUNCTION:  QD3DSupport_CreateShaderFromTexture         *
*                                                           *
*    PURPOSE:   Create our shader object                    *
*                                                           *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC

	static TQ3ShaderObject QD3DSupport_CreateShaderFromTexture(short grndTextureResourceID)
	{
		TQ3ShaderObject 	shaderObject = NULL;
		PicHandle			picH;
		Rect				picRect;

			Utils_Mac_GetPictForTexture(grndTextureResourceID, &picH, &picRect);
			if (picH == NULL)
			{
				Utils_DisplayFatalErrorMsg("Failure retrieving ground textures!");
			}
			
			shaderObject = TextureMap_Get(picH, &picRect);
			
			ReleaseResource((Handle)picH);
			
			return shaderObject;
	}
	
#else if TARGET_OS_WIN32

	static TQ3ShaderObject QD3DSupport_CreateShaderFromTexture(LPTSTR	grndTextureFilePath)
	{
		TQ3ShaderObject 	shaderObject = NULL;
		PicHandle			picH;
		Rect				picRect;
		ComponentResult		result;

		
			result = Utils_Win32_GetPicFromFile(grndTextureFilePath, &picH, &picRect);
			if (picH == NULL)
			{
				Utils_DisplayFatalErrorMsg("Failure retrieving ground textures!");
			}
			
			shaderObject = TextureMap_Get(picH, &picRect);
			
				/* release memory occupied by picture */
			KillPicture(picH);
			
			return shaderObject;
	}
	
#endif

