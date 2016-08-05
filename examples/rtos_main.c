/* xdctools and sysbios header files */
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#define PRI_HIGH        5
void ocl_main(UArg arg0, UArg arg1);

// Use System_exit to ensure the host program terminiates
#define RETURN(x) do { System_exit(x);} while (0);

Int main(Int argc, Char* argv[])
{
    Error_Block     eb;
    Task_Params     taskParams;

    Log_print0(Diags_ENTRY, "--> main:");

    /* must initialize the error block before using it */
    Error_init(&eb);

    /* create server thread */
    Task_Params_init(&taskParams);
    taskParams.instance->name = String("OCL_HostApp");
    taskParams.priority = PRI_HIGH;
    taskParams.arg0 = (UArg)argc;
    taskParams.arg1 = (UArg)argv;
    taskParams.stackSize = 0x20000;
    Task_create(ocl_main, &taskParams, &eb);

    if (Error_check(&eb)) {
        System_abort("main: failed to create OpenCL HostApp thread");
    }

    /* start scheduler, this never returns */
    BIOS_start();

    /* should never get here */
    Log_print0(Diags_EXIT, "<-- main:");
    return (0);
}

