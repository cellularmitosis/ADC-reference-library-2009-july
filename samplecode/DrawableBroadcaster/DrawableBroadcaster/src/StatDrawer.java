/*
 * QuickTime for Java SDK Sample Code
 *
 * Usage subject to restrictions in SDK License Agreement
 * Copyright: © 2000 Apple Computer, Inc.
 */
 
import quicktime.app.time.*;
import quicktime.std.*;
import quicktime.*;
import quicktime.std.clocks.*;
import java.text.DecimalFormat;
import java.awt.*;

/* This class retreives statistical information about the presentation and displays it in the 
 * main application window 
 */
public class StatDrawer extends Tasking {
	private static TaskThread updateThread = new TaskThread ("Default SG Tasker", 600);
	
	/* Constructor- sets up the default tasking thread */
	public StatDrawer (SimplePres p) {
		this.pres = p;
		setDefaultTasker(updateThread);
	}
	
	/* This method resets the active SimplePres object to get data from */
	public void setPres (SimplePres p) {
		boolean tasking = (getTasker() == null);
		stopTasking();
		this.pres = p;
		if ( tasking )
			startTasking();
	}
	
	/**
	 * This method performs idle processing for the SGDrawer and will be automatically
	 * called if this object is added to the TaskThread object.
	 * It the sequence grabber has completed its capture task this method
	 * will stop tasking the SGDrawer, stop the sequence grabber and call
	 * the SGCaptureCallback if registered to notify the application that Sequence grabbing
	 * is complete.
	 * @see quicktime.util.TaskThread
     */
	public synchronized final void task() throws QTException { 
		TimeBase t = pres.pres.getTimeBase();
		TimeRecord r = t.getTRTime();
		timeLabel.setText( timeRecordToHHMMSSFF(r, 30, ':', '.'));
		rateLabel.setText (new Integer(pres.pres.getTotalDataRate()).toString());
	}	
	
   /**
	* Returns the specified time record as a hour-minutes-seconds-frames string
	* according to the specified frames per second value. The first char
	* argument is used to separate hours, minutes and seconds and the second
	* char is used to separate the frames from the rest of the HHMMSSFF string.
	* A null time record or a zero frames-per-second value will result in a null
	* returned by the function. Negative frames-per-second values are converted
	* to positive integers (absolute value). If frames-per-second exceeds 1000
	* 1000 will be used.
	*
	* @author Ilias Argyriou <ilias_argyriou@hotmail.com>
	*
	* Examples:
	* timeRecordToHHMMSSFF(<TimeRecord>, 10, '-', '-')  -> "HH-MM-SS-F"
	* timeRecordToHHMMSSFF(<TimeRecord>, 30, ':', '.')  -> "HH:MM:SS.FF"
	* timeRecordToHHMMSSFF(<TimeRecord>, 500, ';', '.') -> "HH;MM;SS.FFF"
	*/
	public static String timeRecordToHHMMSSFF(TimeRecord tr, int fps, char ss, char fs) {
		if (tr == null || fps == 0) {
			return null;
		}

		fps = Math.abs(fps);

		// If frames-per-second exceeds 1000, 1000 will be used.
		if (fps > 1000) {
			fps = 1000;
		}

		tr.convertTimeScale(fps);

		StringBuffer sb = new StringBuffer();

		long value = tr.getValue();
		int scale = tr.getScale();

		if (scale <= 0) {
			return null;
		}

		long h = value / (scale * 3600);
		long m = (value / (scale * 60)) % 60;
		long s = (value / scale) % 60;
		long f = value % scale;

		sb.append(DOUBLE_DIGIT_DF.format(h));
		sb.append(ss);
		sb.append(DOUBLE_DIGIT_DF.format(m));
		sb.append(ss);
		sb.append(DOUBLE_DIGIT_DF.format(s));
		sb.append(fs);

		if (fps <= 10) {
			sb.append(SINGLE_DIGIT_DF.format(f));
		} else if (fps <= 100) {
			sb.append(DOUBLE_DIGIT_DF.format(f));
		} else if (fps <= 1000) {
			sb.append(TRIPLE_DIGIT_DF.format(f));
		}

		return sb.toString();
	}


//_________________________ INSTANCE VARIABLES
	private SimplePres pres;
	public Label timeLabel, rateLabel;
		
		/** DecimalFormats used to display integer values as strings. */
	private static DecimalFormat SINGLE_DIGIT_DF = new DecimalFormat("0");
	private static DecimalFormat DOUBLE_DIGIT_DF = new DecimalFormat("00");
	private static DecimalFormat TRIPLE_DIGIT_DF = new DecimalFormat("000");

}