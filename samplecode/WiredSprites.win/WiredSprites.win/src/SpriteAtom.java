/*
	File:		SpriteAtom.java
	
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
import quicktime.std.*;
import quicktime.*;
import quicktime.util.*;
import quicktime.std.movies.media.*;
import quicktime.std.image.*;
import quicktime.qd.*;

public class SpriteAtom
{
		/* the override atom container */
	private AtomContainer atomContainer;
		/* the kSpriteURLLinkAtomType atom */
	private static final int kSpriteURLLinkAtomType = 1970433056;	/* 'url ' */
	
		/* constructor needs an atom container
			to add sprites */
	SpriteAtom(AtomContainer container)
	{
		atomContainer = container;
	}
	
		/* add the kSpriteAtomType atom - this atom describes the
			sprite. It contains atoms that describe properties of the sprite. */
	public Atom addSpriteAtom(int spriteID)
	{
		return (getSpriteAtom(spriteID));
	}
	
		/* retrieve the kSpriteAtomType atom - if it does not
			exist, we will create it */
	private Atom getSpriteAtom(int spriteID)
	{
		Atom spriteAtom = null;
		
		try
		{		/* locate the kSpriteAtomType atom */
			spriteAtom = atomContainer.findChildByID_Atom(new Atom(StdQTConstants.kParentAtomIsContainer),
					                                StdQTConstants.kSpriteAtomType,
					                                spriteID);
				/* was the atom not found? */
			if (spriteAtom == null)
			{
					/* atom not found, so let's create it */
				spriteAtom = atomContainer.insertChild(new Atom(StdQTConstants.kParentAtomIsContainer),
						     StdQTConstants.kSpriteAtomType, spriteID, 0);
			}
		}
		catch (QTException qte) { }
		
		return spriteAtom;	
	}

		/* add the kSpritePropertyLayer atom - this atom specifies
			the layer (0 - XX) property for the sprite. Lower layer 
			numbers appear in front */
	public void addSpriteLayerPropertyToSpriteAtom(int spriteID, int layer)
	{
		try
		{
			atomContainer.insertChild(getSpriteAtom(spriteID), StdQTConstants.kSpritePropertyLayer, 1, 0,
						   EndianOrder.flipNativeToBigEndian16((short)layer));
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}
	
		/* add the kSpritePropertyVisible atom - this atom designates
			the sprite as visible or not visible */
	public void addSpriteVisiblePropertyToSpriteAtom(int spriteID, int visible)
	{
		try
		{
			atomContainer.insertChild(getSpriteAtom(spriteID), StdQTConstants.kSpritePropertyVisible, 1, 0,
						   EndianOrder.flipNativeToBigEndian16((short)visible));
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add the kSpritePropertyImageIndex atom - this atom designates
			the id of the image in the key frame shared images to use for 
			the sprite */	
	public void addSpriteImageIndexPropertyToSpriteAtom(int spriteID, int index)
	{
		try
		{
			atomContainer.insertChild(getSpriteAtom(spriteID), StdQTConstants.kSpritePropertyImageIndex, 1, 0,
						     EndianOrder.flipNativeToBigEndian16((short)(index)));
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add the kSpritePropertyMatrix atom - this atom specifies
			the sprite location and scaling within its sprite world or sprite
			track */		
	public void addSpriteMatrixPropertyToSpriteAtom(int spriteID, Matrix theMatrix)
	{
		try
		{
			Matrix endianFlippedMatrix = theMatrix;
						
			quicktime.util.EndianOrder.flipNativeToBigEndian(endianFlippedMatrix,0,new EndianDescriptor(EndianDescriptor.kFlipAllFields32));
			atomContainer.insertChild(getSpriteAtom(spriteID), StdQTConstants.kSpritePropertyMatrix, 1, 0, endianFlippedMatrix);
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add the kSpritePropertyGraphicsMode atom - this atom specifies
			the graphics mode and blend color that indicates how to blend a sprite
			with any sprites behind it and with the background. */		
	public void addSpriteGraphicsModePropertyToSpriteAtom(int spriteID, GraphicsMode grMode)
	{
		try
		{
			GraphicsMode grModeCopy = (GraphicsMode)grMode.clone();
			grModeCopy.setGraphicsMode(grModeCopy.getGraphicsMode());
			QDColor color = grModeCopy.getColor();
			EndianOrder.flipNativeToBigEndian(color,0,EndianDescriptor.flipAll16);
			grModeCopy.setColor(color);

			atomContainer.insertChild(getSpriteAtom(spriteID), StdQTConstants.kSpritePropertyGraphicsMode, 1, 0, grModeCopy);
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add the kSpriteNameAtomType atom - this atom specifies
			the sprite name */		
	public void addSpriteNamePropertyToSpriteAtom(int spriteID, String theName)
	{
		try
		{
				/* name must be a pascal string */
			byte nameByteArray[] =QTUtils.String2PString(theName, theName.length());

			atomContainer.insertChild(getSpriteAtom(spriteID), StdQTConstants.kSpriteNameAtomType, 1, 0,
						     nameByteArray);
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add the kSpriteURLLinkAtomType atom - this atom specifies
			an optional url link to associate with the sprite */		
	public void addSpriteURLLinkPropertyToSpriteAtom(int spriteID, String theName)
	{
		try
		{
				/* name must be a pascal string */
			byte nameByteArray[] =QTUtils.String2PString(theName, theName.length());
			
			atomContainer.insertChild(getSpriteAtom(spriteID), kSpriteURLLinkAtomType, 1, 0,
						     nameByteArray);
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
	}
		
		/* add the kAction atom for a sprite action */
	public Atom addActionAtom(Atom eventAtom, int whichActionConstant)
	{
		Atom actionAtom=null;
		
		try
		{
			actionAtom = atomContainer.insertChild(eventAtom,
						                         StdQTConstants.kAction,
						                         0,
						                         0);

			atomContainer.insertChild(actionAtom,
				                         StdQTConstants.kWhichAction,
				                         1,
				                         1,
				                         EndianOrder.flipNativeToBigEndian32(whichActionConstant));
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
		
		return actionAtom;
	}

		/* add either a kQTEventType atom for the specified event or the 
			kQTEventFrameLoaded atom for frame-loaded events */
	public Atom addQTEventAtom(Atom spriteAtom, int theQTEventType )
	{
		Atom qtEventAtom=null;
		
		try
		{
			if ( theQTEventType == StdQTConstants.kQTEventFrameLoaded ) 
			{
		 		qtEventAtom = atomContainer.insertChild(spriteAtom,
							                         StdQTConstants.kQTEventFrameLoaded,
							                         1,
							                         1);

			}
			else 
			{
				qtEventAtom = atomContainer.findChildByID_Atom(spriteAtom,
							                                StdQTConstants.kQTEventType,
							                                theQTEventType);

				if (qtEventAtom == null) 
				{
			 		qtEventAtom = atomContainer.insertChild(spriteAtom,
								                         StdQTConstants.kQTEventType,
								                         theQTEventType,
								                         1);
				}
			}
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
		
		return qtEventAtom;
	}

		/* add both the kAction atom and the kQTEventType atom for an 
			event/action combination */
	public Atom addQTEventAndActionAtoms( Atom spriteAtom, int whichEvent, int whichAction)
	{
		Atom	eventAtom=null, actionAtom = null;
		
		if ( whichEvent != 0 )
			eventAtom = addQTEventAtom( spriteAtom, whichEvent);
		
		actionAtom = addActionAtom(eventAtom, whichAction);

		return actionAtom;
	}

		/* add the kTargetSpriteID atom to specify a sprite target
			ID */
	public void addSpriteIDActionTargetAtom(int spriteID, Atom actionAtom)
	{
		try
		{
			Atom actionTargetAtom = atomContainer.findChildByIndex_Atom(actionAtom,
									                                StdQTConstants.kActionTarget,
									                                1);
			if (actionTargetAtom == null) 
			{
		 		actionTargetAtom = atomContainer.insertChild(actionAtom,
								                         StdQTConstants.kActionTarget,
								                         1,
								                         1);
			}
			
			atomContainer.insertChild(actionTargetAtom,
				                         StdQTConstants.kTargetSpriteID,
				                         1,
				                         1,
				                         EndianOrder.flipNativeToBigEndian32(spriteID));
		}
		catch(QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add the kTargetTrackIndex atom to specify
			a track index target */
	public void addTrackIndexActionTargetAtom(Atom actionAtom, int trackIndex)
	{
		try
		{
			Atom actionTargetAtom = atomContainer.findChildByIndex_Atom(actionAtom,
						                                StdQTConstants.kActionTarget,
						                                1);
			if (actionTargetAtom == null) 
			{
		 		actionTargetAtom = atomContainer.insertChild(actionAtom,
							                         StdQTConstants.kActionTarget,
							                         1,
							                         1);
			}			
			
			atomContainer.insertChild(actionTargetAtom,
				                         StdQTConstants.kTargetTrackIndex,
				                         1,
				                         1,
				                         EndianOrder.flipNativeToBigEndian32(trackIndex));
		}
		catch(QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add the kTargetTrackType atom to specify a track type target */
	public void addTrackTypeActionTargetAtom(Atom actionAtom, int targetTrackType)
	{
		try
		{
			Atom actionTargetAtom = atomContainer.findChildByIndex_Atom(actionAtom,
										                                StdQTConstants.kActionTarget,
										                                1);
			if (actionTargetAtom == null) 
			{
		 		actionTargetAtom = atomContainer.insertChild(actionAtom,
									                         StdQTConstants.kActionTarget,
									                         1,
									                         1);
			}
			
			atomContainer.insertChild(actionTargetAtom,
				                         StdQTConstants.kTargetTrackType,
				                         1,
				                         1,
				                         EndianOrder.flipNativeToBigEndian32(targetTrackType));
		}
		catch(QTException qte)
		{
			qte.printStackTrace();
		}
	}

		/* add all the necessary atoms to trigger a setVisible sprite action
			when the specified event occurrs */
	public void addSpriteSetVisibleAction( int spriteID,
								int whichEvent, 
								int trackTargetType, 
								int trackTypeIndex,
								int visible)
	{
		Atom actionAtom = null;
		
		Atom spriteAtom = getSpriteAtom(spriteID);
		
		actionAtom = addQTEventAndActionAtoms(spriteAtom, whichEvent, StdQTConstants.kActionSpriteSetVisible);

		try
		{
			atomContainer.insertChild(actionAtom,
				                         StdQTConstants.kActionParameter,
				                         0,
				                         1,
				                         EndianOrder.flipNativeToBigEndian16((short)visible));
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
		
		addTrackTypeActionTargetAtom(actionAtom, trackTargetType);
		if (trackTypeIndex != 0)
			addTrackIndexActionTargetAtom(actionAtom, trackTypeIndex);
		
		addSpriteIDActionTargetAtom(spriteID, actionAtom);
	
	}


		/* add all the necessary atoms to trigger a rotate sprite action
			when the specified event occurrs */
	public void addSpriteRotateAction( int spriteID,
								int whichEvent, 
								int trackTargetType, 
								int trackTypeIndex,
								float degrees)
	{
		Atom actionAtom = null;
		Atom spriteAtom = getSpriteAtom(spriteID);
		
		actionAtom = addQTEventAndActionAtoms(spriteAtom, whichEvent, StdQTConstants.kActionSpriteRotate);
		try
		{
			atomContainer.insertChild(actionAtom,
				                         StdQTConstants.kActionParameter,
				                         0,
				                         1,
				                         EndianOrder.flipNativeToBigEndian32(QTUtils.X2Fix(degrees)));
		}
		catch (QTException qte)
		{
			qte.printStackTrace();
		}
		
		addTrackTypeActionTargetAtom(actionAtom, trackTargetType);
		if (trackTypeIndex != 0)
			addTrackIndexActionTargetAtom(actionAtom, trackTypeIndex);
		
		addSpriteIDActionTargetAtom(spriteID, actionAtom);	
	}
}