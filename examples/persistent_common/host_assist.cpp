#include <stdio.h>
#include <stdlib.h>
#include "host_assist.h"
#include "shared.h"

/*-----------------------------------------------------------------------------
* Given a completion code, print a descriptive message
*----------------------------------------------------------------------------*/
void print_completion_code(uint32_t completion_code)
{
    const char *msg;
    switch ((kernel_status_codes) completion_code)
    {
        case APP_OK:              msg = "DSP status OK";
                                  break;
        case APP_FAIL:            msg = "DSP code failed";
                                  break;
        case APP_TIMEOUT:         msg = "DSP code timed out";
                                  break;
        case APP_FAIL_STACK:      msg = "DSP code failed stack allocation";
                                  break;
        case APP_FAIL_TASK:       msg = "DSP code failed task creation";
                                  break;
        case APP_FAIL_SEMAPHORE:  msg = "DSP code failed semaphore creation";
                                  break;
        case APP_FAIL_CLOCK:      msg = "DSP code failed clock creation";
                                  break;
        default:                  msg = "DSP code unknown";
                                  break;
    }
    printf("%s\n", msg);
}

/******************************************************************************
* Given an OpenCL Device, Abort if the device is not part of the AM57x platform
******************************************************************************/
int assert_am57x(cl::Device &device)
{
    cl::Platform platform;
    std::string  platform_name;

    device.getInfo  (CL_DEVICE_PLATFORM, &platform);
    platform.getInfo(CL_PLATFORM_NAME,   &platform_name);

    /*-------------------------------------------------------------------------
    * Does the platform name not contain the substring 'TI AM57x'
    *------------------------------------------------------------------------*/
    if (platform_name.find("TI AM57x") == std::string::npos)
    {
        printf("This example is only supported on AM57x platforms. Exiting.\n");
        exit(0);
    }
}
