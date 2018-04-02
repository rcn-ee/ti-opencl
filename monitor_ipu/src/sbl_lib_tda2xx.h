/*
 *  Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 *  \defgroup SBL_TDA2XX_LIB SBL TDA2xx Lib
 *
 *  @{
 */
/**
 *  \file     sbl_lib_tda2xx.h
 *
 *  \brief    This file contains the interfaces present in the Secondary
 *            Bootloader(SBL) Library valid for TDA2xx SOC family (TDA2xx and
 *            TDA2Ex platform). This also contains some related macros,
 *            structures and enums.
 */

#ifndef SBL_LIB_TDA2XX_H_
#define SBL_LIB_TDA2XX_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \brief  This macro contains the IPU2's internal RAM base address as accessed
 *         from L3's(SOC) view.
 */
#define SBLLIB_SOC_L3_IPU2_RAM_BASE                     ((uint32_t) 0x55020000U)

/**
 * \brief  This macro contains the maximum number of MPU cores where each
 *         dual core MPU subsystem is counted as two cores.
 */
#if defined (SOC_TDA2EX)
#define SBLLIB_MAX_MPU_CORES                            ((uint32_t) 1U)
#else
#define SBLLIB_MAX_MPU_CORES                            ((uint32_t) 2U)
#endif

/**
 * \brief  This macro contains the maximum number of M4 cores in the system.
 */
#define SBLLIB_MAX_IPU_CORES                            ((uint32_t) 4U)

/**
 * \brief  This macro contains the maximum number of DSP cores.
 */
#if defined (SOC_TDA2EX)
#define SBLLIB_MAX_DSP_CORES                            ((uint32_t) 1U)
#else
#define SBLLIB_MAX_DSP_CORES                            ((uint32_t) 2U)
#endif

/**
 * \brief  This macro contains the maximum number of EVE cores.
 */
#if defined (SOC_TDA2EX)
#define SBLLIB_MAX_EVE_CORES                            ((uint32_t) 0U)
#else
#define SBLLIB_MAX_EVE_CORES                            ((uint32_t) 4U)
#endif

/* Supported EMIF configurations used to set TDA2XX_EMIF_MODE */
#define SBLLIB_DUAL_EMIF_2X512MB                       (0U) /**< Emif mode is DUAL_EMIF_2X512MB */
#define SBLLIB_DUAL_EMIF_1GB_512MB                     (1U) /**< Emif mode is DUAL_EMIF_1GB_512MB */
#define SBLLIB_SINGLE_EMIF_256MB                       (2U) /**< Emif mode is SINGLE_EMIF_256MB */
#define SBLLIB_SINGLE_EMIF_512MB                       (3U) /**< Emif mode is SINGLE_EMIF_512MB */

/* Define addresses of EVE's internal memories for IPU as per L3's view */
#if defined (__TI_ARM_V7M4__)
#define SBLLIB_SOC_L3_EVE1_DMEM_BASE     (SOC_EVE1_DMEM_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_WBUF_BASE     (SOC_EVE1_WBUF_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_IBUF_LA_BASE   (SOC_EVE1_IBUFLA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_IBUF_HA_BASE   (SOC_EVE1_IBUFHA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_IBUF_LB_BASE   (SOC_EVE1_IBUFLB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_IBUF_HB_BASE   (SOC_EVE1_IBUFHB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_MMU0_BASE     (SOC_EVE1_MMU0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_TPTC0_BASE    (SOC_EVE1_TPTC0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE1_TPTC1_BASE    (SOC_EVE1_TPTC1_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_DMEM_BASE     (SOC_EVE2_DMEM_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_WBUF_BASE     (SOC_EVE2_WBUF_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_IBUF_LA_BASE   (SOC_EVE2_IBUFLA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_IBUF_HA_BASE   (SOC_EVE2_IBUFHA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_IBUF_LB_BASE   (SOC_EVE2_IBUFLB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_IBUF_HB_BASE   (SOC_EVE2_IBUFHB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_MMU0_BASE     (SOC_EVE2_MMU0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_TPTC0_BASE    (SOC_EVE2_TPTC0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE2_TPTC1_BASE    (SOC_EVE2_TPTC1_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_DMEM_BASE     (SOC_EVE3_DMEM_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_WBUF_BASE     (SOC_EVE3_WBUF_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_IBUF_LA_BASE   (SOC_EVE3_IBUFLA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_IBUF_HA_BASE   (SOC_EVE3_IBUFHA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_IBUF_LB_BASE   (SOC_EVE3_IBUFLB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_IBUF_HB_BASE   (SOC_EVE3_IBUFHB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_MMU0_BASE     (SOC_EVE3_MMU0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_TPTC0_BASE    (SOC_EVE3_TPTC0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE3_TPTC1_BASE    (SOC_EVE3_TPTC1_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_DMEM_BASE     (SOC_EVE4_DMEM_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_WBUF_BASE     (SOC_EVE4_WBUF_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_IBUF_LA_BASE   (SOC_EVE4_IBUFLA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_IBUF_HA_BASE   (SOC_EVE4_IBUFHA_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_IBUF_LB_BASE   (SOC_EVE4_IBUFLB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_IBUF_HB_BASE   (SOC_EVE4_IBUFHB_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_MMU0_BASE     (SOC_EVE4_MMU0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_TPTC0_BASE    (SOC_EVE4_TPTC0_BASE + 0x20000000U)
#define SBLLIB_SOC_L3_EVE4_TPTC1_BASE    (SOC_EVE4_TPTC1_BASE + 0x20000000U)
#endif

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/**
 * \brief  Enum to select the Core Id. Each dual core CPU is considered as two
 *         separate cores to get number of individual cores. Dual core CPU is
 *         added as one core in the end.
 */
typedef enum sbllibCoreId
{
    SBLLIB_CORE_ID_MPU_CPU0,
    /**< Core: Cortex A15 CPU0 */
    SBLLIB_CORE_ID_MPU_CPU1,
    /**< Core: Cortex A15 CPU1 */
    SBLLIB_CORE_ID_IPU1_CPU0,
    /**< Core: Cortex M4 (IPU1) CPU0 */
    SBLLIB_CORE_ID_IPU1_CPU1,
    /**< Core: Cortex M4 (IPU1) CPU1 */
    SBLLIB_CORE_ID_IPU1,
    /**< Core: Dual Cortex M4 (IPU1) */
    SBLLIB_CORE_ID_IPU2_CPU0,
    /**< Core: Cortex M4 (IPU2) CPU0 */
    SBLLIB_CORE_ID_IPU2_CPU1,
    /**< Core: Cortex M4 (IPU2) CPU1 */
    SBLLIB_CORE_ID_IPU2,
    /**< Core: Dual Cortex M4 (IPU2) */
    SBLLIB_CORE_ID_DSP1,
    /**< Core: C66x DSP1 */
    SBLLIB_CORE_ID_DSP2,
    /**< Core: C66x DSP2 */
    SBLLIB_CORE_ID_EVE1,
    /**< Core: EVE1 */
    SBLLIB_CORE_ID_EVE2,
    /**< Core: EVE2 */
    SBLLIB_CORE_ID_EVE3,
    /**< Core: EVE3 */
    SBLLIB_CORE_ID_EVE4,
    /**< Core: EVE4 */
    SBLLIB_CORE_ID_MPU,
    /**< Core: Dual Cortex A15 */
    SBLLIB_CORE_MAX
    /**< Enum value for maximum number of cores */
} sbllibCoreId_t;

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   This function first asserts the both CPU and sub-system resets of
 *          the IPU1 sub-system and de-asserts the sub-system reset.
 *
 * \return  status       Whether IPU1 sub-system reset is done correctly
 *                       STW_SOK   : Success
 *                       STW_EFAIL : Failure
 */
int32_t SBLLibIPU1SubsystemReset(void);

/**
 * \brief   This function first asserts the both CPU and sub-system resets of
 *          the IPU2 sub-system and de-asserts the sub-system reset.
 *
 * \return  status       Whether IPU2 sub-system reset is done correctly
 *                       STW_SOK   : Success
 *                       STW_EFAIL : Failure
 */
int32_t SBLLibIPU2SubsystemReset(void);

/**
 * \brief   This function sets the entry point and releases both cores of IPU1
 *          from reset.
 *
 * \param   ipuId           IPU's Id
 *                          SBLLIB_CORE_ID_IPU1 : IPU1
 *                          SBLLIB_CORE_ID_IPU2 : IPU2
 * \param   entryPointCPU0  CPU0's entry location on reset.
 * \param   entryPointCPU1  CPU1's entry location on reset.
 * \param   sblBuildMode    SBL's Build Mode.
 *                          Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibIPUBringUp(uint32_t ipuId,
                      uint32_t entryPointCPU0,
                      uint32_t entryPointCPU1,
                      uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the MPU CPU1 from
 *          reset.
 *
 * \param   entryPoint    MPU CPU1 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibMPUCPU1BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the IPU1 CPU0 from
 *          reset.
 *
 * \param   entryPoint    IPU1 CPU0 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibIPU1CPU0BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the IPU1 CPU1 from
 *          reset.
 *
 * \param   entryPoint    IPU1 CPU1 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibIPU1CPU1BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the IPU2 CPU0 from
 *          reset.
 *
 * \param   entryPoint    IPU2 CPU0 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibIPU2CPU0BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the IPU2 CPU1 from
 *          reset.
 *
 * \param   entryPoint    IPU2 CPU1 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibIPU2CPU1BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the EVE2 from
 *          reset.
 *
 * \param   entryPoint    EVE2 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibEVE2BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the EVE3 from
 *          reset.
 *
 * \param   entryPoint    EVE3 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibEVE3BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the EVE4 from
 *          reset.
 *
 * \param   entryPoint    EVE4 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibEVE4BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function opens up EMIF/MA_MPU/OCMC2/OCMC3 firewalls using
 *          PPA service on TDA2x HS devices.
 *          For GP devices and on cores other than A15 on HS devices,
 *          it will not do anything.
 *
 * \return  None.
 */
void SBLLibHSOpenL3Firewalls(void);

/**
 * \brief   This function configures the specified region of OCMC1 firewall
 *          as specified by the arguments using PPA service on TDA2x HS devices.
 *          PPA service will enforce blocking of all accesses to first 4kB of
 *          OCMC1 as required due to silicon erratum. To ensure this, it also
 *          prevents use of regionId 0 and 1.
 *          For GP devices and on cores other than A15 on HS devices,
 *          it will not do anything.
 *
 * \param   regionId            Region ID to be used in the firewall
 * \param   startAddress        4kB aligned start address for the region to be
 *                              firewalled
 * \param   size                Size of region to be firewalled.
 *                              Must be a multiple of 4kB
 * \param   domainAndAccessPerm Value to be configured in
 *                              MRM_PERMISSION_REGION_LOW_j where
 *                              j == regionId
 * \param   initiatorPerm       Value to be configured in
 *                              MRM_PERMISSION_REGION_HIGH_j where
 *                              j == regionId
 *
 * \return  status              Whether configuration was success
 *                              STW_SOK   : Success
 *                              STW_EFAIL : Failure
 *                              Always STW_SOK on GP devices/non-A15 cores on
 *                              HS devices
 */
int32_t SBLLibHSConfigureOcmc1Firewall(uint32_t regionId,
                                       uint32_t startAddress,
                                       uint32_t size,
                                       uint32_t domainAndAccessPerm,
                                       uint32_t initiatorPerm);

/**
 * \brief   This function authenticates a binary image defined by the arguments
 *          using a ROM HAL service on TDA2x HS devices.
 *          For GP devices and on cores other than A15 on HS devices,
 *          it will not do anything.
 *
 * \param   address Location of data blob to be authenticated
 * \param   size    Size of data blob to be authenticated.
 *                  Size includes data size and authentication signature size.
 *
 * \return  status  STW_SOK   : Image has a valid signature
 *                  STW_EFAIL : Image has a in-valid signature
 *                  Always STW_SOK on GP devices/non-A15 cores on
 *                  HS devices
 */
int32_t SBLLibHSAuthenticateBinary(uint32_t address, uint32_t size);

#ifdef __cplusplus
}

#endif

#endif
/* @} */
