/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmn600.h>

#include <internal/cmn600_ccix.h>
#include <internal/cmn600_ctx.h>

#include <mod_clock.h>
#include <mod_cmn600.h>
#include <mod_ppu_v1.h>
#include <mod_system_info.h>
#include <mod_timer.h>

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
#include <stdbool.h>
#include <string.h>

#define MOD_NAME "[CMN600] "

struct cmn600_ctx *ctx;

/* Chip information API */
struct mod_system_info_get_info_api *system_info_api;

static void process_node_hnf(struct cmn600_hnf_reg *hnf)
{
    unsigned int logical_id;
    unsigned int node_id;
    unsigned int group;
    unsigned int bit_pos;
    unsigned int region_idx;
    unsigned int region_sub_count = 0;
    const struct mod_cmn600_memory_region_map *region;
    const struct mod_cmn600_config *config = ctx->config;
    static unsigned int cal_mode_factor = 1;
    uint64_t base_offset;

    logical_id = get_node_logical_id(hnf);
    node_id = get_node_id(hnf);

    /*
     * If CAL mode is set, only even numbered hnf node should be added to the
     * sys_cache_grp_hn_nodeid registers and hnf_count should be incremented
     * only for the even numbered hnf nodes.
     */
    if (config->hnf_cal_mode == true && (node_id % 2 == 1) &&
        is_cal_mode_supported(ctx->root)) {

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
        (CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP * cal_mode_factor);
    bit_pos = (CMN600_HNF_CACHE_GROUP_ENTRY_BITS_WIDTH / cal_mode_factor) *
        (logical_id % (CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP *
                       cal_mode_factor));

    /*
     * If CAL mode is set, add only even numbered hnd node to
     * sys_cache_grp_hn_nodeid registers
     */
    if (config->hnf_cal_mode == true && is_cal_mode_supported(ctx->root)) {
        if (node_id % 2 == 0)
            ctx->hnf_cache_group[group] += ((uint64_t)get_node_id(hnf)) <<
                                           bit_pos;
    } else
        ctx->hnf_cache_group[group] += ((uint64_t)get_node_id(hnf)) << bit_pos;

    /* Set target node */
    hnf->SAM_CONTROL = config->snf_table[logical_id];

    if (ctx->chip_id != 0) {
        base_offset = ((uint64_t)(ctx->config->chip_addr_space *
                                  ctx->chip_id));
    } else
        base_offset = 0;

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
            (((region->base + base_offset) / SAM_GRANULARITY) <<
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
static int cmn600_discovery(void)
{
    unsigned int xp_count;
    unsigned int xp_idx;
    unsigned int node_count;
    unsigned int node_idx;
    bool xp_port;
    struct cmn600_mxp_reg *xp;
    struct node_header *node;
    const struct mod_cmn600_config *config = ctx->config;

    FWK_LOG_INFO(MOD_NAME "Starting discovery...");

    assert(get_node_type(ctx->root) == NODE_TYPE_CFG);

    /* Traverse cross points (XP) */
    xp_count = get_node_child_count(ctx->root);
    for (xp_idx = 0; xp_idx < xp_count; xp_idx++) {

        xp = get_child_node(config->base, ctx->root, xp_idx);
        assert(get_node_type(xp) == NODE_TYPE_XP);

        FWK_LOG_INFO(MOD_NAME);

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

            /* External nodes */
            if (is_child_external(xp, node_idx)) {
                xp_port = get_port_number(get_child_node_id(xp, node_idx));

                /*
                 * If the device type is CXRH, CXHA, or CXRA, then the external
                 * child node is CXLA as every CXRH, CXHA, or CXRA node has a
                 * corresponding external CXLA node.
                 */
                if ((get_device_type(xp, xp_port) == DEVICE_TYPE_CXRH) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXHA) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXRA)) {
                    ctx->cxla_reg = (void *)node;
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
                    ctx->hnf_offset[ctx->hnf_count++] = (uint32_t)node;
                    break;

                case NODE_TYPE_RN_SAM:
                    ctx->internal_rnsam_count++;
                    break;

                case NODE_TYPE_RN_D:
                    if (ctx->rnd_count >= MAX_RND_COUNT) {
                        FWK_LOG_INFO(
                            MOD_NAME "  rnd count %d >= max limit (%d)",
                            ctx->rnd_count,
                            MAX_RND_COUNT);
                        return FWK_E_DATA;
                    }
                    ctx->rnd_ldid[ctx->rnd_count++] = get_node_logical_id(node);
                    break;

                case NODE_TYPE_RN_I:
                    if (ctx->rni_count >= MAX_RNI_COUNT) {
                        FWK_LOG_INFO(
                            MOD_NAME "  rni count %d >= max limit (%d)",
                            ctx->rni_count,
                            MAX_RNI_COUNT);
                        return FWK_E_DATA;
                    }
                    ctx->rni_ldid[ctx->rni_count++] = get_node_logical_id(node);
                    break;

                case NODE_TYPE_CXRA:
                    ctx->cxg_ra_reg = (struct cmn600_cxg_ra_reg *)node;
                    break;

                case NODE_TYPE_CXHA:
                    ctx->cxg_ha_reg = (struct cmn600_cxg_ha_reg *)node;
                    ctx->ccix_host_info.host_ha_count++;
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

    /*
     * RN-F nodes does not have node type identifier and hence the count cannot
     * be determined during the discovery process. RN-F count will be total
     * RN-SAM count minus the total RN-D, RN-I and CXHA count combined.
     */
    ctx->rnf_count = ctx->internal_rnsam_count + ctx->external_rnsam_count -
        (ctx->rnd_count + ctx->rni_count + ctx->ccix_host_info.host_ha_count);

    if (ctx->rnf_count > MAX_RNF_COUNT) {
        FWK_LOG_ERR(
            MOD_NAME "rnf count %d > max limit (%d)",
            ctx->rnf_count,
            MAX_RNF_COUNT);
        return FWK_E_RANGE;
    }

    /* When CAL is present, the number of HN-Fs must be even. */
    if ((ctx->hnf_count % 2 != 0) && (config->hnf_cal_mode == true)) {
        FWK_LOG_ERR(
            MOD_NAME "hnf count: %d should be even when CAL mode is set",
            ctx->hnf_count);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO(
        MOD_NAME "Total internal RN-SAM nodes: %d", ctx->internal_rnsam_count);
    FWK_LOG_INFO(
        MOD_NAME "Total external RN-SAM nodes: %d", ctx->external_rnsam_count);
    FWK_LOG_INFO(MOD_NAME "Total HN-F nodes: %d", ctx->hnf_count);
    FWK_LOG_INFO(MOD_NAME "Total RN-F nodes: %d", ctx->rnd_count);
    FWK_LOG_INFO(MOD_NAME "Total RN-D nodes: %d", ctx->rnf_count);
    FWK_LOG_INFO(MOD_NAME "Total RN-I nodes: %d", ctx->rni_count);

    if (ctx->cxla_reg) {
        FWK_LOG_INFO(MOD_NAME "CCIX CXLA node at: 0x%p", (void *)ctx->cxla_reg);
    }
    if (ctx->cxg_ra_reg) {
        FWK_LOG_INFO(
            MOD_NAME "CCIX CXRA node at: 0x%p", (void *)ctx->cxg_ra_reg);
    }
    if (ctx->cxg_ha_reg) {
        FWK_LOG_INFO(
            MOD_NAME "CCIX CXHA node at: 0x%p", (void *)ctx->cxg_ha_reg);
    }
    return FWK_SUCCESS;
}

static void cmn600_configure(void)
{
    unsigned int xp_count;
    unsigned int xp_idx;
    bool xp_port;
    unsigned int node_count;
    unsigned int node_idx;
    unsigned int xrnsam_entry;
    unsigned int irnsam_entry;
    struct cmn600_mxp_reg *xp;
    void *node;
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
    [MOD_CMN600_MEMORY_REGION_TYPE_IO] = "I/O",
    [MOD_CMN600_MEMORY_REGION_TYPE_SYSCACHE] = "System Cache",
    [MOD_CMN600_REGION_TYPE_SYSCACHE_SUB] = "Sub-System Cache",
    [MOD_CMN600_REGION_TYPE_CCIX] = "CCIX",
    [MOD_CMN600_REGION_TYPE_SYSCACHE_NONHASH] = "System Cache Non-hash",
};

int cmn600_setup_sam(struct cmn600_rnsam_reg *rnsam)
{
    unsigned int region_idx;
    unsigned int region_io_count = 0;
    unsigned int region_sys_count = 0;
    const struct mod_cmn600_memory_region_map *region;
    const struct mod_cmn600_config *config = ctx->config;
    unsigned int bit_pos;
    unsigned int group;
    unsigned int group_count;
    enum sam_node_type sam_node_type;
    uint64_t base;
    unsigned int scg_region = 0;
    unsigned int scg_regions_enabled[CMN600_MAX_NUM_SCG] = {0, 0, 0, 0};

    FWK_LOG_INFO(MOD_NAME "Configuring SAM for node %d", get_node_id(rnsam));

    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        region = &config->mmap_table[region_idx];

        if (ctx->chip_id != 0) {
            if (region->type == MOD_CMN600_REGION_TYPE_CCIX)
                base = 0;
            else if (region->type == MOD_CMN600_MEMORY_REGION_TYPE_SYSCACHE)
                base = region->base;
            else {
                base = ((uint64_t)(ctx->config->chip_addr_space *
                                   ctx->chip_id) + region->base);
            }
        } else
            base = region->base;

        FWK_LOG_INFO(
            MOD_NAME "  [0x%" PRIX64 " - 0x%" PRIX64 "] %s",
            base,
            base + region->size - 1,
            mmap_type_name[region->type]);

        switch (region->type) {
        case MOD_CMN600_MEMORY_REGION_TYPE_IO:
        case MOD_CMN600_REGION_TYPE_CCIX:
            /*
             * Configure memory region
             */
            if (region_io_count >
                    CMN600_RNSAM_MAX_NON_HASH_MEM_REGION_ENTRIES) {
                FWK_LOG_ERR(
                    MOD_NAME
                    "Non-Hashed Memory can have maximum of %d regions only",
                    CMN600_RNSAM_MAX_NON_HASH_MEM_REGION_ENTRIES);
                return FWK_E_DATA;
            }

            group = region_io_count / CMN600_RNSAM_REGION_ENTRIES_PER_GROUP;
            bit_pos = (region_io_count %
                       CMN600_RNSAM_REGION_ENTRIES_PER_GROUP) *
                      CMN600_RNSAM_REGION_ENTRY_BITS_WIDTH;

            sam_node_type =
                (region->type == MOD_CMN600_MEMORY_REGION_TYPE_IO) ?
                    SAM_NODE_TYPE_HN_I : SAM_NODE_TYPE_CXRA;
            configure_region(&rnsam->NON_HASH_MEM_REGION[group],
                             bit_pos,
                             base,
                             region->size,
                             sam_node_type);
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
            if (region_sys_count >= CMN600_RNSAM_MAX_HASH_MEM_REGION_ENTRIES) {
                FWK_LOG_ERR(
                    MOD_NAME
                    "Hashed Memory can have maximum of %d regions only",
                    CMN600_RNSAM_MAX_HASH_MEM_REGION_ENTRIES);
                return FWK_E_DATA;
            }

            group = region_sys_count / CMN600_RNSAM_REGION_ENTRIES_PER_GROUP;
            bit_pos = (region_sys_count %
                       CMN600_RNSAM_REGION_ENTRIES_PER_GROUP) *
                      CMN600_RNSAM_REGION_ENTRY_BITS_WIDTH;
            configure_region(&rnsam->SYS_CACHE_GRP_REGION[group],
                             bit_pos,
                             region->base,
                             region->size,
                             SAM_NODE_TYPE_HN_F);

            /* Mark corresponding region as enabled */
            scg_region = (2 * group) + (bit_pos/32);
            fwk_assert(scg_region < CMN600_MAX_NUM_SCG);
            scg_regions_enabled[scg_region] = 1;

            region_sys_count++;
            break;

        case MOD_CMN600_REGION_TYPE_SYSCACHE_NONHASH:
            group = region_sys_count / CMN600_RNSAM_REGION_ENTRIES_PER_GROUP;
            bit_pos = (region_sys_count %
                       CMN600_RNSAM_REGION_ENTRIES_PER_GROUP) *
                      CMN600_RNSAM_REGION_ENTRY_BITS_WIDTH;
            /*
             * Configure memory region
             */
            configure_region(&rnsam->SYS_CACHE_GRP_REGION[group],
                             bit_pos,
                             region->base,
                             region->size,
                             SAM_NODE_TYPE_HN_I);

            rnsam->SYS_CACHE_GRP_REGION[group] |= (UINT64_C(0x2) << bit_pos);
            bit_pos = CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
                  ((region_sys_count - 1) %
                  CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);
            rnsam->SYS_CACHE_GRP_NOHASH_NODEID &=
                ~(CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK << bit_pos);
            rnsam->SYS_CACHE_GRP_NOHASH_NODEID |= (region->node_id &
                CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK) << bit_pos;

            region_sys_count++;
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

    /* Use CAL mode only if the CMN600 revision is r2p0 or above */
    if (is_cal_mode_supported(ctx->root) && config->hnf_cal_mode) {
        for (region_idx = 0; region_idx < CMN600_MAX_NUM_SCG; region_idx++)
            rnsam->SYS_CACHE_GRP_CAL_MODE = scg_regions_enabled[region_idx] *
                (CMN600_RNSAM_SCG_HNF_CAL_MODE_EN <<
                 (region_idx * CMN600_RNSAM_SCG_HNF_CAL_MODE_SHIFT));
    }

    /* Enable RNSAM */
    rnsam->STATUS = CMN600_RNSAM_STATUS_UNSTALL;
    __sync_synchronize();

    return FWK_SUCCESS;
}

static int cmn600_setup(void)
{
    unsigned int rnsam_idx, i, ccix_mmap_idx;
    int status = FWK_SUCCESS;

    if (!ctx->initialized) {
        status = cmn600_discovery();
        if (status != FWK_SUCCESS)
            return status;
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
                ctx->hnf_count / CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP,
                sizeof(*ctx->hnf_cache_group));
        }
    }

    cmn600_configure();

    /* Setup internal RN-SAM nodes */
    for (rnsam_idx = 0; rnsam_idx < ctx->internal_rnsam_count; rnsam_idx++)
        cmn600_setup_sam(ctx->internal_rnsam_table[rnsam_idx]);

    /* Capture CCIX Host Topology */
    for (i = 0; i < ctx->config->mmap_count; i++) {
        if (ctx->config->mmap_table[i].type == MOD_CMN600_REGION_TYPE_CCIX) {
            ccix_mmap_idx = ctx->ccix_host_info.ccix_host_mmap_count;
            if (ccix_mmap_idx >= MAX_HA_MMAP_ENTRIES)
                return FWK_E_DATA;

            ctx->ccix_host_info.ccix_host_mmap[ccix_mmap_idx].base =
                ctx->config->mmap_table[i].base;
            ctx->ccix_host_info.ccix_host_mmap[ccix_mmap_idx].size =
                ctx->config->mmap_table[i].size;
            ctx->ccix_host_info.ccix_host_mmap_count++;
        }
    }

    FWK_LOG_INFO(MOD_NAME "Done");

    ctx->initialized = true;

    return FWK_SUCCESS;
}

static int cmn600_setup_rnsam(unsigned int node_id)
{
    unsigned int node_idx;

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
 * CCIX configuration APIs invoked by SCMI
 */

static int cmn600_ccix_config_get(
    struct mod_cmn600_ccix_host_node_config *config)
{
    if (ctx->internal_rnsam_count == 0)
        return FWK_E_DATA;

    ctx->ccix_host_info.host_ra_count =
        ctx->internal_rnsam_count + ctx->external_rnsam_count;
    ctx->ccix_host_info.host_sa_count = ctx->config->sa_count;

    ccix_capabilities_get(ctx);

    memcpy((void *)config, (void *)&ctx->ccix_host_info,
        sizeof(struct mod_cmn600_ccix_host_node_config));
    return FWK_SUCCESS;
}


static int cmn600_ccix_config_set(
    struct mod_cmn600_ccix_remote_node_config *config)
{
    unsigned int i;
    int status;


    status = ccix_setup(ctx, config);
    if (status != FWK_SUCCESS)
        return status;

    for (i = 0; i < ctx->config->mmap_count; i++) {
        if (ctx->config->mmap_table[i].type == MOD_CMN600_REGION_TYPE_CCIX)
            cmn600_setup_rnsam(ctx->config->mmap_table[i].node_id);
    }
    return FWK_SUCCESS;
}

static int cmn600_ccix_exchange_protocol_credit(uint8_t link_id)
{
    return ccix_exchange_protocol_credit(ctx, link_id);
}

static int cmn600_ccix_enter_system_coherency(uint8_t link_id)
{
    return ccix_enter_system_coherency(ctx, link_id);
}

static int cmn600_ccix_enter_dvm_domain(uint8_t link_id)
{
    return ccix_enter_dvm_domain(ctx, link_id);
}

static const struct mod_cmn600_ccix_config_api cmn600_ccix_config_api = {
    .get_config = cmn600_ccix_config_get,
    .set_config = cmn600_ccix_config_set,
    .exchange_protocol_credit = cmn600_ccix_exchange_protocol_credit,
    .enter_system_coherency = cmn600_ccix_enter_system_coherency,
    .enter_dvm_domain = cmn600_ccix_enter_dvm_domain,
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

        /* Bind to the timer component */
        status = fwk_module_bind(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                 FWK_ID_API(FWK_MODULE_IDX_TIMER,
                                            MOD_TIMER_API_IDX_TIMER),
                                 &ctx->timer_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        /* Bind to system info module to obtain multi-chip info */
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SYSTEM_INFO),
                                 FWK_ID_API(FWK_MODULE_IDX_SYSTEM_INFO,
                                            MOD_SYSTEM_INFO_GET_API_IDX),
                                 &system_info_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int cmn600_process_bind_request(fwk_id_t requester_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_CMN600_API_IDX_PPU_OBSERVER:
        *api = &cmn600_observer_api;
        break;

    case MOD_CMN600_API_IDX_CCIX_CONFIG:
        *api = &cmn600_ccix_config_api;
        break;
    }

    return FWK_SUCCESS;
}

int cmn600_start(fwk_id_t id)
{
    uint8_t chip_id = 0;
    bool mc_mode = false;
    int status;

    if (fwk_id_is_equal(ctx->config->clock_id, FWK_ID_NONE)) {
        cmn600_setup();
        return FWK_SUCCESS;
    }

    status = system_info_api->get_system_info(&ctx->system_info);
    if (status == FWK_SUCCESS) {
        chip_id = ctx->system_info->chip_id;
        mc_mode = ctx->system_info->multi_chip_mode;
    }

    ctx->chip_id = chip_id;

    FWK_LOG_INFO(MOD_NAME "Multichip mode: %s", mc_mode ? "yes" : "no");
    FWK_LOG_INFO(MOD_NAME "Chip ID: %d", chip_id);

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
