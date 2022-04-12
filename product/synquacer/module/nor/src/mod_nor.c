/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mod_nor.h"
#include "qspi_api.h"

#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

/*
 * macro definitions
 */
/* Status Register */
#define WEL_BIT            1
#define WEL_ENABLE         (0x1 << WEL_BIT)
#define IS_WEL_ENABLE(val) (((val)&WEL_ENABLE) != 0)

#define WIP_BIT          0
#define WIP_BUSY         (0x1 << WIP_BIT)
#define IS_WIP_BUSY(val) (((val)&WIP_BUSY) != 0)

#define RESET_EFFECTIVE_WAIT_US 1000 /* 1msec */

#define ERASE_SIZE_SECTOR_4KB  (0x1U << 12)
#define ERASE_SIZE_SECTOR_32KB (0x1U << 15)

#define PAGE_MASK(page_size) ((page_size)-1)

struct nor_dev_ctx {
    uint8_t slave_num;
    const struct mod_nor_dev_config *config;
    const struct qspi_api *qspi_api;
};

struct nor_ctx {
    struct nor_dev_ctx *dev_ctx;
    const struct mod_timer_api *timer_api;
};
static struct nor_ctx nor_ctx;

static enum mod_nor_command_idx
    nor_read_command_index_list[MOD_NOR_READ_MODE_COUNT] = {
        [MOD_NOR_READ]                  = MOD_NOR_COMMAND_READ,
        [MOD_NOR_READ_FAST]             = MOD_NOR_COMMAND_FAST_READ,
        [MOD_NOR_READ_FAST_1_1_2]       = MOD_NOR_COMMAND_FAST_READ_1_1_2,
        [MOD_NOR_READ_FAST_1_2_2]       = MOD_NOR_COMMAND_FAST_READ_1_2_2,
        [MOD_NOR_READ_FAST_1_1_4]       = MOD_NOR_COMMAND_FAST_READ_1_1_4,
        [MOD_NOR_READ_FAST_1_4_4]       = MOD_NOR_COMMAND_FAST_READ_1_4_4,
        [MOD_NOR_READ_4BYTE]            = MOD_NOR_COMMAND_READ_4B,
        [MOD_NOR_READ_FAST_4BYTE]       = MOD_NOR_COMMAND_FAST_READ_4B,
        [MOD_NOR_READ_FAST_1_1_2_4BYTE] = MOD_NOR_COMMAND_FAST_READ_1_1_2_4B,
        [MOD_NOR_READ_FAST_1_2_2_4BYTE] = MOD_NOR_COMMAND_FAST_READ_1_2_2_4B,
        [MOD_NOR_READ_FAST_1_1_4_4BYTE] = MOD_NOR_COMMAND_FAST_READ_1_1_4_4B,
        [MOD_NOR_READ_FAST_1_4_4_4BYTE] = MOD_NOR_COMMAND_FAST_READ_1_4_4_4B,
    };

static enum mod_nor_command_idx
    nor_program_command_index_list[MOD_NOR_PROGRAM_MODE_COUNT] = {
        [MOD_NOR_PROGRAM]             = MOD_NOR_COMMAND_PROGRAM,
        [MOD_NOR_PROGRAM_1_1_2]       = MOD_NOR_COMMAND_PROGRAM_1_1_2,
        [MOD_NOR_PROGRAM_1_2_2]       = MOD_NOR_COMMAND_PROGRAM_1_2_2,
        [MOD_NOR_PROGRAM_1_1_4]       = MOD_NOR_COMMAND_PROGRAM_1_1_4,
        [MOD_NOR_PROGRAM_1_4_4]       = MOD_NOR_COMMAND_PROGRAM_1_4_4,
        [MOD_NOR_PROGRAM_4BYTE]       = MOD_NOR_COMMAND_PROGRAM_4B,
        [MOD_NOR_PROGRAM_1_1_4_4BYTE] = MOD_NOR_COMMAND_PROGRAM_1_1_4_4B,
        [MOD_NOR_PROGRAM_1_4_4_4BYTE] = MOD_NOR_COMMAND_PROGRAM_1_4_4_4B,
    };

static enum mod_nor_command_idx
    nor_erase_command_index_list[MOD_NOR_ERASE_MODE_COUNT] = {
        [MOD_NOR_ERASE_SECTOR_4KB]        = MOD_NOR_COMMAND_ERASE_SECTOR_4KB,
        [MOD_NOR_ERASE_SECTOR_32KB]       = MOD_NOR_COMMAND_ERASE_SECTOR_32KB,
        [MOD_NOR_ERASE_BLOCK]             = MOD_NOR_COMMAND_ERASE_BLOCK,
        [MOD_NOR_ERASE_CHIP]              = MOD_NOR_COMMAND_ERASE_CHIP,
        [MOD_NOR_ERASE_SECTOR_4KB_4BYTE]  = MOD_NOR_COMMAND_ERASE_SECTOR_4KB_4B,
        [MOD_NOR_ERASE_SECTOR_32KB_4BYTE] = MOD_NOR_COMMAND_ERASE_SECTOR_32KB_4B,
        [MOD_NOR_ERASE_BLOCK_4BYTE]       = MOD_NOR_COMMAND_ERASE_BLOCK_4B,
    };

/*
 * local functions
 */
static bool is_command_not_supported(
    struct nor_dev_ctx *ctx,
    enum mod_nor_command_idx idx)
{
    return IS_QSPI_COMMAND_EMPTY(ctx->config->command_table[idx]);
}

static int set_read_command(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    enum mod_nor_command_idx idx)
{
    if (is_command_not_supported(ctx, idx)) {
        return FWK_E_SUPPORT;
    }

    return ctx->qspi_api->set_read_command(
        id, &ctx->config->command_table[idx]);
}

static int set_write_command(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    enum mod_nor_command_idx idx)
{
    if (is_command_not_supported(ctx, idx)) {
        return FWK_E_SUPPORT;
    }

    return ctx->qspi_api->set_write_command(
        id, &ctx->config->command_table[idx]);
}

static int read_status(fwk_id_t id, struct nor_dev_ctx *ctx, uint8_t *buf)
{
    int status;

    status = set_read_command(id, ctx, MOD_NOR_COMMAND_READ_STATUS);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return ctx->qspi_api->read(id, 0, buf, sizeof(buf[0]));
}

static int set_write_enable(fwk_id_t id, struct nor_dev_ctx *ctx)
{
    uint8_t buf;
    int status;

    do {
        status = set_write_command(id, ctx, MOD_NOR_COMMAND_WRITE_ENABLE);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = read_status(id, ctx, &buf);
        if (status != FWK_SUCCESS) {
            return status;
        }

    } while (!IS_WEL_ENABLE(buf));

    return FWK_SUCCESS;
}

static int wait_until_ready(fwk_id_t id, struct nor_dev_ctx *ctx)
{
    uint8_t buf;
    int status;

    status = set_read_command(id, ctx, MOD_NOR_COMMAND_READ_STATUS);
    if (status != FWK_SUCCESS) {
        return status;
    }

    do {
        status = ctx->qspi_api->read(id, 0, &buf, sizeof(buf));
        if (status != FWK_SUCCESS) {
            return status;
        }

    } while (IS_WIP_BUSY(buf));

    return FWK_SUCCESS;
}

static int set_4byte_address_mode(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    bool enable)
{
    int status;

    if (ctx->config->operation->set_4byte_addr_mode != NULL) {
        status = ctx->config->operation->set_4byte_addr_mode(
            id, ctx->qspi_api, (void *)&enable);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int set_io_protocol(fwk_id_t id, struct nor_dev_ctx *ctx, uint8_t io_num)
{
    int status;

    if (ctx->config->operation->set_io_protocol != NULL) {
        /* should be set write enable */
        status = set_write_enable(id, ctx);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = ctx->config->operation->set_io_protocol(
            id, ctx->qspi_api, (void *)&io_num);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* should be status check */
        status = wait_until_ready(id, ctx);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static void set_single_3byte_address_mode(fwk_id_t id, struct nor_dev_ctx *ctx)
{
    int status;

    status = set_io_protocol(id, ctx, 1);
    fwk_assert(status == FWK_SUCCESS);

    status = set_4byte_address_mode(id, ctx, false);
    fwk_assert(status == FWK_SUCCESS);
}

static int check_program_result(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    bool *is_fail)
{
    int status = FWK_SUCCESS;

    if (ctx->config->operation->get_program_result != NULL) {
        status = ctx->config->operation->get_program_result(
            id, ctx->qspi_api, (void *)&is_fail);
    } else {
        *is_fail = false;
    }

    return status;
}

static int check_erase_result(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    bool *is_fail)
{
    int status = FWK_SUCCESS;

    if (ctx->config->operation->get_erase_result != NULL) {
        status = ctx->config->operation->get_erase_result(
            id, ctx->qspi_api, (void *)&is_fail);
    } else {
        *is_fail = false;
    }

    return status;
}

static int program_erase_sequence(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    enum mod_nor_command_idx command_idx,
    uint32_t offset,
    void *buf,
    uint32_t len)
{
    int status = FWK_SUCCESS;
    bool is_erase = (buf == NULL);
    bool is_fail = false;

    status = set_write_enable(id, ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = set_write_command(id, ctx, command_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (is_erase) {
        /*
         * no need to set offset when len is 0
         * because nothing to transfer on the data phase.
         */
        if (len != 0) {
            status = ctx->qspi_api->erase(id, offset);
        }
    } else {
        status = ctx->qspi_api->write(id, offset, buf, len);
    }
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = wait_until_ready(id, ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (is_erase) {
        status = check_erase_result(id, ctx, &is_fail);
    } else {
        status = check_program_result(id, ctx, &is_fail);
    }
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (is_fail) {
        return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

static uint8_t get_read_io_num(enum mod_nor_read_mode mode)
{
    uint8_t io_num;

    switch (mode) {
    case MOD_NOR_READ:
    case MOD_NOR_READ_FAST:
    case MOD_NOR_READ_4BYTE:
    case MOD_NOR_READ_FAST_4BYTE:
    default:
        io_num = 1;
        break;
    case MOD_NOR_READ_FAST_1_1_2:
    case MOD_NOR_READ_FAST_1_2_2:
    case MOD_NOR_READ_FAST_1_1_2_4BYTE:
    case MOD_NOR_READ_FAST_1_2_2_4BYTE:
        io_num = 2;
        break;
    case MOD_NOR_READ_FAST_1_1_4:
    case MOD_NOR_READ_FAST_1_4_4:
    case MOD_NOR_READ_FAST_1_1_4_4BYTE:
    case MOD_NOR_READ_FAST_1_4_4_4BYTE:
        io_num = 4;
        break;
    }

    return io_num;
}

static uint8_t get_program_io_num(enum mod_nor_program_mode mode)
{
    uint8_t io_num;

    switch (mode) {
    case MOD_NOR_PROGRAM:
    case MOD_NOR_PROGRAM_4BYTE:
    default:
        io_num = 1;
        break;
    case MOD_NOR_PROGRAM_1_1_2:
    case MOD_NOR_PROGRAM_1_2_2:
        io_num = 2;
        break;
    case MOD_NOR_PROGRAM_1_1_4:
    case MOD_NOR_PROGRAM_1_4_4:
    case MOD_NOR_PROGRAM_1_1_4_4BYTE:
    case MOD_NOR_PROGRAM_1_4_4_4BYTE:
        io_num = 4;
        break;
    }

    return io_num;
}

static bool is_read_address_4byte(enum mod_nor_read_mode mode)
{
    switch (mode) {
    case MOD_NOR_READ_4BYTE:
    case MOD_NOR_READ_FAST_4BYTE:
    case MOD_NOR_READ_FAST_1_1_2_4BYTE:
    case MOD_NOR_READ_FAST_1_2_2_4BYTE:
    case MOD_NOR_READ_FAST_1_1_4_4BYTE:
    case MOD_NOR_READ_FAST_1_4_4_4BYTE:
        return true;
    default:
        return false;
    }
}

static bool is_program_address_4byte(enum mod_nor_program_mode mode)
{
    switch (mode) {
    case MOD_NOR_PROGRAM_4BYTE:
    case MOD_NOR_PROGRAM_1_1_4_4BYTE:
    case MOD_NOR_PROGRAM_1_4_4_4BYTE:
        return true;
    default:
        return false;
    }
}

static bool is_erase_address_4byte(enum mod_nor_erase_mode mode)
{
    switch (mode) {
    case MOD_NOR_ERASE_SECTOR_4KB_4BYTE:
    case MOD_NOR_ERASE_SECTOR_32KB_4BYTE:
    case MOD_NOR_ERASE_BLOCK_4BYTE:
        return true;
    default:
        return false;
    }
}

static uint32_t get_erase_size(
    struct nor_dev_ctx *ctx,
    enum mod_nor_erase_mode mode)
{
    switch (mode) {
    case MOD_NOR_ERASE_SECTOR_4KB:
    case MOD_NOR_ERASE_SECTOR_4KB_4BYTE:
        return ERASE_SIZE_SECTOR_4KB;
    case MOD_NOR_ERASE_SECTOR_32KB:
    case MOD_NOR_ERASE_SECTOR_32KB_4BYTE:
        return ERASE_SIZE_SECTOR_32KB;
    case MOD_NOR_ERASE_BLOCK:
    case MOD_NOR_ERASE_BLOCK_4BYTE:
        return ctx->config->erase_block_size; /* block size is specified by
                                                 configuraiton data. */
    case MOD_NOR_ERASE_CHIP:
    default:
        return 0;
    }
}

static int configure_read_command(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    enum mod_nor_read_mode mode)
{
    enum mod_nor_command_idx command_idx;
    int status;

    command_idx = nor_read_command_index_list[mode];
    if (is_command_not_supported(ctx, command_idx)) {
        return FWK_E_SUPPORT;
    }

    status = set_io_protocol(id, ctx, get_read_io_num(mode));
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = set_4byte_address_mode(id, ctx, is_read_address_4byte(mode));
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = set_read_command(id, ctx, command_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return status;
}

static int nor_read(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    enum mod_nor_read_mode mode,
    uint32_t offset,
    void *buf,
    uint32_t req_len)
{
    int status;

    status = configure_read_command(id, ctx, mode);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return ctx->qspi_api->read(id, offset, buf, req_len);
}

static int nor_program(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    enum mod_nor_program_mode mode,
    uint32_t offset,
    void *buf,
    uint32_t req_len)
{
    enum mod_nor_command_idx command_idx;
    uint32_t len, total_len;
    uint32_t page_size, page_mask;
    int status;

    command_idx = nor_program_command_index_list[mode];
    if (is_command_not_supported(ctx, command_idx)) {
        return FWK_E_SUPPORT;
    }

    status = set_io_protocol(id, ctx, get_program_io_num(mode));
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = set_4byte_address_mode(id, ctx, is_program_address_4byte(mode));
    if (status != FWK_SUCCESS) {
        return status;
    }

    total_len = 0;
    page_mask = PAGE_MASK(ctx->config->program_page_size);
    page_size = ctx->config->program_page_size - (offset & page_mask);
    do {
        if ((req_len - total_len) < page_size) {
            len = req_len - total_len;
        } else {
            len = page_size;
        }

        status = program_erase_sequence(id, ctx, command_idx, offset, buf, len);
        if (status != FWK_SUCCESS) {
            return status;
        }

        offset += len;
        buf = (char *)buf + len;
        total_len += len;
        page_size = ctx->config->program_page_size;
    } while (total_len < req_len);

    return FWK_SUCCESS;
}

static int nor_erase(
    fwk_id_t id,
    struct nor_dev_ctx *ctx,
    enum mod_nor_erase_mode mode,
    uint32_t offset,
    uint32_t req_len)
{
    enum mod_nor_command_idx command_idx;
    uint32_t len, total_len;
    int status;

    command_idx = nor_erase_command_index_list[mode];
    if (is_command_not_supported(ctx, command_idx)) {
        return FWK_E_SUPPORT;
    }

    /* single I/O only */
    status = set_io_protocol(id, ctx, 1);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = set_4byte_address_mode(id, ctx, is_erase_address_4byte(mode));
    if (status != FWK_SUCCESS) {
        return status;
    }

    total_len = 0;
    len = get_erase_size(ctx, mode);
    do {
        status =
            program_erase_sequence(id, ctx, command_idx, offset, NULL, len);
        if (status != FWK_SUCCESS) {
            return status;
        }

        offset += len;
        total_len += len;
    } while (len != 0 && total_len < req_len);

    return FWK_SUCCESS;
}

static int nor_reset(fwk_id_t id, struct nor_dev_ctx *ctx)
{
    int status;

    /* prepare to reset */
    status = set_write_command(id, ctx, MOD_NOR_COMMAND_RESET_ENABLE);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* execute reset */
    status = set_write_command(id, ctx, MOD_NOR_COMMAND_RESET_EXECUTE);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * No commands accept during reset operation.
     * To avoid this dead time, it waits for a while.
     */
    status = nor_ctx.timer_api->delay(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0), RESET_EFFECTIVE_WAIT_US);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* wait until reset completes */
    status = wait_until_ready(id, ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

/*
 * Module API
 */
static int read(
    fwk_id_t id,
    uint8_t slave,
    enum mod_nor_read_mode mode,
    uint32_t offset,
    void *buf,
    uint32_t len)
{
    fwk_id_t sub_element_id;
    struct nor_dev_ctx *ctx;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(id);

    if (slave >= ctx->slave_num || mode >= MOD_NOR_READ_MODE_COUNT ||
        buf == NULL || len == 0) {
        return FWK_E_PARAM;
    }

    /* build sub_element_id which indicates slave */
    sub_element_id = fwk_id_build_sub_element_id(ctx->config->driver_id, slave);

    status = nor_read(sub_element_id, ctx, mode, offset, buf, len);

    /* reset I/O protocol and 4byte_address setting to default */
    set_single_3byte_address_mode(sub_element_id, ctx);

    return status;
}

static int program(
    fwk_id_t id,
    uint8_t slave,
    enum mod_nor_program_mode mode,
    uint32_t offset,
    void *buf,
    uint32_t len)
{
    fwk_id_t sub_element_id;
    struct nor_dev_ctx *ctx;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_PARAM;
    }

    ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(id);

    if (slave >= ctx->slave_num || mode >= MOD_NOR_PROGRAM_MODE_COUNT ||
        buf == NULL || len == 0) {
        return FWK_E_PARAM;
    }

    sub_element_id = fwk_id_build_sub_element_id(
        ctx->config->driver_id,
        slave); /* build sub_element_id which indicates slave */
    status = nor_program(sub_element_id, ctx, mode, offset, buf, len);

    /* reset I/O protocol and 4byte_address setting to default */
    set_single_3byte_address_mode(sub_element_id, ctx);

    return status;
}

static int erase(
    fwk_id_t id,
    uint8_t slave,
    enum mod_nor_erase_mode mode,
    uint32_t offset,
    uint32_t len)
{
    fwk_id_t sub_element_id;
    struct nor_dev_ctx *ctx;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_PARAM;
    }

    ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(id);

    if (slave >= ctx->slave_num || len == 0) {
        return FWK_E_PARAM;
    }

    /* build sub_element_id which indicates slave */
    sub_element_id = fwk_id_build_sub_element_id(ctx->config->driver_id, slave);
    status = nor_erase(sub_element_id, ctx, mode, offset, len);

    /* reset I/O protocol and 4byte_address setting to default */
    set_single_3byte_address_mode(sub_element_id, ctx);

    return status;
}

static int reset(fwk_id_t id, uint8_t slave)
{
    fwk_id_t sub_element_id;
    struct nor_dev_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_PARAM;
    }

    ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(id);

    if (slave >= ctx->slave_num) {
        return FWK_E_PARAM;
    }

    /* build sub_element_id which indicates slave */
    sub_element_id = fwk_id_build_sub_element_id(ctx->config->driver_id, slave);

    return nor_reset(sub_element_id, ctx);
}

static int configure_mmap_read(
    fwk_id_t id,
    uint8_t slave,
    enum mod_nor_read_mode mode,
    bool enable)
{
    fwk_id_t sub_element_id;
    struct nor_dev_ctx *ctx;
    int status = FWK_SUCCESS;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_PARAM;
    }

    ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(id);

    if (slave >= ctx->slave_num || mode >= MOD_NOR_READ_MODE_COUNT) {
        return FWK_E_PARAM;
    }

    /* build sub_element_id which indicates slave */
    sub_element_id = fwk_id_build_sub_element_id(ctx->config->driver_id, slave);

    if (enable) {
        status = configure_read_command(sub_element_id, ctx, mode);
    } else {
        /* reset I/O protocol and 4byte_address setting to default */
        set_single_3byte_address_mode(sub_element_id, ctx);
    }

    return status;
}

static struct mod_nor_api nor_api = {
    .read = read,
    .program = program,
    .erase = erase,
    .reset = reset,
    .configure_mmap_read = configure_mmap_read,
};

/*
 * Framework handlers
 */
static int nor_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_PANIC;
    }

    nor_ctx.dev_ctx = fwk_mm_alloc(element_count, sizeof(nor_ctx.dev_ctx[0]));

    return FWK_SUCCESS;
}

static int nor_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct nor_dev_ctx *ctx;
    int driver_sub_element_count;

    if (data == NULL) {
        return FWK_E_PANIC;
    }

    ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(element_id);

    ctx->config = data;
    if (!fwk_id_is_type(ctx->config->driver_id, FWK_ID_TYPE_ELEMENT) ||
        !fwk_id_is_type(ctx->config->api_id, FWK_ID_TYPE_API) ||
        ctx->config->program_page_size == 0 ||
        ctx->config->erase_block_size == 0 ||
        ctx->config->command_table == NULL || ctx->config->operation == NULL) {
        return FWK_E_PARAM;
    }

    /* get number of slave from driver module */
    driver_sub_element_count =
        fwk_module_get_sub_element_count(ctx->config->driver_id);
    if (driver_sub_element_count == FWK_E_PARAM) {
        return FWK_E_PARAM;
    }
    ctx->slave_num = (uint8_t)driver_sub_element_count;

    return FWK_SUCCESS;
}

static int nor_bind(fwk_id_t id, unsigned int round)
{
    struct nor_dev_ctx *ctx;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(id);
        return fwk_module_bind(
            ctx->config->driver_id, ctx->config->api_id, &ctx->qspi_api);
    }

    return fwk_module_bind(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        MOD_TIMER_API_ID_TIMER,
        &nor_ctx.timer_api);
}

static int nor_start(fwk_id_t id)
{
    struct nor_dev_ctx *ctx;
    uint8_t slave;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    ctx = nor_ctx.dev_ctx + fwk_id_get_element_idx(id);

    if (ctx->config->enable_reset_on_boot) {
        for (slave = 0; slave < ctx->slave_num; slave++) {
            status = reset(id, slave);
            /*
             * system can continue the operation normally
             * even if "reset" isn't supported.
             */
            if (status != FWK_SUCCESS && status != FWK_E_SUPPORT) {
                return status;
            }
        }
    }

    return FWK_SUCCESS;
}

static int nor_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_NOR_API_TYPE_DEFAULT:
        *api = &nor_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_nor = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_NOR_API_TYPE_COUNT,
    .init = nor_init,
    .element_init = nor_element_init,
    .bind = nor_bind,
    .start = nor_start,
    .process_bind_request = nor_process_bind_request,
};
