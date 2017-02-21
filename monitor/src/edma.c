#include <xdc/std.h>
#include <string.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/family/c66/Cache.h>
#include <ti/sysbios/knl/Swi.h>

//#define FC_TRACE
#ifdef FC_TRACE
#include <ti/sdo/fc/global/FCSettings.h>
#include <xdc/runtime/Diags.h>
#endif

#include "monitor.h"
#include "util.h"
#include <stdint.h>
#include "edma.h"

#define ADDR_IS_EDMA3_COHERENT(addr) (((unsigned int) (addr) >> 20) == 0x008)

/*-----------------------------------------------------------------------------
* On AM57x, each DSP core has an identical copy of configuration parameters
* table, private to each core
*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* If the edmamgr resource map changes, we may need to revisit these macros
*----------------------------------------------------------------------------*/
#if defined(DEVICE_AM572x)
    #define EDMA_MGR_MAX_NUM_CHANNELS    (32)
#elif defined(DEVICE_K2G)
    #define EDMA_MGR_MAX_NUM_CHANNELS    (16)
#elif defined(DEVICE_K2H) || defined (DEVICE_K2L) || defined (DEVICE_K2E)
    #define EDMA_MGR_MAX_NUM_CHANNELS    (15)
#else
    #error Unknown device
#endif

#define STARTUP_NUM_OF_EDMA_CHANNELS (4)

#define MEMCPY_THRESHOLD             (0x800) 

// Edma stride/pitch field is signed 16-bits
#define EDMA_PITCH_LIMIT            (32767)

extern EdmaMgr_Channel EdmaMgr_channels[EDMA_MGR_MAX_NUM_CHANNELS];

PRIVATE_NOALIGN (copy_event *, available_edma_channel) = NULL;
PRIVATE_NOALIGN (int, num_intra_kernel_edma_channels) = 0;
DDR    (copy_event, memcpy_event) = { EV_MEMCPY, NULL };
DDR_2D (copy_event, edma_channel_pool, MAX_NUM_CORES,
                                       EDMA_MGR_MAX_NUM_CHANNELS);
DDR_2D (EdmaMgr_Handle, intra_kernel_edma_channels, MAX_NUM_CORES,
                                       EDMA_MGR_MAX_NUM_CHANNELS);

/******************************************************************************
* Set up table to manage edma channel usage/allocation
******************************************************************************/
static int initialize_edma_channel_pool()
{
   int i;

   // Pre-allocate a fixed number of edma channels
   for(i = 0; i < STARTUP_NUM_OF_EDMA_CHANNELS; i++)
   {
      if (!(edma_channel_pool[DNUM][i].channel =
                                __ocl_EdmaMgr_alloc_crosskernel(1))) return -1;
      edma_channel_pool[DNUM][i].status = EV_AVAILABLE;
   }

   // Set up the rest of the edma table in case more channels are needed
   for(i = STARTUP_NUM_OF_EDMA_CHANNELS; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
   {
      edma_channel_pool[DNUM][i].status = EV_NOT_ALLOCATED;
      edma_channel_pool[DNUM][i].channel = NULL;
   }

   // Set up next available pointer
   available_edma_channel = &edma_channel_pool[DNUM][0];

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

#ifdef DEVICE_AM572x
   // On AM57x, Each DSP has an identical copy of EDMA
   // Use proc_id 0 to pick Instance 1, region 2 (see edma_config.c)
   if((status = EdmaMgr_init(0, NULL)) != EdmaMgr_SUCCESS)
#else
   if((status = EdmaMgr_init(DNUM, NULL)) != EdmaMgr_SUCCESS)
#endif
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
   for(i = 0; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
   {
      if (edma_channel_pool[DNUM][i].status == EV_IN_USE) continue;
      else
      {
         /*--------------------------------------------------------------------
         * if we eventually support freeing channels, then an available may
         * exist beyond this not allocated and we could recover from an
         * edmamgr_alloc failure.
         *-------------------------------------------------------------------*/
         if (edma_channel_pool[DNUM][i].status == EV_NOT_ALLOCATED)
            if ((edma_channel_pool[DNUM][i].channel =
                                   __ocl_EdmaMgr_alloc_crosskernel(1)) == NULL)
               return NULL;

         edma_channel_pool[DNUM][i].status = EV_IN_USE;
         return &edma_channel_pool[DNUM][i];
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
   if (event && event->status != EV_MEMCPY)
      __ocl_EdmaMgr_wait(event->channel);
   else if (bytes > MEMCPY_THRESHOLD)
      event = get_edma_channel();

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

      __ocl_EdmaMgr_copy1D1D(event->channel, src, dst, bytes);

      /*-----------------------------------------------------------------------
      * The dst needs to be invalidate before the core can read it again.
      * This could be performed after the copy is complete, but we issue it
      * here because we have the size readily available.
      *----------------------------------------------------------------------*/
      if (!ADDR_IS_EDMA3_COHERENT(dst))
         Cache_inv(dst, bytes, Cache_Type_ALL, TRUE);

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
   if (event && event->status != EV_MEMCPY)
      __ocl_EdmaMgr_wait(event->channel);
   else if (pitch < EDMA_PITCH_LIMIT)
      event = get_edma_channel();

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
      __ocl_EdmaMgr_copy1D2D(event->channel, src, dst, bytes, num_lines,
                       bytes * pitch);
      Cache_inv(dst, bytes * (1+(num_lines-1)*pitch), Cache_Type_ALL, TRUE);
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
   if (event && event->status != EV_MEMCPY)
      __ocl_EdmaMgr_wait(event->channel);
   else if (pitch < EDMA_PITCH_LIMIT)
      event = get_edma_channel();

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

      __ocl_EdmaMgr_copy2D1D(event->channel, src, dst, bytes, num_lines,
                             bytes*pitch);
   }
   // pitch(stride) is too large or we didn't get a channel
   else
   {
      int i;
      for(i = 0; i < num_lines; i++)
         memcpy((char *)dst+(i*bytes), (char *)src+(i*bytes*pitch), bytes);

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

   __ocl_EdmaMgr_wait(event->channel);
   event->status = EV_AVAILABLE;
   available_edma_channel = event;
}


/******************************************************************************
* De-allocate all used edma channels
* AM57x: Allocated EdmaMgr channels currently can NOT survive IpcPower
* suspend/resume yet, we have to free them before suspend.
******************************************************************************/
void free_edma_channel_pool()
{
   int i;

   available_edma_channel = NULL;
   for(i = 0; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
      if (edma_channel_pool[DNUM][i].status != EV_NOT_ALLOCATED)
      {
         __ocl_EdmaMgr_free(edma_channel_pool[DNUM][i].channel);
         edma_channel_pool[DNUM][i].status = EV_NOT_ALLOCATED;
      }
}

/******************************************************************************
* Reset bookkeeping for channels NOT intended for cross-kernel use
******************************************************************************/
void reset_intra_kernel_edma_channels()
{
   num_intra_kernel_edma_channels = 0;
}

/******************************************************************************
* Wait for all on-the-fly edma to complete, free channels NOT intended
*     for cross-kernel use so that they are not "leaked"
******************************************************************************/
void wait_and_free_edma_channels()
{
   int i;

   // Wait for on-the-fly edma in OpenCL allocated channels to finish
   for(i = 0; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
      if (edma_channel_pool[DNUM][i].status == EV_IN_USE)
         __copy_wait(&edma_channel_pool[DNUM][i]);

   // Wait for all remaining on-the-fly edma transfers to complete
   for (i = 0; i < EDMA_MGR_MAX_NUM_CHANNELS; i++)
   {
       EdmaMgr_Channel *chan = &EdmaMgr_channels[i];
       if (chan->edmaArgs.numPaRams > 0 && chan->xferPending)
           __ocl_EdmaMgr_wait((EdmaMgr_Handle) chan);
   }

   // Free channels NOT intended for cross-kernel use, if not already freed
   for (i = 0; i < num_intra_kernel_edma_channels; i++)
       if (((EdmaMgr_Channel *)
                  intra_kernel_edma_channels[DNUM][i])->edmaArgs.numPaRams > 0)
           __ocl_EdmaMgr_free(intra_kernel_edma_channels[DNUM][i]);
   num_intra_kernel_edma_channels = 0;
}

/******************************************************************************
* De-allocate edma hardware channels before IpcPower suspend
******************************************************************************/
void free_edma_hw_channels()
{
  EdmaMgr_hwFreeAll();
  cacheWbInvAllL2();
}

/******************************************************************************
* Re-allocate edma hardware channels after IpcPower resume
******************************************************************************/
void restore_edma_hw_channels()
{
  cacheInvAllL2();
  EdmaMgr_hwAllocAll();
}


/******************************************************************************
* OpenCL runtime version of EdmaMgr functions with SOFTWARE INTERRUPTS DISABLED
*     Added for TIMEOUT (SWi) feature: kernels can be preempted and killed
*     Disable SWi to avoid incomplete EDMA state
******************************************************************************/
// Allocated channels intended for single-kernel use will be freed
// if kernel gets killed by timeout.
// Otherwise, they should be freed when kernel finishes or before abort/exit.
EXPORT EdmaMgr_Handle __ocl_EdmaMgr_alloc_intrakernel(
                                          int32_t max_linked_transfers)
{
  uint32_t lvInt = Swi_disable();
  EdmaMgr_Handle h =  EdmaMgr_alloc(max_linked_transfers);
  if (h != NULL && num_intra_kernel_edma_channels < EDMA_MGR_MAX_NUM_CHANNELS-1)
    intra_kernel_edma_channels[DNUM][num_intra_kernel_edma_channels++] = h;
  Swi_restore(lvInt);
  return h;
}

// Pre-allocated channels intended for cross-kernel use will NOT be freed
// if kernel gets killed by timeout.
// They should be freed when no longer needed.
EXPORT EdmaMgr_Handle __ocl_EdmaMgr_alloc_crosskernel(
                                          int32_t max_linked_transfers)
{
  uint32_t lvInt = Swi_disable();
  EdmaMgr_Handle h =  EdmaMgr_alloc(max_linked_transfers);
  Swi_restore(lvInt);
  return h;
}

EXPORT int32_t __ocl_EdmaMgr_free(EdmaMgr_Handle h)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_free(h);
  Swi_restore(lvInt);
  return ret;
}

EXPORT void    __ocl_EdmaMgr_wait(EdmaMgr_Handle h)
{
  uint32_t lvInt = Swi_disable();
  EdmaMgr_wait(h);
  Swi_restore(lvInt);
}

EXPORT int32_t __ocl_EdmaMgr_copy1D1D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy1D1D(h, src, dst, num_bytes);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy1D2D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t pitch)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy1D2D(h, src, dst, num_bytes, num_lines, pitch);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy2D1D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t pitch)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy2D1D(h, src, dst, num_bytes, num_lines, pitch);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy2D2D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t pitch)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy2D2D(h, src, dst, num_bytes, num_lines, pitch);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy2D2DSep(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t src_pitch,
                                      int32_t dst_pitch)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy2D2DSep(h, src, dst, num_bytes, num_lines,
                                    src_pitch, dst_pitch);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy1D1DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[],
                                      int32_t num_transfers)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy1D1DLinked(h, src, dst, num_bytes, num_transfers);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy1D2DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t pitch[], int32_t num_transfers)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy1D2DLinked(h, src, dst, num_bytes, num_lines,
                                       pitch, num_transfers);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy2D1DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t pitch[], int32_t num_transfers)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy2D1DLinked(h, src, dst, num_bytes, num_lines,
                                       pitch, num_transfers);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy2D2DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t pitch[], int32_t num_transfers)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy2D2DLinked(h, src, dst, num_bytes, num_lines,
                                       pitch, num_transfers);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy2D2DSepLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t src_pitch[], int32_t dst_pitch[],
                                      int32_t num_transfers)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy2D2DSepLinked(h, src, dst, num_bytes, num_lines,
                                          src_pitch, dst_pitch, num_transfers);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy1D2DLarge(EdmaMgr_Handle h,
                                      void *restrict src, void *restrict dst,
                                      int32_t num_bytes, int32_t num_lines,
                                      int32_t pitch)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy1D2DLarge(h, src, dst, num_bytes, num_lines, pitch);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copy2D1DLarge(EdmaMgr_Handle h,
                                      void *restrict src, void *restrict dst,
                                      int32_t num_bytes, int32_t num_lines,
                                      int32_t pitch)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copy2D1DLarge(h, src, dst, num_bytes, num_lines, pitch);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copyFast(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copyFast(h, src, dst);
  Swi_restore(lvInt);
  return ret;
}

EXPORT int32_t __ocl_EdmaMgr_copyLinkedFast(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_transfers)
{
  uint32_t lvInt = Swi_disable();
  int32_t ret = EdmaMgr_copyLinkedFast(h, src, dst, num_transfers);
  Swi_restore(lvInt);
  return ret;
}
