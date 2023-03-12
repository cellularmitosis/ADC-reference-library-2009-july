Cocoa_With_Carbon_or_C++

Cocoa_With_Carbon_or_C++ is sample code that shows how to call Carbon routines and C++ code from within an Objective-C/Objective-C++ Cocoa program.
The sample displays a window within which, depending upon the radio button selected, clicking the "Say Hello to the World" button will call functions
that use Objective-C, Carbon, or C++ respectively.

Calling Carbon routines is relatively easy, involving adding Carbon.framework to your project and including Carbon.h in your source code.
Because Objective-C is a superset of ANSI-C, calling Carbon C functions is not a problem.  One thing to note is that although it can be useful at
times to call non-GUI Carbon routines from Cocoa apps, functionality is more limited when attempting to include Carbon GUI in a Cocoa app.
This sample limits things to calling a modal alert panel.

Calling C++ code from Objective-C code involves ending your file with .mm (instead of .m) so that the Objective-C++ compiler will be used.
This compiler can understand both C++ and Objective-C, although you still have to use Objective-C to call into the AppKit and Foundation classes.
In other words, the ObjC++ compiler lets you put C++ code directly in Objective-C methods, and vice versa, but it doesn't change which language you
must use when writing to the Cocoa APIs.

One thing that is not demonstrated in this sample is how to set a given compiler to be used for all source, regardless of filename extension.
For example, say you wanted all your .cpp files to be compiled with the Objective-C++ compiler.  The way you would do this would be to add
the -ObjC++ flag in Targets->Build Settings to the OTHER_CFLAGS field.  You can similarly set the Objective-C compiler to be used for all compilation
by adding the -ObjC flag.

Speaking of setting compiler flags, setting the -cpp-precomp flag in OTHER_CFLAGS has been done for this sample.  By default it's off for
C++ and ObjC++ code compilation, because of the gcc 2.95 precompiler's inability to precompile headers that include C++ (straight C and ObjC only is allowed).
However, we aren't trying to use any precompiled headers that include C++ here - we just want to use the already precompiled versions of Carbon.h and AppKit.h.
Adding the flag greatly improves compilation speed.


Version History:
Version 1.0: Initial version 01/2001
Version 1.1: Updated the sample to include use of the Objective-C++ compiler, on files that end in .mm
Version 1.2: fixed a compile error on Project Builder 2.0.1/gcc3
Version 1.3: fixed compiler error, updated to use Xcode v2.4.1, Universal Binary, Carbon upgrade from .rsrc to .nib


Please send all feedback about this sample to:
<http://developer.apple.com/contact/feedback.html>

Please submit any bug reports about this sample to:
<http://developer.apple.com/bugreporter>