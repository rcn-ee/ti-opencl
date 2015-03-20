#ifndef _EDMA_H_
#define _EDMA_H_

typedef int copy_event;

int  initialize_edmamgr();
void __copy_wait(copy_event *event);
copy_event *__copy_1D1D(copy_event *event, void *dst, void *src, uint32_t bytes);
copy_event *__copy_2D1D(copy_event *event, void *dst, void *src, uint32_t bytes, 
                        uint32_t num_lines, int32_t pitch);
copy_event *__copy_1D2D(copy_event *event, void *dst, void *src, uint32_t bytes, 
                        uint32_t num_lines, int32_t pitch);

void free_edma_channel_pool();
#endif // _EDMA_H_
