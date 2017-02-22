#include <stdio.h>
#include <stdint.h>
#include <dsp_c.h>
#include <c6x.h>
#include "ti/sysbios/BIOS.h"
#include "ti/sysbios/knl/Clock.h"
#include "ti/sysbios/knl/Task.h"
#include "shared.h"

/******************************************************************************
* Clock tick handler - increment global variable counter every time invoked 
*
*    arg0 is assumed to be a pointer to a uint32_t value
******************************************************************************/
void clkFxn(UArg arg0)
{
    uint32_t *cntr = (uint32_t*)arg0;
    (*cntr) += 1;
}

volatile unsigned counter = 0;

/******************************************************************************
* Called from OpenCL kernel wrapper
******************************************************************************/
void start_clock(uint32_t *completion_code)
{
    /*-------------------------------------------------------------------------
    * Create a clock function that will tick every 1 millisecond.
    *------------------------------------------------------------------------*/
    const uint32_t CLK_PERIOD_MS = 1;
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.arg       = (UArg)&counter;
    clkParams.period    = CLK_PERIOD_MS;
    clkParams.startFlag = TRUE;
    Clock_Handle clk    = Clock_create(clkFxn, CLK_PERIOD_MS, &clkParams, NULL);
    if (!clk) { *completion_code = APP_FAIL_CLOCK; return; }

    /*-------------------------------------------------------------------------
    * Sleep this task.  Meanwhile the clock function will be invoked every 1ms.
    *------------------------------------------------------------------------*/
    Task_sleep(1000);

    /*-------------------------------------------------------------------------
    * Delete the clock, report results
    *------------------------------------------------------------------------*/
    Clock_delete(&clk);
    printf("The clock ticked %u times in 1 sec\n", counter);
    *completion_code = APP_OK;
}
