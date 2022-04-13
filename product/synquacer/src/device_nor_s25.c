/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "nor/device_nor_s25.h"
#include "qspi_api.h"

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * local functions
 */
static int set_read_command(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    struct qspi_command command)
{
    if (IS_QSPI_COMMAND_EMPTY(command)) {
        return FWK_E_SUPPORT;
    }

    return qspi_api->set_read_command(id, &command);
}

static int set_write_command(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    struct qspi_command command)
{
    if (IS_QSPI_COMMAND_EMPTY(command)) {
        return FWK_E_SUPPORT;
    }

    return qspi_api->set_write_command(id, &command);
}

static bool s25_is_write_enable(fwk_id_t id, const struct qspi_api *qspi_api)
{
    int status;
    uint8_t buf;

    status = set_read_command(id, qspi_api, COMMAND_READ_STATUS_REG);
    if (status != FWK_SUCCESS) {
        return false;
    }

    status = qspi_api->read(id, 0, &buf, sizeof(buf));
    if (status != FWK_SUCCESS) {
        return false;
    }

    return S25_IS_WEL_ENABLE(buf);
}

static int s25_read_status_reg(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    uint8_t *buf)
{
    int status;

    status = set_read_command(id, qspi_api, COMMAND_READ_STATUS_REG);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return qspi_api->read(id, 0, buf, sizeof(buf[0]));
}

static int s25_read_config_reg(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    uint8_t *buf)
{
    int status;

    status = set_read_command(id, qspi_api, S25_COMMAND_READ_CONFIG_REG);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return qspi_api->read(id, 0, buf, sizeof(buf[0]));
}

static int s25_write_config_reg(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    uint8_t buf)
{
    int status;
    uint8_t write_buf[2];

    status = s25_read_status_reg(id, qspi_api, &write_buf[0]);
    if (status != FWK_SUCCESS) {
        return status;
    }

    write_buf[1] = buf;

    status = set_write_command(id, qspi_api, S25_COMMAND_WRITE_CONFIG_REG);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return qspi_api->write(id, 0, &write_buf, sizeof(write_buf));
}

static int s25_read_bank_reg(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    uint8_t *buf)
{
    int status;

    status = set_read_command(id, qspi_api, S25_COMMAND_READ_BANK_REG);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return qspi_api->read(id, 0, buf, sizeof(buf[0]));
}

static int s25_write_bank_reg(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    uint8_t buf)
{
    int status;

    status = set_write_command(id, qspi_api, S25_COMMAND_WRITE_BANK_REG);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return qspi_api->write(id, 0, &buf, sizeof(buf));
}

/*
 * User APIs
 */
int s25_nor_set_io_protocol(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    uint8_t io_num = *(uint8_t *)arg;
    uint8_t buf;
    bool req_quad;
    int status;

    if (io_num != 1 && io_num != 2 && io_num != 4) {
        return FWK_E_PARAM;
    }

    req_quad = (io_num == 4);

    status = s25_read_config_reg(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (!req_quad || S25_IS_QUAD_ENABLE(buf)) {
        return FWK_SUCCESS;
    }

    if (!s25_is_write_enable(id, qspi_api)) {
        return FWK_E_ACCESS;
    }

    /* At here, quad transfer is requested but not yet enabled. */
    S25_QUAD_ENABLE(buf);

    status = s25_write_config_reg(id, qspi_api, buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

int s25_nor_set_4byte_addr_mode(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    bool enable = *(bool *)arg;
    uint8_t buf;
    int status;

    status = s25_read_bank_reg(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (enable == S25_IS_4BYTE_ENABLE(buf)) {
        return FWK_SUCCESS;
    }

    if (enable) {
        S25_4BYTE_ENABLE(buf);
    } else {
        S25_4BYTE_DISABLE(buf);
    }

    // not require the WREN before this command

    status = s25_write_bank_reg(id, qspi_api, buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

int s25_nor_get_program_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    uint8_t buf;
    bool *is_fail = (bool *)arg;
    int status;

    status = s25_read_status_reg(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (S25_IS_PROGRAM_FAIL(buf)) {
        *is_fail = true;
    } else {
        *is_fail = false;
    }

    return FWK_SUCCESS;
}

int s25_nor_get_erase_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    uint8_t buf;
    bool *is_fail = (bool *)arg;
    int status;

    status = s25_read_status_reg(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (S25_IS_ERASE_FAIL(buf)) {
        *is_fail = true;
    } else {
        *is_fail = false;
    }

    return FWK_SUCCESS;
}
