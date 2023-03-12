/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */

import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.qd.*;
import quicktime.*;
import quicktime.std.*;
import quicktime.io.*;
import quicktime.sound.*;
import quicktime.std.image.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.util.*;

import createmovies.*;

import quicktime.app.display.*;
import quicktime.app.image.*;
import quicktime.app.QTFactory;
/**
 * 	CreateMovie Demo.
 */
 
public class CreateMovie extends Frame implements StdQTConstants, Errors {
	static CreateMovie movieFrame;

//_________________________ CLASS METHODS
	public static void main(String args[]) {
		try {
			QTSession.open();

			movieFrame = new CreateMovie("Movie Creation Demo using QuickTime and Java");
			movieFrame.addWindowListener (new WindowAdapter () {
				public void windowOpened (WindowEvent we) {
					movieFrame.makeMovie();
				}
				public void windowClosing (WindowEvent e) {
					movieFrame.canv.removeClient();
					QTSession.close();
					movieFrame.dispose();
				}
				public void windowClosed (WindowEvent e) { 
					System.exit(0);
				}
			});
			movieFrame.show();
			movieFrame.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	public CreateMovie (String frameTitle) throws Exception {
		super(frameTitle);
		soundFile = QTFactory.findAbsolutePath ("sound.aif");

		canv = new QTCanvas (QTCanvas.kInitialSize, 0.5F, 0.5F);
		add ("Center", canv);

		np = new NumberPainter(numFrames);
		qid = new QTImageDrawer (np, new Dimension (kWidth, kHeight), Redrawable.kMultiFrame);
		qid.setRedrawing(true);

		canv.setClient (qid, true);
	
		pack();
	}		

//_________________________ INSTANCE VARIABLES
	private QTCanvas canv;
	private QTImageDrawer qid;
	private NumberPainter np;
	private static final int numFrames = 30;
	private int kWidth = 330;
	private int kHeight = 140;
	private File soundFile;
	
//_________________________ INSTANCE METHODS
	private void makeMovie() {
		try {		
			//
			// show save-as dialog, create movie file & empty movie
			//
			FileDialog fd = new FileDialog (this, "Save Movie As...", FileDialog.SAVE);
			fd.show();
			if(fd.getFile() == null)
				throw new QTIOException (userCanceledErr, "");
			QTFile f = new QTFile(fd.getDirectory() + fd.getFile());
			Movie theMovie = Movie.createMovieFile (f,
								kMoviePlayer, 
								createMovieFileDeleteCurFile | createMovieFileDontCreateResFile);

			//
			// add content
			//
			System.out.println ("Doing Video Track");
			addVideoTrack( theMovie );
			System.out.println ("Doing Audio Track");
			addAudioTrack( theMovie );

			//
			// save movie to file
			//
			OpenMovieFile outStream = OpenMovieFile.asWrite (f); 
			theMovie.addResource( outStream, movieInDataForkResID, f.getName() );
			outStream.close();
			System.out.println ("Finished movie");
		}
		catch (Exception qte) {
			qte.printStackTrace(); 
		}
	}
	
	private void addVideoTrack (Movie theMovie) throws QTException {
		int kNoVolume	= 0;
		int kVidTimeScale = 600;

		Track vidTrack = theMovie.addTrack (kWidth, kHeight, kNoVolume);
		VideoMedia vidMedia = new VideoMedia (vidTrack, kVidTimeScale);  
								
		vidMedia.beginEdits();
		addVideoSample (vidMedia);
		vidMedia.endEdits();
		
		int kTrackStart	= 0;
		int kMediaTime 	= 0;
		int kMediaRate	= 1;
		vidTrack.insertMedia (kTrackStart, kMediaTime, vidMedia.getDuration(), kMediaRate);
	}
	
	private void addVideoSample( VideoMedia vidMedia ) throws QTException {
		QDRect rect = new QDRect (kWidth, kHeight);
		QDGraphics gw = new QDGraphics (rect);
		int size = QTImage.getMaxCompressionSize (gw, 
												rect, 
												gw.getPixMap().getPixelSize(),
	                                        	codecNormalQuality, 
	                                        	kAnimationCodecType, 
	                                        	CodecComponent.anyCodec);
		QTHandle imageHandle = new QTHandle (size, true);
		imageHandle.lock();
		RawEncodedImage compressedImage = RawEncodedImage.fromQTHandle(imageHandle);
		CSequence seq = new CSequence (gw,
										rect, 
										gw.getPixMap().getPixelSize(),
										kAnimationCodecType, 
										CodecComponent.bestFidelityCodec,
										codecNormalQuality, 
										codecNormalQuality, 
										numFrames,	//1 key frame
										null, //cTab,
										0);
		ImageDescription desc = seq.getDescription();

 //redraw first...
       	np.setCurrentFrame (1);
		qid.redraw(null);

		qid.setGWorld (gw);
		qid.setDisplayBounds (rect);
			
		for (int curSample = 1; curSample <= numFrames; curSample++) {
	       	np.setCurrentFrame (curSample);
			qid.redraw(null);
			CompressedFrameInfo info = seq.compressFrame (gw, 
														rect, 
														codecFlagUpdatePrevious, 
														compressedImage);
 			boolean isKeyFrame = info.getSimilarity() == 0;
 			System.out.println ("f#:" + curSample + ",kf=" + isKeyFrame + ",sim=" + info.getSimilarity());
 			vidMedia.addSample (imageHandle, 
									0, // dataOffset,
									info.getDataSize(),
									60, // frameDuration, 60/600 = 1/10 of a second, desired time per frame	
									desc,
									1, // one sample
									(isKeyFrame ? 0 : mediaSampleNotSync)); // no flags
       }
		
 	//print out ImageDescription for the last video media data ->
 	//this has a sample count of 1 because we add each "frame" as an individual media sample
 		System.out.println (desc);

 	//redraw after finishing...
		qid.setGWorld (canv.getPort());
       	np.setCurrentFrame (numFrames);
		qid.redraw(null);
	}
	
	static class SoundData {
		QTHandle sampleData;
		SoundDescription description;
		int numSamples;
	}
	
	private void addAudioTrack (Movie theMovie) throws QTException, IOException {
		int kFullVolume	= 1;
			
			// fills in the above SoundData class fields
		SoundData theSound = getSound();
	
			//print out the SoundDescription
		System.out.println (theSound.description);
			
			//create Sound track with SoundMedia object -> the rounded sample rate is the time scale of the sound media
		Track sndTrack = theMovie.addTrack (0, 0, kFullVolume);
		SoundMedia sndMedia = new SoundMedia (sndTrack, theSound.description.getSampleRateRounded());  

			//add the sample data into the sound media
		sndMedia.beginEdits();
		sndMedia.addSample (theSound.sampleData,
								0,
								theSound.sampleData.getSize(),
								1, // duration of each sound sample,
								theSound.description,
								theSound.numSamples,
								0);
		sndMedia.endEdits();
			
			// insert the media into the track
		int kTrackStart	= 0;
		int kMediaTime 	= 0;
		int kMediaRate	= 1;
		sndTrack.insertMedia (kTrackStart, kMediaTime, sndMedia.getDuration(), kMediaRate);
	}
	
	private SoundData getSound() throws QTException, IOException {		
		//
		// read sound.aif file into memory and create a description and read the sound data out of it.
		//
												
		SndInfo info = SndInfo.parseAIFFHeader (OpenFile.asRead (new QTFile(soundFile)));
		
		SoundData sd = new SoundData();

		SoundComponentData mySndInfo = info.sndData;

		sd.description = new SoundDescription (mySndInfo.getFormat());
		sd.description.setNumberOfChannels (mySndInfo.getNumChannels());
		sd.description.setSampleSize (mySndInfo.getSampleSize());
		sd.description.setSampleRate (mySndInfo.getSampleRate());
		
		int mySampleDataSize = mySndInfo.getSampleCount() * (mySndInfo.getSampleSize() / 8); 
			//read just the sample data into memory
			// it is sample data size and is found at the data offset location in the file
			// as returned by the ParseAIFF header call
			FileInputStream fis = new FileInputStream (soundFile);
			byte[] ar = new byte [mySampleDataSize];			
			fis.skip (info.dataOffset);
			fis.read (ar, 0, mySampleDataSize);
		
		sd.sampleData = new QTHandle (ar);
		
		sd.numSamples = mySndInfo.getSampleCount();
		return sd;
	}
}


/* THIS way captures the result of the draw method of the QTImageDrawer
	it then adds the raw pixel data to the movie, getting that raw data and description
	from the ImagePresenter of the QTImageDrawer
	
	the way we add data above is to compress the GWorld the QTImageDrawer draws to
	using the animation compressor -> which also gives us frame differencing and a smalller data size
	and better playback for the movie.
	
	this code is presented as an alternative which may be appropriate in certain instances
	-> like adding image data for a sprite where no fidelity on each image is lost.
	in this case the important point is NOT the QTImageDrawer but the addition
	of the raw EncodedImage and ImageDescription -> how we get that is up to the application.
	
		for (int curSample = 1; curSample <= numFrames; curSample++) {
	       	QTUtils.reclaimMemory();
	       	np.setCurrentFrame (curSample);
			qid.redraw(null);
			ImagePresenter ip = qid.toImagePresenter();
			ImageDescription desc = ip.getDescription();
			EncodedImage imageData = ip.getImage(1);			
			vidMedia.addSample (QTHandle.fromEncodedImage(imageData), 
									0, // dataOffset,
									desc.getDataSize(),
									60, // frameDuration, 60/600 = 1/10 of a second, desired time per frame	
									desc,
									1, // one sample
									0 ); // no flags
        }
*/				
