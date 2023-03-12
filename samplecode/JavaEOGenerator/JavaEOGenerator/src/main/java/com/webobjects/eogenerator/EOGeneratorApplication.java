/*
 * Copyright 2005 - 2007  Apple, Inc. All rights reserved.
 *
 * IMPORTANT:  This Apple software is supplied to you by Apple, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 *
 * In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 *
 * The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 * IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
package com.webobjects.eogenerator;

import com.webobjects.appserver.WOApplication;
import com.webobjects.appserver.WOSession;
import com.webobjects.generator.Generator;

/**
 * <pre>
 * Invocation Options
 * There is only one required parameter to the program; the EOModel must be specified.  In most situations custom templates will be used, and if so these need to be specified as well.  The parameters can be can be used in full, or as substrings where unique (e.g. -destination can be specified as -d or -dest).  The order of options is not important.
 * Only one kind of of classes can be generated at once.  If more than one type is needed (e.g. regular and client-side classes) then eogenerator must be invoked multiple times.
 * -objc
 * Specifies that Objective-C source code files are to be generated.  This is the default for WebObjects 4.5 and earlier.
 * -java
 * Specifies that Java source code files are to be generated.  Java is the default for WebObjects 5.x projects; Objective-C is the default with WebObjects 4.5 and earlier.
 * -javaclient
 * Specifies that Java client source code files are to be generated, which use slightly different templates by default and could potentially have a different class name.  Implies -java.
 * -model modelpath
 * Loads the EOModel found at modelpath, and generates classes for all entities found inside (unless specific entities are given on the command line).
 * -refmodel modelpath
 * Loads the EOModel found at modelpath, but does not generate classes for its entities (unless specific entities are given on the command line, in which case this is the same as -model).  This is useful if you only want to generate entities for one model, but need to load other models to resolve all of the relationships in the main model.
 * -force
 * Force overwriting of read-only files.  By default, read-only files will not be overwritten, and a warning message printed instead.  This can be a useful reminder in some revision control environments to check out the necessary files first.
 * -destination destinationdirectory
 * Used to specify a destination directory for generated source code files.  By default, files are generated in the current directory.
 * -subclassDestination destinationdirectory
 * Used to specify a destination directory for the empty subclass source code files.  This allows the generated files and the actively edited source to be placed in different directories.  By default, the subclass files are generated/looked for in the -destination directory.
 * -packagedirs
 * When doing Java generation, creates directories corresponding to the Java package name components underneath the -destination directory.  This will generate the classes into a typical Java directory structure.  [Note that the old ProjectBuilder, still sometimes used on Windows, does not support classes laid out in this manner.  Xcode should not have this problem.]
 * -prefix string
 * Uses string as the prefix for the generated class names (to distinguish them from the real EO class names).  By default this is a single underscore ( &quot;_&quot; ).
 * -templatedir directory
 * Prepends directory to the search path used to find template files.  Multiple -templatedir flags can be specified; the directories will be searched in the order they appear on the command line.
 * -sourceTemplate file
 * -headerTemplate file
 * -subclassSourceTemplate file
 * -subclassHeaderTemplate file
 * -javaTemplate file
 * -subclassJavaTemplate file
 * Used to specify alternate template files to use.  file can be an absolute path, a relative path, or a filename found in one of the search path directories.  If not specified, the default templates are used.
 * -filenameTemplate string
 * This argument causes string to be used as a MiscMerge template, with the result being used as the base name for the generated files.  Normally, filenames based on the entity's class name are used, which is sufficient for most situations.  Occasionally it can be useful to have custom filename patterns (coupled with custom templates), and this parameter can allow for a lot of flexibility.  For example, if you want Java interfaces to be generated along with the classes, a template of &quot;{javaClassName}Interface&quot; will cause 'Interface' to be appended to each filename. The delimiters for this template are `{' and `}', and the base object is the EOEntity instance (just like the regular class templates).  Regular EOEntity methods can be used as keys, as can entries in the userInfo dictionary.  When generating Java classes, if the generated filename has what looks like a Java package, it will override the package normally specified in the EOModel.
 * The boolean variables &quot;isSubclass&quot; and &quot;isSuperclass&quot; are defined for use in if statements as necessary.  For example, to generate the abstract superclasses into a separate &quot;eogen&quot; subpackage, something like the following can be used (coupled with changes to the templates of course): -filenameTemplate '{classPackageName}{if isSuperclass}.eogen{endif}.{classNameWithoutPackage}'
 * -encoding enc
 * Specifies the character encoding to be used for generated files.  The name can be specified as a value returned from the NSString +localizedNameOfStringEncoding: method, or (on MacOS X) an IANA name (typically what Java uses).  Defaults to the local platform string encoding.
 * -templateEncoding enc
 * Specifies the character encoding to be used when reading template files. Defaults to the value specified for -encoding.
 * -define-key value
 * This option adds the specified key-value pair to the list of variables available to the template, in addition to the EOEntity methods.  A key specified in this manner will not come before an EOEntity method of the same name when the template searches for the value.
 * -verbose
 * Causes more verbose debugging output to be logged to standard output.
 * -version
 * Displays the version number for eogenerator.
 * -help
 * Displays the command line options available in eogenerator, with synopses of their purposes.
 * </pre>
 *
 * @since 5.4
 */
public class EOGeneratorApplication extends WOApplication implements Generator.ClassDelegate {

	/**
	 * @param argv
	 */
	public static void main(String[] argv) {
		WOApplication.main(argv, EOGeneratorApplication.class);
	}

	/**
	 *
	 */
	public EOGeneratorApplication() {
		super();
		System.out.println("Welcome to " + this.name() + " !");
		Generator.setClassDelegate(this);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOApplication#run()
	 */
	@Override
	public void run() {
		Generator aGenerator = Generator.newGenerator();
		// First decode he arguments
		aGenerator.configure(this.launchArguments());
		if (this.launchArguments().length <= 0) {
			aGenerator.showHelp();
		}
		aGenerator.run();
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOApplication#_sessionClass()
	 */
	@Override
	protected Class<? extends WOSession> _sessionClass() {
		return EOGeneratorSession.class;
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator.ClassDelegate#newGenerator()
	 */
	public Generator newGenerator() {
		return new EOGenerator();
	}

}
