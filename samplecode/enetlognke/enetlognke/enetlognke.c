/*
(c) Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. (“Apple”) in 
consideration of your agreement to the following terms, and your use, installation, 
modification or redistribution of this Apple software constitutes acceptance of these 
terms.  If you do not agree with these terms, please do not use, install, modify or 
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to 
these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights 
in this original Apple software (the “Apple Software”), to use, reproduce, modify and 
redistribute the Apple Software, with or without modifications, in source and/or binary 
forms; provided that if you redistribute the Apple Software in its entirety and without 
modifications, you must retain this notice and the following text and disclaimers in all 
such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
or logos of Apple Computer, Inc. may be used to endorse or promote products derived 
from the Apple Software without specific prior written permission from Apple.  Except 
as expressly stated in this notice, no other rights or licenses, express or implied, 
are granted by Apple herein, including but not limited to any patent rights that may
 be infringed by your derivative works or by other works in which the Apple Software 
 may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO 
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES 
OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS. 

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING 
IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE 
APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/


#include <sys/lock.h>
#include <mach/vm_types.h>
#include <mach/kmod.h>
#include <sys/socket.h>
#include <sys/kpi_mbuf.h>
#include <net/kpi_interface.h>
#include <net/kpi_interfacefilter.h>
#include <sys/syslog.h>
#include <libkern/OSMalloc.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <netinet/in.h>
#include <kern/locks.h>
#include <sys/kern_event.h>
#include <stdarg.h>

#define SWALLOW_PACKETS		0	// set this define to 1 to demonstrate packet swallowing/re-injection
								// set this define to 0 for for simple filtering of data.
#define SHOWDEBUGMESSAGES	1	// set to 1 to show debug messages, else set to 0 to disable messages

#define MYBUNDLEID	"com.apple.dts.kext.enetlognke"
#define kMY_TAG_TYPE	1

// values to use with the memory allocated by the tag function
enum	{
	INBOUND_DONE	= 1,
	OUTBOUND_DONE
};

// 
#define NUM_HEADER_BYTES_TO_PRINT	18	// defines the number of bytes in the packet header to print

/* name of built-in ethernet name */
static const char		*gBuiltinEnetName = "en0";

#if SWALLOW_PACKETS
static OSMallocTag	gOSMallocTag;	// tag for use with OSMalloc calls which is used to associate memory
							// allocations made with this kext. Preferred to using MALLOC and FREE

/* Fine grain locking variables
   Protect consistency of our data in the queues where swallowed data is accessed 
*/
static lck_mtx_t		*g_input_mutex = NULL;	// for processing inbound packets
static lck_mtx_t		*g_output_mutex = NULL; // for processing outbound packets
static lck_grp_t		*gmutex_grp = NULL;

// SwallowPktQueue is the queue structure which describes the inbound and outbound packet queues
// where swallowed data is held for processing in the respective timer routines.
struct SwallowPktQueue {
	TAILQ_ENTRY(SwallowPktQueue) q_next; /* queued entries */
	ifnet_t				interface;
	mbuf_t				first_mbuf;
	protocol_family_t	protocol;
};

typedef struct SwallowPktQueue SwallowPktQueue;
TAILQ_HEAD(swallow_queue, SwallowPktQueue);
static struct swallow_queue	swallow_queue_in;
static struct swallow_queue	swallow_queue_out;
#endif // SWALLOW_PACKETS

/* tag associated with this kext for use in marking packets that have been previously processed. 
	Note that even if you don't swallow/re-inject packets, it's a good idea to mark them, unless
	you don't care whether you will see the same packet again. Another interface filter could
	swallow/re-inject the packet and your filter will be called to process the packet again.
*/
static mbuf_tag_id_t	gidtag;

static interface_filter_t	gEnetFilter;
static boolean_t			gFilterRegistered = FALSE;
static boolean_t			gUnregisterProc_started = FALSE;
static boolean_t			gUnregisterProc_complete = FALSE;

/* =================================== */
#pragma mark Utility Functions

static void
el_printf(const char *fmt, ...)
{
	va_list listp;
	char log_buffer[92];

#if !SHOWDEBUGMESSAGES
	return;
#endif
	va_start(listp, fmt);
	vsnprintf(log_buffer, sizeof(log_buffer), fmt, listp);
	printf("%s", log_buffer);
	va_end(listp);
}

#if SWALLOW_PACKETS
/*
	IMPORTANT: Note that in both the data_in_timer and data_out_timer routine, a fine grain
	lock is taken and held to check that there is data in the swallow queue to process, and to remove
	the queued item from the packet swallow list. Before the system call is made, the lock is 
	released as it's bad practice to hold a lock across a system call. Within the system call, a process
	might be called which will result in a call to acquire on the mutex and a deadlock could result if the
	lock were already held in the timer routine.
	
	The lock must be released before leaving the timer functions. If you find that 
	Ethernet traffic hangs, check for a mutex deadlock situation.
*/
static void
data_in_timer(void * unused)
{
	register struct SwallowPktQueue *swallow_queue_item;
	errno_t							result;

	lck_mtx_lock(g_input_mutex);	/* take the lock prior to checking the swallow_queue
									   so that it is held while the packet item is removed from the 
									   queue.
									   make sure to release the lock prior to making the system call
									*/
	while (!TAILQ_EMPTY(&swallow_queue_in))
	{
		// get the next item on the input side queue
		swallow_queue_item = TAILQ_FIRST(&swallow_queue_in);
		// remove this item from the queue
		TAILQ_REMOVE(&swallow_queue_in, swallow_queue_item, q_next);

		if (mbuf_pkthdr_header(swallow_queue_item->first_mbuf) == NULL)
		{
			printf("packet with NULL pkthdr found in timer\n");
			mbuf_freem(swallow_queue_item->first_mbuf);
			// free the queue element
			OSFree(swallow_queue_item, sizeof(SwallowPktQueue), gOSMallocTag);
			continue;
		}
		lck_mtx_unlock(g_input_mutex); // release the lock prior to making the system call
		
		result = ifnet_input(swallow_queue_item->interface, swallow_queue_item->first_mbuf, NULL);
		if (result)
		{
			el_printf("error calling ifnet_input, dropping data - result was %d\n", result);
			mbuf_freem(swallow_queue_item->first_mbuf);
		}
		// free the queue element
		OSFree(swallow_queue_item, sizeof(SwallowPktQueue), gOSMallocTag);
		// we've already tagged the packet so go on to the next packet, if queued
		// but first aquire the lock so that we can check the queue and remove the swallow packet item
		// atomicly.
		lck_mtx_lock(g_input_mutex);
	}
	lck_mtx_unlock(g_input_mutex); // release the lock
}

#endif 

#if SWALLOW_PACKETS
static void
data_out_timer(void * unused)
{
	register struct SwallowPktQueue 	*swallow_queue_item;
	errno_t							result;

	lck_mtx_lock(g_output_mutex);	/* take the lock prior to checking the swallow_queue
									   so that it is held while the packet item is removed from the 
									   queue.
									   make sure to release the lock prior to making the system call
									*/
	while (!TAILQ_EMPTY(&swallow_queue_out))
	{
		// get the next item on the input side queue
		swallow_queue_item = TAILQ_FIRST(&swallow_queue_out);
		// remove this item from the queue
		TAILQ_REMOVE(&swallow_queue_out, swallow_queue_item, q_next);
		lck_mtx_unlock(g_output_mutex);	// release the lock prior to making the system call
		
		result = ifnet_output_raw(swallow_queue_item->interface, swallow_queue_item->protocol, swallow_queue_item->first_mbuf);
		if (result)
		{
			el_printf("error calling ifnet_output_raw, dropping data - result was %d\n", result);
			mbuf_freem(swallow_queue_item->first_mbuf);
		}
		// free the queue element
		OSFree(swallow_queue_item, sizeof(SwallowPktQueue), gOSMallocTag);
		// we've already tagged the packet so go on to the next packet, if queued
		// but first aquire the lock so that we can check the queue and remove the swallow packet item
		// atomicly.
		lck_mtx_lock(g_output_mutex);
	}
	lck_mtx_unlock(g_output_mutex);	// release the lock
}
#endif

#if SWALLOW_PACKETS

/* 
	alloc_locks - used to allocate the fine grain locks for use in controlling access to the SwallowPktQueue
				structures. 
	input - nothing
	output - result of allocation of lock group and lock
			0 - SUCCESS
			ENOMEM - an error in allocation of memory for the attributes or locks.
 */

static errno_t alloc_locks(void)
{
	errno_t	result = 0;
	lck_grp_attr_t	*grp_attributes = NULL;
	lck_attr_t		*lck_attributes = NULL;

    /* Allocate a mutex lock group */
	/* allocate a lock group attribute var */
    grp_attributes = lck_grp_attr_alloc_init();
	if (grp_attributes)
	{
		/* set the default values for the lock group attribute var */
		lck_grp_attr_setdefault(grp_attributes);
		// for the name, use the reverse dns name associated with this
		// kernel extension
		/* allocate the lock group */
		gmutex_grp = lck_grp_alloc_init(MYBUNDLEID, grp_attributes);
		if (gmutex_grp == NULL)
		{
			el_printf("error calling lck_grp_alloc_init\n");
			result = ENOMEM;
		}
		// can free the attributes once we've allocated the group lock
		lck_grp_attr_free(grp_attributes);
	}
	else
	{
		el_printf("error calling lck_grp_attr_alloc_init\n");
		result = ENOMEM;
	}
	
	if (result == 0)
	{
		/* allocate a lock attribute var */
		lck_attributes = lck_attr_alloc_init();
		if (lck_attributes)
		{
			/* allocate the lock for use on processing items in the input queue */
			g_input_mutex = lck_mtx_alloc_init(gmutex_grp, lck_attributes);
			if (g_input_mutex == NULL)
			{
				el_printf("error calling lck_mtx_alloc_init\n");
				result = ENOMEM;
			}
			if (result == 0)
			{
				/* allocate the lock for use on processing items in the output queue */
				g_output_mutex = lck_mtx_alloc_init(gmutex_grp, lck_attributes);
				if (g_output_mutex == NULL)
				{
					el_printf("error calling lck_mtx_alloc_init\n");
					result = ENOMEM;
				}
			}
			// can free the attributes once we've allocated the lock
			lck_attr_free(lck_attributes);
		}
		else
		{
			el_printf("error calling lck_attr_alloc_init\n");
			result = ENOMEM;
		}
	}
	return result;	// if we make it here, return success
}
#endif

#if SWALLOW_PACKETS
/* 
	free_locks - used to free the fine grain locks for use in controlling access to the SwallowPktQueue
				structures. 
	input - nothing
	output - nothing - since all of the kernel calls return void results.
 */

static void free_locks(void)
{
	if (g_input_mutex)
	{
		lck_mtx_free(g_input_mutex, gmutex_grp);
		g_input_mutex = NULL;
	}
	if (g_output_mutex)
	{
		lck_mtx_free(g_output_mutex, gmutex_grp);
		g_output_mutex = NULL;
	}
	if (gmutex_grp)
	{
		lck_grp_free(gmutex_grp);
		gmutex_grp = NULL;
	}
	if (gOSMallocTag)
	{
		OSMalloc_Tagfree(gOSMallocTag);
		gOSMallocTag = NULL;
	}
}
#endif

/*
	CheckTag - see if there is a tag associated with the mbuf_t with the matching bitmap bits set in the
				memory associated with the tag. Use global gidtag as id Tag to look for
	input m - mbuf_t variable on which to search for tag
		  value - value to compare in the tag_ref field associated with the tag
	return 1 - success, the bitmap image set in allocated memory associated with tag gidtag has the same bits set
					as does the bitmap
	return 0 - failure, either the mbuf_t is not tagged, or the allocated memory does not have the 
				expected value	
*/
static int	CheckTag(mbuf_t m, int value)
{
	errno_t	status;
	int		*tag_ref;
	size_t	len;
	
	// Check whether we have seen this packet before.
	status = mbuf_tag_find(m, gidtag, kMY_TAG_TYPE, &len, (void**)&tag_ref);
	if ((status == 0) && (*tag_ref & value) && (len == sizeof(int))) 
		return 1;
		
	return 0;
}

/*
	SetTag - Set the tag associated with the mbuf_t with the bitmap bits set in bitmap
	input m - mbuf_t variable on which to search for tag
		  bitmap - bitmap field to set in allocated memory
	return 0 - success, the tag has been allocated and for the mbuf_t and the bitmap bits has been set in the
				allocated memory. 
		   anything else - failure
	
*/
static errno_t	SetTag(mbuf_t m, int value)
{	
	errno_t status;
	int *tag_ref;
	size_t len;
	
	// look for existing tag
	status = mbuf_tag_find(m, gidtag, kMY_TAG_TYPE, &len, (void*)&tag_ref);
	// allocate tag if needed
	if (status != 0) {
		// note that setting the MBUF_DONTWAIT flag for mbuf_tag memory allocation while within a packet
		// processing call will not deadlock packet processing under OS X 10.4 and greater.
		status = mbuf_tag_allocate(m, gidtag, kMY_TAG_TYPE, sizeof(int), MBUF_DONTWAIT, (void**)&tag_ref);
		if (status == 0)
			*tag_ref = 0;		// init tag_ref
		else
			printf("mbuf_tag_allocate failed - result was %d", status);
	}
	else
	{
		// the tag exists - verify that the length of the tag_ref is the expected size
		// this should not happen
		if (len != sizeof(int))
		{
			printf("tag detected at incorrect length - %d\n", len);
			status = EINVAL;	// invalid argument detected.
		}
	}
	if (status == 0) 
		*tag_ref = value;
	return status;
}

/*
	PrintPacketHeader - prints the first N bytes of the Ethernet packet as specified by
			NUM_HEADER_BYTES_TO_PRINT
		input: data - pointer to  mbuf_t of the packet
*/
static void	
PrintPacketHeader(mbuf_t *data)
{
	int			bytesLeftToPrint = NUM_HEADER_BYTES_TO_PRINT;
	int			i, j, bytesToPrintNow;
	mbuf_t		m = *data;
	char		*frame;
	
	j = 0;
	do
	{
		bytesToPrintNow = mbuf_len(m);	// count the number of bytes in the mbuf
		if (bytesToPrintNow > bytesLeftToPrint)	// limit the number of bytes to print
		{
			bytesToPrintNow = bytesLeftToPrint;
		}
		bytesLeftToPrint -= bytesToPrintNow;	// decrement bytesLeftToPrint to what remains 
												// to be printed
		
		frame = mbuf_data(m);
		for (i = 0; i < bytesToPrintNow; i++, j++)
		{
			el_printf("%02X", (u_int8_t)frame[i]);
			if (j == 5) el_printf("  "); // print space after the destination address
			if (j == 11) el_printf("  "); // print space after the source address
			if (j == 13) el_printf("  "); // print space after the protocol/length field
		}
		m = mbuf_next(m);
	} while ((m != NULL) && (bytesLeftToPrint != 0));
	
}

/* =================================== */
#pragma mark Filter Functions
/*!
	@typedef iff_input_func
	
	@discussion iff_input_func is used to filter incoming packets. The
		interface is only valid for the duration of the filter call. If
		you need to keep a reference to the interface, be sure to call
		ifnet_reference and ifnet_release.
	@param cookie The cookie specified when this filter was attached.
	@param interface The interface the packet was recieved on.
	@param protocol The protocol of this packet. If you specified a
		protocol when attaching your filter, the protocol will only ever
		be the protocol you specified. By passing the protocol to your
		filter, you can supply the same function pointer to filters
		registered for different protocols and differetiate the protocol
		using this parameter.
	@param data The inbound packet, may be changed.
	@param frame_ptr A pointer to the pointer to the frame header.
	@result Return:
		0 - The caller will continue with normal processing of the packet.
		EJUSTRETURN - The caller will stop processing the packet, the packet will not be freed.
		Anything Else - The caller will free the packet and stop processing.
*/
static errno_t 
enet_input_func(void* cookie, ifnet_t interface, protocol_family_t protocol,
								  mbuf_t *data, char **frame_ptr)
{
#if SWALLOW_PACKETS
	struct SwallowPktQueue	*swallow_queue_item;
	struct timespec			ts;
#endif
	mbuf_t		m = *data;
	u_int32_t	bytes = 0;
	errno_t		err;

	/* check whether we have seen this packet previously */
	if (CheckTag(*data, INBOUND_DONE))
	{
		/* we have processed this packet previously since the INBOUND_DONE bit was set.
			bail on further processing
		*/
		return 0;
	}
	
	el_printf("enet_input_func     - ");
	switch (protocol)
	{
		case AF_UNSPEC:     el_printf("UNSPEC    - "); break;
		case AF_INET:		el_printf("TCP/IP    - "); break;
		case AF_APPLETALK:	el_printf("AppleTalk - "); break;
		default:			el_printf("proto  %d - ", protocol); break;
	}

	PrintPacketHeader(data);

	do
	{
		bytes += mbuf_len(m);
		m = mbuf_next(m);
	} while (m != NULL);
	el_printf("  %d bytes incoming\n", bytes);

	/*
		If we swallow the packet and later re-inject the packet, we have to be
		prepared to see the packet through this routine once again. In fact, if
		after re-injecting the packet, another nke swallows and re-injects the packet
		we will see the packet an additional time. Rather than cache the mbuf_t reference
		we tag the mbuf_t and search for it's presence which we have already done above
		to decide if we have processed the packet.
		In fact, it's a good idea to tag the packet anyway, since some other filter could 
		swallow/re-inject the packet and we'd end up seeing the packet again.
				
	*/
	err = SetTag(*data, INBOUND_DONE);

	if (err == 0)
	{
#if SWALLOW_PACKETS
		/* allocate a queue entry so that we can store the packet on the ENET inbound process queue.
		   NOTE: the OSMalloc_nowait and OSMalloc_noblock variants of OSMalloc, do not need to 
		   be used while in a packet processing routine. OSMalloc will not deadlock with the packet
		   processing call.
		*/ 
		swallow_queue_item = (struct SwallowPktQueue *)OSMalloc(sizeof (struct SwallowPktQueue), gOSMallocTag);
		if (swallow_queue_item)
		{
			// before we swallow the packet, we need to make sure that the very first mbuf
			// has the pkthdr_header field set, else the system will panic when we inject
			// the packet.  The input routine is passed the frame_header pointer, so we can use
			// this value to set as the pkthdr_header.
			if (mbuf_pkthdr_header(*data) == NULL)
			{
				mbuf_pkthdr_setheader(*data, *frame_ptr);
			}
			swallow_queue_item->interface = interface;
			swallow_queue_item->protocol = protocol;
			swallow_queue_item->first_mbuf = *data;
			
			lck_mtx_lock(g_input_mutex);
			// queue the item into the input queue for processing when the timer fires
			TAILQ_INSERT_TAIL(&swallow_queue_in, swallow_queue_item, q_next);	
			lck_mtx_unlock(g_input_mutex);
			// initiate timer action in 10 microsec - for demonstration purposes to show the
			// continued processing of the packet
			ts.tv_sec = 0;
			ts.tv_nsec = 10000;
			bsd_timeout(data_in_timer, NULL, &ts);	
		}
		else
		{
			printf("Error - failed to allocate memory for inbound queue item, dropping packet.\n");
			return ENOMEM;	// stops processing and will cause the mbuf_t to be released
		}
#endif	// #if SWALLOW_PACKETS
	}
	else
	{
		//  mbuf_tag_allocate failed.
		printf("Error - mbuf_tag_allocate returned an error %d\n", err);
		return err;	// stops processing and will cause the mbuf_t to be released
					/*
						Note that this portion of code will be executed even in the non-packet
						swallow case. This code returns the error which will cause the packet to
						be dropped. 
					*/
	}
#if SWALLOW_PACKETS
	return EJUSTRETURN;		// we're going to deal with the packet one way or the other
#else
	return 0;
#endif
}
								  
/*!
	@typedef iff_output_func
	
	@discussion iff_output_func is used to filter fully formed outbound
		packets. This function is called after the protocol specific
		preoutput function. The interface is only valid for the duration
		of the filter call. If you need to keep a reference to the
		interface, be sure to call ifnet_reference and ifnet_release.
	@param cookie The cookie specified when this filter was attached.
	@param interface The interface the packet is being transmitted on.
	@param data The outbound packet, may be changed.
	@result Return:
		0 - The caller will continue with normal processing of the packet.
		EJUSTRETURN - The caller will stop processing the packet, the packet will not be freed.
		Anything Else - The caller will free the packet and stop processing.
*/
static errno_t 
enet_output_func(void* cookie, ifnet_t interface, protocol_family_t protocol,
								   mbuf_t *data)
{
#if SWALLOW_PACKETS
	struct SwallowPktQueue	*swallow_queue_item;
	struct timespec	ts;
#endif
	int				byteInPacket;
	mbuf_t			m = *data;
	errno_t			err;

	/* check whether we have seen this packet previously */
	if (CheckTag(*data, OUTBOUND_DONE))
	{
		/* we have processed this packet previously since the OUTBOUND_DONE bit was set.
			bail on further processing
		*/
		return 0;
	}
	
	/*
		If we reach this point, then we have not previously seen this packet. 
		First lets get some statistics from the packet.
	*/
	
	el_printf("enet_output_func    - ");
	switch (protocol)
	{
		case AF_UNSPEC:     el_printf("UNSPEC    - "); break;
		case AF_INET:		el_printf("TCP/IP    - "); break;
		case AF_APPLETALK:	el_printf("AppleTalk - "); break;
		default:			el_printf("proto  %d - ", protocol); break;
	}

	// count the number of outgoing bytes
	byteInPacket = 0;
	do
	{
		byteInPacket += mbuf_len(m);
		m = mbuf_next(m);
	} while (m != NULL);
	
	PrintPacketHeader(data);
	el_printf("  %d bytes outgoing\n", byteInPacket);

	/*
		If we swallow the packet and later re-inject the packet, we have to be
		prepared to see the packet through this routine once again. In fact, if
		after re-injecting the packet, another nke swallows and re-injects the packet
		we will see the packet an additional time. Rather than cache the mbuf_t reference
		we tag the mbuf_t and search for it's presence which we have already done above
		to decide if we have processed the packet.
	*/
	err = SetTag(*data, OUTBOUND_DONE);
	
	if (err == 0)
	{
#if SWALLOW_PACKETS
		/* allocate a queue entry so that we can store the packet on the ENET outbound process queue.
		   NOTE: the OSMalloc_nowait and OSMalloc_noblock variants of OSMalloc, do not need to 
		   be used while in a packet processing routine. OSMalloc will not deadlock with the packet
		   processing call.
		*/ 
		swallow_queue_item = (struct SwallowPktQueue *)OSMalloc(sizeof (struct SwallowPktQueue), gOSMallocTag);
		if (swallow_queue_item)
		{
			swallow_queue_item->interface = interface;
			swallow_queue_item->protocol = protocol;
			swallow_queue_item->first_mbuf = *data;
			
			lck_mtx_lock(g_output_mutex);
			// queue the item into the output queue for processing when the timer fires
			TAILQ_INSERT_TAIL(&swallow_queue_out, swallow_queue_item, q_next);	
			lck_mtx_unlock(g_output_mutex);
			// initiate timer action in 10 microsec
			ts.tv_sec = 0;
			ts.tv_nsec = 10000;
			bsd_timeout(data_out_timer, NULL, &ts);	
		}
		else
		{
			printf("Error - failed to allocate memory for outbound queue item, dropping packet.\n");
			mbuf_freem(*data);
		}
#endif
	}
	else
	{
		//  mbuf_tag_allocate failed.
		printf("Error - mbuf_tag_allocate returned an error %d\n", err);
		return err;	// stops processing and will cause the mbuf_t to be released
					/*
						Note that this portion of code will be executed even in the non-packet
						swallow case. This code returns the error which will cause the packet to
						be dropped. 
					*/
	}
#if SWALLOW_PACKETS
	return EJUSTRETURN;
#else
	return 0;
#endif
}

/*!
	@typedef iff_event_func
	
	@discussion iff_event_func is used to filter interface specific
		events. The interface is only valid for the duration of the
		filter call. If you need to keep a reference to the interface,
		be sure to call ifnet_reference and ifnet_release.
	@param cookie The cookie specified when this filter was attached.
	@param interface The interface the packet is being transmitted on.
	@param event_msg The kernel event, may not be changed.
*/
static void 
enet_event_func(void* cookie, ifnet_t interface, protocol_family_t protocol,
							   const struct kev_msg *event_msg)
{
	el_printf("enet_event_func     -  vendor %ld, class %ld, subclass %ld, event code %ld\n",
					event_msg->vendor_code, event_msg->kev_class, 
					event_msg->kev_subclass, event_msg->event_code);

	return;
}

/*!
	@typedef iff_ioctl_func
	
	@discussion iff_ioctl_func is used to filter ioctls sent to an
		interface. The interface is only valid for the duration of the
		filter call. If you need to keep a reference to the interface,
		be sure to call ifnet_reference and ifnet_release.
	@param cookie The cookie specified when this filter was attached.
	@param interface The interface the packet is being transmitted on.
	@param ioctl_cmd The ioctl command.
	@param ioctl_arg A pointer to the ioctl argument.
	@result Return:
		(NOTE: The following return result description is a correction to the return results presented 
			in kpi_interfacefilter.h provided with 10.4.
		EOPNOTSUPP(or ENOTSUP) - indicates that the ioctl was not processed - the caller will 
				continue with normal processing of the packet.
		EJUSTRETURN - indicates that no further processing of the ioctl is desired - the caller 
				will stop processing the packet, the packet will not be freed.
		0 - indicates that the ioctl was handled, however, the caller will continue processing 
			the packet with other filter functions.
		Anything Else - The caller will free the packet and stop processing.
*/
static errno_t 
enet_ioctl_func(void* cookie, ifnet_t interface, protocol_family_t protocol,
								  u_long ioctl_cmd, void* ioctl_arg)
{
	el_printf("enet_ioctl_func     - ");
	switch (protocol)
	{
		case AF_INET:		el_printf("TCP/IP,"); break;
		case AF_APPLETALK:	el_printf("AppleTalk,"); break;
		default:			el_printf("%d,", protocol); break;
	}
	el_printf(" cmd is 0x%X\n", ioctl_cmd);
	return EOPNOTSUPP;
}

/*!
	@typedef iff_detached_func
	
	@discussion iff_detached_func is called to notify the filter that it
		has been detached from an interface. This is the last call to
		the filter that will be made. A filter may be detached if the
		interface is detached or the detach filter function is called.
		In the case that the interface is being detached, your filter's
		event function will be called with the interface detaching event
		before the your detached function will be called.
	@param cookie The cookie specified when this filter was attached.
	@param interface The interface this filter was detached from.
*/
static void 
enet_detached_func(void* cookie, ifnet_t interface)
{
	gUnregisterProc_complete = TRUE;
	gFilterRegistered = FALSE;
	printf("enet_detached_func entered\n");
	return;
}

static struct iff_filter Enet_filter = {
	NULL,				/* iff_cookie field */
	MYBUNDLEID,
	0,					/* interested in all protocol packets */
	enet_input_func,
	enet_output_func,
	enet_event_func,
	enet_ioctl_func,
	enet_detached_func
};

/* =================================== */
#pragma mark kext entry points

kern_return_t com_dts_apple_kext_enetlognke_start (kmod_info_t * ki, void * d) 
{
	kern_return_t	retval;
	ifnet_t			enetifnet;
	
	printf("enetlognke_start entered\n");
	if (gFilterRegistered == TRUE)
		return KERN_SUCCESS;

#if SWALLOW_PACKETS	
	retval = alloc_locks();
	if (retval)
		goto error;	

	TAILQ_INIT(&swallow_queue_in);		// will hold inbound swallowed packets
	TAILQ_INIT(&swallow_queue_out);		// will hold outbound swallowed packets
	
	gOSMallocTag = OSMalloc_Tagalloc(MYBUNDLEID, OSMT_DEFAULT); // don't want the flag set to OSMT_PAGEABLE since
									// it would indicate that the memory was pageable.
	if (gOSMallocTag == NULL)
		goto error;	
#endif

	// set up the tag value associated with this NKE in preparation for swallowing packets and re-injecting them
	retval = mbuf_tag_id_find(MYBUNDLEID , &gidtag);
	if (retval != 0)
	{
		el_printf("mbuf_tag_id_find returned error %d\n", retval);
		goto error;
	}
				
	retval = ifnet_find_by_name(gBuiltinEnetName, &enetifnet);
	if (retval == KERN_SUCCESS)
	{
		retval = iflt_attach(enetifnet, &Enet_filter, &gEnetFilter);
		// release the reference on the interface name
		ifnet_release(enetifnet);
	}
	if (retval == KERN_SUCCESS)
		gFilterRegistered = TRUE;
	else
		goto error;
	
    return retval;

error:
#if SWALLOW_PACKETS
	free_locks();
#endif
	return KERN_FAILURE;
}


kern_return_t com_dts_apple_kext_enetlognke_stop (kmod_info_t * ki, void * d) 
{
	kern_return_t		retval = KERN_FAILURE;	// default result, unless we know that we are 
												// detached from the interface.
	
	if (gFilterRegistered == FALSE)
		return KERN_SUCCESS;
	
	if (gUnregisterProc_started == FALSE)
	{
		// only want to start the detach process once.
		iflt_detach(gEnetFilter);
		gUnregisterProc_started = TRUE;
	}
	
	if (gUnregisterProc_complete)
	{
		retval = KERN_SUCCESS;
	}
	else
	{
		el_printf("enetlognke_stop: incomplete\n");
	}

	if (retval == KERN_SUCCESS)
	{
#if SWALLOW_PACKETS
	free_locks();
#endif
	}
    return retval;
}
