/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "mmu500.h"
#include "synquacer_config.h"
#include "synquacer_ddr.h"
#include "synquacer_mmap.h"

#include <cmsis_os2.h>
#include <ddr_init.h>
#include <sysdef_option.h>

#include <internal/i2c_depend.h>
#include <internal/i2c_driver.h>
#include <internal/reg_DMA330.h>

#include <mod_f_i2c.h>
#include <mod_synquacer_system.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern const struct mod_f_i2c_api *f_i2c_api;

#define CONFIG_DDR_ERROR_FORCE_STOP

static char *dimm_module_type[] = { "Extended DIMM", "RDIMM",    "UDIMM",
                                    "SO-DIMM",       "LRDIMM",   "Mini-RDIMM",
                                    "Mini-UDIMM",    "Reserved", "72b-SO-RDIMM",
                                    "72b-SO-UDIMM" };

typedef struct spd_ddr_info_s {
    uint8_t base_module_type;
    uint8_t ddr_memory_type;
    uint8_t slot_bitmap;
    uint8_t ddr_memory_used_ch;
    bool ecc_available;
    uint8_t rank_address_bits;
    uint8_t row_address_bits;
    uint8_t column_address_bits;
    uint32_t set_memory_type_next;
    uint32_t dmc_address_control;
    uint32_t sdram_slot_total[DIMM_SLOT_NUM];
    uint32_t per_slot_dimm_size;
    uint32_t dq_map_control[5];
} spd_ddr_info_t;

static spd_ddr_info_t spd_ddr_info = { 0 };
static uint8_t buf[SPD_READ_BYTE_NUM];

void sysoc_set_reset(uint32_t sysoc_addr, uint32_t value);
int sysoc_wait_status_change(
    uint32_t sysoc_addr,
    bool reset_set_flag,
    uint32_t set_bit);
int ddr_ch0_init_mp(void);
int ddr_ch1_init_mp(void);
static void dram_init_for_ecc(void);

typedef enum {
    SPD_READ_SLOT_AVAILABLE = 0,
    SPD_READ_SLOT_NONE,
    SPD_READ_ERROR,
    SPD_READ_INVALID_PARAM,
    SPD_READ_SET_PAGE_ADDR_ERROR
} spd_read_err_code_t;

static uint32_t get_i2c_slave_addr_set_page(uint32_t target_page);
static spd_read_err_code_t read_spd(
    uint32_t slot,
    uint8_t *dst,
    int32_t total_len);
static void store_spd_to_nssram(void);

static uint32_t config_ddr4_sdram_total_size;

enum ddr_freq_e {
    DDR_FREQ_1333 = 0,
    DDR_FREQ_1600,
    DDR_FREQ_1800,
    DDR_FREQ_1866,
    DDR_FREQ_2133,
    DDR_FREQ_MAX,
};

typedef enum ddr_freq_e ddr_freq_t;

static void fw_ddr_change_freq(ddr_freq_t freq);

void fw_ddr_init(void)
{
    int result = 0;

    fw_ddr_change_freq(CONFIG_SCB_DDR_FREQ);

    if (spd_ddr_info.ddr_memory_used_ch == DDR_USE_CH0)
        result = ddr_ch0_init_mp();
    else if (spd_ddr_info.ddr_memory_used_ch == DDR_USE_CH1)
        result = ddr_ch1_init_mp();
    else
        result = ddr_dual_ch_init_mp();

    if (result != 0) {
        /* Tentative workaround. Need to implement retrying. */
        do {
            FWK_LOG_ERR("DDR Initialize Failed.(0x%x)", result);
            osDelay(10000);
        } while (1);
    }

    if (fw_get_ddr4_sdram_ecc_available() && dram_ecc_is_enabled()) {
        FWK_LOG_INFO("[DDR] DRAM ECC enabled");
        dram_init_for_ecc();
    } else {
        FWK_LOG_INFO("[DDR] DRAM ECC disabled");
    }

    /*
     * save SPD information to non-secure SRAM
     * will be used for system information such as SMBIOS.
     */
    store_spd_to_nssram();
}

static void fw_ddr_change_freq(ddr_freq_t freq)
{
    uint32_t value = 0;
    if (freq >= DDR_FREQ_MAX)
        return;

    switch (freq) {
    case DDR_FREQ_1333:
        FWK_LOG_INFO("[DDR] 1333MHz");
        value = 0x00000027U; /* 1333MHz */
        break;
    case DDR_FREQ_1600:
        FWK_LOG_INFO("[DDR] 1600MHz");
        value = 0x0000002FU; /* 1600MHz */
        break;
    case DDR_FREQ_1800:
        FWK_LOG_INFO("[DDR] 1800MHz");
        value = 0x00000035U; /* 1800MHz */
        break;
    case DDR_FREQ_1866:
        FWK_LOG_INFO("[DDR] 1866MHz");
        value = 0x00000037U; /* 1866MHz */
        break;
    case DDR_FREQ_2133:
        FWK_LOG_INFO("[DDR] 2133MHz");
        value = 0x0000003FU; /* 2133MHz */
        break;
    default:
        FWK_LOG_INFO("[DDR] Invalid DDR frequency");
        assert(false);
    }

    /* set value */
    writel(0x48370000, value);

    /* change freq value kick */
    writel(0x48370080, 0x00000004);

    /* wait for change freq done */
    while (readl(0x48370080) != 0x00000000)
        ;

    /* set value */
    writel(0x48378000, value);

    /* change freq value kick */
    writel(0x48378080, 0x00000004);

    /* wait for change freq done */
    while (readl(0x48378080) != 0x00000000)
        ;
}

static uint32_t get_i2c_slave_addr_set_page(uint32_t target_page)
{
    if (target_page == 0)
        return SPD_DTIC_SPA0;
    else
        return SPD_DTIC_SPA1;
}

static spd_read_err_code_t read_spd(
    uint32_t slot,
    uint8_t *dst,
    int32_t total_len)
{
    I2C_ERR_t i2c_err;
    uint8_t dummy = 0;
    int32_t read_len;
    uint32_t spd_page;
    uint32_t i2c_slave_addr_read_spd;
    uint32_t i2c_slave_addr_set_page;

    /*
     * SPD for DDR4 consists of 512 bytes information and it is divided into
     * two 256 bytes pages.
     * Setting page address to 0 selects the lower 256 bytes,
     * Setting it to 1 selects upper 256 bytes.
     * To set the page address, dummy write to the SPA0/SPA1.
     */

    if (total_len > (SPD_PAGE_SIZE * SPD_NUM_OF_PAGE))
        return SPD_READ_INVALID_PARAM;

    i2c_slave_addr_read_spd = slot + sysdef_option_get_i2c_for_spd_read_addr();

    for (spd_page = 0; spd_page < SPD_NUM_OF_PAGE; spd_page++) {
        /* dummy write to switch the spd page */
        i2c_slave_addr_set_page = get_i2c_slave_addr_set_page(spd_page);
        i2c_err = f_i2c_api->send_data(
            I2C_EN_CH0, i2c_slave_addr_set_page, 0, &dummy, sizeof(dummy));
        if (i2c_err != I2C_ERR_OK)
            return SPD_READ_SET_PAGE_ADDR_ERROR;

        read_len = (total_len > SPD_PAGE_SIZE) ? SPD_PAGE_SIZE : total_len;

        i2c_err = f_i2c_api->recv_data(
            I2C_EN_CH0, i2c_slave_addr_read_spd, 0, dst, read_len);
        if (i2c_err == I2C_ERR_UNAVAILABLE) {
            FWK_LOG_INFO("[SYSTEM] slot DIMM%" PRIu32 ": not detected", slot);
            return SPD_READ_SLOT_NONE;
        }
        if (i2c_err != I2C_ERR_OK) {
            FWK_LOG_INFO(
                "[SYSTEM] Error detected while reading the first byte of SPD. "
                "slave_addr:0x%02" PRIx32 ", errror code = %d",
                i2c_slave_addr_read_spd,
                i2c_err);
            return SPD_READ_ERROR;
        }

        total_len -= read_len;
        if (total_len <= 0)
            break;

        dst += SPD_PAGE_SIZE;
    }

    return SPD_READ_SLOT_AVAILABLE;
}

bool fw_spd_ddr_info_get(spd_ddr_info_t *spd_ddr_info_p)
{
    int i;
    spd_read_err_code_t i2c_err_check;
    int check_dimm_slot;
    bool error_flag = false;

    spd_ddr_info_p->slot_bitmap = 0;
    spd_ddr_info_p->base_module_type = 0;

    for (check_dimm_slot = 0; check_dimm_slot < DIMM_SLOT_NUM;
         check_dimm_slot++) {
        /* SPD parameter */
        uint32_t sdram_capacity, logical_ranks_per_dimm, primary_bus_width,
            sdram_width, bank_group_bits;

        /* spd parameter initialize */
        memset(buf, 0, SPD_READ_BYTE_NUM);

        i2c_err_check = read_spd(check_dimm_slot, buf, SPD_READ_BYTE_NUM);

        if (i2c_err_check == SPD_READ_SLOT_NONE)
            continue;

        if (i2c_err_check != SPD_READ_SLOT_AVAILABLE) {
            error_flag = true;
            return error_flag;
        }

        /* check base module type */
        if ((spd_ddr_info_p->base_module_type != 0) &&
            (spd_ddr_info_p->base_module_type != (buf[3] & 0x0F))) {
            /* error! mixed base module memory */
            FWK_LOG_ERR("[ERROR] use same sdram type!");
            error_flag = true;
            return error_flag;
        } else {
            spd_ddr_info_p->base_module_type = buf[3] & 0x0F;
        }

        /* Bank Group Bits */
        bank_group_bits = (buf[4] & 0xC0) >> 6;
        if (bank_group_bits == 0x02)
            bank_group_bits = 0x03;

        /* SDRAM capacity */
        sdram_capacity = buf[4] & 0x0F; /* SPD original data */

        /* Row Address Bits */
        spd_ddr_info_p->row_address_bits = ((buf[5] & 0x38) >> 3) + 1;

        /* Column Address Bits */
        spd_ddr_info_p->column_address_bits = (buf[5] & 0x07) + 1;

        /* bus width */
        sdram_width = buf[12] & 0x03;

        /* make memory_type_next */
        spd_ddr_info_p->set_memory_type_next =
            (bank_group_bits << 16) | (sdram_width << 8) | DRAM_DDR4;

        logical_ranks_per_dimm = ((buf[12] & 0x38) >> 3) + 1;

        /* DDR4 package Type check */
        if ((buf[6] & 0x3) == 0x02 || (buf[6] & 0x3) == 0x03) {
            FWK_LOG_ERR("[ERROR] not support sdram type!");
            error_flag = true;
            return error_flag;
        }
        /* Number of Package Ranks per DIMM */
        spd_ddr_info_p->rank_address_bits = logical_ranks_per_dimm - 1;

        spd_ddr_info_p->ecc_available = (buf[13] & 0x08) != 0;
        primary_bus_width = 8 * (1 << (buf[13] & 0x07));

        /*
         * Total[MB] = SDRAM Capacity[GB] / 8 * Primary Bus Width /
         *             SDRAM Width * Logical Ranks per DIMM * 1024[GB->MB]
         */
        spd_ddr_info.sdram_slot_total[check_dimm_slot] =
            (((1 << sdram_capacity) >> 3) * primary_bus_width /
             (4 * (1 << sdram_width)) * logical_ranks_per_dimm)
            << 8;
        spd_ddr_info_p->slot_bitmap |= (1 << check_dimm_slot);
        FWK_LOG_INFO(
            "[SYSTEM] slot DIMM%d: %" PRIu32 "MB %s %s",
            check_dimm_slot,
            spd_ddr_info_p->sdram_slot_total[check_dimm_slot],
            dimm_module_type[spd_ddr_info_p->base_module_type],
            ((spd_ddr_info_p->ecc_available) ? "ECC" : "non-ECC"));

        /* dq_map_control data make */
        for (i = 0; i < 5; i++) {
            spd_ddr_info_p->dq_map_control[i] = buf[60 + i * 4];
            spd_ddr_info_p->dq_map_control[i] |= (uint32_t)buf[61 + i * 4] << 8;
            spd_ddr_info_p->dq_map_control[i] |= (uint32_t)buf[62 + i * 4]
                << 16;
            spd_ddr_info_p->dq_map_control[i] |= (uint32_t)buf[63 + i * 4]
                << 24;
        }
    }
    return error_flag;
}

bool fw_spd_rdimm_support_check(spd_ddr_info_t *spd_ddr_info_p)
{
    bool error_flag = false;

    assert(spd_ddr_info_p->per_slot_dimm_size == FW_MODULE_CAPACITY_16GB);

    /* !ERROR CHECK! and memory kinds select */
    /* 16GB module RDIMM */
    switch (spd_ddr_info_p->slot_bitmap) {
    /* 4slot */
    case 0x0FU:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_DUAL_CH;
        spd_ddr_info_p->ddr_memory_type = RDIMM_16GBPERSLOT_2SLOTPERCH;
        spd_ddr_info_p->rank_address_bits *= 2;
        break;

    /* 2slot one side ch1 */
    case 0x0CU:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH1;
        spd_ddr_info_p->ddr_memory_type = RDIMM_16GBPERSLOT_2SLOTPERCH;
        spd_ddr_info_p->rank_address_bits *= 2;
        break;

    /* 2slot one side ch0 */
    case 0x03U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH0;
        spd_ddr_info_p->ddr_memory_type = RDIMM_16GBPERSLOT_2SLOTPERCH;
        spd_ddr_info_p->rank_address_bits *= 2;
        break;

    /* 2slot far side dual ch*/
    case 0x0AU:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_DUAL_CH;
        spd_ddr_info_p->ddr_memory_type = RDIMM_16GBPERSLOT_1SLOTPERCH;
        break;

    /* 1slot ch0 far side */
    case 0x02U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH0;
        spd_ddr_info_p->ddr_memory_type = RDIMM_16GBPERSLOT_1SLOTPERCH;
        break;

    /* 1slot ch1 far side */
    case 0x08U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH1;
        spd_ddr_info_p->ddr_memory_type = RDIMM_16GBPERSLOT_1SLOTPERCH;
        break;

    /* non support dimm slot layout! */
    default:
        FWK_LOG_ERR("[ERROR] read spd at sdram non support dimm slot layout!");
        error_flag = true;
        return error_flag;
    }

    return error_flag;
}

bool fw_spd_udimm_support_check(spd_ddr_info_t *spd_ddr_info_p)
{
    bool error_flag = false;

    switch (spd_ddr_info_p->per_slot_dimm_size) {
    case FW_MODULE_CAPACITY_8GB:
        spd_ddr_info_p->ddr_memory_type = UDIMM_8GBPERSLOT_1SLOTPERCH;
        break;

    case FW_MODULE_CAPACITY_4GB:
        spd_ddr_info_p->ddr_memory_type = UDIMM_4GBPERSLOT_1SLOTPERCH;
        break;

    default:
        FWK_LOG_ERR("[ERROR] non support capability dimm!");
        error_flag = true;
        return error_flag;
    }

    switch (spd_ddr_info_p->slot_bitmap) {
    /* 2slot far side */
    case 0x0AU:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_DUAL_CH;
        break;

    /* 1slot ch0 far side */
    case 0x02U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH0;
        break;

    /* 1slot ch1 far side */
    case 0x08U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH1;
        break;

    /* non support dimm slot layout! */
    default:
        FWK_LOG_ERR("[ERROR] read spd at sdram non support dimm slot layout!");
        error_flag = true;
        return error_flag;
    }
    return error_flag;
}

bool fw_spd_72bitsoudimm_support_check(spd_ddr_info_t *spd_ddr_info_p)
{
    bool error_flag = false;

    switch (spd_ddr_info_p->per_slot_dimm_size) {
    case FW_MODULE_CAPACITY_16GB:
        spd_ddr_info_p->ddr_memory_type = SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH;
        break;

    case FW_MODULE_CAPACITY_8GB:
        spd_ddr_info_p->ddr_memory_type = SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH;
        break;

    default:
        FWK_LOG_ERR("[ERROR] non support capability dimm!");
        error_flag = true;
        return error_flag;
    }

    switch (spd_ddr_info_p->slot_bitmap) {
    /* 2slot */
    case 0x03U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_DUAL_CH;
        break;

    /* 1slot ch1 */
    case 0x02U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH1;
        break;

    /* 1slot ch0 */
    case 0x01U:
        spd_ddr_info_p->ddr_memory_used_ch = DDR_USE_CH0;
        break;

    /* non support dimm slot layout! */
    default:
        error_flag = true;
        FWK_LOG_ERR("[ERROR] read spd at sdram non support dimm slot layout!");
    }

    return error_flag;
}

bool fw_spd_read_dimm_capacity_check(spd_ddr_info_t *spd_ddr_info_p)
{
    int i;
    bool error_flag = false;

    spd_ddr_info_p->per_slot_dimm_size = 0;
    config_ddr4_sdram_total_size = 0;

    for (i = 0; i < DIMM_SLOT_NUM; i++) {
        if (spd_ddr_info_p->slot_bitmap & (1 << i)) {
            if ((spd_ddr_info_p->per_slot_dimm_size != 0) &&
                (spd_ddr_info_p->per_slot_dimm_size !=
                 spd_ddr_info_p->sdram_slot_total[i])) {
                FWK_LOG_ERR("[ERROR] Please use same capacity DDR memory!");
                error_flag = true;
                return error_flag;
            }
            spd_ddr_info_p->per_slot_dimm_size =
                spd_ddr_info_p->sdram_slot_total[i];
            config_ddr4_sdram_total_size += spd_ddr_info_p->sdram_slot_total[i];
        }
    }
    return error_flag;
}

bool fw_spd_read_dimm_kinds_check(spd_ddr_info_t *spd_ddr_info_p)
{
    bool error_flag = false;

    /* !ERROR CHECK! and memory kinds select */
    switch (spd_ddr_info_p->base_module_type) {
    /* RDIMM */
    case MODULE_TYPE_RDIMM:
        error_flag = fw_spd_rdimm_support_check(spd_ddr_info_p);
        break;

    /* UDIMM */
    case MODULE_TYPE_UDIMM:
        error_flag = fw_spd_udimm_support_check(spd_ddr_info_p);
        break;

    /* SODIMM */
    case MODULE_TYPE_72BITSOUDIMM:
        error_flag = fw_spd_72bitsoudimm_support_check(spd_ddr_info_p);
        break;

    /* other type memory module */
    default:
        FWK_LOG_ERR("[ERROR] sdram other type memory module");
        error_flag = true;
        return error_flag;
    }

    spd_ddr_info_p->dmc_address_control =
        (uint32_t)(spd_ddr_info_p->rank_address_bits << 24) | BANK_BIT_NEXT |
        (uint32_t)(spd_ddr_info_p->row_address_bits << 8) |
        (uint32_t)(spd_ddr_info_p->column_address_bits);

    return error_flag;
}

int fw_ddr_spd_param_check(void)
{
    FWK_LOG_INFO("[SYSTEM] Starting check DRAM");

    if (fw_spd_ddr_info_get(&spd_ddr_info))
        return FWK_E_SUPPORT;

    if (fw_spd_read_dimm_capacity_check(&spd_ddr_info))
        return FWK_E_SUPPORT;

    if (fw_spd_read_dimm_kinds_check(&spd_ddr_info))
        return FWK_E_SUPPORT;

    FWK_LOG_INFO(
        "[SYSTEM] Finished check DRAM memory total %" PRIu32 "GB",
        (config_ddr4_sdram_total_size / 1024));

    return FWK_SUCCESS;
}

bool fw_get_ddr4_sdram_ecc_available(void)
{
    return spd_ddr_info.ecc_available;
}

uint8_t fw_get_used_memory_ch(void)
{
    return spd_ddr_info.ddr_memory_used_ch;
}

uint32_t fw_get_memory_type_next(void)
{
    return spd_ddr_info.set_memory_type_next;
}

uint32_t fw_get_address_control_next(void)
{
    return spd_ddr_info.dmc_address_control;
}

uint32_t fw_get_ddr4_sdram_dq_map_control(uint8_t i)
{
    return spd_ddr_info.dq_map_control[i];
}

uint32_t fw_get_ddr4_sdram_total_size(void)
{
    return config_ddr4_sdram_total_size;
}

uint32_t fw_get_memory_type(void)
{
    return spd_ddr_info.ddr_memory_type;
}

#define CM3_TO_AP_ADDR(addr) ((addr)-EXTERNAL_DEV_BASE)

/* 16MB zero clear*/
static void dma330_zero_clear(
    uint32_t inst_start_addr,
    uint32_t clear_start_addr)
{
    volatile REG_ST_DMA330_S_t *REG_DMA330_S;

    REG_DMA330_S = (volatile REG_ST_DMA330_S_t *)CONFIG_SOC_DMA330_REG_BASE;

    /**
     * Instruction for DMA330
     * DMAMOV CCR SB1 SS8 SP3 SC0 DB16 DS128 DP3 DC1
     *  DMAMOV DAR 0x80000000
     *  DMALP 256
     *    DMALP 256
     *       DMASTZ
     *    DMALPEND
     *  DMALPEND
     *  DMAWMB
     *  DMASEV 0
     *  DMAEND
     */
    writel(inst_start_addr + 0x000U, 0x430101bcU);
    writel(inst_start_addr + 0x004U, 0x02bc02feU);
    writel(inst_start_addr + 0x008U, clear_start_addr);
    /**
     * DMA330 0 clear size setting
     * DMALP(1) 256(1), in loop DMAL 256(0)
     * 16 * 128bit * 256 * 256 = 16MBytes
     */
    writel(inst_start_addr + 0x00cU, 0xff22ff20U);
    writel(inst_start_addr + 0x010U, 0x38013c0cU);
    writel(inst_start_addr + 0x014U, 0x00341305U);
    writel(inst_start_addr + 0x018U, 0xffffff00U);
    writel(inst_start_addr + 0x01cU, 0xffffffffU);

    REG_DMA330_S->INTEN = 0xffffffffU;

    while (REG_DMA330_S->DBGSTATUS != 0) {
        FWK_LOG_INFO("[SYSTEM] Wait DMA330 busy.");
        osDelay(10);
    }

    REG_DMA330_S->DBGINST[0] = 0xa2U << 16;
    REG_DMA330_S->DBGINST[1] = CM3_TO_AP_ADDR(inst_start_addr);
    REG_DMA330_S->DBGCMD = 0;

    /* Wait for DMA done */
    while (REG_DMA330_S->INTMIS == 0)
        osDelay(1U);

    REG_DMA330_S->INTCLR = 0xffffffffU;
}

#define REG_DMAB_REG_dma330_boot_manager \
    (CONFIG_SOC_DMAB_WRAPPER_REG + 0x00000010)
#define REG_DMAB_REG_mmu500_wsb_ns_zero_0_s \
    (CONFIG_SOC_DMAB_WRAPPER_REG + 0x00000118)
#define REG_DMAB_REG_mmu500_rsb_ns_zero_0_s \
    (CONFIG_SOC_DMAB_WRAPPER_REG + 0x00000128)
#define REG_DMAB_REG_mmu500_awdomain_zero_s \
    (CONFIG_SOC_DMAB_WRAPPER_REG + 0x00000110)
#define REG_DMAB_REG_mmu500_ardomain_zero_s \
    (CONFIG_SOC_DMAB_WRAPPER_REG + 0x00000120)

static void dmab_mmu500_init(uint64_t output_addr)
{
    uint32_t i;
    uint32_t page_table_top_ap_addr;
    uint32_t page_table_top_addr;
    uint32_t upper_addr;
    uint32_t lower_addr;
    MMU500_ContextInfo_t context_info;
    MMU500_Type_t *p_mmu_dmab;
    p_mmu_dmab = (MMU500_Type_t *)CONFIG_SOC_DMAB_SMMU_REG_BASE;

    page_table_top_addr = TRUSTED_RAM_BASE;
    page_table_top_ap_addr = CM3_TO_AP_ADDR(page_table_top_addr);

    /* lv1 Excel(TTBR Descriptor address, Level2 Table address) */
    writel(
        page_table_top_addr + 0x00000000 + 0x0,
        3 | (page_table_top_ap_addr + 0x00010000));
    writel(page_table_top_addr + 0x00000000 + 0x4, 0);

    /* Input  Address = 0x2e000000 */
    /* lv2 Excel(Level1 Descriptor address, Level3 Table address) */
    writel(
        page_table_top_addr + 0x00010008 + 0x0,
        3 | (page_table_top_ap_addr + 0x00020000));
    writel(page_table_top_addr + 0x00010008 + 0x4, 0);

    /* Input  Address = 0x80000000 */
    /* lv2 Excel(Level1 Descriptor address, Level3 Table address) */
    writel(
        page_table_top_addr + 0x00010020 + 0x0,
        3 | (page_table_top_ap_addr + 0x00030000));
    writel(page_table_top_addr + 0x00010020 + 0x4, 0);

    /* Input  Address = 0x2e000000 */
    /* Output Address = 0x2e000000 */
    /* lv3  Excel(Level2 Descriptor address) */
    upper_addr = 0x00000000;
    lower_addr = 0x2e000000;

    /* 0x0000 - 0x0001 ( 64KB *     1 = 64KB : 0x00010000 ) */
    for (i = 0; i < 0x0001; i++) {
        writel(
            page_table_top_addr + 0x00027000 + 0x0 + (i * 0x8),
            3 | (lower_addr) | (1 << 10) | (1 << 7) | (1 << 6) | (0 << 5) |
                (0 << 4) | (0 << 3) | (0 << 2));

        writel(page_table_top_addr + 0x00027000 + 0x4 + (i * 0x8), upper_addr);
        lower_addr = (lower_addr + 0x10000);
    }

    /* Input  Address = 0x80000000 */
    /* Output Address = output_addr */
    /* lv3 Excel(Level2 Descriptor address) */
    /*    0x00000 - 0x02000 ( 64KB *   8192 = 512MB : 0x200000000     */
    for (i = 0; i < 0x02000; i++, output_addr += 0x10000) {
        upper_addr = output_addr >> 32;
        lower_addr = output_addr;

        writel(
            page_table_top_addr + 0x00030000 + 0x0 + (i * 0x8),
            3 | (lower_addr) | (1 << 10) | (0 << 7) | (1 << 6) | (0 << 5) |
                (0 << 4) | (0 << 3) | (1 << 2));

        writel(page_table_top_addr + 0x00030000 + 0x4 + (i * 0x8), upper_addr);
    }

    context_info.stream_match_mask = 0x7c00U;
    context_info.stream_match_id = 0;
    context_info.base_addr = (uint64_t)page_table_top_ap_addr;

    SMMU_s_init(
        (MMU500_Type_t *)p_mmu_dmab,
        1,
        (const MMU500_ContextInfo_t *)&context_info,
        MMU500_GRANULE_64KB);
}

#define addr_trans(addr_39_20, size, write) (0)

#define DMA330_ERASE_BLOCK_SIZE UINT32_C(0x1000000)

static void dma330_wrapper_init(void)
{
    uint32_t value;

    /* Change to Non Secure bit0 : 0=Secure */
    value = readl(REG_DMAB_REG_dma330_boot_manager);
    writel(REG_DMAB_REG_dma330_boot_manager, value & 0xFFFFFFFE);

    /* Change to Non Secure bit0 = 0 : mmu500_wsb_ns_zero_0_s */
    value = readl(REG_DMAB_REG_mmu500_wsb_ns_zero_0_s);
    writel(REG_DMAB_REG_mmu500_wsb_ns_zero_0_s, value & 0xFFFFFFFE);

    /* Change to Non Secure bit0 = 0 : mmu500_rsb_ns_zero_0_s */
    value = readl(REG_DMAB_REG_mmu500_rsb_ns_zero_0_s);
    writel(REG_DMAB_REG_mmu500_rsb_ns_zero_0_s, value & 0xFFFFFFFE);

    /* REG_DMAB_REG_mmu500_awdomain_zero_s &= 0xFFFFFFFC; */
    /* bit1:0 = 2'b00(Non shareable) */
    value = readl(REG_DMAB_REG_mmu500_awdomain_zero_s);
    writel(REG_DMAB_REG_mmu500_awdomain_zero_s, value | 0x00000003);

    /* bit1:0 = 2'b11 : System */
    value = readl(REG_DMAB_REG_mmu500_ardomain_zero_s);
    writel(REG_DMAB_REG_mmu500_ardomain_zero_s, value | 0x00000003);
}

#define TEST_SIZE (1024 * 1024)
static void dram_init_for_ecc(void)
{
    uint64_t dst_ddr_addr, dram_size;
    uint32_t dma_dst_addr;

    SMMU_s_disable((MMU500_Type_t *)CONFIG_SOC_DMAB_SMMU_REG_BASE, 1);

    (void)addr_trans(0x00800U, TEST_SIZE, true);
    (void)addr_trans(0x008ffU, TEST_SIZE, true);
    (void)addr_trans(0x00fffU, TEST_SIZE, true);
    (void)addr_trans(0x08800U, TEST_SIZE, true);
    (void)addr_trans(0x088ffU, TEST_SIZE, true);
    (void)addr_trans(0x0ff00U, TEST_SIZE, true);
    (void)addr_trans(0x0ffffU, TEST_SIZE, true);

    dram_size = (uint64_t)config_ddr4_sdram_total_size * 1024 * 1024;

    FWK_LOG_INFO("[DDR] Initializing DRAM for ECC\nNow Initializing[");

    dma330_wrapper_init();

    /* 0x00_8000_0000 - 0x00_ffff_ffff */
    for (dst_ddr_addr = DRAM_AREA_1_START_ADDR, dma_dst_addr = 0x80000000U;
         (dst_ddr_addr < DRAM_AREA_1_END_ADDR) && (dram_size != 0);
         dma_dst_addr += DMA330_ERASE_BLOCK_SIZE) {
        if ((dst_ddr_addr & 0x3fffffffULL) == 0)
            FWK_LOG_INFO("[DDR] +");

        dma330_zero_clear(0xce000000U, dma_dst_addr);
        dst_ddr_addr += DMA330_ERASE_BLOCK_SIZE;
        dram_size -= DMA330_ERASE_BLOCK_SIZE;
    }

    /* 0x08_8000_0000 - 0x0f_ffff_ffff */
    for (dst_ddr_addr = DRAM_AREA_2_START_ADDR, dma_dst_addr = 0x80000000U;
         (dst_ddr_addr < DRAM_AREA_2_END_ADDR) && (dram_size != 0);
         dma_dst_addr += DMA330_ERASE_BLOCK_SIZE) {
        if ((dst_ddr_addr & 0x3fffffffULL) == 0)
            FWK_LOG_INFO("[DDR] -");

        if ((dst_ddr_addr & 0x1fffffffULL) == 0) {
            dmab_mmu500_init(dst_ddr_addr);
            dma_dst_addr = 0x80000000U;
        }

        dma330_zero_clear(0xce000000U, dma_dst_addr);
        dst_ddr_addr += DMA330_ERASE_BLOCK_SIZE;
        dram_size -= DMA330_ERASE_BLOCK_SIZE;
    }

    /* 0x88_0000_0000 - 0x8f_ffff_ffff */
    for (dst_ddr_addr = DRAM_AREA_3_START_ADDR, dma_dst_addr = 0x80000000U;
         (dst_ddr_addr < DRAM_AREA_3_END_ADDR) && (dram_size != 0);
         dma_dst_addr += DMA330_ERASE_BLOCK_SIZE) {
        if ((dst_ddr_addr & 0x3fffffffULL) == 0)
            FWK_LOG_INFO("[DDR] x");

        if ((dst_ddr_addr & 0x1fffffffULL) == 0) {
            dmab_mmu500_init(dst_ddr_addr);
            dma_dst_addr = 0x80000000U;
        }

        dma330_zero_clear(0xce000000U, dma_dst_addr);
        dst_ddr_addr += DMA330_ERASE_BLOCK_SIZE;
        dram_size -= DMA330_ERASE_BLOCK_SIZE;
    }

    FWK_LOG_INFO("[DDR] Finished initializing DRAM for ECC");

    (void)addr_trans(0x00800U, TEST_SIZE, false);
    (void)addr_trans(0x008ffU, TEST_SIZE, false);
    (void)addr_trans(0x00fffU, TEST_SIZE, false);
    (void)addr_trans(0x08800U, TEST_SIZE, false);
    (void)addr_trans(0x088ffU, TEST_SIZE, false);
    (void)addr_trans(0x0ff00U, TEST_SIZE, false);
    (void)addr_trans(0x0ffffU, TEST_SIZE, false);

    SMMU_s_disable((MMU500_Type_t *)CONFIG_SOC_DMAB_SMMU_REG_BASE, 1);

    /* Clear Secure SRAM */
    memset((void *)TRUSTED_RAM_BASE, 0, 256 * 1024);
}

static void store_spd_to_nssram(void)
{
    uint32_t slot;
    uint8_t *dst = (uint8_t *)SPD_STORE_ADDR;
    spd_read_err_code_t ret;

    memset(dst, 0, SPD_STORE_AREA_SIZE);

    for (slot = 0; slot < DIMM_SLOT_NUM; slot++) {
        ret = read_spd(slot, dst, (SPD_PAGE_SIZE * SPD_NUM_OF_PAGE));
        if (ret != SPD_READ_SLOT_AVAILABLE) {
            /* clear invalid data */
            memset(dst, 0, (SPD_PAGE_SIZE * SPD_NUM_OF_PAGE));
        }
        dst += (SPD_PAGE_SIZE * SPD_NUM_OF_PAGE);
    }
}
