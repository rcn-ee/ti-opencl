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

/******************************************************************************
* compute - An alternate computation function
*
*     arg0 is assumed to be a pointer to a Semaphore_Handle.
*     arg1 is unused.
*
*   Forever: wait on semaphore then increment counter
******************************************************************************/
void compute(UArg arg0, UArg arg1)
{
    Semaphore_Handle *sem = (Semaphore_Handle*) arg0;

    while (1)
    {
        Semaphore_pend(*sem, BIOS_WAIT_FOREVER);
        counter++;
    }
}

/*-----------------------------------------------------------------------------
* ccode - Called from a kernel wrapper
*
*    Start another task running and then ping / pong back and forth between 
*    this function task and the other function task using a semaphore.
*----------------------------------------------------------------------------*/
void ccode(uint32_t *completion_code)
{
    counter = 0;

    /*-------------------------------------------------------------------------
    * Create a task to run the compute routine
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
    taskParams.stackSize = T1_STKSZ;
    taskParams.stack     = malloc(T1_STKSZ);
    if (!taskParams.stack) { *completion_code = APP_FAIL_STACK; return; }

    /*-------------------------------------------------------------------------
    * Since the new task priority will be higher than this function which runs 
    * at a priority level between 5 and 10, the task start immediately after 
    * creation function blocks with the semaphore pend.
    *------------------------------------------------------------------------*/
    taskParams.priority = 11;

    /*-------------------------------------------------------------------------
    * Create a semaphore that we will post and the compute function will
    * pend on. The address of the semaphore will be passed to the task function.
    *------------------------------------------------------------------------*/
    Semaphore_Handle sem = Semaphore_create(0, NULL, NULL);
    if (!sem) { *completion_code = APP_FAIL_SEMAPHORE; return; }

    taskParams.arg0      = (UArg)&sem;
    Task_Handle task     = Task_create(compute, &taskParams, NULL);
    if (!task) { *completion_code = APP_FAIL_TASK; return; }

    /*-------------------------------------------------------------------------
    * for multiple iterations, ping/pong between tasks
    *------------------------------------------------------------------------*/
    int i;
    uint32_t c1 = __clock();
    for (i = 0; i < 1000; ++i)
    {
        counter++;
        Semaphore_post(sem);
    }
    uint32_t c2 = __clock() - c1;

    printf("%u task switches in %u cycles ==> %u cycles per switch\n", 
            counter, c2, c2/counter);

    /*-------------------------------------------------------------------------
    * Cleanup the SysBios resources
    *------------------------------------------------------------------------*/
    Task_delete      (&task);
    Semaphore_delete (&sem);

    *completion_code = APP_OK;
}
