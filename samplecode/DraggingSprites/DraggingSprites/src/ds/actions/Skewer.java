/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
package ds.actions;

import quicktime.app.actions.*;
import quicktime.qd.*;
import quicktime.*;
import quicktime.std.image.Matrix;
import quicktime.app.image.Transformable;
import quicktime.app.display.Drawable;

import java.awt.Dimension;
import java.awt.event.MouseEvent;

/**
 * A class that provides the implementation of where a dragged object will be skewed
 * The amount of skew is determined by the distance between the original mouse click and the 
 * current mouse-moved location divided by the specified skewFactors.
 */
public class Skewer extends Dragger {
//____________________________ CLASS METHODS
	/**
	 * Set some parameters that will create DragActions. 
	 * @param modifierKeyMask - if specified will determine which modifier keys must
	 * be depressed for the action to be invoked.
	 */
	public Skewer (int xSkewFactor, int ySkewFactor, int modifierKeyMask) {
		this (xSkewFactor, ySkewFactor, modifierKeyMask, MouseResponder.kModifiersExactMatch, 0);
	}

	/**
	 * Set some parameters that will create DragActions. 
	 * @param modifierKeyMask - if specified will determine which modifier keys must
	 * be depressed for the action to be invoked.
	 * @param modifierTestConditions the test conditions under which the modifier mask is tested
	 */
	public Skewer (int xSkewFactor, int ySkewFactor, int modifierKeyMask, int modifierTestConditions, int addedEventInvoker) {
		super (modifierKeyMask, modifierTestConditions, addedEventInvoker);
		xSkew = xSkewFactor;
		ySkew = ySkewFactor;
	}	
	
//____________________________ INSTANCE VARIABLES
	private int xOrigin, yOrigin;	
	private int xSkew, ySkew;
	private Matrix originalMat;

//____________________________ INSTANCE METHODS
	/**
	 * This method is used by the DragController when the mouse is first pressed down on 
	 * the draggable object. If you wish to do anything to your draggable object before it
	 * is dragged then you should overide this method. The default implementation does nothing.
	 * @param event the mouse down event that may begin the drag action
	 */
	public void mousePressed (MouseEvent event) {
		try {
			xOrigin = event.getX();
			yOrigin = event.getY();
			originalMat = target.getMatrix();
		} catch (QTException e) {
			throw new QTRuntimeException (e);
		}
	}

	/**
	 * This method is called by Dragger when an event is received that meets the conditions for 
	 * the object to be dragged.
	 * <P>
	 * This method will allow the user to skew the object by dragging on it.
	 * @param event the mouse drag event that triggered the drag action.
	 * @param space The drawable object is the space within which the event has occured.
	 */
	public void mouseDragged (MouseEvent event) {
		try {
			// protect against a zero divide
			float skewX = (xSkew != 0 ? (event.getX() - xOrigin) / (float)xSkew : 0);
			float skewY = (ySkew != 0 ? (event.getY() - yOrigin) / (float)ySkew : 0);
			Matrix mat = (Matrix)originalMat.clone();
			
			mat.skew (skewX, skewY, mat.getTx(), mat.getTy());
					
			target.setMatrix (mat);
		} catch (QTException e) {
			throw new QTRuntimeException (e);
		}
	}
}
