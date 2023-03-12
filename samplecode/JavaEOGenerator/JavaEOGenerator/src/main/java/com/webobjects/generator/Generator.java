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
package com.webobjects.generator;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;

import com.webobjects.foundation._NSDelegate;

/**
 * @since 5.4
 */
public class Generator {

	/**
	 * Configuration object
	 */
	protected Configuration		_configuration;

	/**
	 * Generation context
	 */
	protected GeneratorContext	_context;

	/**
	 * Java file extension
	 */
	public static final String	JavaFileExtension	= ".java";

	/**
	 * This interface defines a delegate to the Generator class to allow easy subclassing of the Generator.
	 */
	public static interface ClassDelegate {

		/**
		 * Allows the delegate the opportunity to construct and return a different Generator object.
		 *
		 * @return Should return a Generator Factory Object.
		 * @see #classDelegate()
		 * @see #setClassDelegate(Object anObject)
		 */
		public Generator newGenerator();
	}

	/**
	 *
	 */
	protected Generator() {
		super();
		// WOMLTemplateExtensions.instance().setDynamicAttributePrefix("$");
		// WOMLTemplateExtensions.instance().setDynamicAttributeSuffix("");
	}

	private static final _NSDelegate	_generatorClassDelegate	= new _NSDelegate(ClassDelegate.class);

	/**
	 * Assigns <code>delegate</code> as Generator's class delegate. The class delegate is optional; it allows modification of the creation of theGenerator for a model.
	 *
	 * @param delegate
	 *            The object to set as Generator's class delegate.
	 * @see #classDelegate()
	 * @see Generator.ClassDelegate
	 */
	public static void setClassDelegate(Object delegate) {
		_generatorClassDelegate.setDelegate(delegate);
	}

	/**
	 * Returns Generator's class delegate.
	 *
	 * @return Generator's class delegate.
	 * @see #setClassDelegate(Object anObject)
	 * @see Generator.ClassDelegate
	 */
	public static Object classDelegate() {
		return _generatorClassDelegate.delegate();
	}

	/**
	 * Returns a new instance of Generator using the class delegate if it exists.
	 *
	 * @return new instance of Generator
	 */
	public static Generator newGenerator() {
		Generator generator = null;
		// give our delegate (IB or EOModeler, perhaps) a chance to pick the generator
		if (_generatorClassDelegate != null && _generatorClassDelegate.respondsTo("newGenerator")) {
			generator = (Generator) _generatorClassDelegate.perform("newGenerator");
		}
		return (generator != null ? generator : new Generator());
	}

	/**
	 * Factory method to create a new Configuration object.
	 *
	 * @param argv
	 *            array of string passed to the application.
	 * @return newly initialized Configuration Object
	 */
	protected Configuration createConfiguration(String argv[]) {
		return new Configuration(argv);
	}

	/**
	 * Return the generator context
	 *
	 * @return generator context
	 */
	public GeneratorContext context() {
		if (_context == null) {
			_context = this.createContext();
		}
		return _context;
	}

	/**
	 * Factory method to create a new context
	 *
	 * @return new context
	 */
	protected GeneratorContext createContext() {
		return new GeneratorContext(_configuration);
	}

	/**
	 * Configure the generator with a set of arguments.
	 *
	 * @param argv
	 */
	public void configure(String argv[]) {
		_configuration = this.createConfiguration(argv);
	}

	/**
	 * Logs a statement to the console if in verbose mode
	 *
	 * @param log
	 */
	protected void logStatement(String log) {
		this.logStatement(log, this.verbose());
	}

	/**
	 * Logs a statement to the console
	 *
	 * @param log
	 * @param verbose
	 */
	protected void logStatement(String log, boolean verbose) {
		if (verbose) {
			this.println(log);
		}
	}

	/**
	 * Prints a statement. The base implementation does nothing and needs to be overwritten to display anything.
	 *
	 * @param log
	 */
	protected void println(String log) {
	// Place holder should be verwritten by subclasses
	}

	/**
	 * Excecute the generation based on the arguments.
	 */
	public void run() {
		if (this.help()) {
			this.showHelp();
		}
		if (this.version()) {
			this.showVersion();
		}
		this.logStatement(this.toString());

		this.logStatement("Start generation");
		this.generate();
		this.logStatement("End generation");
	}

	/**
	 * Generate the java file for the requested options. The defualt implementation does nothing.
	 */
	public void generate() {
	// Stub
	}

	/**
	 * Generates the required files for the target
	 *
	 * @param targetObject
	 */
	public void generateForTargetObject(Object targetObject) {
		// First the super class
		URL classUrl = this.urlForClass(targetObject);
		this.logStatement("URL for super class: '" + classUrl + "'");
		if (classUrl != null) {
			File generatedClassFile = null;
			try {
				generatedClassFile = new File(classUrl.toURI());
			} catch (URISyntaxException exception) {
				this.logStatement("Undefined super class file: '" + classUrl + "'", true);
			}
			if (this.shouldGenerateClassFile(generatedClassFile)) {
				this.logStatement("Generating super class file: '" + generatedClassFile + "'");
				String generatedFile = this.generateClassFileForTargetObject(targetObject);
				if (generatedFile.length() > 0) {
					if (!this.isIdenticalToExistingFile(generatedFile, generatedClassFile)) {
						this.logStatement("Overwritting super class file: '" + generatedClassFile + "'");
						this.ensurePathExistance(generatedClassFile);
						try {
							BufferedWriter out = new BufferedWriter(new FileWriter(generatedClassFile));
							out.write(generatedFile);
							out.flush();
							out.close();
						} catch (IOException exception) {
							this.logStatement("Cannot write super class file: '" + generatedClassFile + "'", true);
						}
					} else {
						this.logStatement("Skipping super class file: '" + generatedClassFile + "'");
					}
				}
			}
		} else {
			this.logStatement("Super class url is null: '" + targetObject + "'", true);
		}

		// Then the sub class
		URL subClassUrl = this.urlForSubClass(targetObject);
		this.logStatement("URL for sub class: '" + subClassUrl + "'");
		if (subClassUrl != null) {
			File generatedSubClassFile = null;
			try {
				generatedSubClassFile = new File(subClassUrl.toURI());
			} catch (URISyntaxException exception) {
				this.logStatement("Undefined sub class file: '" + subClassUrl + "'", true);
			}
			if (this.shouldGenerateSubClassFile(generatedSubClassFile)) {
				this.logStatement("Generating sub class file: '" + generatedSubClassFile + "'");
				this.ensurePathExistance(generatedSubClassFile);
				String generatedFile = this.generateSubClassFileForTargetObject(targetObject);
				if (generatedFile.length() > 0) {
					try {
						BufferedWriter out = new BufferedWriter(new FileWriter(generatedSubClassFile));
						out.write(generatedFile);
						out.flush();
						out.close();
					} catch (IOException exception) {
						this.logStatement("Cannot write sub class file: '" + generatedSubClassFile + "'", true);
					}
				}
			}
		} else {
			this.logStatement("Sub class url is null: '" + targetObject + "'", true);
		}
	}

	/**
	 * Checks the that file path exists. If it does not creates it.
	 *
	 * @param aFile
	 */
	protected void ensurePathExistance(File aFile) {
		if ((aFile != null) && !aFile.exists()) {
			File path = aFile.getParentFile();
			if (path != null) {
				path.mkdirs();
			}
		}
	}

	/**
	 * Checks if it needs to generate the class file
	 *
	 * @param aFile
	 * @return true if the class file need to be generated
	 */
	protected boolean shouldGenerateClassFile(File aFile) {
		return true;
	}

	/**
	 * Checks if it needs to generate the sub class file
	 *
	 * @param aFile
	 * @return true if the sub class file need to be generated
	 */
	protected boolean shouldGenerateSubClassFile(File aFile) {
		return (aFile != null) && !aFile.exists();
	}

	/**
	 * Checks if the generated file is identical to the existing file on disk. This checks the actual content of teh files.
	 *
	 * @param generatedFile
	 * @param aFile
	 * @return true if the files are identical.
	 */
	protected boolean isIdenticalToExistingFile(String generatedFile, File aFile) {
		if ((aFile == null) || (!aFile.exists())) {
			return false;
		} else {
			StringBuilder existingContent = new StringBuilder();
			try {
				BufferedReader in = new BufferedReader(new FileReader(aFile));
				try {
					while (in.ready()) {
						existingContent.append(in.readLine() + "\n");
					}
				} catch (IOException exception) {
					this.logStatement("Error reading existing file : '" + aFile + "', " + exception.toString(), true);
				}
			} catch (FileNotFoundException exception) { /* This has already been taken care of */}
			return existingContent.toString().trim().equals(generatedFile.trim());
		}
	}

	/**
	 * Generate the class for the target object
	 *
	 * @param targetObject
	 * @return generated class file
	 */
	public String generateClassFileForTargetObject(Object targetObject) {
		String generatedFile = this.generateFileForTargetObject(targetObject, this.templateForClass(targetObject), GeneratorComponent.GenerateSuperclass);
		this.logStatement("Generated file for  class: \n'" + generatedFile + "'");
		return generatedFile;
	}

	/**
	 * Generate the sub class for the target object
	 *
	 * @param targetObject
	 * @return generated class file
	 */
	public String generateSubClassFileForTargetObject(Object targetObject) {
		String generatedFile = this.generateFileForTargetObject(targetObject, this.templateForSubClass(targetObject), GeneratorComponent.GenerateSubclass);
		this.logStatement("Generated file for sub class: \n'" + generatedFile + "'");
		return generatedFile;
	}

	private <T> String generateFileForTargetObject(T targetObject, String fileTemplate, Boolean superclass) {
		GeneratorComponent<T> template = this.componentForTargetObject(targetObject);
		template.setTargetObject(targetObject);
		template.setSuperclassGeneration(superclass);
		template.setTemplateString(fileTemplate);
		template.setSuperClassPrefix(_configuration != null ? _configuration.prefix() : "_");
		return template.generateResponse().contentString().replace("&lt;", "<").replace("&gt;", ">").replace("&amp;", "&").replace("&quot;", "\"").replace("&apos;", "'").trim();
	}

	/**
	 * Returns the template for the class
	 *
	 * @param targetObject
	 * @return template
	 */
	protected String templateForClass(Object targetObject) {
		URL templateFileUrl = (_configuration != null ? _configuration.javaTemplate() : null);
		return (templateFileUrl != null ? this.templateForObject(targetObject, templateFileUrl) : "");
	}

	/**
	 * Returns the template for the sub class
	 *
	 * @param targetObject
	 * @return template
	 */
	protected String templateForSubClass(Object targetObject) {
		URL templateFileUrl = (_configuration != null ? _configuration.subClassJavaTemplate() : null);
		return (templateFileUrl != null ? this.templateForObject(targetObject, templateFileUrl) : "");
	}

	private String templateForObject(Object targetObject, URL templateFileUrl) {
		StringBuilder template = new StringBuilder();
		File templateFile = null;
		try {
			templateFile = new File(templateFileUrl.toURI());
		} catch (URISyntaxException exception) {
			this.logStatement("Undefined template: '" + templateFileUrl + "'", true);
		}
		if (templateFile != null) {
			try {
				BufferedReader in = new BufferedReader(new FileReader(templateFile));
				try {
					while (in.ready()) {
						template.append(in.readLine() + "\n");
					}
				} catch (IOException exception) {
					this.logStatement("Error reading template: '" + templateFileUrl + "', " + exception.toString(), true);
				}
			} catch (FileNotFoundException exception) {
				this.logStatement("Non existant template: '" + templateFileUrl + "'", true);
			}
		}
		return template.toString().trim();
	}

	/**
	 * Factory method for the generator component class
	 *
	 * @param <T>
	 *            Type of the object for the generation
	 * @param targetObject
	 *            target object for generation
	 * @return generator component
	 */
	protected <T> GeneratorComponent<T> componentForTargetObject(T targetObject) {
		return new GeneratorComponent<T>(this.context());
	}

	/**
	 * Generate the filename from the filename template
	 *
	 * @param targetObject
	 * @param fileExtension
	 * @return filename
	 */
	protected String fileNameForClass(Object targetObject, String fileExtension) {
		return this.generateFilenameForClass(targetObject, this.defaultFileNameForObject(targetObject), fileExtension);
	}

	/**
	 * Generate the filename from the filename template for the sub class
	 *
	 * @param targetObject
	 * @param fileExtension
	 * @return filename
	 */
	protected String fileNameForSubClass(Object targetObject, String fileExtension) {
		return this.generateFilenameForSubClass(targetObject, this.defaultFileNameForObject(targetObject), fileExtension);
	}

	/**
	 * Returns teh defualt filename for the class
	 *
	 * @param targetObject
	 * @return filename
	 */
	protected String defaultFileNameForObject(Object targetObject) {
		return "";
	}

	/**
	 * @param targetObject
	 * @return filename
	 */
	protected String defaultFileExtensionForObject(Object targetObject) {
		return JavaFileExtension;
	}

	/**
	 * @param <T>
	 *            Type of the object for the generation
	 * @param targetObject
	 *            target object for generation
	 * @param defaultName
	 * @param fileExtension
	 * @return filename
	 */
	protected <T> String generateFilenameForClass(T targetObject, String defaultName, String fileExtension) {
		return this.generateFilenameForObject(targetObject, defaultName, fileExtension, GeneratorComponent.GenerateSuperclass);
	}

	/**
	 * @param <T>
	 *            Type of the object for the generation
	 * @param targetObject
	 *            target object for generation
	 * @param defaultName
	 * @param fileExtension
	 * @return filename
	 */
	protected <T> String generateFilenameForSubClass(T targetObject, String defaultName, String fileExtension) {
		return this.generateFilenameForObject(targetObject, defaultName, fileExtension, GeneratorComponent.GenerateSubclass);
	}

	private <T> String generateFilenameForObject(T targetObject, String defaultName, String fileExtension, Boolean superclass) {
		String filename = "";
		String filenameTemplate = (_configuration != null ? _configuration.filenameTemplate() : "");
		if (filenameTemplate.length() > 0) {
			GeneratorComponent<T> template = this.componentForTargetObject(targetObject);
			template.setTargetObject(targetObject);
			template.setSuperclassGeneration(superclass);
			template.setTemplateString(filenameTemplate);
			template.setSuperClassPrefix(_configuration != null ? _configuration.prefix() : "_");
			filename = template.generateResponse().contentString().trim();
		} else {
			if (GeneratorComponent.GenerateSuperclass.equals(superclass)) {
				String className = this.classNameWithoutPackage(defaultName, fileExtension);
				String packageName = this.packageName(defaultName, fileExtension);
				String prefix = (_configuration != null ? _configuration.prefix() : "_");
				filename = (packageName.length() > 0 ? packageName + "." : "") + prefix + className;
			} else {
				filename = defaultName;
			}
		}
		filename = (filename.endsWith(fileExtension) ? filename : filename + fileExtension);
		return filename;
	}

	/**
	 * Returns the class name
	 *
	 * @param name
	 *            fully qualified class name
	 * @param fileExtension
	 * @return class name
	 */
	protected String classNameWithoutPackage(String name, String fileExtension) {
		String nameWithout = name;
		if (name.endsWith(fileExtension)) {
			nameWithout = name.substring(0, name.lastIndexOf(fileExtension));
		}
		int separator = nameWithout.lastIndexOf('.');
		return (separator > 0 ? nameWithout.substring(separator + 1) : nameWithout) + fileExtension;
	}

	/**
	 * Return the package name of the class name passed as parameter
	 *
	 * @param name
	 *            fully qualified class name
	 * @param fileExtension
	 * @return package name
	 */
	public String packageName(String name, String fileExtension) {
		String nameWithout = name;
		if (name.endsWith(fileExtension)) {
			nameWithout = name.substring(0, name.lastIndexOf(fileExtension));
		}
		int separator = nameWithout.lastIndexOf('.');
		return (separator > 0 ? nameWithout.substring(0, separator) : "");
	}

	/**
	 * Return the URL for the class file
	 *
	 * @param targetObject
	 * @return URL of the class file
	 */
	protected URL urlForClass(Object targetObject) {
		return this.buildURL(this.destination(), this.fileNameForClass(targetObject, this.defaultFileExtensionForObject(targetObject)), this.defaultFileExtensionForObject(targetObject));
	}

	/**
	 * Return the URL for the sub class file
	 *
	 * @param targetObject
	 * @return URL of the sub class file
	 */
	protected URL urlForSubClass(Object targetObject) {
		return this.buildURL(this.subClassDestination(), this.fileNameForSubClass(targetObject, this.defaultFileExtensionForObject(targetObject)), this.defaultFileExtensionForObject(targetObject));
	}

	private URL buildURL(URL destination, String filename, String fileExtension) {
		URL result = null;
		File targetDirectory = null;
		try {
			targetDirectory = new File(destination.toURI());
		} catch (URISyntaxException exception) {
			this.logStatement("Undefined path for directory: '" + destination + "'", true);
		}
		String filenameWithoutPackage = this.classNameWithoutPackage(filename, fileExtension);
		String packageName = this.packageName(filename, fileExtension).replace('.', '/');
		boolean packageDirectories = (_configuration != null ? _configuration.packageDirectories().booleanValue() : true);
		String qualifiedPath = (packageDirectories && (packageName.length() > 0) ? packageName + "/" : "") + filenameWithoutPackage;
		if (qualifiedPath.length() > 0) {
			File targetClass = new File(targetDirectory, qualifiedPath);
			try {
				result = targetClass.toURL();
			} catch (MalformedURLException exception) {
				this.logStatement("Undefined path for object: '" + qualifiedPath + "'", true);
			}
		}
		return result;
	}

	/**
	 * Returns the destination directory. By default the current directory.
	 *
	 * @return class directory url
	 */
	protected URL destination() {
		URL directory = (_configuration != null ? _configuration.destination() : null);
		if (directory == null) {
			try {
				directory = (new File(System.getProperty("user.dir"))).toURL();
			} catch (MalformedURLException exception) {
				this.logStatement("Undefined user directory: '" + System.getProperty("user.dir") + "'", true);
				System.exit(1);
			}
		}
		return directory;
	}

	/**
	 * Returns the destination sub class directory. By default the same directory as the target directory.
	 *
	 * @return sub class directory url
	 */
	protected URL subClassDestination() {
		URL directory = (_configuration != null ? _configuration.subClassDestination() : null);
		if (directory == null)
			directory = this.destination();
		return directory;
	}

	/**
	 * Returns true if the verbose argument was set.
	 *
	 * @return true if in verbose mode
	 */
	public boolean verbose() {
		return (_configuration != null ? _configuration.verbose().booleanValue() : false);
	}

	/**
	 * Returns true if the help argument was set.
	 *
	 * @return true is help is displayed
	 */
	public boolean help() {
		return (_configuration != null ? _configuration.help().booleanValue() : false);
	}

	/**
	 * Returns true if the version argument was set.
	 *
	 * @return true if the version is to be displayed
	 */
	public boolean version() {
		return (_configuration != null ? _configuration.version().booleanValue() : false);
	}

	/**
	 * Displays the help string
	 */
	public void showHelp() {
		// We force the help display even if verbose is not set
		this.logStatement(this.buildHelp(), true);
	}

	/**
	 * Helper function to build the help string
	 *
	 * @return help string
	 */
	protected String buildHelp() {
		StringBuilder aLog = new StringBuilder();
		this.buildHelpHeader(aLog);
		this.buildHelpArguments(aLog);
		this.buildHelpFooter(aLog);
		return aLog.toString();
	}

	/**
	 * Prints the help header in the logs
	 *
	 * @param aLog
	 */
	protected void buildHelpHeader(StringBuilder aLog) {
		aLog.append("Options:\n");
	}

	/**
	 * Prints the arguments in the logs
	 *
	 * @param aLog
	 */
	protected void buildHelpArguments(StringBuilder aLog) {
		if (_configuration != null) {
			_configuration.buildHelp(aLog);
		}
	}

	/**
	 * Prints the help footer in the logs
	 *
	 * @param aLog
	 */
	protected void buildHelpFooter(StringBuilder aLog) {
		aLog.append("");
	}

	/**
	 * Prints the generator version in the logs
	 */
	public void showVersion() {
		// We force the version display even if verbose is not set
		this.logStatement("unknown", true);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		StringBuilder aLog = new StringBuilder();
		aLog.append("<" + this.getClass().getName() + " ");
		aLog.append("configuration: \n");
		aLog.append(_configuration);
		aLog.append(" >");
		return aLog.toString();
	}

}
