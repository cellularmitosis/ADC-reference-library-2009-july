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

import com.webobjects.foundation.NSKeyValueCoding;

/**
 * This class is a proxy to enable code generation for Objects in the model
 *
 * @param <T>
 */
public class GeneratorProxy<T> implements NSKeyValueCoding, NSKeyValueCoding.ErrorHandling {
	private String	_name;

	private T		_proxiedObject;

	/**
	 * Construct the proxy with the target proxied object
	 *
	 * @param name
	 *            name of the proxied object
	 * @param proxiedObject
	 *            target object
	 */
	public GeneratorProxy(String name, T proxiedObject) {
		_name = name;
		_proxiedObject = proxiedObject;
	}

	/**
	 * Returns the base proxied object
	 *
	 * @return proxied object
	 */
	public T proxiedObject() {
		return _proxiedObject;
	}

	/**
	 * Return the fetch specification name
	 *
	 * @return fetch specification name
	 */
	public String name() {
		return (_name != null ? _name : "");
	}

	/**
	 * Return the fetch specification name with the first character capitalized
	 *
	 * @return fetch specification name
	 */
	public String uppercaseUnderscoreName() {
		return StringUtils.camelCaseToUnderscore(name()).toUpperCase();
	}

	/**
	 * Return the fetch specification name in JAVA_CONSTANT_STYLE.
	 *
	 * @return fetch specification name
	 */
	public String initialCapitalName() {
		return StringUtils.toUppercaseFirstLetter(name());
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.foundation.NSKeyValueCoding#takeValueForKey(java.lang.Object, java.lang.String)
	 */
	public void takeValueForKey(Object value, String key) {
		NSKeyValueCoding.DefaultImplementation.takeValueForKey(this, value, key);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.foundation.NSKeyValueCoding#valueForKey(java.lang.String)
	 */
	public Object valueForKey(String key) {
		return NSKeyValueCoding.DefaultImplementation.valueForKey(this, key);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.foundation.NSKeyValueCoding.ErrorHandling#handleQueryWithUnboundKey(java.lang.String)
	 */
	public Object handleQueryWithUnboundKey(String key) {
		if (_proxiedObject != null) {
			return NSKeyValueCoding.DefaultImplementation.valueForKey(_proxiedObject, key);
		} else {
			return NSKeyValueCoding.DefaultImplementation.handleQueryWithUnboundKey(this, key);
		}
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.foundation.NSKeyValueCoding.ErrorHandling#handleTakeValueForUnboundKey(java.lang.Object, java.lang.String)
	 */
	public void handleTakeValueForUnboundKey(Object value, String key) {
		if (_proxiedObject != null) {
			NSKeyValueCoding.DefaultImplementation.takeValueForKey(_proxiedObject, value, key);
		} else {
			NSKeyValueCoding.DefaultImplementation.handleTakeValueForUnboundKey(this, value, key);
		}
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.foundation.NSKeyValueCoding.ErrorHandling#unableToSetNullForKey(java.lang.String)
	 */
	public void unableToSetNullForKey(String key) {
	// We should never go there. Or more precisely I cannot see how we can go there so I don't know how to handle it.
	}

}
