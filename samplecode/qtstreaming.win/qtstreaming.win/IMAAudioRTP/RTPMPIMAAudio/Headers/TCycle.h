/*
	File:		TCycle.h
	
	Contains:	Declaration of TCycle, a cyclic list element datatype
	
	Copyright:	© 1997-1999 by Apple Computer, Inc., all rights reserved.
	
*/



#ifndef __TCYCLE__
#define __TCYCLE__

#pragma once



typedef struct TCycle
{
	struct TCycle **	__itsNext;
} TCycle;



extern
TCycle **
CycleNew(
	unsigned long	inSize );

extern
void
CycleDispose(
	TCycle **	inElement );

extern
TCycle **
CyclePut(
	TCycle **	inList,
	TCycle **	inElement );

extern
TCycle **
CycleGet(
	TCycle **	inList );

extern
TCycle **
CycleNext(
	TCycle **	inElement );



#endif /* __TCYCLE__ */
