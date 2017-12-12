#ifndef _TIDL_IMPL_H_
#define _TIDL_IMPL_H_

#ifndef __OPENCL_VERSION__
#define global
#define local
#define _STRUCT
#else
#define _STRUCT struct
#endif

struct TIDL_CreateParams;

void ocl_tidl_setup(TIDL_CreateParams *createParams,
                      ConfigParams      *configParams,
                      const char        *networkParamBuffer,
                      unsigned char     *networkParamHeap,
                      SetupParams       *setupParams);

void ocl_tidl_initialize(const TIDL_CreateParams *createParams,
                                 ConfigParams      *configParams,
                                 const char        *NetworkParamBuffer,
                                 uint8_t           *ExternalMemoryHeapBase,
                                 const InitializeParams *initP,
                                 uint8_t *l2HeapBase);

void ocl_tidl_process(//TIDL_CreateParams *createParams, 
                        ConfigParams      *configParams,
                        ProcessParams     *processParams,
                        const char        *inputFrame,
                              char        *outputData);

void ocl_tidl_cleanup();

#endif
