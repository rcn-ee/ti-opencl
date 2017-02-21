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
void reset_intra_kernel_edma_channels();
void wait_and_free_edma_channels();
void free_edma_hw_channels();
void restore_edma_hw_channels();

/******************************************************************************
* OpenCL runtime version of EdmaMgr functions with SOFTWARE INTERRUPTS DISABLED
*     Added for TIMEOUT (SWi) feature: kernels can be preempted and killed
*     Disable SWi to avoid incomplete state
******************************************************************************/
// Allocated channels intended for single-kernel use will be freed
// if kernel gets killed by timeout.
// Otherwise, they should be freed when kernel finishes or before abort/exit.
EXPORT EdmaMgr_Handle __ocl_EdmaMgr_alloc_intrakernel(
                                      int32_t max_linked_transfers);

// Pre-allocated channels intended for cross-kernel use will NOT be freed
// if kernel gets killed by timeout.
// They should be freed when no longer needed.
EXPORT EdmaMgr_Handle __ocl_EdmaMgr_alloc_crosskernel(
                                      int32_t max_linked_transfers);

EXPORT int32_t __ocl_EdmaMgr_free(EdmaMgr_Handle h);
EXPORT void    __ocl_EdmaMgr_wait(EdmaMgr_Handle h);
EXPORT int32_t __ocl_EdmaMgr_copy1D1D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes);
EXPORT int32_t __ocl_EdmaMgr_copy1D2D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t pitch);
EXPORT int32_t __ocl_EdmaMgr_copy2D1D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t pitch);
EXPORT int32_t __ocl_EdmaMgr_copy2D2D(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t pitch);
EXPORT int32_t __ocl_EdmaMgr_copy2D2DSep(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst, int32_t num_bytes,
                                      int32_t num_lines, int32_t src_pitch,
                                      int32_t dst_pitch);
EXPORT int32_t __ocl_EdmaMgr_copy1D1DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[],
                                      int32_t num_transfers);
EXPORT int32_t __ocl_EdmaMgr_copy1D2DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t pitch[], int32_t num_transfers);
EXPORT int32_t __ocl_EdmaMgr_copy2D1DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t pitch[], int32_t num_transfers);
EXPORT int32_t __ocl_EdmaMgr_copy2D2DLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t pitch[], int32_t num_transfers);
EXPORT int32_t __ocl_EdmaMgr_copy2D2DSepLinked(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_bytes[], int32_t num_lines[],
                                      int32_t src_pitch[], int32_t dst_pitch[],
                                      int32_t num_transfers);
EXPORT int32_t __ocl_EdmaMgr_copy1D2DLarge(EdmaMgr_Handle h,
                                      void *restrict src, void *restrict dst,
                                      int32_t num_bytes, int32_t num_lines,
                                      int32_t pitch);
EXPORT int32_t __ocl_EdmaMgr_copy2D1DLarge(EdmaMgr_Handle h,
                                      void *restrict src, void *restrict dst,
                                      int32_t num_bytes, int32_t num_lines,
                                      int32_t pitch);
EXPORT int32_t __ocl_EdmaMgr_copyFast(EdmaMgr_Handle h, void *restrict src,
                                      void *restrict dst);
EXPORT int32_t __ocl_EdmaMgr_copyLinkedFast(EdmaMgr_Handle h,
                                      void *restrict src[],
                                      void *restrict dst[],
                                      int32_t num_transfers);

#endif // _EDMA_H_
