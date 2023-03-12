README - QTSETMOVIEAUDIODEVICE
Version 1.0 (2/22/06)

OVERVIEW

QTSetMovieAudioDevice is a simple sample which demonstrates how to create a QTAudioContext for a given audio output device and then target a movie to render to this audio context. 

DETAILS

To accomplish this, first use native Windows DirectX APIs to enumerate a list of all available sound output devices. Next, call the QuickTime QTAudioContextCreateForAudioDevice API to create a QuickTime Audio Context (QTAudioContext) from either a device GUID or device name. 

Note -- you must have QT 7.0.4 or better installed to create a QuickTime Audio Context from a GUID.

Finally, call SetMovieAudioContext to target the movie to render to the QuickTime Audio Context.

HOW THE APPLICATION WORKS

Simply launch the application and do the following:
 - Use the popup to select the desired sound output device
 - Choose a QuickTime movie file to play
 - Select the "Play Movie" button to play the movie through the selected sound output device