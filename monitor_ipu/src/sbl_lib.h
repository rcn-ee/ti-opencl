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
 *  \defgroup SBL_LIB SBL Lib
 *
 *  @{
 */
/**
 *  \file     sbl_lib.h
 *
 *  \brief    This file contains the interfaces present in the
 *            Secondary Bootloader(SBL) Library.
 *            This also contains some related macros, structures and enums.
 */

#ifndef SBL_LIB_H_
#define SBL_LIB_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include <ti/drv/pm/pmhal.h>
#if defined (SOC_TDA3XX)
#include <ti/boot/sbl_auto/sbl_lib/sbl_lib_tda3xx.h>
#endif
#if defined (SOC_TDA2XX) || defined (SOC_TDA2PX) || defined (SOC_TDA2EX)
#include "sbl_lib_tda2xx.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#if defined (SOC_TDA3XX)
#define SBLLIB_MAX_ENTRY_POINTS          ((uint32_t) SBLLIB_CORE_INDIVIDUAL_MAX)
#elif defined (SOC_TDA2XX) || defined (SOC_TDA2PX) || defined (SOC_TDA2EX)
#define SBLLIB_MAX_ENTRY_POINTS          ((uint32_t) SBLLIB_CORE_MAX)
#endif

/**
 * \brief  This macro contains the maximum number of RPRC images in multi
 *         core App image.
 *         For tda3xx, images are: M4-0, M4-1, DSP1, DSP2, EVE1
 */
#define SBLLIB_MAX_RPRC_IMAGES                         (SBLLIB_MAX_MPU_CORES + \
                                                        SBLLIB_MAX_IPU_CORES + \
                                                        SBLLIB_MAX_DSP_CORES + \
                                                        SBLLIB_MAX_EVE_CORES)

/**
 *  \name SBL Lib DDR Type Info
 *
 *  Defines the DDR type parameter.
 *  @{
 */
/** \brief  This macro defines the value of DDR type as Invalid */
#define SBLLIB_DDR_TYPE_INVALID           (-1)
/** \brief  This macro defines the value of DDR type as DDR3 */
#define SBLLIB_DDR_TYPE_DDR3              (0)
/** \brief  This macro defines the value of DDR type as DDR3 */
#define SBLLIB_DDR_TYPE_DDR2              (1)
/** \brief  This macro defines the value of DDR type as LPDDR2 */
#define SBLLIB_DDR_TYPE_LPDDR2            (2)
/* @} */

/**
 *  \name SBL Lib DDR Speed Info
 *
 *  Defines the DDR speed parameter.
 *  @{
 */
/** \brief  This macro defines the value of DDR speed as Invalid */
#define SBLLIB_DDR_SPEED_INVALID          (-1)
/** \brief  This macro defines the value of DDR speed as 532 MHz */
#define SBLLIB_DDR_SPEED_532MHZ           (0)
/** \brief  This macro defines the value of DDR speed as 400 MHz */
#define SBLLIB_DDR_SPEED_400MHZ           (1)
/** \brief  This macro defines the value of DDR speed as 333 MHz */
#define SBLLIB_DDR_SPEED_333MHZ           (2)
/** \brief  This macro defines the value of DDR speed as 666 MHz */
#define SBLLIB_DDR_SPEED_666MHZ           (3)
/* @} */

/**
 *  \name SBL Lib Trace Level Info
 *
 *  Defines the Trace Level parameter.
 *  @{
 */
/** \brief  This macro defines the SBL Lib trace level as high */
#define SBLLIB_TRACE_LEVEL_HIGH           (4)
/** \brief  This macro defines the SBL Lib trace level as medium */
#define SBLLIB_TRACE_LEVEL_MEDIUM         (3)
/** \brief  This macro defines the SBL Lib trace level as low */
#define SBLLIB_TRACE_LEVEL_LOW            (2)
/* @} */

/**
 *  \name SBL Lib Optimization Level Info
 *
 *  Defines the Optimization Level parameter.
 *  @{
 */
/** \brief  This macro defines the SBL Lib optimization level as high */
#define SBLLIB_OPT_LEVEL_HIGH             (2)
/** \brief  This macro defines the SBL Lib optimization level as medium */
#define SBLLIB_OPT_LEVEL_MEDIUM           (1)
/** \brief  This macro defines the SBL Lib optimization level as low */
#define SBLLIB_OPT_LEVEL_LOW              (0)
/* @} */

/**
 * \brief  This macro contains the magic string to check if multi-core App
 *         image is valid(MSTR in ASCII).
 */
#define SBLLIB_META_HEADER_MAGIC_STR                    ((uint32_t) 0x5254534DU)

/**
 * \brief  This macro contains the magic string to check end of meta header
 *         (MEND in ASCII).
 */
#define SBLLIB_META_HEADER_MAGIC_END                    ((uint32_t) 0x444E454DU)

/**
 * \brief  This macro contains the magic string to check if the version of multi
 *         core application image header is 2 (VER2 in ASCII).
 */
#define SBLLIB_META_HEADER_VERSION2                     ((uint32_t) 0x32524556U)

/**
 * \brief  This macro contains the magic string to check sanity of RPRC image.
 */
#define SBLLIB_RPRC_MAGIC_STRING                        ((uint32_t) 0x43525052U)

/**
 * \brief  This macro contains the IPU1's internal RAM base address as accessed
 *         from L3's(SOC) view.
 */
#define SBLLIB_SOC_L3_IPU1_RAM_BASE                     ((uint32_t) 0x58820000U)

/**
 * \brief  This typedef contains the prototype of data copy function used
 *         to read from boot media while loading application image.
 */
typedef int32_t (*SBLLibMediaReadFxn)(void    *dstAddr,
                                      uint32_t srcAddr,
                                      uint32_t length);

/**
 * \brief  This typedef contains the prototype of data copy function used
 *         to read from DDR while loading application image.
 */
typedef int32_t (*SBLLibDDRReadFxn)(void    *dstAddr,
                                    uint32_t srcAddr,
                                    uint32_t length);

/**
 * \brief  This typedef contains the prototype of seek function used while
 *         loading image.
 */
typedef void (*SBLLibSeekFxn)(uint32_t *srcAddr, uint32_t location);

/**
 * \brief  This typedef contains the prototype of SBL print function.
 *
 * \param   pBuffer   Pointer to the buffer that should be printed
 *
 * \return  None.
 */
typedef void (*SBLLibPrintFxn)(const char *pBuffer);

/**
 * \brief  This typedef contains the prototype of SBL print hex num function.
 *
 * \param   value   value to be printed
 *
 * \return  None.
 */
typedef void (*SBLLibPrintHexNumFxn)(uint32_t tracelevel, uint32_t value);

/**
 * \brief  This typedef contains the prototype of SBL getc function.
 *
 * \param   None.
 *
 * \return  None.
 */
typedef char (*SBLLibGetcFxn)(void);

/**
 * \brief  This typedef contains the prototype of SBL putc function.
 *
 * \param   value   value to be printed.
 *
 * \return  None.
 */
typedef void (*SBLLibPutcFxn)(uint32_t tracelevel, uint8_t value);

/**
 * \brief  This typedef contains the prototype of SBL get num function.
 *
 * \param   None.
 *
 * \return  None.
 */
typedef uint32_t (*SBLLibGetNumFxn)(void);

/**
 * \brief  This typedef contains the prototype of SBL get hex num function.
 *
 * \param   None.
 *
 * \return  None.
 */
typedef uint32_t (*SBLLibGetHexNumFxn)(void);

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/**
 * \brief  Enum to select the SBL's Build Mode.
 */
typedef enum sbllibSblBuildMode
{
    SBLLIB_SBL_BUILD_MODE_PROD,
    /**< SBL's build mode is production. In production build mode, the slave
     *   core is put in a low power state (normally PD_OFF) when the Multi-core
     *   Application does not have Image for that particular core.
     */
    SBLLIB_SBL_BUILD_MODE_DEV
    /**< SBL's build mode is development. In development build mode, the slave
     *   core is brought out of reset even when the Multi-core Application does
     *   not have Image for that particular core.
     */
} sbllibSblBuildMode_t;

/**
 * \brief  Enum to select the Trace Level. Prints with ERROR and IMP_INFO are
 *         not masked and other prints are masked depending on SBL optimization
 *         level.
 */
typedef enum sbllibTraceLevel
{
    SBLLIB_TRACE_LEVEL_ERROR,
    /**< Trace Level Error: Used for error/warning messages */
    SBLLIB_TRACE_LEVEL_IMP_INFO,
    /**< Trace Level Imp Info: Used for important information */
    SBLLIB_TRACE_LEVEL_INFO,
    /**< Trace Level Info: Used for additional information */
    SBLLIB_TRACE_LEVEL_INFO1,
    /**< Trace Level Info1: Used for additional information (sub level 1) */
    SBLLIB_TRACE_LEVEL_DEBUG
    /**< Trace Level Debug: Used for debug information */
} sbllibTraceLevel_t;

/**
 * \brief Enum to select the operating point(OPP) for which DPLL should be
 *        configured.
 */
typedef enum sbllibPrcmDpllOpp
{
    SBLLIB_PRCM_DPLL_OPP_LOW = 0U,
    /**< DPLL Operating Point Low */
    SBLLIB_PRCM_DPLL_OPP_MIN = SBLLIB_PRCM_DPLL_OPP_LOW,
    /**< DPLL Operating Point Min */
    SBLLIB_PRCM_DPLL_OPP_NOM = 1U,
    /**< DPLL Operating Point NOM */
    SBLLIB_PRCM_DPLL_OPP_OD = 2U,
    /**< DPLL Operating Point OD */
    SBLLIB_PRCM_DPLL_OPP_HIGH = 3U,
    /**< DPLL Operating Point HIGH */
    SBLLIB_PRCM_DPLL_OPP_MAX = (SBLLIB_PRCM_DPLL_OPP_HIGH + 1)
                               /**< Maximum number of opp indices */
} sbllibPrcmDpllOpp_t;

/**
 * \brief  Enum to select the SBL's Boot Mode.
 */
typedef enum sbllibBootMode
{
    SBLLIB_BOOT_MODE_QSPI = 0U,
    /**< SBL's QSPI Boot mode. */
    SBLLIB_BOOT_MODE_SD = 1U,
    /**< SBL's SD Boot mode. */
    SBLLIB_BOOT_MODE_NOR = 2U,
    /**< SBL's NOR Boot mode. */
    SBLLIB_BOOT_MODE_QSPI_SD = 3U
                               /**< SBL's QSPI SD Boot mode. */
} sbllibBootMode_t;

/**
 *  \brief This structure defines the Entry Points for different cores.
 */
typedef struct sbllibEntryPoints
{
    uint32_t entryPoint[SBLLIB_MAX_ENTRY_POINTS];
    /**< Entry point for different cores for TDA3xx:
     * Array Index  0: MPU CPU0
     * Array Index  1: MPU CPU1
     * Array Index  2: IPU1 CPU0
     * Array Index  3: IPU1 CPU1
     * Array Index  4: IPU2 CPU0
     * Array Index  5: IPU2 CPU1
     * Array Index  6: DSP1
     * Array Index  7: DSP2
     * Array Index  8: EVE1
     * Array Index  9: EVE2
     * Array Index 10: EVE3
     * Array Index 11: EVE4
     *
     * Entry point for different cores for TDA2xx:
     * Array Index  0: MPU CPU0
     * Array Index  1: MPU CPU1
     * Array Index  2: IPU1 CPU0
     * Array Index  3: IPU1 CPU1
     * Array Index  4: Hole due to SMP
     * Array Index  5: IPU2 CPU0
     * Array Index  6: IPU2 CPU1
     * Array Index  7: Hole due to SMP
     * Array Index  8: DSP1
     * Array Index  9: DSP2
     * Array Index 10: EVE1
     * Array Index 11: EVE2
     * Array Index 12: EVE3
     * Array Index 13: EVE4
     * Array Index 14: Hole due to SMP
     */
} sbllibEntryPoints_t;

/**
 *  \brief This structure defines the start of meta header of multi-core
 *         app image for version 2 of Multi-core header.
 */
typedef struct sbllibMetaHeaderStartV2
{
    uint32_t magicStr;
    /**< Magic String to check if multi-core app image is valid */
    uint32_t numFiles;
    /**< Number of files in multi-core app image */
    uint32_t devId;
    /**< Device ID of the SoC */
    uint32_t metaHeaderCrcH;
    /**< Upper 32 bits of meta header's CRC signature */
    uint32_t metaHeaderCrcL;
    /**< Lower 32 bits of meta header's CRC signature */
    uint32_t multicoreImageSize;
    /**< Size of the multi-core app image */
    uint32_t metaHeaderVersion;
    /**< Meta Header Version */
}sbllibMetaHeaderStartV2_t;

/**
 *  \brief This structure defines the core meta header of multi-core
 *         app image for version 2 of Multi-core header.
 */
typedef struct sbllibMetaHeaderCoreV2
{
    uint32_t isRprcImageValid;
    /**< Core ID of RPRC image */
    uint32_t coreId;
    /**< Core ID of RPRC image */
    uint32_t imageOffset;
    /**< Image Offset with respect to multi-core image */
    uint32_t rprcImageCrcH;
    /**< Upper 32 bits of RPRC image's CRC signature */
    uint32_t rprcImageCrcL;
    /**< Lower 32 bits of RPRC image's CRC signature */
    uint32_t rprcImageSize;
    /**< Size of the RPRC image */
}sbllibMetaHeaderCoreV2_t;

/**
 *  \brief This structure defines the end of meta header of multi-core
 *         app image.
 */
typedef struct sbllibMetaHeaderEnd
{
    uint32_t rsvd;
    /**< Reserved word */
    uint32_t magicStringEnd;
    /**< Magic String to check end of Meta Header */
}sbllibMetaHeaderEnd_t;

/**
 *  \brief This structure defines the start of meta header of multi-core
 *         app image for version 1 of Multi-core header.
 */
typedef struct sbllibMetaHeaderStartV1
{
    uint32_t magicStr;
    /**< Magic String to check if multi-core app image is valid */
    uint32_t numFiles;
    /**< Number of files in multi-core app image */
    uint32_t devId;
    /**< Device ID of the SoC */
    uint32_t rsvd;
    /**< Reserved word */
}sbllibMetaHeaderStartV1_t;

/**
 *  \brief This structure defines the core meta header of multi-core
 *         app image for version 1 of Multi-core header.
 */
typedef struct sbllibMetaHeaderCoreV1
{
    uint32_t coreId;
    /**< Core ID of RPRC image */
    uint32_t imageOffset;
    /**< Image Offset with respect to multi-core image */
}sbllibMetaHeaderCoreV1_t;

/**
 *  \brief This structure defines the file header of RPRC image.
 */
typedef struct sbllibRprcFileHeader
{
    uint32_t magicStr;
    /**< Magic String to check sanity of RPRC image */
    uint32_t entryPoint;
    /**< Entry Point of CPU */
    uint32_t rsvdAddr;
    /**< Reserved word */
    uint32_t sectionCount;
    /**< Number of sections in RPRC image */
    uint32_t rsvd;
    /**< Reserved word */
} sbllibRprcFileHeader_t;

/**
 *  \brief This structure defines the section header of RPRC image.
 */
typedef struct sbllibRprcSectionHeader
{
    uint32_t loadAddr;
    /**< Load address of the section */
    uint32_t rsvdAddr;
    /**< Reserved word */
    uint32_t sectionSize;
    /**< Size of the section */
    uint32_t rsvdCrc;
    /**< Reserved word */
    uint32_t rsvd;
    /**< Reserved word */
} sbllibRprcSectionHeader_t;

/**
 *  \brief This structure contains the all individual headers required for
 *         the version 2 of meta header of multi-core app image.
 */
typedef struct sbllibMetaHeaderV2
{
    sbllibMetaHeaderStartV2_t metaHeaderStartV2;
    /**< Start Meta Header of Multi-core App Image */
    sbllibMetaHeaderCoreV2_t  metaHeaderCoreV2[SBLLIB_MAX_RPRC_IMAGES];
    /**< Core Meta Header of Multi-core App Image */
    sbllibMetaHeaderEnd_t     metaHeaderEnd;
    /**< End Meta Header of Multi-core App Image */
}sbllibMetaHeaderV2_t;

/**
 *  \brief This structure contains the all individual headers required for
 *         the version 1 of meta header of multi-core app image.
 */
typedef struct sbllibMetaHeaderV1
{
    sbllibMetaHeaderStartV1_t metaHeaderStartV1;
    /**< Start Meta Header of Multi-core App Image */
    sbllibMetaHeaderCoreV1_t  metaHeaderCoreV1[SBLLIB_MAX_RPRC_IMAGES];
    /**< Core Meta Header of Multi-core App Image */
    sbllibMetaHeaderEnd_t     metaHeaderEnd;
    /**< End Meta Header of Multi-core App Image */
}sbllibMetaHeaderV1_t;

/**
 *  \brief This structure contains the all individual headers required for
 *         the RPRC header of multi-core app image.
 *
 *  \note  This has only one instance of RPRC header.
 */
typedef struct sbllibRPRCImageHeader
{
    sbllibRprcFileHeader_t    rprcFileHeader;
    /**< RPRC file header */
    sbllibRprcSectionHeader_t rprcSectionHeader;
    /**< RPRC section header */
}sbllibRPRCImageHeader_t;

/**
 *  \brief This structure defines the SBL lib Initialization parameters.
 */
typedef struct sbllibInitParams
{
    SBLLibPrintFxn       printFxn;
    /**< SBL Lib Print Function */
    SBLLibPrintHexNumFxn printHexNumFxn;
    /**< SBL Lib Print Hex Num Function */
    SBLLibGetcFxn        getcFxn;
    /**< SBL Lib Get Char Function */
    SBLLibPutcFxn        putcFxn;
    /**< SBL Lib Put Char Function */
    SBLLibGetNumFxn      getNumFxn;
    /**< SBL Lib Get Number Function */
    SBLLibGetHexNumFxn   getHexNumFxn;
    /**< SBL Lib Get Hex Number Function */
}sbllibInitParams_t;

/**
 *  \brief This structure defines the multi-core application image parse
 *         parameters.
 */
typedef struct sbllibAppImageParseParams
{
    sbllibMetaHeaderV1_t    *appImgMetaHeaderV1;
    /**< Multi-core application image meta header (Version 1) */
    sbllibMetaHeaderV2_t    *appImgMetaHeaderV2;
    /**< Multi-core application image meta header (Version 2) */
    sbllibRPRCImageHeader_t *appImgRPRCHeader;
    /**< Multi-core application RPRC header */
    sbllibEntryPoints_t     *entryPoints;
    /**< Structure containing entry points for different cores */
    uint32_t                 appImageOffset;
    /**< Multi-core image offset in Boot Media */
    uint32_t                 ddrAppImageAddress;
    /**< DDR location where RPRC image is copied before parsing */
    uint32_t                 skipDDRCopy;
    /**< Skip DDR copy. Set to 1 when AppImage is already DDR.
     *   One such case is EVELOADER on IPU in VisionSDK with A15-Linux.
     *   In this case only gMediaReadFxn is used by SBLLIB
     */
    uint32_t                 enableCrc;
    /**< Whether to enable CRC on Multi-core application image */
}sbllibAppImageParseParams_t;

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   This API initializes the default parameters for SBL Lib.
 *
 * \param   sbllibInitPrms         SBL Lib Initialization parameters.
 *
 * \return  None.
 */
static inline void SBLLibInitParamsInit(sbllibInitParams_t *sbllibInitPrms);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   This function first asserts the both CPU and sub-system resets of
 *          slave core and de-asserts the sub-system reset.
 *
 * \param   cpuId   CPU ID of CPU that is to be reset. Refer to enum
 *                  #sbllibCoreId_t for values.
 *
 * \return  None.
 *
 * \note    Valid CPU IDs depend on slave cores present in SoC. This API can't
 *          be used for slave IPU cores in case of TDA2xx SOC family.
 */
void SBLLibCPUReset(uint32_t cpuId);

/**
 * \brief   This function reset all the slave cores of the SoC. The number of
 *          slave cores depends on the SoC.
 *
 * \return  None.
 *
 * \note    On tda3xx slave cores are DSP1, DSP2 and EVE1 (or only EVE)
 */
void SBLLibResetAllSlaveCores(void);

/**
 * \brief   This function sets the entry point and releases the DSP1 from
 *          reset.
 *
 * \param   entryPoint    DSP1 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibDSP1BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the DSP2 from
 *          reset.
 *
 * \param   entryPoint    DSP2 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibDSP2BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function sets the entry point and releases the EVE1 from
 *          reset.
 *
 * \param   entryPoint    EVE1 entry location on reset.
 * \param   sblBuildMode  SBL's Build Mode.
 *                        Refer enum #sbllibSblBuildMode_t for values.
 *
 * \return  None.
 */
void SBLLibEVE1BringUp(uint32_t entryPoint, uint32_t sblBuildMode);

/**
 * \brief   This function returns the EVE vector table page mask that is
 *          used to derive the vector table address from the entry point.
 *
 *          The implementation in this library return 0xFF000000U which means
 *          that the vector table has been placed at the beginning of a 16MB
 *          page that the entry point is located.  This is the default
 *          behavior of sbl_lib.
 *
 *          However, the implementation in this library is a weak function
 *          definition (see src/sbl_lib_weak.c).  User application could
 *          re-define this function in their own code as a strong definition
 *          (i.e. without "weak" function attribute) to override the weak
 *          function in this library.
 *
 *          If overridden, one possible valid return value is 0xFFF00000U,
 *          which means that the vector table has been placed at the beginning
 *          of an 1MB page that the entry point is located.  This could
 *          potentially be used to reduce the memory footprint.  For example,
 *          vector tables of 4 EVEs need to be placed at the beginnings of 4
 *          different 16MB pages by default, with this override, they can
 *          be placed at the beginnings of 4 different 1MB pages.
 *          Of course, user application needs to ensure that the vector table
 *          and the entry point are located in the same 1MB page.  This can
 *          be arranged by exlicit memory placement or linker command file:
 *          SECTIONS
 *          {
 *            .entry_point_page
 *            {
 *              * (.vecs)
 *              * (.text:_c_int00)
 *            } align = 0x100000 > EVE_VECS_MEM PAGE 1
 *          }
 *
 * \return  EVE vector table page mask.
 */
uint32_t SBLLibEVEGetVecTablePageMask(void);

/**
 * \brief   This function parses the multi-core App image version V1. It reads
 *          image header and check for valid AppImage. Device Id field confirms
 *          the boot device ID. Then it parses the meta header and finds
 *          the number of executables. It first copies the individual RPRC image
 *          to DDR memory and then parses and loads the different sections
 *          into CPU memory and internal memory.
 *
 * \param   imageParams       Multi-core Application Image Parse Parameters.
 *                            Refer struct #sbllibAppImageParseParams_t for
 *                            details.
 *
 * \return  status            Whether image is parsed and copied correctly
 *                            STW_SOK   : Success
 *                            STW_EFAIL : Failure
 */
int32_t SBLLibMultiCoreImageParseV1(sbllibAppImageParseParams_t *imageParams);

/**
 * \brief   This function parses the multi-core App image version V2. It reads
 *          image header and check for valid AppImage. Device Id field confirms
 *          the boot device ID. Then it parses the meta header and finds
 *          the number of executables. It first copies the individual RPRC image
 *          to DDR memory and then parses and loads the different sections
 *          into CPU memory and internal memory.
 *
 * \param   imageParams       Multi-core Application Image Parse Parameters.
 *                            Refer struct #sbllibAppImageParseParams_t for
 *                            details.
 *
 * \return  status            Whether image is parsed and copied correctly
 *                            STW_SOK   : Success
 *                            STW_EFAIL : Failure
 */
int32_t SBLLibMultiCoreImageParseV2(sbllibAppImageParseParams_t *imageParams);

/**
 * \brief   This function does the default initialization of App Image Parse
 *          Parameters.
 *
 * \param   imageParams       Multi-core Application Image Parse Parameters.
 *                            Refer struct #sbllibAppImageParseParams_t for
 *                            details.
 *
 * \return  None
 */
void SBLLibAppImageParamsInit(sbllibAppImageParseParams_t *imageParams);

/**
 * \brief   This function parses the RPRC executable image and copies
 *          the individual section into destination location.
 *
 * \param   rprcHeader      Header of RPRC Image
 *                          Refer struct #sbllibRPRCImageHeader_t for details.
 * \param   rprcImageAddr   Address of RPRC Image
 * \param   entryPoint      CPU entry point location
 * \param   coreId          CPU ID to identify Core.
 *                          Refer enum #sbllibCoreId_t for values.
 * \param   isImageInDDR    Set to 1 if RPRC image is already in DDR.
 *                          This will enforce use of gMediaReadFxn instead of
 *                          gDDRReadFxn
 *
 * \return  status         Whether image is parsed and copied correctly
 *                         STW_SOK   : Success
 *                         STW_EFAIL : Failure
 *
 * \note    Valid CPU IDs depend on slave cores present in SoC. When this API
 *          is called current RPRC read address is equal to RPRC image start
 *          address.
 */
int32_t SBLLibRprcImageParse(sbllibRPRCImageHeader_t *rprcHeader,
                             const void              *rprcImageAddr,
                             uint32_t                *entryPoint,
                             uint32_t                 coreId,
                             uint32_t                 isImageInDDR);

/**
 * \brief   This function stores the CPU entry location in global structure.
 *
 * \param   entryPoint    CPU Entry Location
 * \param   coreId        CPU Id for which entry point is set.
 *                        Refer enum #sbllibCoreId_t for values.
 * \param   entryPoints   Structure to set entry points for different
 *                        cores. Refer struct #sbllibEntryPoints_t for
 *                        details.
 *
 * \return  status        Whether entry point is assigned successfully
 *                        STW_SOK   : Success
 *                        STW_EFAIL : Failure
 *
 * \note    Valid CPU IDs depend on slave cores present in SoC.
 */
int32_t SBLLibBootCore(uint32_t             entryPoint,
                       uint32_t             coreId,
                       sbllibEntryPoints_t *entryPoints);

/**
 * \brief   This function provides the device Id of SoC as defined in SBL Lib
 *
 * \return  deviceId      Device Id of SoC
 *
 * \note    This can be different from the actual device Id of SoC.
 */
int32_t SBLLibGetDeviceId(void);

/**
 * \brief   This function registers the image copy callback functions which
 *          are used while parsing and copying multi-core App image.
 *
 * \param   mediaReadFxn Function for reading data block from boot media
 * \param   ddrReadFxn   Function for reading data block from DDR
 * \param   seekFxn      Function for moving read head
 *
 * \return  status       Whether call back functions are assigned correctly
 *                       STW_SOK   : Success
 *                       STW_EFAIL : Failure
 */
int32_t SBLLibRegisterImageCopyCallback(SBLLibMediaReadFxn mediaReadFxn,
                                        SBLLibDDRReadFxn   ddrReadFxn,
                                        SBLLibSeekFxn      seekFxn);

/**
 * \brief   This API initializes the SBL Lib.
 *
 * \param   sbllibInitPrms         SBL Lib Initialization parameters.
 *
 * \return  None.
 */
void SBLLibInit(const sbllibInitParams_t *sbllibInitPrms);

/**
 * \brief   This function prints the message depending on Trace Level. It uses
 *          print function registered while doing SBL Lib Initialization.
 *
 * \param   traceLevel   UART trace level.
 *                       Refer enum #sbllibTraceLevel_t for details.
 * \param   message      Message to be printed on UART console
 *
 * \return  None
 */
void SBLLibPrintf(uint32_t traceLevel, const char *message);

/**
 * \brief   This function prints hex.
 *
 * \param   traceLevel  UART trace level.
 * \param   value       Hex value to be printed
 *
 * \return  None
 */
void SBLLibPrintHexNum(uint32_t traceLevel, uint32_t value);

/**
 * \brief   This function prints a character.
 *
 * \param   traceLevel  UART trace level.
 * \param   value       character to be printed
 *
 * \return  None
 */
void SBLLibPutc(uint32_t traceLevel, uint8_t value);


/**
 * \brief   This function return a character.
 *
 * \return  char from console
 */
char SBLLibGetc(void);

/**
 * \brief   This function returns a number from console.
 *
 * \return  Number entered by user
 */
uint32_t SBLLibGetNum(void);

/**
 * \brief   This function returns a hex number from console.
 *
 * \return  hex Number entered by user
 */
uint32_t SBLLibGetHexNum(void);

/**
 * \brief   This function gets the DPLL structure for configuring a particular
 *          DPLL depending on the SysClk and OPP.
 *
 * \param   dpllId                DPLL instance for which structure should be
 *                                fetched. Refer enum pmhalPrcmNodeId_t.
 * \param   sysClk                System Clock Frequency.
 *                                Refer enum pmhalPrcmSysClkVal_t for values.
 * \param   opp                   OPP that should be set. Refer
 *                                enum #sbllibPrcmDpllOpp_t for details.
 * \param   dpllCfg               Pointer to DPLL structure pmhalPrcmDpllConfig_t.
 *
 * \return  status           Whether DPLL structure is fetched correctly
 *                           STW_SOK   : Success
 *                           STW_EFAIL : Failure
 *
 * \note    For tda3xx, this API supports only 20 MHz SYSCLK & Opp NOM.
 */
int32_t SBLLibGetDpllStructure(uint32_t                dpllId,
                               uint32_t                sysClk,
                               uint32_t                opp,
                               pmhalPrcmDpllConfig_t **dpllCfg);

/**
 * \brief   This function is called to abort SBL boot.
 *
 * \return  None.
 *
 * \note    This API is essentially a while(1) loop.
 */
void SBLLibAbortBoot(void);

/**
 * \brief   This function converts the Hexadecimal value to string value.
 *
 * \param   hexValue       Hexadecimal Value.
 * \param   stringValue    String Value.
 *
 * \return  None
 *
 * \note    This API supports conversions up to 32 bit values.
 */
void SBLLibConvertHexToString(uint32_t hexValue, char *stringValue);

/**
 * \brief   This function returns the size of RPRC Image including headers.
 *
 * \param   rprcHeader      Header of RPRC Image
 *                          Refer struct #sbllibRPRCImageHeader_t for details.
 * \param   rprcImageAddr   Start address of RPRC image.
 *
 * \return  rprcImageSize   Size of RPRC image
 */
uint32_t SBLLibCalculateRprcImageSize(sbllibRPRCImageHeader_t *rprcHeader,
                                      uint32_t                 rprcImageAddr);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline void SBLLibInitParamsInit(sbllibInitParams_t *sbllibInitPrms)
{
    if (NULL != sbllibInitPrms)
    {
        sbllibInitPrms->printFxn       = NULL;
        sbllibInitPrms->printHexNumFxn = NULL;
        sbllibInitPrms->getcFxn        = NULL;
        sbllibInitPrms->putcFxn        = NULL;
        sbllibInitPrms->getNumFxn      = NULL;
        sbllibInitPrms->getHexNumFxn   = NULL;
    }
}

#ifdef __cplusplus
}

#endif

#endif
/* @} */
/**
 * \mainpage  Secondary Boot Loader
 *
 * \par IMPORTANT NOTE
 *   <b>
 *   The interfaces defined in this package are bound to change.
 *   Release notes/user guide list the additional limitation/restriction
 *   of this module/interfaces.
 *   </b> See also \ref TI_DISCLAIMER.
 *
 *
 * Secondary Boot Loader Library(SBL Lib) contains the interfaces that are
 * generic and can be used by any higher level application including SBL
 * application. Secondary Boot Loader Utility Library (SBL Utils) contains
 * the interfaces that are used by SBL application for communicating with
 * various IO peripherals and boot media. SBL application contains the
 * bootloader application that initializes the SoC, loads the application image
 * and boots the slave cores.
 *
 * <b>
 * Also refer to top level user guide for detailed features,
 * limitations and usage description.
 * </b>
 *
 * - <b> SBL Library
 * </b> <br>
 *  SBL Lib contains the SBL Library layer for all platforms.
 *  SBL Library can be further divided into following:
 *
 *  - <b> SBL Common Lib
 * </b> (See \ref SBL_LIB) <br>
 *  SBL Common Library provides interfaces that are common across different platforms.
 *
 *  - <b> SBL Board Lib
 * </b> (See \ref SBL_BOARD_LIB) <br>
 *  SBL Board Library provides the board related interfaces like pin mux APIs, etc.
 *
 *  - <b> SBL Config Lib
 * </b> (See \ref SBL_CONFIG_LIB) <br>
 *  SBL Library Config contains compile time configurable parameters of TDAxxx SBL.
 *  This can be used to port SBL to custom use case.
 *
 *  - <b> SBL TDA2xx Lib
 * </b> (See \ref SBL_TDA2XX_LIB) <br>
 *  SBL TDA2xx Lib contains the interfaces present in the Secondary Bootloader(SBL)
 *  Library valid for TDA2xx SOC family (TDA2xx and TDA2Ex platform).
 *
 *  - <b> SBL TDA3xx Lib
 * </b> (See \ref SBL_TDA3XX_LIB) <br>
 *  SBL TDA3xx Lib contains the interfaces present in the Secondary Bootloader(SBL)
 *  Library valid for TDA3xx platform.
 *
 *
 * - <b> SBL Utility Library
 * </b> <br>
 *  SBL Utils contains the SBL Utility layer interfaces for all platforms.
 *  SBL Utils can be further divided into following:
 *
 *  - <b> SBL Common Utils
 * </b> (See \ref SBL_COMMON_UTILS) <br>
 *  SBL Common Utils contains the SBL Utility layer common for all platforms.
 *
 *  - <b> SBL DDR Utils
 * </b> (See \ref SBL_DDR_UTILS) <br>
 *  SBL DDR Utils contains the SBL Utility layer specific to TDAxx DDR
 *  configuration.
 *
 *  - <b> SBL TDA2xx Utils
 * </b> (See \ref SBL_TDA2XX_UTILS) <br>
 *  SBL TDA2xx Utils contains the SBL Utility layer specific to TDA2xx SOC
 *  family.
 *
 *  - <b> SBL TDA3xx Utils
 * </b> (See \ref SBL_TDA3XX_UTILS) <br>
 *  SBL TDA3xx Utils contains the SBL Utility layer specific to TDA3xx Platform.
 *
 *  - <b> SBL I2C Utils
 * </b> (See \ref SBL_TDA3XX_I2C_UTILS) <br>
 *  SBL I2C Utils contains the SBL Utility layer specific to TDA3xx platform
 *  needed for I2C communication.
 *
 *
 * - <b> NOR Flash Library
 * </b> <br>
 *  - <b> Nor Flash Lib
 * </b> (See \ref NOR_FLASH) <br>
 *  Nor Flash Library contains the interfaces for accessing the NOR Flash.
 *
 * - <b> QSPI Flash Library
 * </b> <br>
 *  - <b> QSPI Flash Lib
 * </b> (See \ref QSPI_FLASH) <br>
 *  QSPI Flash Library contains the interfaces for accessing the Flash using QSPI
 */

/**
 *  \page  TI_DISCLAIMER  TI Disclaimer
 *
 *  \htmlinclude ti_disclaim.htm
 */
