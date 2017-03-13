#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dsp_c.h>
#include "ti/sysbios/BIOS.h"
#include "ti/sysbios/knl/Task.h"
#include "ti/sysbios/knl/Semaphore.h"
#include "shared.h"

#define T1_STKSZ 4096

volatile unsigned counter;
Task_Handle task1;

/******************************************************************************
* compute - The computation function
*
*     arg0 is unused.
*     arg1 is unused.
*
*   Simply increment a counter.
******************************************************************************/
void compute(UArg arg0, UArg arg1)
{
    while (1) counter++;
}


/******************************************************************************
* start_task - A function called from a kernel that will start a persistent
* task that will span this start function to a subsequent stop function.
******************************************************************************/
void start_task(uint32_t *completion_code)
{
    /*-------------------------------------------------------------------------
    * Create sa task to run the compute routine
    *------------------------------------------------------------------------*/
    Task_Params  taskParams;
    Task_Params_init(&taskParams);
    taskParams.instance->name = "compute";

    /*-------------------------------------------------------------------------
    * The task is provided with a new stack that we simply malloc from the heap.
    * OpenCL kernels and functions called from kernels are provided with a
    * limited heap. If the stack space required for the task is greater than
    * the malloc can provide, then a buffer passed to the kernel can provide
    * the underlying memory for the stack.
    *------------------------------------------------------------------------*/
    taskParams.stackSize  = T1_STKSZ;
    taskParams.stack      = malloc(T1_STKSZ);
    if (!taskParams.stack) { *completion_code = APP_FAIL_STACK; return; }

    /*-------------------------------------------------------------------------
    * Since the new task priority will be lower than this function which runs 
    * at a priority level between 5 and 10, the task will not start until this
    * function ends and the opencl runtime pends waiting for another command
    * from the host.
    *------------------------------------------------------------------------*/
    taskParams.priority   = 4;
    task1                 = Task_create(compute, &taskParams, NULL);
    if (!task1) { *completion_code = APP_FAIL_TASK; return; }

    /*-------------------------------------------------------------------------
    * Initialize the counter that the persistent task will increment and 
    * print an indicator that the persistent task can now being.
    *------------------------------------------------------------------------*/
    counter = 0;
    printf("Starting DSP task and returning to Host\n");
    *completion_code = APP_OK;
}

/******************************************************************************
* stop_task - will be dispatched in a separate OpenCL enqueue command.  This 
* functions job is to simply end the persistent task.
******************************************************************************/
void stop_task(uint32_t *completion_code)
{
    Task_delete (&task1);
    printf("Stopped DSP task and returning to Host: counter(%u)\n", counter);
    *completion_code = APP_OK;
}
