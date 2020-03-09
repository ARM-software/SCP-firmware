/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_SYSTEM_H
#define JUNO_SYSTEM_H

#include "config_power_domain.h"
#include "config_psu.h"

#include <mod_psu.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

/* PSU API identifier */
static const fwk_id_t psu_api_id =
    FWK_ID_API_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_API_IDX_DEVICE);

/* GPU Power Domain identifier */
static const fwk_id_t gpu_pd_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, POWER_DOMAIN_IDX_GPUTOP);

/* GPU PSU identifier */
static const fwk_id_t gpu_psu_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_ELEMENT_IDX_VGPU);

#endif  /* JUNO_SYSTEM_H */
