/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmn700.h>
#include <cmn700_ccg.h>

#include <internal/cmn700_ctx.h>

#include <mod_clock.h>
#include <mod_cmn700.h>
#include <mod_system_info.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_math.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdint.h>

#define MOD_NAME "[CMN700] "

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
static const char *const mmap_type_name[] = {
    [MOD_CMN700_MEM_REGION_TYPE_IO] = "I/O",
    [MOD_CMN700_MEM_REGION_TYPE_SYSCACHE] = "System Cache",
    [MOD_CMN700_REGION_TYPE_SYSCACHE_SUB] = "Sub-System Cache",
    [MOD_CMN700_REGION_TYPE_CCG] = "CCG",
};
#endif

static struct cmn700_device_ctx *ctx;

/* Chip Information */
static struct mod_system_info_get_info_api *system_info_api;
static const struct mod_system_info *system_info;

/* Initialize default for multi-chip mode and chip-id */
static unsigned int chip_id;
static bool multi_chip_mode;

static struct node_pos *hnf_node_pos;

static inline size_t cmn700_hnf_cache_group_count(size_t hnf_count)
{
    return (hnf_count + CMN700_HNF_CACHE_GROUP_ENTRIES_PER_GROUP - 1) /
        CMN700_HNF_CACHE_GROUP_ENTRIES_PER_GROUP;
}

static void process_node_hnf(struct cmn700_hnf_reg *hnf)
{
    unsigned int logical_id;
    unsigned int region_idx;
    unsigned int region_sub_count = 0;
    unsigned int hnf_count_per_cluster;
    unsigned int hnf_cluster_index;
    unsigned int snf_count_per_cluster;
    unsigned int snf_idx_in_cluster;
    unsigned int snf_table_idx;
    unsigned int top_address_bit0, top_address_bit1;
    enum mod_cmn700_hnf_to_snf_mem_strip_mode sn_mode;
    uint64_t base;
    uint64_t sam_control;
    const struct mod_cmn700_mem_region_map *region;
    const struct mod_cmn700_config *config = ctx->config;
    const struct mod_cmn700_hierarchical_hashing *hier_hash_cfg;

    logical_id = get_node_logical_id(hnf);

    hier_hash_cfg = &(config->hierarchical_hashing_config);

    /* SN mode with Hierarchical Hashing */
    if (config->hierarchical_hashing_enable && hier_hash_cfg->sn_mode) {
        sn_mode = hier_hash_cfg->sn_mode;
        top_address_bit0 = hier_hash_cfg->top_address_bit0;
        top_address_bit1 = hier_hash_cfg->top_address_bit1;

        snf_count_per_cluster =
            config->snf_count / hier_hash_cfg->hnf_cluster_count;

        /*
         * For now, 3-SN mode (three SN-Fs per cluster) is supported, other
         * modes are not tested.
         */
        fwk_assert(snf_count_per_cluster == 3);

        /* Number of HN-Fs in a cluster */
        hnf_count_per_cluster =
            ctx->hnf_count / hier_hash_cfg->hnf_cluster_count;

        /* Choose the cluster idx based on the HN-Fs LDID value */
        hnf_cluster_index = logical_id / hnf_count_per_cluster;

        if (top_address_bit1 <= top_address_bit0) {
            FWK_LOG_ERR(
                MOD_NAME
                "top_address_bit1: %d should be greater than top_address_bit0: "
                "%d",
                top_address_bit1,
                top_address_bit0);
            fwk_unexpected();
        }

        sam_control =
            ((UINT64_C(1) << CMN700_HNF_SAM_CONTROL_SN_MODE_POS(sn_mode)) |
             ((uint64_t)top_address_bit0
              << CMN700_HNF_SAM_CONTROL_TOP_ADDR_BIT0_POS) |
             ((uint64_t)top_address_bit1
              << CMN700_HNF_SAM_CONTROL_TOP_ADDR_BIT1_POS));

        for (snf_idx_in_cluster = 0; snf_idx_in_cluster < snf_count_per_cluster;
             snf_idx_in_cluster++) {
            snf_table_idx = (hnf_cluster_index * snf_count_per_cluster) +
                snf_idx_in_cluster;
            sam_control |=
                ((uint64_t)config->snf_table[snf_table_idx]
                 << CMN700_HNF_SAM_CONTROL_SN_NODE_ID_POS(snf_idx_in_cluster));
        }

        hnf->SAM_CONTROL = sam_control;
    } else {
        fwk_assert(logical_id < config->snf_count);

        /* Set target node */
        hnf->SAM_CONTROL = config->snf_table[logical_id];
    }

    /*
     * Map sub-regions to this HN-F node
     */
    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        region = &config->mmap_table[region_idx];

        /* Skip non sub-regions */
        if (region->type != MOD_CMN700_REGION_TYPE_SYSCACHE_SUB)
            continue;

        /* Offset the base with chip address space base on chip-id */
        base =
            ((uint64_t)(ctx->config->chip_addr_space * chip_id)) + region->base;

        /* Configure sub-region entry */
        if (get_hnsam_range_comp_en_mode(hnf)) {
            hnf->SAM_MEMREGION[region_sub_count] = region->node_id |
                ((base / SAM_GRANULARITY)
                 << CMN700_HNF_SAM_MEMREGION_BASE_POS) |
                CMN700_HNF_SAM_MEMREGION_VALID;
            hnf->SAM_MEMREGION_END_ADDR[region_sub_count] =
                ((base + region->size - 1));
        } else {
            hnf->SAM_MEMREGION[region_sub_count] = region->node_id |
                (sam_encode_region_size(region->size)
                 << CMN700_HNF_SAM_MEMREGION_SIZE_POS) |
                ((base / SAM_GRANULARITY)
                 << CMN700_HNF_SAM_MEMREGION_BASE_POS) |
                CMN700_HNF_SAM_MEMREGION_VALID;
        }
        region_sub_count++;
    }

    /* Configure the system cache RAM PPU */
    hnf->PPU_PWPR = CMN700_PPU_PWPR_POLICY_ON | CMN700_PPU_PWPR_OPMODE_FAM |
        CMN700_PPU_PWPR_DYN_EN;
}

/*
 * Discover the topology of the interconnect and identify the number of:
 * - External RN-SAM nodes
 * - Internal RN-SAM nodes
 * - HN-F nodes (cache)
 */
static int cmn700_discovery(void)
{
    unsigned int ccg_ra_reg_count;
    unsigned int ccg_ha_reg_count;
    unsigned int ccla_reg_count;
    unsigned int cxg_ra_reg_count;
    unsigned int cxg_ha_reg_count;
    unsigned int cxla_reg_count;
    unsigned int node_count;
    unsigned int node_idx;
    unsigned int xp_count;
    unsigned int xp_idx;
    unsigned int xp_port;
    struct cmn700_mxp_reg *xp;
    struct node_header *node;
    const struct mod_cmn700_config *config = ctx->config;

    ccg_ra_reg_count = 0;
    ccg_ha_reg_count = 0;
    ccla_reg_count = 0;
    cxg_ra_reg_count = 0;
    cxg_ha_reg_count = 0;
    cxla_reg_count = 0;

    set_encoding_and_masking_bits(config);

    FWK_LOG_INFO(
        MOD_NAME "CMN-700 revision: %s", get_cmn700_revision_name(ctx->root));
    FWK_LOG_INFO(MOD_NAME "Starting discovery...");
    FWK_LOG_DEBUG(MOD_NAME "Rootnode Base address: 0x%x", (uintptr_t)ctx->root);

    fwk_assert(get_node_type(ctx->root) == NODE_TYPE_CFG);

    /* Traverse cross points (XP) */
    xp_count = get_node_child_count(ctx->root);
    for (xp_idx = 0; xp_idx < xp_count; xp_idx++) {
        xp = get_child_node(config->base, ctx->root, xp_idx);
        fwk_assert(get_node_type(xp) == NODE_TYPE_XP);

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
                xp_port = get_port_number(
                    get_child_node_id(xp, node_idx),
                    get_node_device_port_count(xp));

                /*
                 * If the device type is CXRH, CXHA, or CXRA, then the external
                 * child node is CXLA as every CXRH, CXHA, or CXRA node has a
                 * corresponding external CXLA node.
                 */
                if ((get_device_type(xp, xp_port) == DEVICE_TYPE_CXRH) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXHA) ||
                    (get_device_type(xp, xp_port) == DEVICE_TYPE_CXRA)) {
                    cxla_reg_count++;
                    FWK_LOG_INFO(
                        MOD_NAME "  Found CXLA at node ID: %d",
                        get_child_node_id(xp, node_idx));
                } else { /* External RN-SAM Node */
                    ctx->external_rnsam_count++;
                    FWK_LOG_INFO(
                        MOD_NAME "  Found external node ID: %d addr: %p",
                        get_child_node_id(xp, node_idx),
                        xp);
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

                    /*
                     * RN-F nodes does not have node type identifier and hence
                     * the count cannot be determined using the node type id.
                     * Alternatively, check if the device type connected to the
                     * Crosspoint (XP) is one of the RNF types and determine the
                     * RN-F count (if CAL connected RN-F, double the count).
                     */
                    xp_port = get_port_number(
                        get_node_id(node), get_node_device_port_count(xp));

                    if (is_device_type_rnf(xp, xp_port)) {
                        if (is_cal_connected(xp, xp_port)) {
                            ctx->rnf_count += 2;
                            FWK_LOG_INFO(
                                MOD_NAME
                                "  RN-F (CAL connected) found at port: %d",
                                xp_port);
                        } else {
                            ctx->rnf_count++;
                            FWK_LOG_INFO(
                                MOD_NAME "  RN-F found at port: %d", xp_port);
                        }
                    }
                    break;

                case NODE_TYPE_RN_D:
                    if ((ctx->rnd_count) >= MAX_RND_COUNT) {
                        FWK_LOG_ERR(
                            MOD_NAME "  rnd count %d >= max limit (%d)",
                            ctx->rnd_count,
                            MAX_RND_COUNT);
                        return FWK_E_DATA;
                    }
                    ctx->rnd_count++;
                    break;

                case NODE_TYPE_RN_I:
                    if ((ctx->rni_count) >= MAX_RNI_COUNT) {
                        FWK_LOG_ERR(
                            MOD_NAME "  rni count %d >= max limit (%d)",
                            ctx->rni_count,
                            MAX_RNI_COUNT);
                        return FWK_E_DATA;
                    }
                    ctx->rni_count++;
                    break;

                case NODE_TYPE_CCRA:
                    ccg_ra_reg_count++;
                    break;

                case NODE_TYPE_CCHA:
                    ccg_ha_reg_count++;
                    break;

                case NODE_TYPE_CCLA:
                    ccla_reg_count++;
                    break;

                case NODE_TYPE_CXRA:
                    cxg_ra_reg_count++;
                    break;

                case NODE_TYPE_CXHA:
                    cxg_ha_reg_count++;
                    break;

                /* CXLA should not be an internal node */
                case NODE_TYPE_CXLA:
                    FWK_LOG_ERR(MOD_NAME
                                "CXLA node should not be internal node, "
                                "discovery failed");
                    return FWK_E_DEVICE;
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

    ctx->ccg_node_count = ccg_ra_reg_count;

    if (ctx->rnf_count > MAX_RNF_COUNT) {
        FWK_LOG_ERR(
            MOD_NAME "rnf count %d > max limit (%d)",
            ctx->rnf_count,
            MAX_RNF_COUNT);
        return FWK_E_RANGE;
    }

    FWK_LOG_INFO(
        MOD_NAME "Total internal RN-SAM nodes: %d", ctx->internal_rnsam_count);
    FWK_LOG_INFO(
        MOD_NAME "Total external RN-SAM nodes: %d", ctx->external_rnsam_count);
    FWK_LOG_INFO(MOD_NAME "Total HN-F nodes: %d", ctx->hnf_count);
    FWK_LOG_INFO(MOD_NAME "Total RN-D nodes: %d", ctx->rnd_count);
    FWK_LOG_INFO(MOD_NAME "Total RN-F nodes: %d", ctx->rnf_count);
    FWK_LOG_INFO(MOD_NAME "Total RN-I nodes: %d", ctx->rni_count);
    FWK_LOG_INFO(
        MOD_NAME "Total CCIX Request Agent nodes: %d", cxg_ra_reg_count);
    FWK_LOG_INFO(MOD_NAME "Total CCIX Home Agent nodes: %d", cxg_ha_reg_count);
    FWK_LOG_INFO(MOD_NAME "Total CCIX Link Agent nodes: %d", cxla_reg_count);
    FWK_LOG_INFO(
        MOD_NAME "Total CCG Request Agent nodes: %d", ccg_ra_reg_count);
    FWK_LOG_INFO(MOD_NAME "Total CCG Home Agent nodes: %d", ccg_ha_reg_count);
    FWK_LOG_INFO(MOD_NAME "Total CCG Link Agent nodes: %d", ccla_reg_count);

    return FWK_SUCCESS;
}

static void cmn700_configure(void)
{
    unsigned int logical_id;
    unsigned int node_count;
    unsigned int ldid;
    unsigned int node_id;
    unsigned int node_idx;
    unsigned int xp_count;
    unsigned int xp_idx;
    unsigned int irnsam_entry;
    unsigned int xrnsam_entry;
    unsigned int xp_port;
    void *node;
    struct cmn700_mxp_reg *xp;
    const struct mod_cmn700_config *config = ctx->config;

    fwk_assert(get_node_type(ctx->root) == NODE_TYPE_CFG);

    irnsam_entry = 0;
    xrnsam_entry = 0;

    /* Traverse cross points (XP) */
    xp_count = get_node_child_count(ctx->root);
    for (xp_idx = 0; xp_idx < xp_count; xp_idx++) {
        xp = get_child_node(config->base, ctx->root, xp_idx);
        fwk_assert(get_node_type(xp) == NODE_TYPE_XP);

        /* Traverse nodes */
        node_count = get_node_child_count(xp);
        for (node_idx = 0; node_idx < node_count; node_idx++) {
            node = get_child_node(config->base, xp, node_idx);
            xp_port = get_port_number(
                get_child_node_id(xp, node_idx),
                get_node_device_port_count(xp));
            if (is_child_external(xp, node_idx)) {
                node_id = get_child_node_id(xp, node_idx);

                if (!(get_device_type(xp, xp_port) == DEVICE_TYPE_CXRH) &&
                    !(get_device_type(xp, xp_port) == DEVICE_TYPE_CXHA) &&
                    !(get_device_type(xp, xp_port) == DEVICE_TYPE_CXRA)) {
                    fwk_assert(xrnsam_entry < ctx->external_rnsam_count);

                    ctx->external_rnsam_table[xrnsam_entry].node_id = node_id;
                    ctx->external_rnsam_table[xrnsam_entry].node = node;

                    xrnsam_entry++;
                }
            } else {
                enum node_type node_type = get_node_type(node);
                node_id = get_node_id(node);
                if (node_type == NODE_TYPE_RN_SAM) {
                    fwk_assert(irnsam_entry < ctx->internal_rnsam_count);

                    ctx->internal_rnsam_table[irnsam_entry] = node;

                    irnsam_entry++;
                } else if (node_type == NODE_TYPE_CCRA) {
                    ldid = get_node_logical_id(node);
                    fwk_assert(ldid < ctx->ccg_node_count);

                    /* Use ldid as index of the ccg_ra table */
                    ctx->ccg_ra_reg_table[ldid].node_id = node_id;
                    ctx->ccg_ra_reg_table[ldid].ccg_ra_reg =
                        (struct cmn700_ccg_ra_reg *)node;
                } else if (node_type == NODE_TYPE_CCHA) {
                    ldid = get_node_logical_id(node);
                    fwk_assert(ldid < ctx->ccg_node_count);

                    /* Use ldid as index of the ccg_ra table */
                    ctx->ccg_ha_reg_table[ldid].node_id = node_id;
                    ctx->ccg_ha_reg_table[ldid].ccg_ha_reg =
                        (struct cmn700_ccg_ha_reg *)node;
                } else if (node_type == NODE_TYPE_CCLA) {
                    ldid = get_node_logical_id(node);

                    /* Use ldid as index of the ccla table */
                    ctx->ccla_reg_table[ldid].node_id = node_id;
                    ctx->ccla_reg_table[ldid].ccla_reg =
                        (struct cmn700_ccla_reg *)node;
                } else if (node_type == NODE_TYPE_HN_F) {
                    logical_id = get_node_logical_id(node);
                    fwk_assert(logical_id < ctx->hnf_count);

                    ctx->hnf_node[logical_id] = (uintptr_t)(void *)node;

                    hnf_node_pos[logical_id].pos_x = get_node_pos_x(node);
                    hnf_node_pos[logical_id].pos_y = get_node_pos_y(node);
                    hnf_node_pos[logical_id].port_num =
                        get_port_number(node_id, xp_port);

                    process_node_hnf(node);
                }
            }
        }
    }
}

/* Helper function to check if hnf is inside the SCG/HTG square/rectangle */
bool is_hnf_inside_rect(
    struct node_pos hnf_node_pos,
    const struct mod_cmn700_mem_region_map *region)
{
    struct node_pos region_hnf_pos_start;
    struct node_pos region_hnf_pos_end;

    region_hnf_pos_start = region->hnf_pos_start;
    region_hnf_pos_end = region->hnf_pos_end;

    if (((hnf_node_pos.pos_x >= region_hnf_pos_start.pos_x) &&
         (hnf_node_pos.pos_y >= region_hnf_pos_start.pos_y) &&
         (hnf_node_pos.pos_x <= region_hnf_pos_end.pos_x) &&
         (hnf_node_pos.pos_y <= region_hnf_pos_end.pos_y) &&
         (hnf_node_pos.port_num <= region_hnf_pos_end.port_num))) {
        if (hnf_node_pos.pos_y == region_hnf_pos_start.pos_y) {
            if (hnf_node_pos.port_num >= region_hnf_pos_start.port_num) {
                return true;
            } else {
                return false;
            }
        } else if (hnf_node_pos.pos_y == region_hnf_pos_end.pos_y) {
            if (hnf_node_pos.port_num <= region_hnf_pos_end.port_num) {
                return true;
            } else {
                return false;
            }
        }
        return true;
    }
    return false;
}

static void cmn700_setup_sys_cache_group_nodeid(
    struct cmn700_rnsam_reg *rnsam,
    const struct mod_cmn700_mem_region_map *region,
    uint32_t region_idx)
{
    uint8_t logical_id;
    uint32_t group;
    uint32_t cache_group_bit_position;
    uint32_t hnf_count_in_scg = 0;
    uint32_t hnf_nodeid;
    uint32_t hn_nodeid_reg_bits_idx = 0;
    const struct mod_cmn700_config *config = ctx->config;

    for (logical_id = 0; logical_id < ctx->hnf_count; logical_id++) {
        hnf_nodeid = get_node_id((void *)ctx->hnf_node[logical_id]);

        if ((config->hnf_cal_mode) && ((hnf_nodeid % 2) == 1)) {
            /* Ignore odd node ids if cal mode is set */
            continue;
        }

        group =
            hn_nodeid_reg_bits_idx / CMN700_HNF_CACHE_GROUP_ENTRIES_PER_GROUP;

        cache_group_bit_position = CMN700_HNF_CACHE_GROUP_ENTRY_BITS_WIDTH *
            ((hn_nodeid_reg_bits_idx %
              (CMN700_HNF_CACHE_GROUP_ENTRIES_PER_GROUP)));

        if (is_hnf_inside_rect(hnf_node_pos[logical_id], region)) {
            /*
             * If CAL mode is set, add only even numbered hnd node
             * to sys_cache_grp_hn_nodeid registers.
             */
            if (config->hnf_cal_mode && ((hnf_nodeid & 1) == 1)) {
                continue;
            }

            rnsam->SYS_CACHE_GRP_HN_NODEID[group] += (uint64_t)hnf_nodeid
                << cache_group_bit_position;
            rnsam->SYS_CACHE_GRP_SN_NODEID[group] +=
                ((uint64_t)config->snf_table[logical_id])
                << cache_group_bit_position;
            hnf_count_in_scg++;
            hn_nodeid_reg_bits_idx++;
        }
    }

    rnsam->SYS_CACHE_GRP_HN_COUNT |= ((uint64_t)hnf_count_in_scg)
        << CMN700_RNSAM_SYS_CACHE_GRP_HN_CNT_POS(region_idx);
}

static uint32_t get_region_index(enum mod_cmn700_mem_region_type region_type)
{
    uint32_t region_index;

    switch (region_type) {
    case MOD_CMN700_MEM_REGION_TYPE_IO:
        region_index = ctx->region_io_count++;
        break;

    case MOD_CMN700_MEM_REGION_TYPE_SYSCACHE:
        region_index = ctx->region_sys_count++;
        break;

    default:
        region_index = UINT32_MAX;
        break;
    };

    return region_index;
}

static void configure_target_node(
    const struct mod_cmn700_mem_region_map *region,
    struct cmn700_rnsam_reg *rnsam,
    uint32_t region_idx)
{
    uint32_t group;
    uint32_t bit_pos;

    group = region_idx / CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP;
    bit_pos = CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
        (region_idx % CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);

    rnsam->NON_HASH_TGT_NODEID[group] &=
        ~(CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK << bit_pos);
    rnsam->NON_HASH_TGT_NODEID[group] |=
        (region->node_id & CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK)
        << bit_pos;
}

static int cmn700_program_rnsam(const struct mod_cmn700_mem_region_map *region)
{
    uint8_t idx;
    uint64_t base;
    uint32_t region_idx;
    int status;
    struct cmn700_rnsam_reg *rnsam;

    /* Offset the base with chip address space base on chip-id */
    base = ((uint64_t)(ctx->config->chip_addr_space * chip_id) + region->base);

    status = FWK_SUCCESS;
    if (region->type == MOD_CMN700_REGION_TYPE_SYSCACHE_SUB) {
        /* System cache sub-regions are handled by HN-Fs */
        return status;
    }

    region_idx = get_region_index(region->type);
    if (region_idx == UINT32_MAX) {
        return FWK_E_PARAM;
    }

    for (idx = 0; idx < ctx->internal_rnsam_count; idx++) {
        rnsam = ctx->internal_rnsam_table[idx];
        switch (region->type) {
        case MOD_CMN700_MEM_REGION_TYPE_IO:
            configure_region(
                rnsam,
                region_idx,
                base,
                region->size,
                SAM_NODE_TYPE_HN_I,
                SAM_TYPE_NON_HASH_MEM_REGION);

            configure_target_node(region, rnsam, region_idx);
            break;

        case MOD_CMN700_MEM_REGION_TYPE_SYSCACHE:
            configure_region(
                rnsam,
                region_idx,
                region->base,
                region->size,
                SAM_NODE_TYPE_HN_F,
                SAM_TYPE_SYS_CACHE_GRP_REGION);

            /* Mark corresponding region as enabled */
            fwk_assert(region_idx < MAX_SCG_COUNT);
            ctx->scg_regions_enabled[region_idx] = 1;

            cmn700_setup_sys_cache_group_nodeid(rnsam, region, region_idx);

            break;

        default:
            fwk_unexpected();
            status = FWK_E_DATA;
            break;
        }

        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return status;
}

static int setup_internal_rn_sam_nodes(void)
{
    const struct mod_cmn700_config *config;
    uint32_t region_idx;
    int status;

    config = ctx->config;
    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        status = cmn700_program_rnsam(&config->mmap_table[region_idx]);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static void cmn700_print_region_info(void)
{
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    unsigned int idx;
    uint64_t base;
    const struct mod_cmn700_config *config;
    const struct mod_cmn700_mem_region_map *region;
    struct cmn700_rnsam_reg *rnsam;

    config = ctx->config;
    FWK_LOG_INFO(MOD_NAME "Regions to be mapped:");
    for (idx = 0; idx < config->mmap_count; idx++) {
        region = &config->mmap_table[idx];

        if (region->type == MOD_CMN700_MEM_REGION_TYPE_SYSCACHE) {
            base = region->base;
        } else {
            base = (uint64_t)(ctx->config->chip_addr_space * chip_id) +
                region->base;
        }

        FWK_LOG_INFO(
            MOD_NAME "  [0x%llx - 0x%llx] %s",
            base,
            base + region->size - 1,
            mmap_type_name[region->type]);
    }

    FWK_LOG_INFO(MOD_NAME "RNSAM nodes to be configured:");
    for (idx = 0; idx < ctx->internal_rnsam_count; idx++) {
        rnsam = ctx->internal_rnsam_table[idx];
        FWK_LOG_INFO(MOD_NAME "  %d", get_node_id(rnsam));
    }
#endif
}

static int cmn700_setup_sam(struct cmn700_rnsam_reg *rnsam)
{
    unsigned int bit_pos;
    unsigned int cxra_ldid;
    unsigned int cxra_node_id;
    unsigned int group;
    unsigned int hnf_count;
    unsigned int hnf_count_per_cluster;
    unsigned int hnf_cluster_count;
    unsigned int region_idx;
    const struct mod_cmn700_mem_region_map *region;
    const struct mod_cmn700_config *config = ctx->config;
    const struct mod_cmn700_hierarchical_hashing *hier_hash_cfg;

    /* Do configuration for CCG Nodes */
    for (size_t idx = 0; idx < config->ccg_table_count; idx++) {
        region = &config->ccg_config_table[idx].remote_mmap_table;

        FWK_LOG_INFO(
            MOD_NAME "  [0x%" PRIx64 " - 0x%" PRIx64 "] %s",
            region->base,
            region->base + region->size - 1,
            mmap_type_name[region->type]);

        switch (region->type) {
        case MOD_CMN700_REGION_TYPE_CCG:
            /*
             * Configure memory region
             */
            configure_region(
                rnsam,
                ctx->region_io_count,
                region->base,
                region->size,
                SAM_NODE_TYPE_CXRA,
                SAM_TYPE_NON_HASH_MEM_REGION);

            /*
             * Configure target node
             */
            cxra_ldid = config->ccg_config_table[idx].ldid;
            cxra_node_id = ctx->ccg_ra_reg_table[cxra_ldid].node_id;
            group = ctx->region_io_count /
                CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP;
            bit_pos = CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
                (ctx->region_io_count %
                 CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);

            rnsam->NON_HASH_TGT_NODEID[group] &=
                ~(CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK << bit_pos);
            rnsam->NON_HASH_TGT_NODEID[group] |=
                (cxra_node_id & CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK)
                << bit_pos;

            ctx->region_io_count++;
            break;

        default:
            fwk_unexpected();
            return FWK_E_DATA;
        }
    }
    /*
     * If CAL mode is enabled, then only the even numbered HN-F nodes are
     * programmed to the SYS_CACHE registers. Hence reduce the HN-F count by
     * half if CAL mode is enabled.
     */
    if (config->hnf_cal_mode)
        hnf_count = ctx->hnf_count / 2;
    else
        hnf_count = ctx->hnf_count;

    /*
     * If CAL mode is enabled by the configuration program the SCG CAL Mode
     * enable register.
     */
    if (config->hnf_cal_mode) {
        for (region_idx = 0; region_idx < MAX_SCG_COUNT; region_idx++)
            rnsam->SYS_CACHE_GRP_CAL_MODE |=
                ctx->scg_regions_enabled[region_idx] *
                (CMN700_RNSAM_SCG_HNF_CAL_MODE_EN
                 << (region_idx * CMN700_RNSAM_SCG_HNF_CAL_MODE_SHIFT));
    }

    /* Hierarchical Hashing support */
    if (config->hierarchical_hashing_enable) {
        hier_hash_cfg = &(config->hierarchical_hashing_config);

        /* Total number of HN-F clusters */
        hnf_cluster_count = hier_hash_cfg->hnf_cluster_count;

        /* Number of HN-Fs in a cluster */
        hnf_count_per_cluster =
            (hnf_count / hnf_cluster_count) / ctx->region_sys_count;

        /*
         * For each SCG/HTG region, configure the hierarchical hashing mode with
         * number of clusters, hnf count per cluster, hashing address bits etc.
         * and enable hierarchical hashing for each SCG/HTG region.
         */
        for (region_idx = 0; region_idx < ctx->region_sys_count; region_idx++) {
            rnsam->HASHED_TARGET_GRP_HASH_CNTL[region_idx] =
                ((CMN700_RNSAM_HIERARCHICAL_HASH_EN_MASK
                  << CMN700_RNSAM_HIERARCHICAL_HASH_EN_POS) |
                 (fwk_math_log2(hnf_count_per_cluster)
                  << CMN700_RNSAM_HIER_ENABLE_ADDRESS_STRIPING_POS) |
                 (hnf_cluster_count << CMN700_RNSAM_HIER_HASH_CLUSTERS_POS) |
                 (hnf_count_per_cluster << CMN700_RNSAM_HIER_HASH_NODES_POS));

            group =
                region_idx / CMN700_RNSAM_SYS_CACHE_GRP_SN_ATTR_ENTRIES_PER_GRP;
            rnsam->SYS_CACHE_GRP_SN_ATTR[group] |= hier_hash_cfg->sn_mode
                << CMN700_RNSAM_SN_MODE_SYS_CACHE_POS(region_idx);

            group = region_idx /
                CMN700_RNSAM_SYS_CACHE_GRP_SN_SAM_CFG_ENTRIES_PER_GRP;
            rnsam->SYS_CACHE_GRP_SN_SAM_CFG[group] |=
                ((hier_hash_cfg->top_address_bit0
                  << CMN700_RNSAM_TOP_ADDRESS_BIT0_POS(region_idx)) |
                 (hier_hash_cfg->top_address_bit1
                  << CMN700_RNSAM_TOP_ADDRESS_BIT1_POS(region_idx)) |
                 (hier_hash_cfg->top_address_bit2
                  << CMN700_RNSAM_TOP_ADDRESS_BIT2_POS(region_idx)));
        }
    }

    /* Enable RNSAM */
    rnsam->STATUS = (rnsam->STATUS | CMN700_RNSAM_STATUS_UNSTALL) &
        ~(CMN700_RNSAM_STATUS_USE_DEFAULT_TARGET_ID);
    __sync_synchronize();

    return FWK_SUCCESS;
}

static int cmn700_setup(void)
{
    unsigned int rnsam_idx;
    int status;

    if (!ctx->initialized) {
        status = cmn700_discovery();
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
            ctx->hnf_node =
                fwk_mm_calloc(ctx->hnf_count, sizeof(*ctx->hnf_node));
            hnf_node_pos = fwk_mm_calloc(ctx->hnf_count, sizeof(*hnf_node_pos));
            if (ctx->hnf_node == NULL)
                return FWK_E_NOMEM;
            ctx->hnf_cache_group = fwk_mm_calloc(
                cmn700_hnf_cache_group_count(ctx->hnf_count),
                sizeof(*ctx->hnf_cache_group));
        }

        /* Allocate resource for the CCG nodes */
        if (ctx->ccg_node_count != 0) {
            ctx->ccg_ra_reg_table = fwk_mm_calloc(
                ctx->ccg_node_count, sizeof(*ctx->ccg_ra_reg_table));
            ctx->ccg_ha_reg_table = fwk_mm_calloc(
                ctx->ccg_node_count, sizeof(*ctx->ccg_ha_reg_table));
            ctx->ccla_reg_table = fwk_mm_calloc(
                ctx->ccg_node_count, sizeof(*ctx->ccla_reg_table));
        }
    }

    cmn700_configure();

    cmn700_print_region_info();

    status = setup_internal_rn_sam_nodes();
    if (status != FWK_SUCCESS) {
        return status;
    }

    for (rnsam_idx = 0; rnsam_idx < ctx->internal_rnsam_count; rnsam_idx++)
        cmn700_setup_sam(ctx->internal_rnsam_table[rnsam_idx]);

    FWK_LOG_INFO(MOD_NAME "Done");

    ctx->initialized = true;

    return FWK_SUCCESS;
}

static void cmn700_ccg_setup(void)
{
    unsigned int idx;
    const struct mod_cmn700_config *config = ctx->config;

    /* Remote RNF LDID value begins from local chip's last RNF LDID value + 1 */
    ctx->remote_rnf_ldid_value = ctx->rnf_count;

    /* Do configuration for CCG Nodes and enable the links */
    for (idx = 0; idx < config->ccg_table_count; idx++) {
        ccg_setup(chip_id, ctx, &config->ccg_config_table[idx]);
    }

    /*
     * Exchange protocol credits and enter system coherecy and dvm domain for
     * multichip SMP mode operation.
     */
    for (idx = 0; idx < config->ccg_table_count; idx++) {
        ccg_exchange_protocol_credit(ctx, &config->ccg_config_table[idx]);
        ccg_enter_system_coherency(ctx, &config->ccg_config_table[idx]);
        ccg_enter_dvm_domain(ctx, &config->ccg_config_table[idx]);
    }
}

/*
 * Framework handlers
 */

static int cmn700_init(
    fwk_id_t module_id,
    unsigned int device_count,
    const void *data)
{
    /* Atleast one device should be passed as element */
    if (device_count == 0)
        return FWK_E_DATA;

    /* Allocate space for the device context table */
    ctx = fwk_mm_calloc(device_count, sizeof(struct cmn700_device_ctx));

    return FWK_SUCCESS;
}

static int cmn700_device_init(
    fwk_id_t element_id,
    unsigned int element_count,
    const void *data)
{
    struct cmn700_device_ctx *device_ctx;

    fwk_assert(data != NULL);

    device_ctx = ctx + fwk_id_get_element_idx(element_id);
    device_ctx->config = data;

    if (device_ctx->config->base == 0)
        return FWK_E_DATA;

    if ((device_ctx->config->mesh_size_x == 0) ||
        (device_ctx->config->mesh_size_x > CMN700_MESH_X_MAX))
        return FWK_E_DATA;

    if ((device_ctx->config->mesh_size_y == 0) ||
        (device_ctx->config->mesh_size_y > CMN700_MESH_Y_MAX))
        return FWK_E_DATA;

    if (device_ctx->config->snf_count > CMN700_HNF_CACHE_GROUP_ENTRIES_MAX)
        return FWK_E_DATA;

    device_ctx->root = (struct cmn700_cfgm_reg *)device_ctx->config->base;

    return FWK_SUCCESS;
}

static int cmn700_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct cmn700_device_ctx *device_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /* Bind to system info module to obtain multi-chip info */
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_SYSTEM_INFO),
            FWK_ID_API(FWK_MODULE_IDX_SYSTEM_INFO, MOD_SYSTEM_INFO_GET_API_IDX),
            &system_info_api);
        return status;
    }

    /* Use second round only (round numbering is zero-indexed) */
    if (round == 1) {
        device_ctx = ctx + fwk_id_get_element_idx(id);

        /* Bind to the timer component */
        status = fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
            &device_ctx->timer_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

int cmn700_start(fwk_id_t id)
{
    int status;

    /* No need to do anything for element */
    if (!fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    status = system_info_api->get_system_info(&system_info);
    if (status == FWK_SUCCESS) {
        chip_id = system_info->chip_id;
        multi_chip_mode = system_info->multi_chip_mode;
    }

    /* No need to anything for other elements */
    if (fwk_id_get_element_idx(id) != chip_id)
        return FWK_SUCCESS;

    /* Pickup the context based on the chip_id */
    ctx = ctx + fwk_id_get_element_idx(id);

    FWK_LOG_INFO(
        MOD_NAME "Multichip mode: %s",
        multi_chip_mode ? "Enabled" : "Disabled");
    FWK_LOG_INFO(MOD_NAME "Chip ID: %d", chip_id);

    if (fwk_id_is_equal(ctx->config->clock_id, FWK_ID_NONE)) {
        cmn700_setup();
        cmn700_ccg_setup();
        return FWK_SUCCESS;
    }

    /* Register the module for clock state notifications */
    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed, ctx->config->clock_id, id);
}

static int cmn700_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    fwk_assert(
        fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;
    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        cmn700_setup();
        cmn700_ccg_setup();
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_cmn700 = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = cmn700_init,
    .element_init = cmn700_device_init,
    .bind = cmn700_bind,
    .start = cmn700_start,
    .process_notification = cmn700_process_notification,
};
