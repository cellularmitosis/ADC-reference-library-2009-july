Sample Code for WWDC 2006 session 302.

Demonstrates dynamically resolved properties. Uses a new feature of the
Objective C 2.0 runtime:  dynamically resolved methods. To support
dynamically resolved methods, a class may define two new methods:

+ (BOOL)resolveInstanceMethod:(SEL)name;
+ (BOOL)resolveClassMethod:(SEL)name;

The first time an unrecognized message is sent to an object, the runtime
checks to see if the class implements +resolveInstanceMethod:, and if so
calls it. +resolveInstanceMethod: should examine the selector and if it
should be resolved, call class_addMethod() with an appropriate method
implementation, and return YES. If the method returns NO, the method
will be handled by standard method forwarding, as before.
+resolveClassMethod: resolves unrecognized messages sent to a class in a
similar way, except that the method should be added to the metaclass.

Possible uses of dynamic method resolution:  faulting in method
implementations that reside in dynamically loaded libraries;  runtime
environment specialization, such as choosing between different method
implementations based on the capabilities of the OS or underlying
processor; lastly language bridges could use dynamic method resolution
to lazily bind their object wrapper methods.

This example shows how to define add statically typed properties to NSMutableDictionary, using categories and dynamic method resolution.