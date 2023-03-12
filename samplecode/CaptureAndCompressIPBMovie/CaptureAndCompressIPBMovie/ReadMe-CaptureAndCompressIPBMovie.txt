CaptureAndCompressIPBMovie

This sample code shows how to capture video, recompress it on the fly
with H.264 and store the output in a movie file, using QuickTime 7's
Compression Session APIs. This sample also shows how to preview and
record audio using SGAudioMediaType SGChannel, with or without applying
AAC compression.

Required: Mac OS X 10.4, or Mac OS X 10.3.9 and QuickTime 7, and a 
camera with a vdig (eg, an iSight or other IIDC camera, or a DV camera).

Launch the application and it will prompt you for a file to save the 
captured and compressed movie in.  A preview window will appear.  
Close the window to finish the movie and open it QuickTime Player.
Note that this example does not capture audio.

Points of interest:
  * Capture using the sequence grabber
  * Decompression of captured frames to RGB using a decompression 
    session, which emits CVPixelBuffers
  * Wrapping CVPixelBuffers in CGBitmapContexts and drawing on them
  * Wrapping CVPixelBuffers in CGImageRefs, which are placed in an
    HIImageView for preview display
  * Recompression to H.264 using a compression session
  * Output movie file creation using CreateMovieStorage, which is 
    long-file-name aware
  * Storage of individual frames to the output movie using B-frame 
    aware Movie Toolbox APIs

It is important to adopt the new Compression Session APIs in order to
get high-quality compression with H.264.  Older APIs such as
SCCompressSequenceFrame don't allow the codec to reorder frames or to
see a lookahead window.


The principal differences between the compression sequence API 
(introduced in QuickTime 1.0) and the compression session API 
(introduced in QuickTime 7.0) are that with compression sessions:
  * you put the source frames into multiple CVPixelBuffers rather 
    than all into the same GWorld/PixMap;
  * you push the source frames into the compression session, and a 
    callback function you specify is called to return the encoded frames;
  * the callback function is not necessarily called immediately: 
    there may be latency so that the compressor can reorder frames, 
    take advantage of a lookahead window, etc.;
  * there's a call you can make if you need to request that the 
    compressor finish encoding certain frames or all frames you've 
    pushed (you call this at the end of the sequence, for example);
  * compression sessions track time, so you provide the display 
    timestamps and display durations when pushing source frames, 
    and these and the decode timestamps and durations are attached 
    to the emitted encoded frames;
  * the appropriate MediaSampleFlags are attached to the encoded frames 
    too, so it's simpler to store those correctly.
Documentation about individual compression session APIs can be found in
the header file ImageCompression.h.
