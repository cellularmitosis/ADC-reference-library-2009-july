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
package com.webobjects.entitygenerator;

import java.net.URL;

import com.webobjects.generator.Configuration;

/**
 * @since 5.4
 */
public class EntityConfiguration extends Configuration {

	/**
	 * Option key
	 */
	public static final String	JavaOptionKey				= "java";

	/**
	 * Option key
	 */
	public static final String	ObjectifCOptionKey			= "objc";

	/**
	 * Option key
	 */
	public static final String	JavaClientOptionKey			= "javaclient";

	/**
	 * Option key
	 */
	public static final String	ModelOptionKey				= "model";

	/**
	 * Option key
	 */
	public static final String	ReferenceModelOptionKey		= "refmodel";

	/**
	 * Option key
	 */
	public static final String	ForceOptionKey				= "force";

	/**
	 * Option key
	 */
	public static final String	SourceTemplateKey			= "sourceTemplate";

	/**
	 * Option key
	 */
	public static final String	HeaderTemplateKey			= "headerTemplate";

	/**
	 * Option key
	 */
	public static final String	SubClassSourceTemplateKey	= "subclassSourceTemplate";

	/**
	 * Option key
	 */
	public static final String	SubClassHeaderTemplateKey	= "subclassHeaderTemplate";

	/**
	 * @param argv
	 */
	public EntityConfiguration(String[] argv) {
		super(argv);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Configuration#buildOptionList()
	 */
	@Override
	protected void buildOptionList() {
		super.buildOptionList();
		this.addOptionToList(new BooleanOption(JavaOptionKey,
				"Specifies that Java source code files are to be generated.  Java is the default for WebObjects 5.x projects; Objective-C is the default with WebObjects 4.5 and earlier.",
				Boolean.TRUE));
		this.addOptionToList(new BooleanOption(ObjectifCOptionKey, "Specifies that Objective-C source code files are to be generated.  This is the default for WebObjects 4.5 and earlier."));
		this
				.addOptionToList(new BooleanOption(JavaClientOptionKey,
						"Specifies that Java client source code files are to be generated, which use slightly different templates by default and could potentially have a different class name.  Implies -java."));
		this.addOptionToList(new URLOption(ModelOptionKey,
				"Loads the EOModel found at modelpath, and generates classes for all entities found inside (unless specific entities are given on the command line)."));
		this
				.addOptionToList(new URLOption(
						ReferenceModelOptionKey,
						"Loads the EOModel found at modelpath, but does not generate classes for its entities (unless specific entities are given on the command line, in which case this is the same as -model).  This is useful if you only want to generate entities for one model, but need to load other models to resolve all of the relationships in the main model."));
		this
				.addOptionToList(new BooleanOption(
						ForceOptionKey,
						"Force overwriting of read-only files.  By default, read-only files will not be overwritten, and a warning message printed instead.  This can be a useful reminder in some revision control environments to check out the necessary files first."));
		this
				.addOptionToList(new StringOption(
						SourceTemplateKey,
						"Used to specify alternate template files to use.  File can be an absolute path, a relative path, or a filename found in one of the search path directories.  If not specified, the default templates are used.",
						""));
		this
				.addOptionToList(new StringOption(
						HeaderTemplateKey,
						"Used to specify alternate template files to use.  File can be an absolute path, a relative path, or a filename found in one of the search path directories.  If not specified, the default templates are used.",
						""));
		this
				.addOptionToList(new StringOption(
						SubClassSourceTemplateKey,
						"Used to specify alternate template files to use.  File can be an absolute path, a relative path, or a filename found in one of the search path directories.  If not specified, the default templates are used.",
						""));
		this
				.addOptionToList(new StringOption(
						SubClassHeaderTemplateKey,
						"Used to specify alternate template files to use.  File can be an absolute path, a relative path, or a filename found in one of the search path directories.  If not specified, the default templates are used.",
						""));
	}

	/**
	 * Returns true if the java option was set.
	 *
	 * @return true if the java option was set
	 */
	public Boolean java() {
		Option anOption = this.optionForName(JavaOptionKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.TRUE);
	}

	/**
	 * Returns true if the objc option was set.
	 * <p>
	 * Note: this parameter is not used in this implementation
	 * </p>
	 *
	 * @return true if the objc option was set
	 */
	public Boolean objectifC() {
		Option anOption = this.optionForName(ObjectifCOptionKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.FALSE);
	}

	/**
	 * Returns true if the java client option was set.
	 * <p>
	 * Note: this parameter is not used in this implementation
	 * </p>
	 *
	 * @return true if the java client option was set
	 */
	public Boolean javaClient() {
		Option anOption = this.optionForName(JavaClientOptionKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.FALSE);
	}

	/**
	 * Return the url to the model
	 *
	 * @return model url
	 */
	public URL model() {
		Option anOption = this.optionForName(ModelOptionKey);
		return (anOption instanceof URLOption ? ((URLOption) anOption).value() : null);
	}

	/**
	 * Return the url to the prototypes model
	 *
	 * @return model url
	 */
	public URL referenceModel() {
		Option anOption = this.optionForName(ReferenceModelOptionKey);
		return (anOption instanceof URLOption ? ((URLOption) anOption).value() : null);
	}

	/**
	 * Returns true if the force option was set.
	 * <p>
	 * Note: this parameter is not used in this implementation
	 * </p>
	 *
	 * @return true if the force option was set
	 */
	public Boolean force() {
		Option anOption = this.optionForName(ForceOptionKey);
		return (anOption instanceof BooleanOption ? ((BooleanOption) anOption).value() : Boolean.FALSE);
	}

	/**
	 * Returns the source template (ObjC)
	 * <p>
	 * Note: this parameter is not used in this implementation
	 * </p>
	 *
	 * @return source template
	 */
	public URL sourceTemplate() {
		Option anOption = this.optionForName(SourceTemplateKey);
		String value = (anOption instanceof StringOption ? ((StringOption) anOption).value() : "");
		return this.templateURL(value);
	}

	/**
	 * Returns the header template (ObjC)
	 * <p>
	 * Note: this parameter is not used in this implementation
	 * </p>
	 *
	 * @return header template
	 */
	public URL headerTemplate() {
		Option anOption = this.optionForName(HeaderTemplateKey);
		String value = (anOption instanceof StringOption ? ((StringOption) anOption).value() : "");
		return this.templateURL(value);
	}

	/**
	 * Returns the sub class source template (ObjC)
	 * <p>
	 * Note: this parameter is not used in this implementation
	 * </p>
	 *
	 * @return sub class source template
	 */
	public URL subClassSourceTemplate() {
		Option anOption = this.optionForName(SubClassSourceTemplateKey);
		String value = (anOption instanceof StringOption ? ((StringOption) anOption).value() : "");
		return this.templateURL(value);
	}

	/**
	 * Returns the sub class header template (ObjC)
	 * <p>
	 * Note: this parameter is not used in this implementation
	 * </p>
	 *
	 * @return sub class header template
	 */
	public URL subClassHeaderTemplate() {
		Option anOption = this.optionForName(SubClassHeaderTemplateKey);
		String value = (anOption instanceof StringOption ? ((StringOption) anOption).value() : "");
		return this.templateURL(value);
	}

}
