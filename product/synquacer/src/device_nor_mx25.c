/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "nor/device_nor_mx25.h"
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

static int mx25_read_status_reg(
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

static int mx25_write_status_reg(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    uint8_t buf)
{
    int status;

    status = set_write_command(id, qspi_api, COMMAND_WRITE_STATUS_REG);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return qspi_api->write(id, 0, &buf, sizeof(buf));
}

static int mx25_read_security_reg(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    uint8_t *buf)
{
    int status;

    status = set_read_command(id, qspi_api, MX25_COMMAND_READ_SECURITY_REG);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return qspi_api->read(id, 0, buf, sizeof(buf[0]));
}

/*
 * User APIs
 */
int mx25_nor_set_io_protocol(
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

    status = mx25_read_status_reg(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (!req_quad || MX25_IS_QUAD_ENABLE(buf)) {
        return FWK_SUCCESS;
    }

    if (!MX25_IS_WEL_ENABLE(buf)) {
        return FWK_E_ACCESS;
    }

    /* At here, quad transfer is requested but not yet enabled. */
    MX25_QUAD_ENABLE(buf);

    status = mx25_write_status_reg(id, qspi_api, buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

int mx25_nor_set_4byte_addr_mode(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    bool enable = *(bool *)arg;
    struct qspi_command command;

    if (enable) {
        command = MX25_COMMAND_ENTER_4BYTE;
    } else {
        command = MX25_COMMAND_EXIT_4BYTE;
    }

    return set_write_command(id, qspi_api, command);
}

int mx25_nor_get_program_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    uint8_t buf;
    bool *is_fail = (bool *)arg;
    int status;

    status = mx25_read_security_reg(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (MX25_IS_PROGRAM_FAIL(buf)) {
        *is_fail = true;
    } else {
        *is_fail = false;
    }

    return FWK_SUCCESS;
}

int mx25_nor_get_erase_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    uint8_t buf;
    bool *is_fail = (bool *)arg;
    int status;

    status = mx25_read_security_reg(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (MX25_IS_ERASE_FAIL(buf)) {
        *is_fail = true;
    } else {
        *is_fail = false;
    }

    return FWK_SUCCESS;
}
