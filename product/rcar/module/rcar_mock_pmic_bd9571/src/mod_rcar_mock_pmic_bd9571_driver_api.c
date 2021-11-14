/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* The use of "subordinate" may not be in sync with platform documentation */

#include <mod_rcar_mock_pmic_bd9571_private.h>
#include <mod_psu.h>
#include <mod_power_domain.h>

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static int api_set_enabled(fwk_id_t device_id, bool enable)
{
    struct mod_rcar_mock_pmic_device_ctx *ctx;

    ctx = __mod_rcar_mock_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    ctx->enabled = enable;

    return FWK_SUCCESS;
}

static int api_get_enabled(fwk_id_t device_id, bool *enabled)
{
    struct mod_rcar_mock_pmic_device_ctx *ctx;

    ctx = __mod_rcar_mock_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *enabled = ctx->enabled;

    return FWK_SUCCESS;
}

static int api_set_voltage(fwk_id_t device_id, uint32_t voltage)
{
    struct mod_rcar_mock_pmic_device_ctx *ctx;
    uint8_t val;

    ctx = __mod_rcar_mock_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (voltage < BD9571MWV_MIN_MV * 1000 || voltage > BD9571MWV_MAX_MV * 1000)
        return -1;

    val = DIV_ROUND(voltage, BD9571MWV_STEP_MV * 1000);
    val &= REG_DATA_DVFS_SetVID_MASK;

    int ret;
    ret = rcar_iic_dvfs_send(SUBORDINATE_ADDR_PMIC, REG_ADDR_DVFS_SetVID, val);
    if (ret) {
        return ret;
    }

    ctx->voltage = voltage;
    return FWK_SUCCESS;
}

static int api_get_voltage(fwk_id_t device_id, uint32_t *voltage)
{
    struct mod_rcar_mock_pmic_device_ctx *ctx;

    ctx = __mod_rcar_mock_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *voltage = ctx->voltage;

    return FWK_SUCCESS;
}

static int api_set_pmic(fwk_id_t device_id, unsigned int state)
{
    struct mod_rcar_mock_pmic_device_ctx *ctx;
    uint8_t val;
    int ret;

    ctx = __mod_rcar_mock_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (state == MOD_PD_STATE_ON)
        val = REG_DATA_DDR_BKUP_ON;
    else
        val = REG_DATA_DDR_BKUP_OFF;

    /* Not supported because I2C used in kernel */
    ret = rcar_iic_dvfs_send(SUBORDINATE_ADDR_PMIC, REG_ADDR_DDR_BKUP, val);
    if (ret) {
        return ret;
    }

    return FWK_SUCCESS;
}

const struct mod_rcar_pmic_driver_api __mod_rcar_mock_pmic_driver_api = {
    .set_enabled = api_set_enabled,
    .get_enabled = api_get_enabled,
    .set_voltage = api_set_voltage,
    .get_voltage = api_get_voltage,
    .set_pmic = api_set_pmic,
};
