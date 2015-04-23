/******************************************************************************
 * Copyright (c) 2013-2015, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/** rm_qmss_res.c
 *    QMSS resource allocation via RM
 *  Three files combined together: rm_linux_osal.c, sockutils.c, rmclient code
 *  External APIs: alloc_ocl_qmss_res(int *); free_ocl_qmss_res();
 *  Assumption: There is only one OpenCL device requesting QMSS resources.
 *              Otherwise, TODO: need to init RM client once,
 *              allocate res multiple times, cleanup and close RM client once.
 */

/* Standard Includes */
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

/* RM Includes */
#include <ti/drv/rm/rm.h>
#include <ti/drv/rm/rm_transport.h>
#include <ti/drv/rm/rm_services.h>


/**********************************************************************
 ****************************** Defines *******************************
 **********************************************************************/

// #define error_msg printf
// #define info_msg  printf
#define error_msg  
#define info_msg  

/**********************************************************************
 ************************** Global Variables **************************
 **********************************************************************/
uint32_t rmMallocCounter = 0;
uint32_t rmFreeCounter   = 0;

/**********************************************************************
 *************************** OSAL Functions **************************
 **********************************************************************/

/* FUNCTION PURPOSE: Allocates memory
 ***********************************************************************
 * DESCRIPTION: The function is used to allocate a memory block of the
 *              specified size.
 */
void *Osal_rmMalloc (uint32_t num_bytes)
{
    /* Increment the allocation counter. */
    rmMallocCounter++;

	/* Allocate memory. */
	return calloc(1, num_bytes);
}

/* FUNCTION PURPOSE: Frees memory
 ***********************************************************************
 * DESCRIPTION: The function is used to free a memory block of the
 *              specified size.
 */ 
void Osal_rmFree (void *ptr, uint32_t size)
{
    /* Increment the free counter. */
    rmFreeCounter++;
	free(ptr);
}

/* FUNCTION PURPOSE: Critical section enter
 ***********************************************************************
 * DESCRIPTION: The function is used to enter a critical section.
 *              Function protects against 
 *      
 *              access from multiple cores 
 *              and 
 *              access from multiple threads on single core
 */  
void *Osal_rmCsEnter(void)
{
    return NULL;
}

/* FUNCTION PURPOSE: Critical section exit
 ***********************************************************************
 * DESCRIPTION: The function is used to exit a critical section 
 *              protected using Osal_cppiCsEnter() API.
 */  
void Osal_rmCsExit(void *CsHandle)
{

}

/* FUNCTION PURPOSE: Cache invalidate
 ***********************************************************************
 * DESCRIPTION: The function is used to indicate that a block of memory is 
 *              about to be accessed. If the memory block is cached then this 
 *              indicates that the application would need to ensure that the 
 *              cache is updated with the data from the actual memory.
 */  
void Osal_rmBeginMemAccess(void *ptr, uint32_t size)
{
    return;
}

/* FUNCTION PURPOSE: Cache writeback
 ***********************************************************************
 * DESCRIPTION: The function is used to indicate that the block of memory has 
 *              finished being accessed. If the memory block is cached then the 
 *              application would need to ensure that the contents of the cache 
 *              are updated immediately to the actual memory. 
 */  
void Osal_rmEndMemAccess(void *ptr, uint32_t size)
{
    return;
}

/* FUNCTION PURPOSE: Creates a task blocking object
 ***********************************************************************
 * DESCRIPTION: The function is used to create a task blocking object
 *              capable of blocking the task a RM instance is running
 *              within
 */
void *Osal_rmTaskBlockCreate(void)
{
    return(NULL);
}

/* FUNCTION PURPOSE: Blocks a RM instance
 ***********************************************************************
 * DESCRIPTION: The function is used to block a task whose context a
 *              RM instance is running within.
 */
void Osal_rmTaskBlock(void *handle)
{

}

/* FUNCTION PURPOSE: unBlocks a RM instance
 ***********************************************************************
 * DESCRIPTION: The function is used to unblock a task whose context a
 *              RM instance is running within.
 */
void Osal_rmTaskUnblock(void *handle)
{

}

/* FUNCTION PURPOSE: Deletes a task blocking object
 ***********************************************************************
 * DESCRIPTION: The function is used to delete a task blocking object
 *              provided to a RM instance
 */
void Osal_rmTaskBlockDelete(void *handle)
{

}

/* FUNCTION PURPOSE: Multi-threaded critical section enter
 ***********************************************************************
 * DESCRIPTION: The function is used to enter a multi-threaded critical
 *              section. Function protects against
 *              access from multiple threads on single core
 */
void *Osal_rmMtCsEnter(void *mtSemObj)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)mtSemObj;

    pthread_mutex_lock(mutex);
    return NULL;
}

/* FUNCTION PURPOSE: Multi-threaded critical section exit
 ***********************************************************************
 * DESCRIPTION: The function is used to exit a multi-threaded critical
 *              section protected using Osal_rmMtCsEnter() API.
 */
void Osal_rmMtCsExit(void *mtSemObj, void *CsHandle)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)mtSemObj;

    pthread_mutex_unlock(mutex);
}

/* FUNCTION PURPOSE: Prints a variable list
 ***********************************************************************
 * DESCRIPTION: The function is used to print a string to the console
 */
void Osal_rmLog (char *fmt, ... )
{
    va_list ap;
    
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif


typedef enum {
        sock_name_e,
        sock_addr_e
} sock_name_type;

typedef struct {
        sock_name_type type;
        union sock {
                char *name;
                struct sockaddr_un *addr;
        } s;
} sock_name_t;

#define sock_h void *


typedef struct sock_data {
	struct sockaddr_un addr;
	fd_set  readfds;
	int fd;
} sock_data_t;

int check_and_create_path (char *path)
{
	char *d = path;
	if (!d)
		return -1;

	while (d = strchr(d + 1, '/')) {
		*d = 0;
		if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
			if (errno != EEXIST) {
				*d = '/';
				error_msg("can't create path %s (error: %s)",
						path, strerror(errno));
				return -1;
			}
		}
		*d = '/';
	}
	return 0;
}

sock_h sock_open (sock_name_t *sock_name)
{
	sock_data_t *sd = 0;
	int retval = 0;

	if (!sock_name) {
		return 0;
	}

	sd = (sock_data_t *) calloc (1, sizeof(sock_data_t));

	if (sock_name->type == sock_addr_e) {
		memcpy (&sd->addr, sock_name->s.addr, sizeof(struct sockaddr_un));
	} else {
		if (check_and_create_path(sock_name->s.name) < 0) {
			goto check_n_return;
		}
		sd->addr.sun_family = AF_UNIX;
		strncpy(sd->addr.sun_path, sock_name->s.name, UNIX_PATH_MAX);
	}

	sd->fd =  socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sd->fd < 0) {
		error_msg("can't open socket (error: %s)",
				sd->addr.sun_path, strerror(errno));
		goto check_n_return;
	}

	unlink(sd->addr.sun_path);
	if (bind(sd->fd, (struct sockaddr *) &sd->addr, sizeof(struct sockaddr_un)) < 0) {
		error_msg("can't bind socket (error: %s)",
				sd->addr.sun_path, strerror(errno));
		goto check_n_return;
	}

	FD_ZERO(&sd->readfds);
	FD_SET(sd->fd, &sd->readfds);

	retval = (int) sd;

check_n_return:
	if (!retval) {
		sock_close ((sock_h) sd);
	}

	return ((sock_h) retval);
}

int sock_close (sock_h handle)
{
	sock_data_t *sd = (sock_data_t *) handle;

	if (!sd) {
		return -1;
	}

	if (sd->fd)
		close (sd->fd);
	free (sd);

	return 0;	
}

int sock_send (sock_h handle, const char *data, int length,
			sock_name_t *to)
{
	int fd;
	sock_data_t *sd = (sock_data_t *) handle;
	struct sockaddr_un to_addr;

	if (!to) {
		return -1;
	}

	if (to->type == sock_addr_e) {
		memcpy (&to_addr, to->s.addr, sizeof(struct sockaddr_un));
	} else {
		to_addr.sun_family = AF_UNIX;
		strncpy(to_addr.sun_path, to->s.name, UNIX_PATH_MAX);
	}

	if (sd) {
		fd = sd->fd;
	} else {
		fd =  socket(AF_UNIX, SOCK_DGRAM, 0);
		if (fd < 0) {
			error_msg("can't open socket (error: %s)",
					to_addr.sun_path, strerror(errno));
			return -1;
		}
	}

	if (sendto (fd, data, length, 0, (struct sockaddr *) &to_addr,
				sizeof(struct sockaddr_un)) < 0) {
		error_msg("can't send data to %s (error: %s)",
				to_addr.sun_path, strerror(errno));
		return -1;

	}

	return 0;
}

int sock_wait (sock_h handle, int *size, struct timeval *timeout, int extern_fd)
{
	sock_data_t *sd = (sock_data_t *) handle;
	int retval;
	fd_set fds;

	if (!sd) {
		error_msg("invalid hanlde");
		return -1;
	}

	fds = sd->readfds;

	if (extern_fd != -1) {
		FD_SET(extern_fd, &fds);
	}

	retval = select(FD_SETSIZE, &fds, NULL, NULL, timeout);
	if (retval == -1) {
		error_msg("select failed for %s (error: %s)",
				sd->addr.sun_path, strerror(errno));
		return -1;
	}

	if ((extern_fd != -1) && (FD_ISSET(extern_fd, &fds))) {
		return 1;
	}

	if (!FD_ISSET(sd->fd, &fds)) {
		/* Wait timedout */
		return -2;
	}

	if (!retval) {
		return 0;
	}

	if (size != 0) {
		retval = ioctl(sd->fd, FIONREAD, size);
		if (retval == -1) {
			error_msg("can't read datagram size for %s (error: %s)",
					sd->addr.sun_path, strerror(errno));
			return -1;
		}
	}

	return 0;
}

int sock_recv (sock_h handle, char *data, int length, sock_name_t *from)
{
	int size;
	int retval;
	fd_set fds;
	sock_data_t *sd = (sock_data_t *) handle;
	socklen_t from_length = 0;

	if (!sd) {
		error_msg("invalid hanlde");
		return -1;
	}

	if (from) {
		if((from->type = sock_addr_e) && (from->s.addr))
			from_length = sizeof(struct sockaddr_un);
		else {
			error_msg("invalid from parameter");
			return -1;
		}
	}

	size = recvfrom(sd->fd, data, length, 0, (struct sockaddr *)((from_length) ? from->s.addr : NULL), &from_length);
	if (size < 1) {
		error_msg("can't read datagram from socket for %s (error: %s), size %d",
				sd->addr.sun_path, strerror(errno), size);
		return -1;
	}

	return size;
	
}


/**********************************************************************
 ****************************** RM Client *****************************
 **********************************************************************/

/* RM Server Socket Name */
#define RM_SERVER_SOCKET_NAME "/tmp/var/run/rm/rm_server"

/* Client instance name (must match with RM Global Resource List (GRL) and policies */
char   rmClientName[RM_NAME_MAX_CHARS] = "RM_Client3";

/* Client socket name */
char   rmClientSockName[] = "/tmp/var/run/rm/rm_client_ocl";
// char   rmClientSockName[] = "/tmp/rm_client_ocl";

/* RM resource names (must match resource node names in GRL and policies */
/* QM1 and QM2 share memory regions and linking rams */
char   resGpQ1[RM_NAME_MAX_CHARS]        = "GENERAL_PURPOSE_QUEUE-qm1";
char   resGpQ2[RM_NAME_MAX_CHARS]        = "GENERAL_PURPOSE_QUEUE-qm2";
char   resMemRegion1[RM_NAME_MAX_CHARS]  = "memory-regions-qm1";
char   resIntLinkingRam1[RM_NAME_MAX_CHARS] = "linkram-int-qm1";
char   resExtLinkingRam1[RM_NAME_MAX_CHARS] = "linkram-ext-qm1";
char   nameOclQ[RM_NAME_MAX_CHARS]                = "OCL-Q";
char   nameOclMemRegion[RM_NAME_MAX_CHARS]        = "OCL-MEM";
char   nameOclDescInLinkingRam[RM_NAME_MAX_CHARS] = "OCL-DESC";

/* Application's registered RM transport indices */
#define SERVER_TO_CLIENT_MAP_ENTRY   0
/* Maximum number of registered RM transports */
#define MAX_MAPPING_ENTRIES          1

/* Error checking macro */
#define RM_ERROR_CHECK(checkVal, resultVal, rmInstName, printMsg)                 \
    if (resultVal != checkVal) {                                                  \
        char errorMsgToPrint[] = printMsg;                                        \
        printf("RM Inst : %s : ", rmInstName);                                    \
        printf("%s with error code : %d, exiting\n", errorMsgToPrint, resultVal); \
        return(-1);                                                               \
    }

/* RM registered transport mapping structure */
typedef struct trans_map_entry_s {
    /* Registered RM transport handle */
    Rm_TransportHandle        transportHandle;
    /* Remote socket tied to the transport handle */
    sock_name_t              *remote_sock;
} Transport_MapEntry;

/* Client socket handle */
sock_h              rmClientSocket;

/* Client instance handles */
Rm_Handle           rmClientHandle = NULL;

/* Client instance service handles */
Rm_ServiceHandle   *rmClientServiceHandle = NULL;

/* Transport map stores the RM transport handle to IPC MessageQ queue mapping */
Transport_MapEntry  rmTransportMap[MAX_MAPPING_ENTRIES];


Rm_Packet *transportAlloc(Rm_AppTransportHandle appTransport, uint32_t pktSize, Rm_PacketHandle *pktHandle)
{
    Rm_Packet *rm_pkt = NULL;

    rm_pkt = calloc(1, sizeof(*rm_pkt));
    if (!rm_pkt) {
        printf("can't malloc for RM send message (err: %s)\n", strerror(errno));
        return (NULL);
    }
    rm_pkt->pktLenBytes = pktSize;
    *pktHandle = rm_pkt;

    return(rm_pkt);
}

void transportFree (Rm_Packet *rm_pkt)
{
    if (rm_pkt) {
        free (rm_pkt);
    }         
}

void transportReceive (void)
{
    int32_t             rm_result;
    int                 retval;
    int                 length = 0;
    sock_name_t         server_sock_addr;
    Rm_Packet          *rm_pkt = NULL;
    struct sockaddr_un  server_addr;    
    
    retval = sock_wait(rmClientSocket, &length, NULL, -1);
    if (retval == -2) {
        /* Timeout */
        printf("Error socket timeout\n");
        return;
    }
    else if (retval < 0) {
        printf("Error in reading from socket, error %d\n", retval);
        return;
    }
    
    if (length < sizeof(*rm_pkt)) {
        printf("invalid RM message length %d\n", length);
        return;
    }
    rm_pkt = calloc(1, length);
    if (!rm_pkt) {
        printf("can't malloc for recv'd RM message (err: %s)\n",
               strerror(errno));
        return;
    }
    
    server_sock_addr.type = sock_addr_e;
    server_sock_addr.s.addr = &server_addr;
    retval = sock_recv(rmClientSocket, (char *)rm_pkt, length, &server_sock_addr);
    if (retval != length) {
        printf("recv RM pkt failed from socket, received = %d, expected = %d\n",
               retval, length);
        return;
    }
    
    //printf("received RM pkt of size %d bytes from %s\n", length, server_sock_addr.s.addr->sun_path);

    /* Provide packet to RM Client for processing */       
    if ((rm_result = Rm_receivePacket(rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].transportHandle, rm_pkt))) {
        printf("RM failed to process received packet: %d\n", rm_result);
    }

    transportFree(rm_pkt);
}

int32_t transportSendRcv (Rm_AppTransportHandle appTransport, Rm_PacketHandle pktHandle)
{
    sock_name_t *server_sock_name = (sock_name_t *)appTransport;
    Rm_Packet   *rm_pkt = (Rm_Packet *)pktHandle;
    
    if (sock_send(rmClientSocket, (char *)rm_pkt, (int) rm_pkt->pktLenBytes, server_sock_name)) {
        info_msg("send data failed\n");
        return -1;
    }

    /* Wait for response from Server */
    transportReceive();
 
    return (0);
}

int connection_setup(void)
{
    Rm_TransportCfg rmTransCfg;
    int32_t         rm_result;
    int             i;
    sock_name_t     sock_name;
    char            server_sock_name[] = RM_SERVER_SOCKET_NAME;
    
    /* Initialize the transport map */
    for (i = 0; i < MAX_MAPPING_ENTRIES; i++) {
        rmTransportMap[i].transportHandle = NULL;
    }

    sock_name.type = sock_name_e;
    sock_name.s.name = rmClientSockName;

    rmClientSocket = sock_open(&sock_name);
    if (!rmClientSocket) {
        printf("connection_setup: Client socket open failed\n");
        return (-1);
    }

    rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].remote_sock = calloc(1, sizeof(sock_name_t));
    rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].remote_sock->type = sock_name_e;    
    rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].remote_sock->s.name = calloc(1, strlen(server_sock_name)+1);
    strncpy(rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].remote_sock->s.name, server_sock_name, strlen(server_sock_name)+1);

    /* Register the Server with the Client instance */
    rmTransCfg.rmHandle = rmClientHandle;
    rmTransCfg.appTransportHandle = (Rm_AppTransportHandle) rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].remote_sock;
    rmTransCfg.remoteInstType = Rm_instType_SERVER;
    rmTransCfg.transportCallouts.rmAllocPkt = transportAlloc;
    rmTransCfg.transportCallouts.rmSendPkt = transportSendRcv;
    rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].transportHandle = Rm_transportRegister(&rmTransCfg, &rm_result);  

    return(0);
}


/** ============================================================================
 *   @n@b initRm
 *
 *   @b Description
 *   @n This API initializes the RM Client for the QMSS test establishing
 *      a socket connection with the RM Server
 * 
 *   @return    int32_t
 *              -1      -   Error
 *              0       -   Success
 * =============================================================================
 */
int initRm (void)
{
    Rm_InitCfg         rmInitCfg;
    int32_t            result;
    
    /* Initialize the RM Client - RM must be initialized before anything else in the system */
    memset(&rmInitCfg, 0, sizeof(rmInitCfg));
    rmInitCfg.instName = rmClientName;
    rmInitCfg.instType = Rm_instType_CLIENT;
    rmClientHandle = Rm_init(&rmInitCfg, &result);
    RM_ERROR_CHECK(RM_OK, result, rmClientName, "Initialization failed");

    info_msg("\n\nInitialized %s\n\n", rmClientName);

    /* Open Client service handle */
    rmClientServiceHandle = Rm_serviceOpenHandle(rmClientHandle, &result);
    RM_ERROR_CHECK(RM_OK, result, rmClientName, "Service handle open failed");

    return(connection_setup());
}

void setRmRequest(Rm_ServiceReqInfo *reqInfo, Rm_ServiceType type,
                  const char *resName, int32_t resBase, uint32_t resLen,
                  int32_t resAlign, const char *nsName,
                  Rm_ServiceRespInfo *respInfo)
{                                                                                
    memset((void *)reqInfo, 0, sizeof(Rm_ServiceReqInfo));
    memset((void *)respInfo, 0, sizeof(Rm_ServiceRespInfo));

    reqInfo->type = type;
    reqInfo->resourceName = resName;
    reqInfo->resourceBase = resBase;
    reqInfo->resourceLength = resLen;
    reqInfo->resourceAlignment = resAlign;
    reqInfo->resourceNsName = nsName;
}

int allocResFromRm(char *resName, int numRes, int align, char *nsName)
{
    Rm_ServiceReqInfo  requestInfo;
    Rm_ServiceRespInfo responseInfo;
    int allocatedBase = RM_RESOURCE_BASE_UNSPECIFIED;

    /* BEGIN Allocating some resources without providing a callback function.
     * RM should block and not return until the result is returned by the
     * server. */
    setRmRequest(&requestInfo, Rm_service_RESOURCE_ALLOCATE_INIT,
                 resName, RM_RESOURCE_BASE_UNSPECIFIED, numRes, align, NULL,
                 &responseInfo);
    rmClientServiceHandle->Rm_serviceHandler(rmClientServiceHandle->rmHandle,
                                             &requestInfo, &responseInfo);
    info_msg("%s Allocated: base=%d, len=%d, align=%d, #Owners=%d, state=%s\n",
           resName,
           responseInfo.resourceBase, responseInfo.resourceLength,
           requestInfo.resourceAlignment, 
           responseInfo.resourceNumOwners,
           ((responseInfo.serviceState == RM_SERVICE_APPROVED) ? "Approved"
                                                               : "Failed")
          );
    if (responseInfo.serviceState == RM_SERVICE_APPROVED)
        allocatedBase = responseInfo.resourceBase;

    if (allocatedBase != RM_RESOURCE_BASE_UNSPECIFIED && nsName != NULL)
    {
        setRmRequest(&requestInfo, Rm_service_RESOURCE_MAP_TO_NAME,
                 resName, allocatedBase, numRes, align, nsName,
                 &responseInfo);
        rmClientServiceHandle->Rm_serviceHandler(
                 rmClientServiceHandle->rmHandle, &requestInfo, &responseInfo);
        info_msg("%s Mapped: state=%s\n", nsName,
               ((responseInfo.serviceState == RM_SERVICE_APPROVED) ? "Approved"
                                                                   : "Failed")
              );
    }

    return allocatedBase;
}

void freeResFromRm(char *resName, int resIndex, int numRes, int align,
                   char *nsName, int *state)
{
    Rm_ServiceReqInfo  requestInfo;
    Rm_ServiceRespInfo responseInfo;
    int32_t            result;

    /* Free all allocated resources */
    setRmRequest(&requestInfo, Rm_service_RESOURCE_FREE,
                 resName, resIndex, numRes, align, nsName,
                 &responseInfo);
    rmClientServiceHandle->Rm_serviceHandler(rmClientServiceHandle->rmHandle,
                                             &requestInfo, &responseInfo);
    if (state != NULL)  *state = responseInfo.serviceState;
    info_msg("%s Free: state=%s\n", (nsName == NULL ? resName : nsName),
           ((responseInfo.serviceState == RM_SERVICE_APPROVED) ? "Approved"
                                                               : "Failed")
          );

    if (nsName != NULL)
    {
        setRmRequest(&requestInfo, Rm_service_RESOURCE_UNMAP_NAME,
                     resName, resIndex, numRes, align, nsName,
                     &responseInfo);
        rmClientServiceHandle->Rm_serviceHandler(
                     rmClientServiceHandle->rmHandle,
                     &requestInfo, &responseInfo);
        info_msg("%s Unmap: state=%s\n", nsName,
               ((responseInfo.serviceState == RM_SERVICE_APPROVED) ? "Approved"
                                                                   : "Failed")
              );
    }
}

void free_ocl_qmss_res()
{
    if (rmClientServiceHandle == NULL || rmClientHandle == NULL ||
        rmClientSocket == NULL)  return;

    int32_t            result;
    /* Free all allocated resources */
    freeResFromRm(NULL, 0, 0, 0, nameOclQ, NULL);
    freeResFromRm(NULL, 0, 0, 0, nameOclMemRegion, NULL);
    freeResFromRm(NULL, 0, 0, 0, nameOclDescInLinkingRam, NULL);

    /* Cleanup all service ports, transport handles, RM instances,
       and IPC constructs */
    result = Rm_serviceCloseHandle(rmClientServiceHandle);
    if (result != RM_OK)
        printf("Rm_serviceCloseHandle failed, code:%d\n", result);

    result = Rm_transportUnregister(
                   rmTransportMap[SERVER_TO_CLIENT_MAP_ENTRY].transportHandle);
    if (result != RM_OK)
        printf("Rm_transportUnregister failed, code:%d\n", result);

    sock_close(rmClientSocket);

    result = Rm_delete(rmClientHandle, 1);
    if (result != RM_OK)
        printf("Rm_delete failed, code:%d\n", result);

    rmClientServiceHandle = NULL;
    rmClientHandle = NULL;
}

// QMSS resources to be allocated by RM, (OpenCL + OpenMP)
#define NUM_HW_QUEUES             (3+11)
#define NUM_MEM_REGIONS           (1+1)
#define NUM_DESC_IN_LINKING_RAMS  (1024 + 256)

/*----------------------------------------------------------------------------
* Return 1 if succeed, 0 if failed, -1 if no RmServer to talk to
*----------------------------------------------------------------------------*/
int get_ocl_qmss_res(int *res)
{
    res[0] = res[1] = res[2] = RM_RESOURCE_BASE_UNSPECIFIED;
    if (initRm() != 0)  return 0;

    int state;
    freeResFromRm(NULL, 0, 0, 0, nameOclQ, &state);
    if (state == RM_ERROR_TRANSPORT_SEND_ERROR)  return -1;  // no RmServer
    freeResFromRm(NULL, 0, 0, 0, nameOclMemRegion, NULL);
    freeResFromRm(NULL, 0, 0, 0, nameOclDescInLinkingRam, NULL);

    // try QM1 and QM2 for hardware queue
    res[0] = allocResFromRm(resGpQ1, NUM_HW_QUEUES, 0, nameOclQ);
    if (res[0] == RM_RESOURCE_BASE_UNSPECIFIED)
        res[0] = allocResFromRm(resGpQ2, NUM_HW_QUEUES, 0, nameOclQ);

    if (res[0] != RM_RESOURCE_BASE_UNSPECIFIED)
    {
        // QM1 and QM2 share memory regions and link ram (internal & external)
        res[1] = allocResFromRm(resMemRegion1, NUM_MEM_REGIONS, 0,
                                nameOclMemRegion);
        if (res[1] != RM_RESOURCE_BASE_UNSPECIFIED)
        {
            res[2] = allocResFromRm(resIntLinkingRam1, NUM_DESC_IN_LINKING_RAMS,
                                    0, nameOclDescInLinkingRam);
            if (res[2] == RM_RESOURCE_BASE_UNSPECIFIED)
                res[2] = allocResFromRm(resExtLinkingRam1,
                                        NUM_DESC_IN_LINKING_RAMS,
                                        0, nameOclDescInLinkingRam);
        }
    }

    if (res[0] != RM_RESOURCE_BASE_UNSPECIFIED &&
        res[1] != RM_RESOURCE_BASE_UNSPECIFIED &&
        res[2] != RM_RESOURCE_BASE_UNSPECIFIED)  return 1;

    // failed: cleanup
    free_ocl_qmss_res();
    return 0;
}

