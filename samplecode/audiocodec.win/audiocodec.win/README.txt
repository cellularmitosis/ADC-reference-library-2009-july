README - AudioCodec

This is a Sound Manager codec example. It will create a compressor and 
decompressor supporting the uLaw format. This compressor is a built-in 
codec already present in the Sound Manager. To install this example it 
would require the version to be greater than the one already installed 
by the Sound Manager. The example has a high version of 5.0 only for 
this reason.

Use the latest QuickTime interfaces, which includes the "ComponentIncludes" headers.

Keep your data in the component globals long word aligned for best performance.

The kSoundComponentManufacturer is set to '????' and should be set to 
your company's four char code. It's up to you to create this.

Your compression format needs a new four char code as well. The constant 
kCodecFormat is where you define this. You should report your format to 
Apple so that it can be included in the Sound Manager interfaces.  Note 
that all lower-case codes are reserved for Apple's use.  Your code must
have at least one non-lowercase character.

A set of tables has been used to improve the performance of these codecs.
These tables are created as resources, which get loaded by the component's
Open method. Then they're used as part of the globals. Using a resource 
in this manner avoids the difficulties of creating static globals which 
require dependacies on the target OS runtime.

Windows only

You will need to open the .mak file as a MSDEV project, and then modify
the include paths and library paths by hand (Build/Settings) to match
wherever it is that you have your headers and libs.  Don't forget to 
modify the include paths on the Rez command line in the Custom Build
settings for uLawCodec.r (both Release and Debug).  On that same command 
line, you will also need to put in the full path to Rez.exe on your system.

Mac only

Linker gives the warning, "entry-point __uLawDecompComponentDispatchRD is 
not a descriptor".  This is normal due to the nature of creating PowerPC 
components. The entry point of a component has to be a RoutineDescriptor, 
which is a structure, not code.

You can build a debug and a release version of the codec. To build both 
choose "Build All" as the current target.

Once the codec is built, you can drag it to the "Reinstaller" utility or 
restart your system with the codec in the extensions folder.

Source level debugging is a little bit of a challenge. Since the Sound 
Manager runs at interrupt level, you cannot source level debug the code. 
You can use MoviePlayer to "Export" a sound file which does not run at 
interrupt level. Choose the target such as "Compressor PPC Debug" and then
"Run" and set your break points. Drag the built codec file to Reinstaller 
to install the new component. Open an AIFF file (or any movie file with 
audio) in MoviePlayer and then choose Export. Select your codec as the 
compressor type.


Enjoy,
QuickTime Team
