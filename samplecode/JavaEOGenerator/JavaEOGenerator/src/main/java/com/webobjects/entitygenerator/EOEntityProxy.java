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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

import com.webobjects.eoaccess.EOAttribute;
import com.webobjects.eoaccess.EOEntity;
import com.webobjects.eoaccess.EOProperty;
import com.webobjects.eoaccess.EORelationship;
import com.webobjects.generator.GeneratorProxy;

/**
 * This class is a proxy to enable code generation for entities in the model
 */
public class EOEntityProxy extends GeneratorProxy<EOEntity> {
	private List<EOPropertyProxy<?>>		_classProperties;

	private List<EOAttributeProxy>			_classAttributes;

	private List<EORelationshipProxy>		_classRelationships;

	private List<EORelationshipProxy>		_classToOneRelationships;

	private List<EORelationshipProxy>		_classToManyRelationships;

	private List<EOFetchSpecificationProxy>	_classFetchSpecifications;

	/**
	 * Constructor
	 *
	 * @param entity
	 */
	public EOEntityProxy(EOEntity entity) {
		super(entity.name(), entity);
	}

	/**
	 * Return the entity class properties
	 *
	 * @return entity class properties
	 */
	public List<EOPropertyProxy<?>> classProperties() {
		if (_classProperties == null) {
			EOEntity entity = proxiedObject();
			Map<String, EOPropertyProxy<?>> list = new TreeMap<String, EOPropertyProxy<?>>();
			for (EOProperty property : entity.classProperties()) {
				if ((entity.parentEntity() == null) || (!entity.parentEntity().classProperties().contains(property))) {
					if (property instanceof EOAttribute) {
						list.put(property.name(), new EOAttributeProxy((EOAttribute) property));
					} else if (property instanceof EORelationship) {
						list.put(property.name(), new EORelationshipProxy((EORelationship) property));
					} else {
						// NOW WHAT?
					}
				}
			}
			_classProperties = new ArrayList<EOPropertyProxy<?>>(list.values());
		}
		return _classProperties;
	}

	/**
	 * Returns the entity class attributes
	 *
	 * @return entity class attributes
	 */
	public List<EOAttributeProxy> classAttributes() {
		if (_classAttributes == null) {
			Set<EOAttributeProxy> list = new HashSet<EOAttributeProxy>();
			for (EOPropertyProxy<?> property : this.classProperties()) {
				if (property instanceof EOAttributeProxy) {
					list.add((EOAttributeProxy) property);
				}
			}
			_classAttributes = new ArrayList<EOAttributeProxy>(list);
		}
		return _classAttributes;
	}

	/**
	 * Returns the entity class relationships
	 *
	 * @return entity class relationships
	 */
	public List<EORelationshipProxy> classRelationships() {
		if (_classRelationships == null) {
			Set<EORelationshipProxy> list = new HashSet<EORelationshipProxy>();
			for (EOPropertyProxy<?> property : this.classProperties()) {
				if (property instanceof EORelationshipProxy) {
					list.add((EORelationshipProxy) property);
				}
			}
			_classRelationships = new ArrayList<EORelationshipProxy>(list);
		}
		return _classRelationships;
	}

	/**
	 * Returns the entity class to one relationships
	 *
	 * @return entity class to one relationships
	 */
	public List<EORelationshipProxy> classToOneRelationships() {
		if (_classToOneRelationships == null) {
			Set<EORelationshipProxy> list = new HashSet<EORelationshipProxy>();
			for (EORelationshipProxy relationship : this.classRelationships()) {
				if (!relationship.proxiedObject().isToMany()) {
					list.add(relationship);
				}
			}
			_classToOneRelationships = new ArrayList<EORelationshipProxy>(list);
		}
		return _classToOneRelationships;
	}

	/**
	 * Returns the entity class to many relationships
	 *
	 * @return entity class to many relationships
	 */
	public List<EORelationshipProxy> classToManyRelationships() {
		if (_classToManyRelationships == null) {
			Set<EORelationshipProxy> list = new HashSet<EORelationshipProxy>();
			for (EORelationshipProxy relationship : this.classRelationships()) {
				if (relationship.proxiedObject().isToMany()) {
					list.add(relationship);
				}
			}
			_classToManyRelationships = new ArrayList<EORelationshipProxy>(list);
		}
		return _classToManyRelationships;
	}

	/**
	 * Returns the entity named fetch specifications
	 *
	 * @return entity named fetch specifications
	 */
	public List<EOFetchSpecificationProxy> classFetchSpecifications() {
		if (_classFetchSpecifications == null) {
			EOEntity entity = proxiedObject();
			Set<EOFetchSpecificationProxy> list = new HashSet<EOFetchSpecificationProxy>();
			for (String name : entity.fetchSpecificationNames()) {
				list.add(new EOFetchSpecificationProxy(name, entity));
			}
			_classFetchSpecifications = new ArrayList<EOFetchSpecificationProxy>(list);
		}
		return _classFetchSpecifications;
	}

}
