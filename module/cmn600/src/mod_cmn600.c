/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_cmn600.h>
#include <mod_log.h>
#include <mod_power_domain.h>
#include <cmn600.h>
#include <mod_ppu_v1.h>

#define MOD_NAME "[CMN600] "

/* External nodes that require RN-SAM mapping during run-time */
struct external_rnsam_tuple {
    unsigned int node_id;
    struct cmn600_rnsam_reg *node;
};

struct cmn600_ctx {
    const struct mod_cmn600_config *config;

    struct cmn600_cfgm_reg *root;

    /* Number of HN-F (system cache) nodes in the system */
    unsigned int hnf_count;
    uint64_t *hnf_cache_group;

    /*
     * External RN-SAMs. The driver keeps a list of tuples (node identifier and
     * node pointers). The configuration of these nodes is via the SAM API.
     */
    unsigned int external_rnsam_count;
    struct external_rnsam_tuple *external_rnsam_table;

    /*
     * Internal RN-SAMs. The driver keeps a list of RN-SAM pointers to
     * configure them once the system has been fully discovered and all
     * parameters are known
     */
    unsigned int internal_rnsam_count;
    struct cmn600_rnsam_reg **internal_rnsam_table;

    struct mod_log_api *log_api;

    bool initialized;
} *ctx;

static void process_node_hnf(struct cmn600_hnf_reg *hnf)
{
    unsigned int logical_id;
    unsigned int group;
    unsigned int bit_pos;
    unsigned int region_idx;
    unsigned int region_sub_count = 0;
    const struct mod_cmn600_memory_region_map *region;
    const struct mod_cmn600_config *config = ctx->config;

    logical_id = get_node_logical_id(hnf);

    assert(logical_id < config->snf_count);

    group = logical_id / CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP;
    bit_pos = CMN600_HNF_CACHE_GROUP_ENTRY_BITS_WIDTH *
              (logical_id % CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP);

    ctx->hnf_cache_group[group] += ((uint64_t)get_node_id(hnf)) << bit_pos;

    /* Set target node */
    hnf->SAM_CONTROL = config->snf_table[logical_id];

    /*
     * Map sub-regions to this HN-F node
     */
    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        region = &config->mmap_table[region_idx];

        /* Skip non sub-regions */
        if (region->type != MOD_CMN600_REGION_TYPE_SYSCACHE_SUB)
            continue;

        /* Configure sub-region entry */
        hnf->SAM_MEMREGION[region_sub_count] = region->node_id |
            (sam_encode_region_size(region->size) <<
                CMN600_HNF_SAM_MEMREGION_SIZE_POS) |
            ((region->base / SAM_GRANULARITY) <<
                CMN600_HNF_SAM_MEMREGION_BASE_POS) |
            CMN600_HNF_SAM_MEMREGION_VALID;

        region_sub_count++;
    }

    /* Configure the system cache RAM PPU */
    hnf->PPU_PWPR = CMN600_PPU_PWPR_POLICY_ON |
                    CMN600_PPU_PWPR_OPMODE_FAM |
                    CMN600_PPU_PWPR_DYN_EN;
}

/*
 * Scan the CMN600 to find out:
 * - Number of external RN-SAM nodes
 * - Number of internal RN-SAM nodes
 * - Number of HN-F nodes (cache)
 */
static void cmn600_discovery(void)
{
    unsigned int xp_count;
    unsigned int xp_idx;
    unsigned int node_count;
    unsigned int node_idx;
    struct cmn600_mxp_reg *xp;
    struct node_header *node;
    const struct mod_cmn600_config *config = ctx->config;

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, MOD_NAME "Starting discovery...\n");

    assert(get_node_type(ctx->root) == NODE_TYPE_CFG);

    /* Traverse cross points (XP) */
    xp_count = get_node_child_count(ctx->root);
    for (xp_idx = 0; xp_idx < xp_count; xp_idx++) {

        xp = get_child_node(config->base, ctx->root, xp_idx);
        assert(get_node_type(xp) == NODE_TYPE_XP);

        ctx->log_api->log(MOD_LOG_GROUP_DEBUG, MOD_NAME "\n");
        ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
            MOD_NAME "XP (%d, %d) ID:%d, LID:%d\n",
            get_node_pos_x(xp),
            get_node_pos_y(xp),
            get_node_id(xp),
            get_node_logical_id(xp));

        /* Traverse nodes */
        node_count = get_node_child_count(xp);
        for (node_idx = 0; node_idx < node_count; node_idx++) {

            node = get_child_node(config->base, xp, node_idx);

            /* External nodes */
            if (is_child_external(xp, node_idx)) {
                ctx->external_rnsam_count++;
                ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
                    MOD_NAME "  Found external node ID:%d\n",
                    get_child_node_id(xp, node_idx));

            /* Internal nodes */
            } else {
                switch (get_node_type(node)) {
                case NODE_TYPE_HN_F:
                    ctx->hnf_count++;
                    break;

                case NODE_TYPE_RN_SAM:
                    ctx->internal_rnsam_count++;
                    break;

                default:
                    /* Nothing to be done for other node types */
                    break;
                }

                ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
                    MOD_NAME "  %s ID:%d, LID:%d\n",
                    get_node_type_name(get_node_type(node)),
                    get_node_id(node),
                    get_node_logical_id(node));
            }
        }
    }

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Total internal RN-SAM nodes: %d\n"
        MOD_NAME "Total external RN-SAM nodes: %d\n"
        MOD_NAME "Total HN-F nodes: %d\n",
        ctx->internal_rnsam_count,
        ctx->external_rnsam_count,
        ctx->hnf_count);
}

static void cmn600_configure(void)
{
    unsigned int xp_count;
    unsigned int xp_idx;
    unsigned int node_count;
    unsigned int node_idx;
    unsigned int xrnsam_entry;
    unsigned int irnsam_entry;
    struct cmn600_mxp_reg *xp;
    void *node;
    enum node_type node_type;
    const struct mod_cmn600_config *config = ctx->config;

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
            node_type = get_node_type(node);

            if (node_type == NODE_TYPE_RN_SAM) {
                if (is_child_external(xp, node_idx)) {
                    unsigned int node_id = get_child_node_id(xp, node_idx);

                    fwk_assert(xrnsam_entry < ctx->external_rnsam_count);

                    ctx->external_rnsam_table[xrnsam_entry].node_id = node_id;
                    ctx->external_rnsam_table[xrnsam_entry].node = node;

                    xrnsam_entry++;
                } else {
                    fwk_assert(irnsam_entry < ctx->internal_rnsam_count);

                    ctx->internal_rnsam_table[irnsam_entry] = node;

                    irnsam_entry++;
                }
            } else if (node_type == NODE_TYPE_HN_F)
                process_node_hnf(node);
        }
    }
}

static const char * const mmap_type_name[] = {
    [MOD_CMN600_MEMORY_REGION_TYPE_IO] = "I/O",
    [MOD_CMN600_MEMORY_REGION_TYPE_SYSCACHE] = "System Cache",
    [MOD_CMN600_REGION_TYPE_SYSCACHE_SUB] = "Sub-System Cache",
};

static int cmn600_setup_sam(struct cmn600_rnsam_reg *rnsam)
{
    unsigned int region_idx;
    unsigned int region_io_count = 0;
    const struct mod_cmn600_memory_region_map *region;
    const struct mod_cmn600_config *config = ctx->config;
    unsigned int bit_pos;
    unsigned int group;
    unsigned int group_count;

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Configuring SAM for node %d\n",
        get_node_id(rnsam));

    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        region = &config->mmap_table[region_idx];
        ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
            MOD_NAME "  [0x%lx - 0x%lx] %s\n",
            region->base,
            region->base + region->size - 1,
            mmap_type_name[region->type]);

        group = region_io_count / CMN600_RNSAM_REGION_ENTRIES_PER_GROUP;
        bit_pos = (region_io_count % CMN600_RNSAM_REGION_ENTRIES_PER_GROUP) *
                  CMN600_RNSAM_REGION_ENTRY_BITS_WIDTH;

        switch (region->type) {
        case MOD_CMN600_MEMORY_REGION_TYPE_IO:
            /*
             * Configure memory region
             */
            configure_region(&rnsam->NON_HASH_MEM_REGION[group],
                bit_pos,
                region->base,
                region->size,
                SAM_NODE_TYPE_HN_I);

            /*
             * Configure target node
             */
            group = region_io_count /
                CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP;
            bit_pos = CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
                      (region_io_count %
                       CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);

            rnsam->NON_HASH_TGT_NODEID[group] &=
                ~(CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK << bit_pos);
            rnsam->NON_HASH_TGT_NODEID[group] |= (region->node_id &
                CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK) << bit_pos;

            region_io_count++;
            break;

        case MOD_CMN600_MEMORY_REGION_TYPE_SYSCACHE:
            /*
             * Configure memory region
             */
            configure_region(&rnsam->SYS_CACHE_GRP_REGION[group],
                bit_pos,
                region->base,
                region->size,
                SAM_NODE_TYPE_HN_F);
           break;

        case MOD_CMN600_REGION_TYPE_SYSCACHE_SUB:
            /* Do nothing. System cache sub-regions are handled by HN-Fs */
            break;

        default:
            assert(false);
            return FWK_E_DATA;
        }
    }

    group_count = ctx->hnf_count / CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP;
    for (group = 0; group < group_count; group++)
        rnsam->SYS_CACHE_GRP_HN_NODEID[group] = ctx->hnf_cache_group[group];

    /* Program the number of HNFs */
    rnsam->SYS_CACHE_GRP_HN_COUNT = ctx->hnf_count;

    /* Enable RNSAM */
    rnsam->STATUS = CMN600_RNSAM_STATUS_UNSTALL;
    __sync_synchronize();

    return FWK_SUCCESS;
}

static int cmn600_setup(void)
{
    unsigned int rnsam_idx;

    if (!ctx->initialized) {
        cmn600_discovery();

        /*
         * Allocate resources based on the discovery
         */

        /* Pointers for the internal RN-SAM nodes */
        if (ctx->internal_rnsam_count != 0) {
            ctx->internal_rnsam_table = fwk_mm_calloc(
                ctx->internal_rnsam_count, sizeof(*ctx->internal_rnsam_table));
            if (ctx->internal_rnsam_table == NULL)
                return FWK_E_NOMEM;
        }

        /* Tuples for the external RN-RAM nodes (including their node IDs) */
        if (ctx->external_rnsam_count != 0) {
            ctx->external_rnsam_table = fwk_mm_calloc(
                ctx->external_rnsam_count, sizeof(*ctx->external_rnsam_table));
            if (ctx->external_rnsam_table == NULL)
                return FWK_E_NOMEM;
        }

        /* Cache groups */
        if (ctx->hnf_count != 0) {
            /*
             * Allocate enough group descriptors to accommodate all expected
             * HN-F nodes in the system.
             */
            ctx->hnf_cache_group = fwk_mm_calloc(
                ctx->hnf_count / CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP,
                sizeof(*ctx->hnf_cache_group));
            if (ctx->hnf_cache_group == NULL)
                return FWK_E_NOMEM;
        }
    }

    cmn600_configure();

    /* Setup internal RN-SAM nodes */
    for (rnsam_idx = 0; rnsam_idx < ctx->internal_rnsam_count; rnsam_idx++)
        cmn600_setup_sam(ctx->internal_rnsam_table[rnsam_idx]);

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, MOD_NAME "Done\n");

    ctx->initialized = true;

    return FWK_SUCCESS;
}

static int cmn600_setup_rnsam(unsigned int node_id)
{
    int status;
    unsigned int node_idx;

    status = fwk_module_check_call(FWK_ID_MODULE(FWK_MODULE_IDX_CMN600));
    if (status != FWK_SUCCESS)
        return status;

    for (node_idx = 0; node_idx < ctx->external_rnsam_count; node_idx++) {
        if (ctx->external_rnsam_table[node_idx].node_id == node_id) {
            cmn600_setup_sam(ctx->external_rnsam_table[node_idx].node);
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
    cmn600_setup_rnsam(*(unsigned int *)data);
}

static const struct mod_ppu_v1_power_state_observer_api cmn600_observer_api = {
    .post_ppu_on = post_ppu_on,
};

/*
 * Framework handlers
 */

static int cmn600_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    const struct mod_cmn600_config *config = data;

    /* No elements support */
    if (element_count > 0)
        return FWK_E_DATA;

    /* Allocate space for the context */
    ctx = fwk_mm_calloc(1, sizeof(*ctx));
    if (ctx == NULL)
        return FWK_E_NOMEM;

    if (config->base == 0)
        return FWK_E_DATA;

    if ((config->mesh_size_x == 0) || (config->mesh_size_x > CMN600_MESH_X_MAX))
        return FWK_E_DATA;

    if ((config->mesh_size_y == 0) || (config->mesh_size_y > CMN600_MESH_Y_MAX))
        return FWK_E_DATA;

    if (config->snf_count > CMN600_HNF_CACHE_GROUP_ENTRIES_MAX)
        return FWK_E_DATA;

    ctx->root = get_root_node(config->base, config->hnd_node_id,
        config->mesh_size_x, config->mesh_size_y);

    ctx->config = config;

    return FWK_SUCCESS;
}

static int cmn600_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /* Use second round only (round numbering is zero-indexed) */
    if (round == 1) {

        /* Bind to the log component */
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
                                 FWK_ID_API(FWK_MODULE_IDX_LOG, 0),
                                 &ctx->log_api);

        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int cmn600_process_bind_request(fwk_id_t requester_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    *api = &cmn600_observer_api;
    return FWK_SUCCESS;
}

int cmn600_start(fwk_id_t id)
{
    if (fwk_id_is_equal(ctx->config->clock_id, FWK_ID_NONE)) {
        cmn600_setup();
        return FWK_SUCCESS;
    }

    /* Register the module for clock state notifications */
    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        ctx->config->clock_id,
        id);
}

static int cmn600_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    assert(fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING)
        cmn600_setup();

    return FWK_SUCCESS;
}

const struct fwk_module module_cmn600 = {
    .name = "CMN600",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_CMN600_API_COUNT,
    .init = cmn600_init,
    .bind = cmn600_bind,
    .start = cmn600_start,
    .process_bind_request = cmn600_process_bind_request,
    .process_notification = cmn600_process_notification,
};
