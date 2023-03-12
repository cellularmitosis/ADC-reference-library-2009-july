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

import com.webobjects.appserver.WOApplication;
import com.webobjects.appserver.WOComponent;
import com.webobjects.appserver.WOElement;
import com.webobjects.appserver._private.WOComponentDefinition;
import com.webobjects.appserver.parser.WOComponentTemplateParser;

/**
 * @param <T>
 *            Target object type for the source code generation
 * @since 5.4
 */
public class GeneratorComponent<T> extends WOComponent {
	private static final long	serialVersionUID	= -9055470273241337262L;

	transient WOElement			_template;

	private String				_templateString;

	private T					_targetObject;

	private Boolean				_superclassGeneration;

	private String				_superClassPrefix;

	/**
	 * Convenience for super class generation
	 */
	public static final Boolean	GenerateSuperclass	= Boolean.TRUE;

	/**
	 * Convenience for class generation
	 */
	public static final Boolean	GenerateSubclass	= Boolean.FALSE;

	/**
	 * Sole constructor
	 *
	 * @param aContext
	 */
	public GeneratorComponent(GeneratorContext aContext) {
		super(aContext);
	}

	/**
	 * Returns the context for this component
	 *
	 * @return generator context
	 */
	public GeneratorContext generatorContext() {
		return (GeneratorContext) super.context();
	}

	/**
	 * @return the targetObject
	 */
	public T targetObject() {
		return _targetObject;
	}

	/**
	 * @param targetObject
	 *            the targetObject to set
	 */
	public void setTargetObject(T targetObject) {
		_targetObject = targetObject;
	}

	/**
	 * @return the isSuperclass
	 */
	public boolean isSuperclass() {
		return GenerateSuperclass.equals(_superclassGeneration);
	}

	/**
	 * @return the isSuperclass
	 */
	public boolean isSubclass() {
		return !this.isSuperclass();
	}

	/**
	 * @param value
	 *            the isSuperclass to set
	 */
	public void setSuperclassGeneration(Boolean value) {
		_superclassGeneration = value;
	}

	/**
	 * @return the templateString
	 */
	public String templateString() {
		return (_templateString != null ? _templateString : "<__Document__></__Document__>");
	}

	/**
	 * @param value
	 *            the templateString to set
	 */
	public void setTemplateString(String value) {
		_templateString = value;
	}

	/**
	 * @return the superClassPrefix
	 */
	public String superClassPrefix() {
		return (_superClassPrefix != null ? _superClassPrefix : "_");
	}

	/**
	 * @param superClassPrefix
	 *            the superClassPrefix to set
	 */
	public void setSuperClassPrefix(String superClassPrefix) {
		this._superClassPrefix = superClassPrefix;
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#handleQueryWithUnboundKey(java.lang.String)
	 */
	@Override
	public Object handleQueryWithUnboundKey(String key) {
		Object value = null;
		if (key != null) {
			value = this.generatorContext().configuration().valueForAdditionalKey(key);
		}
		if (value == null) {
			value = this.generatorContext().localVariableForKey(key);
		}
		return value;
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#handleTakeValueForUnboundKey(java.lang.Object, java.lang.String)
	 */
	@Override
	public void handleTakeValueForUnboundKey(Object value, String key) {
		this.generatorContext().setLocalVariableForKey(value, key);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#unableToSetNullForKey(java.lang.String)
	 */
	@Override
	public void unableToSetNullForKey(String key) {
		super.unableToSetNullForKey(key);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#validateValueForKey(java.lang.Object, java.lang.String)
	 */
	@Override
	public Object validateValueForKey(Object value, String key) throws ValidationException {
		return super.validateValueForKey(value, key);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#valueForKey(java.lang.String)
	 */
	@Override
	public Object valueForKey(String key) {
		return super.valueForKey(key);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#takeValueForKey(java.lang.Object, java.lang.String)
	 */
	@Override
	public void takeValueForKey(Object value, String key) {
		super.takeValueForKey(value, key);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#template()
	 */
	@Override
	public WOElement template() {
		if (_template == null) {
			if (this.templateString().length() > 0) {
				try {
					_template = WOComponentTemplateParser.templateWithHTMLAndDeclaration("Template", this.templateString(), "", null, WOApplication.application().associationFactory(), WOApplication
							.application().namespaceProvider());
				} catch (Exception exception) {/**/
					exception.printStackTrace();
				}
			}
		}
		return _template;
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.appserver.WOComponent#_componentDefinition()
	 */
	@Override
	public WOComponentDefinition _componentDefinition() {
		return null;
	}

}
