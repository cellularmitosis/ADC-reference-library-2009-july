
Description:
file readme.txt

readme file for the TextNameTool sample.

a command line tool for generating rasterized images of text
using nice looking fonts drawn with ATSUI.


Copyright: 	(c) Copyright 2003 Apple Computer, Inc. All rights reserved.

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by
Apple Computer, Inc.  ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install,
modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the
Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its
entirety and without modifications, you must retain this notice and
the following text and disclaimers in all such redistributions of
the Apple Software.  Neither the name, trademarks, service marks or
logos of Apple Computer, Inc. may be used to endorse or promote
products derived from the Apple Software without specific prior
written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple
herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple
Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE,
EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



output:

Status information and data is printed to standard output as XML
data.  All valid responses, including error information, is 
enclosed in <textnametool> tags.  Below are examples of the 
various formats returned (my comments added in with '--' characters).



error reporting:

<textnametool>
<status>error</status> -- status
<message>error message text</message> -- error message
</textnametool>



Successful generation of an image:
 
<textnametool>
<status>ok</status> -- status 
<generated-image>
	<file>image.jpg</file>   -- image file name
	<font>Special Roman</font>  -- name of font used
	<size>36</size>   -- size of the font used
	<width>545</width>  -- width of final image file
	<height>171</height> -- height of final image file
</generated-image>
</textnametool>



Listing available fonts:

<textnametool>
<status>ok</status> -- status
<fonts>   -- the list of fonts
	<font> -- each font
		<name>Al Bayan Bold</name> -- it's name
		<id>8231</id> -- it's id
	</font>
	<font><name>Al Bayan Plain</name><id>8230</id></font>
	<font><name>American Typewriter</name><id>8232</id></font>
	<font><name>American Typewriter Bold</name><id>8233</id></font>
	<font><name>American Typewriter Condensed</name><id>8234</id></font>
	<font><name>American Typewriter Condensed Bold</name><id>8235</id></font>
	.... many other fonts would normally appear here ...
	<font><name>Verdana Bold Italic</name><id>8364</id></font>
	<font><name>Verdana Italic</name><id>8363</id></font>
	<font><name>Webdings</name><id>8365</id></font>
	<font><name>Zapf Dingbats</name><id>8217</id></font>
	<font><name>Zapfino</name><id>8366</id></font>
</fonts>
</textnametool>




Inquiries about the size of an existing image file:

<textnametool>
<status>ok</status> -- status
<investigated-image>
	<file>image.jpg</file> -- file's name
	<width>545</width> -- file's width
	<height>171</height> -- file's height
</investigated-image>
</textnametool>




command options:

-h
display help


-L
list available fonts


-f name
use the font with the given name.  This name must appear in the 
list of fonts returned by the -L option.


-s size
font size (default=36)


-e
the message text is encoded as html text using html style
entities for describing special characters.  The message text will be
decoded before it is drawn.


-n
observe line breaks in text, use them for wrapping text 
(default=off).  leading, trailing and consecutive line 
breaks are ignored.


-w width
maximum width for wrapping purposes (default=550)


-x file
read input from the file.  Only up to 4096 utf8 chars will 
be read.  if filename is -, then input is read from 
standard input.


-p linespacing
set the spacing between lines of text (default=0).


-q quality
set the jpg encoding quality (default=max).  quality 
should be either min, low, normal, high, or max.


-f file
use the contents of this file as the background for the 
text.  border settings will be relative to the bounds 
of this image rather than the text.


-t
print the dimensions of the resulting image file, 
don't actually generate the file.  useful if you want 
to test to see how big the graphics file will be and 
do some adjustments before actually creating the file.


-b border
set the border surrounding the image.  the default 
border is zero with no adjustment.  border can be 
any one of the following:


-b trim
text is drawn with a large border, but then the image 
is cropped to exactly surround any non-white pixels.  
same as the 'trim' command in photoshop.


-b auto
text is drawn with a zero border, but if any part of the 
text image goes outside of that border, then the border 
will be expanded so the image isn't cropped.


-b nnn
add a white border of nnn pixels around the text 
image.  nnn is a decimal number.


-b hhh,vvv
add a white border of size hhh to the sides of the 
image and a white border of size vvv to the top and 
bottom of the image.  hhh and vvv are decimal numbers, 
no space around the comma please.


-b lll,ttt,rrr,bbb
add a white border of size lll at the left, size ttt 
and the top, size rrr at the right, and size bbb and 
the bottom.  lll, ttt, rrr, and bbb are decimal 
numbers.  no spaces around the commas please.


-a attributes
set fond drawing attributes.  attributes is a group 
of letters and may contain any of the following:
b for bold, i for italic, u for underline, c for 
condense, and e for extend.


-o filename
set the output file name.  it is up to you to add the 
appropriate file name extension for the type of file 
you are saving.


-k kind
set the output file kind.  kind can be 
either 'jpg' or 'png'.  default=jpg.


-d
use four bit quickdraw drawing instead of 8 bit 
quartz drawing.  the default is to use 8 bit 
quartz drawing. 


-m message
set the message text that is to be drawn in the 
resulting image.  message text must be 
valid utf-8 text.


-c color
set the color used for drawing the text.  you can 
use one of the following html color names: "aqua", 
"black", "blue", "fuchsia", "gray", "green", "lime", 
"maroon", "navy", "olive", "purple", "red", "silver", 
"teal", "yellow", or "white".  If those are not enough, 
then you can use the format rr,gg,bb where rr, gg, bb 
are the red, green, and blue components of the colors 
expressed as two digit hex numbers.


-i filename
find out the width and height of an image file.


end of file readme.txt

