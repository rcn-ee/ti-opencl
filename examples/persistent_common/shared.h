#ifndef _SHARED_H_
#define _SHARED_H_

typedef enum 
{
    APP_OK,
    APP_FAIL,
    APP_TIMEOUT,
    APP_FAIL_STACK,
    APP_FAIL_TASK,
    APP_FAIL_SEMAPHORE,
    APP_FAIL_CLOCK,
} kernel_status_codes;

#endif // _SHARED_H_
