#include <xdc/std.h>
#include <string.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/family/c66/Cache.h>

//#define FC_TRACE
#ifdef FC_TRACE
#include <ti/sdo/fc/global/FCSettings.h>
#include <xdc/runtime/Diags.h>
#endif

#include "monitor.h"
#include "util.h"
#include <stdint.h>
#include "edma.h"

#define ADDR_IS_EDMA3_COHERENT(addr) ((addr) < (void*)0x80000000)

/*-----------------------------------------------------------------------------
* On AM57x, each DSP core has an identical copy of configuration parameters
* table, private to each core
*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* If the edmamgr resource map changes, we may need to revisit these macros
*----------------------------------------------------------------------------*/
#define EDMA_MGR_MAX_NUM_CHANNELS    (32)
#define STARTUP_NUM_OF_EDMA_CHANNELS (4)

#define MEMCPY_THRESHOLD             (0x800)

// Edma stride/pitch field is signed 16-bits
#define EDMA_PITCH_LIMIT            (32767)

DDR_2D  (copy_event, edma_channel_pool, NUM_CORES, EDMA_MGR_MAX_NUM_CHANNELS);
PRIVATE (copy_event *, available_edma_channel) = NULL;
DDR     (copy_event,   memcpy_event) = { EV_MEMCPY, NULL };

/******************************************************************************
* Set up table to manage edma channel usage/allocation
******************************************************************************/
static int initialize_edma_channel_pool()
{
   int i;
   uint32_t dspid = get_dsp_id();

   // Initialized the edma table to be not allocated
   for(i = 0; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
   {
      edma_channel_pool[dspid][i].status = EV_NOT_ALLOCATED;
      edma_channel_pool[dspid][i].channel = NULL;
   }

   // Pre-allocate a fixed number of edma channels
   for(i = 0; i < STARTUP_NUM_OF_EDMA_CHANNELS; i++)
   {
      if (!(edma_channel_pool[dspid][i].channel = EdmaMgr_alloc(1))) return -1;
      edma_channel_pool[dspid][i].status = EV_AVAILABLE;
   }

   // Set up next available pointer
   available_edma_channel = &edma_channel_pool[dspid][0];

   return 0;
}


/******************************************************************************
* initialize_edmamgr and allocate an initial pool of edma channels
******************************************************************************/
int initialize_edmamgr()
{
   int status = EdmaMgr_SUCCESS;

#ifdef FC_TRACE
   FCSettings_init();
   Diags_setMask(FCSETTINGS_MODNAME"+EX1234567");
#endif

   // On AM57x, Each DSP has an identical copy of EDMA
   // Use proc_id 0 to pick Instance 1, region 2 (see edma_config.c)
   if((status = EdmaMgr_init(0, NULL)) != EdmaMgr_SUCCESS)
      return !status;

   return !initialize_edma_channel_pool();
}


/******************************************************************************
* Find or allocate an available edma channel
******************************************************************************/
static copy_event *get_edma_channel()
{
   int i;
   copy_event *result = NULL;

   // If we have an edma channel readily available, use it.
   // Don't bother setting up next available. Assuming typical use case
   // is that a wait_group_events() has been called and we set up an available
   // channel then.
   if (available_edma_channel != NULL)
   {
      available_edma_channel->status = EV_IN_USE;
      result = available_edma_channel;
      available_edma_channel = NULL;

      return result;
   }
   // Otherwise search table for available channel, allocate a new one if needed
   // TBD: Not currently attempting to free unused channels
   uint32_t dspid = get_dsp_id();
   for(i = 0; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
   {
      if (edma_channel_pool[dspid][i].status == EV_IN_USE) continue;
      else
      {
         /*--------------------------------------------------------------------
         * if we eventually support freeing channels, then an available may
         * exist beyond this not allocated and we could recover from an
         * edmamgr_alloc failure.
         *-------------------------------------------------------------------*/
         if (edma_channel_pool[dspid][i].status == EV_NOT_ALLOCATED)
            if ((edma_channel_pool[dspid][i].channel = EdmaMgr_alloc(1)) == NULL)
               return NULL;

         edma_channel_pool[dspid][i].status = EV_IN_USE;
         return &edma_channel_pool[dspid][i];
      }
   }

   // Didn't find a channel
   return NULL;
}


/******************************************************************************
* Implement a copy. Using edma if the size warrants, otherwise use memcpy()
******************************************************************************/
EXPORT copy_event * __copy_1D1D(copy_event *event, void *dst, void *src, 
                                uint32_t bytes)
{
   // Wait on any pending edma transfer or get an edma channel, if needed
   if      (event && event->status != EV_MEMCPY) EdmaMgr_wait(event->channel);
   else if (bytes > MEMCPY_THRESHOLD)            event = get_edma_channel();

   // Perform edma copy if size warrants and there is an available edma channel
   if (bytes > MEMCPY_THRESHOLD && event)
   {
      /*-----------------------------------------------------------------------
      * We do not currently need to wb a potentially no coherent source, since
      * we are operating in write through mode.  The code is here as a reminder
      * for the day when we dynamically support writeback mode.
      *----------------------------------------------------------------------*/
      //if (!ADDR_IS_EDMA3_COHERENT(src))
         //Cache_wb(src, bytes, Cache_Type_ALL, TRUE);

      /*-----------------------------------------------------------------------
      * The dst needs to be invalidate before the core can read it again.
      * This could be performed after the copy is complete, but we issue it
      * here because we have the size readily available.
      *----------------------------------------------------------------------*/
      if (!ADDR_IS_EDMA3_COHERENT(dst))
         Cache_inv(dst, bytes, Cache_Type_ALL, TRUE);

      EdmaMgr_copy1D1D(event->channel, src, dst, bytes);
   }
   // Requested copy size isn't efficient using edma or we didn't get a channel
   else
   {
       memcpy(dst, src, bytes);
       if (!event) event = &memcpy_event;
   }

   return event;
}


/******************************************************************************
* Implement a strided copy from 1D local memory to 2D global memory
******************************************************************************/
EXPORT copy_event *__copy_1D2D(copy_event *event, void *dst, void *src,             
			       uint32_t  bytes, uint32_t num_lines,     
			       int32_t pitch)
{
   // Wait on any pending edma transfer or get an edma channel, if needed
   if      (event && event->status != EV_MEMCPY) EdmaMgr_wait(event->channel);
   else if (pitch < EDMA_PITCH_LIMIT)            event = get_edma_channel();

   // Perform edma copy, if possible
   if (pitch < EDMA_PITCH_LIMIT && event)
   {
      /*-----------------------------------------------------------------------
      * Dst is known to be global (aka DDR) and is therefore known to not be
      * coherent.  The dst needs to be invalidate before the core can read it
      * again. This could be performed after the copy is complete, but we issue
      * it here because we have the size readily available.
      *
      * We do not need to wb the source because it is onchip and coherent.
      *----------------------------------------------------------------------*/
      Cache_inv(dst, bytes * (1+(num_lines-1)*pitch), Cache_Type_ALL, TRUE);
      EdmaMgr_copy1D2D(event->channel, src, dst, bytes, num_lines,
                       bytes * pitch);
   }
   // pitch(stride) is too large or we didn't get a channel.
   else
   {
       int i;
       for(i = 0; i < num_lines; i++)
          memcpy((char *)dst+( i*bytes*pitch), (char *)src+(i*bytes), bytes);
      if (!event) event = &memcpy_event;
   }

   return event;
}


/******************************************************************************
* Implement a strided copy from 2D global memory to 1D local memory
******************************************************************************/
EXPORT copy_event *__copy_2D1D(copy_event *event, void *dst, void *src, 
			       uint32_t  bytes, uint32_t num_lines,     
			       int32_t pitch)
{
   // Wait on any pending edma transfer or get an edma channel, if needed
   if      (event && event->status != EV_MEMCPY) EdmaMgr_wait(event->channel);
   else if (pitch < EDMA_PITCH_LIMIT)            event = get_edma_channel();

   // Perform edma strided copy, if possible
   if (pitch < EDMA_PITCH_LIMIT && event)
   {
      /*-----------------------------------------------------------------------
      * We do not currently need to wb the non-coherent source, since
      * we are operating in write through mode.  The code is here as a reminder
      * for the day when we dynamically support writeback mode.
      *
      * We do not need to inv the dst because it is onchip and coherent.
      *----------------------------------------------------------------------*/
      //Cache_wb(...);

      EdmaMgr_copy2D1D(event->channel, src, dst, bytes, num_lines,
                       bytes*pitch);
   }
   // pitch(stride) is too large or we didn't get a channel
   else
   {
       int i;
       for(i = 0; i < num_lines; i++)
          memcpy((char *)dst+(i*bytes), (char *)src+(i*bytes*pitch),
                 bytes);

       if (!event) event = &memcpy_event;
    }

   return event;
}

/******************************************************************************
* Wait on any pending edma transfer associated with this event.
* If the event is associated with a memcpy() nothing to wait on
******************************************************************************/
EXPORT void __copy_wait(copy_event *event)
{
   if(!event || event->status == EV_MEMCPY) return;

   EdmaMgr_wait(event->channel);
   event->status = EV_AVAILABLE;
   available_edma_channel = event;
}


/******************************************************************************
* De-allocate all used edma channels
******************************************************************************/
void free_edma_channel_pool()
{
   int i;
   uint32_t dspid = get_dsp_id();

   for(i = 0; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
      if (edma_channel_pool[dspid][i].status != EV_NOT_ALLOCATED)
         EdmaMgr_free(edma_channel_pool[dspid][i].channel);
}
