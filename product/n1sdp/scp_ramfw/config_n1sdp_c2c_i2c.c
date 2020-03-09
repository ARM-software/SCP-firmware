/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_n1sdp_c2c_i2c.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

const struct fwk_module_config config_n1sdp_c2c = {
    .data = &((struct n1sdp_c2c_dev_config) {
            .i2c_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_I2C, 1),
            .slave_addr = 0x14,
            .ccix_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_PCIE, 1),
        }),
};
