#ifndef _EDMA_H_
#define _EDMA_H_
#include <ti/sdo/fc/edmamgr/edmamgr.h>

typedef enum
{
   EV_NOT_ALLOCATED,
   EV_AVAILABLE,
   EV_IN_USE,
   EV_MEMCPY
} copy_event_status;

typedef struct
{
   copy_event_status status;
   EdmaMgr_Handle channel;
} copy_event;

int  initialize_edmamgr();
void __copy_wait(copy_event *event);
copy_event *__copy_1D1D(copy_event *event, void *dst, void *src, uint32_t bytes);
copy_event *__copy_2D1D(copy_event *event, void *dst, void *src, uint32_t bytes, 
                        uint32_t num_lines, int32_t pitch);
copy_event *__copy_1D2D(copy_event *event, void *dst, void *src, uint32_t bytes, 
                        uint32_t num_lines, int32_t pitch);

void free_edma_channel_pool();
#endif // _EDMA_H_
