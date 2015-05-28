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
 *  \file osal_os_core.h
 *
 *  \brief OSAL header file containing core apis.
 *
 */

#ifndef _osal_core_api_
#define _osal_core_api_

#include "osal.h"
#include "stdarg.h"   /* for va_list */

/*  tables for the properties of objects */

/*tasks */
typedef struct
{
    char name [OSAL_MAX_API_NAME];
    uint32 creator;
    uint32 stack_size;
    uint32 priority;
    uint32 OStask_id;
}OSAL_task_prop_t;
    

/* Mutexes */
typedef struct
{
    char name [OSAL_MAX_API_NAME];
    uint32 creator;
}OSAL_mut_sem_prop_t;

/* Conditional Variable */
typedef struct
{
    char name [OSAL_MAX_API_NAME];
    uint32 creator;
}OSAL_cv_prop_t;




/* This typedef is for the OSAL_GetErrorName function, to ensure
 * everyone is making an array of the same length */

typedef char osal_err_name_t[35];

/*
** These typedefs are for the task entry point
*/
typedef void osal_task;
typedef osal_task ((*osal_task_entry)(void));

/*
** Exported Functions
*/

/*
** Initialization of API
*/
int32 TI_OSAL_API_Init (void);


/*
** Task API
*/

int32 TI_OSAL_TaskCreate            (uint32 *task_id, const char *task_name, 
                                osal_task_entry function_pointer,
                                const uint32 *stack_pointer, 
                                uint32 stack_size,
                                uint32 priority, uint32 flags);

int32 TI_OSAL_TaskDelete            (uint32 task_id); 
void TI_OSAL_TaskExit               (void);
int32 TI_OSAL_TaskInstallDeleteHandler(void *function_pointer);
int32 TI_OSAL_TaskDelay             (uint32 millisecond);
int32 TI_OSAL_TaskSetPriority       (uint32 task_id, uint32 new_priority);
int32 TI_OSAL_TaskRegister          (void);
uint32 TI_OSAL_TaskGetId            (void);
int32 TI_OSAL_TaskGetIdByName       (uint32 *task_id, const char *task_name);
int32 TI_OSAL_TaskGetInfo           (uint32 task_id, OSAL_task_prop_t *task_prop);

/*
** Mutex API
*/

int32 TI_OSAL_MutSemCreate           (uint32 *mut_id, const char *mut_name, uint32 options);
int32 TI_OSAL_MutSemGive             (uint32 mut_id);
int32 TI_OSAL_MutSemTake             (uint32 mut_id);
int32 TI_OSAL_MutSemDelete           (uint32 mut_id);  
int32 TI_OSAL_MutSemGetIdByName      (uint32 *mut_id, const char *mut_name); 
int32 TI_OSAL_MutSemGetInfo          (uint32 mut_id, OSAL_mut_sem_prop_t *mut_prop);

/*
** Conditional Variable API
*/

int32 TI_OSAL_ConVarCreate           (uint32 *cv_id, const char *cv_name, uint32 options);
int32 TI_OSAL_ConVarDelete           (uint32 cv_id);
int32 TI_OSAL_ConVarSignal           (uint32 cv_id);
int32 TI_OSAL_ConVarBroadcast        (uint32 cv_id);
int32 TI_OSAL_ConVarWait             (uint32 cv_id, uint32 mut_id);  


/* 
** Abstraction for printf statements 
*/
void TI_OSAL_printf( const char *string, ...);
void TI_OSAL_printf_disable(void);
void TI_OSAL_printf_enable(void);

#endif
