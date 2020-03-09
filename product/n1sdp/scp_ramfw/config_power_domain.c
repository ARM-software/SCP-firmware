/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "config_ppu_v0.h"
#include "n1sdp_core.h"
#include "n1sdp_power_domain.h"

#include <mod_n1sdp_remote_pd.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>
#include <stdio.h>

/* Maximum power domain name size including the null terminator */
#define PD_NAME_SIZE 16

/* Mask of the allowed states for the systop logical power domain */
static const uint32_t systop_logical_allowed_state_mask_table[] = {
    [0] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK
};

/* Mask of the allowed states for the systop power domain */
static const uint32_t systop_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK |
          (1 << MOD_SYSTEM_POWER_POWER_STATE_SLEEP0) |
          (1 << MOD_SYSTEM_POWER_POWER_STATE_SLEEP1)
};

/*
 * Mask of the allowed states for the top level power domains
 * (but the cluster power domains) depending on the system states.
 */
static const uint32_t toplevel_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = MOD_PD_STATE_OFF_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP1] = MOD_PD_STATE_OFF_MASK
};

/*
 * Mask of the allowed states for the cluster power domain depending on the
 * system states.
 */
static const uint32_t cluster_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] = N1SDP_CLUSTER_VALID_STATE_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = MOD_PD_STATE_OFF_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP1] = MOD_PD_STATE_OFF_MASK
};

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] = N1SDP_CORE_VALID_STATE_MASK,
    [MOD_PD_STATE_SLEEP] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [N1SDP_POWER_DOMAIN_STATE_FUNC_RET] = N1SDP_CORE_VALID_STATE_MASK,
    [N1SDP_POWER_DOMAIN_STATE_MEM_RET] = MOD_PD_STATE_OFF_MASK
};

/* Power module specific configuration data (none) */
static const struct mod_power_domain_config n1sdp_power_domain_config = { 0 };

/* Power domain element table pointer */
struct fwk_element *element_table = NULL;

/* Power domain element configuration table pointer */
struct mod_power_domain_element_config *pd_config_table = NULL;

/*
 * The power domain tree view differs in master and slave chips in multichip
 * use case. In multichip scenario master SCP sees the power domains of both
 * local domains and slave chip's power domain as one single PD tree.
 * The view will look like:
 *
 *                    -----SYSTOP (LOGICAL)-------
 *                   /                            \
 *                  /                              \
 *            ---SYSTOP0--                   ---SYSTOP1--
 *           /      |     \                 /      |     \
 *          /       |      \               /       |      \
 *         /        |       \             /        |       \
 *    CLUS0       CLUS1    DBGTOP0   CLUS2       CLUS3     DBGTOP1
 *   /    \      /    \             /    \      /    \
 * CPU0--CPU1--CPU2--CPU3---------CPU4--CPU5--CPU6--CPU7
 *
 * The slave chip's SCP however looks a single chip view of PD tree having
 * its own local power domains and looks like below:
 *
 *            ---SYSTOP0--
 *           /      |     \
 *          /       |      \
 *         /        |       \
 *    CLUS0       CLUS1    DBGTOP0
 *   /    \      /    \
 * CPU0--CPU1--CPU2--CPU3
 *
 * The PD view will be chosen in runtime based on the multichip enable bit
 * and chip ID value extracted from SCC PLATFORM_CTRL register.
 */

static struct fwk_element n1sdp_pd_single_chip_element_table[] = {
    [PD_SINGLE_CHIP_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 0, 1),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 1),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 1, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 2),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 1, 1),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 3),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUSTER0] = {
        .name = "CLUS0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 0, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 4),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUSTER1] = {
        .name = "CLUS1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 0, 1, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 5),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_DBGTOP0] = {
        .name = "DBGTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 0, 2, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PPU_V0, PPU_V0_ELEMENT_IDX_DBGTOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_SYSTOP0] = {
        .name = "SYSTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2, 0, 0, 0, 0),
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_COUNT] = { 0 },
};

static struct fwk_element n1sdp_pd_multi_chip_element_table[] = {
    [PD_MULTI_CHIP_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 0, 1),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 1),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 1, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 2),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 0, 1, 1),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 3),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUS2CORE0] = {
        .name = "SLV-CLUS0CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 1, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUS2CORE1] = {
        .name = "SLV-CLUS0CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 1, 0, 1),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUS3CORE0] = {
        .name = "SLV-CLUS1CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 1, 1, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUS3CORE1] = {
        .name = "SLV-CLUS1CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0, 0, 1, 1, 1),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUSTER0] = {
        .name = "CLUS0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 0, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 4),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUSTER1] = {
        .name = "CLUS1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 0, 1, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 5),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_DBGTOP0] = {
        .name = "DBGTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 0, 2, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PPU_V0, PPU_V0_ELEMENT_IDX_DBGTOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUSTER2] = {
        .name = "SLV-CLUS0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 1, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_CLUSTER3] = {
        .name = "SLV-CLUS1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 1, 1, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_DBGTOP1] = {
        .name = "SLV-DBGTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1, 0, 1, 2, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_SYSTOP0] = {
        .name = "SYSTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2, 0, 0, 0, 0),
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_SYSTOP1] = {
        .name = "SLV-SYSTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2, 0, 1, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_SYSTOP_LOGICAL] = {
        .name = "SYSTOP-LOGICAL",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_3, 0, 0, 0, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                             8),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_REMOTE_PD,
                                      N1SDP_REMOTE_PD_API_IDX),
            .allowed_state_mask_table = systop_logical_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_logical_allowed_state_mask_table)
        }),
    },
    [PD_MULTI_CHIP_IDX_COUNT] = { 0 },
};

/*
 * Function definitions with internal linkage
 */
static const struct fwk_element *n1sdp_power_domain_get_element_table
    (fwk_id_t module_id)
{
    if (n1sdp_is_multichip_enabled() && (n1sdp_get_chipid() == 0x0))
        return n1sdp_pd_multi_chip_element_table;
    else
        return n1sdp_pd_single_chip_element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_power_domain = {
    .get_element_table = n1sdp_power_domain_get_element_table,
    .data = &n1sdp_power_domain_config,
};
