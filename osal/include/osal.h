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

#define OSAL_SUCCESS                     (0)
#define OSAL_ERROR                       (-1)
#define OSAL_INVALID_POINTER             (-2)
#define OSAL_ERROR_ADDRESS_MISALIGNED    (-3)
#define OSAL_ERROR_TIMEOUT               (-4)
#define OSAL_INVALID_INT_NUM             (-5)
#define OSAL_SEM_FAILURE                 (-6)
#define OSAL_SEM_TIMEOUT                 (-7)
#define OSAL_QUEUE_EMPTY                 (-8)
#define OSAL_QUEUE_FULL                  (-9)
#define OSAL_QUEUE_TIMEOUT               (-10)
#define OSAL_QUEUE_INVALID_SIZE          (-11)
#define OSAL_QUEUE_ID_ERROR              (-12)
#define OSAL_ERR_NAME_TOO_LONG           (-13)
#define OSAL_ERR_NO_FREE_IDS             (-14)
#define OSAL_ERR_NAME_TAKEN              (-15)
#define OSAL_ERR_INVALID_ID              (-16)
#define OSAL_ERR_NAME_NOT_FOUND          (-17)
#define OSAL_ERR_SEM_NOT_FULL            (-18)
#define OSAL_ERR_INVALID_PRIORITY        (-19)
#define OSAL_INVALID_SEM_VALUE           (-20)
#define OSAL_ERR_FILE                    (-27)
#define OSAL_ERR_NOT_IMPLEMENTED         (-28)
#define OSAL_TIMER_ERR_INVALID_ARGS      (-29)
#define OSAL_TIMER_ERR_TIMER_ID          (-30)
#define OSAL_TIMER_ERR_UNAVAILABLE       (-31)
#define OSAL_TIMER_ERR_INTERNAL          (-32)

/*
** Defines for Queue Timeout parameters
*/
#define OSAL_PEND   (0)
#define OSAL_CHECK (-1)


/*
** Include the configuration file
*/
#include "osalconfig.h"

/*
** Include the OS API modules
*/
#include "osal_os_core.h"

#endif
