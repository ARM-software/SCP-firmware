/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bootctl_reg.h"
#include "hsspi_reg.h"
#include "mod_hsspi.h"
#include "qspi_api.h"

#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct hsspi_dev_ctx {
    bool is_start;
    const struct mod_hsspi_dev_config *config;
    struct hsspi_reg *reg;
    void *memory;
    uint8_t slave_num;
};

struct hsspi_ctx {
    struct bootctl_reg *bootctl;
    struct hsspi_dev_ctx *dev_ctx;
};
static struct hsspi_ctx hsspi_ctx;

/*
 * Local macro function
 */
#define REG_MASK_WRITE(addr, mask, val) \
    { \
        uint32_t tmp_val = (*addr) & ~(mask); \
        (*addr) = (tmp_val | (val & mask)); \
    }

/*
 * Local functions for BOOT_CTL
 */
static void set_hsel_mem(int dev_idx, struct bootctl_reg *reg)
{
    uint32_t bs;

    if (dev_idx == 0) {
        bs = ADRDEC_HSSPI1_BIT;
    } else {
        bs = ADRDEC_HSSPI2_BIT;
    }

    REG_MASK_WRITE(
        &reg->BOOT_HSSPI, ADRDEC_HSSPIx_MASK(bs), ADRDEC_HSSPIx(HSEL_MEM, bs));
}

/*
 * Local functions
 */
static void enable_module(struct hsspi_reg *reg, bool enable)
{
    uint32_t mes = (enable) ? MCTRL_MES(ENABLE) : 0;

    if (enable) {
        reg->MCTRL |= MCTRL_MEN(ENABLE);
    } else {
        reg->MCTRL &= ~(MCTRL_MEN(ENABLE));
    }

    while ((reg->MCTRL & MCTRL_MES_MASK) != mes) {
    };
}

static void set_pcc(
    struct hsspi_reg *reg,
    uint8_t clock_div,
    enum hsspi_deselect_time dsel_time)
{
    uint32_t mask, val;

    mask = PCC_RD_DESEL_TIME_MASK | PCC_WR_DESEL_TIME_MASK | PCC_CLOCK_DIV_MASK;
    val = PCC_RD_DESEL_TIME(dsel_time) | PCC_WR_DESEL_TIME(dsel_time) |
        PCC_CLOCK_DIV(clock_div >> 1);

    /*
     * In the command sequencer mode,
     * all slave devices are controlled by HSSPIn_PCC0 register.
     */
    REG_MASK_WRITE(&reg->PCC[0], mask, val);
}

static void set_clock_source(
    struct hsspi_reg *reg,
    enum hsspi_clock_select clock_sel)
{
    REG_MASK_WRITE(&reg->MCTRL, MCTRL_CDSS_MASK, MCTRL_CDSS(clock_sel));
}

static void set_mode(struct hsspi_reg *reg, enum hsspi_mode mode)
{
    REG_MASK_WRITE(&reg->MCTRL, MCTRL_CSEN_MASK, MCTRL_CSEN(mode));
}

static void set_cscfg(
    struct hsspi_reg *reg,
    uint8_t slave_num,
    enum hsspi_memory_bank_size bank_size)
{
    uint32_t mask, val;
    uint8_t ssel0en, ssel1en, ssel2en, ssel3en;

    if (bank_size == HSSPI_MEMORY_BANK_SIZE_256M) {
        ssel0en = ENABLE;
        ssel1en = DISABLE;
        ssel2en = DISABLE;
        ssel3en = DISABLE;
    } else if (bank_size == HSSPI_MEMORY_BANK_SIZE_128M) {
        ssel0en = ENABLE;
        ssel1en = (slave_num > 1) ? ENABLE : DISABLE;
        ssel2en = DISABLE;
        ssel3en = DISABLE;
    } else {
        ssel0en = ENABLE;
        ssel1en = (slave_num > 1) ? ENABLE : DISABLE;
        ssel2en = (slave_num > 2) ? ENABLE : DISABLE;
        ssel3en = (slave_num > 3) ? ENABLE : DISABLE;
    }

    mask = CSCFG_MSEL_MASK | CSCFG_SSELEN_MASK | CSCFG_BOOTEN_MASK |
        CSCFG_SPICHG_MASK;
    val = CSCFG_MSEL(bank_size) |
        CSCFG_SSELEN(ssel0en, ssel1en, ssel2en, ssel3en);
    /* BOOTEN and SPICHG are disable only */

    REG_MASK_WRITE(&reg->CSCFG, mask, val);
}

static void enable_cs_access_mode(
    struct hsspi_reg *reg,
    int io_bit,
    bool enable)
{
    uint32_t mask, val;

    mask = CSCFG_SRAM_MASK;

    if (io_bit == 1) {
        val = CSCFG_MBM(IO_SINGLE);
        mask |= CSCFG_MBM_MASK;
    } else if (io_bit == 2) {
        val = CSCFG_MBM(IO_DUAL);
        mask |= CSCFG_MBM_MASK;
    } else if (io_bit == 4) {
        val = CSCFG_MBM(IO_QUAD);
        mask |= CSCFG_MBM_MASK;
    } else
        val = 0;

    if (enable) {
        val |= CSCFG_SRAM(WRITABLE);
    } else {
        val |= CSCFG_SRAM(READ_ONLY);
    }

    REG_MASK_WRITE(&reg->CSCFG, mask, val);
}

static void set_csitime(struct hsspi_reg *reg, uint16_t timeout)
{
    REG_MASK_WRITE(&reg->CSITIME, CSITIME_MASK, timeout);
}

static uint8_t get_cs_io_bit(struct hsspi_reg *reg)
{
    if ((reg->CSCFG & CSCFG_MBM_MASK) == CSCFG_MBM(IO_DUAL)) {
        return 2;
    } else if ((reg->CSCFG & CSCFG_MBM_MASK) == CSCFG_MBM(IO_QUAD)) {
        return 4;
    } else {
        return 1;
    }
}

static int get_trp_io(int io_bit)
{
    if (io_bit == 1) {
        return TRP_SINGLE;
    } else if (io_bit == 2) {
        return TRP_DUAL;
    } else if (io_bit == 4) {
        return TRP_QUAD;
    } else {
        return TRP_DEFAULT;
    }
}

static uint8_t get_dummy_command_len(
    struct hsspi_reg *reg,
    struct qspi_command *command)
{
    uint8_t dummy_cycle = command->len.dummy_cycle;
    uint8_t dummy_cycle_per_command;

    if (dummy_cycle == 0) {
        return 0;
    }

    /* dummy clcycle should be even */
    if ((dummy_cycle & 1) == 1) {
        dummy_cycle += 1;
    }

    /*
     * number of dummy cycle is affected by the address phase I/O bit
     * because hsspi has 1byte wait command only.
     */
    if (command->io.addr_bit != 0) {
        dummy_cycle_per_command = 8 / command->io.addr_bit;
    } else {
        dummy_cycle_per_command = 8 / get_cs_io_bit(reg);
    }

    return (dummy_cycle / dummy_cycle_per_command);
}

static uint8_t get_command_len(
    struct hsspi_reg *reg,
    struct qspi_command *command)
{
    uint8_t len = 1; /* count of instruction phase */

    len += command->len.addr_byte;
    len += command->len.alt_byte;
    len += get_dummy_command_len(reg, command);

    return len;
}

static int set_command_sequence(
    struct hsspi_reg *reg,
    struct qspi_command *command,
    uint16_t *val)
{
    uint16_t dec_addr[4];
    uint8_t i, cnt = 0;

    /* max commnad length is 8 because number of register is 8 */
    if (8 < get_command_len(reg, command)) {
        return FWK_E_PARAM;
    }

    /* instruction phase */
    val[cnt++] = SET_RAW_DATA(command->code, get_trp_io(command->io.code_bit));

    /* address phase */
    dec_addr[0] = SET_ADDR_1BYTE(get_trp_io(command->io.addr_bit));
    dec_addr[1] = SET_ADDR_2BYTE(get_trp_io(command->io.addr_bit));
    dec_addr[2] = SET_ADDR_3BYTE(get_trp_io(command->io.addr_bit));
    dec_addr[3] = SET_ADDR_4BYTE(get_trp_io(command->io.addr_bit));
    for (i = command->len.addr_byte; i > 0; i--) {
        val[cnt++] = dec_addr[i - 1];
    }

    /* alternative phase */
    for (i = command->len.alt_byte; i > 0; i--) {
        if (i == 1 && command->alt.is_nibble) {
            val[cnt++] = SET_ALT_NIBBLE(
                command->alt.data[0], get_trp_io(command->io.addr_bit));
        } else {
            val[cnt++] = SET_RAW_DATA(
                command->alt.data[i - 1], get_trp_io(command->io.addr_bit));
        }
    }

    /* dummy phase */
    for (i = get_dummy_command_len(reg, command); i > 0; i--) {
        val[cnt++] = SET_DUMMY_1BYTE(get_trp_io(command->io.addr_bit));
    }

    /*
     * hsspi cannot skip data phase in command sequence mode.
     * thus, if data byte is 0, the last one except dummy should be sent by data
     * phase
     */
    if (command->len.data_byte == 0 && command->len.dummy_cycle == 0) {
        val[--cnt] = END_OF_COMMAND(TRP_DEFAULT);
    }

    return FWK_SUCCESS;
}

static int set_rdcsdc(struct hsspi_reg *reg, struct qspi_command *command)
{
    uint16_t i, cs_val[8];
    int status;

    /* initialize command sequence value */
    for (i = 0; i < 8; i++) {
        cs_val[i] = END_OF_COMMAND(TRP_DEFAULT);
    }

    status = set_command_sequence(reg, command, cs_val);
    if (status != FWK_SUCCESS) {
        return status;
    }

    for (i = 0; i < 4; i++) {
        reg->RDCSDC[i] = ((uint32_t)cs_val[i * 2 + 1] << 16) | cs_val[i * 2];
    }

    return FWK_SUCCESS;
}

static int set_wrcsdc(struct hsspi_reg *reg, struct qspi_command *command)
{
    uint16_t i, cs_val[8];
    int status;

    /* initialize command sequence value */
    for (i = 0; i < 8; i++) {
        cs_val[i] = END_OF_COMMAND(TRP_DEFAULT);
    }

    status = set_command_sequence(reg, command, cs_val);
    if (status != FWK_SUCCESS) {
        return status;
    }

    for (i = 0; i < 4; i++) {
        reg->WRCSDC[i] = ((uint32_t)cs_val[i * 2 + 1] << 16) | cs_val[i * 2];
    }

    return FWK_SUCCESS;
}

static void memory_access(
    struct hsspi_dev_ctx *ctx,
    uint8_t slave,
    uint32_t offset,
    void *data,
    uint32_t len,
    bool is_write)
{
    void *slave_addr, *access_addr;
    uint32_t bank_size, bank_mask, bank_len;
    uint32_t access_offset, access_len, total_len;

    bank_size = MEMORY_BANK_SIZE(ctx->config->memory_bank_size);
    bank_mask = MEMORY_BANK_MASK(ctx->config->memory_bank_size);
    bank_len = bank_size - (offset & bank_mask);

    access_offset = offset;
    total_len = 0;

    slave_addr = (char *)ctx->memory + bank_size * slave;

    do {
        /* check whether beyond the boundary of current bank */
        if ((len - total_len) < bank_len) {
            access_len = len - total_len; /* within the bank */
        } else {
            access_len = bank_len; /* to boundary of bank */
        }

        /* set extension register */
        ctx->reg->CSAEXT = (access_offset & ~bank_mask);

        access_addr = (char *)slave_addr + (access_offset & bank_mask);
        if (is_write) {
            memcpy(access_addr, data, access_len);
        } else {
            memcpy(data, access_addr, access_len);
        }

        data = (char *)data + access_len;
        access_offset += access_len;
        total_len += access_len;

        /* next bank_len is bank_size because reach to boundary of bank */
        bank_len = bank_size;
    } while (total_len < len);
}

static void memory_read(
    struct hsspi_dev_ctx *ctx,
    uint8_t slave,
    uint32_t offset,
    void *data,
    uint32_t len)
{
    memory_access(ctx, slave, offset, data, len, false);
}

static void memory_write(
    struct hsspi_dev_ctx *ctx,
    uint8_t slave,
    uint32_t offset,
    void *data,
    uint32_t len)
{
    memory_access(ctx, slave, offset, data, len, true);
}

static int check_command(struct qspi_command *command)
{
    if (command == NULL)
        return FWK_E_PARAM;

    /* length check */
    if (command->len.code_byte != 1) {
        return FWK_E_PARAM;
    }

    if (command->len.addr_byte > 4) {
        return FWK_E_PARAM;
    }

    if (command->len.alt_byte > 4) {
        return FWK_E_PARAM;
    }

    /* I/O bit check */
    if (command->io.code_bit != 0 && command->io.code_bit != 1 &&
        command->io.code_bit != 2 && command->io.code_bit != 4) {
        return FWK_E_PARAM;
    }

    if (command->io.addr_bit != 0 && command->io.addr_bit != 1 &&
        command->io.addr_bit != 2 && command->io.addr_bit != 4) {
        return FWK_E_PARAM;
    }

    if (command->io.data_bit != 0 && command->io.data_bit != 1 &&
        command->io.data_bit != 2 && command->io.data_bit != 4) {
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

/*
 * Module APIs
 */
static int set_read_command(fwk_id_t id, struct qspi_command *command)
{
    struct hsspi_dev_ctx *ctx;
    uint8_t slave;

    ctx = hsspi_ctx.dev_ctx + fwk_id_get_element_idx(id);
    if (!ctx->is_start) {
        return FWK_E_STATE;
    }

    slave = fwk_id_get_sub_element_idx(id);
    if (slave >= ctx->slave_num || (FWK_SUCCESS != check_command(command))) {
        return FWK_E_PARAM;
    }

    enable_cs_access_mode(ctx->reg, command->io.data_bit, false);
    return set_rdcsdc(ctx->reg, command);
}

static int set_write_command(fwk_id_t id, struct qspi_command *command)
{
    struct hsspi_dev_ctx *ctx;
    uint8_t slave;
    int status;

    ctx = hsspi_ctx.dev_ctx + fwk_id_get_element_idx(id);
    if (!ctx->is_start) {
        return FWK_E_STATE;
    }

    slave = fwk_id_get_sub_element_idx(id);
    if (slave >= ctx->slave_num || (FWK_SUCCESS != check_command(command))) {
        return FWK_E_PARAM;
    }

    enable_cs_access_mode(ctx->reg, command->io.data_bit, true);
    status = set_wrcsdc(ctx->reg, command);
    if (status == FWK_SUCCESS) {
        /*
         * if any data except code is nothing, code is sent by data phase
         * because hsspi cannot skip data phase.
         */
        if (command->len.code_byte != 0 && command->len.addr_byte == 0 &&
            command->len.alt_byte == 0 && command->len.dummy_cycle == 0 &&
            command->len.data_byte == 0) {
            memory_write(ctx, slave, 0, &command->code, 1);
        }
    }
    return status;
}

static int read(fwk_id_t id, uint32_t offset, void *buf, uint32_t len)
{
    struct hsspi_dev_ctx *ctx;
    uint8_t slave;

    ctx = hsspi_ctx.dev_ctx + fwk_id_get_element_idx(id);
    if (!ctx->is_start) {
        return FWK_E_STATE;
    }

    slave = fwk_id_get_sub_element_idx(id);
    if (slave >= ctx->slave_num || buf == NULL || len == 0) {
        return FWK_E_PARAM;
    }

    memory_read(ctx, slave, offset, buf, len);
    return FWK_SUCCESS;
}

static int write(fwk_id_t id, uint32_t offset, void *buf, uint32_t len)
{
    struct hsspi_dev_ctx *ctx;
    uint8_t slave;

    ctx = hsspi_ctx.dev_ctx + fwk_id_get_element_idx(id);
    if (!ctx->is_start) {
        return FWK_E_STATE;
    }

    slave = fwk_id_get_sub_element_idx(id);
    if (slave >= ctx->slave_num) {
        return FWK_E_PARAM;
    }

    if (buf == NULL || len == 0) {
        /* set lower 1byte of offset as data, if data size is 0 */
        uint8_t data = (uint8_t)offset;
        memory_write(ctx, slave, offset, &data, sizeof(data));
    } else {
        memory_write(ctx, slave, offset, buf, len);
    }
    return FWK_SUCCESS;
}

static int erase(fwk_id_t id, uint32_t offset)
{
    return write(id, offset, NULL, 0);
}

static void csmode_init(struct hsspi_dev_ctx *ctx)
{
    /* at first, stop module */
    enable_module(ctx->reg, false);

    /* then, start setup */
    set_pcc(ctx->reg, ctx->config->clock_div, ctx->config->deselect_time);
    set_clock_source(ctx->reg, ctx->config->clock_sel);

    /* set memory bank and disable booten */
    set_cscfg(ctx->reg, ctx->slave_num, ctx->config->memory_bank_size);

    /* set single I/O mode, read-only */
    enable_cs_access_mode(ctx->reg, 1, false);

    /* set idle timeout : 256cycle (same as synquacer) */
    set_csitime(ctx->reg, CSITIME_256);

    set_mode(ctx->reg, HSSPI_MODE_COMMAND_SEQUENCE);
    enable_module(ctx->reg, true);
}

static int initialize_csmode(fwk_id_t id)
{
    struct hsspi_dev_ctx *ctx;
    unsigned int dev_idx;

    dev_idx = fwk_id_get_element_idx(id);
    ctx = hsspi_ctx.dev_ctx + dev_idx;

    csmode_init(ctx);

    return FWK_SUCCESS;
}

static struct qspi_api mod_hsspi_api = {
    .set_read_command = set_read_command,
    .set_write_command = set_write_command,
    .read = read,
    .write = write,
    .erase = erase,
    .init_csmode = initialize_csmode,
};

/*
 * Framework handlers
 */

static int hsspi_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    const struct mod_hsspi_config *config;

    if (element_count == 0 || data == NULL) {
        return FWK_E_PANIC;
    }

    config = (const struct mod_hsspi_config *)data;
    hsspi_ctx.bootctl = (struct bootctl_reg *)config->bootctl_base;
    hsspi_ctx.dev_ctx =
        fwk_mm_alloc(element_count, sizeof(hsspi_ctx.dev_ctx[0]));

    return FWK_SUCCESS;
}

static int hsspi_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct hsspi_dev_ctx *ctx;

    if (data == NULL || sub_element_count == 0 ||
        sub_element_count > MAX_SLAVE_NUM) {
        return FWK_E_PANIC;
    }

    ctx = hsspi_ctx.dev_ctx + fwk_id_get_element_idx(element_id);

    ctx->is_start = false;

    ctx->config = (const struct mod_hsspi_dev_config *)data;
    ctx->reg = (struct hsspi_reg *)ctx->config->reg_base;
    ctx->memory = (void *)ctx->config->memory_base;
    ctx->slave_num = (uint8_t)sub_element_count;

    return FWK_SUCCESS;
}

static int hsspi_start(fwk_id_t id)
{
    struct hsspi_dev_ctx *ctx;
    unsigned int dev_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    dev_idx = fwk_id_get_element_idx(id);
    ctx = hsspi_ctx.dev_ctx + dev_idx;

    /* set to BOOT_CTL register */
    set_hsel_mem(dev_idx, hsspi_ctx.bootctl);

    csmode_init(ctx);

    ctx->is_start = true;

    return FWK_SUCCESS;
}

static int hsspi_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case QSPI_API_TYPE_DEFAULT:
        *api = &mod_hsspi_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_hsspi = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = QSPI_API_TYPE_COUNT,
    .init = hsspi_init,
    .element_init = hsspi_element_init,
    .start = hsspi_start,
    .process_bind_request = hsspi_process_bind_request,
};
