=============================================================================
QuickTime for Java SDK                              Updated: 9 April 2001

Read Me Notes to "WiredSprites" Demo and Sample Code

=============================================================================
This demo program shows how to create a QuickTime movie containing wired sprites.

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3 Streaming for URL DataRefs
	- QTJava.zip

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample:

(1) ShipX.pct files (where X is a number indicative of an index value)
(2) The star.pct picture file
(3) The Planet.PICT picture file
(4) The Ship.pct picture file

=============================================================================
Notes & Comments

The program creates a movie containing wired sprites. Launch the program and you will be prompted to specify a location for the newly created movie. Once the movie has been created, simply open it up in QuickTime Player (or any other QuickTime-saavy application) to see the results.

=============================================================================
General Comments

The program creates a sprite track with 5 sprites: a space ship, a planet and 3 stars. The space ship sprite contains overrides images, so when the movie is played, these override images will display in place of the existing images for the sprite, giving an animation effect. The planet sprite is wired to perform a rotate action for each idle event received. The 3 star sprites are wired to change their visible state when mouse-enter/exit events are received.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 2001 Apple Computer Inc. All rights reserved.
