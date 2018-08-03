/******************************************************************************
 * Copyright (c) 2017-2018, Texas Instruments Incorporated - http://www.ti.com/
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

/* Copied and modified from VisionSDK utils.
 */

/**
 *******************************************************************************
 *  \file utils_eveloader.c
 *
 *  \brief This file has implementation for eve loader. This is mainly used
 *         when linux is running on A15. It uses SBL from starterware to
 *         boot load eves
 *
 *
 *  \version 0.0 (Aug 2014) : [YM] First version
 *  \version 0.1 (May 2016) : [RG] Updates for new SBL
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
*/
#include "stdint.h"
#include <ti/drv/pm/pmhal.h>
#include "utils_eveloader.h"

/*******************************************************************************
 *  GLOBAL VARIABLES
 *******************************************************************************
*/

/*
 * Variable for version 1 of Meta Header structure
 */
sbllibMetaHeaderV1_t    gUtilsAppMetaHeaderV1;

/*
 * Variable for RPRC Header structure
 */
sbllibRPRCImageHeader_t gUtilsAppRPRCHeader;

/*
 * Variable for Entry points
 */
sbllibEntryPoints_t gUtilsEntryPoints =
{{0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U}};


/*******************************************************************************
 *  FUNCTION DEFINITIONS
 *******************************************************************************
*/

/**
 * \brief   This function acts as a wrapper for Print function.
 *
 * \param   message       Message to be printed.
 *
 * \return  None.
 */
void Utils_eveLoaderPrintFxn(const char *message)
{
    Vps_printf(message);
}

/**
 * \brief   Override weak definition of vectable page mask in sbl_lib.
 *
 * OpenCL Memory Map Requirement: Use 1MB vector table page alignment to
 * reduce memory usage for EVEs.  In fact, we can reuse part of the CMEM
 * memory that we previously reserved for OpenCL DSP runtime.
 * For this to work, entryPoint must be placed in the same 1MB page as
 * the vector table, so that we can correctly derive vector table address
 * from the entry point address.  This can be achieved by user application
 * in linker command file placement.
 * For example, if entryPoint is _c_int00, assuming that vector table
 * memory is large enough to hold _c_int00 as well (vector table is
 * usually very small, about 64 bytes), then add to linker command file:
 *     SECTIONS
 *     {
 *       .entry_point_page
 *       {
 *         * (.vecs)
 *         * (.text:_c_int00)
 *       } align = 0x100000 > EVE_VECS_MEM PAGE 1
 *     }
 *
 * \return  EVE vector table page mask.
 */
uint32_t SBLLibEVEGetVecTablePageMask()
{
    return 0xFFF00000U;
}

/**
 *******************************************************************************
 *
 * \brief Boots Eves with AppImage
 *
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_eveBoot(Int32 num_eve_devices)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;
    UInt32 sblBuildMode = SBLLIB_SBL_BUILD_MODE_PROD;
    sbllibAppImageParseParams_t appImgParams;
    pmhalPrcmDpllConfig_t      *dpllParams;
    pmhalPrcmSysClkVal_t sysClkFreq = PMHALCMGetSysClockFreqEnum();
    sbllibInitParams_t sblInitPrms;
    UInt32 oppId, multVal, dspMultVal, dspDivVal;

    /* Default initialization of SBL Lib Params */
    SBLLibInitParamsInit(&sblInitPrms);

    /* Assign SBL Params */
    sblInitPrms.printFxn = &Utils_eveLoaderPrintFxn;
    SBLLibInit(&sblInitPrms);

    /* Configure DPLL EVE if it is not already configured, e.g. by u-boot */
    multVal = PMHALCMDpllGetMultiplier(PMHAL_PRCM_DPLL_EVE);
    if (multVal == 0U)
    {
        oppId = SBLLIB_PRCM_DPLL_OPP_HIGH;
        /* if DPLL DSP in OPP_NOM 600MHz, use OPP_NOM for EVE as well */
        dspMultVal = PMHALCMDpllGetMultiplier(PMHAL_PRCM_DPLL_DSP);
        dspDivVal  = PMHALCMDpllGetDivider(PMHAL_PRCM_DPLL_DSP);
        if ((20U * dspMultVal / (dspDivVal + 1U)) <= 600U)  /* 20MHz sys_clk */
            oppId = SBLLIB_PRCM_DPLL_OPP_NOM;

        retVal = SBLLibGetDpllStructure(PMHAL_PRCM_DPLL_EVE,
                                         sysClkFreq,
                                         oppId,
                                         &dpllParams);
        retVal += PMHALCMDpllConfigure(PMHAL_PRCM_DPLL_EVE,
                                       dpllParams,
                                       PM_TIMEOUT_INFINITE);
        if (SYSTEM_LINK_STATUS_SOK != retVal)
        {
            Vps_printf("\n EVELOADER: DPLL EVE not configured Correctly \n");
            SBLLibAbortBoot();
        }
    }

    /* Reset all EVEs */
    Utils_resetAllEVECores(num_eve_devices);

    /* Initialize App Image Params */
    SBLLibAppImageParamsInit(&appImgParams);
    appImgParams.appImgMetaHeaderV1 = &gUtilsAppMetaHeaderV1;
    appImgParams.appImgRPRCHeader   = &gUtilsAppRPRCHeader;
    appImgParams.entryPoints        = &gUtilsEntryPoints;
    appImgParams.skipDDRCopy        = 1U;

    Utils_loadAppImage(&appImgParams, num_eve_devices);

#ifdef PROC_EVE1_INCLUDE
    if (num_eve_devices >= 1)
        SBLLibEVE1BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE1],
                          sblBuildMode);
#endif
#ifdef PROC_EVE2_INCLUDE
    if (num_eve_devices >= 2)
        SBLLibEVE2BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE2],
                          sblBuildMode);
#endif
#ifdef PROC_EVE3_INCLUDE
    if (num_eve_devices >= 3)
        SBLLibEVE3BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE3],
                          sblBuildMode);
#endif
#ifdef PROC_EVE4_INCLUDE
    if (num_eve_devices >= 4)
        SBLLibEVE4BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE4],
                          sblBuildMode);
#endif

    return retVal;
}
