/*
 
 File: JavaMappingSupport.java
 
 Abstract: Java code to manipulate the map displayed in the JavaFrameView.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
 */

import java.awt.*;
import javax.swing.SwingUtilities;
import org.jdesktop.swingx.mapviewer.*;
import org.jdesktop.swingx.*;

public class JavaMappingSupport {
	private Frame fEmbeddedFrame;
	private JXMapKit fMapView;
	
	private static GeoPosition sHomeCoordinates = new GeoPosition(51.47647, -0.00343);
	
	private JavaMappingSupport(Frame hostFrame) {
		// It's mandatory to create a heavyweight java.awt.Panel and place it in the Frame.
		// Otherwise, events won't be delivered to any lightweight components in the window.
		Panel panel = new Panel();

		// openstreetmap.org tile server URL provided by Joshua Marinacci		
        final int max = 17;
        TileFactoryInfo info = new TileFactoryInfo(0,max,max,
                256, true, true, // tile size is 256 and x/y orientation is normal
                "http://tile.openstreetmap.org",//5/15/10.png",
                "x","y","z") {
            public String getTileUrl(int x, int y, int zoom) {
                zoom = max-zoom;
                String url = this.baseURL +"/"+zoom+"/"+x+"/"+y+".png";
                return url;
            }
        };

        TileFactory tf = new DefaultTileFactory(info);
		fMapView = new JXMapKit();
        fMapView.setTileFactory(tf);
        fMapView.setZoom(4);
		fMapView.setAddressLocation(sHomeCoordinates);		
		panel.setLayout(new BorderLayout());
		panel.add(fMapView, BorderLayout.CENTER);
		hostFrame.add(panel, BorderLayout.CENTER);
	}

	public static Object setupFrame(Frame hostFrame) {
		return new JavaMappingSupport(hostFrame);
	}	
		
	public void toggleZoomSlider(final boolean inOn) {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				fMapView.setZoomSliderVisible(inOn);
			}
		});
	}
	
	public void toggleZoomButtons(final boolean inOn) {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				fMapView.setZoomButtonsVisible(inOn);
			}
		});
	}
	
	public void returnHome() {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				fMapView.setAddressLocation(sHomeCoordinates);		
			}
		});
	}
	
	public void setNewCenterPoint(final double inLatitude, final double inLongitude) {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				GeoPosition newCenter = new GeoPosition(inLatitude, inLongitude);
				fMapView.setCenterPosition(newCenter);
				fMapView.setAddressLocation(newCenter);
			}
		});		
	}
		
}
