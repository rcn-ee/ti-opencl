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
 *  \file osalconfig.h
 *
 *  \brief OSAL header file containing configuration parametes.
 *
 */

#ifndef _osconfig_
#define _osconfig_

/*
** Platform Configuration Parameters for the OS API
*/

#define OS_MAX_TASKS                64
#define OS_MAX_QUEUES               64
#define OS_MAX_COUNT_SEMAPHORES     20
#define OS_MAX_BIN_SEMAPHORES       20
#define OS_MAX_MUTEXES              20

/*
** Maximum length for an absolute path name
*/
#define OS_MAX_PATH_LEN     64

/*
** Maximum length for a local or host path/filename.
** This parameter is used for the filename on the host OS. 
** Because the local or host path can add on to the OSAL virtual path,
** This needs to be longer than OS_MAX_PATH_LEN. 
**  On a system such as RTEMS, where the virtual paths can be the same as 
**  local paths, it does not have to be much bigger.
** On a system such as Linux, where a OSAL virtual drive might be 
** mapped to something like: "/home/bob/projects/osal/drive1", the 
**  OS_MAX_LOCAL_PATH_LEN might need to be 32 more than OS_MAX_PATH_LEN.
*/
#define OS_MAX_LOCAL_PATH_LEN (OS_MAX_PATH_LEN + 16)


/* 
** The maxium length allowed for a object (task,queue....) name 
*/
#define OS_MAX_API_NAME     20

/* 
** The maximum length for a file name 
*/
#define OS_MAX_FILE_NAME    20

/* 
** These defines are for OS_printf
*/
#define OS_BUFFER_SIZE 172
#define OS_BUFFER_MSG_DEPTH 100

/* This #define turns on a utility task that
 * will read the statements to print from
 * the OS_printf function. If you want OS_printf
 * to print the text out itself, comment this out 
 * 
 * NOTE: The Utility Task #defines only have meaning 
 * on the VxWorks operating systems
 */
 
#define OS_UTILITY_TASK_ON


#ifdef OS_UTILITY_TASK_ON 
    #define OS_UTILITYTASK_STACK_SIZE 2048
    /* some room is left for other lower priority tasks */
    #define OS_UTILITYTASK_PRIORITY   245
#endif


/* 
** the size of a command that can be passed to the underlying OS 
*/
#define OS_MAX_CMD_LEN 1000



/*
** This define sets the maximum number of timers available
*/
#define OS_MAX_TIMERS         5

#endif
