/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */

import java.awt.Frame;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.Insets;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.lang.Math;

import quicktime.qd.*;
import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.io.QTFile;
import quicktime.util.*;
import quicktime.std.image.*;

import quicktime.app.display.QTCanvas;
import quicktime.app.image.*;

public class CurvesDemo extends Frame {

	public static void main(String args[]) {
		try {
			QTSession.open();
			CurvesDemo cdemo = new CurvesDemo();
			cdemo.show();
			
		}catch(QTException ex) {
			ex.printStackTrace();
			QTSession.close();
		}	
	}
	
	static final int kWidth = 400;
	static final int kHeight = 400;
	QTCanvas canv;
	VectorStream vectorStream;
	Curve curve;
	
	CurvesDemo() throws QTException {
		super("CurvesDemo - in QTJava");
		setLayout(new BorderLayout());

		canv = new QTCanvas ();
		add ("Center", canv);
		
		curve = new Curve(); //Open the default vector codec component
		// create a new vector data stream for holding the data for paths
		vectorStream = new VectorStream(curve); 
		
		//the black rounded square
   		//vectorStream.addAtom(StdQTConstants.kCurveFillTypeAtom, StdQTConstants.gxEvenOddFill);
		CurvePath pathData1 = new CurvePath (curve);
		//Hashtable  round_rect  = roundedSquare();
        
		Vector round_rect  = roundedSquare();
		int i = 0;
        Enumeration e = round_rect.elements();
        while (e.hasMoreElements()) {
			QDPoint pt = (QDPoint)e.nextElement();
            Boolean b = (Boolean)e.nextElement();
  			pathData1.insertPoint (pt, 0, i, b.booleanValue());
            i++;
        }
		vectorStream.addPathAtom (pathData1);

		//the colorful rectangle
   		vectorStream.addAtom (StdQTConstants.kCurvePenThicknessAtom, 0x100000);
   		vectorStream.addAtom (StdQTConstants.kCurveGradientTypeAtom, StdQTConstants.kLinearGradient);
				
		QDColor[] gradientColors = createGradientColors();
		vectorStream.addAtom (gradientColors);

		// set gradient angle
  		vectorStream.addAtom (StdQTConstants.kCurveGradientAngleAtom, 0x00450000);
		
		//adds kARGBColorAtom to the stream
		QDColor color2 = new QDColor (0x0000, 0xffff, 0x0000, 0x0000);
   		vectorStream.addAtom (color2);

		CurvePath gradPath = new CurvePath (curve);
 		Vector greenRect  = greenRectangle();
		i = 0;
        e = null;
        e = greenRect.elements();
        while (e.hasMoreElements()) {
			QDPoint pt = (QDPoint)e.nextElement();
            Boolean b = (Boolean)e.nextElement();
  			gradPath.insertPoint (pt, 0, i, b.booleanValue());
            i++;
        }
       
		/*Hashtable  greenRect  = greenRectangle ();
		i = 0;
		for (Enumeration greenRectKeys = greenRect.keys() ; greenRectKeys.hasMoreElements() ;) {
			QDPoint pt = (QDPoint)greenRectKeys.nextElement();
			gradPath.insertPoint (pt, 0, i, ((Boolean)greenRect.get(pt)).booleanValue());
			i++;
		}*/
		vectorStream.addPathAtom (gradPath);
		
		// disable gradient for all following atoms 
  		vectorStream.removeAtom (StdQTConstants.kCurveGradientRecordAtom);

		QDColor color3 = new QDColor (0xffff, 0x0000, 0x0000, 0x3333);
   		vectorStream.addAtom (color3);
		
		CurvePath sqrPath = new CurvePath (curve); 
 		Vector redSqr  = redSquare();
		i = 0;
        e = null;
        e = redSqr.elements();
        while (e.hasMoreElements()) {
			QDPoint pt = (QDPoint)e.nextElement();
            Boolean b = (Boolean)e.nextElement();
  			sqrPath.insertPoint (pt, 0, i, b.booleanValue());
            i++;
        }
		vectorStream.addPathAtom (sqrPath);
        
		//adds kARGBColorAtom to the stream
		QDColor color1 = new QDColor (0x0000, 0x0000, 0x0000, 0x0000);
		vectorStream.addAtom(color1);
		
		// draw a OpenFrame fill path
  		vectorStream.addAtom (StdQTConstants.kCurveFillTypeAtom, StdQTConstants.gxOpenFrameFill);
   		vectorStream.addAtom (StdQTConstants.kCurvePenThicknessAtom, 0x030000);
   		
   		
		CurvePath openPath = new CurvePath (curve);  
 		Vector pointsTable  = createPointsTable();
		i = 0;
        e = null;
        e = pointsTable.elements();
        while (e.hasMoreElements()) {
			QDPoint pt = (QDPoint)e.nextElement();
            Boolean b = (Boolean)e.nextElement();
  			openPath.insertPoint (pt, 0, i, b.booleanValue());
            i++;
        }
		vectorStream.addPathAtom (openPath);
	

		//outer rectangle -- ClosedFrameFill
   		vectorStream.addAtom (StdQTConstants.kCurvePenThicknessAtom, 0x030000);
   		vectorStream.addAtom(StdQTConstants.kCurveFillTypeAtom, StdQTConstants.gxClosedFrameFill);
		
		//Create a new CurvePath 
		CurvePath closedPath = new CurvePath (curve); 
 		Vector outer_rect  = outerRectangle();
		i = 0;
        e = null;
        e = outer_rect.elements();
        while (e.hasMoreElements()) {
			QDPoint pt = (QDPoint)e.nextElement();
            Boolean b = (Boolean)e.nextElement();
  			closedPath.insertPoint (pt, 0, i, b.booleanValue());
            i++;
        }
		//then add the pathdata to the vector stream
		vectorStream.addPathAtom (closedPath);
		//add the zero atom to the vector stream to mark the end of data stream		
		vectorStream.addZeroAtom();

		displayCurve();
		
		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) {
				System.exit(0);
			}
		});		
		pack();
	}
	
	
	public void displayCurve() throws QTException {
		//ImagePresenter is what will display the QTVector image to the screen.
		ImagePresenter myCurvePresenter = new ImagePresenter (new QDRect (kWidth, kHeight));

		ImageDescription curveDescription = ImageDescription.getJavaDefaultPixelDescription (kWidth, kHeight);
		curveDescription.setCType (StdQTConstants.kVectorCodecType);

		//This call set's up the ImageDescription for the vector graphic. Note that we have not
		//set the image data as this will be set later...
		myCurvePresenter.setImageData (null, curveDescription);

		//ImageDescription curveDescription = new ImageDescription(kVectorCodecType);

		ByteEncodedImage aCurveObject = ByteEncodedImage.fromByteArray(vectorStream.getBytes());

		myCurvePresenter.setImageData(aCurveObject);		

		//NOTES::	
		//It is possible  to create multiple vector images as ByteEncodedImage and then later 
		//set them as the ImageData in an ImagePresenter
		
		canv.setClient (myCurvePresenter, true);	
	
	}

	public QDColor[] createGradientColors() {
		QDColor[] gradientColors = new QDColor[4];
		gradientColors[0] = new QDColor(0xffff, 0x0000, 0x0000, 0xffff, 0.0F);
		gradientColors[1] = new QDColor(0x0000, 0xffff, 0x0000, 0x7777, 0.3F);
		gradientColors[2] = new QDColor(0xffff, 0x0000, 0x0000, 0x3333, 0.7F);
		gradientColors[3] = new QDColor(0x0000, 0x0000, 0xffff, 0xffff, 1.0F);
		
		return gradientColors;
	}

	/* specify the points for the path and whether each one is 
   	on the path */
	public Vector createPointsTable() {
		Vector table = new Vector();
		table.addElement(new QDPoint(200, 100));
		table.addElement(new Boolean(true));
		table.addElement(new QDPoint(150, 50));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(200, 25));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(300, 25));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(350, 50));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(300, 100));
        table.addElement(new Boolean(true));
		return table;
	}

	public Vector createBigRectangle() {
		Vector table = new Vector();
		table.addElement(new QDPoint(0, 0));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(400, 0));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(400, 400));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(0, 400));
        table.addElement(new Boolean(true));
		
		return table;
	}

	public Vector outerRectangle() {
		Vector table = new Vector();
		table.addElement(new QDPoint(10, 10));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(390, 10));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(390, 390));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(10, 390));
        table.addElement(new Boolean(true));
		
		return table;
	}

	public Vector greenRectangle() {
       Vector table = new Vector();
		table.addElement(new QDPoint(20, 20));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(70, 20));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(70, 70));
        table.addElement(new Boolean(true));
		table.addElement(new QDPoint(20, 70));
        table.addElement(new Boolean(true));
        return table;
	}


	public Vector redSquare() {
        Vector table = new Vector();
		table.addElement(new QDPoint(40, 40));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(100, 40));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(100, 100));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(40, 100));
        table.addElement(new Boolean(false));
        return table;
	}

	public Vector roundedSquare() {
        Vector table = new Vector();
		table.addElement(new QDPoint(150,150));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(250, 150));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(250,250));
        table.addElement(new Boolean(false));
		table.addElement(new QDPoint(150,250));
        table.addElement(new Boolean(false));
        return table;
    }

}
