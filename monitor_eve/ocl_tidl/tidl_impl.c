/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "xdais_types.h"
#include "itidl_ti.h"
#include "tidl_alg_int.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../src/eve_memory.h"
#include "ti_mem_manager.h"
#include "common_defines.h"
#include "tidl_impl.h"
 
#define TIDL_BLOCK_WIDTH            (32U)
#define TIDL_BLOCK_HEIGHT           (32U)
#undef ALIGN_SIZE
#define ALIGN_SIZE(x,y) (((x + (y-1)) / y) * y)
#define MEM_CONTAMINATION_DIS


// IVISION API. Declared in tidl_alg_int.h. Brings in too many dependencies,
// so using externs here instead of including the header
extern XDAS_Int32 TIDL_numAlloc(void);
extern XDAS_Int32 TIDL_alloc(const IALG_Params *params,
                               IALG_Fxns **parentFxns, IALG_MemRec *memRec);
extern XDAS_Int32 TIDL_init(IALG_Handle handle,
                              const IALG_MemRec *memRec, IALG_Handle parent,
                                                    const IALG_Params *params);
extern XDAS_Void  TIDL_activate(IALG_Handle handle);
extern XDAS_Void  TIDL_deactivate(IALG_Handle handle);
extern XDAS_Int32 TIDL_free(IALG_Handle handle, IALG_MemRec *memRec);
extern XDAS_Int32 TIDL_process(IVISION_Handle Handle, IVISION_InBufs *inBufs,
                               IVISION_OutBufs *outBufs,IVISION_InArgs *inArgs,
                               IVISION_OutArgs *outArgs);

typedef struct IM_Fxns
{
  IVISION_Fxns * ivision;
} IM_Fxns;

typedef struct
{
    IALG_MemRec *memRec;
    int          numMemRec;
    TIDL_Handle  handle;
    IVISION_BufDesc   inBufDesc[TIDL_MAX_ALG_IN_BUFS];
    IVISION_BufDesc   outBufDesc[TIDL_MAX_ALG_OUT_BUFS];
    IVISION_BufDesc   *inBufDescList[TIDL_MAX_ALG_IN_BUFS];
    IVISION_BufDesc   *outBufDescList[TIDL_MAX_ALG_OUT_BUFS];

    IVISION_InBufs    inBufs;
    IVISION_OutBufs   outBufs;

    TIMemObject memObj_DMEM0;
    TIMemObject memObj_DMEM1;
    TIMemObject memObj_EXTMEM;
    TIMemObject memObj_NetworkParam;

    char pad[60];

} TIDL_State;



static int32_t AllocMemRecords(IALG_MemRec * memRec,int32_t numMemRec,
                               TIDL_State *state);
static int32_t FreeMemRecords(IALG_MemRec * memRec,int32_t numMemRec,
                               TIDL_State *state);

static int32_t tidl_AllocNetInputMem(sTIDL_Network_t *net,
                              IVISION_BufDesc *BufDescList,
                              TIDL_State *state);
static int32_t tidl_AllocNetOutputMem(sTIDL_Network_t *net,
                               IVISION_BufDesc *BufDescList,
                               TIDL_State *state);

static int32_t tidl_allocNetParamsMem(sTIDL_Network_t *net, TIDL_State *state);

static int32_t tidl_fillNetParamsMem(sTIDL_Network_t *net, 
                                     const ConfigParams *params,
                                     const char *NetworkParamBuffer);

static int32_t tidl_ReadNetInput(sTIDL_Network_t *net, 
                                 const ConfigParams *params,
                          IVISION_BufDesc *BufDescList, int32_t frameCount,
                          const char *inputFrame);

static size_t tidl_WriteNetOutputMem (const TIDL_CreateParams *params,
                                IVISION_BufDesc *BufDescList, 
                                uint16_t numBuffs,
                                char    *outputData);

#ifdef PRINT_NET_INFO
static int32_t tidltb_printNetInfo(sTIDL_Network_t * pTIDLNetStructure, 
                            int32_t layersGroupId);
#endif

void print_buffer(const unsigned char *p, int n);

TIDL_State per_core_state[1] __attribute__((aligned(128)));
#define DNUM 0


void ocl_tidl_setup(TIDL_CreateParams *createParams,
                   ConfigParams      *configParams,
             const char              *networkParamBuffer,
          unsigned char              *networkParamHeap,
                   SetupParams       *setupParams)
{
    TIDL_State *state = &per_core_state[DNUM];

    TI_CreateMemoryHandle(&state->memObj_NetworkParam, 
                          networkParamHeap,
                          setupParams->networkParamHeapSize);

    // Allocate space for network parameters
    tidl_allocNetParamsMem(&createParams->net, state);

    // Initialize network parameters
    tidl_fillNetParamsMem(&createParams->net, configParams,
                          networkParamBuffer);

    //tidltb_printNetInfo(&createParams->net, createParams->currLayersGroupId);
}


void ocl_tidl_initialize(const TIDL_CreateParams *createParams, 
                  ConfigParams      *configParams,
                  const char        *NetworkParamBuffer,
                  uint8_t *ExternalMemoryHeapBase,
                  const InitializeParams *initP,
                  uint8_t *l2HeapBase)
{
    TIDL_State *state = &per_core_state[DNUM];

    // Initialize memory handles for use by the heaps
    TI_CreateMemoryHandle(&state->memObj_DMEM0, DMEM0_SCRATCH, DMEM0_SIZE);
    TI_CreateMemoryHandle(&state->memObj_DMEM1, DMEM1_SCRATCH, DMEM1_SIZE);
    TI_CreateMemoryHandle(&state->memObj_EXTMEM, ExternalMemoryHeapBase,
                          initP->tidlHeapSize);

    // Query number of memory record needed
    state->numMemRec = TIDL_numAlloc();
    state->memRec = (IALG_MemRec *)
                    TI_GetMemoryChunk(&state->memObj_EXTMEM,
                                      state->numMemRec*sizeof(IALG_MemRec), 4);

    // Initialize memory records
    int32_t status = TIDL_alloc((IALG_Params *)(createParams), NULL, 
                                state->memRec);
    printf("TIDL_alloc -> %d\n", status);

    // Use information filled out by algAlloc to perform memory allocation
    status = AllocMemRecords(state->memRec, state->numMemRec, state);
    printf("AllocMemRecords -> %d\n", status);

    int i;
    #if 0
    for (i=0; i < state->numMemRec; i++)
        printf("MemRec[%d]: 0x%x, %d, 0x%x\n", 
                i, state->memRec[i].base, 
                state->memRec[i].space, state->memRec[i].size);
    #endif

    if (status == IALG_EOK)
    {
        IM_Fxns *handle;  // Not used in TIDL_init
        printf("TIDL_init...\n");
        status = TIDL_init((IALG_Handle)(&handle),
                            state->memRec,NULL,(IALG_Params *)(createParams));
        printf("TIDL_init -> %d\n", status);
        state->handle = (TIDL_Handle) state->memRec[0].base;
    }

    TIDL_CreateParams *cp = state->handle->createParams;

    // Initialize input and output buffer descriptors
    state->inBufs.bufDesc  = state->inBufDescList;
    state->outBufs.bufDesc = state->outBufDescList;

    state->inBufs.numBufs  = tidl_AllocNetInputMem(&cp->net,
                                                  state->inBufDesc,
                                                  state);
    state->outBufs.numBufs = tidl_AllocNetOutputMem(&cp->net,
                                                   state->outBufDesc,
                                                   state);

    for(i = 0; i < state->inBufs.numBufs; i++)
        state->inBufDescList[i]     = &state->inBufDesc[i];

    for(i = 0; i < state->outBufs.numBufs; i++)
        state->outBufDescList[i]     = &state->outBufDesc[i];

    _tsc_start();
}

void ocl_tidl_process(//TIDL_CreateParams *createParams, 
               ConfigParams      *configParams,
               ProcessParams     *processParams,
               const char        *inputFrame,
                     char        *outputData)
{
    TIDL_State *state = &per_core_state[DNUM];
    TIDL_InArgs   inArgs;
    TIDL_outArgs  outArgs;
    outArgs.iVisionOutArgs.size       = sizeof(TIDL_outArgs);
    inArgs.iVisionInArgs.size         = sizeof(TIDL_InArgs);
    inArgs.iVisionInArgs.subFrameInfo = 0;
 
    printf("[%x] Processing frame %d, [%x -> %x]\n",  state->handle,
                                                processParams->frameIdx,
                                                inputFrame,
                                                outputData);

    assert (configParams->rawImage == 1);
    assert (configParams->randInput == 0);

    tidl_ReadNetInput(&state->handle->createParams->net,
                      configParams,
                      state->inBufDesc,
                      processParams->frameIdx,
                      inputFrame);

    uint64_t cycles_end;
    uint64_t cycles_start = _tsc_gettime();

    int32_t status = TIDL_process((IVISION_Handle)state->handle,
      &state->inBufs,&state->outBufs,(IVISION_InArgs *)&inArgs,(IVISION_OutArgs *)&outArgs);
    cycles_end = _tsc_gettime();
    
    printf("TIDL_process -> %d, %8.2f MCycles\n", status, 
           (cycles_end-cycles_start)*2/1000000.0);  // *2: same as in VSDK


    if (configParams->writeOutput)
    {
        size_t bytesWritten = tidl_WriteNetOutputMem(state->handle->createParams, 
                                                     state->outBufDesc, 
                                                     state->outBufs.numBufs,
                                                     outputData);

        processParams->bytesWritten = bytesWritten;

        printf("Bytes Written: %d\n", bytesWritten);
    }
}


void ocl_tidl_cleanup()
{
    TIDL_State *state = &per_core_state[DNUM];
    // Cleanup
    int status = TIDL_free((IALG_Handle)(state->handle), state->memRec);
    printf("TIDL_free -> %d\n", status);
    FreeMemRecords(state->memRec, state->numMemRec, state);
}

int32_t AllocMemRecords(IALG_MemRec * memRec,int32_t numMemRec,
                        TIDL_State *state)
{
  int32_t i;
  TIMemHandle memHdl_DMEM0  = &state->memObj_DMEM0;
  TIMemHandle memHdl_DMEM1  = &state->memObj_DMEM1;
  TIMemHandle memHdl_EXTMEM = &state->memObj_EXTMEM;

  for (i = 0; i < numMemRec; i++)
  {
    if(memRec[i].space == IALG_DARAM0) {
      memRec[i].base = TI_GetMemoryChunk(memHdl_DMEM0, memRec[i].size, 
        memRec[i].alignment);
    }
    else if(memRec[i].space == IALG_DARAM1) {
      memRec[i].base = TI_GetMemoryChunk(memHdl_DMEM1, memRec[i].size, 
        memRec[i].alignment);
    }
    else {
      //memRec[i].base = (Void *) __calloc_ddr( memRec[i].size,1);
      memRec[i].base = (Void *) TI_GetMemoryChunk(memHdl_EXTMEM,
                                              memRec[i].size,
                                              memRec[i].alignment);
      memset(memRec[i].base, 0, memRec[i].size);
    }

    if(memRec[i].base == NULL)
    {
      return IALG_EFAIL;
    }
#ifndef MEM_CONTAMINATION_DIS
    int32_t j;
    /*Currently in test application all memory are used as scratch across 
    process calls */       
    if(memRec[i].space != IALG_DARAM0){        
      for(j = 0; j < (memRec[i].size >> 2); j++){
        //((int32_t*)memRec[i].base)[j] = 0xDEADBEEF;
      }
    }
#endif
  }
  return IALG_EOK;
}

int32_t FreeMemRecords(IALG_MemRec * memRec,int32_t numMemRec,
                       TIDL_State *state)
{
  int32_t i;
  TIMemHandle memHdl_DMEM0  = &state->memObj_DMEM0;
  TIMemHandle memHdl_DMEM1  = &state->memObj_DMEM1;
  TIMemHandle memHdl_EXTMEM = &state->memObj_EXTMEM;

  for (i = 0; i < numMemRec; i++)
  {
    if(memRec[i].base == NULL)
    {
      return IALG_EFAIL;
    }
    if(memRec[i].space == IALG_DARAM0) {
      TI_ResetMemoryHandle(memHdl_DMEM0);
    }
    else if(memRec[i].space == IALG_DARAM1) {
      TI_ResetMemoryHandle(memHdl_DMEM1);
    }        
    else {
      TI_ResetMemoryHandle(memHdl_EXTMEM);
    }
  }
  return IALG_EOK;
}

int32_t tidltb_isOutDataBuff(sTIDL_Network_t *pTIDLNetStructure, int32_t dataId,
                             int32_t layersGroupId)
{
  int32_t i,j;
  for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      if((pTIDLNetStructure->TIDLLayers[i].layersGroupId != layersGroupId) &&
         (pTIDLNetStructure->TIDLLayers[i].inData[j].dataId == dataId))
      {
        return 1;
      }
    }
  }
  return 0;
}

int32_t tidltb_isInDataBuff(sTIDL_Network_t * pTIDLNetStructure, int32_t dataId,
                            int32_t layersGroupId)
{
  int32_t i,j;
  for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      if((pTIDLNetStructure->TIDLLayers[i].layersGroupId == layersGroupId) &&
         (pTIDLNetStructure->TIDLLayers[i].inData[j].dataId == dataId))
      {
        return 1;
      }
    }
  }
  return 0;
}


int32_t tidl_AllocNetInputMem(sTIDL_Network_t *net,
                              IVISION_BufDesc *BufDescList,
                              TIDL_State *state)
{
  int32_t i,j;
  uint16_t tidlMaxPad     = TIDL_MAX_PAD_SIZE;
  uint16_t numBuffs = 0;

  for(i = 0; i < net->numLayers; i++)
  {
    if(net->TIDLLayers[i].layersGroupId !=TIDL_TB_CURR_LAYERS_GROUP_ID)
    {
      for(j = 0; j < net->TIDLLayers[i].numOutBufs; j++ )
      {
        if(tidltb_isInDataBuff(net,net->TIDLLayers[i].outData[j].dataId,
           TIDL_TB_CURR_LAYERS_GROUP_ID))
        {
          BufDescList[numBuffs].numPlanes                          = 1;
          BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
          BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

          BufDescList[numBuffs].bufPlanes[0].width                 = 
                    net->TIDLLayers[i].outData[j].dimValues[3] + 2*tidlMaxPad;
                    
          BufDescList[numBuffs].bufPlanes[0].height                =
                    net->TIDLLayers[i].outData[j].dimValues[0]*
                    net->TIDLLayers[i].outData[j].dimValues[1]*
                    (net->TIDLLayers[i].outData[j].dimValues[2] + 2*tidlMaxPad);
                    
          BufDescList[numBuffs].bufPlanes[0].frameROI.width        = 
                    net->TIDLLayers[i].outData[j].dimValues[3];
                    
          BufDescList[numBuffs].bufPlanes[0].frameROI.height       = 
                    net->TIDLLayers[i].outData[j].dimValues[2];

          BufDescList[numBuffs].bufPlanes[0].buf = 
                      (int8_t *)TI_GetMemoryChunk(&state->memObj_EXTMEM, 
                      BufDescList[numBuffs].bufPlanes[0].width * 
                      BufDescList[numBuffs].bufPlanes[0].height, 4);
          BufDescList[numBuffs].reserved[0]     = 
                                        net->TIDLLayers[i].outData[j].dataId;
          
          memset(BufDescList[numBuffs].bufPlanes[0].buf,0, 
                  BufDescList[numBuffs].bufPlanes[0].width*
                  BufDescList[numBuffs].bufPlanes[0].height);
          BufDescList[numBuffs].bufferId = net->TIDLLayers[i].outData[j].dataId;

              
          numBuffs++;
        }
      }
    }
  }
  return numBuffs;
}

int32_t tidl_AllocNetOutputMem(sTIDL_Network_t *net,
                               IVISION_BufDesc *BufDescList,
                               TIDL_State *state)
{
  int32_t i,j;
  uint16_t tidlMaxPad     = TIDL_MAX_PAD_SIZE;
  uint16_t numBuffs = 0;

  for(i = 0; i < net->numLayers; i++)
  {
    if(net->TIDLLayers[i].layersGroupId == TIDL_TB_CURR_LAYERS_GROUP_ID)
    {
      for(j = 0; j < net->TIDLLayers[i].numOutBufs; j++ )
      {
        if(tidltb_isOutDataBuff(net,net->TIDLLayers[i].outData[j].dataId, 
                                TIDL_TB_CURR_LAYERS_GROUP_ID))
        {
          BufDescList[numBuffs].numPlanes                          = 1;
          BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
          BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

          uint16_t imHeight      = (uint16_t)(net->TIDLLayers[i].outData[j].dimValues[2]);
          uint16_t imWidth       = (uint16_t)(net->TIDLLayers[i].outData[j].dimValues[3]);

          uint16_t outImWidth   = TIDL_BLOCK_WIDTH > imWidth ? TIDL_BLOCK_WIDTH : ALIGN_SIZE(imWidth,TIDL_BLOCK_WIDTH); 
          uint16_t outImHeight  = TIDL_BLOCK_HEIGHT > imHeight ? TIDL_BLOCK_HEIGHT : ALIGN_SIZE(imHeight,TIDL_BLOCK_HEIGHT); 
          
          BufDescList[numBuffs].bufPlanes[0].width                 = 
                  outImWidth + 2*tidlMaxPad;
          BufDescList[numBuffs].bufPlanes[0].height                = 
                  net->TIDLLayers[i].outData[j].dimValues[0]*
                  net->TIDLLayers[i].outData[j].dimValues[1]*
                  (outImHeight + 2*tidlMaxPad);
          BufDescList[numBuffs].bufPlanes[0].frameROI.width        = 
                  net->TIDLLayers[i].outData[j].dimValues[3];
          BufDescList[numBuffs].bufPlanes[0].frameROI.height       = 
                  net->TIDLLayers[i].outData[j].dimValues[2];

          BufDescList[numBuffs].bufPlanes[0].buf = 
              (int8_t *)TI_GetMemoryChunk(&state->memObj_EXTMEM, 
              BufDescList[numBuffs].bufPlanes[0].width *
              BufDescList[numBuffs].bufPlanes[0].height,4);
          BufDescList[numBuffs].reserved[0]      = 
              net->TIDLLayers[i].outData[j].dataId;
          memset( BufDescList[numBuffs].bufPlanes[0].buf,0, 
              BufDescList[numBuffs].bufPlanes[0].width * 
              BufDescList[numBuffs].bufPlanes[0].height);
          BufDescList[numBuffs].bufferId         = 
              net->TIDLLayers[i].outData[j].dataId;
          numBuffs++;
        }
      }
    }
  }
  return numBuffs;
}

int32_t tidl_allocNetParamsMem(sTIDL_Network_t *net,
                               TIDL_State *state)
{
  int32_t i;
  for(i = 0; i < net->numLayers; i++)
  {
    if((net->TIDLLayers[i].layerType == TIDL_ConvolutionLayer) || 
       (net->TIDLLayers[i].layerType   == TIDL_Deconv2DLayer))
    {
      sTIDL_ConvParams_t *conv2dPrms =&net->TIDLLayers[i].layerParams.convParams;
      conv2dPrms->weights.bufSize =  net->weightsElementSize *
            (conv2dPrms->kernelW * conv2dPrms->kernelH * 
             conv2dPrms->numInChannels * conv2dPrms->numOutChannels)
             /conv2dPrms->numGroups;
             
      conv2dPrms->weights.ptr = 
                (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
                conv2dPrms->weights.bufSize, 4);

      conv2dPrms->bias.bufSize= net->biasElementSize*conv2dPrms->numOutChannels;
      conv2dPrms->bias.ptr= (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
                                                    conv2dPrms->bias.bufSize,4);
    }
    else if(net->TIDLLayers[i].layerType   == TIDL_BiasLayer)
    {
      sTIDL_BiasParams_t *biasPrms =&net->TIDLLayers[i].layerParams.biasParams;
      biasPrms->bias.bufSize =  net->biasElementSize * biasPrms->numChannels;
      biasPrms->bias.ptr = (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
                                                     biasPrms->bias.bufSize, 4);
    }
    else if(net->TIDLLayers[i].layerType   == TIDL_BatchNormLayer)
    {
      sTIDL_BatchNormParams_t *batchNormPrms = 
                 &net->TIDLLayers[i].layerParams.batchNormParams;
      batchNormPrms->weights.bufSize =  
                 net->weightsElementSize * batchNormPrms->numChannels;       
      batchNormPrms->weights.ptr = 
                (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
                batchNormPrms->weights.bufSize, 4);
      batchNormPrms->bias.bufSize =  
                net->biasElementSize * batchNormPrms->numChannels;
      batchNormPrms->bias.ptr = 
                (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
                batchNormPrms->bias.bufSize, 4);
      batchNormPrms->reluParams.slope.bufSize =  
                 net->slopeElementSize * batchNormPrms->numChannels;   
      if(batchNormPrms->reluParams.reluType == TIDL_PRelU)
      {

        batchNormPrms->reluParams.slope.ptr = 
          (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
          batchNormPrms->reluParams.slope.bufSize, 4);
      }
    }    
    else if(net->TIDLLayers[i].layerType   == TIDL_InnerProductLayer)
    {
      sTIDL_InnerProductParams_t *ipPrms = 
                            &net->TIDLLayers[i].layerParams.innerProductParams;      
      ipPrms->bias.bufSize =  net->biasElementSize * ipPrms->numOutNodes;
      ipPrms->bias.ptr = (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
                                        ALIGN_SIZE(ipPrms->bias.bufSize,128),4);
      ipPrms->weights.bufSize =  net->weightsElementSize* ipPrms->numInNodes * 
                                 ipPrms->numOutNodes; //ipPrms->numChannels;
      ipPrms->weights.ptr = (int8_t *)TI_GetMemoryChunk(&state->memObj_NetworkParam, 
                                     ALIGN_SIZE((ipPrms->weights.bufSize + 16*net->TIDLLayers[i].layerParams.innerProductParams.numInNodes), 1024),4);
    }     
  }
  return 0;
}


//
// tidl_fillNetParamsMem and helper functions
//
static void sparseConv2dCoffesS8(int8_t *ptr, int32_t n, uint8_t thr , 
                                 int32_t zeroWeightValue);

static void sparseConv2dCoffesS16(int16_t *ptr, int32_t n, uint8_t thr , 
                                  int32_t zeroWeightValue);


#define FREAD(dst, elemsz, elemcnt, src) \
    do {\
        memcpy(dst, src, elemsz*elemcnt); \
        src+=(elemsz*elemcnt);\
    }  while (0)

int32_t tidl_fillNetParamsMem(sTIDL_Network_t *net, 
                              const ConfigParams *params,
                              const char* NetworkParamBuffer)
{
  const char *fp1 = NetworkParamBuffer;
  int32_t i;
  uint32_t dataSize ;
  for(i = 0; i < net->numLayers; i++)
  {
    if((net->TIDLLayers[i].layerType   == TIDL_ConvolutionLayer) || 
       (net->TIDLLayers[i].layerType   == TIDL_Deconv2DLayer))
    {
      sTIDL_ConvParams_t *conv2dPrms =&net->TIDLLayers[i].layerParams.convParams;
      {
        dataSize = (conv2dPrms->numInChannels* conv2dPrms->numOutChannels* 
                conv2dPrms->kernelW*conv2dPrms->kernelH)/conv2dPrms->numGroups;

        //Read weights based on its size
        if(net->weightsElementSize == 2)
          FREAD((int8_t *)conv2dPrms->weights.ptr,1,2*dataSize, fp1);
        else
          FREAD((int8_t *)conv2dPrms->weights.ptr,1,dataSize, fp1);
          
        //printf("LayerID: %d\n", i);
        //print_buffer(conv2dPrms->weights.ptr, dataSize * (net->weightsElementSize == 2 ? 2 : 1));

        if(params->noZeroCoeffsPercentage < 100)
        {
          if(net->weightsElementSize == 2)
          {
            sparseConv2dCoffesS16((int16_t *)conv2dPrms->weights.ptr, 
              conv2dPrms->weights.bufSize, params->noZeroCoeffsPercentage*2.55,conv2dPrms->zeroWeightValue);
          }
          else
          {
            sparseConv2dCoffesS8((int8_t *)conv2dPrms->weights.ptr, 
              conv2dPrms->weights.bufSize, params->noZeroCoeffsPercentage*2.55,conv2dPrms->zeroWeightValue);
          }
        }

        if(conv2dPrms->enableBias)
        {
          dataSize = conv2dPrms->numOutChannels;
          FREAD((int8_t *)conv2dPrms->bias.ptr,1,dataSize*2, fp1);
        }
        else
        {
          memset((int8_t *)conv2dPrms->bias.ptr,0,conv2dPrms->numOutChannels*2);
        }
      }
    }
    else if(net->TIDLLayers[i].layerType == TIDL_BiasLayer)
    {
      sTIDL_BiasParams_t *biasPrms =&net->TIDLLayers[i].layerParams.biasParams;
      dataSize = biasPrms->numChannels;      
      FREAD((int8_t *)biasPrms->bias.ptr,1,dataSize*2, fp1);
    } 
    else if(net->TIDLLayers[i].layerType == TIDL_BatchNormLayer)
    {
      sTIDL_BatchNormParams_t *bNPrms =&net->TIDLLayers[i].layerParams.batchNormParams;
      dataSize = bNPrms->numChannels;  
      if(net->weightsElementSize == 2)
        FREAD((int8_t *)bNPrms->weights.ptr,1,2*dataSize, fp1);
	  else
        FREAD((int8_t *)bNPrms->weights.ptr,1,dataSize, fp1);

      FREAD((int8_t *)bNPrms->bias.ptr,1,dataSize*2, fp1);

      if(bNPrms->reluParams.reluType == TIDL_PRelU)
      {
          if(net->slopeElementSize == 2)
            FREAD((int8_t *)bNPrms->reluParams.slope.ptr,1,2*dataSize, fp1);
		  else
            FREAD((int8_t *)bNPrms->reluParams.slope.ptr,1,dataSize, fp1);
      }
    }    
    else if(net->TIDLLayers[i].layerType == TIDL_InnerProductLayer)
    {
      sTIDL_InnerProductParams_t *ipPrms = 
                             &net->TIDLLayers[i].layerParams.innerProductParams;      
      dataSize = ipPrms->numInNodes * ipPrms->numOutNodes;      

      //Read weights based on its size
      if(net->weightsElementSize == 2)
		  FREAD((int8_t *)ipPrms->weights.ptr,1,2*dataSize, fp1);
	  else
          FREAD((int8_t *)ipPrms->weights.ptr,1,dataSize, fp1);
      dataSize = ipPrms->numOutNodes;      

      FREAD((int8_t *)ipPrms->bias.ptr,1,dataSize*2, fp1);
    }     
  }

  return 0;
}


static void sparseConv2dCoffesS8(int8_t *ptr, int32_t n, uint8_t thr , int32_t zeroWeightValue)
{
  int32_t   i0;
  for(i0 = 0; i0 < n; i0++)
  {
    if(((uint8_t)(rand() & 0xFF)) > thr)
    {
      ptr[i0] =  zeroWeightValue;
    }
  }
}

static void sparseConv2dCoffesS16(int16_t *ptr, int32_t n, uint8_t thr , int32_t zeroWeightValue)
{
  int32_t   i0;
  for(i0 = 0; i0 < n; i0++)
  {
    if(((uint8_t)(rand() & 0xFF)) > thr)
    {
      ptr[i0] =  zeroWeightValue;
    }
  }
}


//
// tidl_ReadNetInput and helper functions
//
static int32_t readDataS8(const char *readPtr, int8_t *ptr, int16_t roi, int16_t n, 
                          int16_t width, int16_t height, int16_t pitch, 
                          int32_t chOffset);


int32_t tidl_ReadNetInput(sTIDL_Network_t *net, const ConfigParams *params,
                          IVISION_BufDesc *BufDescList, int32_t frameCount,
                          const char *inputFrame)
{
  int32_t i,j;
  uint16_t tidlMaxPad     = TIDL_MAX_PAD_SIZE;
  uint16_t numBuffs = 0;
  const char *readPtr = inputFrame;

  for(i = 0; i < net->numLayers; i++)
  {
    if(net->TIDLLayers[i].layersGroupId !=TIDL_TB_CURR_LAYERS_GROUP_ID)
    {
      for(j = 0; j < net->TIDLLayers[i].numOutBufs; j++ )
      {
        if(tidltb_isInDataBuff(net,net->TIDLLayers[i].outData[j].dataId,
           TIDL_TB_CURR_LAYERS_GROUP_ID))
        {
          size_t bytesRead = readDataS8(readPtr,
                  ((int8_t *)BufDescList[numBuffs].bufPlanes[0].buf +
                  BufDescList[numBuffs].bufPlanes[0].width*tidlMaxPad + 
                  tidlMaxPad), net->TIDLLayers[i].outData[j].dimValues[0],
                  net->TIDLLayers[i].outData[j].dimValues[1],    
                  net->TIDLLayers[i].outData[j].dimValues[3],
                  net->TIDLLayers[i].outData[j].dimValues[2], 
                  BufDescList[numBuffs].bufPlanes[0].width, 
                  BufDescList[numBuffs].bufPlanes[0].width* 
              (net->TIDLLayers[i].outData[j].dimValues[2] + 2*tidlMaxPad));
            readPtr += bytesRead;
            
          numBuffs++;
        }
      }
    }
  }
  return 0;
}

static int32_t readDataS8(const char *readPtr, int8_t *ptr, int16_t roi, int16_t n, 
                          int16_t width, int16_t height, int16_t pitch, 
                          int32_t chOffset)
{
  int32_t   i0, i1, i2, i3;

  if(readPtr)
  {
    for(i2 = 0; i2 < roi; i2++)
    {
      for(i0 = 0; i0 < n; i0++)
      {
        for(i1 = 0; i1 < height; i1++)
        {
          for(i3 = 0; i3 < width; i3++)
          {
           ptr[i2*n*chOffset + i0*chOffset + i1*pitch + i3] = readPtr[i2*n*width*height + i0*width*height+ i1*width + i3];
          }
        }
      }
    }
    return width*height*n*roi;
  }
  else
  {
    return 0;
  }
}


//
// tidl_WriteNetOutputMem and helper functions
//
static size_t writeDataS8(char *writePtr, int8_t *ptr, int16_t n, 
                          int16_t width, int16_t height, int16_t pitch, 
                          int32_t chOffset);

static sTIDL_DataParams_t *tidl_getDataBufDims(const sTIDL_Network_t *net, 
                                               int32_t dataBuffId);

size_t tidl_WriteNetOutputMem (const TIDL_CreateParams *createParams,
                                IVISION_BufDesc *BufDescList, 
                                uint16_t numBuffs,
                                char    *outputData)
{
  int32_t i;
  uint16_t tidlMaxPad     = TIDL_MAX_PAD_SIZE;
  uint16_t nuChs;

  size_t totalBytesWritten = 0;

  for(i = 0; i < numBuffs; i++)
  {
    sTIDL_DataParams_t * dataBuffParam;
    dataBuffParam = tidl_getDataBufDims(&createParams->net,BufDescList[i].bufferId);

    nuChs = dataBuffParam->dimValues[1];
    size_t bytesWritten = writeDataS8(outputData,
                ((int8_t *)BufDescList[i].bufPlanes[0].buf + 
                BufDescList[i].bufPlanes[0].width*tidlMaxPad + tidlMaxPad),
                nuChs, BufDescList[i].bufPlanes[0].frameROI.width ,
                BufDescList[i].bufPlanes[0].frameROI.height ,
                BufDescList[i].bufPlanes[0].width,
                BufDescList[i].bufPlanes[0].width*
                (BufDescList[i].bufPlanes[0].frameROI.height + 2*tidlMaxPad));
    outputData += bytesWritten;

    totalBytesWritten += bytesWritten;
  }
  return totalBytesWritten;
}

size_t writeDataS8(char *writePtr, int8_t *ptr, int16_t n, int16_t width,
                   int16_t height, int16_t pitch, int32_t chOffset)
{
  int32_t   i0, i1;

  if(writePtr)
  {
    for(i0 = 0; i0 < n; i0++)
    {
      for(i1 = 0; i1 < height; i1++)
      {
        memcpy(&writePtr[i0*width*height+ i1*width], 
               &ptr[i0*chOffset + i1*pitch],width);
      }
    }
    return (width*height*n);
  }

  return 0;
}

sTIDL_DataParams_t *tidl_getDataBufDims(const sTIDL_Network_t *net, 
                                        int32_t dataBuffId)
{
  int32_t i,j;
  for (i = 0 ; i < net->numLayers; i++)
  {
    for (j = 0; j < net->TIDLLayers[i].numOutBufs; j++)
    {       
      if( (net->TIDLLayers[i].outData[j].dataId == dataBuffId))
      {

        return (sTIDL_DataParams_t *)&(net->TIDLLayers[i].outData[j]);
      }
    }
  }
  return 0;
}

#ifdef PRINT_NET_INFO
const char * TIDL_LayerString[] =
{
    "TIDL_DataLayer           ",
    "TIDL_ConvolutionLayer    ",
    "TIDL_PoolingLayer        ",
    "TIDL_ReLULayer           ",
    "TIDL_PReLULayer          ",
    "TIDL_EltWiseLayer        ",
    "TIDL_InnerProductLayer   ",
    "TIDL_SoftMaxLayer        ",
    "TIDL_BatchNormLayer      ",
    "TIDL_BiasLayer           ",
    "TIDL_ScaleLayer          ",
    "TIDL_Deconv2DLayer       ",
    "TIDL_ConcatLayer         ",
    "TIDL_SplitLayer          ",
    "TIDL_SliceLayer          ",
    "TIDL_CropLayer           ",
    "TIDL_FlatternLayer       ",
    "TIDL_DropOutLayer        ",
    "TIDL_ArgMaxLayer         ",
};

int32_t tidltb_printNetInfo(sTIDL_Network_t * pTIDLNetStructure, 
                            int32_t layersGroupId)
{
  int32_t i,j;
  for (i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    printf("%3d, %-20s,",i, 
                TIDL_LayerString[pTIDLNetStructure->TIDLLayers[i].layerType]);
    printf("%3d, %3d,%3d,",pTIDLNetStructure->TIDLLayers[i].layersGroupId, 
                             pTIDLNetStructure->TIDLLayers[i].numInBufs,
                             pTIDLNetStructure->TIDLLayers[i].numOutBufs);
                             
    printf(" i[");
    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++) 
    {
      printf("%2d,",pTIDLNetStructure->TIDLLayers[i].inData[j].dataId);
    }
    printf("] o[");

    printf("%3d,",pTIDLNetStructure->TIDLLayers[i].outData[0].dataId);
    printf("] ");

    printf(" i[");
    for (j = 0; j < 4; j++) 
    {
      printf("%5d,",pTIDLNetStructure->TIDLLayers[i].inData[0].dimValues[j]);
    }
    printf("] o[");
    for (j = 0; j < 4; j++) 
    {
      printf("%5d,",pTIDLNetStructure->TIDLLayers[i].outData[0].dimValues[j]);
    }
    printf("]\n");
  }
  return 0;
}
#endif

void print_buffer_with_header(const char *str, const unsigned char *p, int n)
{
    printf("%s:\n", str);
    print_buffer(p, n);
}

void print_buffer(const unsigned char *p, int n)
{
    int i;
    printf("%3d ", p[0]);
    for (i = 1; i < n; i++)
    {
        if (i % 16 == 0)
            printf("\n");
        printf("%3d ", p[i]);
    }
    printf("\n---------------------------\n");
}

void print_buffer32(const unsigned int *p, int n)
{
    int i;
    printf("%8d ", p[0]);
    for (i = 1; i < n; i++)
    {
        if (i % 16 == 0)
            printf("\n");
        printf("%8d ", p[i]);
    }
    printf("\n---------------------------\n");
}


#if 0
#define FILE_NAME_SIZE (128)
uint8_t traceDumpBaseName[FILE_NAME_SIZE];
uint8_t traceDumpName[FILE_NAME_SIZE];

/* A utility function to reverse a string  */
void tidl_reverse(char str[], int32_t length)
{
  int32_t start = 0;
  int32_t end = length -1;
  int8_t temp;
  while (start < end)
  {
    temp = *(str+end);
    *(str+end) = *(str+start);
    *(str+start) = temp;
    start++;
    end--;
  }
}

// Implementation of itoa()
char* tidl_itoa(int32_t num, char* str, int32_t base)
{
  int32_t i = 0;
  int32_t isNegative = 0;

  /* Handle 0 explicitely, otherwise empty string is printed for 0 */
  if (num == 0)
  {
    str[i++] = '0';
    str[i] = '\0';
    return str;
  }

  // In standard itoa(), negative numbers are handled only with 
  // base 10. Otherwise numbers are considered unsigned.
  if (num < 0 && base == 10)
  {
    isNegative = 1;
    num = -num;
  }

  // Process individual digits
  while (num != 0)
  {
    int32_t rem = num % base;
    str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
    num = num/base;
  }

  // If number is negative, append '-'
  if (isNegative)
    str[i++] = '-';

  str[i] = '\0'; // Append string terminator

  // Reverse the string
  tidl_reverse(str, i);

  return str;
}

int32_t tidl_writeTraceDataBuf(int8_t * ptr, sTIDL_Network_t * net, int32_t dataBuffId)
{
  char DataIDString[10];
  sTIDL_DataParams_t * dataBuffParam;

  if (dataBuffId != 0 && dataBuffId != 1)
      return 0;

  dataBuffParam = tidl_getDataBufDims(net,dataBuffId);

  if(dataBuffParam)
  {
    int32_t dataSize = dataBuffParam->dimValues[1]*dataBuffParam->dimValues[2] *
                       dataBuffParam->dimValues[3];
    int32_t linePitch = dataBuffParam->pitch[TIDL_LINE_PITCH];
    int32_t chPitch   = dataBuffParam->pitch[TIDL_CHANNEL_PITCH];
    int8_t * outPtr   = (int8_t * )__malloc_ddr(dataSize);
    int32_t i, j;

    if(outPtr == NULL)
    {
      return -1;
    }
    for(j = 0; j < dataBuffParam->dimValues[1]; j++)
    {
      for(i = 0; i < dataBuffParam->dimValues[2]; i++)
      {
        memcpy((outPtr+j*dataBuffParam->dimValues[2]*dataBuffParam->dimValues[3]+
                i*dataBuffParam->dimValues[3]), 
                (ptr+j*chPitch+i*linePitch + TIDL_MAX_PAD_SIZE*linePitch + 
                TIDL_MAX_PAD_SIZE),
                dataBuffParam->dimValues[3]);
      }
    }

    tidl_itoa(dataBuffId,(char *)DataIDString,10);
    strcpy((char *)traceDumpName,(char *)traceDumpBaseName);
    strcat((char *)traceDumpName,(char *)DataIDString);
    strcat((char *)traceDumpName,"_");
    tidl_itoa(dataBuffParam->dimValues[3],(char *)DataIDString,10);
    strcat((char *)traceDumpName,(char *)DataIDString);
    strcat((char *)traceDumpName,"x");
    tidl_itoa(dataBuffParam->dimValues[2],(char *)DataIDString,10);
    strcat((char *)traceDumpName,(char *)DataIDString);
    strcat((char *)traceDumpName,".y");

    printf("%s\n", traceDumpName);
    print_buffer((unsigned char *)outPtr, 512);
    __free_ddr(outPtr);
  }

  return 0;
}

int32_t tidl_writeLayerMinMax(sTIDL_Network_t * net)
{ return 0; }
#endif
