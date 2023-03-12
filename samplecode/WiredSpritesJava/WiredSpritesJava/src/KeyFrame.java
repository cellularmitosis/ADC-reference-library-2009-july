/*
	File:		KeyFrame.java
	
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

import quicktime.std.movies.*;
import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.qd.*;
import quicktime.util.*;

/* this class contains methods to add sprite shared images to the key
		frame atom container */
public class KeyFrame extends SpriteAtom
{
		/* the keyFrame atom container */
	private AtomContainer keyFrameSample;
	
		/* the kSpriteSharedDataAtomType atom */
	private Atom spriteSharedDataAtom;
	private static final int sharedDataAtomID = 1;
	
		/* the kSpriteImageContainerAtomType atom */
	private Atom spriteImageContainerAtom;
	private static final int imageContainerAtomID = 1;
	private static final int spriteImageDataAtomTypeID = 1;
	private static final int spriteImageRegistrationAtomTypeID = 1;
	private static final int spriteNameAtomTypeID = 1;
	private static final int spriteGroupIDAtomTypeID = 1;

	KeyFrame(AtomContainer container)
	{
		super(container);
		keyFrameSample = container;
		
		spriteSharedDataAtom = null;
		spriteImageContainerAtom = null;
	}
	
		/* add the kSpriteSharedDataAtomType and kSpriteImageContainerAtomType atoms
			for our shared image data */
	private void addSharedDataAtomsToKeyFrameSample()
	{
		try
		{
			 spriteSharedDataAtom = keyFrameSample.insertChild(
												new Atom(StdQTConstants.kParentAtomIsContainer),
												StdQTConstants.kSpriteSharedDataAtomType,
												sharedDataAtomID,
												0);
			if (spriteSharedDataAtom != null)
				 spriteImageContainerAtom = keyFrameSample.insertChild(
														spriteSharedDataAtom, 
														StdQTConstants.kSpriteImagesContainerAtomType, 
														imageContainerAtomID, 
														0);
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}
	
		/* add a kSpriteImageAtomType atom plus the associated
			kSpriteImageDataAtomType, kSpriteNameAtomType,
			kSpriteImageRegistrationAtomType and kSpriteImageGroupIDAtomType atoms
			for a sprite image */
	public void addSpriteImageToSpriteImageContainer(int imageID,
											int imageIndex,
											QTHandle imageHandle,
											String imageName,
											QDPoint regPt,
											int groupID)
	{
		if ((spriteSharedDataAtom == null) && (spriteImageContainerAtom == null))
			addSharedDataAtomsToKeyFrameSample();

		try
		{
				/* add the kSpriteImageAtomType atom */
			Atom spriteImageAtom =
				keyFrameSample.insertChild(spriteImageContainerAtom,
									StdQTConstants.kSpriteImageAtomType,
									imageID,
									imageIndex);
			if (spriteImageAtom != null)
			{
				if (imageHandle != null)
				{
						/* add the kSpriteImageDataAtomType atom, specifying
							a handle with the image */
					keyFrameSample.insertChild(spriteImageAtom, 
											StdQTConstants.kSpriteImageDataAtomType,
											spriteImageDataAtomTypeID,
											0,
							     				imageHandle);
				}
				
				if (imageName != null)
				{
						/* add the kSpriteNameAtomType atom, specifying
							a name to associate with the sprite image */
					keyFrameSample.insertChild(spriteImageAtom, 
											StdQTConstants.kSpriteNameAtomType,
											spriteNameAtomTypeID,
											0,
							     				imageName.getBytes());

				}
				
				if (regPt != null)
				{
					int x,y;
					QDPoint thePoint = regPt;
	
						/* let's first endian-flip our QDPoint */
					EndianDescriptor endianDescForFixedPt = thePoint.getEndianDescriptorFixedPoint();
					byte[] fixedPtAsArray = thePoint.getFixedPoint();
					EndianOrder.flipNativeToBigEndian(fixedPtAsArray,0,endianDescForFixedPt);
					
						/* add the kSpriteImageRegistrationAtomType atom,
							specifying the registration point for the sprite image */
					keyFrameSample.insertChild(spriteImageAtom, 
											StdQTConstants.kSpriteImageRegistrationAtomType,
											spriteImageRegistrationAtomTypeID,
											0,
							     				fixedPtAsArray);
				}
				
				if (groupID != 0)
				{
						/* add the kSpriteImageGroupIDAtomType atom,
							specifying the groupID value for the image */
					keyFrameSample.insertChild(spriteImageAtom, 
											StdQTConstants.kSpriteImageGroupIDAtomType,
											spriteGroupIDAtomTypeID,
											0,
							     				EndianOrder.flipNativeToBigEndian32(groupID));
				}
			}
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}
}