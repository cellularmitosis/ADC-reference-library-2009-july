Read Me: BoingX

This is just a simple little demo that shows off 
using Quartz Extreme to provide borderless OpenGL 
content on the desktop. It was inspired by the 
Boing demo for the Amiga personal computer.  While 
the original demo used color cycling and the 
Amiga's dual playfield hardware for moving the 
ball around, this demo uses an actual 3D object 
with real lighting. Unlike the original, this 
one doesn't include sound.

When the app starts up the ball is just bouncing 
around inside the window.  Hitting the 's' key 
resizes the window to 2X normal size.   Hitting 'm' 
will enable multisampling if a multisample pixel 
format was available.  Hitting 'l' enables lighting 
for that added "ooh aah" effect.   The final magic 
demo key is to hit 't' which makes the background 
fade away and the ball then starts to bounce 
around on your desktop.

The code is fairly straightforward, and uses the 
NSOpenGLCPSurfaceOpacity parameter to mark the 
view as being transparent. Note that the background 
is always marked as being non-opaque, it's just that 
the background is drawn with opaque colors until the 
background fades out.   The other interesting tidbit 
is the use of 

glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA) 

for doing the ball shadow.   Using GL_SRC_ALPHA would 
unfortunately trash the background alpha and the ball 
shadow would show through to the desktop, which isn't 
what is desired in this case.

