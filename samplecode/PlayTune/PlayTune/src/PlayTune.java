/*

File: PlayTune.java

Abstract: Example usage of NoteChannels in QTJ

Version: 1.2

© Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

import java.awt.*;
import java.awt.event.*;
import java.io.*;

import quicktime.*;
import quicktime.app.view.*;
import quicktime.app.time.*;
import quicktime.io.*;
import quicktime.qd.*;
import quicktime.sound.*;
import quicktime.std.movies.*;
import quicktime.std.movies.media.*;
import quicktime.std.music.*;
import quicktime.std.*;
import quicktime.std.qtcomponents.*;

import quicktime.util.*;

// This is a sample of using the TunePlayer translated from the Develop 21 article on QTMusic
public class PlayTune extends Frame	implements StdQTConstants, SoundConstants {	
	public static void main (String args[]) 	{
		try { 
			QTSession.open();
			PlayTune app = new PlayTune("Play Tune");
			app.pack();
			app.show();
			app.toFront();
		} catch (Exception e) {
			e.printStackTrace();
			QTSession.close();
		}
	}

	TunePlayer 		 aTunePlayer;
	MusicData 		 aTune;
	AtomicInstrument myInstrument;
	NoteChannel 	 myNoteChannel;
	
	PlayTune (String title)	throws Exception {
		super (title);
	// Make up a tune player and set the note channels that will be used
	// the order of the NoteChannels in the array when the are added
	// to the TunePlayer is the part those channels take on
	// thus first element of NC array is the NoteChannel for the first part
	    aTunePlayer = new TunePlayer();
		NoteChannel[] noteChanArray = new NoteChannel[2];
		myNoteChannel = noteChanArray[0] = new NoteChannel (1, 4);	//a piano with 4 voice polyphony
		
		File f = QTFactory.findAbsolutePath ("sin440.aif");
		myInstrument = createAtomicInstrument (new QTFile (f));

		noteChanArray[1] = myInstrument.newNoteChannel (0); //new NoteChannel (41, 3);//a violin with 3 voice polyphony
		aTunePlayer.setNoteChannels (noteChanArray);
		
	// this is the tune itself
		aTune = testSequenceCreation();
	
	// create a button to start the tune playing
		Button playBtn = new Button ("Play Tune");
		playBtn.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent ae) {
				try {
					aTunePlayer.queue (aTune, 1.0F, 0, 0x7FFFFFFF, 0);
				} catch (QTException e) {
					e.printStackTrace();
				}
			}
		});
		
		add (playBtn, "North");
		Button saveBtn = new Button ("Save Tune");
		saveBtn.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent ae) {
				makeMovie();
			}
		});
		add (saveBtn, "Center");

		Button rebuildBtn = new Button ("Rebuild Tune");
		rebuildBtn.addActionListener (new ActionListener () {
			public void actionPerformed (ActionEvent ae) {
				rebuildTune();
			}
		});
		add (rebuildBtn, "South");

		addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent e) {
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e) { 
				System.exit(0);
			}
		});	   

	}

	static final int 
				// why 600 -> that is the scale of the media when we add it to the movie
			kNoteDuration = 240, // ¥ in 600ths of a second
			kRestDuration = 300, // ¥ in 600ths -- tempo will be 120 bpm
			
			// 21 music events plus 1 for end of sequence marker
			our_sequence_length = (22 * 4), // ¥ bytes
				//why 9? - the tune we have has 9 rest's in it - the "rest" event essentially
				//sets up the grid and the duration
			our_sequence_duration = (9 * kRestDuration); // ¥ 600ths
	
		// create a tune
	static MusicData testSequenceCreation () throws QTException {
		
		MusicData tune = new MusicData (our_sequence_length);
		
		tune.setMusicEvent (0, MusicData.stuffNoteEvent(1, 60, 100, kNoteDuration)); // ¥ piano C
		tune.setMusicEvent (1, MusicData.stuffRestEvent(kRestDuration));
		tune.setMusicEvent (2, MusicData.stuffNoteEvent(2, 60, 100, kNoteDuration)); // ¥ violin C
		tune.setMusicEvent (3, MusicData.stuffRestEvent(kRestDuration));
		tune.setMusicEvent (4, MusicData.stuffNoteEvent(1, 63, 100, kNoteDuration)); // ¥ piano
		tune.setMusicEvent (5, MusicData.stuffRestEvent(kRestDuration));
		tune.setMusicEvent (6, MusicData.stuffNoteEvent(2, 64, 100, kNoteDuration)); // ¥ violin
		tune.setMusicEvent (7, MusicData.stuffRestEvent(kRestDuration));
		// ¥ Make the 5th and 6th notes much softer, just for fun.
		tune.setMusicEvent (8, MusicData.stuffNoteEvent(1, 67, 60, kNoteDuration)); // ¥ piano
		tune.setMusicEvent (9, MusicData.stuffRestEvent(kRestDuration));
		tune.setMusicEvent (10, MusicData.stuffNoteEvent(2, 66, 60, kNoteDuration)); // ¥ violin
		tune.setMusicEvent (11, MusicData.stuffRestEvent(kRestDuration));
		tune.setMusicEvent (12, MusicData.stuffNoteEvent(1, 72, 100, kNoteDuration)); // ¥ piano
		tune.setMusicEvent (13, MusicData.stuffRestEvent(kRestDuration));
		tune.setMusicEvent (14, MusicData.stuffNoteEvent(2, 73, 100, kNoteDuration)); // ¥ violin
		tune.setMusicEvent (15, MusicData.stuffRestEvent(kRestDuration));
		tune.setMusicEvent (16, MusicData.stuffNoteEvent(1, 60, 100, kNoteDuration)); // ¥ piano
		tune.setMusicEvent (17, MusicData.stuffNoteEvent(1, 67, 100, kNoteDuration)); // ¥ piano
		tune.setMusicEvent (18, MusicData.stuffNoteEvent( 2, 63, 100, kNoteDuration)); // ¥ violin
		tune.setMusicEvent (19, MusicData.stuffNoteEvent(2, 72, 100, kNoteDuration)); // ¥ violin
		tune.setMusicEvent (20, MusicData.stuffRestEvent(kRestDuration));
			// ¥ end-of-sequence marker already in MusicData object
		//tune.setMusicEvent (21, MusicData.stuffMarkerEvent (0, 0));
		
		return tune;
	}

		//makes a TuneHeader up from the current state of the TunePlayer's note channels
	MusicData makeTuneHeaderFromTunePlayer () throws QTException {
	//	Make a MusicHeader with a note request and an atomic instrument
	//	The default constructor places the end marker event at the end of a MusicData allocated size
			
			//we need to find out how many 4byte values are required to fit our ai in
		int instSize = myInstrument.getSize() / 4;
		if (myInstrument.getSize() % 4 != 0)
			instSize++;
			//add 2 for the header and footer music events
		int aiEventLength = instSize + 2;
		
		int endMarkerSize = 4;
		
		MusicData musicHeader = new MusicData (MusicData.kNoteRequestHeaderEventLength * 4 + aiEventLength * 4 + endMarkerSize);

	    	// get its note request
	    NoteRequest nr = myNoteChannel.getNoteRequest();
	    	// stuff the note request into the music header
		musicHeader.setNoteRequest (0, 1, nr);
				
			// 2 is the part number for the instrument
			// offset into music data past the first note request event
		musicHeader.setAtomicInstrument (MusicData.kNoteRequestHeaderEventLength, 2, myInstrument);
			
		return musicHeader;
	}

	void makeMovie () {
		try {
			FileDialog fd = new FileDialog (this, "Save Movie As...", FileDialog.SAVE);
			fd.show();
			if(fd.getFile() == null)
				return;	//not saving at this time
			
			movieFile = new QTFile(fd.getDirectory() + fd.getFile());
			Movie theMovie = Movie.createMovieFile (movieFile,
								kMoviePlayer, 
								createMovieFileDeleteCurFile | createMovieFileDontCreateResFile);
			
			Track t = theMovie.newTrack (0, 0, 1.0F);
			MusicMedia musicMedia = new MusicMedia (t, 600);
			MusicDescription musicDesc = new MusicDescription ();
			
			// create the TuneHeader from the TunePlayer
			// then add it to the end of the MusicDescription
			musicDesc.setTuneHeader (makeTuneHeaderFromTunePlayer());
			
			// add the MD and the tune to the music media.
			musicMedia.beginEdits();	
			musicMedia.addSample (aTune, 0, aTune.getSize(), our_sequence_duration, musicDesc, 1, 0);
			musicMedia.endEdits();
			
			// insert the media into our track
			t.insertMedia (0, 0, our_sequence_duration, 1.0F);
			
			// save the movie to the created file
			OpenMovieFile outStream = OpenMovieFile.asWrite (movieFile); 
			theMovie.addResource (outStream, movieInDataForkResID, movieFile.getName());
			outStream.close();
			
			System.out.println ("Finished");
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
	
	QTFile movieFile;
		
		//rebuild the tune be reading in the previously saved instruments
	void rebuildTune () {
		try {
			if (movieFile == null) {
				System.out.println ("Save Tune to Movie first");
				return;
			}
				
			Movie mov = Movie.fromFile (OpenMovieFile.asRead(movieFile));
			int numTracks = mov.getTrackCount();
			MusicMedia musicMedia = null;
			
			// add the audio media from each track
			for (int i = 1; i <= numTracks; i++) {
				Track curTrack = mov.getIndTrack(i);
				Media trackMedia = Media.getTrackMedia(curTrack);
				if (trackMedia instanceof MusicMedia)
					musicMedia = (MusicMedia)trackMedia;
			}
			
			if (musicMedia == null)
				throw new QTException ("Music Media not found");
			
			MusicDescription mdesc = musicMedia.getMusicDescription(1);
			System.out.println (mdesc);
			
			MusicData theMusicData = mdesc.getTuneHeader();
			System.out.println (theMusicData);
			
				//we know there are two instruments in the file we just created.
			NoteChannel[] noteChanArray = new NoteChannel[2];
		
				//we parse the tune header music data object.
				//pulling out any note requests and atomic instruments
				// we find in the header
			int currentEventIndex = 0;
			int i = 0;
				//to loop through the entire tune header 
				// we loop until the currentIndex is greater than the size (in bytes) of the 
				// music data / 4 (gives us the number of music events) - 1 (to ignore the end marker event)
			while (currentEventIndex < theMusicData.getSize() / 4 - 1) {
				int eventHeader = theMusicData.getMusicEvent(currentEventIndex);
					System.out.print ("event header:" + Integer.toHexString(eventHeader));
				int eventLength =  MusicData.generalLength (eventHeader);
					System.out.print (",event length:" + eventLength);
				int eventFooter = theMusicData.getMusicEvent (eventLength + currentEventIndex - 1);
					System.out.print (",event footer:" + Integer.toHexString(eventFooter));
				int eventSubtype = MusicData.generalSubtype_Footer (eventFooter);
					System.out.print (",event subtype:" + eventSubtype);
				System.out.println ("");
					
					//based on the event sub type we have either
					//stored a note request or an atomic instrument
					// in the movie -> so we read out from the music data the appropriate type
					// the note request/atomic inst. will start at the index of the general event + 1 
					// (to get past the general event header)
				switch (eventSubtype) {
					case kGeneralEventNoteRequest:
						NoteRequest nr = theMusicData.getNoteRequest (currentEventIndex + 1);
						System.out.println (nr);
						myNoteChannel = noteChanArray[i] = new NoteChannel (nr);
						break;
					case kGeneralEventAtomicInstrument:
						AtomicInstrument ai = theMusicData.getAtomicInstrument (currentEventIndex + 1);
						System.out.println (ai);
						 noteChanArray[i] = myInstrument.newNoteChannel (0);
						break;
					default:
						break;	//skip this general event
				}
					//move our index to the next general event header
				currentEventIndex += eventLength;
				i++;
			}
			
			if (noteChanArray[0] == null || noteChanArray[1] == null)
				throw new QTException ("Problem reading instruments");
				
				//swap the parts that the note channels play
				// originally we had the piano playing the first note
				// after rebuilding the sin wave will play the first note
			NoteChannel temp = noteChanArray[0];
			noteChanArray[0] = noteChanArray[1];
			noteChanArray[1] = temp;
			
			aTunePlayer.setNoteChannels (noteChanArray);
			
		} catch (QTException e) {
			e.printStackTrace();
		}
	}
			

	//////////
	//
	// createAtomicInstrument
	// Create a custom atomic instrument using the sampled sound data in the specified resource handle.
	//
	// A custom instrument is defined by a QTAtom structure containing appropriate atoms (hence the name
	// "atomic" instrument). See the QT3.0 book "QuickTime Music Architecture" for information about the
	// structure of an atomic instrument. Page 15 has a good picture of the required structure.
	//
	//////////

	static AtomicInstrument createAtomicInstrument (QTFile f) throws QTException, IOException {
			// some constants that are used
		final int kCustomInstAtomID = 11;
		final int 
				kMIDINoteValue_Lowest = 0,
				kMIDINoteValue_Highest = 127,
				kMIDINoteValue_A440 = 69;
		
		//////////
		//
		// get information about the sound; we'll use this later to construct a sample description atom
		//
		//////////
		SndInfo info = SndInfo.parseAIFFHeader (OpenFile.asRead (f));
		SoundComponentData mySndInfo = info.sndData;
/*
			//assume the AIFF sound data is uncompressed as
			//atomic instruments ONLY accept uncompressed sound
			// so this call and it's consequent usage is not required
		CompressionInfo myCompInfo = CompressionInfo.get (SoundConstants.notCompressed, 
															mySndInfo.getFormat(), 
															mySndInfo.getNumChannels(), 
															mySndInfo.getSampleSize());
		System.out.println (myCompInfo);
*/		
		System.out.println (mySndInfo);
			
		//////////
		//
		// create an atom container with atoms that describe the custom instrument
		//
		//////////
		
		// create a new, empty atom container
		AtomicInstrument myInstrument = new AtomicInstrument();
		
		// insert a tone description atom, which contains a tone description structure
		ToneDescription myToneDesc = new ToneDescription ();
		myToneDesc.setSynthesizerType (kSoftSynthComponentSubType);
		myToneDesc.setInstrumentName ("Sin 440");
		myToneDesc.setInstrumentNumber (1);//kInst_Custom
		myInstrument.insertChild (Atom.kParentIsContainer, kaiToneDescType, 1, 1, myToneDesc);
		
		// insert a note request atom, which contains a note request structure;
		// this atom is optional; if it's not present, QTMA assumes some reasonable values
		NoteRequestInfo myNoteInfo = new NoteRequestInfo ();
		myInstrument.insertChild (Atom.kParentIsContainer, kaiNoteRequestInfoType, 1, 1, myNoteInfo);

		// insert a knob list atom;
		// this atom is optional; if it's not present, QTMA assumes some reasonable values
		InstKnobList myKnobList = new InstKnobList ();
		myInstrument.insertChild (Atom.kParentIsContainer, kaiKnobListType, 1, 1, myKnobList);

		// insert a key range information atom; this is the parent of the sample description atom
	 	Atom myKeyRangeInfoAtom = myInstrument.insertChild (Atom.kParentIsContainer, kaiKeyRangeInfoType, 1, 1);
		
			if (myKeyRangeInfoAtom.getAtom() == 0)
				throw new QTException ("Empty KeyRange Atom");

			//this is defined as a big-endian struct so QTJava automatically flips the fields
		InstSampleDesc	mySampleDesc = new InstSampleDesc ();
		
		mySampleDesc.setDataFormat (mySndInfo.getFormat());
		mySampleDesc.setNumChannels (mySndInfo.getNumChannels());
		mySampleDesc.setSampleSize (mySndInfo.getSampleSize());
		mySampleDesc.setSampleRate (mySndInfo.getSampleRate());

			// define the characteristics of the sampled sound	
		mySampleDesc.setSampleDataID (kCustomInstAtomID);
		mySampleDesc.setOffset (0);
		mySampleDesc.setNumSamples (mySndInfo.getSampleCount());
			
			//set looping characteristics
		mySampleDesc.setLoopType (kMusicLoopTypeNormal);
		mySampleDesc.setLoopStart (0);
		mySampleDesc.setLoopEnd (mySndInfo.getSampleCount());

			//set pitch characteristics
		mySampleDesc.setPitchNormal (kMIDINoteValue_A440);
		mySampleDesc.setPitchLow (kMIDINoteValue_Lowest);
		mySampleDesc.setPitchHigh (kMIDINoteValue_Highest);
			
			System.out.println (mySampleDesc);
			
			// insert the sample description atom
		myInstrument.insertChild (myKeyRangeInfoAtom, kaiSampleDescType, 1, 1, mySampleDesc);
		
		// insert a sample information atom; this is the parent of the sample data atom and must have
		// the same atom ID specified by the sampleDataID field of the instrument sample description 
		Atom mySampleInfoAtom = myInstrument.insertChild (Atom.kParentIsContainer, kaiSampleInfoType, kCustomInstAtomID, 0);

		if (mySampleInfoAtom.getAtom() == 0)
			throw new QTException ("Sample Info Atom is 0");	

		// insert a sample data atom into the sample information atom; this atom contains the actual
		// sample data that defines the custom instrument; the format of the sample data is defined
		// by the corresponding sample description atom
				// was using myCompInfo.getBytesPerSample() but as this is not-compressed seems redundant
				//if worried about dealing with 12/18/20 bit sample sizes could do (getSampleSize() + 7) / 8 => will truncate to correct value
		int mySampleDataSize = mySndInfo.getSampleCount() * (mySndInfo.getSampleSize() / 8); 
			//read just the sample data into memory
			// it is sample data size and is found at the data offset location in the file
			// as returned by the ParseAIFF header call
			FileInputStream fis = new FileInputStream (f);
			byte[] ar = new byte [mySampleDataSize];			
			fis.skip (info.dataOffset);
			fis.read (ar, 0, mySampleDataSize);
			QTByteObject mySampleData = QTByteObject.fromArray (ar);
		myInstrument.insertChild (mySampleInfoAtom, kaiSampleDataType, 1, 0, mySampleData);
		
		// insert an instrument info atom; this is a parent atom
		Atom myInstInfoAtom = myInstrument.insertChild (Atom.kParentIsContainer, kaiInstInfoType, 1, 0);
		
		if (myInstInfoAtom.getAtom() == 0)
			throw new QTException ("InstInfoAtom is 0");
			
		// insert a picture atom into the instrument info atom
		Pict myPictHandle = Pict.fromFile (QTFactory.findAbsolutePath ("images/Ship01.pct"));
		myInstrument.insertChild (myInstInfoAtom, kaiPictType, 1, 0, myPictHandle);

			//doesn't matter what format we have the strings in 
			// becase the insert child call for StringHandle will correctly insert just the string character's (bytes)
		StringHandle myWriter = new StringHandle ("This space for rent!", StringHandle.kCStringFormat);
		StringHandle myRights = new StringHandle ("Copyright 1998 by Apple Computer, Inc.", StringHandle.kCStringFormat);
		StringHandle myOthers = new StringHandle ("Custom Atomic Instrument.", StringHandle.kCStringFormat);

		// insert a writer atom into the instrument info atom
		myInstrument.insertChild (myInstInfoAtom, kaiWriterType, 1, 0, myWriter);
		
		// insert a copyright atom into the instrument info atom
		myInstrument.insertChild (myInstInfoAtom, kaiCopyrightType, 1, 0, myRights);
		
		// insert an other info atom into the instrument info atom
		myInstrument.insertChild (myInstInfoAtom, kaiOtherStrType, 1, 0, myOthers);
			
		return myInstrument;
	}

// code not used in demo - here for your edification
	static MusicData testHeaderCreation () throws QTException {
	    final int kNumberOfNoteRequestsInHeader = 2;

	    // request for piano polyphony 4
		NoteRequest aRequest = new NoteRequest(1, 4);

	    // request for violin polyphony 3
		NoteRequest anotherRequest = new NoteRequest(41, 3);
		anotherRequest.setPolyphony(3);

	//	Make a MusicHeader with 2 note requests
	//	The default constructor places the end marker event at the end of a MusicData allocated size
		MusicData musicHeader = new MusicData (4 + kNumberOfNoteRequestsInHeader * (MusicData.kNoteRequestHeaderEventLength * 4));

	    // stick our request into it for first part
	    musicHeader.setNoteRequest(0, 1, aRequest);
	    // stick our request into it for second part
	    musicHeader.setNoteRequest(MusicData.kNoteRequestHeaderEventLength, 2, anotherRequest);
	    // end-of-sequence marker is already in the MusicData
		
		return musicHeader;
	}
}