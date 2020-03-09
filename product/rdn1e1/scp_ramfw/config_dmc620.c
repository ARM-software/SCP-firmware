/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "scp_system_mmap.h"

#include <mod_dmc620.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define COL_BITS 2
#define BANK_BITS 4
#define RANK_BITS 1
#define ROW_BITS 4
#define BANK_HASH_ENABLE 1
#define MEM_TYPE 2
#define ADDR_DEC 0x68C
#define STRIPE_DEC 1
#define MEM_DEVICE_WIDTH 0
#define BANK_GROUP 3
#define MEM_CHANNEL 2
#define ADDRESS_CONTROL_NEXT_VAL ((BANK_HASH_ENABLE << 28) | \
                                 (RANK_BITS << 24) | (BANK_BITS << 16) | \
                                 (ROW_BITS << 8) | (COL_BITS))
#define DECODE_CONTROL_NEXT_VAL ((ADDR_DEC << 10) | (STRIPE_DEC << 4))
#define MEMORY_TYPE_NEXT_VAL ((BANK_GROUP << 16) | (MEM_DEVICE_WIDTH << 8) | \
                             (MEM_TYPE))

struct mod_dmc620_reg dmc_val = {
    .ADDRESS_CONTROL_NEXT = ADDRESS_CONTROL_NEXT_VAL,
    .DECODE_CONTROL_NEXT = DECODE_CONTROL_NEXT_VAL,
    .FORMAT_CONTROL = 0x00000003,
    .ADDRESS_MAP_NEXT = 0x00000002,
    .LOW_POWER_CONTROL_NEXT = 0x00000010,
    .TURNAROUND_CONTROL_NEXT = 0x1F0F0F0F,
    .HIT_TURNAROUND_CONTROL_NEXT = 0x08909FBF,
    .QOS_CLASS_CONTROL_NEXT = 0x00000FC8,
    .ESCALATION_CONTROL_NEXT = 0x00080F00,
    .QV_CONTROL_31_00_NEXT = 0x76543210,
    .QV_CONTROL_63_32_NEXT = 0xFEDCBA98,
    .RT_CONTROL_31_00_NEXT = 0x00000000,
    .RT_CONTROL_63_32_NEXT = 0x00000000,
    .TIMEOUT_CONTROL_NEXT = 0x00000001,
    .CREDIT_CONTROL_NEXT = 0x00000F03,
    .WRITE_PRIORITY_CONTROL_31_00_NEXT = 0x00000000,
    .WRITE_PRIORITY_CONTROL_63_32_NEXT = 0xECA86421,
    .QUEUE_THRESHOLD_CONTROL_31_00_NEXT = 0x00000008,
    .QUEUE_THRESHOLD_CONTROL_63_32_NEXT = 0x00000000,
    .ADDRESS_SHUTTER_31_00_NEXT = 0x11111110,
    .ADDRESS_SHUTTER_63_32_NEXT = 0x11111111,
    .ADDRESS_SHUTTER_95_64_NEXT = 0x11111111,
    .ADDRESS_SHUTTER_127_96_NEXT = 0x11111111,
    .ADDRESS_SHUTTER_159_128_NEXT = 0x11111111,
    .ADDRESS_SHUTTER_191_160_NEXT = 0x11111111,
    .MEMORY_ADDRESS_MAX_31_00_NEXT = 0xFFFF001F,
    .MEMORY_ADDRESS_MAX_43_32_NEXT = 0x00000FFF,
    .ACCESS_ADDRESS_NEXT = {
        [0] = {.MIN_31_00 = 0x0000000F, .MIN_43_32 = 0x00000000},
        [1] = {.MIN_31_00 = 0x0000000F, .MIN_43_32 = 0x00000000},
        [2] = {.MIN_31_00 = 0x0000000F},
        [3] = {.MIN_31_00 = 0x0000000F},
        [4] = {.MIN_31_00 = 0x0000000F},
        [5] = {.MIN_31_00 = 0x0000000F},
        [6] = {.MIN_31_00 = 0x0000000F},
        [7] = {.MIN_31_00 = 0x0000000F},
    },
    .DCI_REPLAY_TYPE_NEXT = 0x00000000,
    .DCI_STRB = 0x00000007,
    .DCI_DATA = 0x00000000,
    .REFRESH_CONTROL_NEXT = 0x00000000,
    .MEMORY_TYPE_NEXT = MEMORY_TYPE_NEXT_VAL,
    .FEATURE_CONFIG = 0x00001800,
    .FEATURE_CONTROL_NEXT = 0x00000000,
    .MUX_CONTROL_NEXT = 0x00000000,
    .T_REFI_NEXT = 0x90000618,
    .T_RFC_NEXT = 0x06A8C230,
    .T_MRR_NEXT = 0x00000001,
    .T_MRW_NEXT = 0x00010018,
    .T_RCD_NEXT = 0x00000014,
    .T_RAS_NEXT = 0x00000034,
    .T_RP_NEXT = 0x00000014,
    .T_RPALL_NEXT = 0x00000014,
    .T_RRD_NEXT = 0x04000805,
    .T_ACT_WINDOW_NEXT = 0x00001010,
    .T_RTR_NEXT = 0x14060804,
    .T_RTW_NEXT = 0x000A0A0A,
    .T_RTP_NEXT = 0x0000000C,
    .T_WR_NEXT = 0x0000002C,
    .T_WTR_NEXT = 0x00022019,
    .T_WTW_NEXT = 0x14060804,
    .T_XMPD_NEXT = 0x00000510,
    .T_EP_NEXT = 0x00000008,
    .T_XP_NEXT = 0x0014000A,
    .T_ESR_NEXT = 0x00000009,
    .T_XSR_NEXT = 0x04000110,
    .T_ESRCK_NEXT = 0x00000010,
    .T_CKXSR_NEXT = 0x00000010,
    .T_CMD_NEXT = 0x00000000,
    .T_PARITY_NEXT = 0x00001600,
    .T_ZQCS_NEXT = 0x00000090,
    .T_RW_ODT_CLR_NEXT = 0x0000005E,
    .T_RDDATA_EN_NEXT = 0x00000000,
    .T_PHYWRLAT_NEXT = 0x001F000E,
    .T_PHYRDLAT_NEXT = 0x0000002E,
    .RDLVL_CONTROL_NEXT = 0x00000000,
    .RDLVL_MRS_NEXT = 0x00000424,
    .T_RDLVL_EN_NEXT = 0x00000001,
    .T_RDLVL_RR_NEXT = 0x0000001A,
    .WRLVL_CONTROL_NEXT = 0x00100000,
    .WRLVL_MRS_NEXT = 0x00000181,
    .T_WRLVL_EN_NEXT = 0x00000018,
    .T_WRLVL_WW_NEXT = 0x00000001,
    .PHY_POWER_CONTROL_NEXT = 0x00000000,
    .T_LPRESP_NEXT = 0x00000000,
    .PHY_UPDATE_CONTROL_NEXT = 0x00000000,
    .T_ODTH_NEXT = 0x00000006,
    .ODT_TIMING_NEXT = 0x07003900,
    .ODT_WR_CONTROL_31_00_NEXT = 0x08040201,
    .ODT_WR_CONTROL_63_32_NEXT = 0x80402010,
    .ODT_RD_CONTROL_31_00_NEXT = 0x00000000,
    .ODT_RD_CONTROL_63_32_NEXT = 0x00000000,
    .ERR0CTLR0 = DMC_ERR0CTRL0_ED_ENABLE |
                 DMC_ERR0CTRL0_DE_ENABLE |
                 DMC_ERR0CTRL0_UI_ENABLE |
                 DMC_ERR0CTRL0_FI_ENABLE |
                 DMC_ERR0CTRL0_CFI_ENABLE,
};

/* Table of DMC620 elements descriptions. */
static struct fwk_element dmc620_element_table[] = {
    [0] = { .name = "DMC620-0",
            .data = &((struct mod_dmc620_element_config) {
                .dmc = SCP_DMC0,
                .ddr_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_DDR_PHY500, 0),
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
        }),
    },
    [1] = { .name = "DMC620-1",
            .data = &((struct mod_dmc620_element_config) {
                .dmc = SCP_DMC1,
                .ddr_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_DDR_PHY500, 1),
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
        }),
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *dmc620_get_element_table(fwk_id_t module_id)
{
    return dmc620_element_table;
}

static void direct_ddr_cmd(struct mod_dmc620_reg *dmc)
{
    dmc->DIRECT_ADDR = 0x00000004;
    dmc->DIRECT_CMD  = 0x0001000A;
    dmc->DIRECT_ADDR = 0x00000006;
    dmc->DIRECT_CMD  = 0x00030004;
    dmc->DIRECT_ADDR = 0x00000000;
    dmc->DIRECT_CMD  = 0x0001000B;
    dmc->DIRECT_ADDR = 0x00000001;
    dmc->DIRECT_CMD  = 0x0003000B;
    dmc->DIRECT_ADDR = 0x000003E8;
    dmc->DIRECT_CMD  = 0x0001000D;
    dmc->DIRECT_ADDR = 0x00000258;
    dmc->DIRECT_CMD  = 0x0001000D;
    dmc->DIRECT_ADDR = 0x00010001;
    dmc->DIRECT_CMD  = 0x0003000B;
    dmc->DIRECT_ADDR = 0x0000003C;
    dmc->DIRECT_CMD  = 0x0001000D;
    dmc->DIRECT_ADDR = 0x00000000;
    dmc->DIRECT_CMD  = 0x00030000;
    dmc->DIRECT_ADDR = 0x0000003C;
    dmc->DIRECT_ADDR = 0x00000420;
    dmc->DIRECT_CMD  = 0x00030301;
    dmc->DIRECT_ADDR = 0x00001000;
    dmc->DIRECT_CMD  = 0x00030601;
    dmc->DIRECT_ADDR = 0x00000600;
    dmc->DIRECT_CMD  = 0x00030501;
    dmc->DIRECT_ADDR = 0x00000000;
    dmc->DIRECT_CMD  = 0x30030401;
    dmc->DIRECT_ADDR = 0x00000028;
    dmc->DIRECT_CMD  = 0x00030201;
    dmc->DIRECT_ADDR = 0x00000001;
    dmc->DIRECT_CMD  = 0x00030101;
    dmc->DIRECT_ADDR = 0x00000D50;
    dmc->DIRECT_CMD  = 0x00030001;
    dmc->DIRECT_ADDR = 0x000003F6;
    dmc->DIRECT_CMD  = 0x0001000D;
    dmc->DIRECT_ADDR = 0x0000000A;
    dmc->DIRECT_CMD  = 0x0001000D;
    dmc->DIRECT_ADDR = 0x00000400;
    dmc->DIRECT_CMD  = 0x00030005;
    dmc->DIRECT_ADDR = 0x000003FF;
    dmc->DIRECT_CMD  = 0x0001000D;

}

/* Configuration of the DMC500 module. */
const struct fwk_module_config config_dmc620 = {
    .get_element_table = dmc620_get_element_table,
    .data = &((struct mod_dmc620_module_config) {
            .dmc_val = &dmc_val,
            .ddr_module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_DDR_PHY500),
            .ddr_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_DDR_PHY500, 0),
            .direct_ddr_cmd = direct_ddr_cmd,
        }),
};
