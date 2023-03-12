QTAudioContextInsertApp
------------------------
The QTAudioContextInsertApp application demonstrates the use of the QuickTime Audio Context Insert APIs which enable a client application to tap into and perform custom processing on QuickTime's audio stream during playback or movie audio extraction. The application demonstrates both movie-level as well as track-level inserts.
	
This application repurposes code from the QTKitPlayer sample application (see Apple Documentation Reference Library for HTML and PDF guides that walk through the creating the simple QTKitPlayer application) and the QTAudioExtractionPanel sample application (refer to the Apple Developer Sample Code area to download this sample app).

The sample code in this Xcode project is intended as a learning example for QuickTime programmers who would like to make use of the Audio Context Inserts feature introduced in QuickTime for Mac OS X v10.5.

Getting Started
---------------
Before building your QTAudioContextInsertApp project or running the executable, be sure that you are running Mac OS X v10.5, and using XCode 2.2 or later (preferably XCode 3.0), Interface Builder 2.5 or later, and have the QuickTime Kit framework, which is in the Mac OS X /System/Library/Frameworks directory as QTKit.framework.

Running The Application:
To see the Audio Context Inserts in action during:

[A] Playback:
1. Open a movie using File -> Open.
2. Open the Audio Context Insert Window from the Window menu (Shift-Cmd-I ).
3. In the "Apply Insert effect to:" popup list, select whether to attach the insert effect to the movie or a specific audio track.
4. Select input/output layouts and underlying processing Audio Unit for the Audio Context Insert. This configures your insert.
5. Play the movie. The insert effect, selected in step (4) is applied to the playback of the movie/track selected in step (3) using the Audio Context Inserts APIs . 

[B] Extraction:
Follow steps 1 to 4 from the Playback section above
4. Open the Audio Extraction Window from the Window menu (Shift-Cmd-E).
5. Select the extraction channel layout, start time and duration, and then either preview the extraction or export to file. The insert that was configured in the Audio Context Insert Window in step (4) is applied to the audio stream of the movie/track selected in step (3) during the movie audio extraction.