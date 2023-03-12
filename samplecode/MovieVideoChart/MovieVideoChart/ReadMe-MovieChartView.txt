MovieVideoChart

This sample code demonstrates direct access to video samples in movie
files using the new, B-frame-aware APIs in QuickTime 7.  

Requires: Mac OS X 10.4. Xcode 2.1 and later projects build universal binary.

To use this sample, open a movie in the MovieVideoChart application.  
A window will appear containing a scrolling chart showing the internal 
structure of the movie's video track.  The horizontal scroll bar lets 
you browse through time; a slider in the lower left corner lets you 
adjust the magnification.

At the bottom of the window, the frames in the media are displayed in 
decode order; above that is their permutation into display order, if any;
above that is any rearrangement due to track edits.

To gather information about many samples at once (in this case, to 
draw the decode-to-display reordering), the application uses 
CopyMediaMutableSampleTable, which returns a QTSampleTable, which is
a CF-style object that holds information about samples.  

To gather information about a single sample, and to read it into memory
(to draw the thumbnails), the application uses GetMediaSample2.
To render the frames as thumbnails, the application uses 
ICMDecompressionSessions.  Care must be taken to ensure that frames 
are decompressed in correct decode order, walking forward from the 
previous key frame when appropriate.

In many cases, it is not necessary to reach down to these low-level APIs.
You can use QTOpenGLTextureContexts and QTPixelBufferVisualContexts to
render frames from movies without needing to deal with the complexities 
of B frames and inter-frame dependencies.
