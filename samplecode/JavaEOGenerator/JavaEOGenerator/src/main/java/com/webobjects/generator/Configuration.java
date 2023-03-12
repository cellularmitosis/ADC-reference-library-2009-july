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

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.TreeMap;

/**
 * @since 5.4
 */
public class Configuration {

	Map<String, Option>			_options;

	List<String>				_arguments;

	Map<String, String>			_additionalKeyValues;

	List<URL>					_searchPath;

	/**
	 * Option leader key
	 */
	public static final String	OptionLeader				= "-";

	/**
	 * Option key
	 */
	public static final String	HelpOptionKey				= "help";

	/**
	 * Option key
	 */
	public static final String	VerboseOptionKey			= "verbose";

	/**
	 * Option key
	 */
	public static final String	VersionOptionKey			= "version";

	/**
	 * Option key variable prefix
	 */
	public static final String	AdditionalKeyValueKey		= "define-";

	/**
	 * Option key
	 */
	public static final String	EncodingKey					= "encoding";

	/**
	 * Option key
	 */
	public static final String	TemplateEncodingKey			= "templateEncoding";

	/**
	 * Option key
	 */
	public static final String	FileNameTemplateKey			= "filenameTemplate";

	/**
	 * Option key
	 */
	public static final String	DestinationKey				= "destination";

	/**
	 * Option key
	 */
	public static final String	SubClassDestinationKey		= "subclassDestination";

	/**
	 * Option key
	 */
	public static final String	PackageDirectoriesKey		= "packagedirs";

	/**
	 * Option key
	 */
	public static final String	PrefixKey					= "prefix";

	/**
	 * Option key
	 */
	public static final String	TemplateDirectoryKey		= "templatedir";

	/**
	 * Option key
	 */
	public static final String	JavaTemplateKey				= "javaTemplate";

	/**
	 * Option key
	 */
	public static final String	SubClassJavaTemplatesKey	= "subclassJavaTemplate";

	/**
	 *
	 */
	public static class URLOption extends Option {
		URL	_value;

		URL	_defaultValue;

		/**
		 * @param name
		 * @param description
		 */
		public URLOption(String name, String description) {
			this(name, description, null);
		}

		/**
		 * @param name
		 * @param description
		 * @param defaultValue
		 */
		public URLOption(String name, String description, URL defaultValue) {
			super(name, description);
			_defaultValue = defaultValue;
		}

		/*
		 * (non-Javadoc)
		 *
		 * @see com.webobjects.generator.Configuration.Option#decodeOptions(java.util.Queue)
		 */
		@Override
		public void decodeOptions(Queue<String> optionList) throws InvalidOptionException {
			if (this.isSameOptionName(optionList)) {
				String path = Option.optionValueFromList(optionList);
				if (path.length() > 0) {
					// First try for a URL
					try {
						_value = new URL(path);
					} catch (Exception urlException) {
						// We do not want to report the exception
						try {
							File aFile = new File(path);
							if (!aFile.exists()) {
								if (!aFile.mkdirs()) {
									throw new InvalidOptionException(aFile + " does not exist and could not be created.");
								}
							}
							_value = aFile.toURL();
						} catch (Exception fileException) {
							throw new InvalidOptionException(fileException);
						}
					}
				} else {
					_value = this.defaultValue();
				}
			}
		}

		/**
		 * @return value
		 */
		public URL value() {
			return (_value != null ? _value : this.defaultValue());
		}

		/**
		 * Default value of the option
		 *
		 * @return null
		 */
		public URL defaultValue() {
			return _defaultValue;
		}

		/*
		 * (non-Javadoc)
		 *
		 * @see com.webobjects.generator.Configuration.Option#toString()
		 */
		@Override
		public String toString() {
			return super.toString() + " default: '" + this.defaultValue() + "' value: '" + this.value() + "'";
		}

	}

	/**
	 *
	 */
	public static class StringOption extends Option {
		String	_value;

		String	_defaultValue;

		/**
		 * @param name
		 * @param description
		 */
		public StringOption(String name, String description) {
			this(name, description, "");
		}

		/**
		 * @param name
		 * @param description
		 * @param defaultValue
		 */
		public StringOption(String name, String description, String defaultValue) {
			super(name, description);
			_defaultValue = defaultValue;
		}

		/*
		 * (non-Javadoc)
		 *
		 * @see com.webobjects.generator.Configuration.Option#decodeOptions(java.util.Queue)
		 */
		@Override
		public void decodeOptions(Queue<String> optionList) throws InvalidOptionException {
			if (this.isSameOptionName(optionList)) {
				_value = Option.optionValueFromList(optionList);
			}
		}

		/**
		 * @return value
		 */
		public String value() {
			return ((_value != null) && (_value.length() > 0) ? _value : this.defaultValue());
		}

		/**
		 * Default value of the option
		 *
		 * @return empty String
		 */
		public String defaultValue() {
			return (_defaultValue != null ? _defaultValue : "");
		}

		/*
		 * (non-Javadoc)
		 *
		 * @see com.webobjects.generator.Configuration.Option#toString()
		 */
		@Override
		public String toString() {
			return super.toString() + " default: '" + this.defaultValue() + "' value: '" + this.value() + "'";
		}

	}

	/**
	 *
	 */
	public static class BooleanOption extends Option {

		Boolean	_value;

		Boolean	_defaultValue;

		/**
		 * @param name
		 * @param description
		 */
		public BooleanOption(String name, String description) {
			this(name, description, Boolean.FALSE);
		}

		/**
		 * @param name
		 * @param description
		 * @param defaultValue
		 */
		public BooleanOption(String name, String description, Boolean defaultValue) {
			super(name, description);
			_defaultValue = defaultValue;
		}

		/*
		 * (non-Javadoc)
		 *
		 * @see com.webobjects.generator.Configuration.Option#decodeOptions(java.util.Queue)
		 */
		@Override
		public void decodeOptions(Queue<String> optionList) throws InvalidOptionException {
			String aValue = Option.optionNameFromList(optionList);
			_value = (aValue.length() > 0 ? new Boolean(this.name().equals(aValue.toLowerCase())) : this.defaultValue());
		}

		/**
		 * @return value
		 */
		public Boolean value() {
			return (_value != null ? _value : this.defaultValue());
		}

		/**
		 * Default value of the option
		 *
		 * @return Boolean.FALSE
		 */
		public Boolean defaultValue() {
			return (_defaultValue != null ? _defaultValue : Boolean.FALSE);
		}

		/*
		 * (non-Javadoc)
		 *
		 * @see com.webobjects.generator.Configuration.Option#toString()
		 */
		@Override
		public String toString() {
			return super.toString() + " default: '" + this.defaultValue() + "' value: '" + this.value() + "'";
		}

	}

	/**
	 *
	 */
	public static abstract class Option {

		String	_name;

		String	_description;

		/**
		 * @param name
		 * @param description
		 */
		public Option(String name, String description) {
			super();
			_name = (name != null ? name : "").toLowerCase();
			_description = (description != null ? description : "");
		}

		/**
		 * Returns the option name
		 *
		 * @return option name
		 */
		public String name() {
			return (_name != null ? _name : "");
		}

		/**
		 * Returns the option description
		 *
		 * @return option description
		 */
		public String description() {
			return (_description != null ? _description : "");
		}

		/**
		 * Decode the options from the option list and remove the one it needs
		 *
		 * @param optionList
		 * @throws InvalidOptionException
		 */
		public abstract void decodeOptions(Queue<String> optionList) throws InvalidOptionException;

		/**
		 * Checks if this option name is the same than the top of the list.
		 *
		 * @param optionList
		 * @return true if this option has the same name than the top of the list
		 */
		public boolean isSameOptionName(Queue<String> optionList) {
			return this.name().equals(Option.optionNameFromList(optionList));
		}

		/**
		 * Return the option name i.e. strips the "-" or an empty string. Removing the option from the list if there is an option there.
		 *
		 * @param optionList
		 * @return option name from the top of the list
		 */
		public static String optionNameFromList(Queue<String> optionList) {
			String result = "";
			if ((!optionList.isEmpty()) && Option.isOption(optionList.peek())) {
				result = Option.optionName(optionList.remove());
			}
			return result.toLowerCase();
		}

		/**
		 * Return the option name i.e. strips the "-" or an empty string.
		 *
		 * @param value
		 * @return name
		 */
		public static String optionName(String value) {
			return (Option.isOption(value) ? value.trim().substring(OptionLeader.length()) : "");
		}

		/**
		 * Check if the string is an option
		 *
		 * @param value
		 * @return true if this is a non null option
		 */
		public static boolean isOption(String value) {
			return (value != null) && (value.length() > 0) && (value.trim().startsWith(OptionLeader));
		}

		/**
		 * Return the option value or an empty string. Removing the option from the list if there is an option there.
		 *
		 * @param optionList
		 * @return value form the top of teh list
		 */
		public static String optionValueFromList(Queue<String> optionList) {
			String result = "";
			if ((!optionList.isEmpty()) && Option.isValue(optionList.peek())) {
				result = Option.optionValue(optionList.remove());
			}
			return result;
		}

		/**
		 * Return the option value or an empty string. A value does not start by the option prefix.
		 *
		 * @param value
		 * @return value
		 */
		public static String optionValue(String value) {
			return (Option.isValue(value) ? value.trim() : "");
		}

		/**
		 * Check if the string is a value option
		 *
		 * @param value
		 * @return true if this is a non null value
		 */
		public static boolean isValue(String value) {
			return (value != null) && (value.length() > 0) && (!value.trim().startsWith(OptionLeader));
		}

		/*
		 * (non-Javadoc)
		 *
		 * @see java.lang.Object#toString()
		 */
		@Override
		public String toString() {
			return this.getClass().getName() + " name: '" + this.name() + "'";
		}

	}

	/**
	 *
	 */
	public static class InvalidOptionException extends Exception {
		private static final long	serialVersionUID	= 6860481828883562525L;

		/**
		 * Sole constructor
		 */
		public InvalidOptionException() {
			super();
		}

		/**
		 * Sole constructor
		 *
		 * @param message
		 *            the detail message. The detail message is saved for later retrieval by the Throwable.getMessage() method.
		 */
		public InvalidOptionException(String message) {
			super(message);
		}

		/**
		 * Sole constructor
		 *
		 * @param rootCause
		 *            the cause (which is saved for later retrieval by the Throwable.getCause() method). (A null value is permitted, and indicates that the cause is nonexistent or unknown.)
		 */
		public InvalidOptionException(Exception rootCause) {
			super(rootCause);
		}

	}

	/**
	 * @param argv
	 */
	public Configuration(String[] argv) {
		super();
		this.buildOptionList();
		LinkedList<String> optionList = new LinkedList<String>();
		if (argv != null) {
			for (int i = 0; i < argv.length; i++) {
				optionList.add(argv[i]);
			}
		}
		this.decodeOptions(optionList);
	}

	/**
	 * @param optionList
	 */
	protected void decodeOptions(Queue<String> optionList) {
		while (!optionList.isEmpty()) {
			String option = optionList.peek();
			if (Option.isOption(option)) {
				// If it is an option then decode it.
				try {
					Option anOption = this.optionForName(Option.optionName(option));
					if (anOption != null) {
						anOption.decodeOptions(optionList);
					} else {
						// Checks if it is a define option
						// AdditionalKeyValueKey
						String define = Option.optionName(option);
						if (define.startsWith(AdditionalKeyValueKey)) {
							String key = define.substring(AdditionalKeyValueKey.length());
							optionList.remove();
							if (!optionList.isEmpty()) {
								String value = optionList.peek();
								if (Option.isValue(value)) {
									this.setValueForAdditionalKey(value, key);
									optionList.remove();
								}
							}
						} else {
							// Spurious option ignore it.
							optionList.remove();
						}
					}
				} catch (InvalidOptionException exception) {
					System.err.println("Invalid Option " + option);
				}
			} else {
				// This is a loose value probably an argument
				this.addArgumentToList(option);
				optionList.remove();
			}
		}
	}

	/**
	 * Build the option list. Subclasses need to override this methods to add new options. All subclasses must call super.
	 */
	protected void buildOptionList() {
		this.addOptionToList(new BooleanOption(HelpOptionKey, "Displays the command line options available in eogenerator, with synopses of their purposes.", Boolean.FALSE));
		this.addOptionToList(new BooleanOption(VerboseOptionKey, "Causes more verbose debugging output to be logged to standard output.", Boolean.FALSE));
		this.addOptionToList(new BooleanOption(VersionOptionKey, "Displays the version number for the generator.", Boolean.FALSE));
		this.addOptionToList(new StringOption(EncodingKey, "Displays the version number for the generator.", "UTF-8"));
		this.addOptionToList(new StringOption(TemplateEncodingKey, "Displays the version number for the generator.", "UTF-8"));
		this
				.addOptionToList(new StringOption(
						FileNameTemplateKey,
						"JavaEOGenerator differs from EOGenerator with this option.  It uses the WOComponent generation engine in order to define a template which should be familiar to a lot of developers.  A template will be defined using wo tags.    For example, to generate the abstract superclasses into a separate \"eogen\" subpackage, something like the following can be used (coupled with changes to the templates of course): -filenameTemplate \"<wo:WOString value=\"[entity.classPackageName]\"/><wo:WOConditional condition=\"[isSuperclass]\">.base.<wo:WOString value=\"[superClassPrefix]\"/></wo:WOConditional><wo:WOConditional condition=\"[isSubclass]\">.</wo:WOConditional><wo:WOString value=\"[entity.classNameWithoutPackage]\"/>\nThis argument causes string to be used as a MiscMerge template, with the result being used as the base name for the generated files.  Normally, filenames based on the entity's class name are used, which is sufficient for most situations.  Occasionally it can be useful to have custom filename patterns (coupled with custom templates), and this parameter can allow for a lot of flexibility.  For example, if you want Java interfaces to be generated along with the classes, a template of \"{javaClassName}Interface\" will cause 'Interface' to be appended to each filename. The delimiters for this template are '{' and '}', and the base object is the EOEntity instance (just like the regular class templates).  Regular EOEntity methods can be used as keys, as can entries in the userInfo dictionary.  When generating Java classes, if the generated filename has what looks like a Java package, it will override the package normally specified in the EOModel. The boolean variables \"isSubclass\" and \"isSuperclass\" are defined for use in if statements as necessary (Using WOConditionals). For example, \"<wo:WOString value=\"[entity.classPackageName]\"/><wo:WOConditional condition=\"[isSuperclass]\">.base.<wo:WOString value=\"[superClassPrefix]\"/></wo:WOConditional><wo:WOConditional condition=\"[isSubclass]\">.</wo:WOConditional><wo:WOString value=\"[entity.classNameWithoutPackage]\"/>",
						"<wo:WOString value=\"[entity.classPackageName]\"/><wo:WOConditional condition=\"[isSuperclass]\">.base.<wo:WOString value=\"[superClassPrefix]\"/></wo:WOConditional><wo:WOConditional condition=\"[isSubclass]\">.</wo:WOConditional><wo:WOString value=\"[entity.classNameWithoutPackage]\"/>"));
		this.addOptionToList(new URLOption(DestinationKey, "Used to specify a destination directory for generated source code files.  By default, files are generated in the current directory."));
		this
				.addOptionToList(new URLOption(
						SubClassDestinationKey,
						"Used to specify a destination directory for the empty subclass source code files.  This allows the generated files and the actively edited source to be placed in different directories.  By default, the subclass files are generated/looked for in the -destination directory."));

		this
		.addOptionToList(new URLOption(
			DestinationKey,
			"Used to specify a destination directory for generated source code files.  By default, files are generated in the current directory."));

		this
				.addOptionToList(new BooleanOption(
						PackageDirectoriesKey,
						"When doing Java generation, creates directories corresponding to the Java package name components underneath the -destination directory.  This will generate the classes into a typical Java directory structure.  [Note that the old ProjectBuilder, still sometimes used on Windows, does not support classes laid out in this manner.  Xcode should not have this problem.]",
						Boolean.TRUE));
		this.addOptionToList(new StringOption(PrefixKey,
				"Uses string as the prefix for the generated class names (to distinguish them from the real EO class names).  By default this is a single underscore ( \"_\" ).", "_"));
		this
				.addOptionToList(new URLOption(TemplateDirectoryKey,
						"Prepends directory to the search path used to find template files.  Multiple -templatedir flags can be specified; the directories will be searched in the order they appear on the command line."));
		this
				.addOptionToList(new StringOption(
						JavaTemplateKey,
						"Used to specify alternate template files to use.  File can be an absolute path, a relative path, or a filename found in one of the search path directories.  If not specified, the default templates are used.",
						"JavaSourceEOF.eotemplate"));
		this
				.addOptionToList(new StringOption(
						SubClassJavaTemplatesKey,
						"Used to specify alternate template files to use.  File can be an absolute path, a relative path, or a filename found in one of the search path directories.  If not specified, the default templates are used.",
						"JavaSubClassSourceEOF.eotemplate"));
	}

	/**
	 * Add a new option to the options list.
	 *
	 * @param option
	 */
	protected void addOptionToList(Option option) {
		if (_options == null) {
			_options = new TreeMap<String, Option>();
		}
		_options.put(option.name(), option);
	}

	/**
	 * Returns the Option object for the name
	 *
	 * @param name
	 * @return option object
	 */
	protected Option optionForName(String name) {
		return (_options != null ? _options.get(name.toLowerCase()) : null);
	}

	/**
	 * Return the list of arguments in the order they appeared on the command line. The returned list is immutable.
	 *
	 * @return argument list
	 */
	public List<String> arguments() {
		return Collections.unmodifiableList(this._arguments());
	}

	/**
	 * Checks if arguments have been passed to the generator
	 *
	 * @return true if there are arguments
	 */
	public boolean hasArguments() {
		return _arguments != null && _arguments.size() > 0;
	}

	private List<String> _arguments() {
		if (_arguments == null) {
			_arguments = new ArrayList<String>();
		}
		return _arguments;
	}

	/**
	 * @param argument
	 */
	protected void addArgumentToList(String argument) {
		this._arguments().add(argument);
	}

	/**
	 * Return a dictionary of additional key values. The returned map is immutable.
	 *
	 * @return map of additional value pairs
	 */
	public Map<String, String> additionalKeyValues() {
		return Collections.unmodifiableMap(this._additionalKeyValues());
	}

	/**
	 * Checks if additional key values have been passed to the generator
	 *
	 * @return true if the user defined additional value pairs
	 */
	public boolean hasAdditionalKeyValues() {
		return _additionalKeyValues != null && _additionalKeyValues.size() > 0;
	}

	private Map<String, String> _additionalKeyValues() {
		if (_additionalKeyValues == null) {
			_additionalKeyValues = new TreeMap<String, String>();
		}
		return _additionalKeyValues;
	}

	/**
	 * @param key
	 * @param value
	 */
	protected void setValueForAdditionalKey(String value, String key) {
		this._additionalKeyValues().put(key, value);
	}

	/**
	 * Return the value for the additional key
	 *
	 * @param key
	 * @return additional key value
	 */
	public String valueForAdditionalKey(String key) {
		return this._additionalKeyValues().get(key);
	}

	/**
	 * Returns true if the verbose option was set.
	 *
	 * @return true if verbose
	 */
	public Boolean verbose() {
		Option anOption = this.optionForName(VerboseOptionKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.FALSE);
	}

	/**
	 * Returns true if the help option was set.
	 *
	 * @return true if help was requested
	 */
	public Boolean help() {
		Option anOption = this.optionForName(HelpOptionKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.FALSE);
	}

	/**
	 * Returns true if the version option was set.
	 *
	 * @return true if the version was requested
	 */
	public Boolean version() {
		Option anOption = this.optionForName(VersionOptionKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.FALSE);
	}

	/**
	 * Return the value of the encoding option
	 *
	 * @return encoding option
	 */
	public String encoding() {
		Option anOption = this.optionForName(EncodingKey);
		return (anOption instanceof StringOption ? ((StringOption) anOption).value() : "UTF8");
	}

	/**
	 * Return the value of the template encoding option
	 *
	 * @return template encoding option
	 */
	public String templateEncoding() {
		Option anOption = this.optionForName(TemplateEncodingKey);
		return (anOption instanceof StringOption ? ((StringOption) anOption).value() : "UTF8");
	}

	/**
	 * Return the value of the filename template option
	 *
	 * @return filename template option
	 */
	public String filenameTemplate() {
		Option anOption = this.optionForName(FileNameTemplateKey);
		return (anOption instanceof StringOption ? ((StringOption) anOption).value() : "");
	}

	/**
	 * Returns the file class destination option
	 *
	 * @return file class destination
	 */
	public URL destination() {
		Option anOption = this.optionForName(DestinationKey);
		return (anOption instanceof URLOption ? ((URLOption) anOption).value() : null);
	}

	/**
	 * Returns the file sub class destination option
	 *
	 * @return file sub class destination
	 */
	public URL subClassDestination() {
		Option anOption = this.optionForName(SubClassDestinationKey);
		return (anOption instanceof URLOption ? ((URLOption) anOption).value() : null);
	}

	/**
	 * Returns true if the packagedirs option was set.
	 *
	 * @return package directory option
	 */
	public Boolean packageDirectories() {
		Option anOption = this.optionForName(PackageDirectoriesKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.TRUE);
	}

	/**
	 * Return the value of thePrefix option
	 *
	 * @return Prefix option
	 */
	public String prefix() {
		Option anOption = this.optionForName(PrefixKey);
		return (anOption instanceof StringOption ? ((StringOption) anOption).value() : "_");
	}

	/**
	 * Return the value of the template directoryoption
	 *
	 * @return template directory option
	 */
	public URL templateDirectory() {
		Option anOption = this.optionForName(TemplateDirectoryKey);
		return (anOption instanceof URLOption ? ((URLOption) anOption).value() : null);
	}

	/**
	 * Return the value of the Java Template option
	 *
	 * @return Java Template option
	 */
	public URL javaTemplate() {
		Option anOption = this.optionForName(JavaTemplateKey);
		String value = (anOption instanceof StringOption ? ((StringOption) anOption).value() : "");
		return this.templateURL(value);
	}

	/**
	 * Return the path to the sub class template file, using the templateDirectory if necessary.
	 *
	 * @return sub class template url
	 */
	public URL subClassJavaTemplate() {
		Option anOption = this.optionForName(SubClassJavaTemplatesKey);
		String value = (anOption instanceof StringOption ? ((StringOption) anOption).value() : "");
		return this.templateURL(value);
	}

	/**
	 * Return the list of URL to search.
	 *
	 * @return rearch path URL list
	 */
	public List<URL> searchPath() {
		if (_searchPath == null) {
			List<URL> aList = new ArrayList<URL>();
			if (this.templateDirectory() != null) {
				aList.add(this.templateDirectory());
			}
			_searchPath = aList;
		}
		return Collections.unmodifiableList(_searchPath);
	}

	/**
	 * Return the path to the template file, using the templateDirectory if necessary.
	 *
	 * @param path
	 * @return class template url
	 */
	protected URL templateURL(String path) {
		URL templateURL = null;
		if (path.length() > 0) {
			// First try for a URL
			try {
				templateURL = new URL(path);
			} catch (Exception urlException) { /* We do not want to report the exception */}
			if (templateURL == null) {
				// next try the absolute path
				try {
					File aFile = new File(path);
					if (aFile.exists()) {
						templateURL = aFile.toURL();
					}
				} catch (Exception urlException) { /* We do not want to report the exception */}
			}
			if (templateURL == null) {
				// and now for the relative path
				for (URL parentDir : this.searchPath()) {
					try {
						URL fileURL = new URL(parentDir, path);
						File aFile = new File(fileURL.toURI());
						if (aFile.exists()) {
							templateURL = aFile.toURL();
							break;
						}
					} catch (Exception urlException) { /* We do not want to report the exception */}
				}
			}
		}
		return templateURL;
	}

	/**
	 * Build the list of options for the help display
	 *
	 * @param aLog
	 */
	public void buildHelp(StringBuilder aLog) {
		if (_options != null) {
			for (Option option : _options.values()) {
				aLog.append("\t-" + option.name() + ": " + option.description() + "\n");
			}
		}
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		StringBuilder aLog = new StringBuilder();
		aLog.append(this.getClass().getName());
		if (_options != null) {
			aLog.append("\n");
			for (Option option : _options.values()) {
				aLog.append(option.toString());
				aLog.append("\n");
			}
		} else {
			aLog.append(" no options ");
		}
		if (_additionalKeyValues != null) {
			aLog.append("\n");
			for (String key : _additionalKeyValues.keySet()) {
				aLog.append(key + ": '" + _additionalKeyValues.get(key) + "'");
				aLog.append("\n");
			}
		} else {
			aLog.append(" no additional key values ");
		}
		if (_arguments != null) {
			aLog.append("\n");
			for (String argument : _arguments) {
				aLog.append(argument);
				aLog.append("\n");
			}
		} else {
			aLog.append(" no arguments ");
		}
		return aLog.toString();
	}

}
