#include "monitor.h"
#include <stdint.h>
#include "edma.h"

/******************************************************************************
* initialize_edmamgr and allocate an initial pool of edma channels
******************************************************************************/
int initialize_edmamgr()
{
    return 1;
}

/******************************************************************************
* Implement a copy. Using edma if the size warrants, otherwise use memcpy()
******************************************************************************/
EXPORT copy_event * __copy_1D1D(copy_event *event, void *dst, void *src, 
                                uint32_t bytes)
{
   memcpy(dst, src, bytes); 
   return event;
}


/******************************************************************************
* Implement a strided copy from 1D local memory to 2D global memory
******************************************************************************/
EXPORT copy_event *__copy_1D2D(copy_event *event, void *dst, void *src,             
			       uint32_t  bytes, uint32_t num_lines,     
			       int32_t pitch)
{
   int i;
   for(i = 0; i < num_lines; i++)
      memcpy((char *)dst+( i*bytes*pitch), (char *)src+(i*bytes), bytes); 
   return event;
}


/******************************************************************************
* Implement a strided copy from 2D global memory to 1D local memory
******************************************************************************/
EXPORT copy_event *__copy_2D1D(copy_event *event, void *dst, void *src, 
			       uint32_t  bytes, uint32_t num_lines,     
			       int32_t pitch)
{
  int i;
  for(i = 0; i < num_lines; i++)
     memcpy((char *)dst+(i*bytes), (char *)src+(i*bytes*pitch), 
            bytes); 

   return event;
}

/******************************************************************************
* Wait on any pending edma transfer associated with this event.
* If the event is associated with a memcpy() nothing to wait on
******************************************************************************/
EXPORT void __copy_wait(copy_event *event)
{
}


/******************************************************************************
* De-allocate all used edma channels
******************************************************************************/
void free_edma_channel_pool()
{
}
