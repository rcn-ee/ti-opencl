/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file ocl_qmss.c
 */

#include "ocl_device_defs.h"
#include "ocl_qmss.h"
#include "message.h"
#include "monitor.h"
#include "util.h"
#include <ti/runtime/openmp/src/tomp_qmss_api.h>
#include <ti/csl/csl_cacheAux.h>

#define MIN_QMSS_DESC_SIZE 16
#define EVENT_WG_BUF_SIZE  ROUNDUP(sizeof(Msg_t), MIN_QMSS_DESC_SIZE)
#define MAX_WG_EVENTS      (1024)    // maximum WGs     in flight

/*! Queues used by the OpenCL Runtime */
FAST_SHARED(OCLQueues_t, ocl_global_queues);

/*! To reduce access time, each core has a local copy of ocl_global_queues */
PRIVATE(OCLQueues_t, ocl_queues);

/*! Memory region used by the OpenCL Runtime */
FAST_SHARED(Qmss_MemRegInfo, oclQmssMemRegInfo);

/*! Memory area used for buffer allocations */
FAST_SHARED_1D(uint8_t,  ocl_DescriptorMemory, 
               MAX_WG_EVENTS * EVENT_WG_BUF_SIZE);

/* struct defined in qmss_device.c */
#if defined (TI_66AK2H)
extern Qmss_GlobalConfigParams qmssGblCfgParams;
#else
extern Qmss_GlobalConfigParams qmssGblCfgParams[];
#endif


static bool initQmss (Qmss_MemRegInfo* regionConfigTbl, uint8_t* extLinkTbl);
static void initRegionConfig(Qmss_MemRegInfo* memRegInfo);

static Qmss_QueueHnd initFreeDesc(Qmss_MemRegInfo* memRegInfo, int32_t qid);

static void         resetQueues (void);
static bool         openQueues(OCLQueues_t *queues,Qmss_MemRegInfo *memRegInfo);
static void         closeQueues (void);

extern Qmss_QueueHnd tomp_queueOpen(int32_t qid);


/**
 * Initializes the QMSS sub system for OpenCL
 *
 * Called by the master core only. Sets up the memory region, initializes
 * QMSS and opens the queues required by OpenCL.
 *
 * NOTE: This function must be called BEFORE ocl_dispatch_once is 
 *       called on the worker cores.
 */
bool ocl_initGlobalQMSS()
{
    resetQueues();

    Qmss_MemRegInfo *memRegInfo = &oclQmssMemRegInfo;

    // Build table of region configurations
    initRegionConfig(memRegInfo);

    if (!initQmss (memRegInfo, NULL))
        return false;

    // Insert descriptor regions
    Qmss_Result region = Qmss_insertMemoryRegion (memRegInfo);
    if (region >= Qmss_MemRegion_MEMORY_REGION0)
        memRegInfo->memRegion = (Qmss_MemRegion)region;
    else
        return false;

    // Setup OCL queues
    if (!openQueues(&ocl_global_queues, memRegInfo))
        return false;

    // Ensure all writes to ocl_queue object land. This object is
    // read by worker cores. There is also a barrier after this call.
    ocl_mfence();

    return true;
}

/**
 * Per-core initialization for QMSS
 */
bool ocl_initLocalQMSS(void)
{
    // Update the core-local copy of struct
    ocl_queues = ocl_global_queues;

    if (Qmss_start() != QMSS_SOK) return false;

    return true;
}


/**
 * Close queues and exit QMSS. Called by master core only
 */
void ocl_exitGlobalQMSS(void)
{
#ifdef TI_66AK2H
    closeQueues();

    // Must remove memory region before calling Qmss_exit!
    Qmss_removeMemoryRegion(oclQmssMemRegInfo.memRegion, /*qGroup*/0);

    Qmss_exit();
#endif
}



/**
 *
 */
void initRegionConfig(Qmss_MemRegInfo* memRegInfo)
{
    // Initialize descriptor regions
    memRegInfo->descSize       = EVENT_WG_BUF_SIZE;
    memRegInfo->descNum        = MAX_WG_EVENTS;
    memRegInfo->descBase       = (uint32_t *) ocl_DescriptorMemory;
    memRegInfo->manageDescFlag = Qmss_ManageDesc_UNMANAGED_DESCRIPTOR;
    memRegInfo->memRegion      = (Qmss_MemRegion)
                                    OCL_QMSS_FIRST_MEMORY_REGION_IDX;
    memRegInfo->startIndex     = OCL_QMSS_FIRST_DESC_IDX_IN_LINKING_RAM;
}


/**
 * Open all queues required by OpenCL and populate the free queue
 */
bool openQueues(OCLQueues_t *queues, Qmss_MemRegInfo *memRegInfo)
{
    int32_t qid = OCL_QMSS_HW_QUEUE_BASE_IDX;

    queues->freeQ      = initFreeDesc(memRegInfo, qid++);
    queues->wgQ        = tomp_queueOpen(qid++);
    queues->exitQ      = tomp_queueOpen(qid++);

    Qmss_queueEmpty (queues->wgQ);
    Qmss_queueEmpty (queues->exitQ);

    return true;
}


/**
 * Close and reset all queues used by OpenCL
 */
void closeQueues(void)
{
    Qmss_queueEmpty (ocl_queues.wgQ);
    Qmss_queueEmpty (ocl_queues.freeQ);
    Qmss_queueEmpty (ocl_queues.exitQ);

    Qmss_queueClose (ocl_queues.wgQ);
    Qmss_queueClose (ocl_queues.freeQ);
    Qmss_queueClose (ocl_queues.exitQ);

    resetQueues();
}


/**
 * Reset all queues used by OpenCL 
 */
void resetQueues(void)
{
    ocl_queues.wgQ          = QMSS_PARAM_NOT_SPECIFIED;
    ocl_queues.freeQ        = QMSS_PARAM_NOT_SPECIFIED;
    ocl_queues.exitQ        = QMSS_PARAM_NOT_SPECIFIED;
}


/**
 * Initialize the free queue
 */
Qmss_QueueHnd initFreeDesc(Qmss_MemRegInfo* memRegInfo, int32_t qid)
{
    // Qmss_initDescriptor uses a temporary queue internally to store
    // descriptors. On Hawking, this temporary queue could potentially
    // conflict with queues used by Linux etc. Use Qmss_queuePushDesc
    // onto the OpenCL free queue instead.
    Qmss_QueueHnd freeDescQ = tomp_queueOpen(qid);

    uint8_t* descPtr = (uint8_t *)memRegInfo->descBase;
    uint32_t descSize = memRegInfo->descSize;

    int i;
    for (i = 0; i < memRegInfo->descNum; i++)
    {
        Qmss_queuePushDesc(freeDescQ, (void*)descPtr);
        descPtr += descSize;
    }

    return freeDescQ;
}


typedef void (*ocl_dispatchFn) (void* eventHdl);
        
static inline bool ocl_dispatchEvent(ocl_dispatchFn fn, uint32_t qid)
{
    void* event = ocl_descriptorPop(qid);
    if (event != NULL)
        (*fn)(event);

    return (event != NULL);
}

/** 
 *  OpenCL dispatch function
 */ 
void ocl_dispatch_once (void)
{      
    ocl_dispatchEvent(&queue_handler,       ocl_queues.wgQ);

    // If there is an event in the exit Q, exit() is called and we will not
    // return
    ocl_dispatchEvent(&service_exit, ocl_queues.exitQ);
}

/**
 * Initialize QMSS for OpenCL.
 */
bool initQmss (Qmss_MemRegInfo* regionConfigTbl, uint8_t* extLinkTbl)
{
    Qmss_InitCfg             qmssCfg;
    Qmss_GlobalConfigParams* qmssGlobalCfg;

    // Set up the linking RAM configuration. Use the internal Linking RAM. 
    qmssCfg.linkingRAM0Base = 0; // Use linking RAM of Navigator SS

#if defined (TI_C6678)
    qmssGlobalCfg = &qmssGblCfgParams[0];
    qmssCfg.linkingRAM0Size = 0; // Use all linking RAM of Navigator SS
    qmssCfg.linkingRAM1Base = (uint32_t)extLinkTbl;
#elif defined (TI_66AK2H)
    qmssGlobalCfg = &qmssGblCfgParams;
    // Linux already configured QMSS 
    // Retrieving the value
    // QM1 and QM2 share internal and external linking ram
    {
        CSL_Qm_configRegs *qm_config_regs_ptr;
        qm_config_regs_ptr = (CSL_Qm_configRegs *) CSL_QMSS_CFG_QM_1_CFG_REGS;
        qmssCfg.linkingRAM0Size = 
                            qm_config_regs_ptr->LINKING_RAM_REGION_0_SIZE_REG;
        qmssCfg.linkingRAM1Base = 
                    qm_config_regs_ptr->LINKING_RAM_REGION_1_BASE_ADDRESS_REG;
    }

    // mode field available only in the KS2 PDK
    qmssCfg.mode = Qmss_Mode_JOINT_LOADBALANCED;
#else
#error "Device not supported"
#endif

    // Get memory regions used by OpenMP-DSP runtime
    Qmss_MemRegInfo tomp_qmssMemRegions[OMP_NUM_QMSS_MEM_REGIONS];
    int tomp_qmssMemRegionCount;
    __TI_omp_get_qmss_memory_regions(tomp_qmssMemRegions, 
                                     &tomp_qmssMemRegionCount);

    // Initialize start index & memory region for OpenCL
    tomp_qmssMemRegions[0].startIndex = OCL_QMSS_FIRST_DESC_IDX_IN_LINKING_RAM +
                                        regionConfigTbl->descNum;
    tomp_qmssMemRegions[0].memRegion  = (Qmss_MemRegion)(OCL_QMSS_FIRST_MEMORY_REGION_IDX +
                                                         OCL_NUM_QMSS_MEM_REGIONS);

    // Update OpenCL runtime with start index & region
    __TI_omp_update_qmss_memory_regions(tomp_qmssMemRegions, 
                                        tomp_qmssMemRegionCount);

    // Max descriptor count
    qmssCfg.maxDescNum = OCL_QMSS_FIRST_DESC_IDX_IN_LINKING_RAM + 
                         regionConfigTbl->descNum +
                         tomp_qmssMemRegions[0].descNum;       


    // Pdsp's are not used by the OpenCL runtime
    int i;
    for (i=0; i<QMSS_MAX_PDSP; i++)
    {
        qmssCfg.pdspFirmware[i].pdspId   = (Qmss_PdspId)i;
        qmssCfg.pdspFirmware[i].firmware = NULL;
        qmssCfg.pdspFirmware[i].size     = 0;
    }

    // Initialize QMSS 
    if (Qmss_init (&qmssCfg, qmssGlobalCfg) != QMSS_SOK)
        return false;

    // Start QMSS
    if (Qmss_start () != QMSS_SOK)
        return false;

    return true;
}
