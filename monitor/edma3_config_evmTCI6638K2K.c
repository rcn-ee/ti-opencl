/*
 * (C) Copyright 2013-2014, Texas Instruments Incorporated.
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
 */

#include <xdc/std.h>
#include <ti/sdo/edma3/rm/edma3_rm.h>
#include <ti/sdo/fc/edma3/edma3_config.h>

#define EDMA_MGR_NUM_EDMA_INSTANCES 5

/* Definition of EDMA3_InstanceInitConfig structure from                    */
/* <ti/sdo/fc/edma3/edma3_config.h>. It is attached here and put under      */
/* #if 0 for reference                                                      */

/* EDMA3_InstanceInitConfig is init-time Region Specific Configuration      */
/* structure for EDMA3 to provide region specific Information. It is used   */
/* to specify which EDMA3 resources are owned and reserved by the EDMA3     */
/* instance.                                                                */

#if 0
typedef struct
{
    unsigned int        ownPaRAMSets[EDMA3_MAX_PARAM_DWRDS];
                            /**< PaRAM Sets owned by the EDMA3 RM Instance. */
    unsigned int        ownDmaChannels[EDMA3_MAX_DMA_CHAN_DWRDS];
                            /**< DMA Channels owned by the EDMA3 RM Instance. */
    unsigned int        ownQdmaChannels[EDMA3_MAX_QDMA_CHAN_DWRDS];
                            /**< QDMA Channels owned by the EDMA3 RM Instance.*/
    unsigned int        ownTccs[EDMA3_MAX_TCC_DWRDS];
                            /**< TCCs owned by the EDMA3 RM Instance. */
    /**
     * @brief       Reserved PaRAM Sets
     */
    unsigned int        resvdPaRAMSets[EDMA3_MAX_PARAM_DWRDS];
    /**
     * @brief       Reserved DMA channels
     */
    unsigned int        resvdDmaChannels[EDMA3_MAX_DMA_CHAN_DWRDS];
    /**
     * @brief       Reserved QDMA channels
     */
    unsigned int        resvdQdmaChannels[EDMA3_MAX_QDMA_CHAN_DWRDS];
    /**
     * @brief       Reserved TCCs
     */
    unsigned int        resvdTccs[EDMA3_MAX_TCC_DWRDS];
} EDMA3_InstanceInitConfig;
#endif

/* In the arrays below, each bit of a 32-bit word corresponds to a single   */
/* PaRAMSet/EDMAChannel/QDMAChannel/TCC owned by the corresponding region,  */
/* i.e., can be used for general purpose EDMA tranfers, or reserved for     */
/* EDMA transfers by hardware peripherals (cannot be used for general       */
/* purpose EDMA tranfers)                                                   */

#define DMA_CHANNEL_TO_EVENT_MAPPING_0          (0x00000000u)
#define DMA_CHANNEL_TO_EVENT_MAPPING_1          (0x00000000u)

/* EDMA3_InstanceInitConfig sample0 with region neither owning nor          */
/* reserving any EDMA resources                                             */
#define regionSample0                                         \
{                                                             \
    /* Resources owned by Region */                           \
    /* ownPaRAMSets */                                        \
    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u},     \
                                                              \
    /* ownDmaChannels */                                      \
    {0x00000000u, 0x00000000u},                               \
                                                              \
    /* ownQdmaChannels */                                     \
    {0x00000000u},                                            \
                                                              \
    /* ownTccs */                                             \
    {0x00000000u, 0x00000000u},                               \
                                                              \
    /* Resources reserved by Region */                        \
    /* resvdPaRAMSets */                                      \
    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u},     \
                                                              \
    /* resvdDmaChannels */                                    \
    {DMA_CHANNEL_TO_EVENT_MAPPING_0, DMA_CHANNEL_TO_EVENT_MAPPING_1}, \
                                                              \
    /* resvdQdmaChannels */                                   \
    {0x00000000u},                                            \
                                                              \
    /* resvdTccs */                                           \
    {DMA_CHANNEL_TO_EVENT_MAPPING_0, DMA_CHANNEL_TO_EVENT_MAPPING_1} \
}

/*-----------------------------------------------------------------------------
* Note that the first N PaRAM sets (N=number of EDMA channels available 
* on an EDMA instance) are reserved in EDMA3 LLD ).
*----------------------------------------------------------------------------*/
#define regionSample1                                         \
{                                                             \
    /* Resources owned by Region */                           \
    /* ownPaRAMSets */                                        \
    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u},     \
                                                              \
    /* ownDmaChannels */                                      \
    {0xFFFFFFFFu, 0x00000000u},                               \
                                                              \
    /* ownQdmaChannels */                                     \
    {0x00000000u},                                            \
                                                              \
    /* ownTccs */                                             \
    {0xFFFFFFFFu, 0x00000000u},                               \
                                                              \
    /* Resources reserved by Region */                        \
    /* resvdPaRAMSets */                                      \
    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u},     \
                                                              \
    /* resvdDmaChannels */                                    \
    {DMA_CHANNEL_TO_EVENT_MAPPING_0, DMA_CHANNEL_TO_EVENT_MAPPING_1}, \
                                                              \
    /* resvdQdmaChannels */                                   \
    {0x00000000u},                                            \
                                                              \
    /* resvdTccs */                                           \
    {DMA_CHANNEL_TO_EVENT_MAPPING_0, DMA_CHANNEL_TO_EVENT_MAPPING_1} \
}


#define regionSample2                                         \
{                                                             \
    /* Resources owned by Region */                           \
    /* ownPaRAMSets */                                        \
    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u},     \
                                                              \
    /* ownDmaChannels */                                      \
    {0x00000000u, 0xFFFFFFFFu},                               \
                                                              \
    /* ownQdmaChannels */                                     \
    {0x00000000u},                                            \
                                                              \
    /* ownTccs */                                             \
    {0x00000000u, 0xFFFFFFFFu},                               \
                                                              \
    /* Resources reserved by Region */                        \
    /* resvdPaRAMSets */                                      \
    {0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,      \
     0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u},     \
                                                              \
    /* resvdDmaChannels */                                    \
    {DMA_CHANNEL_TO_EVENT_MAPPING_0, DMA_CHANNEL_TO_EVENT_MAPPING_1}, \
                                                              \
    /* resvdQdmaChannels */                                   \
    {0x00000000u},                                            \
                                                              \
    /* resvdTccs */                                           \
    {DMA_CHANNEL_TO_EVENT_MAPPING_0, DMA_CHANNEL_TO_EVENT_MAPPING_1} \
}

EDMA3_InstanceInitConfig edmaMgrInstanceInitConfig[EDMA_MGR_NUM_EDMA_INSTANCES][EDMA3_MAX_REGIONS] =
{
 /* EDMA3 INSTANCE# 0 */
 { regionSample0,  regionSample0,  regionSample0,  regionSample0,
   regionSample0,  regionSample0,  regionSample0,  regionSample0
 },
 /* EDMA3 INSTANCE# 1 */
 { regionSample1,  regionSample2,  regionSample0,  regionSample0,
   regionSample0,  regionSample0,  regionSample0,  regionSample0
 },
 /* EDMA3 INSTANCE# 2 */
 { regionSample0,  regionSample0,  regionSample1,  regionSample2,
   regionSample0,  regionSample0,  regionSample0,  regionSample0
 },
 /* EDMA3 INSTANCE# 3 */
 { regionSample0,  regionSample0,  regionSample0,  regionSample0,
   regionSample1,  regionSample2,  regionSample0,  regionSample0
 },
 /* EDMA3 INSTANCE# 4 */
 { regionSample0,  regionSample0,  regionSample0,  regionSample0,
   regionSample0,  regionSample0,  regionSample1,  regionSample2
 }
};

int32_t edmaMgrRegion2Instance[EDMA3_MAX_REGIONS] = {1,1,2,2,3,3,4,4};

/* Driver Object Initialization Configuration */
EDMA3_GblConfigParams edmaMgrGblConfigParams [EDMA_MGR_NUM_EDMA_INSTANCES] =
	{
		{
		/* EDMA3 INSTANCE# 0 */
		/** Total number of DMA Channels supported by the EDMA3 Controller */
		64u,
		/** Total number of QDMA Channels supported by the EDMA3 Controller */
		8u,
		/** Total number of TCCs supported by the EDMA3 Controller */
		64u,
		/** Total number of PaRAM Sets supported by the EDMA3 Controller */
		512u,
		/** Total number of Event Queues in the EDMA3 Controller */
		2u,
		/** Total number of Transfer Controllers (TCs) in the EDMA3 Controller */
		2u,
		/** Number of Regions on this EDMA3 controller */
		8u,

		/**
		 * \brief Channel mapping existence
		 * A value of 0 (No channel mapping) implies that there is fixed association
		 * for a channel number to a parameter entry number or, in other words,
		 * PaRAM entry n corresponds to channel n.
		 */
		1u,

		/** Existence of memory protection feature */
		1u,

		/** Global Register Region of CC Registers */
		(void *)0x02700000u,
		/** Transfer Controller (TC) Registers */
		{
		(void *)0x02760000u,
		(void *)0x02768000u,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL
		},
		/** Interrupt no. for Transfer Completion */
		38u,
		/** Interrupt no. for CC Error */
		32u,
		/** Interrupt no. for TCs Error */
		{
		34u,
		35u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u,
		},

		/**
		 * \brief EDMA3 TC priority setting
		 *
		 * User can program the priority of the Event Queues
		 * at a system-wide level.  This means that the user can set the
		 * priority of an IO initiated by either of the TCs (Transfer Controllers)
		 * relative to IO initiated by the other bus masters on the
		 * device (ARM, DSP, USB, etc)
		 */
		{
		0u,
		1u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},
		/**
		 * \brief To Configure the Threshold level of number of events
		 * that can be queued up in the Event queues. EDMA3CC error register
		 * (CCERR) will indicate whether or not at any instant of time the
		 * number of events queued up in any of the event queues exceeds
		 * or equals the threshold/watermark value that is set
		 * in the queue watermark threshold register (QWMTHRA).
		 */
		{
		16u,
		16u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief To Configure the Default Burst Size (DBS) of TCs.
		 * An optimally-sized command is defined by the transfer controller
		 * default burst size (DBS). Different TCs can have different
		 * DBS values. It is defined in Bytes.
		 */
		{
		256u,
		256u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief Mapping from each DMA channel to a Parameter RAM set,
		 * if it exists, otherwise of no use.
		 */
		{
		EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP
		},

		 /**
		  * \brief Mapping from each DMA channel to a TCC. This specific
		  * TCC code will be returned when the transfer is completed
		  * on the mapped channel.
		  */
		{
		0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
		8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u,
		16u, 17u, 18u, 19u, 20u, 21u, 22u, 23u,
		24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP
		},

		/**
		 * \brief Mapping of DMA channels to Hardware Events from
		 * various peripherals, which use EDMA for data transfer.
		 * All channels need not be mapped, some can be free also.
		 */
		{
		0xFFFFFFFFu,
		0x00000000u
		}
		},

		{
		/* EDMA3 INSTANCE# 1 */
		/** Total number of DMA Channels supported by the EDMA3 Controller */
		64u,
		/** Total number of QDMA Channels supported by the EDMA3 Controller */
		8u,
		/** Total number of TCCs supported by the EDMA3 Controller */
		64u,
		/** Total number of PaRAM Sets supported by the EDMA3 Controller */
		512u,
		/** Total number of Event Queues in the EDMA3 Controller */
		4u,
		/** Total number of Transfer Controllers (TCs) in the EDMA3 Controller */
		4u,
		/** Number of Regions on this EDMA3 controller */
		8u,

		/**
		 * \brief Channel mapping existence
		 * A value of 0 (No channel mapping) implies that there is fixed association
		 * for a channel number to a parameter entry number or, in other words,
		 * PaRAM entry n corresponds to channel n.
		 */
		1u,

		/** Existence of memory protection feature */
		1u,

		/** Global Register Region of CC Registers */
		(void *)0x02720000u,
		/** Transfer Controller (TC) Registers */
		{
		(void *)0x02770000u,
		(void *)0x02778000u,
		(void *)0x02780000u,
		(void *)0x02788000u,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL
		},
		/** Interrupt no. for Transfer Completion */
		8u,
		/** Interrupt no. for CC Error */
		0u,
		/** Interrupt no. for TCs Error */
		{
		2u,
		3u,
		4u,
		5u,
		0u,
		0u,
		0u,
		0u,
		},

		/**
		 * \brief EDMA3 TC priority setting
		 *
		 * User can program the priority of the Event Queues
		 * at a system-wide level.  This means that the user can set the
		 * priority of an IO initiated by either of the TCs (Transfer Controllers)
		 * relative to IO initiated by the other bus masters on the
		 * device (ARM, DSP, USB, etc)
		 */
		{
		0u,
		1u,
		2u,
		3u,
		0u,
		0u,
		0u,
		0u
		},
		/**
		 * \brief To Configure the Threshold level of number of events
		 * that can be queued up in the Event queues. EDMA3CC error register
		 * (CCERR) will indicate whether or not at any instant of time the
		 * number of events queued up in any of the event queues exceeds
		 * or equals the threshold/watermark value that is set
		 * in the queue watermark threshold register (QWMTHRA).
		 */
		{
		16u,
		16u,
		16u,
		16u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief To Configure the Default Burst Size (DBS) of TCs.
		 * An optimally-sized command is defined by the transfer controller
		 * default burst size (DBS). Different TCs can have different
		 * DBS values. It is defined in Bytes.
		 */
		{
		128u,
		128u,
		128u,
		128u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief Mapping from each DMA channel to a Parameter RAM set,
		 * if it exists, otherwise of no use.
		 */
		{
		EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP
		},

		 /**
		  * \brief Mapping from each DMA channel to a TCC. This specific
		  * TCC code will be returned when the transfer is completed
		  * on the mapped channel.
		  */
		{
		0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
		8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u,
		16u, 17u, 18u, 19u, 20u, 21u, 22u, 23u,
		24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP
		},

		/**
		 * \brief Mapping of DMA channels to Hardware Events from
		 * various peripherals, which use EDMA for data transfer.
		 * All channels need not be mapped, some can be free also.
		 */
		{
		0xFFFFFFFFu,
		0x00000000u
		}
		},

		{
		/* EDMA3 INSTANCE# 2 */
		/** Total number of DMA Channels supported by the EDMA3 Controller */
		64u,
		/** Total number of QDMA Channels supported by the EDMA3 Controller */
		8u,
		/** Total number of TCCs supported by the EDMA3 Controller */
		64u,
		/** Total number of PaRAM Sets supported by the EDMA3 Controller */
		512u,
		/** Total number of Event Queues in the EDMA3 Controller */
		4u,
		/** Total number of Transfer Controllers (TCs) in the EDMA3 Controller */
		4u,
		/** Number of Regions on this EDMA3 controller */
		8u,

		/**
		 * \brief Channel mapping existence
		 * A value of 0 (No channel mapping) implies that there is fixed association
		 * for a channel number to a parameter entry number or, in other words,
		 * PaRAM entry n corresponds to channel n.
		 */
		1u,

		/** Existence of memory protection feature */
		1u,

		/** Global Register Region of CC Registers */
		(void *)0x02740000u,
		/** Transfer Controller (TC) Registers */
		{
		(void *)0x02790000u,
		(void *)0x02798000u,
		(void *)0x027A0000u,
		(void *)0x027A8000u,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL
		},
		/** Interrupt no. for Transfer Completion */
		24u,
		/** Interrupt no. for CC Error */
		16u,
		/** Interrupt no. for TCs Error */
		{
		18u,
		19u,
		20u,
		21u,
		0u,
		0u,
		0u,
		0u,
		},

		/**
		 * \brief EDMA3 TC priority setting
		 *
		 * User can program the priority of the Event Queues
		 * at a system-wide level.  This means that the user can set the
		 * priority of an IO initiated by either of the TCs (Transfer Controllers)
		 * relative to IO initiated by the other bus masters on the
		 * device (ARM, DSP, USB, etc)
		 */
		{
		0u,
		1u,
		2u,
		3u,
		0u,
		0u,
		0u,
		0u
		},
		/**
		 * \brief To Configure the Threshold level of number of events
		 * that can be queued up in the Event queues. EDMA3CC error register
		 * (CCERR) will indicate whether or not at any instant of time the
		 * number of events queued up in any of the event queues exceeds
		 * or equals the threshold/watermark value that is set
		 * in the queue watermark threshold register (QWMTHRA).
		 */
		{
		16u,
		16u,
		16u,
		16u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief To Configure the Default Burst Size (DBS) of TCs.
		 * An optimally-sized command is defined by the transfer controller
		 * default burst size (DBS). Different TCs can have different
		 * DBS values. It is defined in Bytes.
		 */
		{
		128u,
		128u,
		128u,
		128u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief Mapping from each DMA channel to a Parameter RAM set,
		 * if it exists, otherwise of no use.
		 */
		{
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP
		},

		 /**
		  * \brief Mapping from each DMA channel to a TCC. This specific
		  * TCC code will be returned when the transfer is completed
		  * on the mapped channel.
		  */
		{
		0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
		8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u,
		16u, 17u, 18u, 19u, 20u, 21u, 22u, 23u,
		24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP
		},

		/**
		 * \brief Mapping of DMA channels to Hardware Events from
		 * various peripherals, which use EDMA for data transfer.
		 * All channels need not be mapped, some can be free also.
		 */
		{
		0xFFFFFFFFu,
		0x00000000u
		}
		},

		{
		/* EDMA3 INSTANCE# 3 */
		/** Total number of DMA Channels supported by the EDMA3 Controller */
		64u,
		/** Total number of QDMA Channels supported by the EDMA3 Controller */
		8u,
		/** Total number of TCCs supported by the EDMA3 Controller */
		64u,
		/** Total number of PaRAM Sets supported by the EDMA3 Controller */
		512u,
		/** Total number of Event Queues in the EDMA3 Controller */
		2u,
		/** Total number of Transfer Controllers (TCs) in the EDMA3 Controller */
		2u,
		/** Number of Regions on this EDMA3 controller */
		8u,

		/**
		 * \brief Channel mapping existence
		 * A value of 0 (No channel mapping) implies that there is fixed association
		 * for a channel number to a parameter entry number or, in other words,
		 * PaRAM entry n corresponds to channel n.a
		 */
		1u,

		/** Existence of memory protection feature */
		1u,

		/** Global Register Region of CC Registers */
		(void *)0x02728000u,
		/** Transfer Controller (TC) Registers */
		{
		(void *)0x027B0000u,
		(void *)0x027B8000u,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL
		},
		/** Interrupt no. for Transfer Completion */
		225u,
		/** Interrupt no. for CC Error */
		220u,
		/** Interrupt no. for TCs Error */
		{
		222u,
		223u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u,
		},

		/**
		 * \brief EDMA3 TC priority setting
		 *
		 * User can program the priority of the Event Queues
		 * at a system-wide level.  This means that the user can set the
		 * priority of an IO initiated by either of the TCs (Transfer Controllers)
		 * relative to IO initiated by the other bus masters on the
		 * device (ARM, DSP, USB, etc)
		 */
		{
		0u,
		1u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},
		/**
		 * \brief To Configure the Threshold level of number of events
		 * that can be queued up in the Event queues. EDMA3CC error register
		 * (CCERR) will indicate whether or not at any instant of time the
		 * number of events queued up in any of the event queues exceeds
		 * or equals the threshold/watermark value that is set
		 * in the queue watermark threshold register (QWMTHRA).
		 */
		{
		16u,
		16u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief To Configure the Default Burst Size (DBS) of TCs.
		 * An optimally-sized command is defined by the transfer controller
		 * default burst size (DBS). Different TCs can have different
		 * DBS values. It is defined in Bytes.
		 */
		{
		128u,
		128u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief Mapping from each DMA channel to a Parameter RAM set,
		 * if it exists, otherwise of no use.
		 */
		{
		EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP
		},

		 /**
		  * \brief Mapping from each DMA channel to a TCC. This specific
		  * TCC code will be returned when the transfer is completed
		  * on the mapped channel.
		  */
		{
		0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
		8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u,
		16u, 17u, 18u, 19u, 20u, 21u, 22u, 23u,
		24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP
		},

		/**
		 * \brief Mapping of DMA channels to Hardware Events from
		 * various peripherals, which use EDMA for data transfer.
		 * All channels need not be mapped, some can be free also.
		 */
		{
		0xFFFFFFFFu,
		0x00000000u
		}
		},

		{
		/* EDMA3 INSTANCE# 4 */
		/** Total number of DMA Channels supported by the EDMA3 Controller */
		64u,
		/** Total number of QDMA Channels supported by the EDMA3 Controller */
		8u,
		/** Total number of TCCs supported by the EDMA3 Controller */
		64u,
		/** Total number of PaRAM Sets supported by the EDMA3 Controller */
		512u,
		/** Total number of Event Queues in the EDMA3 Controller */
		2u,
		/** Total number of Transfer Controllers (TCs) in the EDMA3 Controller */
		2u,
		/** Number of Regions on this EDMA3 controller */
		8u,

		/**
		 * \brief Channel mapping existence
		 * A value of 0 (No channel mapping) implies that there is fixed association
		 * for a channel number to a parameter entry number or, in other words,
		 * PaRAM entry n corresponds to channel n.
		 */
		1u,

		/** Existence of memory protection feature */
		1u,

		/** Global Register Region of CC Registers */
		(void *)0x02708000u,
		/** Transfer Controller (TC) Registers */
		{
		(void *)0x027B8400u,
		(void *)0x027B8800u,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL,
		(void *)NULL
		},
		/** Interrupt no. for Transfer Completion */
		212u,
		/** Interrupt no. for CC Error */
		207u,
		/** Interrupt no. for TCs Error */
		{
		209u,
		210u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u,
		},

		/**
		 * \brief EDMA3 TC priority setting
		 *
		 * User can program the priority of the Event Queues
		 * at a system-wide level.  This means that the user can set the
		 * priority of an IO initiated by either of the TCs (Transfer Controllers)
		 * relative to IO initiated by the other bus masters on the
		 * device (ARM, DSP, USB, etc)
		 */
		{
		0u,
		1u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},
		/**
		 * \brief To Configure the Threshold level of number of events
		 * that can be queued up in the Event queues. EDMA3CC error register
		 * (CCERR) will indicate whether or not at any instant of time the
		 * number of events queued up in any of the event queues exceeds
		 * or equals the threshold/watermark value that is set
		 * in the queue watermark threshold register (QWMTHRA).
		 */
		{
		16u,
		16u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief To Configure the Default Burst Size (DBS) of TCs.
		 * An optimally-sized command is defined by the transfer controller
		 * default burst size (DBS). Different TCs can have different
		 * DBS values. It is defined in Bytes.
		 */
		{
		256u,
		256u,
		0u,
		0u,
		0u,
		0u,
		0u,
		0u
		},

		/**
		 * \brief Mapping from each DMA channel to a Parameter RAM set,
		 * if it exists, otherwise of no use.
		 */
		{
		EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP,
        EDMA3_RM_CH_NO_PARAM_MAP, EDMA3_RM_CH_NO_PARAM_MAP
		},

		 /**
		  * \brief Mapping from each DMA channel to a TCC. This specific
		  * TCC code will be returned when the transfer is completed
		  * on the mapped channel.
		  */
		{
		0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
		8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u,
		16u, 17u, 18u, 19u, 20u, 21u, 22u, 23u,
		24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP,
		EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP, EDMA3_RM_CH_NO_TCC_MAP
		},

		/**
		 * \brief Mapping of DMA channels to Hardware Events from
		 * various peripherals, which use EDMA for data transfer.
		 * All channels need not be mapped, some can be free also.
		 */
		{
		0xFFFFFFFFu,
		0x00000000u
		}
		},
	};



int32_t   *ti_sdo_fc_edmamgr_region2Instance = &edmaMgrRegion2Instance[0];
EDMA3_GblConfigParams    *ti_sdo_fc_edmamgr_edma3GblConfigParams = &edmaMgrGblConfigParams[0];
EDMA3_InstanceInitConfig *ti_sdo_fc_edmamgr_edma3RegionConfig    = &edmaMgrInstanceInitConfig[0][0];
