This simple example shows how to override the C++ 'new' and 'delete' operators.  See sections [lib.support.dynamic] and [basic.stc.dynamic.allocation] in the ISO C++ standard, ISO/IEC 14882:2003 or later, for a precise description of the rules affecting this example.

There's only one source file, main.cc, which contains both the overridden operators and a simple main program to exercise them.

Typical output from the main program is:

new was called 2 times and delete was called 2 times

The sample also shows:
- How to set up Xcode when doing this;
- How to work around a bug which, especially in small examples, might cause this overriding to not work; and
- The variations on new and delete you don't need to override because the standard C++ library will forward them to your overridden versions.
