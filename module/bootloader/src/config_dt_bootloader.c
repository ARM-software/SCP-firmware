/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_bootloader.h>

#include <fwk_module.h>

#include <fwk_dt_config_common.h>

#define BOOTLOADER_COMPAT arm_scp_bootloader


/* must only be a single bootloader defined */
static_assert(DT_N_INST_arm_scp_bootloader_NUM_OKAY == 1,
              "Error - multiple bootloaders defined in device tree - must only be one");


static const struct mod_bootloader_config bootloader_module_config = {
    .source_base = FWK_DT_INST_IDX_PROP(BOOTLOADER_COMPAT, 0, source, 0),
    .source_size = FWK_DT_INST_IDX_PROP(BOOTLOADER_COMPAT, 0, source, 1),
    .destination_base = FWK_DT_INST_IDX_PROP(BOOTLOADER_COMPAT, 0, destination, 0),
    .destination_size = FWK_DT_INST_IDX_PROP(BOOTLOADER_COMPAT, 0, destination, 1),
#ifdef BUILD_HAS_MOD_SDS
    .sds_struct_id = FWK_DT_INST_PROP_DEFAULT(BOOTLOADER_COMPAT, 0, sds_id, 0),
#endif
};

struct fwk_module_config config_dt_bootloader = {
    .data = &bootloader_module_config,
};
