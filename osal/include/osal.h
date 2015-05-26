/******************************************************************************
 *                                                                            *
 * Copyright (c) 2012 Texas Instruments Incorporated - http://www.ti.com/     *
 *                                                                            *
 * All rights reserved. Property of Texas Instruments Incorporated.           *
 * Restricted rights to use, duplicate or disclose this code are              *
 * granted through contract.                                                  *
 *                                                                            *
 * The program may not be used without the written permission                 *
 * of Texas Instruments Incorporated or against the terms and conditions      *
 * stipulated in the agreement under which this program has been              *
 * supplied.                                                                  *
 *                                                                            *
 *****************************************************************************/

/**
 *  \file osal.h
 *
 *  \brief OSAL header file containing error types and includes.
 *
 */
 #ifndef _osal_
#define _osal_

#include "common_types.h"

#define OS_SUCCESS                     (0)
#define OS_ERROR                       (-1)
#define OS_INVALID_POINTER             (-2)
#define OS_ERROR_ADDRESS_MISALIGNED    (-3)
#define OS_ERROR_TIMEOUT               (-4)
#define OS_INVALID_INT_NUM             (-5)
#define OS_SEM_FAILURE                 (-6)
#define OS_SEM_TIMEOUT                 (-7)
#define OS_QUEUE_EMPTY                 (-8)
#define OS_QUEUE_FULL                  (-9)
#define OS_QUEUE_TIMEOUT               (-10)
#define OS_QUEUE_INVALID_SIZE          (-11)
#define OS_QUEUE_ID_ERROR              (-12)
#define OS_ERR_NAME_TOO_LONG           (-13)
#define OS_ERR_NO_FREE_IDS             (-14)
#define OS_ERR_NAME_TAKEN              (-15)
#define OS_ERR_INVALID_ID              (-16)
#define OS_ERR_NAME_NOT_FOUND          (-17)
#define OS_ERR_SEM_NOT_FULL            (-18)
#define OS_ERR_INVALID_PRIORITY        (-19)
#define OS_INVALID_SEM_VALUE           (-20)
#define OS_ERR_FILE                    (-27)
#define OS_ERR_NOT_IMPLEMENTED         (-28)
#define OS_TIMER_ERR_INVALID_ARGS      (-29)
#define OS_TIMER_ERR_TIMER_ID          (-30)
#define OS_TIMER_ERR_UNAVAILABLE       (-31)
#define OS_TIMER_ERR_INTERNAL          (-32)

/*
** Defines for Queue Timeout parameters
*/
#define OS_PEND   (0)
#define OS_CHECK (-1)


/*
** Include the configuration file
*/
#include "osconfig.h"

/*
** Include the OS API modules
*/
#include "osapi-os-core.h"
#include "osapi-os-filesys.h"
#include "osapi-os-net.h"
#include "osapi-os-loader.h"
#include "osapi-os-timer.h"

#endif