


all:
	/usr/bin/pbxbuild  \
		DSTROOT=$$DSTROOT OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT)

install clean installhdrs:
	/usr/bin/pbxbuild $@  \
		DSTROOT=$$DSTROOT OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT)

debug:
	/usr/bin/pbxbuild  \
        -buildstyle Development	\
		DSTROOT=$$DSTROOT OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT)

debuginstall:
	/usr/bin/pbxbuild install  \
        -buildstyle Development	\
		DSTROOT=$$DSTROOT OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT)

installsrc:
	ditto . $(SRCROOT)
