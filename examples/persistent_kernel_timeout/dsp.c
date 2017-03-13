#include <stdio.h>
#include <stdint.h>
#include <dsp_c.h>
#include <c6x.h>
#include "ti/sysbios/BIOS.h"
#include "ti/sysbios/knl/Semaphore.h"
#include "ti/sysbios/knl/Task.h"
#include "shared.h"

#define T1_STKSZ 4096

/******************************************************************************
* compute - The computation function
*
*     arg0 is assumed to be a pointer to a semaphore handle
*     arg1 is unused.
******************************************************************************/
void compute(UArg arg0, UArg arg1)
{
    __cycle_delay(15000000);                   // simulate 20 ms computation
    Semaphore_post(*(Semaphore_Handle*) arg0); // signal completion
}

/******************************************************************************
* ccode - Called from a simple OpenCL C kernel wrapper
******************************************************************************/
void ccode(uint32_t timeout_ms, uint32_t *completion_code)
{
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
    * Since the new task priority will be lower than this function which runs 
    * at a priority level between 5 and 10, the task will not start until this
    * function blocks with the semaphore pend.
    *------------------------------------------------------------------------*/
    taskParams.priority = 4;

    /*-------------------------------------------------------------------------
    * Create a semaphore that we will wait on and the compute function will
    * post. The address of the semaphore will be passed to the task function.
    *------------------------------------------------------------------------*/
    Semaphore_Handle time_to_return = Semaphore_create(0, NULL, NULL);
    if (!time_to_return) { *completion_code = APP_FAIL_SEMAPHORE; return; }

    taskParams.arg0                 = (UArg)&time_to_return;
    Task_Handle      task_compute   = Task_create(compute, &taskParams, NULL);
    if (!task_compute) { *completion_code = APP_FAIL_TASK; return; }

    /*-------------------------------------------------------------------------
    * Wait for either timeout_ms to expire or the semaphore to be posted and
    * set the completion code status based on which event occurred first.
    *------------------------------------------------------------------------*/
    Bool compute_completed = Semaphore_pend(time_to_return, timeout_ms);

    /*-------------------------------------------------------------------------
    * Cleanup the SysBios resources
    *------------------------------------------------------------------------*/
    Task_delete      (&task_compute);
    Semaphore_delete (&time_to_return);

    *completion_code = compute_completed ? APP_OK : APP_TIMEOUT;
}
