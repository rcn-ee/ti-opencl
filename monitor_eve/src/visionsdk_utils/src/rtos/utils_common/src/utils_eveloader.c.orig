/*
Copyright (c) [2012 - 2017] Texas Instruments Incorporated

All rights reserved not granted herein.

Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 license under copyrights and patents it now or hereafter owns or controls to
 make,  have made, use, import, offer to sell and sell ("Utilize") this software
 subject to the terms herein.  With respect to the foregoing patent license,
 such license is granted  solely to the extent that any such patent is necessary
 to Utilize the software alone.  The patent license shall not apply to any
 combinations which include this software, other than combinations with devices
 manufactured by or for TI ("TI Devices").  No hardware patent is licensed
 hereunder.

 Redistributions must preserve existing copyright notices and reproduce this
 license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

 * No reverse engineering, decompilation, or disassembly of this software
   is permitted with respect to any software provided in binary form.

 * Any redistribution and use are licensed by TI for use only with TI Devices.

 * Nothing shall obligate TI to provide you with source code for the software
   licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution of
 the source code are permitted provided that the following conditions are met:

 * Any redistribution and use of the source code, including any resulting
   derivative works, are licensed by TI for use only with TI Devices.

 * Any redistribution and use of any object code compiled from the source code
   and any resulting derivative works, are licensed by TI for use only with TI
   Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this software
 without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI’S LICENSORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI’S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <ti/boot/sbl_auto/sbl_lib/sbl_lib.h>
#include <ti/boot/sbl_auto/sbl_lib/sbl_lib_tda2xx.h>
#include <ti/drv/pm/pmhal.h>
#include <src/rtos/utils_common/include/utils.h>
#include <src/rtos/utils_common/include/utils_eveloader.h>

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
 *******************************************************************************
 *
 * \brief Boots Eves with AppImage
 *
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_eveBoot(void)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;
    UInt32 sblBuildMode = SBLLIB_SBL_BUILD_MODE_PROD;
    UInt32 oppId = SBLLIB_PRCM_DPLL_OPP_NOM;
    sbllibAppImageParseParams_t appImgParams;
    pmhalPrcmDpllConfig_t      *dpllParams;
    pmhalPrcmSysClkVal_t sysClkFreq = PMHALCMGetSysClockFreqEnum();
    sbllibInitParams_t sblInitPrms;

    /* Default initialization of SBL Lib Params */
    SBLLibInitParamsInit(&sblInitPrms);

    /* Assign SBL Params */
    sblInitPrms.printFxn = &Utils_eveLoaderPrintFxn;
    SBLLibInit(&sblInitPrms);

    /* Configure DPLL EVE */
    retVal = SBLLibGetDpllStructure(PMHAL_PRCM_DPLL_EVE,
                                     sysClkFreq,
                                     oppId,
                                     &dpllParams);

    retVal += PMHALCMDpllConfigure(PMHAL_PRCM_DPLL_EVE,
                                   dpllParams,
                                   PM_TIMEOUT_INFINITE);
    if (SYSTEM_LINK_STATUS_SOK != retVal)
    {
        Vps_printf("\n UTILS: EVELOADER: DPLL EVE not configured Correctly \n");
        SBLLibAbortBoot();
    }

    /* Reset all EVEs */
    Utils_resetAllEVECores();

    /* Initialize App Image Params */
    SBLLibAppImageParamsInit(&appImgParams);
    appImgParams.appImgMetaHeaderV1 = &gUtilsAppMetaHeaderV1;
    appImgParams.appImgRPRCHeader   = &gUtilsAppRPRCHeader;
    appImgParams.entryPoints        = &gUtilsEntryPoints;
    appImgParams.skipDDRCopy        = 1U;

    Utils_loadAppImage(&appImgParams);

#ifdef PROC_EVE1_INCLUDE
    SBLLibEVE1BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE1],
                      sblBuildMode);
#endif
#ifdef PROC_EVE2_INCLUDE
    SBLLibEVE2BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE2],
                      sblBuildMode);
#endif
#ifdef PROC_EVE3_INCLUDE
    SBLLibEVE3BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE3],
                      sblBuildMode);
#endif
#ifdef PROC_EVE4_INCLUDE
    SBLLibEVE4BringUp(gUtilsEntryPoints.entryPoint[SBLLIB_CORE_ID_EVE4],
                      sblBuildMode);
#endif

    return retVal;
}
