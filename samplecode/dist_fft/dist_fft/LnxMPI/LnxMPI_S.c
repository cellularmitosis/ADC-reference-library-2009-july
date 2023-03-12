/* Partial MPI library based on Sockets in Linux,
   using TCP/IP protocol.
   No local buffering of messages is implemented, so that all messages
   must be received in the order sent, and receives with wildcard
   sources are not supported. the following subroutines are implemented:
   MPI_Init, MPI_Finalize, MPI_Send, MPI_Recv, MPI_Isend, MPI_Irecv
   MPI_Test, MPI_Wait, MPI_Sendrecv, MPI_Ssend, MPI_Issend, MPI_Waitall
   MPI_Waitany, MPI_Get_count, MPI_Initialized, MPI_Comm_size
   MPI_Comm_rank, MPI_Comm_dup, MPI_Comm_split, MPI_Comm_free
   MPI_Cart_create, MPI_Cart_coords, MPI_Cart_get, MPI_Cart_shift
   MPI_Cart_rank, MPI_Cart_sub, MPI_Dims_create
   MPI_Bcast, MPI_Barrier, MPI_Reduce, MPI_Scan
   MPI_Allreduce, MPI_Gather, MPI_Allgather, MPI_Scatter, MPI_Alltoall
   MPI_Gatherv, MPI_Allgatherv, MPI_Scatterv, MPI_Alltoallv
   MPI_Reduce_scatter, MPI_Abort, MPI_Wtime, MPI_Wtick, MPI_Type_extent
   MPI_Request_free, MPI_Get_processor_name
   Sockets are described in Unix: Network Programming, vol. 1, by
   W. Richards Stevens [Prentice Hall PTR, Upper Saddle River, NJ, 1998].
   The Message Passing Interface (MPI) is described in the reference,
   M. Snir, S. Otto, S. Huss-Lederman, D. Walker, and J. Dongarra,
   MPI: The Complete Reference [MIT Press, Cambridge, MA,1996].
   The file MPIerrs is used throughout for error messages
   written by viktor k. decyk, ucla
   copyright 2002, regents of the university of california.
   all rights reserved.
   no warranty for proper operation of this software is given or implied.
   software or information may be copied, distributed, and used at own
   risk; it may not be distributed without this notice included verbatim
   with each file. 
   update: january 26, 2004                                              */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

/* MAXS = maximum number of nodes connected */
#define MAXS                 64
/* MAXM = maximum number of outstanding messages on a node */
#define MAXM                 (2*MAXS)
/* MAXC = maximum number of communicators */
#define MAXC                 10
/* MAXD = maximum number of topology dimensions */
#define MAXD                 6

/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   epref = array of socket endpoints for each participating node
   ioc = array of context pointers for notifier function
   stime = first time stamp if MPI_Init successful
   mapcomm = communicator map
   maxfds = maximum descriptor value
   fdsset = fd_set reprsenting all endpoints
   sset = signal mask                                               */

   static int nproc = -1, idproc;
   static int epref[MAXS+2];
   static int ioc[MAXS+2][4], mapcomm[MAXC][MAXS+MAXD+3];
   static struct timeval stime;
   static int maxfds;
   static fd_set fdsset;
   static sigset_t sset;
      
/* internal common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages
   curreq = request record for transmission parameters
   header = message envelope
   rwrec = read/write record for asynchronous messages
   trash = trash bin for unwanted data
   mqueue = message request queue                                     */

   struct RWRec {
      int ref;
      int ioflag;
      void *buf;
      size_t nbytes;
      int flags;
      void *sbuf;
      int sln;
      int len;
      struct timeval ts[2];
      int nextm;
      int nfatal;
   };
   struct ipdata {
      struct iovec iov[2];
      int hdata[4];
   };
   static int monitor = 1, curreq[MAXM][5], mqueue[MAXS+2][2];
   static struct ipdata header[MAXM];
   static struct RWRec rwrec[MAXM];
   static unsigned char trash[1024];

static FILE *unit2;

/* prototypes for internal procedures */

static void alarmf(int signo);
static void notifier(int signo);
long otpinit(int np, struct sockaddr_in *addrn, int alen);
long etmsec(struct timeval *ptime);
long sbtusec(struct timeval *pstime, struct timeval *crtime);
int ioresult(int *pblock);
int checkesc(long stk);
void sndmsgf(MPI_Request request, int dest);
void rcvmsgf(MPI_Request request, int source);
static int imax(int val1, int val2);
static int imin(int val1, int val2);
static float flmax(float val1, float val2);
static float flmin(float val1, float val2);
static double dmax(double val1, double val2);
static double dmin(double val1, double val2);
static void iredux(int *recvbuf, int *sendbuf, int offset, int count, MPI_Op op);
static void fredux(float *recvbuf, float *sendbuf, int offset, int count, MPI_Op op);  
static void dredux(double *recvbuf, double *sendbuf, int offset, int count, MPI_Op op);
void writerrs(char *source, int ierror);
void rwstat(int request, FILE *unit);
void wqueue(FILE *unit);
void messwin(int nvp);
void logmess(int idp, int lstat, int lsize, int mticks, int tag);
void showmess(int idp, int istat, int istyle);
void showdism(int ibin, int nbin, int mbin, int lmax, int istyle);
void shospeed(float atime, float ctime, float arate, float crate);
void Logname(char *name);
void Set_Mon(int monval);
int Get_Mon();
void delmess();

/* function definitions */

int MPI_Init(int *argc, char ***argv) {
/* initialize the MPI execution environment
   input: argc, argv, output: none
local data                                  */
   int ierror, nerr, nv, i, alen;
   int portnum = 0, pshift = 0, pnumid[3], epnum[8];
   long oss, response;
   char *cnerr = 0;
   struct utsname uinfo;
   struct hostent *info;
   struct in_addr address, oldadd;
   char ename[36], myself[36], location[18];
   struct sockaddr_in addrn, addrb, addrl;
   struct timeval ptime;
   struct sigaction act;
   MPI_Status stat;
   FILE *unit3;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   epref = array of socket endpoints for each participating node
   ioc = array of context pointers for notifier function
   ioc[i][0] = endpoint reference for notifier for endpoint i
   ioc[i][1] = processor id for listener for endpoint i
   ioc[i][2] = handle for current receive from endpoint i
   ioc[i][3] = handle for current send to endpoint i
   stime = first time stamp if MPI_Init successful
   maxfds = maximum descriptor value
   fdsset = fd_set reprsenting all endpoints
   sset = signal mask
   mapcomm = communicator map
   mapcomm[i][0:nproc-1] = actual proc id for given rank in communicator i
   mapcomm[i][nproc:MAXS-1] = MPI_UNDEFINED
   mapcomm[i][MAXS] = number of processes in comm i
   mapcomm[i][MAXS+1] = rank for this node in comm i
   mapcomm[i][MAXS+2] = ndims = number of dimensions in topology in comm i
   mapcomm[i][iMAXS+3:MAXS+2+ndims] = size of dimension in comm i,
   negative if non-periodic
   mapcomm[i][MAXS+3+ndims:MAXS+2+MAXD] = 0                             */  
/* internal common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages
   curreq = request record for transmission parameters, see rwstat
   header = message envelope
   hdata[i][0] = communicator, hdata[i][1] = tag, hdata[i][2] = datatype
   hdata[i][3] = length of data (in bytes) for message handle i
   rwrec = read/write record for asynchronous messages, see rwstat
   mqueue = message request queue
   mqueue[i][0] = end of message queue for receives from endpoint i
   mqueue[i][1] = end of message queue for sends to endpoint i          */
/* internal common block for adsp
   epref0 = endpoint reference used by tcp and adsp providers */
/* Initialize common block data */
   for (i = 0; i < MAXS+2; i++) {
      epref[i] = 0;
      for (nv = 0; nv < 2; nv++)
         mqueue[i][nv] = 0;
   }
   for (i = 0; i < MAXC; i++) {
      for (nv = 0; nv < MAXS+MAXD+3; nv++)
         mapcomm[i][nv] = 0;
   }
   for (i = 0; i < MAXM; i++) {
      for (nv = 0; nv < 5; nv++)
         curreq[i][nv] = 0;
   }
/* Set MPI_COMM_WORLD and MPI_COMM_SELF mapping */
   for (i = 0; i < MAXS; i++) {
      mapcomm[0][i] = i;
      mapcomm[1][i] = MPI_UNDEFINED;
   }
/* Zero out descriptor array */
   FD_ZERO(&fdsset);
/* Open error file */
   unit2 = fopen("MPIerrs","w");
   if (monitor==2)
      fprintf(unit2,"MPI_Init started\n");
/* Obtain the current time stamp */
   oss = gettimeofday(&stime,NULL);
   if (oss < 0) {
      oss = errno;
      fprintf(unit2,"Gettimeofday Error = %ld, %s\n",oss,strerror(oss));
   }
/* Obtain information about the Internet environment */
   oss = uname(&uinfo);
   if (oss < 0) {
      ierror = errno;
      fprintf(unit2,"Uname Error = %d, %s\n",ierror,strerror(ierror));
      return ierror;
   }
   strcpy(myself,uinfo.nodename);
/* debug */
   if (monitor==2) {
      fprintf(unit2,"local host name=%s\n",myself);
   }
   info = gethostbyname(uinfo.nodename);
   if (info==NULL) {
      oss = h_errno;
      fprintf(unit2,"Gethostbyname Error = %ld for %s\n",oss,myself);
      fprintf(unit2,"%s, Trying INADDR_ANY\n",hstrerror(oss));
      address.s_addr = INADDR_ANY;
   }
   else
/* debug */
      if (monitor==2) {
         i = 1;
         cnerr = ((info->h_addr_list)[i]);
         while (cnerr) {
            address.s_addr = *(in_addr_t *) cnerr;
            cnerr = inet_ntoa(address);
            strcpy(myself,cnerr);
            fprintf(unit2,"other local addresses=%d,%s\n",i,myself);
            i =+ 1;
            cnerr = ((info->h_addr_list)[i]);
         }
      }
      address.s_addr = *(in_addr_t *) ((info->h_addr_list)[0]);
/* debug */
   if (monitor==2) {
      cnerr = inet_ntoa(address);
      strcpy(myself,cnerr);
      fprintf(unit2,"local address=%s\n",myself);
   }
   oldadd = address;
/*
   Everyone opens a port
*/
/* Open file containing portnum (and possibly participating nodes)
   first line in nodelist file on all nodes contains common portnum
   if the file is missing or empty, a default number of 5013 is used */
   unit3 = fopen("nodelist_ip","r");
   if (!unit3)
      unit3 = fopen("nodelist","r");
   if (unit3) {
      cnerr = fgets(location,11,unit3);
      if (!cnerr)
         strcpy(location,"5013");
/* Replace trailing newlines with nulls */
      cnerr = strchr(location,'\n');
      if (cnerr)
         cnerr[0] = '\0';
      if (location[0]=='\0')
         strcpy(location,"5013");
   }
   else
      strcpy(location,"5013");
/* Convert string to integer */
   nerr = sscanf(location,"%d",&portnum);
   if ((!nerr) || (portnum < 5000) || (portnum > 49152))
      portnum = 5013;
/* Construct sockaddr_in of listener */
   addrn.sin_family = AF_INET;
   addrn.sin_port = portnum;
   addrn.sin_addr = address;
   alen = sizeof(addrn);
   for (i = 0; i < 8; i++)
      addrn.sin_zero[i] = '\0';
   strcpy(ename,myself);
/* Save copy of portnum */
   pnumid[0] = portnum;
   nproc = 0;
/* Initialize synchronous listener endpoint provider */
   oss = otpinit(MAXS+1,&addrn,alen);
   if (oss) {
      ierror = oss;
      nerr = MPI_Finalize();
      return ierror;
   }
/* Set struct sockaddr_in for address which is returned */
   nerr = alen;
   oss = getsockname(epref[MAXS+1],(struct sockaddr *)&addrb,&nerr);
   if (oss < 0) {
      ierror = errno;
      fprintf(unit2,"Getsockname Error = %d, %s\n",ierror,strerror(ierror));
      nerr = MPI_Finalize();
      return ierror;
   }
/* Pass processor id to listener notifier */
   ioc[MAXS+1][1] = nproc + 1;
/* debug */
   if (monitor==2)
      fprintf(unit2,"listener=%s,port=%d,socket=%d\n",myself,portnum,
              epref[MAXS+1]);
/*
   Determine if node is master (idproc=0) or slave (idproc>0).
   on the master node, the second and subsequent lines of nodelist file
   contain IP addresses of the nodes participating, in dotted-decimal
   format (for example, "12.13.14.15").
   if this list of nodes is missing, then the node is a slave.
   every node also makes a connection to itself.
*/
   if (unit3) {
      cnerr = fgets(location,17,unit3);
   }
/* Must be slave */
   if (!unit3 || !cnerr)
      idproc = 1;
   else {
/* Replace trailing newlines with nulls */
      cnerr = strchr(location,'\n');
      if (cnerr)
         cnerr[0] = '\0';
/* Must be slave */
      if (location[0]=='\0')
         idproc = 1;
/* Must be master */
      else
         idproc = 0;
   }
/* 
   * * * begin main iteration loop * * *

   Prepare to accept connection
*/
/* Establish connection end */
/* Listen for an incoming connection request */
L10: oss = listen(epref[MAXS+1],1);
   if (oss < 0) {
      ierror = errno;
      fprintf(unit2,"Listen Error = %d, %s\n",ierror,strerror(ierror));
      nerr = MPI_Finalize();
      return ierror;
   }
/* Prepare alarm */
   act.sa_handler = alarmf;
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   oss = sigaction(SIGALRM,&act,NULL);
   if (oss < 0) {
      ierror = errno;
      fprintf(unit2,"Sigaction SIGALRM Error = %d, %s\n",ierror,
              strerror(ierror));
      nerr = MPI_Finalize();
      return ierror;
   }
/* For connections to oneself, jump to connect */
   if (idproc==nproc) {
      ioc[MAXS+1][1] = MAXS+2;
      goto L70;
   }
/* Set alarm */
   alarm(60);
/* Accept an incoming connection request */
   nerr = alen;
   epref[nproc] = accept(epref[MAXS+1],(struct sockaddr *)&addrl,&nerr);
/* Clear alarm */
   alarm(0);
   if (epref[nproc] < 0) {
      ierror = errno;
      if (ierror==EINTR)
         ierror = ETIMEDOUT;
      fprintf(unit2,"Accept Error = %d, %s\n",ierror,strerror(ierror));
      return ierror;
   }
/* debug */
   if (monitor==2) {
      cnerr = inet_ntoa(addrl.sin_addr);
      fprintf(unit2,"socket=%d accepted connection with=%s,%d\n",
              epref[nproc],cnerr,addrl.sin_port);
   }
   ioc[MAXS+1][1] = 0;
/* Receive portnum for verification and processor id from remote node */
   nproc += 1;
   mapcomm[0][MAXS] = nproc;
/* Reset iocompletion flag to notifier */
   ioc[MAXS+1][1] = nproc + 1;
   ierror = MPI_Recv(epnum,3,MPI_INT,nproc-1,3,0,&stat);
/* Extract processor id on first connection */
   if (nproc==1) {
      idproc = epnum[1];
      pshift = portnum - epnum[2];
      portnum = epnum[2];
      mapcomm[0][MAXS+1] = idproc;
   }
/* Check if remote portnum agrees with local portnum */
   nerr = pnumid[0] - epnum[0];
/* Send reject flag to remote node */
   ierror = MPI_Send(&nerr,1,MPI_INT,nproc-1,4,0);
/* Reject if portnums disagree */
   if (nerr) {
      fprintf(unit2,"Session rejected, idproc = %d\n",idproc);
      fprintf(unit2,"Portnums do not agree\n");
      ierror = 5;
      fprintf(unit2,"remote portnum = %d\n",epnum[0]);
      nerr = MPI_Finalize();
      return ierror;
   }
/* Check for processor number overflow */
   if (nproc > MAXS) {
      fprintf(unit2,"processor number overflow, nproc = %d\n",nproc);
      ierror = 6;
      nerr = MPI_Finalize();
      return ierror;
   }
/* debug */
   if (monitor==2)
      fprintf(unit2,"connection accepted with idproc=%d\n",nproc-1);
/* Accept more connections */
   if (idproc >= nproc)
      goto L10;
/*
   Master prepares to start connection
*/
/* Find internet address of requested node */
/* Convert a character string into an address */
L40: oss = inet_aton(location,&address);
   if (!oss) {
      fprintf(unit2,"inet_aton failed, location = %s\n",location);
      fprintf(unit2,"Invalid nodelist file possibly being used\n");
      ierror = oss;
      nerr = MPI_Finalize();
      return ierror;
   }
/* Second cpu on a node uses incremented port number */
   if (address.s_addr==oldadd.s_addr)
      pshift = pshift + 1;
   else {
      pshift = 0;
      oldadd = address;
   }
   pnumid[0] = portnum + pshift;
   addrn.sin_family = AF_INET;
   addrn.sin_port = pnumid[0];
/* addrn.sin_addr.s_addr = INADDR_ANY; */
   addrn.sin_addr = address;
   for (i = 0; i < 8; i++)
      addrn.sin_zero[i] = '\0';
/* Convert an address into a character string */
   cnerr = inet_ntoa(address);
   strcpy(ename,cnerr);
/* Establish connection end */
/* Create a new socket for this endpoint */
L70: addrb.sin_port = 0;
/* Initialize synchronous endpoint provider */
   oss = otpinit(nproc,&addrb,alen);
   if (oss) {
      ierror = oss;
      nerr = MPI_Finalize();
      return ierror;
   }
/* debug */
   if (monitor==2)
      fprintf(unit2,"socket=%d requesting connection with=%s,%d\n",
              epref[nproc],ename,pnumid[0]);
/* Pause to let OS get some time */
   if (checkesc(1)) {
      ierror = -9;
      writerrs("MPI_Init: ",ierror);
      return ierror;
   }
/* Obtain the current time stamp */
   oss = gettimeofday(&ptime,NULL);
/* Set alarm */
   alarm(60);
/* Request a connection to a remote peer */
   oss = connect(epref[nproc],(struct sockaddr *)&addrn,alen);
/* Clear alarm */
   alarm(0);
   if (oss < 0) {
      ierror = errno;
      if (ierror==EINTR)
         ierror = ETIMEDOUT;
      fprintf(unit2,"Connect Error = %d, %s\n",ierror,strerror(ierror));
      return ierror;
   }
/* Accept an incoming connection request to oneself */
   if (idproc==nproc) {
      nerr = alen;
      epref[MAXS] = accept(epref[MAXS+1],(struct sockaddr *)&addrl,&nerr);
      if (epref[MAXS] < 0) {
         ierror = errno;
         fprintf(unit2,"Accept Error = %d, %s\n",ierror,strerror(ierror));
         return ierror;
      }
/* debug */
      if (monitor==2) {
         cnerr = inet_ntoa(addrl.sin_addr);
         fprintf(unit2,"self socket=%d accepted connection with=%s,%d\n",
                 epref[MAXS],cnerr,addrl.sin_port);
      }
   }
   ioc[nproc][1] = 0;
/* debug */
   if (monitor==2) {
      nerr = alen;
      oss = getpeername(epref[nproc],(struct sockaddr *)&addrl,&nerr);
      if (oss < 0) {
         oss = errno;
         fprintf(unit2,"Getpeername Error = %ld, %s\n",oss,strerror(oss));
         addrl.sin_addr.s_addr = 0;
      }
      cnerr = inet_ntoa(addrl.sin_addr);
      fprintf(unit2,"tentative connection started with=%s,%d\n",cnerr,
              addrl.sin_port);
   }
/* Send portnum for verification and processor id to remote node */
   nerr = nproc;
   nproc += 1;
   mapcomm[0][MAXS] = nproc;
   if (nerr > idproc) {
/* Set processor id */
      pnumid[1] = nerr;
      pnumid[2] = portnum;
      ierror = MPI_Send(pnumid,3,MPI_INT,nproc-1,3,0);
/* Read and check reject flag */
      ierror = MPI_Recv(&nerr,1,MPI_INT,nproc-1,4,0,&stat);
      if (nerr) {
         fprintf(unit2,"Connection rejected, reject info = %d\n",nerr);
         fprintf(unit2,"Portnums do not agree, idproc = %d\n",nproc-1);
         ierror = 12;
         nerr = MPI_Finalize();
         return ierror;
      }
   }
/* Check for processor number overflow */
   if (nproc > MAXS) {
      fprintf(unit2,"processor number overflow, nproc = %d\n",nproc);
      ierror = 6;
      nerr = MPI_Finalize();
      return ierror;
   }
/* debug */
   if (monitor==2)
      fprintf(unit2,"connection confirmed with idproc=%d\n",nproc-1);
/* Pass current location to next node */
   if (nproc > (idproc+2))
      nerr = MPI_Send(location,4,MPI_INT,idproc+1,1,0);
/* Read location of next node from file */
   if (idproc==0) {
      if ((nproc >= 2) || (!strcmp(location,"self")) || (!strcmp(location,myself))) {
         if (!unit3)
            goto L90;
         cnerr = fgets(location,17,unit3);
         if (!cnerr)
            goto L90;
/* Replace trailing newlines with nulls */
         cnerr = strchr(location,'\n');
         if (cnerr)
            cnerr[0] = '\0';
         if (location[0]=='\0')
            goto L90;
      }
   }
/* Receive location of next node from another processor */
   else {
      nerr = MPI_Recv(location,4,MPI_INT,idproc-1,1,0,&stat);
/* End of file marker received */
      if (stat.len==0)
         goto L90;
   }
/* Start another connection */
   goto L40;
/* 
   * * * end main iteration loop * * *
*/
/* All expected nodes activated */
L90: nv = nproc - 1;
/* debug */
   if (monitor==2)
      fprintf(unit2,"all nodes activated, idproc, nproc=%d,%d\n",
              idproc,nproc);
/* Send null record to next processor */
   if (idproc < nv)
      nerr = MPI_Send(location,0,MPI_INT,idproc+1,1,0);
   if (unit3)
      fclose(unit3);
/* Check number of processors */
   if (idproc==nv) {
      for (i = 1; i <= nv; i++) {
         nerr = MPI_Send(&nproc,1,MPI_INT,nv-i,2,0);
      }
   }
   else {
      nerr = MPI_Recv(&response,1,MPI_INT,nv,2,0,&stat);
/* Local processor does not agree with last processor on total number */
      if (response != nproc) {
         fprintf(unit2,"processor number error, local/remote nproc = %d,%ld\n",
                 nproc,response);
         ierror = 7;
         nerr = MPI_Finalize();
         return ierror;
      }
   }
/* Clear unused MPI_COMM_WORLD mapping */
   for (i = nproc; i < MAXS; i++) {
      mapcomm[0][i] = MPI_UNDEFINED;
   }
   mapcomm[0][MAXS+2] = 0;
/* Set MPI_COMM_SELF */
   mapcomm[1][0] = idproc;
   mapcomm[1][MAXS] = 1;
   mapcomm[1][MAXS+1] = 0;
   mapcomm[1][MAXS+2] = 0;
/* Set descriptor array for notifier */
   maxfds = -1;
   for (i = 0; i < MAXS+1; i++) {
      if (epref[i]) {
         maxfds = imax(epref[i],maxfds);
         FD_SET(epref[i],&fdsset);
      }
   }
   maxfds = maxfds + 1;
/* Prepare signal mask */
   oss = sigemptyset(&sset);
   oss = sigaddset(&sset,SIGIO);
/* Prepare signal action */
   act.sa_handler = notifier;
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   oss = sigaction(SIGIO,&act,NULL);
   if (oss < 0) {
      ierror = errno;
      fprintf(unit2,"Sigaction SIGIO Error = %d, %s\n",ierror,
              strerror(ierror));
      nerr = MPI_Finalize();
      return ierror;
   }
/* Set providers to asynchronous mode */
   nv = 1;
   nerr = getpid();
   for (i = 0; i < MAXS+1; i++) {
      if (epref[i]) {
         oss = fcntl(epref[i],F_SETOWN,nerr);
         if (oss < 0) {
            oss = errno;
            fprintf(unit2,"F_SETOWN fcntl Error, oss = %ld, %s\n",oss,
                    strerror(oss));
            return oss;
         }
         oss = ioctl(epref[i],FIOASYNC,&nv);
         if (oss < 0) {
            oss = errno;
            fprintf(unit2,"FIOASYNC ioctl Error, oss = %ld, %s\n",oss,
                    strerror(oss));
            return oss;
         }
         oss = ioctl(epref[i],FIONBIO,&nv);
         if (oss < 0) {
            oss = errno;
            fprintf(unit2,"FIONBIO ioctl Error, oss = %ld, %s\n",oss,
                    strerror(oss));
            return oss;
         }
      }
   }
/* Create window for showing MPI message status */
   if (monitor > 0) {
      messwin(nproc);
      checkesc(1);
      if (monitor==2)
         fprintf(unit2,"MPI_Init complete\n\n");
   }
/* Set error code to success */
   ierror = 0;
   return ierror;
}

static void alarmf(int signo) {
/* callback function to handle connect/accept alarms */
   return;
}

static void notifier(int signo) {
/* notifier function for asynchronous and completion events
local data */
   int i, j, l, n, m;
   long num, oss;
   struct timeval timeout = {0,0};
   fd_set lfdsset;
/* internal mpi common block
   epref = array of endpoint references for each participating node
   maxfds = maximum descriptor value
   fdsset = fd_set reprsenting all endpoints                         */
/* internal common block for non-blocking messages
   rwrec = read/write record for asynchronous messages
   trash = trash bin for unwanted data
   mqueue = message request queue                                    */
/* Check if any descriptors are ready for reading */
   lfdsset = fdsset;
   num = select(maxfds,&lfdsset,NULL,NULL,&timeout);
/* Check for error in select */
   if (num <= 0) {
      num = 0;
      goto L20;
   }
   l = 0;
/* Find which descriptor has a read event */
   for (i = 0; i < nproc; i++) {
      if (FD_ISSET(epref[i],&lfdsset)) {
/* Get reference to endpoint */
         n = ioc[i][0];
/* Read pointer to current read record */
         m = ioc[i][2] - 1;
/* Read data sent from a remote peer */
         if ((m >= 0) && (m < MAXM)) {
/* Obtain the current time stamp */
            oss = gettimeofday(&rwrec[m].ts[1],NULL);
L10:        oss = recv(rwrec[m].ref,rwrec[m].buf,rwrec[m].nbytes,
                       rwrec[m].flags);
            if (oss < 0)
               oss = errno - 1024;
/* Process data which arrived */
            if (oss > 0) {
/* Clear non-fatal error code */
               rwrec[m].nfatal = 0;
/* Set actual length received */
               rwrec[m].len += oss;
/* Check if all the data has arrived */
               if (rwrec[m].len < header[m].hdata[3]) {
/* Incomplete message */
                  if (rwrec[m].nbytes > oss) {
/* Readjust buffer pointer */
                     rwrec[m].buf = (void *)(((unsigned char *)rwrec[m].buf) + oss);
                     rwrec[m].nbytes -= oss;
                     if (rwrec[m].len >= 0)
                        rwrec[m].ioflag += 1;
                  }
/* Header is received, readjust parameters to receive data */
                  else if (rwrec[m].ioflag==1) {
                     rwrec[m].buf = rwrec[m].sbuf;
                     rwrec[m].nbytes = imin(header[m].hdata[3],rwrec[m].sln);
                     rwrec[m].ioflag = 2;
                  }
/* Data is received and buffer is full */
                  else {
                     rwrec[m].buf = &trash;
                     rwrec[m].nbytes = 1024;
                  }
                  goto L10;
               }
/* Message complete */
               else {
/* Obtain the current time stamp */
                  oss = gettimeofday(&rwrec[m].ts[1],NULL);
/* Set iocompletion flag */
                  rwrec[m].ioflag = 0;
/* Get next message if messages are queued */
                  if (rwrec[m].nextm > 0) {
                     m = rwrec[m].nextm;
                     if (m==mqueue[n][0])
                        mqueue[n][0] = 0;
                     ioc[i][2] = m;
                     m -= 1;
                     goto L10;
                  }
/* Clear pointer to current send record */
                  else
                     ioc[i][2] = 0;
               }
            }
/* Check for errors */
            else {
/* Check for EOF */
               if (oss==0)
                  oss = -1;
/* Quit if no data is available */
               if ((oss+1024) != EWOULDBLOCK) {
/* Set iocompletion flag to error */
                  rwrec[m].ioflag = oss;
                  ioc[i][2] = 0;
               }
            }
         }
/* All receive events processed */
         l += 1;
         if (l==num)
            goto L20;
      }
   }
/* Check if any descriptors are ready for writing */
L20: lfdsset = fdsset;
   oss = select(maxfds,NULL,&lfdsset,NULL,&timeout);
   if (oss > 0)
      num += oss;
/* Check for error in select */
   if (oss <= 0)
      return;
/* Find which descriptor has a write event */
   for (j = 0; j < nproc+1; j++) {
      i = j;
      if (j==nproc)
         i = MAXS;
      if (FD_ISSET(epref[i],&lfdsset)) {
/* Get reference to endpoint */
         n = ioc[i][0];
/* Read pointer to current send record */
         m = ioc[i][3] - 1;
/* Send data to a remote peer */
         if ((m >= 0) && (m < MAXM)) {
/* Obtain the current time stamp */
            oss = gettimeofday(&rwrec[m].ts[1],NULL);
L30:        if (rwrec[m].ioflag==1)
               oss = writev(rwrec[m].ref,rwrec[m].buf,rwrec[m].nbytes);
            else
               oss = send(rwrec[m].ref,rwrec[m].buf,rwrec[m].nbytes,
                      rwrec[m].flags);
            if (oss < 0)
               oss = errno - 1024;
/* Process data which has been sent */
            if (oss > 0) {
/* Clear non-fatal error code */
               rwrec[m].nfatal = 0;
/* Set actual length sent */
               rwrec[m].len += oss;
/* Check for incomplete header */
               if (rwrec[m].len < 0) {
                  header[m].iov[0].iov_base = 
                     (void *)(((unsigned char *)header[m].iov[0].iov_base) + oss);
                  header[m].iov[0].iov_len -= oss;
                  goto L30;
               }
/* Check for incomplete data */
               else if (rwrec[m].sln > rwrec[m].len) {
/* Header is sent, readjust parameters to send data */
                  if (rwrec[m].ioflag==1) {
                     rwrec[m].buf = rwrec[m].sbuf;
                     rwrec[m].nbytes = rwrec[m].sln;
                     oss -= header[m].iov[0].iov_len;
                  }
/* Readjust buffer pointer */
                  rwrec[m].buf = (void *)(((unsigned char *)rwrec[m].buf) + oss);
                  rwrec[m].nbytes -= oss;
                  rwrec[m].ioflag += 1;
                  goto L30;
               }
/* Data is sent */
               else {
/* Obtain the current time stamp */
                  oss = gettimeofday(&rwrec[m].ts[1],NULL);
/* Set iocompletion flag */
                  rwrec[m].ioflag = 0;
/* Get next message if messages are queued */
                  if (rwrec[m].nextm > 0) {
                     m = rwrec[m].nextm;
                     if (m==mqueue[n][1])
                        mqueue[n][1] = 0;
                     ioc[i][3] = m;
                     m -= 1;
                     goto L30;
                  }
/* Clear pointer to current send record */
                  else
                     ioc[i][3] = 0;
               }
            }
/* Check for errors */
            else {
/* Check for EOF */
               if (oss==0)
                  oss = -1;
/* Quit if no data can be sent */
               if ((oss+1024) != EWOULDBLOCK) {
/* Set iocompletion flag to error */
                  rwrec[m].ioflag = oss;
                  ioc[i][3] = 0;
               }
            }
         }
/* All send events processed */
         l += 1;
         if (l==num)
            return;
      }
   }
   return;
}

long otpinit(int np, struct sockaddr_in *addrn, int alen) {
/* this subroutine initializes a Socket provider for
   index np, using address specified in addrn.
   provider is left in asynchronous, blocking mode
   np = index to endpoint reference array
   addrn = address to which endpoint is to be bound
   alen = length of addrn structure
   returns error indicator
   input: np, addrn, alen
local data                                                       */
   long oss, value;
/* internal mpi common block
   epref = array of endpoint references for each participating node
   ioc = array of context pointers for notifier function                  */
   oss = 0;
/* Create a synchronous endpoint provider */
   epref[np] = socket(AF_INET,SOCK_STREAM,0);
   if (epref[np] < 0) {
      oss = errno;
      fprintf(unit2,"Socket Error = %ld, %s\n",oss,strerror(oss));
      return oss;
   }
/* Set TCP_NODELAY option */
   value = 1;
   oss = setsockopt(epref[np],IPPROTO_TCP,TCP_NODELAY,&value,4);
   if (oss < 0) {
      oss = errno;
      fprintf(unit2,"TCP_NODELAY setsockopt Error %ld, %s\n",oss,strerror(oss));
      return oss;
   }
/* Set SO_RCVBUF option */
   value = 65536;
/* oss = setsockopt(epref[np],SOL_SOCKET,SO_RCVBUF,&value,4); */
   if (oss < 0) {
      oss = errno;
      fprintf(unit2,"SO_RCVBUF setsockopt Error %ld, %s\n",oss,strerror(oss));
      return oss;
   }
/* Set SO_SNDBUF option */
   value = 65536;
/* oss = setsockopt(epref[np],SOL_SOCKET,SO_SNDBUF,&value,4); */
   if (oss < 0) {
      oss = errno;
      fprintf(unit2,"SO_SNDBUF setsockopt Error %ld, %s\n",oss,strerror(oss));
      return oss;
   }
/* Set SO_DEBUG option for trpt program */
   value = 1;
/* oss = setsockopt(epref[np],SOL_SOCKET,SO_DEBUG,&value,4); */
   if (oss < 0) {
      oss = errno;
      fprintf(unit2,"SO_DEBUG setsockopt Error %ld, %s\n",oss,strerror(oss));
      return oss;
   }
/* Assign an address to an endpoint */
   oss = bind(epref[np],(struct sockaddr *)addrn,alen);
   if (oss < 0) {
      oss = errno;
      fprintf(unit2,"Bind Error = %ld, %s\n",oss,strerror(oss));
      return oss;
   }
/* Pass processor epref index and iocompletion to notifier */
   ioc[np][0] = np;
   ioc[np][1] = 1;
/* Clear read and send record pointers */
   ioc[np][2] = 0;
   ioc[np][3] = 0;
   return oss;
}

long etmsec(struct timeval *ptime) {
/* returns elapsed time since in milliseconds since ptime */
   const long tscale=1000;
   long oss, msec;
   struct timeval pstime, crtime;
/* calculate time elapsed in microseconds */
   oss = gettimeofday(&crtime,NULL);
   if (oss < 0)
      return -1;
   pstime = *ptime;
   msec = (crtime.tv_usec - pstime.tv_usec)/tscale;
   msec = msec + (crtime.tv_sec - pstime.tv_sec)*tscale;
   return msec;
}

long sbtusec(struct timeval *pstime, struct timeval *crtime) {
/* returns difference time in microseconds between crtime and pstime */
   const long tscale=1000000;
   long usec;
   usec = (crtime->tv_usec - pstime->tv_usec);
   usec = usec + (crtime->tv_sec - pstime->tv_sec)*tscale;
   return usec;
}

int ioresult(int *pblock) {
/* this function returns ioResult for asynchronous procedures
   input: pblock                                              */
   return pblock[1];
}

int checkesc(long stk) {
/* this procedure allows user to abort a procedure by checking for
   escape, Cmd-. or Ctrl-C keystrokes.  Calling EventAvail also
   permits an idle procedure to time-share and checks for Quit Events
   returns true if an escape event occurred.
   recent keyboard events not processed are not removed from event queue
   stk = maximum number of sleepTicks (sixtieths of a second) that
   application agrees to relinquish the processor if no events are 
   pending for it.
   input: stk                                                */
   return 0;
}

int MPI_Finalize(void) {
/* terminate MPI execution environment
local data                             */
   int ierror, i;
   long oss, state;
   struct sigaction act;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   epref = array of endpoint references for each participating node
   mapcomm = communicator map                                            */
/* internal common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages
   curreq = request record for transmission parameters                   */
   ierror = 0;
/* MPI already finalized */
   if (nproc < 0) {
      ierror = 1;
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Finalize started\n");
/* Close providers */
   for (i = 0; i <= MAXS+1; i++) {
      if (epref[i]) {
/* Close a provider of any type */
         oss = close(epref[i]);
         if (oss < 0) {
            oss = errno;
            fprintf(unit2,"Close Error, i, oss = %d, %ld, %s\n",i,oss,
                    strerror(oss));
            ierror = oss;
         }
      }
   }
/* Revert signal action to default */
   act.sa_handler = SIG_DFL;
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   oss = sigaction(SIGIO,&act,NULL);
/* Close window for showing MPI message status */
   if (monitor > 0) {
      logmess(0,0,-1,0,0);
      delmess();
   }
/* Nullify nproc */
   nproc = -1;
/* Nullify communicator mappings */
   for (i = 0; i < MAXC; i++) {
      mapcomm[i][MAXS] = 0;
   }
/* Nullify endpoint references */
   for (i = 0; i <= MAXS+1; i++) {
      epref[i] = 0;
   }
/* check if any messages remain outstanding */
   state = 0;
   for (i = 0; i < MAXM; i++) {
      if (curreq[i][0])
         state += 1;
   }
   if (state > 0) {
      fprintf(unit2,"%ld message(s) never cleared\n",state);
      for (i = 0; i < MAXM; i++) {
         if (curreq[i][0]) {
            if (curreq[i][0]==(-1))
              fprintf(unit2," transmission mode = send\n");
            else if (curreq[i][0]==1)
               fprintf(unit2," transmission mode = receive\n");
            fprintf(unit2," destination/source = %d\n",curreq[i][1]);
            fprintf(unit2," communicator = %d\n",curreq[i][2]);
            fprintf(unit2," tag = %d\n",curreq[i][3]);
            fprintf(unit2," datatype = %d\n",curreq[i][4]);
            fprintf(unit2,"\n");
         }
      }
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Finalize complete\n");
/* Delete file if empty */
   if (!fseek(unit2,0,SEEK_END)) {
      i = ftell(unit2);
      fclose(unit2);
      if (!i)
         remove("MPIerrs");
   }
   return ierror;
}

int MPI_Send(void* buf, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm) {
/* blocking standard mode send
   buf = initial address of send buffer
   count = number of entries to send
   datatype = datatype of each entry
   dest = rank of destination
   tag = message tag
   comm = communicator
   input: buf, count, datatype, dest, tag, comm
local data                                                       */
   int ierror;
   MPI_Request request;
   MPI_Status status;
   ierror = MPI_Isend(buf,count,datatype,dest,tag,comm,&request);
   ierror = MPI_Wait(&request,&status);
   return ierror;
}
 
int MPI_Recv(void* buf, int count, MPI_Datatype datatype, int source,
             int tag, MPI_Comm comm, MPI_Status *status) {
/* blocking receive
   buf = initial address of receive buffer
   count = maximum number of entries to receive
   datatype = datatype of each entry
   source = rank of source
   tag = message tag
   comm = communicator
   status = return status
   input: count, datatype, source, tag, comm
   output: buf, status
local data                                                       */
   int ierror;
   MPI_Request request;
   ierror = MPI_Irecv(buf,count,datatype,source,tag,comm,&request);
   ierror = MPI_Wait(&request,status);
   return ierror;
}

int MPI_Isend(void* buf, int count, MPI_Datatype datatype, int dest,
              int tag, MPI_Comm comm, MPI_Request *request) {
/* start a non-blocking send
   buf = initial address of send buffer
   count = number of entries to send
   datatype = datatype of each entry
   dest = rank of destination
   tag = message tag
   comm = communicator
   request = request handle
   input: buf, count, datatype, dest, tag, comm
   output: request
local data                                                           */
   int ierror, np, longw, i;
   long response;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   epref = array of endpoint references for each participating node
   ioc = array of context pointers for notifier function
   sset = signal mask
   mapcomm = communicator map                                       */
/* internal common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages
   curreq = request record for transmission parameters
   header = message envelope
   rwrec = read/write record for asynchronous messages
   mqueue = message request queue                                     */
   ierror = 0;
   if (dest==MPI_PROC_NULL) {
      *request = MPI_REQUEST_NULL;
      return ierror;
   }
/* Find space for record */
   i = -1;
L10: i += 1;
   if (i >= MAXM) {
      fprintf(unit2,"too many sends waiting, dest, tag = %d,%d,\n",
              dest,tag);
      *request = MPI_REQUEST_NULL;
      ierror = 14;
      writerrs("MPI_Isend: ",ierror);
      return ierror;
   }
   else if (curreq[i][0])
      goto L10;
/* Check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* Invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* Invalid count */
   else if (count < 0)
      ierror = 3;
/* Invalid destination */
   else if ((dest < 0) || (dest >= nproc)) {
      fprintf(unit2,"destination = %d\n",dest);
      ierror = 4;
   }
/* Invalid tag */
   else if (tag < (-1))
      ierror = 6;
/* Communicator errors */
   else {
      longw = mapcomm[comm][MAXS];
      np = mapcomm[comm][dest];
/* Communicator not mapped */
      if ((longw <= 0) || (longw > nproc))
         ierror = 2;
/* Invalid destination */
      else if ((dest < 0) || (dest >= longw)) {
         fprintf(unit2,"destination = %d\n",dest);
         ierror = 4;
      }
/* Invalid mapping */
      else if ((np < 0) || (np >= nproc)) {
         fprintf(unit2,"Invalid mapping, destination, node = %d,%d\n",dest,np);
         ierror = 2;
      }
   }
/* Handle errors */
   if (ierror) {
      writerrs("MPI_Isend: ",ierror);
      return ierror;
   }
/* Create header */
/* Iovec structure for header */
   header[i].iov[0].iov_base = (void *)&header[i].hdata[0];
   header[i].iov[0].iov_len = 16;
/* Save communicator */
   header[i].hdata[0] = comm;
/* Save tag */
   header[i].hdata[1] = tag;
/* Save datatype */
   header[i].hdata[2] = datatype;
/* Set destination id for selfsends */
   if (idproc==np)
      np = MAXS;
/* Set endpoint reference pointer */
   rwrec[i].ref = epref[np];
/* Reset iocompletion flag */
   rwrec[i].ioflag = 1;
/* Set pointer to header */
   rwrec[i].buf = (void *)&header[i].iov;
/* Set buffer length for header */
   rwrec[i].nbytes = 2;
/* Find buffer length for data */
   if ((datatype==MPI_INT) || (datatype==MPI_FLOAT))
      longw = 4*count;
   else if (datatype==MPI_DOUBLE)
      longw = 8*count;
   else if (datatype==MPI_BYTE)
      longw = count;
   else if ((datatype==MPI_2INT) || (datatype==MPI_FLOAT_INT)) 
      longw = 8*count;
   else if (datatype==MPI_DOUBLE_INT)
      longw = 16*count;
/* Invalid datatype */
   else {
      ierror = 7;
      writerrs("MPI_Isend: ",ierror);
      return ierror;
   }
/* Set pointer to data buffer */
   rwrec[i].sbuf = buf;
/* Set buffer lengths for data */
   rwrec[i].sln = longw;
   rwrec[i].len = -header[i].iov[0].iov_len;
/* Clear more flag */
   rwrec[i].flags = 0;
/* Clear next message flag */
   rwrec[i].nextm = 0;
/* Clear non-fatal error code */
   rwrec[i].nfatal = 0;
/* OTData structure for data */;
   header[i].iov[1].iov_base = rwrec[i].sbuf;
   header[i].iov[1].iov_len = longw;
/* Save length */
   header[i].hdata[3] = longw;
/* Obtain the current time stamp */
   response = gettimeofday(&rwrec[i].ts[0],NULL);
/* Limit notifications that can be sent to notifier */
   sigprocmask(SIG_BLOCK,&sset,NULL);
/* Append this message to send queue if necessary */
   if (ioc[np][3] > 0) {
      if (mqueue[np][1] > 0)
         rwrec[mqueue[np][1]-1].nextm = i + 1;
      else
         rwrec[ioc[np][3]-1].nextm = i + 1;
      mqueue[np][1] = i + 1;
/* Obtain the current time stamp */
      response = gettimeofday(&rwrec[i].ts[1],NULL);
      goto L30;
   }
/* First send 4 word header, then data */
      sndmsgf(i,np);
      response = ioresult((int *)&rwrec[i]);
/* Set pointer to current send record */
      if (response > 0)
         ioc[np][3] = i + 1;
/* Set iocompletion flag to error */
      else if (response < 0)
         ierror = response + 1024;
/* Allow Open Transport to resume sending events */
L30: sigprocmask(SIG_UNBLOCK,&sset,NULL);
/* Handle read and write errors */
   if (ierror) {
/* Write out readwrite record */
      rwstat(i,unit2);
      wqueue(unit2);
      for (i = 0; i < MAXM; i++) {
         if (curreq[i][0] != 0)
            rwstat(i,unit2);
      }
      writerrs("MPI_Send: ",ierror);
   }
/* Find actual destination */
   if (np==MAXS)
      np = idproc;
/* Log MPI message state change and display status */
   if (monitor > 0) 
      logmess(np,1,rwrec[i].sln,0,tag);
/* Save transmission mode as send */
   curreq[i][0] = -1;
/* Save destination/source id */
   curreq[i][1] = np;
/* Save communicator */
   curreq[i][2] = comm;
/* Save tag */
   curreq[i][3] = tag;
/* Save datatype */
   curreq[i][4] = datatype;
/* Assign request handle */
   *request = i;
   return ierror;
}

int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request) {
/* begin a non-blocking receive
   buf = initial address of receive buffer
   count = maximum number of entries to receive
   datatype = datatype of each entry
   source = rank of source
   tag = message tag
   comm = communicator
   request = request handle
   input: count, datatype, source, tag, comm
   output: buf, request
local data                                                            */
   int ierror, np, longw, i;
   long response;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   epref = array of endpoint references for each participating node
   ioc = array of context pointers for notifier function
   sset = signal mask
   mapcomm = communicator map                                         */
/* internal common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages
   curreq = request record for transmission parameters
   header = message envelope
   rwrec = read/write record for asynchronous messages
   mqueue = message request queue                                     */
   ierror = 0;
   if (source==MPI_PROC_NULL) {
      *request = MPI_REQUEST_NULL;
      return ierror;
   }
/* Find space for record */
   i = -1;
L10: i += 1;
   if (i >= MAXM) {
      fprintf(unit2,"too many receives waiting, source, tag = %d,%d,\n",
              source,tag);
      *request = MPI_REQUEST_NULL;;
      ierror = 15;
      writerrs("MPI_Irecv: ",ierror);
      return ierror;
   }
   else if (curreq[i][0])
      goto L10;
/* Check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* Invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* Invalid count */
   else if (count < 0)
      ierror = 3;
/* Invalid source */
   else if ((source < 0) || (source >= nproc)) {
      if (source==MPI_ANY_SOURCE)
         fprintf(unit2,"MPI_ANY_SOURCE not supported\n");
      else
         fprintf(unit2,"source = %d\n",source);
      ierror = 5;
   }
/* Invalid tag */
   else if (tag < (-1))
      ierror = 6;
/* Communicator errors */
   else {
      longw = mapcomm[comm][MAXS];
      np = mapcomm[comm][source];
/* Communicator not mapped */
      if ((longw <= 0) || (longw > nproc))
         ierror = 2;
/* Invalid source */
      else if ((source < 0) || (source >= longw)) {
         fprintf(unit2,"source = %d\n",source);
         ierror = 4;
      }
/* Invalid mapping */
      else if ((np < 0) || (np >= nproc)) {
         fprintf(unit2,"Invalid mapping, source, node = %d,%d\n",source,np);
         ierror = 2;
      }
   }
/* Handle errors */
   if (ierror) {
      writerrs("MPI_Irecv: ",ierror);
      return ierror;
   }
/* Set header to undefined state */
   header[i].hdata[0] = -1;
   header[i].hdata[1] = -1;
   header[i].hdata[2] = -1;
/* Set endpoint reference pointer */
   rwrec[i].ref = epref[np];
/* Reset iocompletion flag for receive */
   rwrec[i].ioflag = 1;
/* Set pointer to header */
   rwrec[i].buf = (void *)&header[i].hdata[0];
/* Set buffer length for header */
   rwrec[i].nbytes = 16;
/* Set buffer length for data */
   if ((datatype==MPI_INT) || (datatype==MPI_FLOAT))
      longw = 4*count;
   else if (datatype==MPI_DOUBLE)
      longw = 8*count;
   else if (datatype==MPI_BYTE)
      longw = count;
   else if ((datatype==MPI_2INT) || (datatype==MPI_FLOAT_INT)) 
      longw = 8*count;
   else if (datatype==MPI_DOUBLE_INT)
      longw = 16*count;
/* Invalid datatype */
   else {
      ierror = 7;
      writerrs("MPI_Irecv: ",ierror);
      return ierror;
   }  
/* Set pointer to data buffer */
   rwrec[i].sbuf = buf;
/* Set buffer lengths */
   rwrec[i].sln = longw;
   rwrec[i].len = -rwrec[i].nbytes;
/* Clear more flag */
   rwrec[i].flags = 0;
/* Clear next message flag */
   rwrec[i].nextm = 0;
/* Clear non-fatal error code */
   rwrec[i].nfatal = 0;
/* Clear length */
   header[i].hdata[3] = 0;
/* Obtain the current time stamp */
   response = gettimeofday(&rwrec[i].ts[0],NULL);
/* Limit notifications that can be sent to notifier */
   sigprocmask(SIG_BLOCK,&sset,NULL);
/* Append this message to receive queue if necessary */
   if (ioc[np][2] > 0) {
      if (mqueue[np][0] > 0)
         rwrec[mqueue[np][0]-1].nextm = i + 1;
      else
         rwrec[ioc[np][2]-1].nextm = i + 1;
      mqueue[np][0] = i + 1;
/* Obtain the current time stamp */
      response = gettimeofday(&rwrec[i].ts[1],NULL);
      goto L40;
   }
/* First receive 4 word header, then data */
      rcvmsgf(i,np);
      response = ioresult((int *)&rwrec[i]);
/* Set pointer to current receive record */
      if (response > 0)
         ioc[np][2] = i + 1;
/* Set iocompletion flag to error */
      else if (response < 0)
         ierror = response + 1024;
/* Allow Open Transport to resume sending events */
L40: sigprocmask(SIG_UNBLOCK,&sset,NULL);
/* Handle read and write errors */
   if (ierror) {
/* Write out readwrite record */
      rwstat(i,unit2);
      wqueue(unit2);
      for (i = 0; i < MAXM; i++) {
         if (curreq[i][0] != 0)
            rwstat(i,unit2);
      }
      writerrs("MPI_Recv: ",ierror);
   }
/* Log MPI message state change and display status */
   if (monitor > 0) 
      logmess(np,2,rwrec[i].sln,0,tag);
/* Save transmission mode as receive */
   curreq[i][0] = 1;
/* Save destination/source id */
   curreq[i][1] = np;
/* Save communicator */
   curreq[i][2] = comm;
/* Save tag */
   curreq[i][3] = tag;
/* Save datatype */
   curreq[i][4] = datatype;
/* Assign request handle */
   *request = i;
   return ierror;
}

int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status) {
/* check to see if a nonblocking send or receive operation has completed
   request = request handle
   flag = true if operation completed
   status = status object
   input: request
   output: request, flag, status
local data                                                            */
   int ierror, i, dest, source, slen, tag, rlen, rtag, nerr;
/* int j; */
   float ts;
   MPI_Comm comm, rcomm;
   MPI_Datatype datatype, rdatat;
/* mstime = maximum time (msec) to wait for message to start arriving
   mptime = maximum time (msec) to wait for next part of message      */
   static int mstime = 300000, mptime = 10000;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   ioc = array of context pointers for notifier function
   sset = signal mask                                               */
/* internal common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages
   curreq = request record for transmission parameters
   header = message envelope
   rwrec = read/write record for asynchronous messages                   */
   ierror = 0;
/* Check for error conditions */
   i = *request;
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* Null request */
   else if (*request < 0) {
      *flag = 1;
      return 0;
   }
/* Invalid request handle */
   else if (i >= MAXM)
      ierror = 16;
   else if (curreq[i][0]==0)
      ierror = 16;
/* Handle errors */
   if (ierror) {
      status->error = ierror;
      writerrs("MPI_Test: ",ierror);
      return ierror;
   }
/* Set status to empty */
   status->source = -1;
   status->tag = -1;
   status->error = ierror;
   status->len = 0;
   status->type = 0;
   *flag = 0;
/* Check if data read or write has completed */
   if (ioresult((int *)&rwrec[i]) > 0) {
      if (checkesc(0)) {
         if (curreq[i][0] < 0)
            fprintf(unit2,"Send killed,dest,tag=%d,%d\n",
                    curreq[i][1],curreq[i][3]);
         else
            fprintf(unit2,"Receive killed,source,tag=%d,%d\n",
                    curreq[i][1],curreq[i][3]);
         ierror = -9;
         writerrs("MPI_Test: ",ierror);
         return ierror;
      }
/* Check Timeout */
      else {
/* Limit notifications that can be sent to notifier */
         sigprocmask(SIG_BLOCK,&sset,NULL);
/* Calculate time elapsed in milliseconds */
         nerr = etmsec(&rwrec[i].ts[1]);
/* Retry Send or Receive */
         if (nerr < mptime) {
/* Allow Open Transport to resume sending events */
            sigprocmask(SIG_UNBLOCK,&sset,NULL);
            return ierror;
         }
/* Check if message arrived during checkesc */
         nerr = ioresult((int *)&rwrec[i]);
         if (nerr <= 0) {
            fprintf(unit2,"Info: message arrived during checkesc\n");
            *flag = 1;
            goto L20;
         }
/* Try to determine cause of Timeout in Send */
         if (curreq[i][0] < 0) {
            dest = curreq[i][1];
            ts = .001*(float) etmsec(&rwrec[i].ts[0]);
            fprintf(unit2,"Send Timeout, %f sec, Retrying...\n",ts);
/* Debug information */
            fprintf(unit2,"destination=%d,size=%d,tag=%d\n",
                       dest,rwrec[i].sln,curreq[i][3]);
            if (idproc==dest)
               dest = MAXS;
/* Attempt to send next block of data */
            sndmsgf(i,dest);
            nerr = ioresult((int *)&rwrec[i]);
/* Non-fatal errors */
            if (rwrec[i].nfatal) {
               if ((rwrec[i].nfatal+1024)==EWOULDBLOCK)
                  fprintf(unit2,"Flow Control prevents sending data\n");
/* Do not wait more than 5 minutes for message to start sending */
               if (etmsec(&rwrec[i].ts[0]) >= mstime) {
                  nerr = rwrec[i].nfatal;
                  fprintf(unit2,"Send Retry failed\n");
                  ierror = nerr;
                  ioc[dest][3] = 0;
                  *flag = 1;
               }
            }
/* Fatal errors */
            else if (nerr < 0) {
               ierror = nerr + 1024;
               fprintf(unit2,"Send Error, oss = %d, %s\n",ierror,
                    strerror(ierror));
               ierror = nerr;
               *flag = 1;
            }
/* Data successfully sent */
            else {
/* Debug information */
               fprintf(unit2,"data block sent, current total = %d\n",
                       rwrec[i].len);
               if (!nerr)
                  *flag = 1;
            }
         }
/* Try to determine cause of Timeout in Receive */
         else {
            source = curreq[i][1];
            ts = .001*(float) etmsec(&rwrec[i].ts[0]);
            fprintf(unit2,"Receive Timeout, %f sec, Retrying...\n",ts);
/* Debug information */
            fprintf(unit2,"source=%d,size=%d,tag=%d\n",
                       source,rwrec[i].sln,curreq[i][3]);
/* Attempt to send next block of data */
            rcvmsgf(i,source);
            nerr = ioresult((int *)&rwrec[i]);
/* Non-fatal errors */
            if (rwrec[i].nfatal) {
               if ((rwrec[i].nfatal+1024)==EWOULDBLOCK)
                  fprintf(unit2,"Flow Control prevents accepting data\n");
/* Do not wait more than 5 minutes for message to start sending */
               if (etmsec(&rwrec[i].ts[0]) >= mstime) {
                  nerr = rwrec[i].nfatal;
                  fprintf(unit2,"Receive Retry failed\n");
                  ierror = nerr;
                  ioc[source][2] = 0;
                  *flag = 1;
               }
            }
/* Fatal errors */
            else if (nerr < 0) {
               ierror = nerr + 1024;
               fprintf(unit2,"Receive Error, oss = %d, %s\n",ierror,
                    strerror(ierror));
               ierror = nerr;
               *flag = 1;
            }
/* Data successfully received */
            else {
/* Debug information */
               fprintf(unit2,"data block received, current total = %d\n",
                       rwrec[i].len);
               if (!nerr)
                  *flag = 1;
            }
         }
/* Allow Open Transport to resume sending events */
L20:     sigprocmask(SIG_UNBLOCK,&sset,NULL);
         if (!(*flag))
            return ierror;
      }
   }
/* Data read or write has completed */
   else {
      nerr = ioresult((int *)&rwrec[i]);
      *flag = 1;
   }
/* Get requested length */
   slen = rwrec[i].sln;
/* Get actual length */
   rlen = rwrec[i].len;
/* Read current request record */
   tag = curreq[i][3];
/* Define length for MPI_Get_count */
   status->len = rlen;
/* Check for send errors */
   if (curreq[i][0] < 0) {
      dest = curreq[i][1];
/* Define type for MPI_Get_count */
      status->type = curreq[i][4];
/* Check for write errors */
      if (nerr < 0) {
         if (!ierror) {
            ierror = nerr + 1024;
            fprintf(unit2,"Send Error, oss = %d, %s\n",ierror,
                    strerror(ierror));
            fprintf(unit2,"dest, tag = %d,%d\n",dest,tag);
            ierror = nerr;
         }
      }
      else if (rlen != slen) {
         fprintf(unit2,"Send Length Error,dest,tag,requested/actual length=%d,%d,%d,%d\n",
                 dest,tag,slen,rlen);
         ierror = 8;
      }
/* Log MPI message state change and display status */
      if (monitor > 0) {
         if (checkesc(0)) {
            ierror = -9;
            writerrs("MPI_Test: ",ierror);
            return ierror;
         }
         else if (!ierror) {
/* Convert difference between time steps into microseconds */
            nerr = sbtusec(&rwrec[i].ts[0],&rwrec[i].ts[1]);
            logmess(dest,-1,rlen,nerr,tag);
         }
      }
      goto L30;
   }
/* Read current request record */
   source = curreq[i][1];
   comm = curreq[i][2];
   datatype = curreq[i][4];
/* Read header */
/* Get received tag from header */
   rtag = header[i].hdata[1];
/* Get received comm from header */
   rcomm = header[i].hdata[0];
/* Get received datatype from header */
   rdatat = header[i].hdata[2];
/* Define source, tag and type for MPI_Get_count */
   status->source = source;
   status->tag = rtag;
   status->type = rdatat;
/* Check for receive errors */
   if (nerr < 0) {
      if (!ierror) {
         ierror = nerr + 1024;
         fprintf(unit2,"Receive Error, oss = %d, %s\n",ierror,
                 strerror(ierror));
         fprintf(unit2,"source, tag = %d,%d\n",source,tag);
         ierror = nerr;
      }
   }
/* Length error */
   else if (rlen > slen) {
      fprintf(unit2,"Read Length Error, source, tag, requested/actual = %d,%d,%d,%d\n",
              source,tag,slen,rlen);
      fprintf(unit2,"Possibly attempting to receive data out of order\n");
      ierror = 13;
   }
/* Check for incomplete message, this should never be able to happen */
   else if (rlen < header[i].hdata[3]) {
      fprintf(unit2,"Incomplete Read, source, tag, requested/actual = %d,%d,%d,%d\n",    
              source,tag,header[i].hdata[3],rlen);
      ierror = 12;
   }
/* Comm error */
   else if (rcomm != comm) {
      fprintf(unit2,"Read Comm Error, source, tag, expected/received comm = %d,%d,%d,%d\n",
              source,tag,comm,rcomm);
      ierror = 9;
   }
/* Tag error */
   else if ((tag >= 0) && (rtag != tag)) {
      fprintf(unit2,"Read Tag Error, source, expected/received tag = %d,%d,%d\n",
              source,tag,rtag);
      fprintf(unit2,"Possibly attempting to receive data out of order\n");
      ierror = 10;
   }
/* Type error */
   else if (rdatat != datatype) {
      fprintf(unit2,"Read Type Error, source, tag, expected/received type = %d,%d,%d,%d\n",
              source,tag,datatype,rdatat);
      ierror = 11;
   }
/* Log MPI message state change and display status */
   if (monitor > 0) {
      if (checkesc(0)) {
         ierror = -9;
         writerrs("MPI_Test: ",ierror);
         return ierror;
      }
      else if (!ierror) {
/* Convert difference between time steps into microseconds */
         nerr = sbtusec(&rwrec[i].ts[0],&rwrec[i].ts[1]);
         logmess(source,-2,rlen,nerr,tag);
      }
   }
/* Store error code */
L30: status->error = ierror;
/* Nullify transmission mode */
   curreq[i][0] = 0;
/* Nullify request handle */
   *request = MPI_REQUEST_NULL;;
/* Handle read and write errors */
   if (ierror) {
/* Write out readwrite record */
      rwstat(i,unit2);
      wqueue(unit2);
      for (i = 0; i < MAXM; i++) {
         if (curreq[i][0] != 0)
            rwstat(i,unit2);
      }
      writerrs("MPI_Test: ",ierror);
   }
   return ierror;
}

void sndmsgf(MPI_Request request, int dest) {
/* send a message fragment
   request = request handle
   dest = rank of destination
local data                          */
   int i, np, nps;
   long response, oss;
/* internal mpi common block
   ioc = array of context pointers for notifier function            */
/* internal common block for non-blocking messages
   curreq = request record for transmission parameters
   rwrec = read/write record for asynchronous messages
   mqueue = message request queue                                     */
   i = request;
   np = dest;
/* Obtain the current time stamp */
   response = gettimeofday(&rwrec[i].ts[1],NULL);
/* Send data to a remote peer */
L10: if (rwrec[i].ioflag==1)
      response = writev(rwrec[i].ref,rwrec[i].buf,rwrec[i].nbytes);
   else
      response = send(rwrec[i].ref,rwrec[i].buf,rwrec[i].nbytes,
                      rwrec[i].flags);
   if (response < 0)
      response = errno - 1024;
/* Process data which has been sent */
   if (response > 0) {
/* Clear non-fatal error code */
      rwrec[i].nfatal = 0;
/* Set actual length sent */
      rwrec[i].len += response;
/* Check for incomplete header */
      if (rwrec[i].len < 0) {
         header[i].iov[0].iov_base = 
                     (void *)(((unsigned char *)header[i].iov[0].iov_base) + response);
         header[i].iov[0].iov_len -= response;
         goto L10;
      }
/* Check for incomplete data */
      else if (rwrec[i].sln > rwrec[i].len) {
/* Header is sent, readjust parameters to send data */
         if (rwrec[i].ioflag==1) {
            rwrec[i].buf = rwrec[i].sbuf;
            rwrec[i].nbytes = rwrec[i].sln;
            response -= header[i].iov[0].iov_len;
         }
/* Readjust buffer pointer */
         rwrec[i].buf = (void *)(((unsigned char *)rwrec[i].buf) + response);
         rwrec[i].nbytes -= response;
         rwrec[i].ioflag += 1;
      } 
/* Data is sent */
      else {
/* Obtain the current time stamp */
         response = gettimeofday(&rwrec[i].ts[1],NULL);
         rwrec[i].ioflag = 0;
/* Get next message if messages are queued */
         if (rwrec[i].nextm > 0) {
            i = rwrec[i].nextm;
            if (i==mqueue[np][1])
               mqueue[np][1] = 0;
            ioc[np][3] = i;
            i -= 1;
            goto L10;
         }
/* Clear pointer to current send record */
         else
            ioc[np][3] = 0;
      }
   }
/* Check for errors */
   else {
/* Check for EOF */
      if (response==0)
         response = -1;
      oss = response + 1024;
/* Process non-fatal errors */
      if (oss==EWOULDBLOCK)
         rwrec[i].nfatal = response;
/* Process fatal errors */
      else {
/* Find actual destination */
         nps = np;
         if (nps==MAXS)
            nps = idproc;
         if (response==(-1))
            fprintf(unit2,"Send Error, oss = %ld, EOF\n",response);
         else
            fprintf(unit2,"Send Error, oss = %ld, %s\n",oss,
                    strerror(oss));
         fprintf(unit2,"dest, tag = %d,%d\n",nps,header[i].hdata[1]);
/* Set iocompletion flag to error */
         rwrec[i].ioflag = response;
/* Clear pointer to current send record */
         ioc[np][3] = 0;
      }
   }
   return;
}

void rcvmsgf(MPI_Request request, int source) {
/* receive a message fragment
   request = request handle
   source = rank of source
local data                          */
   int i, np;
/* int j; */
   long response, oss;
/* internal mpi common block
   ioc = array of context pointers for notifier function            */
/* internal common block for non-blocking messages
   curreq = request record for transmission parameters
   rwrec = read/write record for asynchronous messages
   trash = trash bin for unwanted data
   mqueue = message request queue                                     */
   i = request;
   np = source;
/* Obtain the current time stamp */
   response = gettimeofday(&rwrec[i].ts[1],NULL);
/* Read data sent from a remote peer */
L10: response = recv(rwrec[i].ref,rwrec[i].buf,rwrec[i].nbytes,
                     rwrec[i].flags);
   if (response < 0)
      response = errno - 1024;
/* Process data which has arrived */
   if (response > 0) {
/* Clear non-fatal error code */
      rwrec[i].nfatal = 0;
/* Set actual length received */
      rwrec[i].len += response;
/* Check if all the data has arrived */
      if (rwrec[i].len < header[i].hdata[3]) {
/* Incomplete data */
         if (rwrec[i].nbytes > response) {
/* Readjust buffer pointer */
            rwrec[i].buf = (void *)(((unsigned char *)rwrec[i].buf) + response);
            rwrec[i].nbytes -= response;
            if (rwrec[i].len >= 0)
               rwrec[i].ioflag += 1;
         }
/* Header is received, readjust parameters to receive data */
         else if (rwrec[i].ioflag==1) {
            rwrec[i].buf = rwrec[i].sbuf;
            rwrec[i].nbytes = imin(header[i].hdata[3],rwrec[i].sln);
            rwrec[i].ioflag = 2;
            goto L10;
         }
/* Data is received and buffer is full */
         else {
            rwrec[i].buf = &trash;
            rwrec[i].nbytes = 1024;
            goto L10;
         }
      }
/* Data is received */
      else {
/* Obtain the current time stamp */
         response = gettimeofday(&rwrec[i].ts[1],NULL);
         rwrec[i].ioflag = 0;
/* Get next message if messages are queued */
         if (rwrec[i].nextm > 0) {
            i = rwrec[i].nextm;
            if (i==mqueue[np][0])
               mqueue[np][0] = 0;
            ioc[np][2] = i;
         }
/* Clear pointer to current read record */
         else
            ioc[np][2] = 0;
      }
   }
/* Check for errors */
   else {
/* Check for EOF */
      if (response==0)
         response = -1;
      oss = response + 1024;
/* Process non-fatal errors */
      if (oss==EWOULDBLOCK)
         rwrec[i].nfatal = response;
/* Process fatal errors */
      else {
         if (response==(-1))
            fprintf(unit2,"Receive Error, oss = %ld, EOF\n",response);
         else
            fprintf(unit2,"Receive Error, oss = %ld, %s\n",oss,
                    strerror(oss));
         fprintf(unit2,"source, tag = %d,%d\n",np,header[i].hdata[1]);
/* Set iocompletion flag to error */
         rwrec[i].ioflag = response;
/* Clear pointer to current receive record */
         ioc[np][2] = 0;
      }
   }
   return;
}

int MPI_Wait(MPI_Request *request, MPI_Status *status) {
/* wait for an MPI send or receive to complete
   request = request handle
   status = status object
   input: request
   output: request, status
local data                                     */
   int ierror, flag;
L10: ierror = MPI_Test(request,&flag,status);
   if (!flag) goto L10;
   return ierror;
}

int MPI_Request_free(MPI_Request *request) {
/* free a communication request object
   request = request handle
   input: request
   output: request
local data                                                            */
   int ierror, i;
/* internal mpi common block
   nproc = number of real or virtual processors obtained              */
/* internal common block for non-blocking messages
   curreq = request record for transmission parameters
   rwrec = read/write record for asynchronous messages                */
   ierror = 0;
/* check for error conditions */
   i = *request;
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* null request */
   else if (*request < 0) {
      return 0;
   }
/* invalid request handle */
   else if (i >= MAXM)
      ierror = 16;
   else if (curreq[i][0]==0)
      ierror = 16;
/* handle errors */
   if (ierror) {
      writerrs("MPI_Request_free ",ierror);
      return ierror;
   }
/* Check if data read or write has completed */
   if (ioresult((int *)&rwrec[i]) <= 0) {
/* Nullify transmission mode */
      curreq[i][0] = 0;
/* Nullify request handle */
      *request = MPI_REQUEST_NULL;
   }
   else {
      fprintf(unit2,"MPI_Request_free: Message not Completed\n");
/* Write out readwrite record */
      rwstat(i,unit2);
      ierror = 32;
   }
   return ierror;
}

int MPI_Sendrecv(void* sendbuf, int sendcount, MPI_Datatype sendtype,
                 int dest, int sendtag, void* recvbuf, int recvcount,
                 MPI_Datatype recvtype, int source, int recvtag,
                 MPI_Comm comm, MPI_Status *status) {
/* blocking send and receive operation
   sendbuf = initial address of send buffer
   sendcount = number of entries to send
   sendtype = type of entries in send buffer
   dest = rank of destination
   sendtag = send tag
   recvbuf = initial address of receive buffer
   recvcount = max number of entries to receive
   recvtype = type of entries in receive buffer
   source = rank of source
   recvtag = receive tag
   comm = communicator
   status = return status
   input: sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount
         recvtype, source, recvtag, comm
   output: recvbuf, status
local data                                                                */
   int ierror;
   MPI_Request recvreq, sendreq;
/* post non-blocking receive and send */
   ierror = MPI_Irecv(recvbuf,recvcount,recvtype,source,recvtag,comm,&recvreq);
   ierror = MPI_Isend(sendbuf,sendcount,sendtype,dest,sendtag,comm,&sendreq);
/* wait for send and receive */
   ierror = MPI_Wait(&sendreq,status);
   ierror = MPI_Wait(&recvreq,status);
   return ierror;
}

int MPI_Ssend(void* buf, int count, MPI_Datatype datatype, int dest,
              int tag, MPI_Comm comm) {
/* blocking synchronous mode send
   buf = initial address of send buffer
   count = number of entries to send
   datatype = datatype of each entry
   dest = rank of destination
   tag = message tag
   comm = communicator
   input: buf, count, datatype, dest, tag, comm
   comment: this is just a temporary stub
local data                                                           */
   int ierror;
   ierror = MPI_Send(buf,count,datatype,dest,tag,comm);
   return ierror;
}

int MPI_Issend(void* buf, int count, MPI_Datatype datatype, int dest,
               int tag, MPI_Comm comm, MPI_Request *request) {
/* start a non-blocking synchronous mode send
   buf = initial address of send buffer
   count = number of entries to send
   datatype = datatype of each entry
   dest = rank of destination
   tag = message tag
   comm = communicator
   request = request handle
   input: buf, count, datatype, dest, tag, comm
   output: request
   comment: this is just a temporary stub
local data                                                        */
   int ierror;
   ierror = MPI_Isend(buf,count,datatype,dest,tag,comm,request);
   return ierror;
}

int MPI_Waitall(int count, MPI_Request *array_of_requests,
                MPI_Status *array_of_statuses) {
/* wait for a collection of specified MPI sends or receives to complete
   count = list length
   array_of_requests = array of request handles
   array_of_statuses = array of status objects
   input: count, array_of_requests
   output: array_of_requests, array_of_statuses
local data                                                             */
   int ierror, i, ierr;
/* invalid count */
   if (count < 0) {
      fprintf(unit2,"Invalid list length = %d\n",count);
      ierror = 17;
      writerrs("MPI_Waitall: ",ierror);
      return ierror;
   }
   ierror = 0;
   for (i = 0; i < count; i++) {
      ierr = MPI_Wait(&array_of_requests[i],&array_of_statuses[i]);
      if (ierr)
      ierror = MPI_ERR_IN_STATUS;
   }
   return ierror;
}

int MPI_Waitany(int count, MPI_Request *array_of_requests,
                int *index, MPI_Status *status) {
/* wait for any specified MPI send or receive to complete
   count = list length
   array_of_requests = array of request handles
   index = index of request handle that completed
   status = status object
   input: count, array_of_requests
   output: array_of_requests, index, status
local data                                                 */
   int ierror, i, k, flag;
/* invalid count */
   if (count < 0) {
      fprintf(unit2,"Invalid list length = %d\n",count);
      ierror = 17;
      writerrs("MPI_Waitany: ",ierror);
      return ierror;
   }
/* find number of requests already completed */
   k = 0;
   for (i = 0; i < count; i++)
      if (array_of_requests[i] < 0)
         k = k + 1;
   if (k==count) {
      *index = -1;
      ierror = 0;
      return ierror;
   }
   i = 0;
L20: flag = 0;
   if (array_of_requests[i] >= 0)
      ierror = MPI_Test(&array_of_requests[i],&flag,status);
   if (flag)
      *index = i;
   else {
      i += 1;
      if (i >= count)
         i = 0;
      goto L20;
   }
   return ierror;
}

int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype,
                  int *count) {
/* get the number of "top level" elements
   status = return status of receive operation
   datatype = datatype of each receive buffer entry
   count = number of received entries
   input: status, datatype
   output: count
local data                                           */
   int ierror;
/* internal mpi common block
   nproc = number of real or virtual processors obtained    */     
   ierror = 0;
   *count = 0;
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* mismatched datatype */
   else if (datatype != status->type)
      ierror = 18;
/* calculate count */
   else if ((datatype==MPI_INT) || (datatype==MPI_FLOAT)) {
      *count = status->len/4;
      if (4*(*count) != status->len)
         *count = MPI_UNDEFINED;
   }
   else if (datatype==MPI_DOUBLE) {
      *count = status->len/8;
      if (8*(*count) != status->len)
         *count = MPI_UNDEFINED;
   }
   else if (datatype==MPI_BYTE)
      *count = status->len;
   else if ((datatype==MPI_2INT) || (datatype==MPI_FLOAT_INT)) {
         *count = status->len/8;
      if (8*(*count) != status->len)
         *count = MPI_UNDEFINED;
   }
   else if (datatype==MPI_DOUBLE_INT) {
      *count = status->len/16;
      if (16*(*count) != status->len)
         *count = MPI_UNDEFINED;
   }
/* invalid datatype */
   else
      ierror = 7;
/* handle errors */
   if (ierror)
      writerrs("MPI_Get_count: ",ierror);
   return ierror;
}

int MPI_Initialized(int *flag) {
/* indicate whether MPI_Init has been called
   flag = true if MPI_Init has been called, false otherwise
   output: flag
local data                                                  */
   int ierror;
/* internal mpi common block
   nproc = number of real or virtual processors obtained    */
   if (nproc > 0)
      *flag = 1;
   else
      *flag = 0;
   ierror = 0;
   return ierror;
}

int MPI_Comm_size(MPI_Comm comm, int *size) {
/* determine the size of the group associated with a communicator
   comm = communicator (this is ignored)
   size = number of processors in the group of comm
   input: comm
   output: size
local data                                                         */
   int ierror, np;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                                      */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* get size */
   else {
      np = mapcomm[comm][MAXS];
      if ((np > 0) && (np <= nproc))
         *size = np;
      else
         ierror = 2;
   }
/* handle errors */
   if (ierror)
      writerrs("MPI_Comm_size: ",ierror);
   return ierror;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank) {
/* determine the rank of the calling process in the communicator
   comm = communicator (this is ignored)
   rank = rank of the calling process in group of comm
   input: comm
   output: rank
local data                                                        */
   int ierror, np;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                     */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* get rank */
   else {
      np = mapcomm[comm][MAXS];
      if ((np > 0) && (np <= nproc)) {
         *rank = mapcomm[comm][MAXS+1];
         if ((*rank >= 0) && (*rank < np)) {
            if (mapcomm[comm][*rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
      else
         ierror = 2;
   }
/* handle errors */
   if (ierror)
      writerrs("MPI_Comm_rank: ",ierror);
   return ierror;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm) {
/* duplicate existing communicator with all its cached information
   comm = communicator
   newcomm = new communicator
   input: comm
   output: newcomm
local data                                                               */
   int ierror, np, i, j, k;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
   *newcomm = MPI_COMM_NULL;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Comm_dup: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Comm_dup started\n");
/* find space for communication record */
   i = 1;
L10: i += 1;
   if (i >= MAXC) {
      fprintf(unit2,"too many communicators\n");
      ierror = 25;
      writerrs("MPI_Comm_dup: ",ierror);
      return ierror;
   }
   else if (mapcomm[i][MAXS] > 0)
      goto L10;
/* check if all nodes agree on new communicator */
   ierror =  MPI_Allreduce(&i,&j,1,MPI_INT,MPI_MIN,comm);
   ierror =  MPI_Allreduce(&i,&k,1,MPI_INT,MPI_MAX,comm);
   if (j != k) {
/* try to find another communicator */
      i = k - 1;
      goto L10;
  }
/* duplicate mapping */
   for (j = 0; j < MAXS+MAXD+3; j++)
      mapcomm[i][j]= mapcomm[comm][j];
/* assign communicator */
   *newcomm = i;
   if (monitor==2)
      fprintf(unit2,"MPI_Comm_dup complete\n");
   return ierror;
}

int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm) {
/* create a new communicator based on color and key
   comm = communicator
   color = control of subset assignment
   key = control of rank assignment
   newcomm = new communicator
   input: comm, color, key
   output: newcomm
local data                                                               */
   int ierror, np, i, mp, j, k, l, kmin;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
   *newcomm = MPI_COMM_NULL;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid color */
   else if (color < (-1)) {
      fprintf(unit2,"Invalid color = %d\n",color);
      ierror = 23;
   }
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Comm_split: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Comm_split started\n");
/* find space for communication record */
   i = 1;
L10: i += 1;
   if (i >= MAXC) {
      fprintf(unit2,"too many communicators, color = %d\n",color);
      ierror = 25;
      writerrs("MPI_Comm_split: ",ierror);
      return ierror;
   }
   else if (mapcomm[i][MAXS] > 0)
      goto L10;
/* check if all nodes agree on new communicator */
   ierror =  MPI_Allreduce(&i,&j,1,MPI_INT,MPI_MIN,comm);
   ierror =  MPI_Allreduce(&i,&k,1,MPI_INT,MPI_MAX,comm);
   if (j != k) {
/* try to find another communicator */
      i = k - 1;
      goto L10;
  }
/* gather all the colors */
   ierror = MPI_Allgather(&color,1,MPI_INT,&mapcomm[i][0],1,MPI_INT,comm);
/* this node does not participate */
   if (color==MPI_UNDEFINED) {
      if (monitor==2)
         fprintf(unit2,"MPI_Comm_split complete\n");
      return ierror;
   }
/* find other processors with same color */
   mp = -1;
   mapcomm[i][MAXS+1] = -1;
   for (j = 0; j < np; j++) {
      if (mapcomm[i][j]==color) {
         mp += 1;
         k = mapcomm[comm][j];
         if ((k >= 0) || (k < np)) {
            mapcomm[i][mp] = k;
            if (k==idproc)
               mapcomm[i][MAXS+1] = mp;
         }
         else
            ierror = 2;
      }
   }
   mp += 1;
/* no nodes with found with color */
   if (!mp) {
      fprintf(unit2,"Self color not found\n");
      ierror = 2;
   }
/* no nodes found with idproc */
   else if (mapcomm[i][MAXS+1] < 0) {
      fprintf(unit2,"Self rank not found\n");
      ierror = 2;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Comm_split: ",ierror);
      return ierror;
   }
/* finish mapping */
   for (j = mp; j < MAXS; j++)
      mapcomm[i][j] = MPI_UNDEFINED;
   mapcomm[i][MAXS] = mp;
/* assign communicator */
   *newcomm = i;
/* gather all the keys into MPI_COMM_SELF mapping */
   ierror = MPI_Allgather(&key,1,MPI_INT,&mapcomm[1][0],1,MPI_INT,*newcomm);
   k = 0;
/* find lowest remaining key */
L40: kmin = mapcomm[1][k];
   for (j = k+1; j < mp; j++) {
      if (mapcomm[1][j]  < kmin)
         kmin = mapcomm[1][j];
   }
/* order all nodes with lowest remaining key */
   for (j = k; j < mp; j++) {
      if (mapcomm[1][j]==kmin) {
         k += 1;
/* right shift node and key order, if necessary */
         if (j >= k) {
            np = mapcomm[i][j];
            for (l = 0; l < j-k+1; l++) {
               mapcomm[i][j-l] = mapcomm[i][j-l-1];
               mapcomm[1][j-l] = mapcomm[1][j-l-1];
            }
            mapcomm[i][k-1] = np;
         }
      }
   }
   if (k < mp)
      goto L40;
/* find self rank */
   mapcomm[i][MAXS+1] = -1;
   for (j = 0; j < mp; j++) {
      k = mapcomm[i][j];
      if (k==idproc)
         mapcomm[i][MAXS+1] = j;
    }
   mapcomm[i][MAXS+2] = 0;
/* restore MPI_COMM_SELF map */
   mapcomm[1][0] = idproc;
   for (j = 1; j < mp; j++)
      mapcomm[1][j] = MPI_UNDEFINED;
   if (monitor==2)
      fprintf(unit2,"MPI_Comm_split complete\n");
   return ierror;
}

int MPI_Comm_free(MPI_Comm *comm) {
/* mark the communicator object for deallocation
   comm = communicator
   input: comm
   output: comm
local data                                                 */
   int ierror, np, i;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                              */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((*comm < 0) || (*comm >= MAXC))
      ierror = 2;
/* release communicator */
   else if (*comm > 1) {
      np = mapcomm[*comm][MAXS];
      if ((np > 0) && (np <= nproc)) {
/* clear mapping */
         for (i = 0; i < MAXS; i++) {
            mapcomm[*comm][i] = 0;
            mapcomm[*comm][MAXS] = 0;
            mapcomm[*comm][MAXS+1] = MPI_UNDEFINED;
            mapcomm[*comm][MAXS+2] = 0;
            *comm = MPI_COMM_NULL;
         }
      }
      else
         ierror = 2;
   }
/* handle errors */
   if (ierror)
      writerrs("MPI_Comm_free: ",ierror);
   return ierror;
}

int MPI_Cart_create(MPI_Comm comm_old, int ndims, int *dims, int *periods,
                    int reorder, MPI_Comm *comm_cart) {
/* make new communicator to which topology information has been attached
   comm_old = input communicator
   ndims = number of dimensions of Cartesian grid
   dims = array specifying the number of processes in each dimension
   periods = array specifying whether grid is periodic or not
   reorder = specifies whether ranks may be reordered or not (ignored)
   comm_cart = communicator with new Carteisan topology
   input: comm_old, ndims, dims, periods, reorder
   output: comm_cart
local data                                                               */
   int ierror, np, mp, i, j, k;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
   *comm_cart = MPI_COMM_NULL;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm_old < 0) || (comm_old >= MAXC))
      ierror = 2;  
/* invalid topology */
   else if ((ndims < 0) || (ndims > MAXD)) {
       fprintf(unit2,"Invalid topology dimension = %d\n",ndims);
       ierror = 26;
   }
/* communicator errors */
   else {
      np = mapcomm[comm_old][MAXS];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* check topology length */
      else {
         mp = 1;
         for (i = 0; i < ndims; i++)
            mp = mp*dims[i];
         if (!ndims)
            mp = 0;
         if ((mp < 0) || (mp > np)) {
            fprintf(unit2,"Invalid Cartesian topology size = %d\n",mp);
            ierror = 27;
         }
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Cart_create: ",ierror);
      return ierror;
   }
   if (!mp)
      return ierror;
   if (monitor==2)
      fprintf(unit2,"MPI_Cart_create started\n");
/* find space for communication record */
   i = 1;
L20: i += 1;
   if (i >= MAXC) {
      fprintf(unit2,"too many communicators\n");
      ierror = 25;
      writerrs("MPI_Cart_create: ",ierror);
      return ierror;
   }
   else if (mapcomm[i][MAXS] > 0)
      goto L20;
/* check if all nodes agree on new communicator */
   ierror =  MPI_Allreduce(&i,&j,1,MPI_INT,MPI_MIN,comm_old);
   ierror =  MPI_Allreduce(&i,&k,1,MPI_INT,MPI_MAX,comm_old);
   if (j != k) {
/* try to find another communicator */
      i = k - 1;
      goto L20;
  }
/* quit if processor is beyond range of topology */
   if (mapcomm[comm_old][MAXS+1] >= mp) {
      if (monitor==2)
         fprintf(unit2,"MPI_Cart_create complete\n");
      return ierror;
   }
/* create mapping */
   for (j = 0; j < mp; j++)
      mapcomm[i][j] = mapcomm[comm_old][j];
   for (j = mp; j < MAXS; j++)
      mapcomm[i][j]= MPI_UNDEFINED;
   mapcomm[i][MAXS] = mp;
   mapcomm[i][MAXS+1] = mapcomm[comm_old][MAXS+1];
/* store topology */
   mapcomm[i][MAXS+2] = ndims;
   for (j = 0; j < ndims; j++)
   if (periods[j])
      mapcomm[i][MAXS+3+j] = dims[j];
   else
      mapcomm[i][MAXS+3+j] = -dims[j];
/* assign communicator */
   *comm_cart = i;
   if (monitor==2)
      fprintf(unit2,"MPI_Cart_create complete\n");
   return ierror;
}

int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int *coords) {
/* determine process coords in Cartesian topology, given rank in group
   comm = communicator with Cartesian structure
   rank = rank of a process within group of comm
   maxdims = length of vector coords in the calling program
   coords = array containing Cartesian coordinates of specified process
   input: comm, rank, maxdims
   output: coords
local data                                                              */
   int ierror, np, ndims, i, j;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                                            */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      ndims = mapcomm[comm][MAXS+2];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid topology */
      else if ((ndims < 1) || (ndims > MAXD)) {
         fprintf(unit2,"Invalid topology dimension = %d\n",ndims);
         ierror = 26;
      }
/* invalid vector length */
      else if (maxdims < ndims) {
          fprintf(unit2,"Vector length too small = %d\n",maxdims);
          ierror = 28;
      }
/* invalid rank */
      else {
         if ((rank < 0) || (rank >= np)) {
            fprintf(unit2,"Invalid rank = %d\n",rank);
            ierror = 29;
         }
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Cart_coords: ",ierror);
      return ierror;
   }
/* calculate cartesian coordinates */
   j = rank;
   for (i = 0; i < ndims; i++) {
      np = np/abs(mapcomm[comm][MAXS+3+i]);
      coords[i] = j/np;
      j -= coords[i]*np;
   }
   return ierror;
}

int MPI_Cart_get(MPI_Comm comm, int maxdims, int *dims, int *periods,
                int *coords) {
/* retrieve cartesian topology information associated with communicator
   comm = communicator with Cartesian structure
   maxdims = length of vectors dims, periods and coords
   dims = number of processes for each Cartesian dimension
   periods = periodicity (true/false) for each Cartesian dimension
   coords = array containing Cartesian coordinates of specified process
   input: comm, maxdims
   output: dims, periods, coords
local data                                                               */
   int ierror, np, ndims, rank, i, j, k;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      ndims = mapcomm[comm][MAXS+2];
      rank = mapcomm[comm][MAXS+1];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid topology */
      else if ((ndims < 1) || (ndims > MAXD)) {
         fprintf(unit2,"Invalid topology dimension = %d\n",ndims);
         ierror = 26;
      }
/* invalid vector length */
      else if (maxdims < ndims) {
          fprintf(unit2,"Vector length too small = %d\n",maxdims);
          ierror = 28;
      }
/* get rank */
      else if ((rank >= 0) && (rank < np)) {
         if (mapcomm[comm][rank] != idproc)
            ierror = 29;
      }
      else
         ierror = 29;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Cart_get: ",ierror);
      return ierror;
   }
/* calculate dimension, periodicity, and cartesian coordinates */
   j = rank;
   for (i = 0; i < ndims; i++) {
      k = mapcomm[comm][MAXS+3+i];
      if (k > 0)
         periods[i] = 1;
      else {
         periods[i] = 0;
         k = -k;
      }
      dims[i] = k;
      np = np/k;
      coords[i] = j/np;
      j -= coords[i]*np;
   }
   return ierror;
}

int MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int *rank_source,
                   int *rank_dest) {
/* return shifted source and destination ranks given shift direction and
   amount
   comm = communicator with Cartesian structure
   direction = coordinate dimension shift
   disp = displacement (> 0: upwards shift, < 0: downwards shift)
   rank_source = rank of source process
   rank_dest = rank of destination process
   input: comm, direction, disp
   output: rank_source, rank_dest
local data                                                               */
   int ierror, np, ndims, rank, i, j, k, l, shift;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                                            */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      ndims = mapcomm[comm][MAXS+2];
      rank = mapcomm[comm][MAXS+1];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid topology */
      else if ((ndims < 1) || (ndims > MAXD)) {
         fprintf(unit2,"Invalid topology dimension = %d\n",ndims);
         ierror = 26;
      }
/* invalid direction */
      else if ((direction < 0) || (direction >= ndims)) {
          fprintf(unit2,"Invalid direction = %d\n",direction);
          ierror = 31;
      }
/* get rank */
      else if ((rank >= 0) && (rank < np)) {
         if (mapcomm[comm][rank] != idproc)
            ierror = 29;
      }
      else
         ierror = 29;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Cart_shift: ",ierror);
      return ierror;
   }
/* check if shift amount is zero */
   if (!disp) {
      *rank_source = rank;
      *rank_dest = rank;
      return ierror;
   }
/* find coordinate for selected direction */
   j = rank;
   shift = np;
   for (i = 0; i < direction+1; i++) {
      shift = shift/abs(mapcomm[comm][MAXS+3+i]);
      k = j/shift;
      j -= k*shift;
   }
/* calculate size of shift */
   shift = 1;
   for (i = direction+1; i < ndims; i++)
      shift = shift*abs(mapcomm[comm][MAXS+3+i]);
/* size of selected direction */
   l = mapcomm[comm][MAXS+3+direction];
/* calculate new coordinate */
/* periodic boundary conditions */
   if (l > 0) {
/* make disp also periodic */
      i = abs(disp)%l;
      if (disp < 0)
         i = -i;
/* right neighbor */
      j = k + i;
      if (j < 0)
         j += l;
      else if (j >= l)
         j -= l;
      *rank_dest = rank + (j - k)*shift;
/* left neighbor */
      j = k - i;
      if (j < 0)
         j += l;
      else if (j >= l)
         j -= l;
      *rank_source = rank + (j - k)*shift;
   }
/* non-periodic boundary conditions */
   else if (l < 0) {
/* right neighbor */
      j = k + disp;
      if ((j < 0) || (j >= (-l)))
         *rank_dest = MPI_PROC_NULL;
      else
         *rank_dest = rank + (j - k)*shift;
/* left neighbor */
      j = k - disp;
      if ((j < 0) || (j >= (-l)))
         *rank_source = MPI_PROC_NULL;
      else
         *rank_source = rank + (j - k)*shift; 
   }
/* verify ranks */
   if (*rank_source != MPI_PROC_NULL) {
      if ((*rank_source < 0) || (*rank_source >= np)) {
         fprintf(unit2,"rank_source = %d\n",*rank_source);
         ierror = 29;
      }
   }
   if (*rank_dest != MPI_PROC_NULL) {
      if ((*rank_dest < 0) || (*rank_dest >= np)) {
         fprintf(unit2,"rank_dest = %d\n",*rank_dest);
         ierror = 29;
      }
   }
/* process errors */
   if (ierror)
      writerrs("MPI_Cart_shift: ",ierror);
   return ierror;
}

int MPI_Cart_rank(MPI_Comm comm, int *coords, int *rank) {
/* determine process rank in communicator, given Cartesian location
   comm = communicator with Cartesian structure
   coords = array specifying Cartesian coordinates of a process
   rank = rank of specified process
   input: comm, coords
   output: rank
local data                                                           */
   int ierror, np, ndims, i, j, k, l;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                                        */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      ndims = mapcomm[comm][MAXS+2];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid topology */
      else if ((ndims < 1) || (ndims > MAXD)) {
         fprintf(unit2,"Invalid topology dimension = %d\n",ndims);
         ierror = 26;
      }
/* invalid coords */
      else {
         for (i = 0; i < ndims; i++) {
            j = mapcomm[comm][MAXS+3+i];
            if (j < 0) {
               if ((coords[i] < 0) || (coords[i] >= (-j))) {
                  fprintf(unit2,"Invalid ith coord = %d,%d\n",i,coords[i]);
                  ierror = 30;
               }
            }
         }
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Cart_rank: ",ierror);
      return ierror;
   }
/* calculate rank, wrap periodic coordinates */
   l = 0;
   for (i = 0; i < ndims; i++) {
      j = mapcomm[comm][MAXS+3+i];
      k = coords[i];
      if (j > 0) {
         if (k < 0)
            k += j;
         else if (k >= j)
            k -= j;
      }
      else
         j = -j;
      l = k + j*l;
   }
/* verify rank */
   if ((l < 0) || (l >= np)) {
      fprintf(unit2,"Invalid coords, resulting rank = %d\n",l);
      ierror = 30;
      writerrs("MPI_Cart_rank: ",ierror);
   }
   else
      *rank = l;
   return ierror;
}

int MPI_Cart_sub(MPI_Comm comm, int *remain_dims, MPI_Comm *newcomm) {
/* partition communicator into subgroups that form lower-dimensional
   Cartesian subgrids
   comm = communicator with Cartesian structure
   remain_dims = each entry of remain_dims specifies whether dimension is
   kept in the subgrid or not
   newcomm = communicator containing the subgrid
   input: comm, remain_dims
   output: newcomm
local data                                                               */
   int ierror, np, ndims, rank, i, j, k, l, m, mp, color, key;
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
   *newcomm = MPI_COMM_NULL;
/* check for error conditions */
/* MPI not initialized */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;  
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      ndims = mapcomm[comm][MAXS+2];
      rank = mapcomm[comm][MAXS+1];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid topology */
      else if ((ndims < 1) || (ndims > MAXD)) {
         fprintf(unit2,"Invalid topology dimension = %d\n",ndims);
         ierror = 26;
      }
/* get rank */
      else if ((rank >= 0) && (rank < np)) {
         if (mapcomm[comm][rank] != idproc)
            ierror = 29;
      }
      else
         ierror = 29;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Cart_sub: ",ierror);
      return ierror;
   }
/* find dimension of new topology */
   k = 0;
   for (j = 0; j < ndims; j++) {
      if (remain_dims[j])
         k += 1;
   }
/* empty topology */
   if (!k)
      return ierror;
   if (monitor==2)
      fprintf(unit2,"MPI_Cart_sub started\n");
/* find space for communication record */
   i = 1;
L20: i += 1;
   if (i >= MAXC) {
      fprintf(unit2,"too many communicators\n");
      ierror = 25;
      writerrs("MPI_Cart_sub: ",ierror);
      return ierror;
   }
   else if (mapcomm[i][MAXS] > 0)
      goto L20;
/* check if all nodes agree on new communicator */
   ierror =  MPI_Allreduce(&i,&j,1,MPI_INT,MPI_MIN,comm);
   ierror =  MPI_Allreduce(&i,&k,1,MPI_INT,MPI_MAX,comm);
   if (j != k) {
/* try to find another communicator */
      i = k - 1;
      goto L20;
  }
/* find color for missing dimension */
   j = rank;
   color = 0;
   key = 0;
   mp = np;
   for (l = 0; l < ndims; l++) {
      m = abs(mapcomm[comm][MAXS+3+l]);
      mp = mp/m;
      k = j/mp;
      j -= k*mp;
      if (remain_dims[l])
         key = k + key*m;
      else
         color = k + color*m;
   }
/* gather all the colors */
   ierror = MPI_Allgather(&color,1,MPI_INT,&mapcomm[i][0],1,MPI_INT,comm);
/* find other processors with same color */
   mp = -1;
   mapcomm[i][MAXS+1] = -1;
   for (j = 0; j < np; j++) {
      if (mapcomm[i][j]==color) {
         mp += 1;
         k = mapcomm[comm][j];
         if ((k >= 0) || (k < np)) {
            mapcomm[i][mp] = k;
            if (k==idproc)
               mapcomm[i][MAXS+1] = mp;
         }
         else
            ierror = 2;
      }
   }
   mp += 1;
/* no nodes with found with color */
   if (!mp) {
      fprintf(unit2,"Self color not found\n");
      ierror = 2;
   }
/* no nodes found with idproc */
   else if (mapcomm[i][MAXS+1] < 0) {
      fprintf(unit2,"Self rank not found\n");
      ierror = 2;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Cart_sub: ",ierror);
      return ierror;
   }
/* finish mapping */
   for (j = mp; j < MAXS; j++)
      mapcomm[i][j] = MPI_UNDEFINED;
   mapcomm[i][MAXS] = mp;
/* assign communicator */
   *newcomm = i;
/* create new topology */
   k = 0;
   for (j = 0; j < ndims; j++) {
      if (remain_dims[j]) {
         k += 1;
         mapcomm[i][MAXS+2+k] = mapcomm[comm][MAXS+3+j];
      }
   }
   mapcomm[i][MAXS+2] = k;
   if (monitor==2)
      fprintf(unit2,"MPI_Cart_sub complete\n");
   return ierror;
}

int MPI_Dims_create(int nnodes, int ndims, int *dims) {
/* create a division of processes in a Cartesian grid
   nnodes = number of nodes in a grid
   ndims = number of Cartesian dimensions
   dims = array specifying number of nodes in each dimension
   input: nnodes, ndims, dims
   output: dims
local data                                                    */
   int ierror, i, j, nd, mp, md;
   ierror = 0;
/* check for errors */
   nd = 0;
   mp = 1;
   for (i = 0; i < ndims; i++) {
      if (dims[i]==0)
         nd += 1;
      else if (dims[i] > 0)
         mp = mp*dims[i];
      else if (dims[i] < 0)
         ierror = 26;
   }
   if (!nd)
      return ierror;
   if (ierror)
      fprintf(unit2,"Invalid topology dimension\n");
   else if ((nnodes < 1) || (nnodes > MAXS)) {
      fprintf(unit2,"Invalid number of nodes = %d\n",nnodes);
      ierror = 24;
   }
   else {
      md = nnodes/mp;
      if ((md*mp) != nnodes) {
         fprintf(unit2,"Topology dimension, node number inconsistent\n");
         ierror = 26;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Dims_create: ",ierror);
      return ierror;
   }
/* look for nd factors of md */
   for (i = 0; i < ndims; i++) {
      if (!dims[i]) {
         mp = (int) (exp(log((float) md/(float) nd)) + .0001);
         if (((int) (pow(mp,nd) + .0001)) < md)
            mp += 1;
L20:     j = md/mp;
         if ((j*mp)==md) {
            dims[i] = mp;
            md = j;
            nd -= 1;
         }
         else {
            mp += 1;
            if (mp <= md)
               goto L20;
            fprintf(unit2,"MPI_Dims_create: factor not found\n");
            ierror = 26;
         }
      }
   }
/* sanity check */
   if ((md != 1) || (nd != 0)) {
      fprintf(unit2,"MPI_Dims_create: missing factors\n");
      ierror = 26;
   }
   return ierror;
}

int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype,
              int root, MPI_Comm comm) {
/* broadcast a message from root to all processes in comm
   buffer = starting address of buffer
   count = number of entries in buffer
   datatype = datatype of buffer
   root = rank of broadcast root
   comm = communicator
   input: buffer, count, datatype, root, comm
   output: buffer
local data                                                               */
   int ierror, i, np, id, rank;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid root */
   else if ((root < 0) || (root >= nproc))
      ierror = 19;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      id = mapcomm[comm][root];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid root */
      else if ((root < 0) || (root >= np))
         ierror = 19;
/* invalid mapping */
      else if ((id < 0) || (id >= nproc)) {
         fprintf(unit2,"Invalid mapping, root, node = %d,%d\n",root,id);
         ierror = 2;
      }
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Bcast: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Bcast started\n");
/* start broadcast */
   if (rank==root) {
      for (i = 0; i < np; i++) {
         if (i != root)
           ierror = MPI_Send(buffer,count,datatype,i,0,comm);
      }
   }
   else
      ierror = MPI_Recv(buffer,count,datatype,root,0,comm,&status);
   if (monitor==2)
      fprintf(unit2,"MPI_Bcast complete\n");
   return ierror;
}

int MPI_Barrier(MPI_Comm comm) {
/* blocks each process in comm until all processes have called it.
   comm = communicator
   input: comm
local data                                                               */
   int ierror, np, rank, ntasks, isync, irync, i;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Barrier: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Barrier started\n");
/* begin synchronization */
   ntasks = np - 1;
   isync = -1;
   if (rank==0) {
/* processor 0 receives a message from everyone else */
      for (i=1; i <= ntasks; i++) {
         ierror = MPI_Recv(&irync,1,MPI_INT,i,0,comm,&status);
         if (irync != isync)
            fprintf(unit2,"sync error from proc %d\n",i);
      }
/* then sends an acknowledgment back */
      isync = 1;
      ierror = MPI_Bcast(&isync,1,MPI_INT,0,comm);
   }
   else {
/* remaining processors send a message to processor 0 */
      ierror = MPI_Send(&isync,1,MPI_INT,0,0,comm);
/* then receive an acknowledgement back */
      isync = 1;
      ierror = MPI_Bcast(&irync,1,MPI_INT,0,comm);
      if (irync != isync)
         fprintf(unit2,"rsync error at proc %d\n",rank);
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Barrier complete\n");
   return ierror;
}

int MPI_Reduce(void* sendbuf, void* recvbuf, int count,
               MPI_Datatype datatype, MPI_Op op, int root,
               MPI_Comm comm) {
/* applies a reduction operation to the vector sendbuf over the set of
   processes specified by comm and places the result in recvbuf on root
   sendbuf = address of send buffer
   recvbuf = address of receive buffer
   count = number of elements in send buffer
   datatype = datatype of elements in send buffer
   op = reduce operation (only max, min and sum currently supported)
   root = rank of root process
   comm = communicator
   input: sendbuf, count, datatype, op, root, comm
   output: recvbuf
local data                                                               */
   int ierror, i, j, np, rank, id, ltmp, loct, nl, lcnt, lsize;
   void *tmpbuf;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid root */
   else if ((root < 0) || (root >= nproc))
      ierror = 19;
/* invalid op */
   else if ((op < 0) || (op > 5) || (op==3))
      ierror = 20;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      id = mapcomm[comm][root];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid root */
      else if ((root < 0) || (root >= np))
         ierror = 19;
/* invalid mapping */
      else if ((id < 0) || (id >= nproc)) {
         fprintf(unit2,"Invalid mapping, root, node = %d,%d\n",root,id);
         ierror = 2;
      }
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Reduce: ",ierror);
      return ierror;
   }
/* determine size of temporary buffer */
   if ((datatype==MPI_INT) || (datatype==MPI_FLOAT))
      lsize = 4;
   else if (datatype==MPI_DOUBLE)
      lsize = 8;
   else if ((datatype==MPI_2INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 8;
   else if ((datatype==MPI_FLOAT_INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 8;
   else if ((datatype==MPI_DOUBLE_INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 16;
/* invalid datatype */
   else {
      ierror = 7;
      writerrs("MPI_Reduce: ",ierror);
      return ierror;
   }
   loct = 0;
   if (rank==root) {
/* initialize by copying from send to receive buffer */
      if (datatype==MPI_INT)
         iredux((int *)recvbuf,(int *)sendbuf,loct,count,-1);
      else if (datatype==MPI_FLOAT)
         fredux((float *)recvbuf,(float *)sendbuf,loct,count,-1);
      else if (datatype==MPI_DOUBLE)
         dredux((double *)recvbuf,(double *)sendbuf,loct,count,-1);
      else if (datatype==MPI_2INT)
         iredux((int *)recvbuf,(int *)sendbuf,loct,2*count,-1);
      else if (datatype==MPI_FLOAT_INT)
         fredux((float *)recvbuf,(float *)sendbuf,loct,2*count,-1);
   }
   ltmp = lsize*count;
/* allocate a nonrelocatable block of memory */
   tmpbuf = malloc(ltmp);
/* memory not available */
   if (!tmpbuf) {
      ierror = 21;
      writerrs("MPI_Reduce: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Reduce started\n");
   ltmp = ltmp/lsize;
/* send messages in groups of ltmp */
   nl = (count - 1)/ltmp + 1;
   lcnt = ltmp;
   lsize = lsize*ltmp/4;
   for (j = 0; j < nl; j++) {
/* better check to see if this is OK */
      if (j==(nl-1))
         lcnt = count - ltmp*(nl - 1);
      if (rank==root) {
/* root receives data from everyone else */
         for (i = 0; i < np; i++) {
            if (i != root) {
               ierror = MPI_Recv(tmpbuf,lcnt,datatype,i,j+1,comm,&status);
/* reduce data */
               if (datatype==MPI_INT)
                  iredux((int *)recvbuf,(int *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_FLOAT)
                  fredux((float *)recvbuf,(float *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_DOUBLE)
                  dredux((double *)recvbuf,(double *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_2INT)
                  iredux((int *)recvbuf,(int *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_FLOAT_INT)
                  fredux((float *)recvbuf,(float *)tmpbuf,loct,lcnt,op);
            }
         }
         loct = loct + ltmp;
      }
      else {
/* remaining processors send data to root */
         ierror = MPI_Send(&((int *)sendbuf)[loct],lcnt,datatype,root,j+1,comm);
         loct = loct + lsize;
      }
   }
/* release nonrelocatable memory block */
   free(tmpbuf);
   if (monitor==2)
      fprintf(unit2,"MPI_Reduce complete\n");
   return ierror;
}

int MPI_Scan(void* sendbuf, void* recvbuf, int count,
             MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
/* performs a parallel prefix reduction on data distributed across a
   group
   sendbuf = address of send buffer
   recvbuf = address of receive buffer
   count = number of elements in send buffer
   datatype = datatype of elements in send buffer
   op = reduce operation (only max, min and sum currently supported)
   comm = communicator
   input: sendbuf, count, datatype, op, comm
   output: recvbuf
local data                                                               */
   int ierror, i, j, np, rank, id, root = 0, ltmp, loct, nl, lcnt, lsize;
   void *tmpbuf;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid root */
   else if ((root < 0) || (root >= nproc))
      ierror = 19;
/* invalid op */
   else if ((op < 0) || (op > 5) || (op==3))
      ierror = 20;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      id = mapcomm[comm][root];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid root */
      else if ((root < 0) || (root >= np))
         ierror = 19;
/* invalid mapping */
      else if ((id < 0) || (id >= nproc)) {
         fprintf(unit2,"Invalid mapping, root, node = %d,%d\n",root,id);
         ierror = 2;
      }
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Scan: ",ierror);
      return ierror;
   }
/* determine size of temporary buffer */
   if ((datatype==MPI_INT) || (datatype==MPI_FLOAT))
      lsize = 4;
   else if (datatype==MPI_DOUBLE)
      lsize = 8;
   else if ((datatype==MPI_2INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 8;
   else if ((datatype==MPI_FLOAT_INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 8;
   else if ((datatype==MPI_DOUBLE_INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 16;
/* invalid datatype */
   else {
      ierror = 7;
      writerrs("MPI_Scan: ",ierror);
      return ierror;
   }
   loct = 0;
   if (rank==root) {
/* initialize by copying from send to receive buffer */
      if (datatype==MPI_INT)
         iredux((int *)recvbuf,(int *)sendbuf,loct,count,-1);
      else if (datatype==MPI_FLOAT)
         fredux((float *)recvbuf,(float *)sendbuf,loct,count,-1);
      else if (datatype==MPI_DOUBLE)
         dredux((double *)recvbuf,(double *)sendbuf,loct,count,-1);
      else if (datatype==MPI_2INT)
         iredux((int *)recvbuf,(int *)sendbuf,loct,2*count,-1);
      else if (datatype==MPI_FLOAT_INT)
         fredux((float *)recvbuf,(float *)sendbuf,loct,2*count,-1);
   }
   ltmp = lsize*count;
/* allocate a nonrelocatable block of memory */
   tmpbuf = malloc(ltmp);
/* memory not available */
   if (!tmpbuf) {
      ierror = 21;
      writerrs("MPI_Scan: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Scan started\n");
   ltmp = ltmp/lsize;
/* send messages in groups of ltmp */
   nl = (count - 1)/ltmp + 1;
   lcnt = ltmp;
   lsize = lsize*ltmp/4;
   for (j = 0; j < nl; j++) {
/* better check to see if this is OK */
      if (j==(nl-1))
         lcnt = count - ltmp*(nl - 1);
      if (rank==root) {
/* root receives data from everyone else */
         for (i = 0; i < np; i++) {
            if (i != root) {
               ierror = MPI_Recv(tmpbuf,lcnt,datatype,i,j+1,comm,&status);
/* reduce data */
               if (datatype==MPI_INT)
                  iredux((int *)recvbuf,(int *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_FLOAT)
                  fredux((float *)recvbuf,(float *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_DOUBLE)
                  dredux((double *)recvbuf,(double *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_2INT)
                  iredux((int *)recvbuf,(int *)tmpbuf,loct,lcnt,op);
               else if (datatype==MPI_FLOAT_INT)
                  fredux((float *)recvbuf,(float *)tmpbuf,loct,lcnt,op);
/* send partial result data to processor i */
               ierror = MPI_Send(&((int *)recvbuf)[loct],lcnt,datatype,i,
                                 j+nproc+1,comm);
            }
         }
         loct = loct + ltmp;
      }
      else {
/* remaining processors send data to root */
         ierror = MPI_Send(&((int *)sendbuf)[loct],lcnt,datatype,root,j+1,comm);
/* receive partial result data from root  */
         ierror = MPI_Recv(&((int *)recvbuf)[loct],lcnt,datatype,root,j+nproc+1,
                           comm,&status);
         loct = loct + lsize;
      }
   }
   if (rank==root) {
      loct = 0;
/* initialize by copying from send to receive buffer */
      if (datatype==MPI_INT)
         iredux((int *)recvbuf,(int *)sendbuf,loct,count,-1);
      else if (datatype==MPI_FLOAT)
         fredux((float *)recvbuf,(float *)sendbuf,loct,count,-1);
      else if (datatype==MPI_DOUBLE)
         dredux((double *)recvbuf,(double *)sendbuf,loct,count,-1);
      else if (datatype==MPI_2INT)
         iredux((int *)recvbuf,(int *)sendbuf,loct,2*count,-1);
      else if (datatype==MPI_FLOAT_INT)
         fredux((float *)recvbuf,(float *)sendbuf,loct,2*count,-1);
   }
/* release nonrelocatable memory block */
   free(tmpbuf);
   if (monitor==2)
      fprintf(unit2,"MPI_Scan complete\n");
   return ierror;
}

static int imax(int val1, int val2) {
   return (val1 > val2 ? val1 : val2);
}
     
static int imin(int val1, int val2) {
   return (val1 < val2 ? val1 : val2);
}
   
static float flmax(float val1, float val2) {
   return (val1 > val2 ? val1 : val2);
}
     
static float flmin(float val1, float val2) {
   return (val1 < val2 ? val1 : val2);
}
   
static double dmax(double val1, double val2) {
   return (val1 > val2 ? val1 : val2);
}
     
static double dmin(double val1, double val2) {
   return (val1 < val2 ? val1 : val2);
}

static void iredux(int *recvbuf, int *sendbuf, int offset, int count, MPI_Op op) {
/* perform reduction operation for int types
   recvbuf = address of receive buffer
   sendbuf = address of send buffer
   offset = starting location in receive buffer
   count = number of elements in send buffer
   op = reduce operation (only max, min and sum currently supported)
   input: recvbuf, sendbuf, offset, count, op
   output: recvbuf
local data                                                                 */
   int i, j, k;
/* perform reduction */
   if (op==MPI_MAX) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = imax(recvbuf[i+offset],sendbuf[i]);
      }
   }
   else if (op==MPI_MIN) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = imin(recvbuf[i+offset],sendbuf[i]);
      }
   }
   else if (op==MPI_SUM) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = recvbuf[i+offset] + sendbuf[i];
      }
   }
/* perform reduction and location */
   else if (op==MPI_MAXLOC) {
      for (i = 0; i < count; i++) {
         j = recvbuf[2*i+offset];
         k = sendbuf[2*i];
         if (j < k) {
            recvbuf[2*i+offset] = sendbuf[2*i];
            recvbuf[2*i+offset+1] = sendbuf[2*i+1];
         }
         else if (j==k)
            recvbuf[2*i+offset+1] = imin(recvbuf[2*i+offset+1],sendbuf[2*i+1]);
      }
   }
   else if (op==MPI_MINLOC) {
      for (i = 0; i < count; i++) {
         j = recvbuf[2*i+offset];
         k = sendbuf[2*i];
         if (j > k) {
            recvbuf[2*i+offset] = sendbuf[2*i];
            recvbuf[2*i+offset+1] = sendbuf[2*i+1];
         }
         else if (j==k)
            recvbuf[2*i+offset+1] = imax(recvbuf[2*i+offset+1],sendbuf[2*i+1]);
      }
   }
/* copy initial data */
   else if (op==(-1)) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = sendbuf[i];
      }
   }
   return;
}

static void fredux(float *recvbuf, float *sendbuf, int offset, int count, MPI_Op op) {
/* perform reduction operation for float types
   recvbuf = address of receive buffer
   sendbuf = address of send buffer
   offset = starting location in receive buffer
   count = number of elements in send buffer
   op = reduce operation (only max, min and sum currently supported)
   input: recvbuf, sendbuf, offset, count, op
   output: recvbuf
local data                                                                 */
   int i;
   float j, k;
/* perform reduction */
   if (op==MPI_MAX) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = flmax(recvbuf[i+offset],sendbuf[i]);
      }
   }
   else if (op==MPI_MIN) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = flmin(recvbuf[i+offset],sendbuf[i]);
      }
   }
   else if (op==MPI_SUM) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = recvbuf[i+offset] + sendbuf[i];
      }
   }
/* perform reduction and location */
   else if (op==MPI_MAXLOC) {
      for (i = 0; i < count; i++) {
         j = recvbuf[2*i+offset];
         k = sendbuf[2*i];
         if (j < k) {
            recvbuf[2*i+offset] = sendbuf[2*i];
            *((int *)&recvbuf[2*i+offset+1]) = *((int *)&sendbuf[2*i+1]);
         }
         else if (j==k)
            *((int *)&recvbuf[2*i+offset+1]) = imin(
            *((int *)&recvbuf[2*i+offset+1]),*((int *)&sendbuf[2*i+1]));
      }
   }
   else if (op==MPI_MINLOC) {
      for (i = 0; i < count; i++) {
         j = recvbuf[2*i+offset];
         k = sendbuf[2*i];
         if (j > k) {
            recvbuf[2*i+offset] = sendbuf[2*i];
            *((int *)&recvbuf[2*i+offset+1]) = *((int *)&sendbuf[2*i+1]);
         }
         else if (j==k)
            *((int *)&recvbuf[2*i+offset+1]) = imax(
            *((int *)&recvbuf[2*i+offset+1]),*((int *)&sendbuf[2*i+1]));
      }
   }
/* copy initial data */
   else if (op==(-1)) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = sendbuf[i];
      }
   }
   return;
}

static void dredux(double *recvbuf, double *sendbuf, int offset, int count, MPI_Op op) {
/* perform reduction operation for double types
   recvbuf = address of receive buffer
   sendbuf = address of send buffer
   offset = starting location in receive buffer
   count = number of elements in send buffer
   op = reduce operation (only max, min and sum currently supported)
   input: recvbuf, sendbuf, offset, count, op
   output: recvbuf
local data                                                                 */
   int i;
   double j, k;
/* perform reduction */
   if (op==MPI_MAX) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = dmax(recvbuf[i+offset],sendbuf[i]);
      }
   }
   else if (op==MPI_MIN) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = dmin(recvbuf[i+offset],sendbuf[i]);
      }
   }
   else if (op==MPI_SUM) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = recvbuf[i+offset] + sendbuf[i];
      }
   }
/* perform reduction and location */
   else if (op==MPI_MAXLOC) {
      for (i = 0; i < count; i++) {
         j = recvbuf[2*i+offset];
         k = sendbuf[2*i];
         if (j < k) {
            recvbuf[2*i+offset] = sendbuf[2*i];
            *((int *)&recvbuf[2*i+offset+1]) = *((int *)&sendbuf[2*i+1]);
         }
         else if (j==k)
            *((int *)&recvbuf[2*i+offset+1]) = imin(
            *((int *)&recvbuf[2*i+offset+1]),*((int *)&sendbuf[2*i+1]));
      }
   }
   else if (op==MPI_MINLOC) {
      for (i = 0; i < count; i++) {
         j = recvbuf[2*i+offset];
         k = sendbuf[2*i];
         if (j > k) {
            recvbuf[2*i+offset] = sendbuf[2*i];
            *((int *)&recvbuf[2*i+offset+1]) = *((int *)&sendbuf[2*i+1]);
         }
         else if (j==k)
            *((int *)&recvbuf[2*i+offset+1]) = imax(
            *((int *)&recvbuf[2*i+offset+1]),*((int *)&sendbuf[2*i+1]));
      }
   }
/* copy initial data */
   else if (op==(-1)) {
      for (i = 0; i < count; i++) {
         recvbuf[i+offset] = sendbuf[i];
      }
   }
   return;
}

int MPI_Allreduce(void* sendbuf, void* recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
/* applies a reduction operation to the vector sendbuf over the set of
   processes specified by comm and places result in recvbuf on all nodes
   sendbuf = address of send buffer
   recvbuf = address of receive buffer
   count = number of elements in send buffer
   datatype = datatype of elements in send buffer
   op = reduce operation (only max, min and sum currently supported)
   comm = communicator
   input: sendbuf, count, datatype, op, root, comm
   output: recvbuf
local data                                                               */
   int ierror, root = 0, ierr;
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   if (monitor==2)
      fprintf(unit2,"MPI_Allreduce started\n");
   ierror = MPI_Reduce(sendbuf,recvbuf,count,datatype,op,root,comm);
   ierr = MPI_Bcast(recvbuf,count,datatype,root,comm);
   if (monitor==2)
      fprintf(unit2,"MPI_Allreduce complete\n");
   return ierror;
}

int MPI_Gather(void* sendbuf, int sendcount, MPI_Datatype sendtype,
               void* recvbuf, int recvcount, MPI_Datatype recvtype,
               int root, MPI_Comm comm) {
/* collect individual messages from each process in comm at root
   sendbuf = starting address of send buffer
   sendcount = number of elements in send buffer
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcount = number of elements for any single receive
   recvtype = datatype of recv buffer elements
   root = rank of receiving process
   comm = communicator
   input: sendbuf, sendcount, sendtype, recvcount, recvtype, root, comm
   output: recvbuf
local data                                                               */
   int ierror, np, loct, lsize, id, rank, i, j;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid root */
   else if ((root < 0) || (root >= nproc))
      ierror = 19;
/* invalid count */
   else if (sendcount < 0)
      ierror = 3;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      id = mapcomm[comm][root];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid root */
      else if ((root < 0) || (root >= np))
         ierror = 19;
/* invalid mapping */
      else if ((id < 0) || (id >= nproc)) {
         fprintf(unit2,"Invalid mapping, root, node = %d,%d\n",root,id);
         ierror = 2;
      }
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Gather: ",ierror);
      return ierror;
   }
/* root receives data */
   if (rank==root) {
/* invalid count */
      if (recvcount < 0)
         ierror = 3;
/* determine size of data to be sent */
      if ((sendtype==MPI_INT) || (sendtype==MPI_FLOAT))
         loct = sendcount;
      else if (sendtype==MPI_DOUBLE)
         loct = 2*sendcount;
/* invalid datatype */
      else {
         loct = 0;
         ierror = 7;
      }
/* determine size of data to be received */
      if ((recvtype==MPI_INT) || (recvtype==MPI_FLOAT))
         lsize = recvcount;
      else if (recvtype==MPI_DOUBLE)
         lsize = 2*recvcount;
/* invalid datatype */
      else {
         lsize = 0;
         ierror = 7;
      }
/* unequal message length error */
      if (loct != lsize) {
         fprintf(unit2,"Unequal message length, send/receive bytes = %d,%d\n",
                loct,lsize);
         ierror = 22;
      }
/* handle count, datatype and length errors */
      if (ierror) {
         writerrs("MPI_Gather: ",ierror);
         return ierror;
      }
      if (monitor==2)
         fprintf(unit2,"MPI_Gather started\n");
      for (i = 0; i < np; i++) {
         loct = lsize*i;
/* root copies its own data directly */
         if (i==root) {
            for (j = 0; j < lsize; j++)
               ((int *)recvbuf)[j+loct] = ((int *)sendbuf)[j];
         }
/* otherwise, root receives data from other processors */
         else
            ierror = MPI_Recv(&((int *)recvbuf)[loct],recvcount,recvtype,i,1,comm,
                              &status);
      }
   }
/* processors other than root send data to root */
   else {
      if (monitor==2)
         fprintf(unit2,"MPI_Gather started\n");
      ierror = MPI_Send(sendbuf,sendcount,sendtype,root,1,comm);
   }
   if (monitor==2)
         fprintf(unit2,"MPI_Gather complete\n");
   return ierror;
}

int MPI_Allgather(void* sendbuf, int sendcount,
                  MPI_Datatype sendtype, void* recvbuf, int recvcount,
                  MPI_Datatype recvtype, MPI_Comm comm) {
/* gather individual messages from each process in comm and distribute
   the resulting message to each process.
   sendbuf = starting address of send buffer
   sendcount = number of elements in send buffer
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcount = number of elements for any process
   recvtype = datatype of receive buffer elements
   comm = communicator
   input: sendbuf, sendcount, sendtype, recvcount, recvtype, comm
   output: recvbuf
local data                                                               */
   int ierror, np, root = 0, ierr;
/* internal mpi common block
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   if (monitor==2)
      fprintf(unit2,"MPI_Allgather started\n");
   ierror = MPI_Gather(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,
                       root,comm);
   np = mapcomm[comm][MAXS];
   ierr = MPI_Bcast(recvbuf,np*recvcount,recvtype,root,comm);
   if (monitor==2)
      fprintf(unit2,"MPI_Allgather complete\n");
   return ierror;
}

int MPI_Scatter(void* sendbuf, int sendcount, MPI_Datatype sendtype,
                void* recvbuf, int recvcount, MPI_Datatype recvtype,
                int root, MPI_Comm comm) {
/* distribute individual messages from root to each process in comm
   sendbuf = starting address of send buffer
   sendcount = number of elements in send buffer
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcount = number of elements for any single receive
   recvtype = datatype of recv buffer elements
   root = rank of sending process
   comm = communicator
   input: sendbuf, sendcount, sendtype, recvcount, recvtype, root, comm
   output: recvbuf
local data                                                               */
   int ierror, np, lsize, loct, id, rank, i, j;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid root */
   else if ((root < 0) || (root >= nproc))
      ierror = 19;
/* invalid count */
   else if (recvcount < 0)
      ierror = 3;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      id = mapcomm[comm][root];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid root */
      else if ((root < 0) || (root >= np))
         ierror = 19;
/* invalid mapping */
      else if ((id < 0) || (id >= nproc)) {
         fprintf(unit2,"Invalid mapping, root, node = %d,%d\n",root,id);
         ierror = 2;
      }
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Scatter: ",ierror);
      return ierror;
   }
/* root sends data */
   if (rank==root) {
/* invalid counts */
      if (sendcount < 0)
         ierror = 3;
/* determine size of data to be sent */
      if ((sendtype==MPI_INT) || (sendtype==MPI_FLOAT))
         lsize = sendcount;
      else if (sendtype==MPI_DOUBLE)
         lsize = 2*sendcount;
/* invalid datatype */
      else {
         lsize = 0;
         ierror = 7;
      }
/* determine size of data to be received */
      if ((recvtype==MPI_INT) || (recvtype==MPI_FLOAT))
         loct = recvcount;
      else if (recvtype==MPI_DOUBLE)
         loct = 2*recvcount;
/* invalid datatype */
      else {
         loct = 0;
         ierror = 7;
      }
/* unequal message length error */
      if (loct != lsize) {
         fprintf(unit2,"Unequal message length, send/receive bytes = %d,%d\n",
                lsize,loct);
         ierror = 22;
      }
/* handle count, datatype and length errors */
      if (ierror) {
         writerrs("MPI_Scatter: ",ierror);
         return ierror;
      }
      if (monitor==2)
         fprintf(unit2,"MPI_Scatter started\n");
      for (i = 0; i < np; i++) {
         loct = lsize*i;
/* root copies its own data directly */
         if (i==root) {
            for (j = 0; j < lsize; j++)
              ((int *)recvbuf)[j] = ((int *)sendbuf)[j+loct];
         }
/* otherwise, root sends data to other processors */
         else
            ierror = MPI_Send(&((int *)sendbuf)[loct],sendcount,sendtype,i,1,comm);
      }
   }
/* processors other than root receive data from root */
   else {
      if (monitor==2)
         fprintf(unit2,"MPI_Scatter started\n");
      ierror = MPI_Recv(recvbuf,recvcount,recvtype,root,1,comm,&status);
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Scatter complete\n");
   return ierror;
}

int MPI_Alltoall(void* sendbuf, int sendcount, MPI_Datatype sendtype,
                 void* recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm) {
/* send a distinct message from each process to every process
   sendbuf = starting address of send buffer
   sendcount = number of elements in send buffer
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcount = number of elements for any single receive
   recvtype = datatype of recv buffer elements
   comm = communicator
   input: sendbuf, sendcount, sendtype, recvcount, recvtype, comm
   output: recvbuf
local data                                                               */
   int ierror, np, loct, lsize, id, rank, i, j;
   MPI_Request request;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid counts */
   else if ((sendcount < 0) || (recvcount < 0))
      ierror = 3;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Alltoall: ",ierror);
      return ierror;
   }
/* determine size of data to be sent */
   if ((sendtype==MPI_INT) || (sendtype==MPI_FLOAT))
      loct = sendcount;
   else if (sendtype==MPI_DOUBLE)
      loct = 2*sendcount;
/* invalid datatype */
   else {
      loct = 0;
      ierror = 7;
   }
/* determine size of data to be received */
   if ((recvtype==MPI_INT) || (recvtype==MPI_FLOAT))
      lsize = recvcount;
   else if (recvtype==MPI_DOUBLE)
      lsize = 2*recvcount;
/* invalid datatype */
   else {
      lsize = 0;
      ierror = 7;
   }
/* unequal message length error */
   if (loct != lsize) {
      fprintf(unit2,"Unequal message length, send/receive bytes = %d,%d\n",
             loct,lsize);
      ierror = 22;
   }
/* handle count, datatype and length errors */
   if (ierror) {
      writerrs("MPI_Alltoall: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Alltoall started\n");
   for (i = 0; i < np; i++) {
      id = i - rank;
      if (id < 0)
         id += np;
      loct = lsize*id;
/* each node copies its own data directly */
      if (rank==id) {
         for (j = 0; j < lsize; j++)
            ((int *)recvbuf)[j+loct] = ((int *)sendbuf)[j+loct];
      }
/* otherwise, each node receives data from other nodes */
      else {
         ierror = MPI_Irecv(&((int *)recvbuf)[loct],recvcount,recvtype,id,i+1,comm,
                            &request) ;
         ierror = MPI_Send(&((int *)sendbuf)[loct],sendcount,sendtype,id,i+1,comm);
         ierror = MPI_Wait(&request,&status);
      }
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Alltoall complete\n");
   return ierror;
}

int MPI_Gatherv(void* sendbuf, int sendcount, MPI_Datatype sendtype,
                void* recvbuf, int *recvcounts, int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm) {
/* collect individual messages from each process in comm at root
   messages can have different sizes and displacements
   sendbuf = starting address of send buffer
   sendcount = number of elements in send buffer
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcounts = integer array
   displs = integer array of displacements
   recvtype = datatype of recv buffer elements
   root = rank of receiving process
   comm = communicator
   input: sendbuf, sendcount, sendtype, recvcounts, displs, recvtype
   input: root, comm
   output: recvbuf
local data                                                               */
   int ierror, np, loct, lsize, id, rank, i, j;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid root */
   else if ((root < 0) || (root >= nproc))
      ierror = 19;
/* invalid count */
   else if (sendcount < 0)
      ierror = 3;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      id = mapcomm[comm][root];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid root */
      else if ((root < 0) || (root >= np))
         ierror = 19;
/* invalid mapping */
      else if ((id < 0) || (id >= nproc)) {
         fprintf(unit2,"Invalid mapping, root, node = %d,%d\n",root,id);
         ierror = 2;
      }
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Gatherv: ",ierror);
      return ierror;
   }
/* root receives data */
   if (rank==root) {
/* invalid counts */
      for (i = 0; i < np; i++) {
         if (recvcounts[i] < 0)
            ierror = 3;
      }
/* determine size of data to be sent */
      if ((sendtype==MPI_INT) || (sendtype==MPI_FLOAT))
         loct = 1;
      else if (sendtype==MPI_DOUBLE)
         loct = 2;
/* invalid datatype */
      else {
         ierror = 7;
      }
/* determine size of data to be received */
      if ((recvtype==MPI_INT) || (recvtype==MPI_FLOAT))
         lsize = 1;
      else if (recvtype==MPI_DOUBLE)
         lsize = 2;
/* invalid datatype */
      else {
         ierror = 7;
      }
/* unequal message length error */
      id = lsize*recvcounts[rank];
      if ((!ierror) && (loct*sendcount != id)) {
         fprintf(unit2,"Unequal self message, send/receive bytes = %d,%d\n",
                loct*sendcount,id);
         ierror = 22;
      }
/* handle count, datatype and length errors */
      if (ierror) {
         writerrs("MPI_Gatherv: ",ierror);
         return ierror;
      }
      if (monitor==2)
         fprintf(unit2,"MPI_Gatherv started\n");
      for (i = 0; i < np; i++) {
         loct = lsize*displs[i];
/* root copies its own data directly */
         if (i==root) {
            for (j = 0; j < lsize*recvcounts[i]; j++)
               ((int *)recvbuf)[j+loct] = ((int *)sendbuf)[j];
         }
/* otherwise, root receives data from other processors */
         else {
            if (monitor==2)
               fprintf(unit2,"MPI_Gatherv started\n");
            ierror = MPI_Recv(&((int *)recvbuf)[loct],recvcounts[i],recvtype,
                              i,1,comm,&status);
         }
      }
   }
/* processors other than root send data to root */
   else
      ierror = MPI_Send(sendbuf,sendcount,sendtype,root,1,comm);
   if (monitor==2)
      fprintf(unit2,"MPI_Gatherv complete\n");
   return ierror;
}

int MPI_Allgatherv(void* sendbuf, int sendcount,
                   MPI_Datatype sendtype, void* recvbuf, int *recvcounts,
                   int *displs, MPI_Datatype recvtype, MPI_Comm comm) {
/* gather individual messages from each process in comm and distribute
   the resulting message to each process.
   messages can have different sizes and displacements
   sendbuf = starting address of send buffer
   sendcount = number of elements in send buffer
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcounts = integer array
   displs = integer array of displacements
   recvtype = datatype of receive buffer elements
   comm = communicator
   input: sendbuf, sendcount, sendtype, recvcounts, displs, recvtype
   input: comm
   output: recvbuf
local data                                                               */
   int ierror, np, i, ierr;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* communicator errors */
   if ((comm >= 0) && (comm < MAXC)) {
      np = mapcomm[comm][MAXS];
      if ((np <= 0) || (np > nproc))
         ierror = 2;
   }
   else
      ierror = 2;
/* handle errors */
   if (ierror) {
      writerrs("MPI_Allgatherv: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Allgatherv started\n");
   for (i = 0; i < np; i++) {
      ierr = MPI_Gatherv(sendbuf,sendcount,sendtype,recvbuf,recvcounts,displs,
                         recvtype,i,comm);
      if (ierr)
         ierror = ierr;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Allgatherv complete\n");
   return ierror;
}

int MPI_Scatterv(void* sendbuf, int *sendcounts, int *displs,
                 MPI_Datatype sendtype, void* recvbuf, int recvcount,
                 MPI_Datatype recvtype, int root, MPI_Comm comm) {
/* distribute individual messages from root to each process in comm
   messages can have different sizes and displacements
   sendbuf = starting address of send buffer
   sendcounts = integer array
   displs = integer array of displacements
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcount = number of elements for any single receive
   recvtype = datatype of recv buffer elements
   root = rank of sending process
   comm = communicator
   input: sendbuf, sendcounts, displs, sendtype, recvcount, recvtype
   input: root, comm
   output: recvbuf
local data                                                               */
   int ierror, np, lsize, loct, id, rank, i, j;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* invalid root */
   else if ((root < 0) || (root >= nproc))
      ierror = 19;
/* invalid counts */
   else if (recvcount < 0)
      ierror = 3;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
      id = mapcomm[comm][root];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* invalid root */
      else if ((root < 0) || (root >= np))
         ierror = 19;
/* invalid mapping */
      else if ((id < 0) || (id >= nproc)) {
         fprintf(unit2,"Invalid mapping, root, node = %d,%d\n",root,id);
         ierror = 2;
      }
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Scatterv: ",ierror);
      return ierror;
   }
/* root sends data */
   if (rank==root) {
/* invalid counts */
      for (i = 0; i < np; i++) {
         if (sendcounts[i] < 0)
            ierror = 3;
      }
/* determine size of data to be sent */
      if ((sendtype==MPI_INT) || (sendtype==MPI_FLOAT))
         lsize = 1;
      else if (sendtype==MPI_DOUBLE)
         lsize = 2;
/* invalid datatype */
      else {
         ierror = 7;
      }
/* determine size of data to be received */
      if ((recvtype==MPI_INT) || (recvtype==MPI_FLOAT))
         loct = 1;
      else if (recvtype==MPI_DOUBLE)
         loct = 2;
/* invalid datatype */
      else {
         ierror = 7;
      }
/* unequal message length error */
      id = lsize*sendcounts[rank];
      if ((!ierror) && (loct*recvcount != id)) {
         fprintf(unit2,"Unequal self message, send/receive bytes = %d,%d\n",
                id,loct*recvcount);
         ierror = 22;
      }
/* handle count, datatype and length errors */
      if (ierror) {
         writerrs("MPI_Scatterv: ",ierror);
         return ierror;
      }
      if (monitor==2)
         fprintf(unit2,"MPI_Scatterv started\n");
      for (i = 0; i < np; i++) {
         loct = lsize*displs[i];
/* root copies its own data directly */
         if (i==root) {
            for (j = 0; j < lsize*sendcounts[i]; j++)
              ((int *)recvbuf)[j] = ((int *)sendbuf)[j+loct];
         }
/* otherwise, root sends data to other processors */
         else
            ierror = MPI_Send(&((int *)sendbuf)[loct],sendcounts[i],sendtype,
                              i,1,comm);
      }
   }
/* processors other than root receive data from root */
   else {
      if (monitor==2)
         fprintf(unit2,"MPI_Scatterv started\n");
      ierror = MPI_Recv(recvbuf,recvcount,recvtype,root,1,comm,&status);
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Scatterv complete\n");
   return ierror;
}

int MPI_Alltoallv(void* sendbuf, int *sendcounts, int *sdispls,
                  MPI_Datatype sendtype, void* recvbuf, int *recvcounts,
                  int *rdispls, MPI_Datatype recvtype, MPI_Comm comm) {
/* send a distinct message from each process to every process
   messages can have different sizes and displacements
   sendbuf = starting address of send buffer
   sendcounts = integer array
   sdispls = integer array of send displacements
   sendtype = datatype of send buffer elements
   recvbuf = address of receive buffer
   recvcounts = integer array
   rdispls = integer array of receive displacements
   recvtype = datatype of recv buffer elements
   comm = communicator
   input: sendbuf, sendcount, sendtype, recvcount, recvtype, comm
   output: recvbuf
local data                                                               */
   int ierror, np, locs, msize, loct, lsize, id, ld, rank, i, j;
   MPI_Request request;
   MPI_Status status;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* check for error conditions */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* invalid comm */
   else if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* invalid counts */
   for (i = 0; i < np; i++) {
      if ((sendcounts[i] < 0) || (recvcounts[i] < 0))
         ierror = 3;
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Alltoallv: ",ierror);
      return ierror;
   }
/* determine size of data to be sent */
   if ((sendtype==MPI_INT) || (sendtype==MPI_FLOAT))
      msize = 1;
   else if (sendtype==MPI_DOUBLE)
      msize = 2;
/* invalid datatype */
   else {
      ierror = 7;
   }
/* determine size of data to be received */
   if ((recvtype==MPI_INT) || (recvtype==MPI_FLOAT))
      lsize = 1;
   else if (recvtype==MPI_DOUBLE)
      lsize = 2;
/* invalid datatype */
   else {
      ierror = 7;
   }
/* unequal message length error */
   id = msize*sendcounts[rank];
   ld = lsize*recvcounts[rank];
   if ((!ierror) && (id != ld)) {
      fprintf(unit2,"Unequal self message length, send/receive bytes = %d,%d\n",
             id,ld);
      ierror = 22;
   }
/* handle count, datatype and length errors */
   if (ierror) {
      writerrs("MPI_Alltoallv: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Alltoallv started\n");
   for (i = 0; i < np; i++) {
      id = i - rank;
      if (id < 0)
         id += np;
      locs = msize*sdispls[id];
      loct = lsize*rdispls[id];
/* each node copies its own data directly */
      if (rank==id) {
         for (j = 0; j < lsize*recvcounts[id]; j++)
            ((int *)recvbuf)[j+loct] = ((int *)sendbuf)[j+locs];
      }
/* otherwise, each node receives data from other nodes */
      else {
         ierror = MPI_Irecv(&((int *)recvbuf)[loct],recvcounts[id],recvtype,id,
                            i+1,comm,&request) ;
         ierror = MPI_Send(&((int *)sendbuf)[locs],sendcounts[id],sendtype,id,
                           i+1,comm);
         ierror = MPI_Wait(&request,&status);
      }
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Alltoallv complete\n");
   return ierror;
}

int MPI_Reduce_scatter(void* sendbuf, void* recvbuf, int *recvcounts,
                       MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
/* applies a reduction operation to the vector sendbuf over the set of
   processes specified by comm and scatters the result according to the
   values in recvcounts
   sendbuf = starting address of send buffer
   recvbuf = starting address of receive buffer
   recvcounts = integer array
   datatype = datatype of elements in input buffer
   op = reduce operation (only max, min and sum currently supported)
   comm = communicator
   input: sendbuf, recvcounts, datatype, op, comm
   output: recvbuf
local data                                                               */
   int ierror, np, rank, root = 0, count, lsize, ltmp, i;
   int *displs;
   void *tmpbuf;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   mapcomm = communicator map                                            */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   ierror = 0;
/* invalid comm */
   if ((comm < 0) || (comm >= MAXC))
      ierror = 2;
/* communicator errors */
   else {
      np = mapcomm[comm][MAXS];
/* communicator not mapped */
      if ((np <= 0) || (np > nproc))
         ierror = 2;
/* get rank */
      else {
         rank = mapcomm[comm][MAXS+1];
         if ((rank >= 0) && (rank < np)) {
            if (mapcomm[comm][rank] != idproc)
               ierror = 29;
         }
         else
            ierror = 29;
      }
   }
/* handle errors */
   if (ierror) {
      writerrs("MPI_Reduce_scatter: ",ierror);
      return ierror;
   }
   ltmp = 4*np;
/* allocate a nonrelocatable block of memory */
   displs = (int *)malloc(ltmp);
/* memory not available */
   if (!displs) {
      ierror = 21;
      writerrs("MPI_Reduce_scatter: ",ierror);
      return ierror;
   }
   count = 0;
/* determines overall count and displacements */
   for (i = 0; i < np; i++) {
      displs[i] = count;
      count = count + recvcounts[i];
   }
/* determine size of temporary buffer */
   if ((datatype==MPI_INT) || (datatype==MPI_FLOAT))
      lsize = 4;
   else if (datatype==MPI_DOUBLE)
      lsize = 8;
   else if ((datatype==MPI_2INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 8;
   else if ((datatype==MPI_FLOAT_INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 8;
   else if ((datatype==MPI_DOUBLE_INT) && ((op==MPI_MAXLOC) || (op==MPI_MINLOC)))
      lsize = 16;
/* invalid datatype */
   else {
      ierror = 7;
      writerrs("MPI_Reduce_scatter: ",ierror);
      return ierror;
   }
   if (monitor==2)
      fprintf(unit2,"MPI_Reduce_scatter started\n");
   ltmp = lsize*count;
/* allocate a nonrelocatable block of memory */
   tmpbuf = malloc(ltmp);
/* memory not available */
   if (!tmpbuf) {
      ierror = 21;
      writerrs("MPI_Reduce_scatter: ",ierror);
      return ierror;
   }
   ierror = MPI_Reduce(sendbuf,tmpbuf,count,datatype,op,root,comm);
   ierror = MPI_Scatterv(tmpbuf,recvcounts,displs,datatype,recvbuf,
                         recvcounts[rank],datatype,root,comm);
/* release nonrelocatable memory block */
   free(tmpbuf);
/* release nonrelocatable memory block */
   free(displs);
   if (monitor==2)
      fprintf(unit2,"MPI_Reduce_scatter complete\n");
   return ierror;
}

int MPI_Abort(MPI_Comm comm, int errorcode) {
/* force all tasks on an MPI environment to terminate
   comm = communicator
   errorcode = error code to return to invoking environment
   input: comm, errorcode
local data                                                              */
   int ierror;
/* internal mpi common block
   nproc = number of real or virtual processors obtained                */
/* MPI not initialized        */
   if (nproc <= 0)
      ierror = 1;
/* this is just a temporary patch, have not yet notified everyone else  */
   else
      ierror = MPI_Finalize();
   return ierror;
}

double MPI_Wtime(void) {
/* return an elapsed time on the calling processor in seconds */
   long oss, usec;
   const double tick=1.0/1000000.0;
   double time;
   struct timeval ptime;
/* internal mpi common block
   stime = first time stamp if MPI_Init successful */
/* calculate time elapsed in microseconds */
   oss = gettimeofday(&ptime,NULL);
   if (oss < 0)
      return 0.;
   usec = ptime.tv_usec - stime.tv_usec;
   time = (ptime.tv_sec - stime.tv_sec) + usec*tick;
   return time;
}

double MPI_Wtick(void) {
/* return the resolution of MPI_Wtime in seconds */
   const double tick=1.0/1000000.0;
   return tick;
}

int MPI_Type_extent(MPI_Datatype datatype, MPI_Aint *extent) {
/* returns the size of a datatype
   datatype = datatype
   extent = datatype extent
   input: dataype
   output: extent
local data                                                              */
   int ierror;
   ierror = 0;
/* find size of datatype */
   if ((datatype==MPI_INT) || (datatype==MPI_FLOAT))
      *extent = 4;
   else if (datatype==MPI_DOUBLE)
      *extent = 8;
   else if (datatype==MPI_BYTE)
      *extent = 1;
   else if ((datatype==MPI_2INT) || (datatype==MPI_FLOAT_INT)) 
      *extent = 8;
   else if (datatype==MPI_DOUBLE_INT)
      *extent = 16;
/* invalid datatype */
   else {
      ierror = 7;
      fprintf(unit2,"MPI_Type_extent: Invalid datatype\n");
   }
   return ierror;
}

int MPI_Get_processor_name(char *name,int *resultlen) {
/* returns the name of the processor on which it was called
   name = a unique specifier for the current physical node
   resultlen = length of the result returned in name
   input: none
   output: name, resultlen  
local data                                                  */
   int ierror, nv;
   struct utsname uinfo;
   struct hostent *info;
   struct in_addr address;
   long oss;
   char myself[36], ename[8];
/* declare internal mpi common block
   idproc = processor id              */
   ierror = 0;
/* Convert processor id to printable integer */
   sprintf(ename,"%d",idproc);
/* Obtain information about the Internet environment */
   oss = uname(&uinfo);
   if (oss < 0) {
      ierror = errno;
      fprintf(unit2,"MPI_Get_processor_name: Uname Error");
      return ierror;
   }
   info = gethostbyname(uinfo.nodename);
   if (info==NULL) {
      ierror = h_errno;
      fprintf(unit2,"MPI_Get_processor_name: Gethostbyname Error");
   }
   address.s_addr = *(in_addr_t *) ((info->h_addr_list)[0]);
   strcpy(myself,inet_ntoa(address));
   strcat(myself,":");
   strcat(myself,ename);
   nv = imin(strlen(myself),MPI_MAX_PROCESSOR_NAME);
   strncpy(name,myself,nv+1);
   *resultlen = nv;
   return ierror;
}

void writerrs(char *source, int ierror) {
/* this subroutine writes out error descriptions from error codes
   source = source subroutine of error message
   ierror = error indicator
   input: source, ierror
local data                                                         */
   int ierr, fatal=1;
/* check error code and print corresponding message */
   if (ierror==1)
      fprintf(unit2,"%s MPI not initialized\n",source);
   else if (ierror==2)
      fprintf(unit2,"%s Invalid Communicator\n",source);
   else if (ierror==3)
      fprintf(unit2,"%s Invalid count\n",source);
   else if (ierror==4)
      fprintf(unit2,"%s Invalid destination\n",source);
   else if (ierror==5)
      fprintf(unit2,"%s Invalid source\n",source);
   else if (ierror==6)
      fprintf(unit2,"%s Invalid tag\n",source);
   else if (ierror==7)
      fprintf(unit2,"%s Invalid datatype\n",source);
   else if (ierror==12)
      fprintf(unit2,"%s Incomplete read\n",source);
   else if (ierror==16)
      fprintf(unit2,"%s Invalid request handle\n",source);
   else if (ierror==18)
      fprintf(unit2,"%s Mismatched dataype\n",source);
   else if (ierror==19)
      fprintf(unit2,"%s Invalid root\n",source);
   else if (ierror==20)
      fprintf(unit2,"%s Invalid operation\n",source);
   else if (ierror==21)
      fprintf(unit2,"%s Unable to allocate memory\n",source);
   else if (ierror==29)
      fprintf(unit2,"%s Invalid rank or communicator\n",source);
   else if (ierror==(-1))
      fprintf(unit2,"%s Orderly disconnect request received\n",source);
   else if (ierror==(-2))
      fprintf(unit2,"%s Disorderly disconnect request received\n",source);
/* unlisted error code */
   else
      fprintf(unit2,"%s Error code = %d\n",source,ierror);
/* abort if error is fatal */
   if (fatal) {
      fflush(unit2);
      ierr = MPI_Abort(MPI_COMM_WORLD,ierror);
      exit(0);
   }
}

void rwstat(int request, FILE *unit) {
/* this subroutine writes out the status of a read-write record
   request = request handle
   unit = output file pointer
   input: request, unit                                         */
/* internal common block for non-blocking messages
   rwrec = read/write record for asynchronous messages */
/* invalid request handle */
   if ((request < 0) || (request >= MAXM))
      return;
   fprintf(unit,"\n");
   if (curreq[request][0]==(-1))
      fprintf(unit," transmission mode = send\n");
   else if (curreq[request][0]==1)
      fprintf(unit," transmission mode = receive\n");
   fprintf(unit," destination/source = %d\n",curreq[request][1]);
   fprintf(unit," communicator = %d\n",curreq[request][2]);
   fprintf(unit," tag = %d\n",curreq[request][3]);
   fprintf(unit," datatype = %d\n",curreq[request][4]);
   fprintf(unit," request handle = %d\n",request);
   fprintf(unit," Endpoint reference pointer = %p\n",(void*) rwrec[request].ref);
   fprintf(unit," iocompletion flag = %d\n",rwrec[request].ioflag);
   fprintf(unit," current buffer pointer = %p\n",rwrec[request].buf);
   fprintf(unit," current buffer length = %ld\n",rwrec[request].nbytes);
   fprintf(unit," T_MORE flag = %d\n",rwrec[request].flags);
   fprintf(unit," data pointer = %p\n",rwrec[request].sbuf);
   fprintf(unit," data length = %d\n",rwrec[request].sln);
   fprintf(unit," actual length sent/received = %d\n",rwrec[request].len);
   fprintf(unit," first time stamp = %d,%d\n",rwrec[request].ts[0].tv_sec,
           rwrec[request].ts[0].tv_usec);
   fprintf(unit," second time stamp = %d,%d\n",rwrec[request].ts[1].tv_sec,
           rwrec[request].ts[1].tv_usec);
   fprintf(unit," next message pointer = %d\n",rwrec[request].nextm);
   fprintf(unit," non-fatal error code = %d\n",rwrec[request].nfatal);
   fprintf(unit," comm in header = %d\n",header[request].hdata[0]);
   fprintf(unit," tag in header = %d\n",header[request].hdata[1]);
   fprintf(unit," type in header = %d\n",header[request].hdata[2]);
   fprintf(unit," length in header = %d\n",header[request].hdata[3]);
   fprintf(unit,"\n");
   return;
}

void wqueue(FILE *unit) {
/* this subroutine writes queue of pending messages 
   unit = output file pointer
   input: iunit
local data                                                */
   int i, is = 0;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id
   ioc = array of context pointers for notifier function  */
/* internal common block for non-blocking messages
   mqueue = message request queue                         */
   for (i = 0; i < nproc; i++) {
      if (ioc[i][2]) {
         fprintf(unit,"Incomplete receive: node, handle =%d,%d\n",
                i,ioc[i][2]);
         is += 1;
      }
      if (mqueue[i][0]) {
         fprintf(unit,"Non-empty receive queue end: node, handle =%d,%d\n",
                i,mqueue[i][0]);
         is += 1;
      }
      if (ioc[i][3]) {
         fprintf(unit,"Incomplete send: node, handle =%d,%d\n",
                i,ioc[i][3]);
         is += 1;
      }
      if (mqueue[i][1]) {
         fprintf(unit,"Non-empty send queue end: node, handle =%d,%d\n",
                i,mqueue[i][1]);
         is += 1;
      }
   }
   i = MAXS;
   if (ioc[i][3]) {
      fprintf(unit,"Incomplete selfsend: node, handle =%d,%d\n",
             idproc,ioc[i][3]);
      is += 1;
   }
   if (mqueue[i][1]) {
      fprintf(unit,"Non-empty selfsend queue end: node, handle =%d,%d\n",
             idproc,mqueue[i][1]);
      is += 1;
   }
   if (is > 0)
      fprintf(unit,"\n");
   return;
}

void messwin(int nvp) {
/* this subroutine creates a window for showing MPI message status
   nvp = number of real or virtual processors
   input argument: nvp                                        */
/* display status */
   logmess(0,0,0,0,0);
   return;
}

void logmess(int idp, int lstat, int lsize, int mticks, int tag) {
/* this subroutine logs MPI message state change and displays status
   idp = remote processor id
   lstat = (-1,1,-2,2) = (clear send,add send,clear receive,add receive)
   lstat = 0 means display current status for all processors
   lsize = size of message (in bytes)
   lsize = -1 means print out message size distribution function
   mticks = wait time in microseconds
   tag = message tag
   input argument: idp, lstat, lsize, mticks, tag
local data                                                               */
#define NDSIZE               24
   int istat, istyle, i, i1, i2;
   float ar, cr, at, ct;
   double dt;
/* mstat = number of outstanding sends and receives for each node
   msize = message size distribution function
   nmax = maximum number of messages in display
   lmax = log2 of nmax
   ssize/times = total bytes/time sent
   rsize/timer = total bytes/time received
   ptime = time since last short term average                     */
   static int mstat[MAXS][2] = {{0,0}}, msize[2*NDSIZE] = {0};
   static int lmax = 8, nmax = 256, nums = 0, numr = 0;
   static double ssize[2] = {0.0,0.0}, times[2] = {0.0,0.0};
   static double rsize[2] = {0.0,0.0}, timer[2] = {0.0,0.0};
   static double ptime = 0.0;
/* internal mpi common block
   nproc = number of real or virtual processors obtained
   idproc = processor id                                                 */
/* internal common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
/* check for errors */
   if ((idp < 0) || (idp >= nproc))
      return;
/* print out message size distribution function */
   if (lsize < 0) {
      fprintf(unit2," Message Size Distribution Function\n");
      i1 = 1;
      for (i = 0; i < NDSIZE; i++) {
         i2 = i + NDSIZE;
         fprintf(unit2," Size(bytes) = %d Sends = %d Receives = %d\n",
                i1,msize[i2],msize[i]-msize[i2]);
         i1 = 2*i1;
      }
      dt = 1.0/MPI_Wtime();
      i1 = 100.0*(float) (times[0]*dt) + .5;
      ar = (float) (ssize[0]/times[0]);
      fprintf(unit2," Sending Time = %d %% Speed = %f MB/s\n",i1,ar);
      i1 = 100.0*(float) (timer[0]*dt) + .5;
      ar = (float) (rsize[0]/timer[0]);
      fprintf(unit2," Receiving Time = %d %% Speed = %f MB/s\n",i1,ar);
      return;
   }
/* process message size and time data */
   else if ((lstat < 0) && (lsize > 0)) {
/* accumulate sending data */
      if (lstat==(-1)) {
         dt = 1.0e-6*(double) lsize;
         ssize[0] += dt;
         ssize[1] += dt;
         dt = 1.0e-6*(double) mticks;
         times[0] += dt;
         times[1] += dt;
/* calculate short term average */
         nums += 1;
         if (nums==4) {
            nums = 0;
            ssize[1] = 0.0;
            times[1] = 0.0;
         }
      }
/* accumulate receiving data */
      else if (lstat==(-2)) {
         dt = 1.0e-6*(double) lsize;
         rsize[0] += dt;
         rsize[1] += dt;
         dt = 1.0e-6*(double) mticks;
         timer[0] += dt;
         timer[1] += dt;
/* calculate short term average */
         numr += 1;
         if (numr==4) {
            dt = MPI_Wtime();
            ct = (float) ((int) (100.0*(float) ((times[0]+timer[0])/dt) + .5));
            dt = dt - ptime;
            at = (float) ((int) (100.0*(float) ((times[1]+timer[1])/dt) + .5));
            ptime = MPI_Wtime();
            cr = (float) (rsize[0]/timer[0]);
            ar = (float) (rsize[1]/timer[1]);
            shospeed(at,ct,ar,cr);
            numr = 0;
            rsize[1] = 0.0;
            timer[1] = 0.0;
         }
      }
/* increment message size distribution function */
      i1 = imin((int) (log((float) (lsize))/log(2.) + .5),NDSIZE-1);
      msize[i1] += 1;
/* increment message size distribution function for sends */
      i2 = i1 + NDSIZE;
      if (lstat==(-1))
         msize[i2] += 1;
      if (msize[i1] > nmax) {
/* erase display of distribution function */
         showdism(0,nmax,0,lmax,0);
         lmax += 1;
         nmax += nmax;
/* redisplay distribution function */
         for (i = 0; i < NDSIZE; i++) {
            showdism(i,msize[i],msize[i+NDSIZE],lmax,1);
         }
      }
/* display distribution function */
      else
         showdism(i1,msize[i1],msize[i2],lmax,1);
   }
   i1 = idp;
/* calculate all current statuses */
   if (lstat==0) {
      for (i = 0; i < nproc; i++) {
         istat = 0;
         if (mstat[i][0] >= 1)
            istat += 1;
         if (mstat[i][1] >= 1)
            istat += 2;
/* differentiate single from multiple sends/receives */
/*       if ((mstat[i][0] > 1) || (mstat[i][1] > 1))
            istat += 3;                              */
/* display status, outline local node                */
         if (i==idproc)
            istyle = 1;
         else
            istyle = 0;
         showmess(i,istat,istyle);
      }
/* display scale */
      showdism(0,msize[0],msize[NDSIZE],lmax,0);
/* display current distribution function */
      for (i = 0; i < NDSIZE; i++) {
         showdism(i,msize[i],msize[i+NDSIZE],lmax,1);
      }
      return;
   }
/* add state change to log */
   else if (lstat==1) {
      mstat[i1][0] += 1;
      if (monitor==2)
         fprintf(unit2,"send posted: destination= %d size= %d tag= %d\n",
                 idp,lsize,tag);
   }
   else if (lstat==(-1)) {
      mstat[i1][0] -= 1;
      if (monitor==2)
         fprintf(unit2,"sent: destination= %d size= %d time= %d tag= %d\n",
                 idp,lsize,mticks,tag);
   }
   else if (lstat==2) {
      mstat[i1][1] += 1;
      if (monitor==2)
         fprintf(unit2,"receive posted: source= %d size= %d tag= %d\n",
                 idp,lsize,tag);
   }
   else if (lstat==(-2)) {
      mstat[i1][1] -= 1;
      if (monitor==2)
         fprintf(unit2,"received: source= %d size= %d time= %d tag= %d\n",
                 idp,lsize,mticks,tag);
   }
/* calculate current status */
   istat = 0;
   if (mstat[i1][0] >= 1)
      istat += 1;
   if (mstat[i1][1] >= 1)
      istat += 2;
/* differentiate single from multiple sends/receives */
/* if ((mstat[i1][0] > 1) || (mstat[i1][1] > 1))
      istat += 3;                                    */
/* display status, outline local node */
   if (idp==idproc)
      istyle = 1;
   else
      istyle = 0;
   showmess(idp,istat,istyle);
   return;
#undef NDSIZE
}

void showmess(int idp, int istat, int istyle) {
/* this subroutine shows MPI message status
   idp = remote processor id
   istat = message status = (0,1,2,3) = (none,sending,receiving,both)
   istyle = (0,1) = (no,yes) outline rectangle
   input argument: idp, istat, istyle                                 */
   return;
}

void showdism(int ibin,int nbin,int mbin,int lmax,int istyle) {
/* this subroutine shows distribution of MPI messages
   ibin = bin number for distribution function
   nbin = number of total messages in ibin
   mbin = number of send messages in ibin
   lmax = log2 of maximum number of messages in display
   istyle = (0,1) = (erase and rescale,draw) display
   input argument: ibin, nbin, nmax                       */
   return;
}

void shospeed(float atime,float ctime,float arate,float crate) {
/* this subroutine shows communication rates for MPI messages
   atime = short term average communication time (% of total)
   ctime = long term average communication time (% of total)
   arate = short term average reception rate (MB/sec)
   crate = long term average reception rate (MB/sec)
   input argument: atime, ctime, arate, crate                    */
   return;
}

void Logname(char* name) {
/* this subroutine records and displays user-defined label
   name = label to display                                  */
   if (monitor==2) {
      fprintf(unit2,"%s",name);
      fprintf(unit2,"\n");
   }
   return;
}

void Set_Mon(int monval) {
/* this subroutine sets new monitor value and corresponding window
   monval = new monitor value                                            */
/* declare internal mpi common block
   nproc = number of real or virtual processors obtained                 */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
/* create or destroy window if MPI has been initialized */
   if (nproc > 0) {
/* open window */
      if ((monitor==0) && (monval > 0))
         messwin(nproc);
/* close window */
      if ((monitor > 0) && (monval < 1))
         delmess();
   }
/* reset monitor value */
   if (monval > 1)
      monitor = 2;
   else if (monval < 1)
      monitor = 0;
   else
      monitor = 1;
   return;
}

int Get_Mon() {
/* this function gets current monitor value */
/* declare common block for non-blocking messages
   monitor = (0,1,2) = (suppress,display,display & log) monitor messages */
   return monitor;
}

void delmess() {
/* this subroutine closes a window for showing MPI message status */
/* internal common block for message window
   cpptr = pointer to window structure */
   return;
}
