NetworkAuthentication

Copyright (c) 2005 Apple Computer, Inc.
All rights reserved.

________________________________________________________________________________

About
NetworkAuthentication

NetworkAuthentication is a collection of sample code that demonstrates how to
authenticate users using Directory Services.  Example include three forms of
authentication:  CRAM-MD5, plaintext password, and GSSAPI.

The routines were designed for applications running on Mac OS X 10.4.
The project and files were built with XCode 2.0

Sample client / server authentication and user lookup applications are included 
in this Sample Code.

	lookupuser <username>
	demoserver <port>
	democlient <options>

In order to test GSSAPI, demoserver must be run as root and the machine that 
is running it must have a keytab.

________________________________________________________________________________

Files
in the NetworkAuthentication Package

The routines source files below are re-usable routines that can be used by 
applications to simplify handling various forms of authentications.

	DSUtility.h
	DSUtility.c
	GSSauthenticate.h
	GSSauthenticate.c
    
Other source files provided are purely support files and code for the example.

________________________________________________________________________________

How
to use NetworkAuthentication

You can cut and paste portions of it into your programs. You can use it as an
example. Since NetworkAuthentication is sample code, many routines are there 
simply to show you how to use the Directory Service APIs. If a routine does more 
or less than what you want, you can have the source so you can modify it to do 
exactly you want it to do. Feel free to rip NetworkAuthentication off and modify 
its code in whatever ways you find work best for you.

To use the built applications you simply launch the demoserver with a port,
(e.g., "demoserver 2500").  Then from the same client or a different client
launch the democlient against that server with some method (e.g.,
"democlient -m CRAM-MD5 -h 127.0.0.1 -t 2500 -u testuser -p apple").  The full
usage output is below.

If testing GSSAPI, you should not use loopback as it will not resolve the name
of the host correctly, so I recommend using the real IP address of the host.

Usage:  democlient -m method -h ipaddress -t port [-u username]
                   [-p password] [-S service] 
        -m   method 'cleartext', 'CRAM-MD5', 'GSSAPI'
        -h   host IP address or dns name
        -t   port for server
        -u   username to authenticate (except GSSAPI)
        -p   password to authenticate (except GSSAPI)
        -S   service principal 'host' to use (GSSAPI)


________________________________________________________________________________

Documentation

The documentation for the routines can be found in the header files. There, you'll
find function prototypes, and a description of each call that includes a
complete listing of all input and output parameters. For example, here's the
function prototype and documentation for one of the routines, DoPasswordAuth.

/*!
    @function	DoPasswordAuth
    @abstract   Will take a record name and cleartext password to authenticate a user
    @discussion Authenticates a recordname with the supplied password.  This does not mean the password
				will be sent in the clear, it just means the password is cleartext-based.
	@param      inDSRef Existing tDirReference from dsOpenDirService
	@param		inNodeRef The node found with LocateUserRecordNameAndNode for the user to be authenticated
	@param		inAuthMethod The method being used (e.g., kDSStdAuthNodeNativeNoClearText)
	@param		inRecordName The record name returned by LocateUserRecordNameAndNode for the specific user
	@param		inPassword Is a cleartext password supplied by the user
	@result     Will return eDSNoErr if successful, otherwise any error that may have occurred
*/
tDirStatus DoPasswordAuth( tDirReference inDSRef, tDirNodeReference inNodeRef, const char *inAuthMethod,
						   const char *inRecordName, const char *inPassword );

