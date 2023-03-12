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
 * A class that provides the implementation of where a dragged object will be scaled according
 * to the amount of pixels from the origin the user drags the mouse.
 */
public class Scaler extends Dragger {
//____________________________ CLASS METHODS
	/**
	 * Set some parameters that will create DragActions. 
	 * @param modifierKeyMask - if specified will determine which modifier keys must
	 * be depressed for the action to be invoked.
	 */
	public Scaler (int scaleFactor, int modifierKeyMask) {
		this (scaleFactor, modifierKeyMask, MouseResponder.kModifiersExactMatch, 0);
	}

	/**
	 * Set some parameters that will create DragActions. 
	 * @param modifierKeyMask - if specified will determine which modifier keys must
	 * be depressed for the action to be invoked.
	 * @param modifierTestConditions the test conditions under which the modifier mask is tested
	 */
	public Scaler (int scaleFactor, int modifierKeyMask, int modifierTestConditions, int addedEventInvoker) {
		super (modifierKeyMask, modifierTestConditions, addedEventInvoker);
		this.scaleFactor = scaleFactor;
	}	
	
//____________________________ INSTANCE VARIABLES
	private int xOrigin, yOrigin;	
	private int scaleFactor;

//____________________________ INSTANCE METHODS
	/**
	 * This method is used by the DragController when the mouse is first pressed down on 
	 * the draggable object. If you wish to do anything to your draggable object before it
	 * is dragged then you should overide this method. The default implementation does nothing.
	 * @param event the mouse down event that may begin the drag action
	 * @param space The drawable object is the space within which the event has occured.
	 */
	public void mousePressed (MouseEvent event) {
		xOrigin = event.getX();
		yOrigin = event.getY();
	}

	/**
	 * This method is called by Dragger when an event is received that meets the conditions for 
	 * the object to be dragged.
	 * <P>
	 * This method will allow the user to alter the size of the object by dragging on it.
	 * @param event the mouse drag event that triggered the drag action.
	 * @param space The drawable object is the space within which the event has occured.
	 */
	public void mouseDragged (MouseEvent event) {
		try {
			Matrix mat = target.getMatrix();

			float x = (event.getX() - xOrigin) / (float)scaleFactor;
			float y = (event.getY() - yOrigin) / (float)scaleFactor;
			xOrigin = event.getX();
			yOrigin = event.getY();
			
			mat.setSx (mat.getSx() + x);
			mat.setSy (mat.getSy() + y);
			
			target.setMatrix (mat);
		} catch (QTException e) {
			throw new QTRuntimeException (e);
		}
	}
}
