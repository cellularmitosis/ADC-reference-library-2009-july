/*
 
 File: TimeCallbackDemo.java
 
 Abstract: Highlights the TimeCode media services of QuickTime by installing TimeCode call-backs for a movie. 
 
 Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
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
 
 Copyright © 2006 Apple Computer, Inc., All Rights Reserved
 
 */

import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.io.IOException;
import quicktime.app.time.*;
import quicktime.std.clocks.*;
import quicktime.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.std.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.app.view.*;

//Note : When using callback it should be noted that the callbacks need to be properly cleanedup when thier use is over.
// You should take proper care to just cancel the callback if you wish to reschedule it . To do this you should call just cancel() .
// You should take proper care to call cancelAndCleanup() when you are quitting the application itself, this will ensure proper clean up
// of any memory and thread hanging around.
/** 
 * This sample code demonstrates how to use the various TimeCallBack features of QuickTime for Java 
 * including Extremes, rate changes, and time callbacks
 */

public class TimeCallbackDemo extends Frame implements Errors {	
/* main entry point of the application */	
	public static void main(String args[]) {		
		try {			
			System.out.println ("Time Base callback Demo - time callback *ONLY*"); 
			System.out.println ("will work the first time through the movie as eventually");
			System.out.println ("it is rescheduled past the end of the movie and there is"); 
			System.out.println ("no logic to correct that in this code");

			QTSession.open();	
					
			TimeCallbackDemo movieWindow = new TimeCallbackDemo("QT in Java");
			movieWindow.pack();
			movieWindow.show();
			movieWindow.toFront();
		}catch (Exception ex) {
			ex.printStackTrace();
			QTSession.close();
		}			
		
	}
        private QTComponent qtc;	
	private TimeBaseRateCallBack theMoviesRateCallback;
	private TimeBaseExtremesCallBack theMoviesExtremeCallback;
	private TimeBaseTimeJumpCallBack theMoviesTimeJumpCallback;
	private TimeBaseTimeCallBack theMoviesTimeCallback;

	/**
	 * Default constructor 
	 * @param title the title of the window to be created 
	 */
	public TimeCallbackDemo (String title) throws QTException {
		super (title);
		
		createMovie();
		add((Component) qtc);
		pack();

		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				theMoviesRateCallback.cancelAndCleanup();
				theMoviesExtremeCallback.cancelAndCleanup();
				theMoviesTimeJumpCallback.cancelAndCleanup();
				theMoviesTimeCallback.cancelAndCleanup();
				
				QTSession.close();
				System.exit(0);	
			}
			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});
	}

	/**
	 * Opens a movie from a file and installs the various movie callbacks 
	 */
	public void createMovie() throws QTException {
		Media m = null;
		QTFile qtf = null;
		try
		{
			qtf = QTFile.standardGetFilePreview(QTFile.kStandardQTFileTypes);
		}catch (QTException e)
		{
				// catch a userCanceledErr and just exit the program
			if (e.errorCode() != Errors.userCanceledErr)
				e.printStackTrace();
			else
				System.out.println ("UserCanceled : Application needs media file to run. Quitting....");
            QTSession.close();
            System.exit(1);
		}
		Movie mov = Movie.fromFile (OpenMovieFile.asRead (qtf));
                MovieController mc = new MovieController(mov);
                qtc = QTFactory.makeQTComponent(mc);
                
		TimeBase theMoviesTimeBase = mov.getTimeBase();
		
		theMoviesRateCallback = new TimeBaseRateCallBack(theMoviesTimeBase, 1.0F, StdQTConstants.triggerRateChange); // this callback is triggered
		theMoviesRateCallback.callMeWhen ();																			 // when the rate of the movie changes

		theMoviesExtremeCallback = new TimeBaseExtremesCallBack(theMoviesTimeBase, StdQTConstants.triggerAtStop );	// this callback is triggered when the movie stops
		theMoviesExtremeCallback.callMeWhen ();
		
		theMoviesTimeJumpCallback = new TimeBaseTimeJumpCallBack(theMoviesTimeBase);					// this callback is triggered when there is a jump in the timebase
		theMoviesTimeJumpCallback.callMeWhen ();
		
			// this schedules the time callback once every 2 seconds
		theMoviesTimeCallback = new TimeBaseTimeCallBack(theMoviesTimeBase, 1, 2, StdQTConstants.triggerTimeEither); // this callback is triggered at a specific time interval
		theMoviesTimeCallback.callMeWhen ();
		
			//Using the Timer class you can get rescheduled properly and get callbacks at the set intervals. It uses the same callback 
			//mechanism of internally of the TimeCallback.
			//Its recomended to use this Timer class to do callbacks , which would take care of the time base time changes and 
			//recscheduling of the tickle method .
		Timer timer = new Timer (1, 2, new Tickler(), mov);		
		timer.setActive(true);
	}
	

	/**
	 * This class extends the RateCallBack class and provides an execute routine that is invoked by the callback
	 * when the rate of the movie changes
	 */

	public class TimeBaseRateCallBack extends RateCallBack {
		public TimeBaseRateCallBack (TimeBase tb, float rate, int flag) throws QTException {
			super(tb, rate, flag);
		}
		public void execute () {
			try {
				System.out.println( "---  RateCallBack@ratechange---" ); 
				cancel();
				callMeWhen ();
			}catch (StdQTException e) {}
		}
	}

	/**
	 * This class implements a method that is called when the movie stops
	 */
	class TimeBaseExtremesCallBack extends quicktime.std.clocks.ExtremesCallBack {
		/** 
		* @param mov The movie to monitor.
		*/
		public TimeBaseExtremesCallBack(TimeBase tb, int flag ) throws QTException {
			super(tb, flag);
		}
              
		public void execute() {
			try {
				System.out.println( "---  ExtremesCallBack@stop---" ); 
		  	    cancel();
				callMeWhen ();
			}catch (StdQTException e) {}
		}
	}       

	/**
	 * This class extends the TimeJumpCallBack class to provide a method that is called when the
	 * timebase of the movie changes (IE, if the user clicks in the movie controller)
	 */
	class TimeBaseTimeJumpCallBack extends quicktime.std.clocks.TimeJumpCallBack {

		public TimeBaseTimeJumpCallBack (TimeBase tb) throws QTException {
			super (tb);
		}
	
		public void execute() {
			try {
				System.out.println( "---  TimeJumpCallBack---" ); 
				cancel();
				callMeWhen ();
			}catch (StdQTException e) {}
		}
	}
	/**
	 * This class extends the TimeCallBack class to provide a method that is called when a
	 * specific interval of time elapses.
	 */
	class TimeBaseTimeCallBack extends quicktime.std.clocks.TimeCallBack {

		public TimeBaseTimeCallBack (TimeBase tb, int scale, int value, int flags) throws QTException {
			super (tb, scale, value, flags);
			period = value;
			theTimeBase = tb;
		}
		
		int period;
		TimeBase theTimeBase;
		
		public void execute() {
			try {
				System.out.println( "---  TimeCallBack@triggerTimeEither--- called at:" + timeWhenCalledMsecs + "msecs"); 
				cancel();
				value += period;
				callMeWhen ();
			}catch (StdQTException e) {}
		}
	}	
	
	public class Tickler implements Ticklish {
	
		public void timeChanged (int newTime) throws QTException {
			System.out.println ("* * * * Timer Class * * * timeChanged at:" + newTime);
		}
		
		public boolean tickle (float er, int time) throws QTException {
			System.out.println ("* * * * Timer Class  * * * tickle at:" + time);
			return true;		
		}		
	}

}	

	