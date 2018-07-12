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
 *
 * \ingroup UTILS_API
 * \defgroup UTILS_EVELOADER_API APIs to load EVE binary from M4 when Linux run's on A15
 *
 * \brief This module define APIs used to load EVEs when a15 is running linux
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_eveloader.h
 *
 * \brief APIs to load EVE binary from M4 when Linux run's on A15
 *
 * \version 0.0 (Aug 2014) : [YM] First version
 * \version 0.1 (May 2016) : [RG] Updates for new SBL
 *
 *******************************************************************************
 */

#ifndef _UTILS_EVELOADER_H_
#define _UTILS_EVELOADER_H_

#ifndef NON_VISIONSDK_BUILD
#include <src/rtos/utils_common/include/utils.h>
#else
#define SYSTEM_LINK_STATUS_SOK 0x0
#define Vps_printf(...)
#endif

#include <ti/csl/soc.h>
#include "sbl_lib.h"

/**
 *******************************************************************************
 *
 * \brief Boots Eves with AppImage
 *
 * \param num_eve_devices  Number of EVEs
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_eveBoot(Int32 num_eve_devices);

/**
 *******************************************************************************
 *
 * \brief Load multi-core AppImage for EVEs
 *
 * \param imageParams  Parameters for parsing App Image
 * \param num_eve_devices  Number of EVEs, App Image may contain more images
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_loadAppImage(sbllibAppImageParseParams_t *imageParams,
                         Int32 num_eve_devices);

/**
 *******************************************************************************
 *
 * \brief Reset all EVE cores
 *
 * \param num_eve_devices  Number of EVEs
 *
 * \return None
 *
 *******************************************************************************
 */
void Utils_resetAllEVECores(Int32 num_eve_devices);

#endif

/* @} */

