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

void ocl_tidl_setup(      TIDL_CreateParams* createParams,
                          ConfigParams*      configParams,
                    const uint8_t*           networkParamBuffer,
                          uint8_t*           networkParamHeap,
                          SetupParams*       setupParams);

void ocl_tidl_initialize(const TIDL_CreateParams* createParams,
                               ConfigParams*      configParams,
                         const uint8_t*           networkParamBuffer,
                               uint8_t*           externalMemoryHeapBase,
                         const InitializeParams*  initP,
                               uint8_t*           l2HeapBase);

void ocl_tidl_process(      ConfigParams*  configParams,
                            ProcessParams* processParams,
                      const uint8_t*       inputFrame,
                            uint8_t*       outputData);

void ocl_tidl_cleanup();

#endif
