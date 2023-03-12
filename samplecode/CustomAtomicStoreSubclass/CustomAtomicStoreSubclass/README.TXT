CustomAtomicStoreSubclass

This example implements a custom CoreData store using the NSAtomicStore APIs. The store 
supports all CoreData property types.

AtomicStoreSubclass is the main class; it is a subclass of NSAtomicStore that:
	- converts between managed objects and an external representation of the objects
	- creates unique object identifiers
	- stores the objects in files using NSKeyedArchiver
	- manages store metadata
	
AtomicStoreCacheNodeSubclass is used to allow lazy relationship creation. If there are no relationships
being stored in a store, the developer is free to simply use the NSAtomicStoreCacheNode class itself.

The external format of this store is an archived dictionary (archived using NSKeyedArchiver)
with the following structure:

{	
	"Metadata" = {
				"NSStoreUUID" = <uuid> , 
				"NSStoreType" = "MyAtomicStoreSubclass" ,
				....
				}
	"Objects" = (
				{"idRefData" = <refdata>, "entityName" = <>, "propertyData" = { ... }},
				...
			   )
}

In order for this store to be used in a CoreData application, the store class must be registered with
the persistent store coordinator. This can be done by invoking +registerStoreClass:forStoreType: on the 
NSPersistentStoreCoordinator class with the appropriate parameters. Once a store has been registered,
you can add/remove/query instances of that store the way you would any of the stores shipped with
the framework.

Some things to note:

- AtomicStoreSubclass creates reference data for object identifiers by incrementing and returning 
	the new value of an NSNumber tracked in the metadata. This ensures that the reference data for an
	object will be unique across an entity hierarchy. It is not required that reference datas be
	unique across different hierarchies, but the implementation is simpler this way.
- AtomicStoreCacheNodeSubclass are created containing the exernal representation of the object. When
	valueForKey: is called, the external representation is unpacked and relationships are connected.
	This allows the store to defer the work of populating objects until they are needed. This
	mechanism is reused when nodes are updated, with the unpacked values being cleared and a new
	external representation being pushed into the node.
- AtomicStoreSubclass overrides willRemoveCacheNodes: and willRemoveFromPersistentStoreCoordinator:
	to break retain cycles created within the cache nodes.
