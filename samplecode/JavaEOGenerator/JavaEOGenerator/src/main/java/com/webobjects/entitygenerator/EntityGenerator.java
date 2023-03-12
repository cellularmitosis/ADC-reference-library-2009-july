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
import java.util.ArrayList;
import java.util.List;

import com.webobjects.appserver.WOApplication;
import com.webobjects.eoaccess.EOEntity;
import com.webobjects.eoaccess.EOModel;
import com.webobjects.eoaccess.EOModelGroup;
import com.webobjects.eocontrol.EOGenericRecord;
import com.webobjects.generator.Configuration;
import com.webobjects.generator.Generator;
import com.webobjects.generator.GeneratorComponent;
import com.webobjects.generator.GeneratorContext;

/**
 * @since 5.4
 */
public class EntityGenerator extends Generator {

	/**
	 *
	 */
	protected EntityGenerator() {
		super();
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#createContext()
	 */
	@Override
	protected GeneratorContext createContext() {
		return new EntityContext((EntityConfiguration) _configuration);
	}

	/**
	 * Return the generation context
	 *
	 * @return entity generation context
	 */
	public EntityContext eoContext() {
		return (EntityContext) this.context();
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#createConfiguration(java.lang.String[])
	 */
	@Override
	protected Configuration createConfiguration(String[] argv) {
		return new EntityConfiguration(argv);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#println(java.lang.String)
	 */
	@Override
	protected void println(String log) {
		System.out.println(log);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#buildHelpHeader(java.lang.StringBuilder)
	 */
	@Override
	protected void buildHelpHeader(StringBuilder aLog) {
		aLog.append("Usage: JavaEOGenerator -model EOModelName.eomodeld [options ...] [entities ...]\n\n");
		super.buildHelpHeader(aLog);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#showVersion()
	 */
	@Override
	public void showVersion() {
		// We force the version display even if verbose is not set
		this.logStatement("JavaEOGenerator " + WOApplication.application().getWebObjectsVersion(), true);
	}

	/**
	 * Return the url to the model
	 *
	 * @return model url
	 */
	public URL model() {
		return (_configuration != null ? ((EntityConfiguration) _configuration).model() : null);
	}

	/**
	 * Return the url to the prototypes model
	 *
	 * @return model url
	 */
	public URL referenceModel() {
		return (_configuration != null ? ((EntityConfiguration) _configuration).referenceModel() : null);
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#generate()
	 */
	@Override
	public void generate() {
		if (this.model() != null) {
			// Load the models
			EOModel targetModel = null;
			try {
				targetModel = new EOModel(this.model());
				EOModelGroup group = new EOModelGroup();
				group.addModel(targetModel);
				if (this.referenceModel() != null)
					group.addModelWithPathURL(this.referenceModel());
				EOModelGroup.setDefaultGroup(group);
			} catch (Exception exception) {
				this.logStatement("Cannot load model: '" + this.model() + "'", true);
			}
			// And Iterate through the entities
			for (EOEntity anEntity : this.entitiesForModel(targetModel)) {
				try {
					this.logStatement("Generating for entity: '" + anEntity.name() + "'");
					this.generateForTargetObject(new EOEntityProxy(anEntity));
					this.logStatement("Finished generating for entity: '" + anEntity.name() + "'");
				} catch (Exception exception) {
					exception.printStackTrace();
					this.logStatement("Cannot generate file for entity: '" + anEntity.name() + "' exception " + exception, true);
				}
			}
		}
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#componentForTargetObject(java.lang.Object)
	 */
	@SuppressWarnings("unchecked")
	@Override
	protected <T> GeneratorComponent<T> componentForTargetObject(T targetObject) {
		if (targetObject instanceof EOEntityProxy) {
			return (GeneratorComponent<T>) new EntityComponent(this.eoContext());
		} else {
			return (GeneratorComponent<T>) new EntityBaseComponent(this.eoContext());
		}
	}

	/**
	 * Builds the list of entities for the source generation
	 *
	 * @param model
	 * @return entities list
	 */
	protected List<EOEntity> entitiesForModel(EOModel model) {
		List<EOEntity> entityList = new ArrayList<EOEntity>();
		if ((_configuration != null) && _configuration.hasArguments()) {
			// We generate for a subset of the entities
			for (String entityName : _configuration.arguments()) {
				EOEntity anEntity = EOModelGroup.defaultGroup().entityNamed(entityName);
				if (anEntity != null) {
					entityList.add(anEntity);
				}
			}
		}
		if ((entityList.size() == 0) && (model != null)) {
			// We need to remove entities that are not subclassed
			for (EOEntity anEntity : model.entities()) {
				if (!EOGenericRecord.class.getName().equals(anEntity.className())) {
					entityList.add(anEntity);
				}
			}
		}
		return entityList;
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see com.webobjects.generator.Generator#defaultFileNameForObject(java.lang.Object)
	 */
	@Override
	protected String defaultFileNameForObject(Object targetObject) {
		if (targetObject instanceof EOEntity) {
			return ((EOEntityProxy) targetObject).proxiedObject().className();
		} else {
			return super.defaultFileNameForObject(targetObject);
		}
	}

}