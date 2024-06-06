/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2024, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <system_mmap.h>

#include <mod_reg_sensor.h>
#include <mod_sensor.h>
#include <mod_rcar_system.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdint.h>

/* default values if FUSEs are missing */
/* TODO: Read values from hardware on supported platforms */
int ptat[3] = { 2631, 1509, 435 };
int thcode[TSC_MAX_NUM][3] = {
    { 3397, 2800, 2221 },
    { 3393, 2795, 2216 },
    { 3389, 2805, 2237 },
};
struct rcar_gen3_thermal_tsc *tscs[TSC_MAX_NUM];

static struct mod_reg_sensor_dev_config **config_table;
static const int rcar_gen3_ths_tj_1 = 126;

static inline uint32_t rcar_gen3_thermal_read(
    struct rcar_gen3_thermal_tsc *tsc,
    uint32_t reg)
{
    return mmio_read_32(tsc->base + reg);
}

static inline void rcar_gen3_thermal_write(
    struct rcar_gen3_thermal_tsc *tsc,
    uint32_t reg,
    uint32_t data)
{
    mmio_write_32(tsc->base + reg, data);
}

static void rcar_gen3_thermal_calc_coefs(
    struct rcar_gen3_thermal_tsc *tsc,
    const int *ptat,
    const int *thcode,
    int ths_tj_1)
{
    /* TODO: Find documentation and document constant calculation formula */

    /*
     * Division is not scaled in BSP and if scaled it might overflow
     * the dividend (4095 * 4095 << 14 > INT_MAX) so keep it unscaled
     */
    tsc->tj_t = (FIXPT_INT((ptat[1] - ptat[2]) * 157) / (ptat[0] - ptat[2])) +
        FIXPT_INT(TJ_3);

    tsc->coef.a1 = FIXPT_DIV(
        FIXPT_INT(thcode[1] - thcode[2]), tsc->tj_t - FIXPT_INT(TJ_3));
    tsc->coef.b1 = FIXPT_INT(thcode[2]) - tsc->coef.a1 * TJ_3;

    tsc->coef.a2 = FIXPT_DIV(
        FIXPT_INT(thcode[1] - thcode[0]), tsc->tj_t - FIXPT_INT(ths_tj_1));
    tsc->coef.b2 = FIXPT_INT(thcode[0]) - tsc->coef.a2 * ths_tj_1;
}

static int rcar_gen3_thermal_round(int temp)
{
    int result, round_offs;

    round_offs = temp >= 0 ? RCAR3_THERMAL_GRAN / 2 : -RCAR3_THERMAL_GRAN / 2;
    result = (temp + round_offs) / RCAR3_THERMAL_GRAN;
    return result * RCAR3_THERMAL_GRAN;
}

static int rcar_gen3_thermal_get_temp(void *devdata, int *temp)
{
    struct rcar_gen3_thermal_tsc *tsc = devdata;
    int mcelsius, val;
    int reg;

    /* Read register and convert to mili Celsius */
    reg = rcar_gen3_thermal_read(tsc, REG_GEN3_TEMP) & CTEMP_MASK;

    if (reg <= thcode[tsc->id][1])
        val = FIXPT_DIV(FIXPT_INT(reg) - tsc->coef.b1, tsc->coef.a1);
    else
        val = FIXPT_DIV(FIXPT_INT(reg) - tsc->coef.b2, tsc->coef.a2);
    mcelsius = FIXPT_TO_MCELSIUS(val);

    /* Make sure we are inside specifications */
    if ((mcelsius < MCELSIUS(TEMP_LOWER_LIMIT)) ||
        (mcelsius > MCELSIUS(TEMP_UPPER_LIMIT)))
        return -1 /*-EIO*/;

    /* Round value to device granularity setting */
    *temp = rcar_gen3_thermal_round(mcelsius);

    return 0;
}

/*
 * Module API
 */
static int get_value(fwk_id_t id, mod_sensor_value_t *value)
{
#ifdef BUILD_HAS_SENSOR_SIGNED_VALUE
    return FWK_E_SUPPORT;
#else
    int tmp = 0;
    int64_t *ivalue;

    if (value == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    if (rcar_gen3_thermal_get_temp(tscs[fwk_id_get_element_idx(id)], &tmp))
        return FWK_E_DATA;

    ivalue = (void *)value;
    *ivalue = (int64_t)tmp;

    return FWK_SUCCESS;
#endif
}

static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    struct mod_reg_sensor_dev_config *config;

    config = config_table[fwk_id_get_element_idx(id)];
    fwk_assert(config != NULL);

    if (info == NULL)
        return FWK_E_PARAM;

    *info = *(config->info);

    return FWK_SUCCESS;
}

static int reg_sensor_resume(void)
{
    int i;

    for (i = 0; i < TSC_MAX_NUM; i++) {
        if (tscs[i]) {
            rcar_gen3_thermal_write(tscs[i], REG_GEN3_THCTR, THCTR_THSST);
        }
    }
    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api reg_sensor_api = {
    .get_value = get_value,
    .get_info = get_info,
};

static const struct mod_rcar_system_drv_api api_system = {
    .resume = reg_sensor_resume,
};

/*
 * Framework handlers
 */
static int reg_sensor_start(fwk_id_t id)
{
    int eid, pid;
    struct rcar_gen3_thermal_tsc *tsc;
    struct mod_reg_sensor_dev_config *config;

    /* for Module */
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    /* for Elements */
    eid = fwk_id_get_element_idx(id);
    tsc = fwk_mm_alloc(1, sizeof(*tsc));
    if (!tsc)
        return FWK_E_NOMEM;

    config = config_table[eid];
    pid = ADR2INDEX(config->reg);
    if (pid < 0)
        return FWK_E_DATA;
    tsc->base = config->reg;
    rcar_gen3_thermal_calc_coefs(tsc, ptat, thcode[pid], rcar_gen3_ths_tj_1);
    tsc->id = pid;
    tscs[eid] = tsc;
    rcar_gen3_thermal_write(tsc, REG_GEN3_THCTR, THCTR_THSST);

    return FWK_SUCCESS;
}

static int reg_sensor_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    config_table = fwk_mm_alloc(element_count, sizeof(*config_table));
    if (config_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int reg_sensor_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_reg_sensor_dev_config *config =
        (struct mod_reg_sensor_dev_config *)data;

    if (config->reg == 0)
        return FWK_E_DATA;

    config_table[fwk_id_get_element_idx(element_id)] = config;

    return FWK_SUCCESS;
}

static int reg_sensor_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_type,
    const void **api)
{

    switch (fwk_id_get_api_idx(api_type)) {
    case MOD_RCAR_REG_SENSOR_API_TYPE_SYSTEM:
        *api = &api_system;
        break;
    default:
        *api = &reg_sensor_api;
        break;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_reg_sensor = {
    .api_count = MOD_RCAR_REG_SENSOR_API_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = reg_sensor_init,
    .start = reg_sensor_start,
    .element_init = reg_sensor_element_init,
    .process_bind_request = reg_sensor_process_bind_request,
};
