/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "nor/device_nor_w25.h"
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

static bool w25_is_write_enable(fwk_id_t id, const struct qspi_api *qspi_api)
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

    return W25_IS_WEL_ENABLE(buf);
}

static int w25_read_status_reg_2(
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

static int w25_write_status_reg_2(
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

/*
 * User APIs
 */
int w25_nor_set_io_protocol(
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

    status = w25_read_status_reg_2(id, qspi_api, &buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (!req_quad || W25_IS_QUAD_ENABLE(buf)) {
        return FWK_SUCCESS;
    }

    if (!w25_is_write_enable(id, qspi_api)) {
        return FWK_E_ACCESS;
    }

    /* At here, quad transfer is requested but not yet enabled. */
    W25_QUAD_ENABLE(buf);

    status = w25_write_status_reg_2(id, qspi_api, buf);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

int w25_nor_set_4byte_addr_mode(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg)
{
    bool enable = *(bool *)arg;
    struct qspi_command command;

    if (enable) {
        command = W25_COMMAND_ENTER_4BYTE;
    } else {
        command = W25_COMMAND_EXIT_4BYTE;
    }

    return set_write_command(id, qspi_api, command);
}
