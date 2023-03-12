/*
	File:		WiredSprites.java
	
	Description:	This demo program shows how to create a QuickTime movie containing wired sprites.
        
	Author:		Apple Computer, Inc.

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
				
	Change History (most recent first):
            11/22/2002	md	new SampleCode revisions

*/

import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.qd.*;
import quicktime.std.StdQTConstants;
import quicktime.*;
import quicktime.io.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.std.image.*;
import quicktime.util.*;

import quicktime.app.QTFactory;
import quicktime.app.players.*;
import quicktime.app.display.*;
import quicktime.app.image.*;


public class WiredSprites extends Frame
{
	static final int defaultWidth = 400;
	static final int defaultHeight = 300;

	static final int noVolume = 0;

	static int timeScale = 600; // time units per second

	static final int spriteCount = 5;
	static final String imageFolderNames[] = {"pics/","pics/","pics/","pics/","pics/"};
	static final String imageFileNamePrefixes[] = {"planet", "Ship2","star","star","star"};
	static final String imageFileNameSuffix[] = {".PICT", ".pct",".pct", ".pct",".pct"};
	static final QDPoint imageRegistrationPoints[] =  {new QDPoint(-70,-50),
														new QDPoint(-20,-20),
														new QDPoint(-100,-100),
														new QDPoint(-200,-50),
														new QDPoint(-100,-200)};
	static final int spriteImageOverridCount = 24;
	static final int spriteImageOverridFirstIndex = spriteCount + 1;
	static final int spriteImageOverridLastIndex = spriteImageOverridFirstIndex + spriteImageOverridCount;
	static final String imageOverridesFolderNames = "imageOverrides/";
	static final String imageOverridesFileNamePrefixes = "Ship";
	static final String imageOverridesFileNameSuffix = ".pct";

	public static WiredSprites app;

	public static void main (String args[])
	{
		try
		{
			QTSession.open();
			app = new WiredSprites("WiredSprites");			
			app.produceMovie();
			
		}
		catch (Exception e)
		{
			e.printStackTrace();
			QTSession.close();
		}
	}

	WiredSprites(String title) throws Exception
	{
		super(title);

		addWindowListener(new WindowAdapter ()
		{
			public void windowClosing (WindowEvent e)
			{
				QTSession.close();
				dispose();
			}
			public void windowClosed (WindowEvent e)
			{
				System.exit(0);
			}
		}
		);
	}

		/* create a movie with wired sprites */
	protected void produceMovie() throws Exception
	{
		try
		{
			FileDialog fd = new FileDialog(this, "Save movie as...",
								FileDialog.SAVE);
			fd.setFile("WiredSprite.mov");
			fd.show();
			if (fd.getFile() == null)
			{
				throw new QTIOException(Errors.userCanceledErr, "");
			}
			QTFile f = new QTFile(fd.getDirectory() + fd.getFile());
			
			System.out.println("Creating movie " + fd.getFile());

			Movie movie = Movie.createMovieFile(f,
										StdQTConstants.kMoviePlayer,
										StdQTConstants.createMovieFileDeleteCurFile |
										StdQTConstants.createMovieFileDontCreateResFile);
			
			System.out.print("Adding Sprite Tracks");

			addSpriteTrack(movie);

			OpenMovieFile movieFile = OpenMovieFile.asWrite(f);
			movie.addResource(movieFile, StdQTConstants.movieInDataForkResID, f.getName());
			movieFile.close();
			
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

		/* add a sprite track to our movie */
	private void addSpriteTrack(Movie movie) throws Exception
	{

	Track spriteTrack = movie.addTrack(defaultWidth, defaultHeight, noVolume);
	SpriteMedia spriteMedia = new SpriteMedia(spriteTrack, timeScale);
	
		/* create a new keyframe atom container to hold all our
			sprites and their associated images */
	AtomContainer keyFrameContainer = new AtomContainer();
		/* we'll use the methods of the KeyFrame class to construct our
			sprites - the methods in this class act on the keyframe
			atom container we pass to it*/
	KeyFrame keyFrameSample = new KeyFrame(keyFrameContainer);
	
	QTHandle imageDataHandle;
		/* here's the loop where we build each sprite */
	for (int spriteID=1;spriteID<=spriteCount;++spriteID)
	{
		System.out.print(".");
			/* get the image file for our sprite */
		String imageFileName = imageFolderNames[spriteID-1] + 
											imageFileNamePrefixes[spriteID-1] + 
											imageFileNameSuffix[spriteID-1];
		imageDataHandle = getImageHandleForImageFile(imageFileName);

			/* add the images for our sprite to the keyframe sample */
		keyFrameSample.addSpriteImageToSpriteImageContainer(spriteID,			/* id */
														0,				/* index */
														imageDataHandle, 	/* image data */
														imageFileName,	/* image name */
														imageRegistrationPoints[spriteID-1],	/* registration point*/
														spriteID			/* groupID */
														);
			/* add the kSpriteAtomType sprite atom for this sprite */
		keyFrameSample.addSpriteAtom(spriteID);
		System.out.print(".");
		if (spriteID == 1 || spriteID == 2)
		{
					/* make sprite 1 & 2 visible initially */
				keyFrameSample.addSpriteVisiblePropertyToSpriteAtom(spriteID /* id */,
													1 /* 1=visible */);
				if (spriteID == 1)
				{		/* make this sprite rotate for each idle event received */
					keyFrameSample.addSpriteRotateAction( spriteID,
												StdQTConstants.kQTEventIdle, 
												StdQTConstants.spriteMediaType, 
												1,	/* track type index */
												0.1F);	/* degrees of rotation */

				}
		}
		else	/* spriteID = 3 or 4 or 5*/
		{
				/* make the sprite not visible initially */
			keyFrameSample.addSpriteVisiblePropertyToSpriteAtom(spriteID /* id */,
												0 /* 0=not visible */);
				/* make the sprite visible on mouse-enter events */
			keyFrameSample.addSpriteSetVisibleAction( spriteID,
										StdQTConstants.kQTEventMouseEnter, 
										StdQTConstants.spriteMediaType, 
										1,	/* track type index */
										1);	/* 1=visible */
				/* make the sprite invisible on mouse-exit events */
			keyFrameSample.addSpriteSetVisibleAction( spriteID,
										StdQTConstants.kQTEventMouseExit, 
										StdQTConstants.spriteMediaType, 
										1,	/* track type index */
										0);	/* 0=not visible */
		}

			/* next we will set the following properties for the
				sprite: image index, layer, matrix, graphics mode,
				name and url link */
		keyFrameSample.addSpriteImageIndexPropertyToSpriteAtom(spriteID, 	/* sprite ID */
												spriteID);	/* image index (into shared images) */
		keyFrameSample.addSpriteLayerPropertyToSpriteAtom(spriteID, 
										0);	/* layer  */
		Matrix theMatrix = new Matrix();
		theMatrix.setIdentity();
		
		keyFrameSample.addSpriteMatrixPropertyToSpriteAtom(spriteID, theMatrix);
		keyFrameSample.addSpriteGraphicsModePropertyToSpriteAtom(spriteID,
											new GraphicsMode(QDConstants.srcCopy, new QDColor() /* black */));
		keyFrameSample.addSpriteNamePropertyToSpriteAtom(spriteID, new String("spriteID: "+spriteID));
		keyFrameSample.addSpriteURLLinkPropertyToSpriteAtom(spriteID, new String("http://www.apple.com"));

		imageDataHandle = null;
		imageFileName = null;
		theMatrix = null;
		quicktime.util.QTUtils.reclaimMemory();
	}

		/* now let's add the override images to our key frame sample */
	for (int overrideIndex = spriteImageOverridFirstIndex, imageCount=0;
			 overrideIndex <= spriteImageOverridLastIndex;
			  ++ overrideIndex, ++ imageCount)
	{
		System.out.print(".");
		String imageFileName = new String (new String(imageOverridesFolderNames + 
											imageOverridesFileNamePrefixes + 
											imageCount +
											imageOverridesFileNameSuffix));
		imageDataHandle = getImageHandleForImageFile(imageFileName);
			/* add the specified override image to the shared
				images in our key frame sample */
		keyFrameSample.addSpriteImageToSpriteImageContainer(overrideIndex,		/* id */
														0,				/* index */
														imageDataHandle, 	/* image data */
														imageFileName,	/* image name */
														imageRegistrationPoints[1],	/* registration point*/
														overrideIndex		/* groupID */
														);
		imageDataHandle = null;
		imageFileName = null;
		quicktime.util.QTUtils.reclaimMemory();
	}

	  	/* add the keyFrameSample to the sprite media for our sprite track */
	  spriteMedia.beginEdits();

	  spriteMedia.addSample(keyFrameContainer,
					    0,  // dataOffset
					    keyFrameContainer.getSize(),
					    20, // duration in ticks : one frame
					    new SpriteDescription(),
					    1,  // 1 samples
					    0); // flags - this is a sync sample


	AtomContainer overrideContainer;
	SpriteAtom overrideSample;

		/* here we specify (via the spriteID & image index) which images 
			we'll use to override the existing images for the designated sprite */
	for (int imageIndex = spriteImageOverridFirstIndex; imageIndex <= spriteImageOverridLastIndex; imageIndex++)
	{
		System.out.print(".");
			/* each override is a separate atom container */
		overrideContainer = new AtomContainer();
		overrideSample = new SpriteAtom(overrideContainer);
			/* specify the sprite along with the index of
				the override image */
		overrideSample.addSpriteImageIndexPropertyToSpriteAtom(2, 		/* sprite ID */
											imageIndex	/* image index */);
			/* add the override sprite atoms to the atom
				container */
		spriteMedia.addSample(overrideContainer,
						    0,      // dataOffset
						    overrideContainer.getSize(),
						    20,
						    new SampleDescription(0),
						    1,      // one sample
						    StdQTConstants.mediaSampleNotSync);    // flags
						    
		overrideContainer = null;
		overrideSample = null;
		quicktime.util.QTUtils.reclaimMemory();
	}

	spriteMedia.endEdits();

	
	int trackStart = 0;
	int mediaTime = 0;
	int mediaRate = 1;
		/* add our sprite media to our sprite track */
	spriteTrack.insertMedia(trackStart, mediaTime, spriteMedia.getDuration(), mediaRate);
		/* specify the track's properties */
	addSpriteTrackPropertyAtoms(spriteTrack, spriteMedia);

	System.out.println("\nWired sprite movie creation complete!");
	}


		/* build a QTHandle for the specified image file */
	private QTHandle getImageHandleForImageFile(String theImageFileName)
	{
		ImagePresenter imagePresenter;

		try {
			File f = QTFactory.findAbsolutePath(theImageFileName);
			QTFile imageFile = new QTFile(f.getAbsolutePath());
			GraphicsImporterDrawer gid = new GraphicsImporterDrawer(imageFile);
			imagePresenter = ImagePresenter.fromGraphicsImporterDrawer(gid);

			EncodedImage encodedImage = imagePresenter.getImage();
			ImageDescription imageDescription = imagePresenter.getDescription();

			QDRect rect = new QDRect(imageDescription.getWidth(), imageDescription.getHeight());
			QDGraphics gw = new QDGraphics(QDGraphics.kDefaultPixelFormat,rect);
			ImageSpec is = ImageUtil.makeTransparent(imagePresenter, QDColor.black,
							new QDGraphics(QDGraphics.kDefaultPixelFormat,rect));

				// Endian flip the ImageDescription
			EndianOrder.flipNativeToBigEndian(imageDescription,
				0, ImageDescription.getEndianDescriptor());

			return (new QTHandle(imageDescription, QTHandle.fromEncodedImage(encodedImage)));

		}
		catch (Exception e)
		{
			e.printStackTrace();	
		}
		
		return null;
	}

		/* here we'll specify the following properties for our sprite track: background
			color, plus the "hasAction" property, which enables sprite actions for the
			track */
	private void addSpriteTrackPropertyAtoms(Track spriteTrack, SpriteMedia spriteMedia)
	{
		try
		{
			 AtomContainer spriteTrackProperties = new AtomContainer();
			 
			 byte visible = 1;	/* visible = true */
			 	/* the "hasActions" property tells the movie controller to
			 		execute the actions in the sprite tracks media */
			 spriteTrackProperties.insertChild(new Atom(StdQTConstants.kParentAtomIsContainer),
						   		StdQTConstants.kSpriteTrackPropertyHasActions, 1, 1, visible);
			
			int idleAsFastAsPossible = 0;
			 spriteTrackProperties.insertChild(new Atom(StdQTConstants.kParentAtomIsContainer),
						   		StdQTConstants.kSpriteTrackPropertyQTIdleEventsFrequency, 1, 1, EndianOrder.flipNativeToBigEndian32(idleAsFastAsPossible));

			 QDColor bgColor = QDColor.black;
			 
			 EndianOrder.flipNativeToBigEndian(bgColor, 0,
									QDColor.getEndianDescriptorRGBColor());
			 spriteTrackProperties.insertChild(new Atom(StdQTConstants.kParentAtomIsContainer),
						   		StdQTConstants.kSpriteTrackPropertyBackgroundColor, 1, 1, bgColor);

			 spriteMedia.setPropertyAtom(spriteTrackProperties);

		}
		catch (QTException qte)
		{
			qte.printStackTrace();	
		}
	}

}
