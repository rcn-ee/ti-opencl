#ifndef _COMMON_DEFINES_H_
#define _COMMON_DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t randInput;
    uint32_t noZeroCoeffsPercentage;
    uint32_t updateNetWithStats;    
    uint32_t rawImage;
    uint32_t writeInput;
    uint32_t writeOutput;
    uint32_t compareRef;
    uint32_t numFrames;
    uint32_t preProcType;
    uint8_t  performanceTestcase ;
} ConfigParams;


typedef struct
{
    uint32_t tidlHeapSize;
    uint32_t runtimeHeapSize;
    uint32_t l2HeapSize;
    uint32_t l1HeapSize;
} InitializeParams;

typedef struct
{
    uint32_t frameIdx;
    uint32_t bytesWritten;
} ProcessParams;

typedef struct
{
    uint32_t networkParamHeapSize;
} SetupParams;

#define TIDL_TB_CURR_CORE_ID (1)
#define TIDL_TB_CURR_LAYERS_GROUP_ID (1)


#ifdef __cplusplus
}
#endif

#endif // _COMMON_DEFINES_H_
