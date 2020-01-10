/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "synquacer_mmap.h"

#include <sysdef_option.h>

#include <internal/nic400.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>
#include <fwk_macros.h>

#include <inttypes.h>
#include <stdint.h>

#define NIC_TOP_ADDR_SEC_REG (0x00000008)
#define NIC_SEC_REG_OFFSET (0x04)

static void nic_sec_slave_security(
    uint32_t nic_top_addr,
    uint32_t slave_index,
    uint32_t value)
{
    FWK_LOG_INFO(
        "%s addr 0x%08" PRIx32 " value 0x%08" PRIx32,
        __func__,
        (nic_top_addr + NIC_TOP_ADDR_SEC_REG +
         NIC_SEC_REG_OFFSET * slave_index),
        value);

    writel(
        (nic_top_addr + NIC_TOP_ADDR_SEC_REG +
         NIC_SEC_REG_OFFSET * slave_index),
        value);
}

void nic_secure_access_ctrl_init(void)
{
    uint32_t n, m;
    static const uint32_t nic_base_addr[] = CONFIG_SOC_NIC_ADDR_INFO;
    static const uint32_t nic_config[][32] = CONFIG_SCB_NIC_INFO;
    const uint32_t *config;

    for (n = 0; n < FWK_ARRAY_SIZE(nic_base_addr); n++) {
        for (m = 0; nic_config[n][m] != END_OF_NIC_LIST; m++) {
            if (nic_config[n][m] == NIC_SETUP_SKIP)
                continue;

            nic_sec_slave_security(nic_base_addr[n], m, nic_config[n][m]);
        }
    }

    config = sysdef_option_get_scbm_mv_nic_config();
    for (n = 0; config[n] != END_OF_NIC_LIST; n++) {
        if (config[n] == NIC_SETUP_SKIP)
            continue;

        nic_sec_slave_security(SCBM_MV_NIC, n, config[n]);
    }
}
