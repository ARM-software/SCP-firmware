/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "system_mmap.h"

#include <mod_dmc500.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define COL_BITS 1
#define BANK_BITS 0
#define RANK_BITS 1
#define BANK_GROUP 0
#define ROW_BITS 5
#define MEM_TYPE 3
#define MEM_BURST 2
#define DEVICE_WIDTH 2
#define ADDR_SHUTTER 2

static const struct mod_dmc500_reg reg_val = {
    .ADDRESS_CONTROL                          = ADDRESS_CONTROL_VAL(RANK_BITS,
                                                                    BANK_BITS,
                                                                    ROW_BITS,
                                                                    COL_BITS),
    .RANK_REMAP_CONTROL                       = 0x00000000,
    .MEMORY_TYPE                              = MEMORY_TYPE_VAL(BANK_GROUP,
                                                                DEVICE_WIDTH,
                                                                MEM_TYPE),
    .FORMAT_CONTROL                           = FORMAT_CONTROL_VAL(MEM_BURST),
    .DECODE_CONTROL                           = 0x00000011,
    .FEATURE_CONTROL                          = 0x00000000,
    .ODT_WR_CONTROL_31_00                     = 0x00000000,
    .ODT_RD_CONTROL_31_00                     = 0x00000000,
    .ODT_TIMING                               = 0x10001000,
    .T_REFI                                   = 0x0000030b,
    .T_RFC                                    = 0x000340d0,
    .T_RDPDEN                                 = 0x0000002e,
    .T_RCD                                    = 0x0000001d,
    .T_RAS                                    = 0x80000044,
    .T_RP                                     = 0x0000221d,
    .T_RRD                                    = 0x00001010,
    .T_ACT_WINDOW                             = 0x00000040,
    .T_RTR                                    = 0x000c0808,
    .T_RTW                                    = 0x001f1f1f,
    .T_RTP                                    = 0x0000000C,
    .T_WR                                     = 0x00000035,
    .T_WTR                                    = 0x00082929,
    .T_WTW                                    = 0x000b0808,
    .T_XTMW                                   = 0x00000020,
    .T_CLOCK_CONTROL                          = 0x1119030d,
    .T_EP                                     = 0x0000000C,
    .T_XP                                     = 0x000c000c,
    .T_ESR                                    = 0x00000019,
    .T_XSR                                    = 0x00e100e1,
    .ADDRESS_MAP                              = ADDRESS_MAP_VAL(ADDR_SHUTTER),
    .SI0_SI_INTERRUPT_CONTROL                 = 0x00000000,
    .SI0_PMU_REQ_CONTROL                      = 0x00000B1A,
    .SI0_PMU_REQ_ATTRIBUTE_MASK_0             = 0xB0562AA1,
    .SI0_PMU_REQ_ATTRIBUTE_MATCH_0            = 0xD0FB6716,
    .SI0_PMU_REQ_ATTRIBUTE_MASK_1             = 0x7FC24C15,
    .SI0_PMU_REQ_ATTRIBUTE_MATCH_1            = 0xF7A9B2AC,
    .SI0_PMU_REQ_ATTRIBUTE_MASK_2             = 0xDD35FA69,
    .SI0_PMU_REQ_ATTRIBUTE_MATCH_2            = 0x3555A8F5,
    .SI0_PMU_REQ_ATTRIBUTE_MASK_3             = 0xDE382B10,
    .SI0_PMU_REQ_ATTRIBUTE_MATCH_3            = 0x3484B32C,
    .SI0_THRESHOLD_CONTROL                    = 0x00000000,
    .SI1_SI_INTERRUPT_CONTROL                 = 0x00000000,
    .SI1_PMU_REQ_CONTROL                      = 0x00000B1A,
    .SI1_PMU_REQ_ATTRIBUTE_MASK_0             = 0xB0562AA1,
    .SI1_PMU_REQ_ATTRIBUTE_MATCH_0            = 0xD0FB6716,
    .SI1_PMU_REQ_ATTRIBUTE_MASK_1             = 0x7FC24C15,
    .SI1_PMU_REQ_ATTRIBUTE_MATCH_1            = 0xF7A9B2AC,
    .SI1_PMU_REQ_ATTRIBUTE_MASK_2             = 0xDD35FA69,
    .SI1_PMU_REQ_ATTRIBUTE_MATCH_2            = 0x3555A8F5,
    .SI1_PMU_REQ_ATTRIBUTE_MASK_3             = 0xDE382B10,
    .SI1_PMU_REQ_ATTRIBUTE_MATCH_3            = 0x3484B32C,
    .SI1_THRESHOLD_CONTROL                    = 0x00000000,
    .QUEUE_THRESHOLD_CONTROL_31_00            = 0x00000000,
    .QUEUE_THRESHOLD_CONTROL_63_32            = 0x99887700,
    .DCB_INTERRUPT_CONTROL                    = 0x00000000,
    .PMU_DCB_CONTROL                          = 0x00000800,
    .PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_0  = 0xFD98CF7D,
    .PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_0 = 0x9F276EB5,
    .PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_1  = 0x40B1FC24,
    .PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_1 = 0x04BBF4FA,
    .PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_2  = 0x8089B0AF,
    .PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_2 = 0x7D26E0BE,
    .PMU_TAG_ENTRIES_ATTRIBUTE_MASK           = 0x000000CE,
    .PMU_TAG_ENTRIES_ATTRIBUTE_MATCH          = 0x00000056,
    .QE_INTERRUPT_CONTROL                     = 0x00000000,
    .RANK_TURNAROUND_CONTROL                  = 0x0F0F0F0F,
    .HIT_TURNAROUND_CONTROL                   = 0x8290BF8F,
    .QOS_CLASS_CONTROL                        = 0x00000D50,
    .ESCALATION_CONTROL                       = 0x00000F03,
    .QV_CONTROL_31_00                         = 0xE6543210,
    .QV_CONTROL_63_32                         = 0xFEDCBA98,
    .RT_CONTROL_31_00                         = 0x00000000,
    .RT_CONTROL_63_32                         = 0x00008800,
    .TIMEOUT_CONTROL                          = 0x00000001,
    .WRITE_PRIORITY_CONTROL_31_00             = 0x00000000,
    .WRITE_PRIORITY_CONTROL_63_32             = 0xEC840000,
    .DIR_TURNAROUND_CONTROL                   = 0x05050F00,
    .HIT_PREDICTION_CONTROL                   = 0x00020705,
    .REFRESH_PRIORITY                         = 0x00000204,
    .MC_UPDATE_CONTROL                        = 0x0000ff00,
    .PHY_UPDATE_CONTROL                       = 0x15A3925F,
    .PHY_MASTER_CONTROL                       = 0x6875AF9A,
    .LOW_POWER_CONTROL                        = 0x000E0801,
    .PMU_QE_CONTROL                           = 0x00000C0D,
    .PMU_QE_MUX                               = 0x05670023,
    .PMU_QOS_ENGINE_ATTRIBUTE_MASK_0          = 0x000000F1,
    .PMU_QOS_ENGINE_ATTRIBUTE_MATCH_0         = 0x00000662,
    .PMU_QOS_ENGINE_ATTRIBUTE_MASK_1          = 0x000000DD,
    .PMU_QOS_ENGINE_ATTRIBUTE_MATCH_1         = 0x00000097,
    .PMU_QOS_ENGINE_ATTRIBUTE_MASK_2          = 0x0000001A,
    .PMU_QOS_ENGINE_ATTRIBUTE_MATCH_2         = 0x00000755,
    .PMU_QUEUED_ENTRIES_ATTRIBUTE_MASK        = 0xAD625ED5,
    .PMU_QUEUED_ENTRIES_ATTRIBUTE_MATCH       = 0x853C65BB,
    .MI_INTERRUPT_CONTROL                     = 0x00000000,
    .POWER_DOWN_CONTROL                       = 0x00000005,
    .REFRESH_CONTROL                          = 0x00000000,
    .PMU_MI_CONTROL                           = 0x00000100,
    .PMU_MEMORY_IF_ATTRIBUTE_MASK_0           = 0x0032BB0E,
    .PMU_MEMORY_IF_ATTRIBUTE_MATCH_0          = 0x0033F5AB,
    .PMU_MEMORY_IF_ATTRIBUTE_MASK_1           = 0x00296B28,
    .PMU_MEMORY_IF_ATTRIBUTE_MATCH_1          = 0x002C67BF,
    .PMU_BANK_STATES_ATTRIBUTE_MASK           = 0x00000005,
    .PMU_BANK_STATES_ATTRIBUTE_MATCH          = 0x00000019,
    .PMU_RANK_STATES_ATTRIBUTE_MASK           = 0x0000001B,
    .PMU_RANK_STATES_ATTRIBUTE_MATCH          = 0x00000020,
    .CFG_INTERRUPT_CONTROL                    = 0x00000000,
    .T_RDDATA_EN                              = 0x00000001,
    .T_PHYRDLAT                               = 0x0000003f,
    .T_PHYWRLAT                               = 0x010f170e,
    .ERR_RAMECC_CTLR                          = 0x00000000,
    .PHY_POWER_CONTROL                        = 0x0000012a,
    .T_PHY_TRAIN                              = 0x00f8000a,
    .PHYUPD_INIT                              = 0x00000000,
    .REFRESH_ENABLE                           = 0x00000001,
    .MI_STATE_CONTROL                         = 0,
    .QUEUE_STATE_CONTROL                      = 0,
    .SI0_SI_STATE_CONTROL                     = 0,
    .SI1_SI_STATE_CONTROL                     = 0,
};

/* Table of DMC500 elements descriptions. */
static const struct fwk_element dmc500_element_table[] = {
    [0] = { .name = "DMC500-0",
            .data = &((struct mod_dmc500_element_config) {
                .dmc = DMC_INTERNAL0,
                .ddr_phy_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DDR_PHY500, 0),
        }),
    },
    [1] = { .name = "DMC500-1",
            .data = &((struct mod_dmc500_element_config) {
                .dmc = DMC_INTERNAL1,
                .ddr_phy_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DDR_PHY500, 1),
        }),
    },
    [2] = { .name = "DMC500-2",
            .data = &((struct mod_dmc500_element_config) {
                .dmc = DMC_INTERNAL2,
                .ddr_phy_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DDR_PHY500, 2),
        }),
    },
    [3] = { .name = "DMC500-3",
            .data = &((struct mod_dmc500_element_config) {
                .dmc = DMC_INTERNAL3,
                .ddr_phy_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DDR_PHY500, 3),
        }),
    },
    [4] = { 0 }, /* Termination description. */
};

static const struct fwk_element *dmc500_get_element_table(fwk_id_t module_id)
{
    return dmc500_element_table;
}

static void direct_ddr_cmd(struct mod_dmc500_reg *dmc)
{
    dmc->DIRECT_CMD_SETTINGS  = 0x00C80000;
    dmc->DIRECT_CMD           = 0x00000000;
    dmc->DIRECT_CLK_DISABLE   = 0x00280003;
    dmc->CLK_STATUS_OVERRIDE  = 0x00000003;
    dmc->DIRECT_CMD_SETTINGS  = 0x01F40003;
    dmc->DIRECT_CMD           = 0x00800080;
    dmc->RANK_STATUS_OVERRIDE = 0x30000003;
    dmc->DIRECT_CMD_SETTINGS  = 0x04b00003;
    dmc->DIRECT_CMD           = 0x00800FE0;
    dmc->DIRECT_CMD_SETTINGS  = 0x00500003;
    dmc->DIRECT_CMD           = 0x008011E0;
    dmc->DIRECT_CMD_SETTINGS  = 0x00140003;
    dmc->DIRECT_CMD           = 0x06d601c6;
    dmc->DIRECT_CMD_SETTINGS  = 0x01000003;
    dmc->DIRECT_CMD           = 0x00f60dc6;
    dmc->DIRECT_CMD_SETTINGS  = 0x00140003;
    dmc->DIRECT_CMD           = 0x31d603c6;
    dmc->DIRECT_CMD_SETTINGS  = 0x00140003;
    dmc->DIRECT_CMD           = 0x16f601c6;
    dmc->DIRECT_CMD_SETTINGS  = 0x00140003;
    dmc->DIRECT_CMD           = 0x2dd602c6;
    dmc->DIRECT_CMD_SETTINGS  = 0x02000003;
    dmc->DIRECT_CMD           = 0x00d60de6;
}

/* Configuration of the DMC500 module. */
const struct fwk_module_config config_dmc500 = {
    .get_element_table = dmc500_get_element_table,
    .data = &((struct mod_dmc500_module_config) {
        .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
        .ddr_phy_module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_DDR_PHY500),
        .ddr_phy_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_DDR_PHY500, 0),
        .reg_val = &reg_val,
        .direct_ddr_cmd = direct_ddr_cmd,
    }),
};
