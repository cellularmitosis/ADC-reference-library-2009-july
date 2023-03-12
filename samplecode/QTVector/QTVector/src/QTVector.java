/*
 * quicktime.app: Sample Code for Initial Seeding
 *
 * © 1996, 97 Copyright, Apple Computer
 * All rights reserved
 */
import java.awt.Frame;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.Insets;
import java.awt.event.*;
import java.io.*;
import java.lang.Math;

import quicktime.qd.*;
import quicktime.*;
import quicktime.std.StdQTConstants;
import quicktime.std.image.GraphicsImporter;
import quicktime.io.QTFile;
import quicktime.util.*;
import quicktime.std.movies.*;
import quicktime.std.image.*;

import quicktime.app.display.QTCanvas;
import quicktime.app.image.*;


/** QuickTime Vector Graphics Example */
// Shows the use of an ImagePresenter to display
// a QuickTime vector
public class QTVector extends Frame implements StdQTConstants, QDConstants {		

	static final int kWidth = 200;
	static final int kHeight = 200;

	static final int kSizeOfSizeAndTagFields = 8;
	static final int kSizeOfZeroAtomHeader   = 0;
	static final int gxEvenOddFill = 3;


	public static void main (String args[]) {
		try {
			QTSession.open();
			new QTVector().show();
			
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	QTVector () throws NativeGraphicsException, QTException, IOException {
		super ("QTVector");
				
		QTCanvas canv = new QTCanvas ();
		add ("Center", canv);
		
		//ImagePresenter is what will display the QTVector image to the screen.
		ImagePresenter myCurvePresenter = new ImagePresenter (new QDRect (kWidth, kHeight));

//		Create an Image Description and set it to use the the Curve Codec
		ImageDescription curveDescription = ImageDescription.getJavaDefaultPixelDescription (kWidth, kHeight);
		curveDescription.setCType (kVectorCodecType);
		
		//This call set's up the ImageDescription for the vector graphic. Note that we have not
		//set the image data as this will be set later...
		myCurvePresenter.setImageData (null, curveDescription);
		
		//The curve info is stored as atoms in an IntEncodedImage
		IntEncodedImage aCurveObject = createACurveObject();

		//Now that we have the Curve Data, we go ahead and set it in the ImagePresenter
		myCurvePresenter.setImageData(aCurveObject);		

	
		//NOTES::	
		//It is possible  to create multiple vector images as IntEncodedImages and then later 
		//set them as the ImageData in an ImagePresenter
		
		canv.setClient (myCurvePresenter, true);		

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
	

//AWK!!!
// Yes we know it's somewhat ugly to have to build up your image this way. QuickTime 4.0 added
//Curve API's that simplify this tremendously, however those API's are not currently accessible from QTJava. Support for  
//the Curve API's will be added at a later date.

//It is necessary to endian flip the data if you want this to work under MS Windows. On the MacOS the flipNativeToBigEndian32
//calls do nothing since the data is already in BigEndian format.. 

	public IntEncodedImage createACurveObject() throws QTException {
		
		int		curveSample[] = {	
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4),EndianOrder.flipNativeToBigEndian32(kCurveAntialiasControlAtom),
		EndianOrder.flipNativeToBigEndian32(kCurveAntialiasOn),

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4),EndianOrder.flipNativeToBigEndian32(kCurveFillTypeAtom),
		EndianOrder.flipNativeToBigEndian32(gxEvenOddFill),

		// a big white enclosing rectangle (600 x 600)
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 8), EndianOrder.flipNativeToBigEndian32(kCurveARGBColorAtom),
		EndianOrder.flipNativeToBigEndian32(0xffffffff),	// alpha, red
		EndianOrder.flipNativeToBigEndian32(0xffffffff),	// green, blue
										// it's white!

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4*11), EndianOrder.flipNativeToBigEndian32(kCurvePathAtom),
			EndianOrder.flipNativeToBigEndian32(1),			// one contour in path
			EndianOrder.flipNativeToBigEndian32(4),			// four points in path
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// all points are on the curve: it's a rectangle! 
			EndianOrder.flipNativeToBigEndian32(0x00000000), EndianOrder.flipNativeToBigEndian32(0x00000000), 	// top left
			EndianOrder.flipNativeToBigEndian32(0x02580000), EndianOrder.flipNativeToBigEndian32(0x00000000),		// top right
			EndianOrder.flipNativeToBigEndian32(0x02580000), EndianOrder.flipNativeToBigEndian32(0x02580000),		// bottom right 
			EndianOrder.flipNativeToBigEndian32(0x00000000), EndianOrder.flipNativeToBigEndian32(0x02580000),		// bottom left


		// a black rounded square, centered at 150,150
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 8), EndianOrder.flipNativeToBigEndian32(kCurveARGBColorAtom),
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// alpha, red
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// green, blue
										// it's black!

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4*11), EndianOrder.flipNativeToBigEndian32(kCurvePathAtom),
			EndianOrder.flipNativeToBigEndian32(1),			// one contour in path
			EndianOrder.flipNativeToBigEndian32(4),			// four points in path
			EndianOrder.flipNativeToBigEndian32(0xffffffff), // all points are off the curve: it's a rounded square! 
			EndianOrder.flipNativeToBigEndian32(0x00640000), EndianOrder.flipNativeToBigEndian32(0x00640000),
			EndianOrder.flipNativeToBigEndian32(0x00C80000), EndianOrder.flipNativeToBigEndian32(0x00640000),
			EndianOrder.flipNativeToBigEndian32(0x00C80000), EndianOrder.flipNativeToBigEndian32(0x00C80000), 
			EndianOrder.flipNativeToBigEndian32(0x00640000), EndianOrder.flipNativeToBigEndian32(0x00C80000),

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4), EndianOrder.flipNativeToBigEndian32(kCurveFillTypeAtom),
		EndianOrder.flipNativeToBigEndian32(gxEvenOddFill),

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4), EndianOrder.flipNativeToBigEndian32(kCurvePenThicknessAtom),
		EndianOrder.flipNativeToBigEndian32(0x100000),
											
		// enable linear gradient for all following atoms
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4), EndianOrder.flipNativeToBigEndian32(kCurveGradientTypeAtom),
		EndianOrder.flipNativeToBigEndian32(kLinearGradient),
		
		// define the gradient: red -> green -> red -> blue									
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + (12*4)), EndianOrder.flipNativeToBigEndian32(kCurveGradientRecordAtom),
										
			EndianOrder.flipNativeToBigEndian32(0xffffffff),	// gradient color record 1:
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// red
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// beginning of gradient
										
			EndianOrder.flipNativeToBigEndian32(0x77770000),	// gradient color record 2:
			EndianOrder.flipNativeToBigEndian32(0xffff0000),	// green
			EndianOrder.flipNativeToBigEndian32(0x00004000),
										
			EndianOrder.flipNativeToBigEndian32(0x3333ffff),	// gradient color record 3:
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// red
			EndianOrder.flipNativeToBigEndian32(0x0000C000),
										
			EndianOrder.flipNativeToBigEndian32(0xffff0000),	// gradient color record 4:
			EndianOrder.flipNativeToBigEndian32(0x0000ffff),	// blue
			EndianOrder.flipNativeToBigEndian32(0x00010000),	// end of gradient

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4), EndianOrder.flipNativeToBigEndian32(kCurveGradientAngleAtom),
			EndianOrder.flipNativeToBigEndian32(0x00450000),	// gradient at 45û angle
		
		// a green rectangle, centered at 40,40, painted with a linear gradient									
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 8), EndianOrder.flipNativeToBigEndian32(kCurveARGBColorAtom),
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// alpha, red
			EndianOrder.flipNativeToBigEndian32(0xffff0000),	// green, blue
										// it's green!

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + (4*11)), EndianOrder.flipNativeToBigEndian32(kCurvePathAtom),
			EndianOrder.flipNativeToBigEndian32(1),			// one contour in path
			EndianOrder.flipNativeToBigEndian32(4),			// four points in path
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// all points are on the curve: it's a rectangle! 
			EndianOrder.flipNativeToBigEndian32(0x00100000), EndianOrder.flipNativeToBigEndian32(0x00100000),
			EndianOrder.flipNativeToBigEndian32(0x00400000), EndianOrder.flipNativeToBigEndian32(0x00100000),
			EndianOrder.flipNativeToBigEndian32(0x00400000), EndianOrder.flipNativeToBigEndian32(0x00400000),
			EndianOrder.flipNativeToBigEndian32(0x00100000), EndianOrder.flipNativeToBigEndian32(0x00400000),

		// disable gradient for all following atoms (since no atom data)
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields), EndianOrder.flipNativeToBigEndian32(kCurveGradientRecordAtom),
									
		// a red rounded square, centered at 50,50
		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 8), EndianOrder.flipNativeToBigEndian32(kCurveARGBColorAtom),
			EndianOrder.flipNativeToBigEndian32(0x3333ffff),	// alpha, red
			EndianOrder.flipNativeToBigEndian32(0x00000000),	// green, blue
										// it's red!

		EndianOrder.flipNativeToBigEndian32(kSizeOfSizeAndTagFields + 4*11), EndianOrder.flipNativeToBigEndian32(kCurvePathAtom),
			EndianOrder.flipNativeToBigEndian32(1),			// one contour in path
			EndianOrder.flipNativeToBigEndian32(4),			// four points in path
			EndianOrder.flipNativeToBigEndian32(0xffffffff), // all points are off the curve: it's a rounded square! 
			EndianOrder.flipNativeToBigEndian32(0x001e0000), EndianOrder.flipNativeToBigEndian32(0x001e0000),
			EndianOrder.flipNativeToBigEndian32(0x00460000), EndianOrder.flipNativeToBigEndian32(0x001e0000),
			EndianOrder.flipNativeToBigEndian32(0x00460000), EndianOrder.flipNativeToBigEndian32(0x00460000),
			EndianOrder.flipNativeToBigEndian32(0x001e0000), EndianOrder.flipNativeToBigEndian32(0x00460000),

		EndianOrder.flipNativeToBigEndian32(kSizeOfZeroAtomHeader), EndianOrder.flipNativeToBigEndian32(kCurveEndAtom),
		};
		
		return IntEncodedImage.fromIntArray(curveSample,EncodedImage.kRowBytesUnknown);
	}
}


