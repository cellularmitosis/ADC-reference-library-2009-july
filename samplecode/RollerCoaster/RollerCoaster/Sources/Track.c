/*
	File:		Track.c
	
	Contains:	Contains code to create our rollercoaster track
	
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

#include "Track.h"


/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

static float Track_AmountToRotateNubs(TrackSectionType	*trackSectionList,
									long 			numTrackSections,
									NubEntryType 	*coordPtr,
									long 			numNubsInPart,
									long 			sectionNum,
									float 			*scale);
static void Track_PutCameraOnTrack(TQ3CameraObject 	camera,
								NubEntryType 		*splinePtArray,
								long 				numSplinePoints,
								long 				curTrackLocation);

#if TARGET_OS_WIN32

	HANDLE WinIO_CreateFile();
	BOOL WinIO_WriteToFile(HANDLE fileHndl, PartType *thisPart);

#endif


/************************************************************
*                                                           *
*    FUNCTION:  Track_MakeRandomTrack                       *
*                                                           *
*    PURPOSE:   Generates a random series of track sections *
*                                                           *
*************************************************************/


void Track_MakeRandomTrack(TrackSectionType	*trackSectionList,
							long			numTrackSections)
{
	long	i,r;
		
		for (i = 0; i < numTrackSections; i++)
		{
			do
			{
				r = Utils_MyRandomLong() & numTrackSections;
			} while(r==7);
			
			trackSectionList[i].partNum = i;	
			trackSectionList[i].nubCoord.y = 0;
			trackSectionList[i].nubCoord.x = sin(6.24F/numTrackSections*(float)i)*(LAZY_SUSAN_RADIUS - 6.0F);
			trackSectionList[i].nubCoord.z = cos(6.24F/numTrackSections*(float)i)*(LAZY_SUSAN_RADIUS - 6.0F);
		}
				
}

/************************************************************
*                                                           *
*    FUNCTION:  Track_CreateMasterNubList                   *
*                                                           *
*    PURPOSE:   creates a master list of spline nubs which  *
*               contains all of the nubs within all         *
*               sections of a track                         *
*                                                           *
*************************************************************/

void Track_CreateMasterNubList(TrackSectionType *trackSectionList,
								unsigned long		numTrackSections,
								PartType 			*partsList,
								NubEntryType		*nubList,
								long				*nubTotal)
{
	unsigned long	sectionNum,partNum,i,numNubsInPart;
	NubEntryType	*coordPtr;
	TQ3Point3D	sectionStartCoords,basePt,upPt;
	float		rotation,scale;

		*nubTotal = 0;

		for (sectionNum=0; sectionNum < numTrackSections; sectionNum++)
		{
			partNum = trackSectionList[sectionNum].partNum;	/* get part # to add */

						/* GET INFO FOR PART */
		
			sectionStartCoords = trackSectionList[sectionNum].nubCoord;	/* get coords where track section should start */
			numNubsInPart = partsList[partNum].numNubs;		/* get # nubs in part */
			coordPtr = partsList[partNum].coordsPtr;			/* get ptr to coord list */

			rotation = Track_AmountToRotateNubs(trackSectionList,
												numTrackSections,
												coordPtr,
												numNubsInPart,
												sectionNum,
												&scale);

					/* COPY & ADJUST PART NUBS */
			
			for (i=1; i < (numNubsInPart-1); i++)				/* skip nub 0 & last nub */
			{

				basePt = coordPtr->basePt;						/* get coords from part data */
				upPt = coordPtr->upPt;
				
				Utils_RotatePoint(&basePt,rotation);					/* rotate the points into position */
				Utils_RotatePoint(&upPt,rotation);
				
				basePt.x *= scale;	basePt.y *= scale;	basePt.z *= scale;	/* scale it */
				upPt.x *= scale;	upPt.y *= scale;	upPt.z *= scale;
				
				nubList[*nubTotal].basePt.x = basePt.x + sectionStartCoords.x;	/* tag nubs to end of previous part */
				nubList[*nubTotal].basePt.y = basePt.y + sectionStartCoords.y;
				nubList[*nubTotal].basePt.z = basePt.z + sectionStartCoords.z;


				nubList[*nubTotal].upPt.x = upPt.x + sectionStartCoords.x;
				nubList[*nubTotal].upPt.y = upPt.y + sectionStartCoords.y;
				nubList[*nubTotal].upPt.z = upPt.z + sectionStartCoords.z;


				nubList[*nubTotal].sectionNum = sectionNum;			/* remember which section this belongs to */


				coordPtr++;
				(*nubTotal)++;
			}		
		}
		
			/* CREATE 3 FINAL NUBS WHICH WRAP BACK TO BEGINNING TO CLOSE THE LOOP */

		partNum = trackSectionList[0].partNum;								/* get part # */
		coordPtr = partsList[partNum].coordsPtr;							/* point back to beginning */
		sectionStartCoords = trackSectionList[0].nubCoord;					/* get coords where track section should start */
		numNubsInPart = partsList[partNum].numNubs;						/* get # nubs in part */

		rotation = Track_AmountToRotateNubs(trackSectionList,
											numTrackSections,
											coordPtr,
											numNubsInPart,
											0,
											&scale);

		for (i=0; i < 3; i++)
		{
			basePt = coordPtr->basePt;						/* get coords from part data */
			upPt = coordPtr->upPt;

			Utils_RotatePoint(&basePt,rotation);				/* rotate the points into position */
			Utils_RotatePoint(&upPt,rotation);

			basePt.x *= scale;	basePt.y *= scale;	basePt.z *= scale;	/* scale it */
			upPt.x *= scale;	upPt.y *= scale;	upPt.z *= scale;
			
			nubList[*nubTotal].basePt.x = basePt.x + sectionStartCoords.x;	/* tag nubs to end of previous part */
			nubList[*nubTotal].basePt.y = basePt.y + sectionStartCoords.y;
			nubList[*nubTotal].basePt.z = basePt.z + sectionStartCoords.z;


			nubList[*nubTotal].upPt.x = upPt.x + sectionStartCoords.x;
			nubList[*nubTotal].upPt.y = upPt.y + sectionStartCoords.y;
			nubList[*nubTotal].upPt.z = upPt.z + sectionStartCoords.z;


			(*nubTotal)++;
			coordPtr++;
		}

}

/************************************************************
*                                                           *
*    FUNCTION:  Track_AmountToRotateNubs                    *
*                                                           *
*    PURPOSE:   Calculates the amount to rotate nubs on the *
*               y axis so that the track section will go    *
*               from point A to point B. It also returns a  *
*               scaling value used to scale the nub based   *
*               on the change in length.                    *
*                                                           *
*************************************************************/

static float Track_AmountToRotateNubs(TrackSectionType	*trackSectionList,
										long 			numTrackSections,
										NubEntryType 	*coordPtr,
										long 			numNubsInPart,
										long 			sectionNum,
										float 			*scale)
{
	TQ3Vector3D	startVec,endVec,originalVec,desiredVec;
	long	i;
	float	rotation,originalSize,desiredSize;
	TQ3Point3D	originalPt,desiredPt,zeroPt = {0,0,0};


					/* CALC ORIGINAL VECTOR */


		startVec.x = coordPtr[1].basePt.x;					/* use 2nd nub as start coord (remember that we skip the 1st nub in a part) */
		startVec.y = coordPtr[1].basePt.y;
		startVec.z = coordPtr[1].basePt.z;
		endVec.x = coordPtr[numNubsInPart-1].basePt.x;		/* use last nub as end coord of original data */
		endVec.y = coordPtr[numNubsInPart-1].basePt.y;	
		endVec.z = coordPtr[numNubsInPart-1].basePt.z;	
		Q3Vector3D_Subtract(&endVec,&startVec,&originalVec); /* calc vector from start to end */

					/* CALC DESIRED VECTOR */

		i = sectionNum+1;
		if (i >= numTrackSections)
		{
			i = 0;
		}
		startVec.x = trackSectionList[sectionNum].nubCoord.x;	/* get vector of where we want it to endup */
		startVec.y = trackSectionList[sectionNum].nubCoord.y;				
		startVec.z = trackSectionList[sectionNum].nubCoord.z;				
		endVec.x = trackSectionList[i].nubCoord.x;				/* get vector of where we want it to endup */
		endVec.y = trackSectionList[i].nubCoord.y;				
		endVec.z = trackSectionList[i].nubCoord.z;				
		Q3Vector3D_Subtract(&endVec,&startVec,&desiredVec); /* calc vector from start to desired end */
		
					/* CALC ANGLE */

		rotation = Utils_AngleBetweenVectors(originalVec,desiredVec);	/* calc amount we need to rotate all nubs in part */
		if (desiredVec.z > originalVec.z)					/* see if need to negate (since we only have absolute angle between vecs) */
		{
			rotation = -rotation;
		}

			/* CALC SCALING TO ADJUST FOR DISTANCE CHANGE */

		originalPt.x = originalVec.x;					/* calc size of original segment */
		originalPt.y = originalVec.y;
		originalPt.z = originalVec.z;
		originalSize = Q3Point3D_Distance(&originalPt,&zeroPt);
		
		desiredPt.x = desiredVec.x;						/* calc size of desired seg */
		desiredPt.y = desiredVec.y;
		desiredPt.z = desiredVec.z;
		desiredSize = Q3Point3D_Distance(&desiredPt,&zeroPt);
			
		*scale = desiredSize/originalSize;				/* return scaling value */
			
		return(rotation);
}


#if TARGET_OS_WIN32

/************************************************************
*                                                           *
*    FUNCTION:  Track_LoadPartsFromFile                     *
*                                                           *
*    PURPOSE:   Loads our pre-generated track parts         *
*               from a file                                 *
*                                                           *
*************************************************************/

void Track_LoadPartsFromFile(PartType *partsList, short *partCount)

{
	HANDLE		fileHndl;
	DWORD		err;
	char		partFilePath[MAX_PATH];


		err = Utils_Win32_BuildCurDirPath((Ptr)&partFilePath, kPartDataFileName);
		fileHndl = CreateFile((char *)&partFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if ((fileHndl == NULL) || (fileHndl == INVALID_HANDLE_VALUE))

		{

			Utils_DisplayFatalErrorMsg("Failure loading track part file TrackPartData.dat!");

		}

		else

		{
			Ptr		currentIndex;
			HLOCAL	localMem;
			DWORD	numBytesRead, fileSize;
			BOOL	successfull = FALSE;


				fileSize = GetFileSize(fileHndl, NULL);
				localMem = LocalAlloc(LMEM_FIXED, fileSize);
				if (localMem != NULL)
				{
					currentIndex = localMem;
					*partCount = 0;
					successfull = FALSE;


					do
					{
							/* get count of nubs for this part */

						successfull = ReadFile(fileHndl, &partsList[*partCount].numNubs, sizeof(partsList->numNubs), &numBytesRead, NULL);
						if ((successfull == TRUE) && (numBytesRead != 0))
						{
							numBytesRead = 0;
							successfull = ReadFile(fileHndl, currentIndex, sizeof(NubEntryType) * partsList[*partCount].numNubs, &numBytesRead, NULL);
							if ((successfull == TRUE) && (numBytesRead != 0))
							{
									/* SAVE PART RECORD */
								partsList[*partCount].coordsPtr = (NubEntryType *)currentIndex;
									/* move pointer to point to memory for next part */
								currentIndex = currentIndex + sizeof(NubEntryType) * partsList[*partCount].numNubs;
								++ (*partCount);
							}
							else
							{
								Utils_DisplayFatalErrorMsg("Error reading track part file TrackPartData.dat!");

							}
						}
					}
					while ((successfull == TRUE) && (numBytesRead != 0));
				}
				else
				{
					Utils_DisplayFatalErrorMsg("Memory allocation failure!");

				}
		}
}


#endif




/************************************************************
*                                                           *
*    FUNCTION:  Track_LoadPartsFromRez                      *
*                                                           *
*    PURPOSE:   Loads our pre-generated track parts         *
*               from a resource file                        *
*                                                           *
*************************************************************/


#if TARGET_OS_MAC


void Track_LoadPartsFromRez(PartType *partsList, short *partCount)
{
OSErr		err;
short		i;
Handle		partHandle;
PartType	*thisPart;
Byte		numNubs;
Ptr			nubCoordsPtr;


	*partCount = 0;

		/* get count of part resources in our resource file */
	*partCount = Count1Resources(kPartType);
	err = ResError();
	if (err != noErr)
	{
		Utils_DisplayFatalErrorMsg("Error Getting Part Resource");
	}
	else
	{
		for (i = 1; i <= *partCount; i++)
		{
				/* get next part resource in our file */
			partHandle = GetIndResource(kPartType,i);
			if (partHandle == nil)
			{
				Utils_DisplayFatalErrorMsg("Error Getting Part Resource");
			}
			else
			{
				DetachResource(partHandle);			/* give it to me */
				HLockHi(partHandle);

				thisPart = (PartType *)(*partHandle);

					/* get number of points in this part from the numNubs field of
						the PartType structure */
						
				numNubs = thisPart->numNubs;

					/* allocate enough NubEntryType structures to hold all of the points
						in this part */

				nubCoordsPtr = NewPtr(sizeof(NubEntryType) * numNubs);// alloc memory for nub coords
				if (nubCoordsPtr == nil)
				{
					Utils_DisplayFatalErrorMsg("Not enough memory to load Part");
				}
				else
				{
						/* copy NubEntryType data to our memory block */
					//BlockMove((Ptr)&thisPart->coordsPtr,nubCoordsPtr,sizeof(NubEntryType) * numNubs);
					BlockMove((Ptr)thisPart + sizeof(thisPart->numNubs),nubCoordsPtr,sizeof(NubEntryType) * numNubs);
					DisposeHandle(partHandle);
					
						/* save part record as a PartType structure */
					partsList[i-1].numNubs = numNubs;
					partsList[i-1].coordsPtr = (NubEntryType *)nubCoordsPtr;

				}
			}
		}
	}
}


#endif


/************************************************************
*                                                           *
*    FUNCTION:  Track_PutCameraOnTrack                      *
*                                                           *
*    PURPOSE:   Calculates placement of the camera on the   *
*               track                                       *
*                                                           *
*                                                           *
*                                                           *
*************************************************************/

void Track_PutCameraOnTrack(TQ3CameraObject 	camera,
							NubEntryType 		*splinePtArray,
							long 				numSplinePoints,
							long 				curTrackLocation
							)
{
	TQ3Point3D			cameraFrom,cameraTo;
	TQ3Vector3D			cameraUp,cameraFromVect;
	long				desti;
	TQ3CameraPlacement	placement;
		
			
		if (numSplinePoints == 0)
			return;

						
					/* CALC UP VECTOR */
					
		Q3Point3D_Subtract(&splinePtArray[curTrackLocation].upPt,		/* calc vector from base to up point */
							&splinePtArray[curTrackLocation].basePt,
							&cameraUp);
		Q3Vector3D_Normalize(&cameraUp,&cameraUp);
		

			/* USE UP VECTOR TO CALC "FROM" POSITION */
			
		Q3Vector3D_Scale(&cameraUp,DISTANCE_FROM_TRACK_TO_CAMERA,&cameraFromVect);	/* calc how far above track to put camera */
		cameraFrom.x = (splinePtArray[curTrackLocation].basePt.x +
						 cameraFromVect.x);
		cameraFrom.y = (splinePtArray[curTrackLocation].basePt.y +
						 cameraFromVect.y);
		cameraFrom.z = (splinePtArray[curTrackLocation].basePt.z +
						 cameraFromVect.z);
		
					/* SET "TO" PT */
										
		desti = curTrackLocation + kSkipAheadPoints;
		if (desti >= (numSplinePoints-1))
		{
			desti -= numSplinePoints;		/* loop back around */
		}

		cameraTo = splinePtArray[desti].basePt;			/* look at base spline point in distance */
				
							
					/* UPDATE CAMERA PLACEMENT */
					
		placement.cameraLocation = cameraFrom;				/* set placement data */
		placement.pointOfInterest = cameraTo;
		placement.upVector = cameraUp;
		Q3Camera_SetPlacement(camera,&placement);
	
}


/************************************************************
*                                                           *
*    FUNCTION:  Track_BuildCoasterGeometry_Mesh_Textured    *
*                                                           *
*    PURPOSE:   Build a textured track using mesh objects   *
*                                                           *
*                                                           *
*************************************************************/

void Track_BuildCoasterGeometry_Mesh(long 			skipValue,
									TQ3GroupObject 	theGroup,
									long 			numSplinePoints,
									NubEntryType 	*splinePointsPtr)
{
TQ3GeometryObject	myMesh = NULL;
TQ3GroupPosition	myGroupPosition;
TQ3ColorRGB			whiteColor = {1,1,1};
long				i,faceCount,colorTick = 0;
float				x2,y2,z2;
TQ3Vector3D			tangentVector;	
TQ3MeshVertex		meshVertexList[4];
TQ3Vertex3D			vertexList[4],firstLeft,firstRight;
TQ3AttributeSet		generalAttribs = NULL,vAttrib[4] = {NULL, NULL, NULL, NULL};
TQ3MeshFace			face;
float				ambient = 0.8F;
float				spec = 0.0F;
TQ3Param2D			corner1 = {0,1};	/* uv's for texture mapping */
TQ3Param2D			corner2 = {1,1};
TQ3Param2D			corner3 = {1,0};
TQ3Param2D			corner4 = {0,0};

	

				/* CREATE NEW MESH OBJECT */
	
	myMesh = Q3Mesh_New();
	
	if( myMesh == NULL )
	{
		Utils_DisplayErrorMsg("Group_AddObject failed!");
		goto memoryError;
	}
								
	Q3Mesh_DelayUpdates(myMesh);


				/* SETUP GENERAL ATTRIBUTES */

	generalAttribs = Q3AttributeSet_New();	
	if( generalAttribs == NULL )
	{
		Utils_DisplayErrorMsg("Q3AttributeSet_New failed!");
		goto memoryError;
	}
	Q3AttributeSet_Add(generalAttribs, kQ3AttributeTypeAmbientCoefficient, &ambient);
	Q3AttributeSet_Add(generalAttribs, kQ3AttributeTypeSpecularControl, &spec);	


			/* CREATE UV ATTRIBUTES FOR THE VERTICES */

	vAttrib[0] = Q3AttributeSet_New();	
	if( vAttrib[0] == NULL )
	{
		Utils_DisplayErrorMsg("Q3AttributeSet_New failed!");
		goto memoryError;
	}
	Q3AttributeSet_Add(vAttrib[0], kQ3AttributeTypeShadingUV, &corner1);	

	vAttrib[1] = Q3AttributeSet_New();	
	if( vAttrib[1] == NULL )
	{
		Utils_DisplayErrorMsg("Q3AttributeSet_New failed!");
		goto memoryError;
	}
	Q3AttributeSet_Add(vAttrib[1], kQ3AttributeTypeShadingUV, &corner2);	

	vAttrib[2] = Q3AttributeSet_New();	
	if( vAttrib[2] == NULL )
	{
		Utils_DisplayErrorMsg("Q3AttributeSet_New failed!");
		goto memoryError;
	}
	Q3AttributeSet_Add(vAttrib[2], kQ3AttributeTypeShadingUV, &corner3);	

	vAttrib[3] = Q3AttributeSet_New();	
	if( vAttrib[3] == NULL )
	{
		Utils_DisplayErrorMsg("Q3AttributeSet_New failed!");
		goto memoryError;
	}
	Q3AttributeSet_Add(vAttrib[3], kQ3AttributeTypeShadingUV, &corner4);	


	vertexList[0].attributeSet = nil;
	vertexList[1].attributeSet = nil;
	vertexList[2].attributeSet = nil;
	vertexList[3].attributeSet = nil;

	faceCount = 0;


	for (i = 0; i < (numSplinePoints - 1 - skipValue); i += skipValue)
	{		
				/* GET SPLINE POINTS */
				
		x2 = splinePointsPtr[i+skipValue].basePt.x;		/* get coords of end pt #2 (far point) */
		y2 = splinePointsPtr[i+skipValue].basePt.y;
		z2 = splinePointsPtr[i+skipValue].basePt.z;


				/* CALC TANGENT VECTOR */

		Q3Point3D_CrossProductTri(&splinePointsPtr[i].basePt,
								&splinePointsPtr[i+skipValue].basePt,
								&splinePointsPtr[i].upPt,&tangentVector);

		Q3Vector3D_Normalize(&tangentVector,&tangentVector);
		Q3Vector3D_Scale(&tangentVector,kTrackWidth,&tangentVector);


				/* CALC NEW "UPPER" COORDS OF FACE */

		Q3Point3D_Set(&vertexList[0].point,			/* upper left */
						x2-tangentVector.x,
						y2-tangentVector.y,
						z2-tangentVector.z);

		Q3Point3D_Set(&vertexList[1].point,			/* upper right */
						x2+tangentVector.x,
						y2+tangentVector.y,
						z2+tangentVector.z);


		meshVertexList[0] = Q3Mesh_VertexNew(myMesh, &vertexList[0]);		/* get new vertex for "upper/far" */
		meshVertexList[1] = Q3Mesh_VertexNew(myMesh, &vertexList[1]);


					/* FOR 1ST FACE, MUST RIG "BOTTOM" COORDS */
					
		if (i == 0)
		{
			Q3Point3D_Set(&vertexList[2].point,			/* lower right */
							splinePointsPtr[0].basePt.x+tangentVector.x,
							splinePointsPtr[0].basePt.y+tangentVector.y,
							splinePointsPtr[0].basePt.z+tangentVector.z);

			Q3Point3D_Set(&vertexList[3].point,			/* lower left */
							splinePointsPtr[0].basePt.x-tangentVector.x,
							splinePointsPtr[0].basePt.y-tangentVector.y,
							splinePointsPtr[0].basePt.z-tangentVector.z);

			firstRight.point = vertexList[2].point;			/* remember these coords for wrap-back later */
			firstLeft.point = vertexList[3].point;
			
			meshVertexList[2] = Q3Mesh_VertexNew(myMesh, &vertexList[2]);	/* set vertex */
			meshVertexList[3] = Q3Mesh_VertexNew(myMesh, &vertexList[3]);
		}

		
					/* CREATE FACE */
					
		face = Q3Mesh_FaceNew(myMesh,4,&meshVertexList[0],generalAttribs);

		if( face == NULL )
		{
			Utils_DisplayErrorMsg("Q3AttributeSet_New failed!");
			goto memoryError;
		}

					/* APPLY UV COORD ATTRIBS */
					
		Q3Mesh_SetCornerAttributeSet(myMesh, meshVertexList[0], face, vAttrib[0]);
		Q3Mesh_SetCornerAttributeSet(myMesh, meshVertexList[1], face, vAttrib[1]);
		Q3Mesh_SetCornerAttributeSet(myMesh, meshVertexList[2], face, vAttrib[2]);
		Q3Mesh_SetCornerAttributeSet(myMesh, meshVertexList[3], face, vAttrib[3]);

		colorTick++;		
		faceCount++;
		
			/* SEE IF MESH OBJECT IS LARGE ENOUGH TO USE NOW */
		
		if (faceCount > kMaxFacesInMesh)
		{
			Q3Mesh_ResumeUpdates(myMesh);

			myGroupPosition = Q3Group_AddObject(theGroup, myMesh);	/* add mesh to group */
			Q3Object_Dispose(myMesh);									/* make another mesh object */
			if ( myGroupPosition == nil )
			{
				Utils_DisplayErrorMsg("Q3Group_AddObject failed!");
				goto memoryError;
			}
			
			myMesh = Q3Mesh_New();
			if ( myMesh == nil )
			{
				Utils_DisplayErrorMsg("Q3Mesh_New failed!");
				goto memoryError;
			}
			Q3Mesh_DelayUpdates(myMesh);
			meshVertexList[0] = Q3Mesh_VertexNew(myMesh, &vertexList[0]);	/* reset these to the new mesh */
			meshVertexList[1] = Q3Mesh_VertexNew(myMesh, &vertexList[1]);
			
			faceCount = 0;		
		}	
	
				/* UPPERS WILL BE LOWERS ON NEXT POLY */
	
		vertexList[2] = vertexList[1];				/* this is so next poly's bottom will match last poly's top */
		vertexList[3] = vertexList[0];
		meshVertexList[2] = meshVertexList[1];
		meshVertexList[3] = meshVertexList[0];		

		if (colorTick & 1)
		{
			vertexList[0].attributeSet = vAttrib[0];			/* apply uv attribs to the vertices */
			vertexList[1].attributeSet = vAttrib[1];
			vertexList[2].attributeSet = vAttrib[2];
			vertexList[3].attributeSet = vAttrib[3];
		}
		else
		{
			vertexList[0].attributeSet = vAttrib[3];			/* apply uv attribs to the vertices */
			vertexList[1].attributeSet = vAttrib[2];
			vertexList[2].attributeSet = vAttrib[1];
			vertexList[3].attributeSet = vAttrib[0];
		}
	}

				/* CREATE 1 FINAL FACE TO LINK BACK TO BEGINNING */

	vertexList[0].point = firstLeft.point;
	vertexList[1].point = firstRight.point;
	meshVertexList[0] = Q3Mesh_VertexNew(myMesh, &vertexList[0]);		/* create vertex based on 1st */
	meshVertexList[1] = Q3Mesh_VertexNew(myMesh, &vertexList[1]);
	
	if (faceCount & 1)
	{
		face = Q3Mesh_FaceNew(myMesh,4,&meshVertexList[0],nil);
	}
	else
	{
		face = Q3Mesh_FaceNew(myMesh,4,&meshVertexList[0],nil);
	}
	faceCount++;

					/* APPLY ANY REMAINING MESH */
					
	Q3Mesh_ResumeUpdates(myMesh);

	if (faceCount > 0)
	{
		myGroupPosition = Q3Group_AddObject(theGroup, myMesh);
		if ( myGroupPosition == nil )
		{
			Utils_DisplayErrorMsg("Q3Group_AddObject failed!");
			goto memoryError;
		}
	}

	Q3Object_Dispose(myMesh);							/* kill mesh object */
	Q3Object_Dispose(generalAttribs);	
	Q3Object_Dispose(vAttrib[0]);	
	Q3Object_Dispose(vAttrib[1]);	
	Q3Object_Dispose(vAttrib[2]);	
	Q3Object_Dispose(vAttrib[3]);
	
	return;
	
memoryError:
	if( myMesh )
	{
		Q3Object_Dispose(myMesh);
	}

	if( generalAttribs )
	{
		Q3Object_Dispose(generalAttribs);
	}

	if( vAttrib[0] )
	{
		Q3Object_Dispose(vAttrib[0]);
	}

	if( vAttrib[1] )
	{
		Q3Object_Dispose(vAttrib[1]);
	}

	if( vAttrib[2] )
	{
		Q3Object_Dispose(vAttrib[2]);
	}

	if( vAttrib[3] )
	{
		Q3Object_Dispose(vAttrib[3]);
	}
	
}


/************************************************************
*                                                           *
*    FUNCTION:  Track_CalcSplineCurve                       *
*                                                           *
*    PURPOSE:   Use our control points to construct a       *
*               spline curve for each section of the        *
*               track                                       *
*                                                           *
*                                                           *
*************************************************************/

void Track_CalcSplineCurve(NubEntryType 	**splinePoints,
							long			maxSplinePoints,
							NubEntryType	*nubPoints,
							long			numSplineNubs,
							long			*numSplinePoints,
							float 			subDivFactor)
{
	float		t,tSquared,tCubed,a,b,c,d,incVal;
	long		subCount,nubNum,numNubs,numSubDivs;
	TQ3Point3D	highestPoint = {0,0,0};

				/* ALLOC MEMORY FOR SPLINE DATA */

		*splinePoints = (NubEntryType *)NewPtr( sizeof(NubEntryType) * maxSplinePoints );
		if (*splinePoints == nil)
		{
			Utils_DisplayFatalErrorMsg("Sorry, but there doesnt seem to be enough memory to allocate the track spline curve");
		}
		else
		{

			numNubs = numSplineNubs;	/* # nubs */
			*numSplinePoints = 0;		/* # points generated */
			
						/* SCAN THRU NUBS */

				/* Note: skips 1st & last nubs (sorta) */
			for (nubNum=1; nubNum < (numNubs-2); nubNum++)
			{				
				numSubDivs = Q3Point3D_Distance(&nubPoints[nubNum].basePt,
												&nubPoints[nubNum+1].basePt)*subDivFactor + 0.5F;	/* # segments between next nubs */
				if (numSubDivs < 1)
					numSubDivs = 1;
				
				incVal = 1.0F/numSubDivs;				/* increment value */

				for (t=0, subCount=0; subCount < numSubDivs; t+=incVal,subCount++)
				{
					tSquared = t*t;
					tCubed = tSquared*t;
					a = (-0.166F * tCubed)+(0.5F * tSquared)-(0.5F * t) + 0.166F;
					b = (0.5F * tCubed) - tSquared + 0.666F;
					c = (-0.5F * tCubed) + (0.5F * tSquared) + (0.5F * t + 0.166F);
					d = 0.166F * tCubed;

					if (*numSplinePoints < maxSplinePoints)
					{	
										/* CALC SEG OF BASE */
										
						(*splinePoints)[*numSplinePoints].basePt.x =
											(a * nubPoints[nubNum-1].basePt.x) +
											(b * nubPoints[nubNum].basePt.x) +
											(c * nubPoints[nubNum+1].basePt.x) +
											(d * nubPoints[nubNum+2].basePt.x);
										
						(*splinePoints)[*numSplinePoints].basePt.y =
											(a * nubPoints[nubNum-1].basePt.y) + 
											(b * nubPoints[nubNum].basePt.y) +
											(c * nubPoints[nubNum+1].basePt.y) +
											(d * nubPoints[nubNum+2].basePt.y);

						(*splinePoints)[*numSplinePoints].basePt.z =
											(a * nubPoints[nubNum-1].basePt.z) +
											(b * nubPoints[nubNum].basePt.z) +
											(c * nubPoints[nubNum+1].basePt.z) +
											(d * nubPoints[nubNum+2].basePt.z);

										/* CALC SEG OF UP */
										
						(*splinePoints)[*numSplinePoints].upPt.x =
											(a * nubPoints[nubNum-1].upPt.x) +
											(b * nubPoints[nubNum].upPt.x) +
											(c * nubPoints[nubNum+1].upPt.x) +
											(d * nubPoints[nubNum+2].upPt.x);
										
						(*splinePoints)[*numSplinePoints].upPt.y =
											(a * nubPoints[nubNum-1].upPt.y) + 
											(b * nubPoints[nubNum].upPt.y) +
											(c * nubPoints[nubNum+1].upPt.y) +
											(d * nubPoints[nubNum+2].upPt.y);

						(*splinePoints)[*numSplinePoints].upPt.z =
											(a * nubPoints[nubNum-1].upPt.z) +
											(b * nubPoints[nubNum].upPt.z) +
											(c * nubPoints[nubNum+1].upPt.z) +
											(d * nubPoints[nubNum+2].upPt.z);

										/* REMEMBER WHAT TYPE IT IS */
										
						(*splinePoints)[*numSplinePoints].sectionNum = nubPoints[nubNum].sectionNum;

						(*numSplinePoints)++;
					}
					else
					{
						Utils_DisplayFatalErrorMsg("Too many spline points!  Overflowed array!");
					}
				}
			}
		}
}

/************************************************************
*                                                           *
*    FUNCTION:  Track_MoveCamera                            *
*                                                           *
*    PURPOSE:   Move the camera to the next location on the *
*               track                                       *
*                                                           *
*                                                           *
*************************************************************/

void Track_MoveCamera(TQ3CameraObject 	camera,
					NubEntryType 		*splinePtArray,
					long 				numSplinePoints,
					long 				*curTrackLocation)
{
	Track_PutCameraOnTrack(camera,
							splinePtArray,
							numSplinePoints,
							*curTrackLocation);
		/* have we reached the end of the track? */
	if ( ((*curTrackLocation) + 1) >= numSplinePoints)
	{
			/* end-of-track, so wrap back to the beginning */
		(*curTrackLocation) = 0;
	}
	else
	{
			/* move to next location on track */
		++(*curTrackLocation);
	}

}


/************************************************************
*                                                           *
*    FUNCTION:  Track_GetForwardVector                      *
*                                                           *
*    PURPOSE:   Returns the vector representing forward at  *
*               trackIndex                                  *
*                                                           *
*                                                           *
*************************************************************/

void Track_GetForwardVector(long trackIndex, NubEntryType *splinePointsPtr, long numSplinePoints, TQ3Vector3D *theVector)
{
TQ3Vector3D	forward;

	if ((trackIndex+1) == numSplinePoints)					/* see if +1 will wrap it */
		Q3Point3D_Subtract(&splinePointsPtr[0].basePt,		/* calc vector from here to one in front */
						&splinePointsPtr[trackIndex].basePt,
						&forward);
	else
		Q3Point3D_Subtract(&splinePointsPtr[trackIndex+1].basePt,		/* calc vector from here to one in front */
						&splinePointsPtr[trackIndex].basePt,
						&forward);
	Q3Vector3D_Normalize(&forward,theVector);					/* normalize & return it */
}


/************************************************************
*                                                           *
*    FUNCTION:  Track_GetNormalVector                       *
*                                                           *
*    PURPOSE:   Returns the vector to the normal of the     *
*               surface of the track at trackIndex          *
*                                                           *
*                                                           *
*************************************************************/

void Track_GetNormalVector(NubEntryType *splinePointsPtr, long trackIndex, TQ3Vector3D *theVector)
{
TQ3Vector3D	cameraUp;

	Q3Point3D_Subtract(&splinePointsPtr[trackIndex].upPt,		/* calc vector from base to up point */
						&splinePointsPtr[trackIndex].basePt,
						&cameraUp);
	Q3Vector3D_Normalize(&cameraUp,theVector);					/* normalize & return it */
}

