/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Application Processor address remap module for SGI/RD platforms.
 */

#include "apremap.h"

#include <mod_apremap.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdint.h>

/* Module Context */
struct apremap_ctx {
    /* Flag to track whether CMN Address Translation is enabled */
    bool cmn_addr_trans_enabled;

    /* Pointer to the MSCP Address Translation register */
    struct mod_apremap_reg *apremap_reg;
};

static struct apremap_ctx ctx;

/*
 * Enables CMN Address Translation to translate MSCP address
 * `0x6000_0000 - 0x9FFF_FFFFF` to `(4TB * CHIPID) + (CMN register offset)`
 */
static inline int enable_cmn_addr_trans(void)
{
    __DSB();
    ctx.apremap_reg->ADDR_TRANS = (UINT32_C(1) << APREMAP_CMN_ATRANS_EN_POS);
    ctx.cmn_addr_trans_enabled = true;
    __DSB();

    return FWK_SUCCESS;
}

/*
 * Disable CMN Address Translation. MSCP address `0x6000_0000 - 0x9FFF_FFFFF`
 * will be translated to AP's (4TB * CHIP_ID) + 1GB + offset.
 */
static inline int disable_cmn_addr_trans(void)
{
    __DSB();
    ctx.apremap_reg->ADDR_TRANS &= ~(UINT32_C(1) << APREMAP_CMN_ATRANS_EN_POS);
    ctx.cmn_addr_trans_enabled = false;
    __DSB();

    return FWK_SUCCESS;
}

/*
 * Program the 1MB AP address and enable MSCP address to AP memory remap.
 */
static inline void enable_addr_trans(uint64_t addr)
{
    uint32_t ap_mem_1mb_window_addr = addr >> APREMAP_ADDR_TRANS_AP_ADDR_SHIFT;

    __DSB();
    ctx.apremap_reg->ADDR_TRANS =
        (ap_mem_1mb_window_addr << 1) | APREMAP_ADDR_TRANS_EN;
    __DSB();
}

/*
 * Disable 1MB AP memory remap.
 */
static inline void disable_addr_trans(void)
{
    __DSB();
    ctx.apremap_reg->ADDR_TRANS &= ~APREMAP_ADDR_TRANS_EN;
    __DSB();
}

/*
 * Check if the address belongs between 0-1GB block.
 */
static inline bool is_addr_first_1gb_block(uint64_t addr)
{
    return (addr < (1 * FWK_GIB));
}

/*
 * Check if the address belongs between 1GB-2GB block.
 */
static inline bool is_addr_second_1gb_block(uint64_t addr)
{
    return ((addr >= (1 * FWK_GIB)) && (addr < (2 * FWK_GIB)));
}

/*
 * Check if the address is above 2GB.
 */
static inline bool is_addr_above_2gb(uint64_t addr)
{
    return (addr >= (2 * FWK_GIB));
}

/*
 * Read AP memory through 1MB window based on the data_type passed.
 */
static inline void read_ap_memory_1mb_window(
    uint64_t addr,
    void *value,
    mod_apremap_type_idx_t data_type)
{
    bool cmn_addr_trans_active = false;

    if (ctx.cmn_addr_trans_enabled) {
        cmn_addr_trans_active = true;
        disable_cmn_addr_trans();
    }

    enable_addr_trans(addr);

    switch (data_type) {
    case TYPE_UINT8:
        *(uint8_t *)value = *(volatile uint8_t *)(APREMAP_1MB_ADDR(addr));
        break;

    case TYPE_UINT16:
        *(uint16_t *)value = *(volatile uint16_t *)(APREMAP_1MB_ADDR(addr));
        break;

    case TYPE_UINT32:
        *(uint32_t *)value = *(volatile uint32_t *)(APREMAP_1MB_ADDR(addr));
        break;

    case TYPE_UINT64:
        *(uint64_t *)value = *(volatile uint64_t *)(APREMAP_1MB_ADDR(addr));
        break;

    default:
        break;
    }

    disable_addr_trans();

    if (cmn_addr_trans_active)
        enable_cmn_addr_trans();
}

/*
 * Common MMIO read function shared among mmio_ap_mem_read_<data_type>
 * functions.
 */
static void mmio_ap_mem_read(
    uint64_t addr,
    void *value,
    mod_apremap_type_idx_t data_type)
{
    bool cmn_addr_trans_active = false;

    if (is_addr_first_1gb_block(addr)) {
        switch (data_type) {
        case TYPE_UINT8:
            *(uint8_t *)value =
                *(volatile uint8_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr));
            break;

        case TYPE_UINT16:
            *(uint16_t *)value =
                *(volatile uint16_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr));
            break;

        case TYPE_UINT32:
            *(uint32_t *)value =
                *(volatile uint32_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr));
            break;

        case TYPE_UINT64:
            *(uint64_t *)value =
                *(volatile uint64_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr));
            break;

        default:
            break;
        }

        return;
    } else if (is_addr_second_1gb_block(addr)) {
        if (ctx.cmn_addr_trans_enabled) {
            cmn_addr_trans_active = true;
            disable_cmn_addr_trans();
        }

        switch (data_type) {
        case TYPE_UINT8:
            *(uint8_t *)value =
                *(volatile uint8_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr));
            break;

        case TYPE_UINT16:
            *(uint16_t *)value =
                *(volatile uint16_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr));
            break;

        case TYPE_UINT32:
            *(uint32_t *)value =
                *(volatile uint32_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr));
            break;

        case TYPE_UINT64:
            *(uint64_t *)value =
                *(volatile uint64_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr));
            break;

        default:
            break;
        }

        if (cmn_addr_trans_active)
            enable_cmn_addr_trans();

        return;
    } else if (is_addr_above_2gb(addr)) {
        read_ap_memory_1mb_window(addr, value, data_type);

        return;
    } else
        fwk_unexpected();
}

/*
 * Write to AP memory through 1MB window based on the data_type passed.
 */
static inline void write_ap_memory_1mb_window(
    uint64_t addr,
    void *value,
    mod_apremap_type_idx_t data_type)
{
    bool cmn_addr_trans_active = false;

    if (ctx.cmn_addr_trans_enabled) {
        cmn_addr_trans_active = true;
        disable_cmn_addr_trans();
    }

    enable_addr_trans(addr);

    switch (data_type) {
    case TYPE_UINT8:
        *(volatile uint8_t *)(APREMAP_1MB_ADDR(addr)) = *(uint8_t *)value;
        break;

    case TYPE_UINT16:
        *(volatile uint16_t *)(APREMAP_1MB_ADDR(addr)) = *(uint16_t *)value;
        break;

    case TYPE_UINT32:
        *(volatile uint32_t *)(APREMAP_1MB_ADDR(addr)) = *(uint32_t *)value;
        break;

    case TYPE_UINT64:
        *(volatile uint64_t *)(APREMAP_1MB_ADDR(addr)) = *(uint64_t *)value;
        break;

    default:
        break;
    }

    disable_addr_trans();

    if (cmn_addr_trans_active)
        enable_cmn_addr_trans();
}

/*
 * Common MMIO write function shared among mmio_ap_mem_write_<data_type>
 * functions.
 */
static void mmio_ap_mem_write(
    uint64_t addr,
    void *value,
    mod_apremap_type_idx_t data_type)
{
    bool cmn_addr_trans_active = false;

    if (is_addr_first_1gb_block(addr)) {
        switch (data_type) {
        case TYPE_UINT8:
            *(volatile uint8_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr)) =
                *(uint8_t *)value;
            break;

        case TYPE_UINT16:
            *(volatile uint16_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr)) =
                *(uint16_t *)value;
            break;

        case TYPE_UINT32:
            *(volatile uint32_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr)) =
                *(uint32_t *)value;
            break;

        case TYPE_UINT64:
            *(volatile uint64_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr)) =
                *(uint64_t *)value;
            break;

        default:
            break;
        }

        return;
    } else if (is_addr_second_1gb_block(addr)) {
        if (ctx.cmn_addr_trans_enabled) {
            cmn_addr_trans_active = true;
            disable_cmn_addr_trans();
        }

        switch (data_type) {
        case TYPE_UINT8:
            *(volatile uint8_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr)) =
                *(uint8_t *)value;
            break;

        case TYPE_UINT16:
            *(volatile uint16_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr)) =
                *(uint16_t *)value;
            break;

        case TYPE_UINT32:
            *(volatile uint32_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr)) =
                *(uint32_t *)value;
            break;

        case TYPE_UINT64:
            *(volatile uint64_t *)(ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr)) =
                *(uint64_t *)value;
            break;

        default:
            break;
        }

        if (cmn_addr_trans_active)
            enable_cmn_addr_trans();

        return;
    } else if (is_addr_above_2gb(addr)) {
        write_ap_memory_1mb_window(addr, value, data_type);

        return;
    } else
        fwk_unexpected();
}

/*
 * Module API functions
 */

static uint8_t mmio_ap_mem_read_8(uint64_t addr)
{
    uint8_t value;

    mmio_ap_mem_read(addr, &value, TYPE_UINT8);
    return value;
}

static uint16_t mmio_ap_mem_read_16(uint64_t addr)
{
    uint16_t value;

    mmio_ap_mem_read(addr, &value, TYPE_UINT16);
    return value;
}

static uint32_t mmio_ap_mem_read_32(uint64_t addr)
{
    uint32_t value;

    mmio_ap_mem_read(addr, &value, TYPE_UINT32);
    return value;
}

static uint64_t mmio_ap_mem_read_64(uint64_t addr)
{
    uint64_t value;

    mmio_ap_mem_read(addr, &value, TYPE_UINT64);
    return value;
}

static void mmio_ap_mem_write_8(uint64_t addr, uint8_t value)
{
    mmio_ap_mem_write(addr, &value, TYPE_UINT8);
}

static void mmio_ap_mem_write_16(uint64_t addr, uint16_t value)
{
    mmio_ap_mem_write(addr, &value, TYPE_UINT16);
}

static void mmio_ap_mem_write_32(uint64_t addr, uint32_t value)
{
    mmio_ap_mem_write(addr, &value, TYPE_UINT32);
}

static void mmio_ap_mem_write_64(uint64_t addr, uint64_t value)
{
    mmio_ap_mem_write(addr, &value, TYPE_UINT64);
}

/*
 * API to be used by a module that needs to read/write AP memory region.
 */
static struct interface_address_remapper_rw_api apremap_mem_api = {
    .read8 = mmio_ap_mem_read_8,
    .read16 = mmio_ap_mem_read_16,
    .read32 = mmio_ap_mem_read_32,
    .read64 = mmio_ap_mem_read_64,
    .write8 = mmio_ap_mem_write_8,
    .write16 = mmio_ap_mem_write_16,
    .write32 = mmio_ap_mem_write_32,
    .write64 = mmio_ap_mem_write_64,
};

/* API to enable/disable CMN Address Translation */
static struct mod_apremap_cmn_atrans_api cmn_atrans_api = {
    .enable = enable_cmn_addr_trans,
    .disable = disable_cmn_addr_trans,
};

/*
 * Framework handlers
 */

static int apremap_init(fwk_id_t id, unsigned int unused, const void *data)
{
    const struct mod_apremap_config *config = data;

    ctx.apremap_reg = (struct mod_apremap_reg *)config->base;
    fwk_assert(ctx.apremap_reg != NULL);

    enable_cmn_addr_trans();

    return FWK_SUCCESS;
}

static int apremap_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t targer_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_APREMAP_API_IDX_AP_MEM_RW:
        *api = &apremap_mem_api;
        break;

    case MOD_APREMAP_API_IDX_CMN_ATRANS:
        *api = &cmn_atrans_api;
        break;

    default:
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

/*
 * This module would require appropriate locking/interrupt handling on a
 * multi-core MSCP platform.
 */
const struct fwk_module module_apremap = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = apremap_init,
    .process_bind_request = apremap_process_bind_request,
    .api_count = MOD_APREMAP_API_COUNT,
};
