/*
	File:		TQueue.h

	Contains:	Declaration of TQueue, a generic queue datatype

	Copyright:	© 1997-1999 by Apple Computer, Inc., all rights reserved.

*/



#ifndef __TQUEUE__
#define __TQUEUE__

#pragma once



#include "TCycle.h"
#include <MacTypes.h>



typedef struct TQueue
{
	TCycle **	__itsList;
	UInt32		__itsCount;
} TQueue;



extern
void
QueueInitialize(
	TQueue *	inQueue );

extern
UInt32
QueueCount(
	const TQueue *	inQueue );

extern
void *
QueueHead(
	const TQueue *	inQueue );

extern
void *
QueueEnqueue(
	TQueue *	inQueue,
	void *		inElement );

extern
void *
QueueDequeue(
	TQueue *	inQueue );



#endif /* __TQUEUE__ */
