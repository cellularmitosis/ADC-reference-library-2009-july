PRODUCT = JNISample
USER    = $(shell whoami)

# These variables are used for building separate to the source code,
# which makes building read-only examples possible
OBJROOT = /Builds/$(USER)/$(PRODUCT)/$(PRODUCT).obj
SYMROOT = /Builds/$(USER)/$(PRODUCT)/$(PRODUCT).sym
DSTROOT = /Builds/$(USER)/$(PRODUCT)/$(PRODUCT).dst
SRCROOT = .

JARFILE = $(SYMROOT)/JNISample.jar
JNILIB  = $(SYMROOT)/libExample.jnilib
DYLIB   = $(SYMROOT)/libExample.dylib

all: directories $(JARFILE) $(JNILIB) $(DYLIB)
	@echo Finished building.

install: all
	$(CP) $(JARFILE) /Library/Java/Extensions
	$(CP) $(JNILIB) /Library/Java/Extensions
	$(CP) $(DYLIB) /usr/local/lib

clean:
	/bin/rm -rf $(SYMROOT) $(OBJROOT)

directories: $(OBJROOT) $(SYMROOT)


C_FLAGS      = -g -I$(OBJROOT) -I/System/Library/Frameworks/JavaVM.framework/Headers
DYLIB_FLAGS  =
BUNDLE_FLAGS = -lExample -L$(SYMROOT)


$(OBJROOT) $(SYMROOT):
	mkdir -p $@

$(JARFILE): $(OBJROOT)/JNISample.class
	cd $(OBJROOT); jar cf $@ *.class

$(JNILIB): $(OBJROOT)/ExampleJNILib.o $(DYLIB)
	cc -bundle $(BUNDLE_FLAGS) -o $@ $<

$(OBJROOT)/ExampleJNILib.o : ExampleJNILib.c $(OBJROOT)/JNISample.h
	cc $(C_FLAGS) -c -o $@ $<

$(OBJROOT)/JNISample.h: $(OBJROOT)/JNISample.class
	javah -classpath $(OBJROOT) -d $(OBJROOT) JNISample

$(OBJROOT)/JNISample.class: JNISample.java
	javac -classpath $(OBJROOT) -d $(OBJROOT) $<

$(DYLIB): $(OBJROOT)/ExampleDylib.o
	cc -dynamiclib $(DYLIB_FLAGS) -o $@ $<

$(OBJROOT)/ExampleDylib.o: ExampleDylib.c ExampleDylib.h
	cc $(C_FLAGS) -c -o $@ $<

test: $(JARFILE)
	java -classpath $(JARFILE) -Djava.library.path=$(SYMROOT) JNISample
