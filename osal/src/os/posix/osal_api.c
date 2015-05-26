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
 *  \file osal_api.c
 *
 *  \brief OSAL source file containing core apis.
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>

#include <errno.h> 
#include <string.h>     

/*
** The __USE_UNIX98 is for advanced pthread features on linux
*/
#define __USE_UNIX98
#include <pthread.h>

/*
** User defined include files
*/
#include "common_types.h"
#include "osal.h"


/*
** Global data for the API
*/

/*  
** Tables for the properties of objects 
*/

/*tasks */
typedef struct
{
    int       free;
    pthread_t id;
    char      name [OSAL_MAX_API_NAME];
    int       creator;
    uint32    stack_size;
    uint32    priority;
    void     *delete_hook_pointer;
}OSAL_task_record_t;
   

/* Mutexes */
typedef struct
{
    int             free;
    pthread_mutex_t id;
    char            name [OSAL_MAX_API_NAME];
    int             creator;
}OSAL_mut_sem_record_t;

/* Conditional variables */
typedef struct
{
    int             free;
    pthread_cond_t id;
    char            name [OSAL_MAX_API_NAME];
    int             creator;
}OSAL_con_var_record_t;



/* function pointer type */
typedef void (*FuncPtr_t)(void);

/* Tables where the OS object information is stored */
OSAL_task_record_t    OSAL_task_table          [OSAL_MAX_TASKS];
OSAL_mut_sem_record_t OSAL_mut_sem_table       [OSAL_MAX_MUTEXES];
OSAL_con_var_record_t OSAL_con_var_table       [OSAL_MAX_CONVARS];

pthread_key_t    thread_key;

pthread_mutex_t OSAL_task_table_mut;
pthread_mutex_t OSAL_mut_sem_table_mut;
pthread_mutex_t OSAL_con_var_table_mut;

uint32          OSAL_printf_enabled = TRUE;

/*
** Local Function Prototypes
*/

void    TI_OSAL_ThreadKillHandler(int sig );
uint32  TI_OSAL_FindCreator(void);
int32   TI_OSAL_PriorityRemap(uint32 InputPri);

/*---------------------------------------------------------------------------------------
   Name: TI_OSAL_API_Init

   Purpose: Initialize the tables that the OS API uses to keep track of information
            about objects

   returns: OSAL_SUCCESS or OSAL_ERROR
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_API_Init(void)
{
   int                 i;
   int                 ret;
   pthread_mutexattr_t mutex_attr ;    
   int32               return_code = OSAL_SUCCESS;
    
    /* Initialize Task Table */
   
   for(i = 0; i < OSAL_MAX_TASKS; i++)
   {
        OSAL_task_table[i].free                = TRUE;
        OSAL_task_table[i].creator             = UNINITIALIZED;
        OSAL_task_table[i].delete_hook_pointer = NULL;
        strcpy(OSAL_task_table[i].name,"");    
    }

       /* Initialize Mutex Semaphore Table */

    for(i = 0; i < OSAL_MAX_MUTEXES; i++)
    {
        OSAL_mut_sem_table[i].free        = TRUE;
        OSAL_mut_sem_table[i].creator     = UNINITIALIZED;
        strcpy(OSAL_mut_sem_table[i].name,"");
    }

	    /* Initialize conditional variable Table */

    for(i = 0; i < OSAL_MAX_CONVARS; i++)
    {
        OSAL_con_var_table[i].free        = TRUE;
        OSAL_con_var_table[i].creator     = UNINITIALIZED;
        strcpy(OSAL_con_var_table[i].name,"");
    }
  
   ret = pthread_key_create(&thread_key, NULL );
   if ( ret != 0 )
   {
      printf("Error creating thread key\n");
      return_code = OSAL_ERROR;
      return(return_code);
   }

   /* 
   ** initialize the pthread mutex attribute structure with default values 
   */
   return_code = pthread_mutexattr_init(&mutex_attr); 
   if ( return_code != 0 )
   {
      printf("Error: pthread_mutexattr_init failed\n");
      return_code = OSAL_ERROR;
      return (return_code);
   }

   /*
   ** Allow the mutex to use priority inheritance  
   */  
   return_code = pthread_mutexattr_setprotocol(&mutex_attr,PTHREAD_PRIO_INHERIT) ;
   if ( return_code != 0 )
   {
      printf("Error: pthread_mutexattr_setprotocol failed\n");
      return_code = OSAL_ERROR;
      return (return_code);
   }	

   /*
   **  Set the mutex type to RECURSIVE so a thread can do nested locks
   */
   return_code = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
   if ( return_code != 0 )
   {
      printf("Error: pthread_mutexattr_settype failed\n");
      return_code = OSAL_ERROR;
      return (return_code);
   }

   /*
   ** create the mutexes that protect the OSAPI structures 
   ** the function returns on error, since we dont want to go through
   ** the extra trouble of creating and deleting resoruces for nothing
   */   
   ret = pthread_mutex_init((pthread_mutex_t *) & OSAL_task_table_mut,&mutex_attr);
   if ( ret != 0 )
   {
      return_code = OSAL_ERROR;
      return(return_code);
   }
  
   ret = pthread_mutex_init((pthread_mutex_t *) & OSAL_mut_sem_table_mut,&mutex_attr); 
   if ( ret != 0 )
   {
      return_code = OSAL_ERROR;
      return(return_code);
   }

    ret = pthread_mutex_init((pthread_mutex_t *) & OSAL_con_var_table_mut,&mutex_attr); 
   if ( ret != 0 )
   {
      return_code = OSAL_ERROR;
      return(return_code);
   }

   
   /*
   ** All other initializations if needed add here
   */


   return(return_code);
   
}

/*
**********************************************************************************
**          TASK API
**********************************************************************************
*/

/*---------------------------------------------------------------------------------------
   Name: TI_OSAL_TaskCreate

   Purpose: Creates a task and starts running it.

   returns: OSAL_INVALID_POINTER if any of the necessary pointers are NULL
            OSAL_ERR_NAME_TOO_LONG if the name of the task is too long to be copied
            OSAL_ERR_INVALID_PRIORITY if the priority is bad
            OSAL_ERR_NO_FREE_IDS if there can be no more tasks created
            OSAL_ERR_NAME_TAKEN if the name specified is already used by a task
            OSAL_ERROR if the operating system calls fail
            OSAL_SUCCESS if success
            
    NOTES: task_id is passed back to the user as the ID. stack_pointer is usually null.
           the flags parameter is unused.

---------------------------------------------------------------------------------------*/
int32 TI_OSAL_TaskCreate (uint32 *task_id, const char *task_name, osal_task_entry function_pointer,
                      const uint32 *stack_pointer, uint32 stack_size, uint32 priority,
                      uint32 flags)
{
    int                return_code = 0;
    pthread_attr_t     custom_attr ;
    struct sched_param priority_holder ;
    int                possible_taskid;
    int                i;
    uint32             local_stack_size;
    int                ret;  
    int                os_priority;

    
    /* Check for NULL pointers */    
    if( (task_name == NULL) || (function_pointer == NULL) || (task_id == NULL) )
    {
        return OSAL_INVALID_POINTER;
    }
    
    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */
    if (strlen(task_name) >= OSAL_MAX_API_NAME)
    {
        return OSAL_ERR_NAME_TOO_LONG;
    }

    /* Check for bad priority */
    if (priority > MAX_PRIORITY)
    {
        return OSAL_ERR_INVALID_PRIORITY;
    }

    /* Change OSAL priority into a priority that will work for this OS */
    os_priority = OSAL_PriorityRemap(priority);
    
    /* Check Parameters */
    pthread_mutex_lock(&OSAL_task_table_mut); 

    for(possible_taskid = 0; possible_taskid < OSAL_MAX_TASKS; possible_taskid++)
    {
        if (OSAL_task_table[possible_taskid].free == TRUE)
        {
            break;
        }
    }

    /* Check to see if the id is out of bounds */
    if( possible_taskid >= OSAL_MAX_TASKS || OSAL_task_table[possible_taskid].free != TRUE)
    {
        pthread_mutex_unlock(&OSAL_task_table_mut);
        return OSAL_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */ 
    for (i = 0; i < OSAL_MAX_TASKS; i++)
    {
        if ((OSAL_task_table[i].free == FALSE) &&
           ( strcmp((char*) task_name, OSAL_task_table[i].name) == 0)) 
        {       
            pthread_mutex_unlock(&OSAL_task_table_mut);
            return OSAL_ERR_NAME_TAKEN;
        }
    }
    
    /* 
    ** Set the possible task Id to not free so that
    ** no other task can try to use it 
    */
    OSAL_task_table[possible_taskid].free = FALSE;
    
    pthread_mutex_unlock(&OSAL_task_table_mut);

    if ( stack_size < PTHREAD_STACK_MIN )
    {
       local_stack_size = PTHREAD_STACK_MIN;
    }
    else
    {
        local_stack_size = stack_size;
    }

   /*
   ** Initialize the pthread_attr structure. 
   ** The structure is used to set the stack and priority
   */
    memset(&custom_attr, 0, sizeof(custom_attr));
    if(pthread_attr_init(&custom_attr))
    {  
        pthread_mutex_lock(&OSAL_task_table_mut); 
        OSAL_task_table[possible_taskid].free = TRUE;
        pthread_mutex_unlock(&OSAL_task_table_mut); 
        printf("pthread_attr_init error in OSAL_TaskCreate, Task ID = %d\n",possible_taskid);
		  perror("pthread_attr_init");
        return(OSAL_ERROR); 
    }

    /*
    ** Set the Stack Size
    */
    if (pthread_attr_setstacksize(&custom_attr, (size_t)local_stack_size ))
    {
        printf("pthread_attr_setstacksize error in OSAL_TaskCreate, Task ID = %d\n",possible_taskid);
        return(OSAL_ERROR); 
    }

    /*
    ** Set the scheduling policy 
    ** On Linux, the schedpolity must be SCHED_FIFO or SCHED_RR to set the priorty
    */
    if (pthread_attr_setschedpolicy(&custom_attr, SCHED_FIFO))
    {
        printf("pthread_attr_setschedpolity error in OSAL_TaskCreate, Task ID = %d\n",possible_taskid);
        return(OSAL_ERROR);
    }
        
    /* 
    ** Set priority 
    */
    memset(&priority_holder, 0, sizeof(priority_holder));
    priority_holder.sched_priority = os_priority;
    ret = pthread_attr_setschedparam(&custom_attr,&priority_holder);
    if(ret !=0)
    {
       printf("pthread_attr_setschedparam error in OSAL_TaskCreate, Task ID = %d\n",possible_taskid);
       return(OSAL_ERROR);
    }

    /*
    ** Create thread
    */
    return_code = pthread_create(&(OSAL_task_table[possible_taskid].id),
                                 &custom_attr,
                                 function_pointer,
                                 (void *)0);
    if (return_code != 0)
    {
        pthread_mutex_lock(&OSAL_task_table_mut); 
        OSAL_task_table[possible_taskid].free = TRUE;
        pthread_mutex_unlock(&OSAL_task_table_mut); 
        printf("pthread_create error in OSAL_TaskCreate, Task ID = %d\n",possible_taskid);
        return(OSAL_ERROR);
    }

    /*
    ** Free the resources that are no longer needed
    */
    return_code = pthread_detach(OSAL_task_table[possible_taskid].id);
    if (return_code !=0)
    {
       pthread_mutex_lock(&OSAL_task_table_mut);
       OSAL_task_table[possible_taskid].free = TRUE;
       pthread_mutex_unlock(&OSAL_task_table_mut);
       printf("pthread_detach error in OSAL_TaskCreate, Task ID = %d\n",possible_taskid);
       return(OSAL_ERROR);
    }

    return_code = pthread_attr_destroy(&custom_attr);
    if (return_code !=0)
    {
       pthread_mutex_lock(&OSAL_task_table_mut);
       OSAL_task_table[possible_taskid].free = TRUE;
       pthread_mutex_unlock(&OSAL_task_table_mut);
       printf("pthread_attr_destroy error in OSAL_TaskCreate, Task ID = %d\n",possible_taskid);
       return(OSAL_ERROR);
    }

    /*
    ** Assign the task ID
    */
    *task_id = possible_taskid;

    /* 
    ** Initialize the table entries 
    */
    pthread_mutex_lock(&OSAL_task_table_mut); 

    OSAL_task_table[possible_taskid].free = FALSE;
    strcpy(OSAL_task_table[*task_id].name, (char*) task_name);
    OSAL_task_table[possible_taskid].creator = OSAL_FindCreator();
    OSAL_task_table[possible_taskid].stack_size = stack_size;
    /* Use the abstracted priority, not the OS one */
    OSAL_task_table[possible_taskid].priority = priority;

    pthread_mutex_unlock(&OSAL_task_table_mut);

    return OSAL_SUCCESS;
}/* end OSAL_TaskCreate */


/*--------------------------------------------------------------------------------------
     Name: TI_OSAL_TaskDelete

    Purpose: Deletes the specified Task and removes it from the OSAL_task_table.

    returns: OSAL_ERR_INVALID_ID if the ID given to it is invalid
             OSAL_ERROR if the OS delete call fails
             OSAL_SUCCESS if success
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_TaskDelete (uint32 task_id)
{    
    int       ret;
    FuncPtr_t FunctionPointer;
    
    /* 
    ** Check to see if the task_id given is valid 
    */
    if (task_id >= OSAL_MAX_TASKS || OSAL_task_table[task_id].free == TRUE)
    {
        return OSAL_ERR_INVALID_ID;
    }

    /*
    ** Call the thread Delete hook if there is one.
    */
    if ( OSAL_task_table[task_id].delete_hook_pointer != NULL)
    {
       FunctionPointer = (FuncPtr_t)(OSAL_task_table[task_id].delete_hook_pointer);
       (*FunctionPointer)();
    }

    /* 
    ** Try to delete the task 
    */
    ret = pthread_cancel(OSAL_task_table[task_id].id);
    if (ret != 0)
    {
        /*debugging statement only*/
        /*printf("FAILED PTHREAD CANCEL %d, %d \n",ret, ESRCH); */
        return OSAL_ERROR;
    }    
    
    /*
    ** Now that the task is deleted, remove its 
    ** "presence" in OSAL_task_table
    */
    pthread_mutex_lock(&OSAL_task_table_mut); 

    OSAL_task_table[task_id].free = TRUE;
    strcpy(OSAL_task_table[task_id].name, "");
    OSAL_task_table[task_id].creator = UNINITIALIZED;
    OSAL_task_table[task_id].stack_size = UNINITIALIZED;
    OSAL_task_table[task_id].priority = UNINITIALIZED;    
    OSAL_task_table[task_id].id = UNINITIALIZED;
    OSAL_task_table[task_id].delete_hook_pointer = NULL;
    
    pthread_mutex_unlock(&OSAL_task_table_mut);

    return OSAL_SUCCESS;
    
}/* end OSAL_TaskDelete */

/*--------------------------------------------------------------------------------------
     Name: TI_OSAL_TaskExit

    Purpose: Exits the calling task and removes it from the OSAL_task_table.

    returns: Nothing 
---------------------------------------------------------------------------------------*/

void TI_OSAL_TaskExit()
{
    uint32 task_id;

    task_id = OSAL_TaskGetId();

    pthread_mutex_lock(&OSAL_task_table_mut); 

    OSAL_task_table[task_id].free = TRUE;
    strcpy(OSAL_task_table[task_id].name, "");
    OSAL_task_table[task_id].creator = UNINITIALIZED;
    OSAL_task_table[task_id].stack_size = UNINITIALIZED;
    OSAL_task_table[task_id].priority = UNINITIALIZED;
    OSAL_task_table[task_id].id = UNINITIALIZED;
    OSAL_task_table[task_id].delete_hook_pointer = NULL;
    
    pthread_mutex_unlock(&OSAL_task_table_mut);

    pthread_exit(NULL);

}/*end TI_OSAL_TaskExit */
/*---------------------------------------------------------------------------------------
   Name: TI_OSAL_TaskDelay

   Purpose: Delay a task for specified amount of milliseconds

   returns: OSAL_ERROR if sleep fails or millisecond = 0
            OSAL_SUCCESS if success
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_TaskDelay(uint32 millisecond )
{
    if (usleep(millisecond * 1000 ) != 0)
    {
        return OSAL_ERROR;
    }
    else
    {
        return OSAL_SUCCESS;
    }
    
}/* end TI_OSAL_TaskDelay */

/*---------------------------------------------------------------------------------------
   Name: TI_OSAL_TaskSetPriority

   Purpose: Sets the given task to a new priority

    returns: OSAL_ERR_INVALID_ID if the ID passed to it is invalid
             OSAL_ERR_INVALID_PRIORITY if the priority is greater than the max 
             allowed
             OSAL_ERROR if the OS call to change the priority fails
             OSAL_SUCCESS if success
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_TaskSetPriority(uint32 task_id, uint32 new_priority)
{
    pthread_attr_t     custom_attr ;
    struct sched_param priority_holder ;
    int                os_priority;

    if(task_id >= OSAL_MAX_TASKS || OSAL_task_table[task_id].free == TRUE)
    {
        return OSAL_ERR_INVALID_ID;
    }

    if (new_priority > MAX_PRIORITY)
    {
        return OSAL_ERR_INVALID_PRIORITY;
    }
   
    /* Change OSAL priority into a priority that will work for this OS */
    os_priority = TI_OSAL_PriorityRemap(new_priority);

    /* 
    ** Set priority
    */
    priority_holder.sched_priority = os_priority ;
    if(pthread_attr_setschedparam(&custom_attr,&priority_holder))
    {
       printf("pthread_attr_setschedparam error in OSAL_TaskSetPriority, Task ID = %lu\n",task_id);
       return(OSAL_ERROR);
    }

    /* Use the abstracted priority, not the OS one */
    /* Change the priority in the table as well */
    OSAL_task_table[task_id].priority = new_priority;

   return OSAL_SUCCESS;
} /* end TI_OSAL_TaskSetPriority */


/*---------------------------------------------------------------------------------------
   Name: TI_OSAL_TaskRegister
  
   Purpose: Registers the calling task id with the task by adding the var to the tcb
            It searches the OSAL_task_table to find the task_id corresponding to the tcb_id
            
   Returns: OSAL_ERR_INVALID_ID if there the specified ID could not be found
            OSAL_ERROR if the OS call fails
            OSAL_SUCCESS if success
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_TaskRegister (void)
{
    int          i;
    int          ret;
    uint32       task_id;
    pthread_t    pthread_id;

    /* 
    ** Get PTHREAD Id
    */
    pthread_id = pthread_self();

    /*
    ** Look our task ID in table 
    */
    for(i = 0; i < OSAL_MAX_TASKS; i++)
    {
       if(OSAL_task_table[i].id == pthread_id)
       {
          break;
       }
    }
    task_id = i;

    if(task_id == OSAL_MAX_TASKS)
    {
        return OSAL_ERR_INVALID_ID;
    }

    /*
    ** Add pthread variable
    */
    ret = pthread_setspecific(thread_key, (void *)task_id);
    if ( ret != 0 )
    {
       printf("TI_OSAL_TaskRegister Failed during pthread_setspecific function\n");
       return(OSAL_ERROR);
    }

    return OSAL_SUCCESS;
}/* end TI_OSAL_TaskRegister */

/*---------------------------------------------------------------------------------------
   Name: TI_OSAL_TaskGetId

   Purpose: This function returns the #defined task id of the calling task

   Notes: The OSAL_task_key is initialized by the task switch if AND ONLY IF the 
          OSAL_task_key has been registered via TI_OSAL_TaskRegister(..).  If this is not 
          called prior to this call, the value will be old and wrong.
---------------------------------------------------------------------------------------*/
uint32 TI_OSAL_TaskGetId (void)
{ 
   void*   task_id;
   int     task_id_int;
   uint32   task_key;
   task_key = 0;
   
   task_id = (void *)pthread_getspecific(thread_key);

   memcpy(& task_id_int,&task_id, sizeof(uint32));
   task_key = task_id_int & 0xFFFF;
   
   return(task_key);
}/* end TI_OSAL_TaskGetId */

/*--------------------------------------------------------------------------------------
    Name: TI_OSAL_TaskGetIdByName

    Purpose: This function tries to find a task Id given the name of a task

    Returns: OSAL_INVALID_POINTER if the pointers passed in are NULL
             OSAL_ERR_NAME_TOO_LONG if th ename to found is too long to begin with
             OSAL_ERR_NAME_NOT_FOUND if the name wasn't found in the table
             OSAL_SUCCESS if SUCCESS
---------------------------------------------------------------------------------------*/

int32 TI_OSAL_TaskGetIdByName (uint32 *task_id, const char *task_name)
{
    uint32 i;

    if (task_id == NULL || task_name == NULL)
    {
       return OSAL_INVALID_POINTER;
    }
    
    /* 
    ** we don't want to allow names too long because they won't be found at all 
    */
    if (strlen(task_name) >= OSAL_MAX_API_NAME)
    {
       return OSAL_ERR_NAME_TOO_LONG;
    }

    for (i = 0; i < OSAL_MAX_TASKS; i++)
    {
        if((OSAL_task_table[i].free != TRUE) &&
                (strcmp(OSAL_task_table[i].name,(char*) task_name) == 0 ))
        {
            *task_id = i;
            return OSAL_SUCCESS;
        }
    }
    /* The name was not found in the table,
    **  or it was, and the task_id isn't valid anymore 
    */
    return OSAL_ERR_NAME_NOT_FOUND;

}/* end TI_OSAL_TaskGetIdByName */            

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_TaskGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info (creator, stack size, priority, name) about the 
             specified task. 

    Returns: OSAL_ERR_INVALID_ID if the ID passed to it is invalid
             OSAL_INVALID_POINTER if the task_prop pointer is NULL
             OSAL_SUCCESS if it copied all of the relevant info over
 
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_TaskGetInfo (uint32 task_id, OSAL_task_prop_t *task_prop)  
{
    /* 
    ** Check to see that the id given is valid 
    */
    if (task_id >= OSAL_MAX_TASKS || OSAL_task_table[task_id].free == TRUE)
    {
       return OSAL_ERR_INVALID_ID;
    }

    if( task_prop == NULL)
    {
       return OSAL_INVALID_POINTER;
    }

    /* put the info into the stucture */
    pthread_mutex_lock(&OSAL_task_table_mut); 

    task_prop -> creator =    OSAL_task_table[task_id].creator;
    task_prop -> stack_size = OSAL_task_table[task_id].stack_size;
    task_prop -> priority =   OSAL_task_table[task_id].priority;
    task_prop -> OStask_id =  (uint32) OSAL_task_table[task_id].id;
    
    strcpy(task_prop-> name, OSAL_task_table[task_id].name);

    pthread_mutex_unlock(&OSAL_task_table_mut);
    
    return OSAL_SUCCESS;
    
} /* end TI_OSAL_TaskGetInfo */

/*--------------------------------------------------------------------------------------
     Name: TI_OSAL_TaskInstallDeleteHandler

    Purpose: Installs a handler for when the task is deleted.

    returns: status
---------------------------------------------------------------------------------------*/

int32 TI_OSAL_TaskInstallDeleteHandler(void *function_pointer)
{
    uint32 task_id;

    task_id = TI_OSAL_TaskGetId();

    if ( task_id >= OSAL_MAX_TASKS )
    {
       return(OSAL_ERR_INVALID_ID);
    }

    pthread_mutex_lock(&OSAL_task_table_mut); 

    if ( OSAL_task_table[task_id].free != FALSE )
    {
       /* 
       ** Somehow the calling task is not registered 
       */
       pthread_mutex_unlock(&OSAL_task_table_mut);
       return(OSAL_ERR_INVALID_ID);
    }

    /*
    ** Install the pointer
    */
    OSAL_task_table[task_id].delete_hook_pointer = function_pointer;    
    
    pthread_mutex_unlock(&OSAL_task_table_mut);

    return(OSAL_SUCCESS);
    
}/*end TI_OSAL_TaskInstallDeleteHandler */

/****************************************************************************************
                                  MUTEX API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_MutSemCreate

    Purpose: Creates a mutex semaphore initially full.

    Returns: OSAL_INVALID_POINTER if mut_id or mut_name are NULL
             OSAL_ERR_NAME_TOO_LONG if the mut_name is too long to be stored
             OSAL_ERR_NO_FREE_IDS if there are no more free mutex Ids
             OSAL_ERR_NAME_TAKEN if there is already a mutex with the same name
             OSAL_SEM_FAILURE if the OS call failed
             OSAL_SUCCESS if success
    
    Notes: the options parameter is not used in this implementation

---------------------------------------------------------------------------------------*/
int32 TI_OSAL_MutSemCreate (uint32 *mut_id, const char *mut_name, uint32 options)
{
    int                 return_code;
    pthread_mutexattr_t mutex_attr ;    
    uint32              possible_mutid;
    uint32              i;      

    /* Check Parameters */
    if (mut_id == NULL || mut_name == NULL)
    {
        return OSAL_INVALID_POINTER;
    }
    
    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */
    if (strlen(mut_name) >= OSAL_MAX_API_NAME)
    {
        return OSAL_ERR_NAME_TOO_LONG;
    }

    pthread_mutex_lock(&OSAL_mut_sem_table_mut);  

    for (possible_mutid = 0; possible_mutid < OSAL_MAX_MUTEXES; possible_mutid++)
    {
        if (OSAL_mut_sem_table[possible_mutid].free == TRUE)    
            break;
    }
    
    if( (possible_mutid == OSAL_MAX_MUTEXES) ||
        (OSAL_mut_sem_table[possible_mutid].free != TRUE) )
    {
        pthread_mutex_unlock(&OSAL_mut_sem_table_mut);
        return OSAL_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */

    for (i = 0; i < OSAL_MAX_MUTEXES; i++)
    {
        if ((OSAL_mut_sem_table[i].free == FALSE) &&
                strcmp ((char*) mut_name, OSAL_mut_sem_table[i].name) == 0)
        {
            pthread_mutex_unlock(&OSAL_mut_sem_table_mut);
            return OSAL_ERR_NAME_TAKEN;
        }
    }

    /* Set the free flag to false to make sure no other task grabs it */

    OSAL_mut_sem_table[possible_mutid].free = FALSE;
    pthread_mutex_unlock(&OSAL_mut_sem_table_mut);

    /* 
    ** initialize the attribute with default values 
    */
    return_code = pthread_mutexattr_init(&mutex_attr); 
    if ( return_code != 0 )
    {
        /* Since the call failed, set free back to true */
        pthread_mutex_lock(&OSAL_mut_sem_table_mut);
        OSAL_mut_sem_table[possible_mutid].free = TRUE;
        pthread_mutex_unlock(&OSAL_mut_sem_table_mut);

       printf("Error: Mutex could not be created. pthread_mutexattr_init failed ID = %lu\n",possible_mutid);
       return OSAL_SEM_FAILURE;
    }


    /* 
    ** create the mutex 
    ** upon successful initialization, the state of the mutex becomes initialized and unlocked 
    */
    return_code =  pthread_mutex_init((pthread_mutex_t *) &OSAL_mut_sem_table[possible_mutid].id, mutex_attr); 
    if ( return_code != 0 )
    {
        /* Since the call failed, set free back to true */
        pthread_mutex_lock(&OSAL_mut_sem_table_mut);
        OSAL_mut_sem_table[possible_mutid].free = TRUE;
        pthread_mutex_unlock(&OSAL_mut_sem_table_mut);

       printf("Error: Mutex could not be created. ID = %lu\n",possible_mutid);
       return OSAL_SEM_FAILURE;
    }
    else
    {
       /*
       ** Mark mutex as initialized
       */
       *mut_id = possible_mutid;
    
       pthread_mutex_lock(&OSAL_mut_sem_table_mut);  

       strcpy(OSAL_mut_sem_table[*mut_id].name, (char*) mut_name);
       OSAL_mut_sem_table[*mut_id].free = FALSE;
       OSAL_mut_sem_table[*mut_id].creator = OSAL_FindCreator();
    
       pthread_mutex_unlock(&OSAL_mut_sem_table_mut);

       return OSAL_SUCCESS;
    }

}/* end TI_OSAL_MutexSemCreate */

/*--------------------------------------------------------------------------------------
     Name: TI_OSAL_MutSemDelete

    Purpose: Deletes the specified Mutex Semaphore.
    
    Returns: OSAL_ERR_INVALID_ID if the id passed in is not a valid mutex
             OSAL_SEM_FAILURE if the OS call failed
             OSAL_SUCCESS if success

    Notes: The mutex must be full to take it, so we have to check for fullness

---------------------------------------------------------------------------------------*/

int32 TI_OSAL_MutSemDelete (uint32 mut_id)
{
    int status=-1;

    /* Check to see if this mut_id is valid   */
    if (mut_id >= OSAL_MAX_MUTEXES || OSAL_mut_sem_table[mut_id].free == TRUE)
    {
        return OSAL_ERR_INVALID_ID;
    }

    status = pthread_mutex_destroy( &(OSAL_mut_sem_table[mut_id].id)); /* 0 = success */   
    
    if( status != 0)
    {
        return OSAL_SEM_FAILURE;
    }
    /* Delete its presence in the table */
   
    pthread_mutex_lock(&OSAL_mut_sem_table_mut);  

    OSAL_mut_sem_table[mut_id].free = TRUE;
    strcpy(OSAL_mut_sem_table[mut_id].name , "");
    OSAL_mut_sem_table[mut_id].creator = UNINITIALIZED;
    
    pthread_mutex_unlock(&OSAL_mut_sem_table_mut);
    
    return OSAL_SUCCESS;

}/* end TI_OSAL_MutSemDelete */

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_MutSemGive

    Purpose: The function releases the mutex object referenced by mut_id.The 
             manner in which a mutex is released is dependent upon the mutex's type 
             attribute.  If there are threads blocked on the mutex object referenced by 
             mutex when this function is called, resulting in the mutex becoming 
             available, the scheduling policy shall determine which thread shall 
             acquire the mutex.

    Returns: OSAL_SUCCESS if success
             OSAL_SEM_FAILURE if the semaphore was not previously  initialized 
             OSAL_ERR_INVALID_ID if the id passed in is not a valid mutex

---------------------------------------------------------------------------------------*/

int32 TI_OSAL_MutSemGive ( uint32 mut_id )
{
    uint32 ret_val ;

    /* Check Parameters */

    if(mut_id >= OSAL_MAX_MUTEXES || OSAL_mut_sem_table[mut_id].free == TRUE)
    {
        return OSAL_ERR_INVALID_ID;
    }

    /*
    ** Unlock the mutex
    */
    if(pthread_mutex_unlock(&(OSAL_mut_sem_table[mut_id].id)))
    {
        ret_val = OSAL_SEM_FAILURE ;
    }
    else
    {
        ret_val = OSAL_SUCCESS ;
    }
    
    return ret_val;
} /* end TI_OSAL_MutSemGive */

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_MutSemTake

    Purpose: The mutex object referenced by mut_id shall be locked by calling this
             function. If the mutex is already locked, the calling thread shall
             block until the mutex becomes available. This operation shall return
             with the mutex object referenced by mutex in the locked state with the              
             calling thread as its owner.

    Returns: OSAL_SUCCESS if success
             OSAL_SEM_FAILURE if the semaphore was not previously initialized or is 
             not in the array of semaphores defined by the system
             OSAL_ERR_INVALID_ID the id passed in is not a valid mutex
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_MutSemTake ( uint32 mut_id )
{
    int status;

    /* 
    ** Check Parameters
    */  
    if(mut_id >= OSAL_MAX_MUTEXES || OSAL_mut_sem_table[mut_id].free == TRUE)
    {
       return OSAL_ERR_INVALID_ID;
    }
 
    /*
    ** Lock the mutex - unlike the sem calls, the pthread mutex call
    ** should not be interrupted by a signal
    */
    status = pthread_mutex_lock(&(OSAL_mut_sem_table[mut_id].id));
    if( status == EINVAL )
    {
      return OSAL_SEM_FAILURE ;
    }
    else if ( status == EDEADLK )
    {
       printf("Task would deadlock--nested mutex call!\n");
       return OSAL_SUCCESS ;
    }
    else
    {
      return OSAL_SUCCESS;
    }

}
/*--------------------------------------------------------------------------------------
    Name: TI_OSAL_MutSemGetIdByName

    Purpose: This function tries to find a mutex sem Id given the name of a mut_sem
             The id is returned through mut_id

    Returns: OSAL_INVALID_POINTER is mutid or mut_name are NULL pointers
             OSAL_ERR_NAME_TOO_LONG if the name given is to long to have been stored
             OSAL_ERR_NAME_NOT_FOUND if the name was not found in the table
             OSAL_SUCCESS if success
             
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_MutSemGetIdByName (uint32 *mut_id, const char *mut_name)
{
 
 /* Not implemented */
    return OSAL_ERR_NAME_NOT_FOUND;

}/* end OSAL_MutSemGetIdByName */

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_MutSemGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info( name and creator) about the specified mutex
             semaphore.
             
    Returns: OSAL_ERR_INVALID_ID if the id passed in is not a valid semaphore 
             OSAL_INVALID_POINTER if the mut_prop pointer is null
             OSAL_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 TI_OSAL_MutSemGetInfo (uint32 mut_id, OSAL_mut_sem_prop_t *mut_prop)  
{
    /* Check to see that the id given is valid */
    
    if (mut_id >= OSAL_MAX_MUTEXES || OSAL_mut_sem_table[mut_id].free == TRUE)
    {
        return OSAL_ERR_INVALID_ID;
    }

    if (mut_prop == NULL)
    {
        return OSAL_INVALID_POINTER;
    }
    
    /* put the info into the stucture */    
    
    pthread_mutex_lock(&OSAL_mut_sem_table_mut);  

    mut_prop -> creator =   OSAL_mut_sem_table[mut_id].creator;
    strcpy(mut_prop-> name, OSAL_mut_sem_table[mut_id].name);

    pthread_mutex_unlock(&OSAL_mut_sem_table_mut);
    
    return OSAL_SUCCESS;
    
} /* end TI_OSAL_BinSemGetInfo */


/****************************************************************************************
                                  CONDITIONAL VARIABLEs API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_ConVarCreate

    Purpose: Creates a conditinal variable.

    Returns: OSAL_INVALID_POINTER if mut_id or cv_name are NULL
             OSAL_ERR_NAME_TOO_LONG if the cv_name is too long to be stored
             OSAL_ERR_NO_FREE_IDS if there are no more free CV Ids
             OSAL_ERR_NAME_TAKEN if there is already a CV with the same name
             OSAL_CV_FAILURE if the OS call failed
             OSAL_SUCCESS if success
    
    Notes: the options parameter is not used in this implementation

---------------------------------------------------------------------------------------*/
int32 TI_OSAL_ConVarCreate (uint32 *cv_id, const char *cv_name, uint32 options)
{
    int                 return_code;
    uint32              possible_cvid;
    uint32              i;      

    /* Check Parameters */
    if (cv_id == NULL || cv_name == NULL)
    {
        return OSAL_INVALID_POINTER;
    }
    
    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */
    if (strlen(cv_name) >= OSAL_MAX_API_NAME)
    {
        return OSAL_ERR_NAME_TOO_LONG;
    }

    pthread_mutex_lock(&OSAL_con_var_table_mut);  

    for (possible_cvid = 0; possible_cvid < OSAL_MAX_CVS; possible_cvid++)
    {
        if (OSAL_con_var_table[possible_cvid].free == TRUE)    
            break;
    }
    
    if( (possible_cvid == OSAL_MAX_CVS) ||
        (OSAL_con_var_table[possible_cvid].free != TRUE) )
    {
        pthread_mutex_unlock(&OSAL_con_var_table_mut);
        return OSAL_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */

    for (i = 0; i < OSAL_MAX_CVS; i++)
    {
        if ((OSAL_con_var_table[i].free == FALSE) &&
                strcmp ((char*) mut_name, OSAL_con_var_table[i].name) == 0)
        {
            pthread_mutex_unlock(&OSAL_con_var_table_mut);
            return OSAL_ERR_NAME_TAKEN;
        }
    }

    /* Set the free flag to false to make sure no other task grabs it */

    OSAL_con_var_table[possible_cvid].free = FALSE;
    pthread_mutex_unlock(&OSAL_con_var_table_mut);

    
    /* 
    ** create the CV 
    ** upon successful initialization, the state of the CV becomes initialized 
	*/
    return_code =  pthread_cond_init(&OSAL_con_var_table[possible_cvid].id, NULL); 
    if ( return_code != 0 )
    {
        /* Since the call failed, set free back to true */
        pthread_mutex_lock(&OSAL_con_var_table_mut);
        OSAL_mut_sem_table[possible_cvid].free = TRUE;
        pthread_mutex_unlock(&OSAL_con_var_table_mut);

       printf("Error: CV could not be created. ID = %lu\n",possible_cvid);
       return OSAL_SEM_FAILURE;
    }
    else
    {
       /*
       ** Mark conditional variable as initialized
       */
       *cv_id = possible_cvid;
    
       pthread_mutex_lock(&OSAL_con_var_table_mut);  

       strcpy(OSAL_con_var_table[*cv_id].name, (char*) cv_name);
       OSAL_con_var_table[*cv_id].free = FALSE;
       OSAL_con_var_table[*cv_id].creator = OSAL_FindCreator();
    
       pthread_mutex_unlock(&OSAL_con_var_table_mut);

       return OSAL_SUCCESS;
    }

}/* end TI_OSAL_ConVarCreate */

/*--------------------------------------------------------------------------------------
     Name: TI_OSAL_ConVarDelete

    Purpose: Deletes the specified Conditional variable.
    
    Returns: OSAL_ERR_INVALID_ID if the id passed in is not a valid Conditional variable
             OSAL_SEM_FAILURE if the OS call failed
             OSAL_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 TI_OSAL_ConVarDelete (uint32 cv_id)
{
    int status=-1;

    /* Check to see if this cv_id is valid   */
    if (cv_id >= OSAL_MAX_CVS || OSAL_con_var_table[cv_id].free == TRUE)
    {
        return OSAL_ERR_INVALID_ID;
    }

    status = pthread_cond_destroy( &(OSAL_con_var_table[cv_id].id)); /* 0 = success */   
    
    if( status != 0)
    {
        return OSAL_SEM_FAILURE;
    }
    /* Delete its presence in the table */
   
    pthread_mutex_lock(&OSAL_con_var_table_mut);  

    OSAL_con_var_table[cv_id].free = TRUE;
    strcpy(OSAL_con_var_table[cv_id].name , "");
    OSAL_con_var_table[cv_id].creator = UNINITIALIZED;
    
    pthread_mutex_unlock(&OSAL_con_var_table_mut);
    
    return OSAL_SUCCESS;

}/* end TI_OSAL_ConVarDelete */

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_ConVarSignal

    Purpose: .

    Returns: OSAL_SUCCESS if success
             OSAL_SEM_FAILURE if the conditional varibale was not previously initialized
             OSAL_ERR_INVALID_ID the id passed in is not a valid mutex
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_ConVarSignal ( uint32 cv_id )
{
    int status;

    /* 
    ** Check Parameters
    */  
    if(cv_id >= OSAL_MAX_CVS || OSAL_con_var_table[cv_id].free == TRUE)
    {
       return OSAL_ERR_INVALID_ID;
    }
 
    
    status = pthread_cond_signal(&(OSAL_con_var_table[cv_id].id));
	
     if( status != NULL )
    {
      return OSAL_SEM_FAILURE ;
    }
    else
    {
      return OSAL_SUCCESS;
    }

}


/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_ConVarBroadcast

    Purpose: .

    Returns: OSAL_SUCCESS if success
             OSAL_SEM_FAILURE if the conditional varibale was not previously initialized
             OSAL_ERR_INVALID_ID the id passed in is not a valid mutex
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_ConVarBroadcast ( uint32 cv_id )
{
    int status;

    /* 
    ** Check Parameters
    */  
    if(cv_id >= OSAL_MAX_CVS || OSAL_con_var_table[cv_id].free == TRUE)
    {
       return OSAL_ERR_INVALID_ID;
    }
 
   
    status = pthread_cond_broadcast(&(OSAL_con_var_table[cv_id].id));
	
    if( status != NULL )
    {
      return OSAL_SEM_FAILURE ;
    }
    else
    {
      return OSAL_SUCCESS;
    }

}

/*---------------------------------------------------------------------------------------
    Name: TI_OSAL_ConVarWait

    Purpose: .

    Returns: OSAL_SUCCESS if success
             OSAL_SEM_FAILURE if the conditional varibale was not previously initialized
             OSAL_ERR_INVALID_ID the id passed in is not a valid mutex
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_ConVarWait (uint32 cv_id, uint32 mut_id);  
{
    int status;

    /* 
    ** Check Parameters
    */  
    if((cv_id >= OSAL_MAX_CVS || OSAL_con_var_table[cv_id].free == TRUE)
	   || (mut_id >= OSAL_MAX_MUTEXES || OSAL_mut_sem_table[mut_id].free == TRUE))
    {
       return OSAL_ERR_INVALID_ID;
    }
 
    
	status = pthread_cond_wait(&(OSAL_con_var_table[cv_id].id),&(OSAL_mut_sem_table[mut_id].id));
    
    if( status != NULL )
    {
      return OSAL_SEM_FAILURE ;
    }
    else
    {
      return OSAL_SUCCESS;
    }

}


/*--------------------------------------------------------------------------------------
 * HELPER FUNCTIONS FOR THE USAGE OF OSAL
 *
---------------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
 * Name: OSAL_printf 
 * 
 * Purpose: This function abstracts out the printf type statements. This is 
 *          useful for using OS- specific thats that will allow non-polled
 *          print statements for the real time systems. 
 ---------------------------------------------------------------------------*/
void TI_OSAL_printf( const char *String, ...)
{
    va_list     ptr;
    char msg_buffer [OSAL_BUFFER_SIZE];

    if ( OSAL_printf_enabled == TRUE )
    {
       va_start(ptr,String);
       vsnprintf(&msg_buffer[0], (size_t)OSAL_BUFFER_SIZE, String, ptr);
       va_end(ptr);
    
       msg_buffer[OSAL_BUFFER_SIZE -1] = '\0';
       printf("%s", &msg_buffer[0]);
    }

    
}/* end TI_OSAL_printf*/

/* ---------------------------------------------------------------------------
 * Name: TI_OSAL_printf_disable
 * 
 * Purpose: This function disables the output to the UART from OSAL_printf.  
 *
 ---------------------------------------------------------------------------*/
void TI_OSAL_printf_disable(void)
{
   TI_OSAL_printf_enabled = FALSE;
}/* end TI_OSAL_printf_disable*/

/* ---------------------------------------------------------------------------
 * Name: OSAL_printf_enable
 * 
 * Purpose: This function enables the output to the UART through OSAL_printf.  
 *
 ---------------------------------------------------------------------------*/
void TI_OSAL_printf_enable(void)
{
   TI_OSAL_printf_enabled = TRUE;
}/* end TI_OSAL_printf_enable*/

/*---------------------------------------------------------------------------------------
 *  Name: TI_OSAL_GetErrorName()
---------------------------------------------------------------------------------------*/
int32 TI_OSAL_GetErrorName(int32 error_num, os_err_name_t * err_name)
{
    os_err_name_t local_name;
    uint32 return_code;

    return_code = OSAL_SUCCESS;

    switch (error_num)
    {
        case OSAL_SUCCESS:
            strcpy(local_name,"OSAL_SUCCESS"); break;
        case OSAL_ERROR:
            strcpy(local_name,"OSAL_ERROR"); break;
        case OSAL_INVALID_POINTER:
            strcpy(local_name,"OSAL_INVALID_POINTER"); break;
        case OSAL_ERROR_ADDRESS_MISALIGNED:
            strcpy(local_name,"OSAL_ADDRESS_MISALIGNED"); break;
        case OSAL_ERROR_TIMEOUT:
            strcpy(local_name,"OSAL_ERROR_TIMEOUT"); break;
        case OSAL_SEM_FAILURE:
            strcpy(local_name,"OSAL_SEM_FAILURE"); break;
        case OSAL_SEM_TIMEOUT:
            strcpy(local_name,"OSAL_SEM_TIMEOUT"); break;
        case OSAL_ERR_NAME_TOO_LONG:
            strcpy(local_name,"OSAL_ERR_NAME_TOO_LONG"); break;
        case OSAL_ERR_NO_FREE_IDS:
            strcpy(local_name,"OSAL_ERR_NO_FREE_IDS"); break;
        case OSAL_ERR_NAME_TAKEN:
            strcpy(local_name,"OSAL_ERR_NAME_TAKEN"); break;
        case OSAL_ERR_INVALID_ID:
            strcpy(local_name,"OSAL_ERR_INVALID_ID"); break;
        case OSAL_ERR_NAME_NOT_FOUND:
            strcpy(local_name,"OSAL_ERR_NAME_NOT_FOUND"); break;
        case OSAL_ERR_INVALID_PRIORITY:
            strcpy(local_name,"OSAL_ERR_INVALID_PRIORITY"); break;

        default: strcpy(local_name,"ERROR_UNKNOWN");
                 return_code = OSAL_ERROR;
    }

    strcpy((char*) err_name, local_name);

    return return_code;
}

/*--------------------------------------------------------------------------------------
 * LOCAL HELPER FUNCTIONS 
 *
---------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------
 * uint32 TI_OSAL_FindCreator
 * purpose: Finds the creator of the calling thread
---------------------------------------------------------------------------------------*/
uint32 TI_OSAL_FindCreator(void)
{
    pthread_t    pthread_id;
    uint32 i;  
   
    pthread_id = pthread_self();
    /* 
    ** Get PTHREAD Id
    */
    for (i = 0; i < OSAL_MAX_TASKS; i++)
    {
        if (pthread_equal(pthread_id, OSAL_task_table[i].id) != 0 )
        {
            break;
        }
    }

    return i;
}



/*----------------------------------------------------------------------------
 * Name: TI_OSAL_PriorityRemap
 *
 * Purpose: Remaps the OSAL priority into one that is viable for this OS
----------------------------------------------------------------------------*/

int32 TI_OSAL_PriorityRemap(uint32 InputPri)
{
    int OutputPri;
    int pmax = sched_get_priority_max( SCHED_FIFO );
    int pmin = sched_get_priority_min( SCHED_FIFO );
    int prange = abs((pmax - pmin)  +1);
    int numbins, offset;
    int IsMinNegative = 0;
    int MinNegOffset = 0;
    int IsMaxNegative = 0;
    int MaxNegOffset = 0;
    int InputRev;

    /* If an end point is negative, adjust the range upward so that there is no negative */
    if (pmin < 0)
    {
        IsMinNegative = 1;
        MinNegOffset = -pmin;
        pmin += MinNegOffset;
        pmax += MinNegOffset;
    } 

    if (pmax < 0)
    {
        IsMaxNegative = 1;
        MaxNegOffset = -pmax;
        pmin += MaxNegOffset;
        pmax += MaxNegOffset;
    }

    /* calculate the number of 'bins' to map the OSAL priorities into.
     * Since the Underlying OS will have AT MOST as many priority levels as
     * the OSAL (256 values), then we will be pigeon-holing a larger range 
     * (OSAL priorities) into a small range (OS priorities), or the ranges are
     * equal, which is OK too.
     */

    numbins = MAX_PRIORITY/prange;

    /* If we are more than half way to making a new bin, add another one */
   if (MAX_PRIORITY % prange > prange/2)
   {
      numbins++;
   } 
    

    /* Since the OSAL priorities have 0 as the highest and 255 as the lowest,
     * we need to reverse this so that the numerically higher number is a higher priority
     * to work with the OS's
     */

     InputRev = MAX_PRIORITY - InputPri;
       
     /* calculate the offset from the min value */
     offset = InputRev / numbins ;
          
     OutputPri = pmin + offset ;  

     /* take care of extraneous cases at ends, if they occur. */
     if (OutputPri > pmax)
     {
         OutputPri = pmax;
     }

     if ( OutputPri < pmin)
     {
            OutputPri = pmin;
     }

     /* if an end point was negative, shift it back */
     if (IsMinNegative == 1)
     {
         OutputPri -= MinNegOffset;
     }

     if (IsMaxNegative == 1)
     {
         OutputPri -= MaxNegOffset;
     }


    return OutputPri;
}/*end TI_OSAL_PriorityRemap*/

/* ---------------------------------------------------------------------------
 * Name: TI_OSAL_ThreadKillHandler
 * 
 * Purpose: This function allows for a task to be deleted when TI_OSAL_TaskDelete
 * is called  
----------------------------------------------------------------------------*/

void    TI_OSAL_ThreadKillHandler(int sig)
{
    pthread_exit(NULL);

}/*end TI_OSAL_ThreadKillHandler */


