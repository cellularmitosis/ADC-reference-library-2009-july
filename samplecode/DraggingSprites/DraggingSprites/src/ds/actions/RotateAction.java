/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */
package ds.actions;

import quicktime.app.actions.*;
import quicktime.QTException;
import quicktime.qd.*;
import quicktime.std.StdQTException;
import quicktime.std.image.Matrix;
import quicktime.app.display.QTDrawable;
import quicktime.app.image.Transformable;
import java.awt.Dimension;
/**
 * This class provides the capability of moving and bouncing a Transformable
 * object around within the space provided by the QTDrawable space that is
 * the space within which the transformable object is displayed, by the amount specified
 * by the deltaMatrix object.
 */ 
public class RotateAction extends MatrixAction {
//____________________________ CLASS VARIABLES
//____________________________ CLASS METHODS
	/**
	 * Constructs a BounceAction object.
	 * @param deltaX the amount by which the object is moved in the X dimension
	 * @param deltaY the amount by which the object is moved in the Y dimension
	 * @param space the space within which the object will be bounced and moved
	 * @param scale the amount with which a second is divided into at a rate of one
	 * @param period the number of scale ticks that elapse between invocations of the action.
	 * @param target the target of the bounce action - the object that is moved
	 */
	public RotateAction (int scale, int period, QTDrawable space, Transformable t) throws QTException {
		super (scale, period, space, t);
		doConstraintBoundsTesting = false;
 	}

//____________________________ INSTANCE VARIABLES
	private float deltaDegree = 1;
//____________________________ INSTANCE METHODS	
	protected void rateDirectionChanged (boolean forwards) throws QTException {
		deltaDegree = -deltaDegree;
	}
	
	protected void transformMatrix (Matrix theMatrix) throws QTException {
		theMatrix.rotate (deltaDegree, theMatrix.getTx(), theMatrix.getTy());
	}
}

/*
 * $Log: /Biscotti/QTJavaDemos/DraggingSprites/src/ds/actions/RotateAction.java $
 * 
 * 2     3/11/99 5:36 PM Roger Smith
 * Update Source License Agreement
 * 
 * 1     16/11/98 6:50 PM Bill Stewart
 * new action
 */
