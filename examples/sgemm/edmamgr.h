#ifndef _EdmaMgr_h
#define _EdmaMgr_h
#include <stdint.h>

typedef void *EdmaMgr_Handle;

EdmaMgr_Handle  EdmaMgr_alloc            (int32_t max_linked_transfers);

int32_t         EdmaMgr_free             (EdmaMgr_Handle h);

void            EdmaMgr_wait             (EdmaMgr_Handle h); 

int32_t         EdmaMgr_copy1D1D         (EdmaMgr_Handle  h, 
                                          void *restrict  src, 
                                          void *restrict  dst, 
                                          int32_t         num_bytes);

int32_t         EdmaMgr_copy1D2D         (EdmaMgr_Handle  h, 
                                          void *restrict  src, 
                                          void *restrict  dst, 
                                          int32_t         num_bytes, 
                                          int32_t         num_lines, 
                                          int32_t         pitch);

int32_t         EdmaMgr_copy2D1D         (EdmaMgr_Handle  h,
                                          void *restrict  src,
                                          void *restrict  dst,
                                          int32_t         num_bytes,
                                          int32_t         num_lines,
                                          int32_t         pitch);

int32_t         EdmaMgr_copy2D2D         (EdmaMgr_Handle  h,
                                          void *restrict  src,
                                          void *restrict  dst,
                                          int32_t         num_bytes,
                                          int32_t         num_lines,
                                          int32_t         pitch);

int32_t         EdmaMgr_copy2D2DSep      (EdmaMgr_Handle  h,
                                          void *restrict  src,
                                          void *restrict  dst,
                                          int32_t         num_bytes,
                                          int32_t         num_lines,
                                          int32_t         src_pitch,
                                          int32_t         dst_pitch);

int32_t         EdmaMgr_copy1D1DLinked   (EdmaMgr_Handle  h,
                                          void *restrict  src[],
                                          void *restrict  dst[],
                                          int32_t         num_bytes[],
                                          int32_t         num_transfers);

int32_t         EdmaMgr_copy1D2DLinked   (EdmaMgr_Handle  h,
                                          void *restrict  src[],
                                          void *restrict  dst[],
                                          int32_t         num_bytes[],
                                          int32_t         num_lines[],
                                          int32_t         pitch[],
                                          int32_t         num_transfers);

int32_t         EdmaMgr_copy2D1DLinked   (EdmaMgr_Handle  h,
                                          void *restrict  src[],
                                          void *restrict  dst[],
                                          int32_t         num_bytes[],
                                          int32_t         num_lines[],
                                          int32_t         pitch[],
                                          int32_t         num_transfers);

int32_t         EdmaMgr_copy2D2DLinked   (EdmaMgr_Handle  h,
                                          void *restrict  src[],
                                          void *restrict  dst[],
                                          int32_t         num_bytes[],
                                          int32_t         num_lines[],
                                          int32_t         pitch[],
                                          int32_t         num_transfers);

int32_t         EdmaMgr_copy2D2DSepLinked(EdmaMgr_Handle  h,
                                          void *restrict  src[],
                                          void *restrict  dst[],
                                          int32_t         num_bytes[],
                                          int32_t         num_lines[],
                                          int32_t         src_pitch[],
                                          int32_t         dst_pitch[],
                                          int32_t         num_transfers);

int32_t         EdmaMgr_copyFast         (EdmaMgr_Handle  h,
                                          void *restrict  src,
                                          void *restrict  dst);

int32_t         EdmaMgr_copyLinkedFast   (EdmaMgr_Handle  h,
                                          void *restrict  src[],
                                          void *restrict  dst[],
                                          int32_t         num_transfers);

#define EdmaMgr_SUCCESS           0
#define EdmaMgr_ERROR_INVARG     -1
#define EdmaMgr_ERROR_INVCFG     -2
#define EdmaMgr_ERROR_RMANINIT   -3
#define EdmaMgr_ERROR_INVHANDLE  -4
#define EdmaMgr_ERROR_FREE       -5

#endif 
