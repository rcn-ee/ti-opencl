#include <stdio.h>
#include <dsp_c.h>
#include <c6x.h>
#include "ti/sysbios/BIOS.h"
#include "ti/sysbios/knl/Clock.h"
#include "ti/sysbios/knl/Task.h"
#include "shared.h"

volatile unsigned counter = 0;
Clock_Handle      clk;

/******************************************************************************
* Clock tick handler - increment global variable counter every time invoked 
*
*    arg0 is assumed to be a pointer to a uint32_t value
******************************************************************************/
void clkFxn(UArg arg0)
{
    UInt32 *cntr = (UInt32*)arg0;
    *cntr += 1;
}

/******************************************************************************
* start_clock - Called from OpenCL kernel wrapper
******************************************************************************/
void start_clock(uint32_t *completion_code)
{
    /*-------------------------------------------------------------------------
    * Create a clock function that will tick every 1 millisecond.
    *------------------------------------------------------------------------*/
    const UInt32 CLK_PERIOD_MS = 1;
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.arg       = (UArg)&counter;
    clkParams.period    = CLK_PERIOD_MS;
    clkParams.startFlag = TRUE;
    clk                 = Clock_create(clkFxn, CLK_PERIOD_MS, &clkParams, NULL);
    if (!clk) {*completion_code = APP_FAIL_CLOCK; return; }

    *completion_code    = APP_OK;
}

/*-----------------------------------------------------------------------------
* stop_clock - Called from OpenCL kernel wrapper
*----------------------------------------------------------------------------*/
void stop_clock(uint32_t *completion_code)
{
    Clock_delete(&clk);
    printf("The clock ticked %u times\n", counter);
    *completion_code = APP_OK;
}
