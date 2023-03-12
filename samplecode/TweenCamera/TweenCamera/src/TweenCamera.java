/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
import java.io.*;

import quicktime.*;
import quicktime.io.*;
import quicktime.std.*;
import quicktime.util.*;
import quicktime.qd.*;
import quicktime.qd3d.camera.*;
import quicktime.qd3d.math.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.app.QTFactory;

public class TweenCamera implements StdQTConstants {
	public static void main (String args[]) {
		try {
			System.out.println ("This will open the jet.mov and add a tween track to it, saving the result to the original movie");
			QTSession.open();
			makeMovie ();
			System.out.println ("Finished");
		}
		catch (Exception e) {
			e.printStackTrace();
		} finally {
			QTSession.close();
		}
	}
		
	private static void makeMovie () throws Exception {
		// Get the 3d movie file from the user
		QTFile movieFile = new QTFile (QTFactory.findAbsolutePath ("jet.mov"));
		System.out.println (movieFile);
		OpenMovieFile instream = OpenMovieFile.asRead(movieFile);
		Movie myMovie = Movie.fromFile(instream);
		instream.close();

		// get the 3d track and media from the movie
		Track threeDTrack = myMovie.getIndTrack(1);
		ThreeDMedia threeDMedia = (ThreeDMedia) Media.getTrackMedia(threeDTrack);

		// make a tween track and media
		Track tweenTrack = myMovie.addTrack(0, 0, 0);
		TweenMedia tweenMedia = new TweenMedia(tweenTrack, threeDMedia.getTimeScale());

		// make empty tween sample
		AtomContainer tweenSample = new AtomContainer();
		
		// Construct shared data for the two camera descriptions
		Point3D cameraTo = new Point3D (0.0F, 0.0F, 0.0F);
		Vector3D cameraUp = new Vector3D (0.0F, 1.0F, 0.0F);
		
		float hither = 0.001F;
		float yon = 1000.0F;
		CameraRange range = new CameraRange(hither, yon);
		
		QDPoint origin = new QDPoint (-1.0F, 1.0F);
		float width = 2.0F;
		float height = 2.0F;
		CameraViewPort cameraView = new CameraViewPort (origin, width, height);
		

//**	addTweenEntryToSample ...	sets the type of the tween 	
			Point3D cameraFrom = new Point3D(0.0F, 0.0F, 30.0F);
			CameraData tweenCamera = new CameraData(new CameraPlacement(cameraFrom, cameraTo, cameraUp), range, cameraView);

			// create entry for this tween in the sample
			Atom tweenAtom = tweenSample.insertChild (new Atom(kParentAtomIsContainer), kTweenEntry, 1, 0);

			// define the type of this tween entry
			tweenSample.insertChild(tweenAtom, kTweenType, 1, 0, EndianOrder.flipNativeToBigEndian32(kTweenType3dCameraData));
			
			//Endian flip tweenCamera in place
			EndianOrder.flipNativeToBigEndian (tweenCamera, 0, CameraData.getEndianDescriptor());
			
			// define the 'flipped' data for this tween entry
			tweenSample.insertChild(tweenAtom, kTweenData, 1, 0, tweenCamera);

//**	setTweenEntryInitialConditions
		// Set camera initial conditions => the only difference is the initialLocation
		// but because the camera data may have been flipped we create a clean copy
		// we could have cloned this and then flipped it but this way we are explicit about dealing with 
		// two different camera stuctures (even though they share values)
		
			// modify coordinates
			Point3D initialLocation = new Point3D (0, 0, 150.0F);
			CameraData initialCamera = new CameraData(new CameraPlacement(initialLocation, cameraTo, cameraUp), range, cameraView);

			// look up the tween entry
			Atom initialTweenAtom = tweenSample.findChildByID_Atom (new Atom(kParentAtomIsContainer), kTweenEntry, 1);

			//Endian flip tweenCamera in place
			EndianOrder.flipNativeToBigEndian (initialCamera, 0, CameraData.getEndianDescriptor());

			// add in the intial 'flipped' data
			tweenSample.insertChild (initialTweenAtom, kTween3dInitialCondition, 1, 0, initialCamera); 

//**	addTweenEntryToInputMapEntry
			// make up tween sample description
			ThreeDDescription sampleDescription = new ThreeDDescription(0);
			
			// add the tween sample to the tween media
			tweenMedia.beginEdits();
			tweenMedia.addSample(tweenSample, 0, tweenSample.getSize(), threeDMedia.getDuration(),
													 sampleDescription, 1, 0);
			tweenMedia.endEdits();
			
			// add the tween media into the track
			tweenTrack.insertMedia(0, 0, tweenMedia.getDuration(), 1);

			// create the reference between the 3d and tween tracks
			int referenceIndex1 = threeDTrack.addReference(tweenTrack, kTrackModifierReference);
			
			// create input map for 3d track
			AtomContainer inputMap = new AtomContainer();

			// add an input entry to the input map
			Atom inputAtom = inputMap.insertChild (new Atom(kParentAtomIsContainer), kTrackModifierInput, referenceIndex1, 0);
			
			// set the type of the modifier input
			inputMap.insertChild(inputAtom, kTrackModifierType, 1, 0, EndianOrder.flipNativeToBigEndian32(kTrackModifierCameraData));
			
			// set the sub input id (the id of the tween entry)
			inputMap.insertChild(inputAtom, kInputMapSubInputID, 1, 0, EndianOrder.flipNativeToBigEndian32(1));

//** attach the input map to the 3d media handler
			threeDMedia.setInputMap(inputMap);

//** save the newly created movie
			OpenMovieFile outstream = OpenMovieFile.asWrite(movieFile);
			myMovie.updateResource(outstream, movieInDataForkResID, movieFile.getName());
			outstream.close();
	}
}
