WhackedTV

WhackedTV is a replacement for the venerable Carbon HackTV.  It shows 
how to use the Sequence Grabber API's to capture movies from external 
video and audio sources, just as HackTV, but adds many important new
features, including simultaneous capture from multiple SGChannel's, use
of the new SGAudioChannel, video preview using ICMDecompressionSession's
and OpenGL.


Required: QuickTime 7.  WhackedTV uses the SGAudioChannel instead of the
SoundMediaType SGChannel to record audio.  The SGAudioMediaType channel
became available in QuickTime 7.  The app runs a Gestalt check and will
not run without QuickTime 7.  In order to insert Audio Unit FX into the
audio recording chain, Mac OS X 10.4 (Tiger) is required, though the app
will run on Mac OS X 10.3.9 (where the "Audio FX" button is disabled).
To capture video, a camera with a vdig is required (such as a DV or
iSight camera).  To capture audio, an audio device capable of receiving
input using the CoreAudio interfaces is required (built-in audio on 
most Macintosh computers works fine).


General points of interest:
	* Includes a set of Cocoa wrappers to Sequence Grabber classes
	* The app itself is written in Cocoa
	* Captures movies using Sequence Grabber
	* Performs real-time preview of audio and video
	* Supports arbitrary numbers of source SGChannels
	* Supports saving of Sequence Grabber settings files
	* Supports restoration of previous captures from saved Sequence Grabber 
	  settings files
	* Demonstrates proper procedure for ensuring Spotlight doesn't prematurely
	  index a file to which Sequence Grabber is still recording.  The file
	  path given to Sequence Grabber has an appended ".noindex" extension,
	  which is removed as a post-processing step.
	  
Video-specific points of interest:
	* Demonstrates video preview outside the Sequence Grabber using
	  the SGDataProc -> ICMDecompressionSession -> CIImage -> NSOpenGLView 
	  sub-class
	* Supports real-time scaling of video preview (live-resizing)
	* Supports setting video preview codec quality (low, normal, high, etc.)
	* Supports dynamic throttling of video preview fps
	* Shows standard SGSettingsDialog for video channel configuration
	
Audio-specific points of interest:
	* Demonstrates use of the new SGAudioChannel
	* Includes a sample SGSettingsDialog panel UI for the SGAudioChannel
	  (The QT 7 SGAudioChannel implementation does not implement the
	  SGSettingsDialog interface).
	* Demonstrates getting/setting most SGAudioChannel properties, including:
		** Listing record devices
		** Selection of record device
		** Selection of record device input selection
		** Selection of record device stream format
		** Setting of record device master and per-channel gain 
		   (in hardware or software)
		** Enabling/disabling source channels on record device
		** Assignment of audio channel labels to source channels
		** Solo'ing and muting of source record channels
		** Record device per-channel level metering
		** Listing of preview devices
		** Selection of preview device
		** Selection of preview device output selection
		** Selection of preview device stream format
		** Setting of preview device master gain
		** Hardware playthru or software real-time preview
		** Selection of output format (including VBR formats,
		   sample rate conversion, down-mixing, etc.)
	* Demonstrates insertion of FX AudioUnits into the SGAudioChannel record
	  chain using the SGAudioChannelCallback mechanism
	  
	  
	  
	  
It is important to adopt the new SGAudioChannel over the SoundMediaType 
SGChannel, as the SoundMediaType channel is in maintenance mode (not
deprecated in the headers, but not undergoing any active development either).
The best sources of information about the SGAudioChannel (besides the code
within this sample app) are QuickTimeComponents.h's inline documentation,
"What's New in QuickTime 7" PDF, and the QuickTime 7 developer documentation.