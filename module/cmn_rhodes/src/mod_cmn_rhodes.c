/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmn_rhodes.h>

#include <internal/cmn_rhodes_ctx.h>

#include <mod_clock.h>
#include <mod_cmn_rhodes.h>
#include <mod_ppu_v1.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <inttypes.h>

#define MOD_NAME "[CMN_RHODES] "

static struct cmn_rhodes_ctx *ctx;

static void process_node_hnf(struct cmn_rhodes_hnf_reg *hnf)
{
    unsigned int bit_pos;
    unsigned int group;
    unsigned int logical_id;
    unsigned int node_id;
    unsigned int region_idx;
    unsigned int region_sub_count = 0;
    static unsigned int cal_mode_factor = 1;
    const struct mod_cmn_rhodes_mem_region_map *region;
    const struct mod_cmn_rhodes_config *config = ctx->config;

    logical_id = get_node_logical_id(hnf);
    node_id = get_node_id(hnf);

    /*
     * If CAL mode is set, only even numbered hnf node should be added to the
     * sys_cache_grp_hn_nodeid registers and hnf_count should be incremented
     * only for the even numbered hnf nodes.
     */
    if (config->hnf_cal_mode == true && (node_id % 2 == 1)) {
        /* Factor to manipulate the group and bit_pos */
        cal_mode_factor = 2;

        /*
         * Reduce the hnf_count as the current hnf node is not getting included
         * in the sys_cache_grp_hn_nodeid register
         */
        ctx->hnf_count--;
    }

    assert(logical_id < config->snf_count);

    group = logical_id /
            (CMN_RHODES_HNF_CACHE_GROUP_ENTRIES_PER_GROUP * cal_mode_factor);
    bit_pos = (CMN_RHODES_HNF_CACHE_GROUP_ENTRY_BITS_WIDTH / cal_mode_factor) *
              ((logical_id %
                (CMN_RHODES_HNF_CACHE_GROUP_ENTRIES_PER_GROUP *
                 cal_mode_factor)));

    /*
     * If CAL mode is set, add only even numbered hnd node to
     * sys_cache_grp_hn_nodeid registers
     */
    if (config->hnf_cal_mode == true) {
        if (node_id % 2 == 0)
            ctx->hnf_cache_group[group] += ((uint64_t)get_node_id(hnf)) <<
                                           bit_pos;
    } else
        ctx->hnf_cache_group[group] += ((uint64_t)get_node_id(hnf)) << bit_pos;

    /* Set target node */
    hnf->SAM_CONTROL = config->snf_table[logical_id];

    /*
     * Map sub-regions to this HN-F node
     */
    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        region = &config->mmap_table[region_idx];

        /* Skip non sub-regions */
        if (region->type != MOD_CMN_RHODES_REGION_TYPE_SYSCACHE_SUB)
            continue;

        /* Configure sub-region entry */
        hnf->SAM_MEMREGION[region_sub_count] = region->node_id |
            (sam_encode_region_size(region->size) <<
             CMN_RHODES_HNF_SAM_MEMREGION_SIZE_POS) |
            ((region->base / SAM_GRANULARITY) <<
             CMN_RHODES_HNF_SAM_MEMREGION_BASE_POS) |
            CMN_RHODES_HNF_SAM_MEMREGION_VALID;

        region_sub_count++;
    }

    /* Configure the system cache RAM PPU */
    hnf->PPU_PWPR = CMN_RHODES_PPU_PWPR_POLICY_ON |
                    CMN_RHODES_PPU_PWPR_OPMODE_FAM |
                    CMN_RHODES_PPU_PWPR_DYN_EN;
}

/*
 * Discover the topology of the interconnect and identify the number of:
 * - External RN-SAM nodes
 * - Internal RN-SAM nodes
 * - HN-F nodes (cache)
 */
static int cmn_rhodes_discovery(void)
{
    unsigned int xp_count;
    unsigned int xp_idx;
    unsigned int node_count;
    unsigned int node_idx;
    bool xp_port;
    struct cmn_rhodes_mxp_reg *xp;
    struct node_header *node;
    const struct mod_cmn_rhodes_config *config = ctx->config;

    FWK_LOG_INFO(MOD_NAME "Starting discovery...");

    assert(get_node_type(ctx->root) == NODE_TYPE_CFG);

    /* Traverse cross points (XP) */
    xp_count = get_node_child_count(ctx->root);
    for (xp_idx = 0; xp_idx < xp_count; xp_idx++) {
        xp = get_child_node(config->base, ctx->root, xp_idx);
        assert(get_node_type(xp) == NODE_TYPE_XP);

        FWK_LOG_INFO(
            MOD_NAME "XP (%d, %d) ID:%d, LID:%d",
            get_node_pos_x(xp),
            get_node_pos_y(xp),
            get_node_id(xp),
            get_node_logical_id(xp));

        /* Traverse nodes */
        node_count = get_node_child_count(xp);
        for (node_idx = 0; node_idx < node_count; node_idx++) {
            node = get_child_node(config->base, xp, node_idx);
            if (is_child_external(xp, node_idx)) { /* External nodes */
                xp_port = get_port_number(get_child_node_id(xp, node_idx));

                /*
                 * If the device type is CXRH, CXHA, or CXRA, then the external
                 * child node is CXLA as every CXRH, CXHA, or CXRA node has a
                 * corresponding external CXLA node.
                 */
                if ((get_device_type(xp, xp_port) == DEVICE_TYPE_CXRH) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXHA) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXRA)) {
                    FWK_LOG_INFO(
                        MOD_NAME "  Found CXLA at node ID: %d",
                        get_child_node_id(xp, node_idx));
                } else { /* External RN-SAM Node */
                    ctx->external_rnsam_count++;
                    FWK_LOG_INFO(
                        MOD_NAME "  Found external node ID: %d",
                        get_child_node_id(xp, node_idx));
                }
            } else { /* Internal nodes */
                switch (get_node_type(node)) {
                case NODE_TYPE_HN_F:
                    if (ctx->hnf_count >= MAX_HNF_COUNT) {
                        FWK_LOG_INFO(
                            MOD_NAME "  hnf count %d >= max limit (%d)",
                            ctx->hnf_count,
                            MAX_HNF_COUNT);
                        return FWK_E_DATA;
                    }
                    ctx->hnf_count++;
                    break;

                case NODE_TYPE_RN_SAM:
                    ctx->internal_rnsam_count++;
                    break;

                default:
                    /* Nothing to be done for other node types */
                    break;
                }

                FWK_LOG_INFO(
                    MOD_NAME "  %s ID:%d, LID:%d",
                    get_node_type_name(get_node_type(node)),
                    get_node_id(node),
                    get_node_logical_id(node));
            }
        }
    }

    /* When CAL is present, the number of HN-Fs must be even. */
    if ((ctx->hnf_count % 2 != 0) && (config->hnf_cal_mode == true)) {
        FWK_LOG_ERR(
            MOD_NAME "hnf count: %d should be even when CAL mode is set",
            ctx->hnf_count);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO(
        MOD_NAME "Total internal RN-SAM nodes: %d" MOD_NAME
                 "Total external RN-SAM nodes: %d" MOD_NAME
                 "Total HN-F nodes: %d",
        ctx->internal_rnsam_count,
        ctx->external_rnsam_count,
        ctx->hnf_count);

    return FWK_SUCCESS;
}

static void cmn_rhodes_configure(void)
{
    unsigned int node_count;
    unsigned int node_idx;
    unsigned int xp_count;
    unsigned int xp_idx;
    unsigned int xrnsam_entry;
    unsigned int irnsam_entry;
    bool xp_port;
    void *node;
    struct cmn_rhodes_mxp_reg *xp;
    const struct mod_cmn_rhodes_config *config = ctx->config;

    assert(get_node_type(ctx->root) == NODE_TYPE_CFG);

    xrnsam_entry = 0;
    irnsam_entry = 0;

    /* Traverse cross points (XP) */
    xp_count = get_node_child_count(ctx->root);
    for (xp_idx = 0; xp_idx < xp_count; xp_idx++) {
        xp = get_child_node(config->base, ctx->root, xp_idx);
        assert(get_node_type(xp) == NODE_TYPE_XP);

        /* Traverse nodes */
        node_count = get_node_child_count(xp);
        for (node_idx = 0; node_idx < node_count; node_idx++) {
            node = get_child_node(config->base, xp, node_idx);
            if (is_child_external(xp, node_idx)) {
                unsigned int node_id = get_child_node_id(xp, node_idx);
                xp_port = get_port_number(get_child_node_id(xp, node_idx));

                /* Skip if the device type is CXG */
                if ((get_device_type(xp, xp_port) == DEVICE_TYPE_CXRH) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXHA) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXRA))
                    continue;

                fwk_assert(xrnsam_entry < ctx->external_rnsam_count);

                ctx->external_rnsam_table[xrnsam_entry].node_id = node_id;
                ctx->external_rnsam_table[xrnsam_entry].node = node;

                xrnsam_entry++;
            } else {
                enum node_type node_type = get_node_type(node);

                if (node_type == NODE_TYPE_RN_SAM) {
                    fwk_assert(irnsam_entry < ctx->internal_rnsam_count);

                    ctx->internal_rnsam_table[irnsam_entry] = node;

                    irnsam_entry++;
                } else if (node_type == NODE_TYPE_HN_F)
                    process_node_hnf(node);
            }
        }
    }
}

static const char * const mmap_type_name[] = {
    [MOD_CMN_RHODES_MEM_REGION_TYPE_IO] = "I/O",
    [MOD_CMN_RHODES_MEM_REGION_TYPE_SYSCACHE] = "System Cache",
    [MOD_CMN_RHODES_REGION_TYPE_SYSCACHE_SUB] = "Sub-System Cache",
};

static int cmn_rhodes_setup_sam(struct cmn_rhodes_rnsam_reg *rnsam)
{
    unsigned int bit_pos;
    unsigned int group;
    unsigned int group_count;
    unsigned int region_idx;
    unsigned int region_io_count = 0;
    unsigned int region_sys_count = 0;
    unsigned int scg_regions_enabled[MAX_SCG_COUNT] = {0, 0, 0, 0};
    const struct mod_cmn_rhodes_mem_region_map *region;
    const struct mod_cmn_rhodes_config *config = ctx->config;

    FWK_LOG_INFO(MOD_NAME "Configuring SAM for node %d", get_node_id(rnsam));

    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        region = &config->mmap_table[region_idx];

        FWK_LOG_INFO(
            MOD_NAME "  [0x%" PRIx64 " - 0x%" PRIx64 "] %s",
            region->base,
            region->base + region->size - 1,
            mmap_type_name[region->type]);

        switch (region->type) {
        case MOD_CMN_RHODES_MEM_REGION_TYPE_IO:
            /*
             * Configure memory region
             */
            configure_region(&rnsam->NON_HASH_MEM_REGION[region_io_count],
                             region->base,
                             region->size,
                             SAM_NODE_TYPE_HN_I);

            /*
             * Configure target node
             */
            group = region_io_count /
                    CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP;
            bit_pos = CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
                      (region_io_count %
                       CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);

            rnsam->NON_HASH_TGT_NODEID[group] &=
                ~(CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK << bit_pos);
            rnsam->NON_HASH_TGT_NODEID[group] |=
                (region->node_id &
                 CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK) << bit_pos;

            region_io_count++;
            break;

        case MOD_CMN_RHODES_MEM_REGION_TYPE_SYSCACHE:
            /*
             * Configure memory region
             */
            configure_region(&rnsam->SYS_CACHE_GRP_REGION[region_sys_count],
                             region->base,
                             region->size,
                             SAM_NODE_TYPE_HN_F);

            /* Mark corresponding region as enabled */
            fwk_assert(region_sys_count < MAX_SCG_COUNT);
            scg_regions_enabled[region_sys_count] = 1;

            region_sys_count++;
            break;

        case MOD_CMN_RHODES_REGION_TYPE_SYSCACHE_SUB:
            /* Do nothing. System cache sub-regions are handled by HN-Fs */
            break;

        default:
            assert(false);
            return FWK_E_DATA;
        }
    }

    group_count = ctx->hnf_count / CMN_RHODES_HNF_CACHE_GROUP_ENTRIES_PER_GROUP;
    for (group = 0; group < group_count; group++)
        rnsam->SYS_CACHE_GRP_HN_NODEID[group] = ctx->hnf_cache_group[group];

    /* Program the number of HNFs */
    rnsam->SYS_CACHE_GRP_HN_COUNT = ctx->hnf_count;

    /*
     * If CAL mode is enabled by the configuration program the SCG CAL Mode
     * enable register.
     */
    if (config->hnf_cal_mode)
        for (region_idx = 0; region_idx < MAX_SCG_COUNT; region_idx++)
            rnsam->SYS_CACHE_GRP_CAL_MODE = scg_regions_enabled[region_idx] *
                (CMN_RHODES_RNSAM_SCG_HNF_CAL_MODE_EN <<
                 (region_idx * CMN_RHODES_RNSAM_SCG_HNF_CAL_MODE_SHIFT));

    /* Program the SYS_CACHE_GRP_SN_NODEID register for PrefetchTgt */
     if (config->hnf_cal_mode)
         group_count = config->snf_count/
             (CMN_RHODES_RNSAM_SYS_CACHE_GRP_SN_NODEID_ENTRIES_PER_GROUP * 2);
     else
         group_count = config->snf_count/
             CMN_RHODES_RNSAM_SYS_CACHE_GRP_SN_NODEID_ENTRIES_PER_GROUP;

     for (group = 0; group < group_count; group++)
         rnsam->SYS_CACHE_GRP_SN_NODEID[group] = ctx->sn_nodeid_group[group];

    /* Enable RNSAM */
    rnsam->STATUS = ((uint64_t)config->hnd_node_id <<
                     CMN_RHODES_RNSAM_STATUS_DEFAULT_NODEID_POS) |
                    CMN_RHODES_RNSAM_STATUS_UNSTALL;
    __sync_synchronize();

    return FWK_SUCCESS;
}

static int cmn_rhodes_setup(void)
{
    unsigned int rnsam_idx;
    int status = FWK_SUCCESS;

    if (!ctx->initialized) {
        status = cmn_rhodes_discovery();
        if (status != FWK_SUCCESS)
            return FWK_SUCCESS;

        /*
         * Allocate resources based on the discovery
         */

        /* Pointers for the internal RN-SAM nodes */
        if (ctx->internal_rnsam_count != 0) {
            ctx->internal_rnsam_table = fwk_mm_calloc(
                ctx->internal_rnsam_count, sizeof(*ctx->internal_rnsam_table));
        }

        /* Tuples for the external RN-RAM nodes (including their node IDs) */
        if (ctx->external_rnsam_count != 0) {
            ctx->external_rnsam_table = fwk_mm_calloc(
                ctx->external_rnsam_count, sizeof(*ctx->external_rnsam_table));
        }

        /* Cache groups */
        if (ctx->hnf_count != 0) {
            /*
             * Allocate enough group descriptors to accommodate all expected
             * HN-F nodes in the system.
             */
            ctx->hnf_cache_group = fwk_mm_calloc(
                ctx->hnf_count / CMN_RHODES_HNF_CACHE_GROUP_ENTRIES_PER_GROUP,
                sizeof(*ctx->hnf_cache_group));
            ctx->sn_nodeid_group = fwk_mm_calloc(
                ctx->hnf_count /
                CMN_RHODES_RNSAM_SYS_CACHE_GRP_SN_NODEID_ENTRIES_PER_GROUP,
                sizeof(*ctx->sn_nodeid_group));
        }
    }

    cmn_rhodes_configure();

    /* Setup internal RN-SAM nodes */
    for (rnsam_idx = 0; rnsam_idx < ctx->internal_rnsam_count; rnsam_idx++)
        cmn_rhodes_setup_sam(ctx->internal_rnsam_table[rnsam_idx]);

    FWK_LOG_INFO(MOD_NAME "Done");

    ctx->initialized = true;

    return FWK_SUCCESS;
}

static int cmn_rhodes_setup_rnsam(unsigned int node_id)
{
    unsigned int node_idx;

    for (node_idx = 0; node_idx < ctx->external_rnsam_count; node_idx++) {
        if (ctx->external_rnsam_table[node_idx].node_id == node_id) {
            cmn_rhodes_setup_sam(ctx->external_rnsam_table[node_idx].node);
            return FWK_SUCCESS;
        }
    }

    return FWK_E_PARAM;
}

/*
 * PPUv1 State Observer API
 */

static void post_ppu_on(void *data)
{
    assert(data != NULL);
    cmn_rhodes_setup_rnsam(*(unsigned int *)data);
}

static const struct mod_ppu_v1_power_state_observer_api
cmn_rhodes_observer_api = {
    .post_ppu_on = post_ppu_on,
};

/*
 * Framework handlers
 */

static int cmn_rhodes_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    const struct mod_cmn_rhodes_config *config = data;

    /* No elements support */
    if (element_count > 0)
        return FWK_E_DATA;

    /* Allocate space for the context */
    ctx = fwk_mm_calloc(1, sizeof(*ctx));

    if (config->base == 0)
        return FWK_E_DATA;

    if ((config->mesh_size_x == 0) ||
        (config->mesh_size_x > CMN_RHODES_MESH_X_MAX))
        return FWK_E_DATA;

    if ((config->mesh_size_y == 0) ||
        (config->mesh_size_y > CMN_RHODES_MESH_Y_MAX))
        return FWK_E_DATA;

    if (config->snf_count > CMN_RHODES_HNF_CACHE_GROUP_ENTRIES_MAX)
        return FWK_E_DATA;

    ctx->root = get_root_node(config->base, config->hnd_node_id,
        config->mesh_size_x, config->mesh_size_y);

    ctx->config = config;

    return FWK_SUCCESS;
}

static int cmn_rhodes_process_bind_request(fwk_id_t requester_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    *api = &cmn_rhodes_observer_api;
    return FWK_SUCCESS;
}

int cmn_rhodes_start(fwk_id_t id)
{
    if (fwk_id_is_equal(ctx->config->clock_id, FWK_ID_NONE)) {
        cmn_rhodes_setup();
        return FWK_SUCCESS;
    }

    /* Register the module for clock state notifications */
    return fwk_notification_subscribe(mod_clock_notification_id_state_changed,
                                      ctx->config->clock_id, id);
}

static int cmn_rhodes_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    assert(fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    params = (struct clock_notification_params *)event->params;
    if (params->new_state == MOD_CLOCK_STATE_RUNNING)
        cmn_rhodes_setup();

    return FWK_SUCCESS;
}

const struct fwk_module module_cmn_rhodes = {
    .name = "CMN_RHODES",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_CMN_RHODES_API_COUNT,
    .init = cmn_rhodes_init,
    .start = cmn_rhodes_start,
    .process_bind_request = cmn_rhodes_process_bind_request,
    .process_notification = cmn_rhodes_process_notification,
};
