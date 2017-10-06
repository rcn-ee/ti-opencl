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

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <stdint.h>
#include <src/rtos/utils_common/include/utils_eveloader.h>
#include <ti/csl/hw_types.h>
#include <ti/csl/soc.h>
#include <ti/boot/sbl_auto/sbl_lib/sbl_lib.h>
#include <ti/drv/pm/pmhal.h>

/****************************************************************
 *  GLOBAL VARIABLES
 ****************************************************************/
#ifdef IPU1_LOAD_EVES
/* This expected to be defined in IPU1 application */
extern unsigned char gTDA2XX_EVE_FIRMWARE;
#endif

/****************************************************************
 *  FUNCTION DEFINITIONS
 ****************************************************************/

/**
 * \brief   This function copies data from DDR
 *
 * \param   dstAddr       Destination Address
 * \param   srcOffsetAddr NOR Source Offset Address
 * \param   length        The length of data block to be copied.
 *
 * \return  status        Whether copy is done successfully.
 */
Int32 Utils_ddr3ReadLocal(void    *dstAddr,
                          UInt32 srcOffsetAddr,
                          UInt32 length)
{
    memcpy((UInt32 *)dstAddr, (UInt32 *)srcOffsetAddr, length);

    return 0;
}

/**
 * \brief   This function moves the read head by n bytes.
 *
 * \param   srcAddr     Read head pointer.
 * \param   numBytes    Number of bytes of data by which read head is moved.
 *
 * \return  None
 */
void Utils_ddr3Seek(UInt32 *srcAddr, UInt32 numBytes)
{
    *(srcAddr) = numBytes;
}

/**
 * \brief   This is a dummy DDR copy function. TDA2xx SOC family doesn't have
 *          CRC feature and hence data is directly read from boot media.
 *
 * \param   dstAddr   Destination Address
 * \param   srcAddr   DDR Source Address
 * \param   length    The length of data block to be copied.
 *
 * \return  status    Whether data is copied correctly
 */
Int32 Utils_dummyDDRRead(void   *dstAddr,
                         UInt32  srcAddr,
                         UInt32  length)
{
    Int32 retVal = 0;

    /* This is dummy function */
    return retVal;
}

Int32 Utils_loadAppImage(sbllibAppImageParseParams_t *imageParams)
{
    imageParams->appImageOffset = (UInt32) &gTDA2XX_EVE_FIRMWARE;

    SBLLibRegisterImageCopyCallback(&Utils_ddr3ReadLocal,
                                    &Utils_dummyDDRRead,
                                    &Utils_ddr3Seek);

    return (SBLLibMultiCoreImageParseV1(imageParams));
}

void Utils_resetAllEVECores(void)
{
    Int32 retVal = SYSTEM_LINK_STATUS_SOK;

    /* Enable EVE clock domains */
    retVal = PMHALCMSetCdClockMode(
        (pmhalPrcmCdId_t) PMHAL_PRCM_CD_EVE1,
        (pmhalPrcmCdClkTrnModes_t) PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
        PM_TIMEOUT_NOWAIT);
    retVal += PMHALCMSetCdClockMode(
        (pmhalPrcmCdId_t) PMHAL_PRCM_CD_EVE2,
        (pmhalPrcmCdClkTrnModes_t) PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
        PM_TIMEOUT_NOWAIT);
    retVal += PMHALCMSetCdClockMode(
        (pmhalPrcmCdId_t) PMHAL_PRCM_CD_EVE3,
        (pmhalPrcmCdClkTrnModes_t) PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
        PM_TIMEOUT_NOWAIT);
    retVal += PMHALCMSetCdClockMode(
        (pmhalPrcmCdId_t) PMHAL_PRCM_CD_EVE4,
        (pmhalPrcmCdClkTrnModes_t) PMHAL_PRCM_CD_CLKTRNMODES_SW_WAKEUP,
        PM_TIMEOUT_NOWAIT);

    /* Enable EVE modules */
    retVal += PMHALModuleModeSet(
        (pmhalPrcmModuleId_t) PMHAL_PRCM_MOD_EVE1,
        (pmhalPrcmModuleSModuleMode_t) PMHAL_PRCM_MODULE_MODE_AUTO,
        PM_TIMEOUT_INFINITE);
    retVal += PMHALModuleModeSet(
        (pmhalPrcmModuleId_t) PMHAL_PRCM_MOD_EVE2,
        (pmhalPrcmModuleSModuleMode_t) PMHAL_PRCM_MODULE_MODE_AUTO,
        PM_TIMEOUT_INFINITE);
    retVal += PMHALModuleModeSet(
        (pmhalPrcmModuleId_t) PMHAL_PRCM_MOD_EVE3,
        (pmhalPrcmModuleSModuleMode_t) PMHAL_PRCM_MODULE_MODE_AUTO,
        PM_TIMEOUT_INFINITE);
    retVal += PMHALModuleModeSet(
        (pmhalPrcmModuleId_t) PMHAL_PRCM_MOD_EVE4,
        (pmhalPrcmModuleSModuleMode_t) PMHAL_PRCM_MODULE_MODE_AUTO,
        PM_TIMEOUT_INFINITE);

    if (SYSTEM_LINK_STATUS_SOK != retVal)
    {
        Vps_printf("\n UTILS: EVELOADER: EVE PRCM Failed \n");
    }

    /* Reset EVE1 */
    SBLLibCPUReset(SBLLIB_CORE_ID_EVE1);

    /* Reset EVE2 */
    SBLLibCPUReset(SBLLIB_CORE_ID_EVE2);

    /* Reset EVE3 */
    SBLLibCPUReset(SBLLIB_CORE_ID_EVE3);

    /* Reset EVE4 */
    SBLLibCPUReset(SBLLIB_CORE_ID_EVE4);
}

