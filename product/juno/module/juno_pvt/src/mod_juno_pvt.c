/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno PVT Sensors driver.
 */

#include "juno_id.h"
#include "juno_pvt.h"
#include "pvt_sensor_calibration.h"
#include "system_clock.h"

#include <mod_juno_pvt.h>
#include <mod_power_domain.h>
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stddef.h>

/* Location Identifier */
#define PVTGROUP_GROUP_INFO_LOC                 UINT32_C(0x00000001)

/* Group Based Register Identifier */
#define PVTGROUP_GROUP_INFO_LOC_GROUP_LITE      UINT32_C(0x00000001)

/* Number of sensors in Group Field Mask */
#define PVTGROUP_SENSOR_COUNT_MASK              UINT32_C(0x0000003E)

/* Group Measurement Enable */
#define PVTGROUP_MEASUREMENT_ENABLE             UINT32_C(0x00000001)

/* Sensor Measurement mask for a sensor */
#define SAMPLE_VALUE_MASK                       UINT32_C(0x0000FFFF)

/* Sample Window time for a sensor */
#define SAMPLE_WINDOW_MASK                      UINT32_C(0x0000FFFF)

/* Interrupt masks */
#define IRQ_MASK_DATA_VALID      UINT32_C(0x00000004)
#define IRQ_MASK_ALL             UINT32_C(0x0000000F)
#define IRQ_MASK_ENABLED         IRQ_MASK_DATA_VALID
#define IRQ_MASK_DISABLED        ~IRQ_MASK_ENABLED

/* Sampling Options */
#define FULL_SCALE_TEMP     200         /* Degrees Celsius */
#define FULL_SCALE_MVOLT    1000        /* Milli Volts */

/* Constant temperature offset for Juno R1 & R2 [millidegrees] */
#define R1_TEMP_OFFSET      9000

#define REFCLK_KHZ (CLOCK_RATE_REFCLK / 1000)

/* Module context */
struct pvt_ctx {
    /* Board revision */
    enum juno_idx_revision board_rev;

    /* Flag indicating whether the driver is disabled */
    bool driver_is_disabled;
};

/*
 * Sensors are grouped in elements and each sensor is represented by a
 * sub-element.
 */

/* Group (element) context */
struct pvt_dev_ctx {
    /*
     * Pointer to table of individual sensors (sub-elements) configuration data
     */
    struct mod_juno_pvt_dev_config *sensor_cfg_table;

    /* Pointer to the table of sensor context */
    struct pvt_sub_dev_ctx *sensor_ctx_table;

    /* Identifier of the sensor (sub-element) being processed */
    fwk_id_t sensor_read_id;

    /* Sensor Driver Input API */
    const struct mod_sensor_driver_response_api *driver_response_api;

    /* Tracker to Power Domain ON state */
    bool pd_state_on;

    /* Cookie of the notification to respond to */
    uint32_t cookie;

    /*
     * Power Domain notification delayed flag
     *
     * If a pre-state notification is received when the sensor reading is in
     * progress, the response to the notification is delayed until the sensor
     * reading is completed.
     */
    bool pd_notification_delayed;
};

/* Sensor (sub-element) context */
struct pvt_sub_dev_ctx {
    /* Last raw reading from the sensor */
    uint32_t last_reading;

    /* Sample Window for measurement */
    unsigned int sample_window;

    /* Slope coefficient for measurement */
    int slope_m;

    /* Offset coefficient for measurement */
    int freq_b;

    /* Sensor HAL Identifier */
    fwk_id_t sensor_hal_id;
};

/* Events indices */
enum pvt_event_idx {
    JUNO_PVT_EVENT_IDX_READ_REQUEST,
    JUNO_PVT_EVENT_IDX_DATA_READY,
    JUNO_PVT_EVENT_IDX_COUNT
};

static struct pvt_ctx mod_ctx;
static struct pvt_dev_ctx *dev_ctx;

/*
 * Static helper functions
 */

/*
 * For the temperature-based sensors, there are two calibration points:
 * 45 degrees C and 85 degrees C
 *
 * The maths in this function considers temperature on the X-axis and
 * sensor readings on the Y-axis.
 */
static int process_pvt_calibration(struct pvt_sub_dev_ctx *sensor_ctx,
                                   uint16_t cal_45C,
                                   uint16_t cal_85C)
{
    unsigned int freq_45;
    unsigned int freq_85;
    unsigned int freq_fs;
    unsigned int sample_window;
    int freq_b;
    int slope_m;

    if ((cal_45C == 0) || (cal_85C == 0))
        return FWK_E_PARAM;

    /* Convert into kHz */
    freq_45 = cal_45C * 10;
    freq_85 = cal_85C * 10;

    /* Find the slope */
    slope_m = (freq_85 - freq_45) / (85 - 45);
    if (!fwk_expect(slope_m != 0))
        return FWK_E_PARAM;

    /* Find the intercept of the line */
    freq_b = freq_85 - (slope_m * 85);

    /* Use line equation to find full-scale frequency */
    freq_fs = (slope_m * FULL_SCALE_TEMP) + freq_b;

    /*
     * freq_fs must always be bigger than or equal to REFCLK_KHZ or
     * sample_window calculation won't work.
     */
    if (freq_fs < REFCLK_KHZ)
        return FWK_E_RANGE;

    if (freq_fs == 0)
        return FWK_E_PARAM;

    /* Calculate sample window to fit the full scale reading */
    sample_window = (SAMPLE_WINDOW_MASK * REFCLK_KHZ) / freq_fs;
    if (sample_window > SAMPLE_WINDOW_MASK)
        return FWK_E_PARAM;

    /* Store constants for run-time calculations */
    sensor_ctx->slope_m = slope_m;
    sensor_ctx->freq_b = freq_b;
    sensor_ctx->sample_window = sample_window;

    return FWK_SUCCESS;
}

/*
 * For the ring oscillator based sensors, there are two calibration points:
 * 810mV and 900mV
 *
 * The maths in this function considers mV on the X-axis and sensor readings
 * on the Y-axis.
 */
static int process_osc_calibration(struct pvt_sub_dev_ctx *sensor_ctx,
                                   uint16_t cal_810,
                                   uint16_t cal_900)
{
    unsigned int freq_810;
    unsigned int freq_900;
    unsigned int freq_fs;
    unsigned int freq_b;
    unsigned int sample_window;
    unsigned int slope_m;

    if ((cal_810 == 0) || (cal_900 == 0))
        return FWK_E_PARAM;

    /* Convert into kHz */
    freq_810 = cal_810 * 20;
    freq_900 = cal_900 * 20;

    /* Find the slope */
    slope_m = (freq_900 - freq_810) / (900 - 810);

    /* Find the intercept of the line */
    freq_b = freq_900 - (slope_m * 900);

    /* Use line equation to find full-scale frequency */
    freq_fs = (slope_m * FULL_SCALE_MVOLT) + freq_b;

    /*
     * freq_fs must be always bigger than or equal to REFCLK_KHZ or
     * sample_window calculation won't work.
     */
    if (freq_fs < REFCLK_KHZ)
        return FWK_E_RANGE;

    if (freq_fs == 0)
        return FWK_E_PARAM;

    /* Calculate sample window to fit the full scale reading */
    sample_window = (SAMPLE_WINDOW_MASK * REFCLK_KHZ) / freq_fs;
    if (sample_window > SAMPLE_WINDOW_MASK)
        return FWK_E_PARAM;

    /* Store constants for the run-time calculations */
    sensor_ctx->slope_m = slope_m;
    sensor_ctx->freq_b = freq_b;
    sensor_ctx->sample_window = sample_window;

    return FWK_SUCCESS;
}

static void pvt_interrupt_handler(uintptr_t param)
{
    struct mod_juno_pvt_dev_config *sensor_cfg;
    uint32_t osc_counter = 0, sensor_value;
    int freq_khz;
    uint64_t value = 0;
    int status = FWK_E_PARAM;
    struct fwk_event event;
    struct pvt_dev_ctx *group_ctx;
    struct pvt_sub_dev_ctx *sensor_ctx;
    struct mod_sensor_driver_resp_params *isr_params =
        (struct mod_sensor_driver_resp_params *)event.params;
    uint8_t sub_elt_idx;

    group_ctx = (struct pvt_dev_ctx *)param;
    sub_elt_idx = fwk_id_get_sub_element_idx(group_ctx->sensor_read_id);
    sensor_cfg = &group_ctx->sensor_cfg_table[sub_elt_idx];
    sensor_ctx = &group_ctx->sensor_ctx_table[sub_elt_idx];

    sensor_cfg->group->regs->IRQ_CLEAR = IRQ_MASK_ALL;

    if ((sensor_cfg->group->regs->SENSOR_DATA_VALID &
        (1 << sensor_cfg->index))) {
        osc_counter = sensor_cfg->group->regs->SENSOR_DATA[sensor_cfg->index] &
            SAMPLE_VALUE_MASK;
    } else {
        /* Return the last raw reading */
        value = (uint64_t)sensor_ctx->last_reading;
        status = FWK_SUCCESS;

        goto exit;
    }

    sensor_ctx->last_reading = osc_counter;

    if (!fwk_expect(sensor_ctx->sample_window != 0))
        goto exit;

    freq_khz = (osc_counter * REFCLK_KHZ) / sensor_ctx->sample_window;

    fwk_assert(sensor_ctx->slope_m != 0);

    sensor_value = ((freq_khz - sensor_ctx->freq_b) * 1000) /
        sensor_ctx->slope_m;

    if (sensor_cfg->type == JUNO_PVT_TYPE_TEMP) {

        if ((mod_ctx.board_rev == JUNO_IDX_REVISION_R1) ||
            (mod_ctx.board_rev == JUNO_IDX_REVISION_R2))
            sensor_value -= R1_TEMP_OFFSET;

        value = (uint64_t)sensor_value;
        status = FWK_SUCCESS;
    } else if (sensor_cfg->type == JUNO_PVT_TYPE_VOLT) {
        /* Convert to millivolts */
        sensor_value /= 1000;

        value = (uint64_t)sensor_value;
        status = FWK_SUCCESS;
    } else
        status = FWK_E_PARAM;

exit:
    event = (struct fwk_event) {
        .target_id = fwk_id_build_element_id(
            fwk_module_id_juno_pvt,
            fwk_id_get_element_idx(group_ctx->sensor_read_id)),
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_JUNO_PVT),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_JUNO_PVT,
                           JUNO_PVT_EVENT_IDX_DATA_READY),
    };

    isr_params->status = status;
    isr_params->value = value;

    status = fwk_thread_put_event(&event);
    fwk_assert(status == FWK_SUCCESS);
}

static int respond(struct pvt_dev_ctx *group_ctx)
{
    struct pvt_sub_dev_ctx *sensor_ctx;

    /* The request to initiate the reading failed, respond back */
    struct mod_sensor_driver_resp_params resp_params = {0};
    resp_params.status = FWK_E_STATE;

    sensor_ctx = &group_ctx->sensor_ctx_table[
        fwk_id_get_sub_element_idx(group_ctx->sensor_read_id)];

    group_ctx->driver_response_api->reading_complete(
        sensor_ctx->sensor_hal_id,
        &resp_params);

    group_ctx->sensor_read_id = FWK_ID_NONE;

    return FWK_E_STATE;
}

/*
 * PVT driver API functions
 */
static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    struct mod_juno_pvt_dev_config *sensor_cfg;
    struct pvt_dev_ctx *group_ctx;

    if (mod_ctx.driver_is_disabled)
        return FWK_E_DEVICE;

    group_ctx = &dev_ctx[fwk_id_get_element_idx(id)];
    sensor_cfg = &group_ctx->sensor_cfg_table[fwk_id_get_sub_element_idx(id)];

    fwk_assert(sensor_cfg != NULL);

    *info = *(sensor_cfg->info);

    return FWK_SUCCESS;
}

static int get_value(fwk_id_t id, uint64_t *value)
{
    uint8_t elt_idx;
    struct pvt_dev_ctx *group_ctx;
    struct fwk_event read_req;
    int status;

    if (mod_ctx.driver_is_disabled)
        return FWK_E_DEVICE;

    elt_idx = fwk_id_get_element_idx(id);
    group_ctx = &dev_ctx[elt_idx];

    if (!group_ctx->pd_state_on)
        return FWK_E_PWRSTATE;

    if (fwk_id_is_equal(group_ctx->sensor_read_id, FWK_ID_NONE)) {
        /* No other sensors within the group are being read, mark this one */
        group_ctx->sensor_read_id = id;

        read_req = (struct fwk_event) {
            .target_id = fwk_id_build_element_id(fwk_module_id_juno_pvt,
                                                 elt_idx),
            .id = FWK_ID_EVENT(FWK_MODULE_IDX_JUNO_PVT,
                               JUNO_PVT_EVENT_IDX_READ_REQUEST),
        };

        status = fwk_thread_put_event(&read_req);
    } else {
        /* At least one sensor is being read, the sensor group is busy */
        status = FWK_E_BUSY;
    }

    if (status == FWK_SUCCESS)
        return FWK_PENDING;

    return status;
}

static const struct mod_sensor_driver_api pvt_sensor_api = {
    .get_info = get_info,
    .get_value = get_value,
};

/*
 * Framework handler functions
 */

static int juno_pvt_init(fwk_id_t module_id,
                         unsigned int element_count,
                         const void *data)
{
    int status;
    enum juno_idx_platform plat;

    if (element_count == 0)
        return FWK_E_PARAM;

    dev_ctx = fwk_mm_calloc(element_count, sizeof(struct pvt_dev_ctx));

    status = juno_id_get_platform(&plat);
    if (status != FWK_SUCCESS)
        return status;

    if (plat == JUNO_IDX_PLATFORM_FVP)
        mod_ctx.driver_is_disabled = true;

    status = juno_id_get_revision(&mod_ctx.board_rev);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int juno_pvt_element_init(fwk_id_t element_id,
                                 unsigned int sub_element_count,
                                 const void *data)
{
    struct pvt_dev_ctx *group_ctx;

    if (mod_ctx.driver_is_disabled)
        return FWK_SUCCESS;

    /* When no sub-elements are defined, the group is not defined */
    if (sub_element_count == 0)
        return FWK_SUCCESS;

    group_ctx = &dev_ctx[fwk_id_get_element_idx(element_id)];

    group_ctx->sensor_ctx_table =
        fwk_mm_calloc(sub_element_count, sizeof(struct pvt_sub_dev_ctx));

    group_ctx->sensor_cfg_table = (struct mod_juno_pvt_dev_config *)data;
    group_ctx->sensor_read_id = FWK_ID_NONE;

    return FWK_SUCCESS;
}

static int juno_pvt_bind(fwk_id_t id, unsigned int round)
{
    int status;
    int sub_element_count;
    struct pvt_dev_ctx *group_ctx;
    struct pvt_sub_dev_ctx *sensor_ctx;

    /* Bind in the second round */
    if ((round == 0) ||
        mod_ctx.driver_is_disabled ||
        fwk_module_is_valid_module_id(id))
        return FWK_SUCCESS;

    group_ctx = &dev_ctx[fwk_id_get_element_idx(id)];

    sub_element_count = fwk_module_get_sub_element_count(id);
    if (sub_element_count < 0)
        return FWK_E_DATA;

    /*
     * When no sub-elements are defined, there sensor group does not require
     * binding.
     */
    if (sub_element_count == 0)
        return FWK_SUCCESS;

    /*
     * Bind to sensor HAL module
     *
     * The first sensor sub_element identifier for each group is used to bind to
     * sensor module. The sensor HAL module has already bound to us at this
     * point, so we know that sensor_hal_id is initialized correctly.
     */
    sensor_ctx = &group_ctx->sensor_ctx_table[0];

    status = fwk_module_bind(sensor_ctx->sensor_hal_id,
                             mod_sensor_api_id_driver_response,
                             &group_ctx->driver_response_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int juno_pvt_process_bind_request(fwk_id_t source_id,
                                         fwk_id_t target_id,
                                         fwk_id_t api_type,
                                         const void **api)
{
    struct pvt_dev_ctx *group_ctx;
    struct pvt_sub_dev_ctx *sensor_ctx;

    if ((!fwk_module_is_valid_element_id(source_id)) ||
        (!fwk_module_is_valid_sub_element_id(target_id)) ||
        (fwk_id_get_module_idx(source_id) != FWK_MODULE_IDX_SENSOR) ||
        (mod_ctx.driver_is_disabled))
        return FWK_E_ACCESS;

    group_ctx = &dev_ctx[fwk_id_get_element_idx(target_id)];
    sensor_ctx = &group_ctx->sensor_ctx_table[
        fwk_id_get_sub_element_idx(target_id)];

    sensor_ctx->sensor_hal_id = source_id;

    *api = &pvt_sensor_api;

    return FWK_SUCCESS;
}

static int pvt_start(fwk_id_t id)
{
    int status;
    struct mod_juno_pvt_dev_config *sensor_cfg;
    int sub_element_count;
    struct pvt_dev_ctx *group_ctx;
    struct pvt_sub_dev_ctx *sensor_ctx;
    uint16_t calibration_a;
    uint16_t calibration_b;
    uint8_t sub_elem_ix;

    if (mod_ctx.driver_is_disabled)
        return FWK_SUCCESS;

    if (fwk_module_is_valid_module_id(id)) {

        /* Verify the board has the calibration data */
        if ((JUNO_PVT_CALIBRATION->PART_ID_PREFIX != 'C') &&
            (JUNO_PVT_CALIBRATION->PART_ID_PREFIX != 'D') &&
            (JUNO_PVT_CALIBRATION->PART_ID_PREFIX != 'E') &&
            (JUNO_PVT_CALIBRATION->PART_ID_PREFIX != 'F') &&
            (JUNO_PVT_CALIBRATION->PART_ID_PREFIX != 'G')) {
            mod_ctx.driver_is_disabled = true;

            return FWK_E_PARAM;
        }

        return FWK_SUCCESS;
    }

    group_ctx = &dev_ctx[fwk_id_get_element_idx(id)];

    sub_element_count = fwk_module_get_sub_element_count(id);
    if (sub_element_count < 0)
        return FWK_E_DATA;

    /*
     * When no sub-elements are defined, there are no sensors to be
     * calibrated.
     */
    if (sub_element_count == 0)
        return FWK_SUCCESS;

    sensor_cfg = group_ctx->sensor_cfg_table;

    /* Perform calibration for each sensor within the group */
    for (sub_elem_ix = 0; sub_elem_ix < (uint8_t)sub_element_count;
         sub_elem_ix++) {
        sensor_ctx = &group_ctx->sensor_ctx_table[sub_elem_ix];
        sensor_cfg = &group_ctx->sensor_cfg_table[sub_elem_ix];

        fwk_expect(sensor_cfg->cal_reg_a != NULL);
        fwk_expect(sensor_cfg->cal_reg_b != NULL);

        calibration_a = *(sensor_cfg->cal_reg_a) + sensor_cfg->offset_cal_reg_a;
        calibration_b = *(sensor_cfg->cal_reg_b) + sensor_cfg->offset_cal_reg_b;

        switch (sensor_cfg->type) {
        case JUNO_PVT_TYPE_TEMP:
            status = process_pvt_calibration(sensor_ctx,
                                             calibration_a,
                                             calibration_b);
            break;

        case JUNO_PVT_TYPE_VOLT:
            status = process_osc_calibration(sensor_ctx,
                                             calibration_a,
                                             calibration_b);
            break;

        default:
            status = FWK_E_PARAM;

            break;
        }

        if (status != FWK_SUCCESS)
            goto error;
    }

    sensor_cfg = group_ctx->sensor_cfg_table;

    status = fwk_notification_subscribe(
                mod_pd_notification_id_power_state_pre_transition,
                sensor_cfg->group->pd_id,
                id);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_notification_subscribe(
                mod_pd_notification_id_power_state_transition,
                sensor_cfg->group->pd_id,
                id);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;

error:
    mod_ctx.driver_is_disabled = true;

    return status;
}

static int pvt_process_event(const struct fwk_event *event,
                             struct fwk_event *resp_event){
    int status = FWK_SUCCESS;
    struct mod_juno_pvt_dev_config *sensor_cfg;
    const struct juno_group_desc *group;
    struct pvt_dev_ctx *group_ctx;
    struct pvt_sub_dev_ctx *sensor_ctx;
    struct mod_sensor_driver_resp_params *isr_params =
        (struct mod_sensor_driver_resp_params *)event->params;
    struct fwk_event resp_notif;
    uint8_t elt_idx = fwk_id_get_element_idx(event->target_id);
    unsigned int sub_elt_idx;
    unsigned int sensor_count;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp_notif.params;

    fwk_assert(fwk_module_is_valid_element_id(event->target_id));

    group_ctx = &dev_ctx[elt_idx];
    sub_elt_idx = fwk_id_get_sub_element_idx(group_ctx->sensor_read_id);
    sensor_ctx = &group_ctx->sensor_ctx_table[sub_elt_idx];

    switch (fwk_id_get_event_idx(event->id)) {

    case JUNO_PVT_EVENT_IDX_READ_REQUEST:
        sensor_cfg = &group_ctx->sensor_cfg_table[sub_elt_idx];

        group = sensor_cfg->group;

        if ((group->regs->GROUP_INFO & PVTGROUP_GROUP_INFO_LOC) !=
            PVTGROUP_GROUP_INFO_LOC_GROUP_LITE)
            return respond(group_ctx);

        sensor_count = (group->regs->GROUP_INFO &
                        PVTGROUP_SENSOR_COUNT_MASK) >> 1;

        if (sensor_count < group->sensor_count)
            return respond(group_ctx);

        /*
         * Configure the group before reading a sensor within it.
         * This must be performed each time because the configuration is lost
         * if the power domain that the group resides in powers off.
         */
        group->regs->IRQ_MASK_CLEAR = IRQ_MASK_ENABLED;
        group->regs->IRQ_MASK_SET = IRQ_MASK_DISABLED;
        group->regs->IRQ_CLEAR = IRQ_MASK_ALL;
        group->regs->SSI_RATE_DIV = 0;

        status = fwk_interrupt_clear_pending(group->irq);
        if (status != FWK_SUCCESS)
            return respond(group_ctx);

        status = fwk_interrupt_set_isr_param(group->irq,
                                             &pvt_interrupt_handler,
                                             (uintptr_t)group_ctx);
        if (status != FWK_SUCCESS)
            return respond(group_ctx);

        status = fwk_interrupt_enable(group->irq);
        if (status != FWK_SUCCESS)
            return respond(group_ctx);

        /* Initiate measurement for group/sensor */
        group->regs->SENSOR_ENABLE = (1 << sensor_cfg->index);
        group->regs->SAMPLE_WINDOW =
            sensor_ctx->sample_window & SAMPLE_WINDOW_MASK;
        group->regs->MEASUREMENT_ENABLE = PVTGROUP_MEASUREMENT_ENABLE;

        return FWK_SUCCESS;

    case JUNO_PVT_EVENT_IDX_DATA_READY:
        if (fwk_id_get_element_idx(group_ctx->sensor_read_id) ==
            fwk_id_get_element_idx(event->target_id)) {
            /* Set the sensor group available */
            group_ctx->sensor_read_id = FWK_ID_NONE;

            group_ctx->driver_response_api->reading_complete(
                sensor_ctx->sensor_hal_id,
                isr_params);

            /* Respond to the Power Domain notification */
            if (group_ctx->pd_notification_delayed) {
                group_ctx->pd_notification_delayed = false;
                status = fwk_thread_get_delayed_response(event->target_id,
                                                         group_ctx->cookie,
                                                         &resp_notif);
                if (status != FWK_SUCCESS)
                    return FWK_E_PANIC;

                pd_resp_params->status = FWK_SUCCESS;

                status = fwk_thread_put_event(&resp_notif);
                if (status != FWK_SUCCESS)
                    return FWK_E_PANIC;
            }

            return FWK_SUCCESS;
        }
        /* Fall-through */

    default:
        return FWK_E_PARAM;
    }
}

static int pvt_process_notification(const struct fwk_event *event,
                                    struct fwk_event *resp_event)
{
    struct pvt_dev_ctx *group_ctx;
    struct mod_pd_power_state_pre_transition_notification_params
        *pre_state_params;
    struct mod_pd_power_state_transition_notification_params
        *post_state_params;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *resp_params;

    group_ctx = &dev_ctx[fwk_id_get_element_idx(event->target_id)];

    if (fwk_id_is_equal(event->id,
                        mod_pd_notification_id_power_state_pre_transition)) {
        pre_state_params =
            (struct mod_pd_power_state_pre_transition_notification_params *)
                event->params;
        if (pre_state_params->target_state == MOD_PD_STATE_OFF)
            group_ctx->pd_state_on = false;

        if (!fwk_id_is_equal(group_ctx->sensor_read_id, FWK_ID_NONE)) {
            /* Read request ongoing, delay the response */
            group_ctx->cookie = event->cookie;
            group_ctx->pd_notification_delayed = true;

            /*
            * The response to Power Domain is delayed so we can process the
            * sensor reading within a defined power state.
            */
            resp_event->is_delayed_response = true;
        } else {
            resp_params =
            (struct
                mod_pd_power_state_pre_transition_notification_resp_params *)
                resp_event->params;

            resp_params->status = FWK_SUCCESS;
        }

    } else if (fwk_id_is_equal(event->id,
                        mod_pd_notification_id_power_state_transition)) {
        post_state_params =
            (struct mod_pd_power_state_transition_notification_params *)
                event->params;
        if (post_state_params->state == MOD_PD_STATE_ON)
            group_ctx->pd_state_on = true;
    } else
        return FWK_E_PARAM;

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_pvt = {
    .name = "Juno PVT Driver",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .event_count = JUNO_PVT_EVENT_IDX_COUNT,
    .init = juno_pvt_init,
    .element_init = juno_pvt_element_init,
    .bind = juno_pvt_bind,
    .process_bind_request = juno_pvt_process_bind_request,
    .start = pvt_start,
    .process_event = pvt_process_event,
    .process_notification = pvt_process_notification,
};
