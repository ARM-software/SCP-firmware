/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_REG_SENSOR_H
#define MOD_RCAR_REG_SENSOR_H

#include <utils_def.h>

#include <mod_sensor.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARRegSensor Register Sensor Driver
 *
 * \brief Driver for simple, register-based sensors.
 * @{
 */

/*! \brief Element configuration */
struct mod_reg_sensor_dev_config {
    /*! Address of the sensor register */
    uintptr_t reg;

    /*! Auxiliary sensor information */
    struct mod_sensor_info *info;
};

/*!
 * @cond
 */

/*!
 * \brief APIs provided by the driver.
 */
enum mod_rcar_reg_sensor_api_type {
    MOD_RCAR_REG_SENSOR_API_TYPE_PUBLIC,
    MOD_RCAR_REG_SENSOR_API_TYPE_SYSTEM,
    MOD_RCAR_REG_SENSOR_API_COUNT,
};

/* Register base */
#define GEN3_THERMAL_BASE (PERIPHERAL_BASE + 0x198000)
#define GEN3_THERMAL_OFFSET (0x8000)
/* Register offsets */
#define REG_GEN3_IRQSTR (0x04)
#define REG_GEN3_IRQMSK (0x08)
#define REG_GEN3_IRQCTL (0x0C)
#define REG_GEN3_IRQEN (0x10)
#define REG_GEN3_IRQTEMP1 (0x14)
#define REG_GEN3_IRQTEMP2 (0x18)
#define REG_GEN3_IRQTEMP3 (0x1C)
#define REG_GEN3_CTSR (0x20)
#define REG_GEN3_THCTR (0x20)
#define REG_GEN3_TEMP (0x28)
#define REG_GEN3_THCODE1 (0x50)
#define REG_GEN3_THCODE2 (0x54)
#define REG_GEN3_THCODE3 (0x58)
#define REG_GEN3_PTAT1 (0x5C)
#define REG_GEN3_PTAT2 (0x60)
#define REG_GEN3_PTAT3 (0x64)
#define REG_GEN3_THSCP (0x68)
#define NEXT_REG_OFFSET (4)

/* THCTR bits */
#define THCTR_PONM BIT(6)
#define THCTR_THSST BIT(0)

#define CTEMP_MASK (0xFFF)
#define MCELSIUS(temp) ((temp)*1000)

#define AVAILABLE_HARDWARE_PARAM \
    (mmio_read_32(GEN3_THERMAL_BASE + REG_GEN3_THSCP) == (0x03 << 14))

/* Structure for thermal temperature calculation */
struct equation_coefs {
    int a1;
    int b1;
    int a2;
    int b2;
};

struct rcar_gen3_thermal_tsc {
    uintptr_t base;
    struct thermal_zone_device *zone;
    struct equation_coefs coef;
    int low;
    int high;
    int tj_t;
    int id; /* thermal channel id */
};

/*
 * Linear approximation for temperature
 *
 * [reg] = [temp] * a + b => [temp] = ([reg] - b) / a
 *
 * The constants a and b are calculated using two triplets of int values PTAT
 * and THCODE. PTAT and THCODE can either be read from hardware or use hard
 * coded values from driver. The formula to calculate a and b are taken from
 * BSP and sparsely documented and understood.
 *
 * Examining the linear formula and the formula used to calculate constants a
 * and b while knowing that the span for PTAT and THCODE values are between
 * 0x000 and 0xfff the largest integer possible is 0xfff * 0xfff == 0xffe001.
 * Integer also needs to be signed so that leaves 7 bits for binary
 * fixed point scaling.
 */
#define DIV_ROUND_CLOSEST(x, divisor) \
    ({ \
        __typeof(x) __x = x; \
        __typeof(divisor) __d = divisor; \
        (((__typeof(x)) - 1) > 0 || ((__typeof(divisor)) - 1) > 0 || \
         (__x) > 0) ? \
            (((__x) + ((__d) / 2)) / (__d)) : \
            (((__x) - ((__d) / 2)) / (__d)); \
    })
#define FIXPT_SHIFT (7)
#define FIXPT_INT(_x) ((_x) * (1 << FIXPT_SHIFT))
#define INT_FIXPT(_x) ((_x) >> FIXPT_SHIFT)
#define FIXPT_DIV(_a, _b) DIV_ROUND_CLOSEST(((_a) * (1 << FIXPT_SHIFT)), (_b))
#define FIXPT_TO_MCELSIUS(_x) ((_x)*1000 >> FIXPT_SHIFT)

#define RCAR3_THERMAL_GRAN (500) /* mili Celsius */

#define TSC_MAX_NUM (3)
#define TSC_PARAM_NUM (3)
#define TEMP_UPPER_LIMIT (125)
#define TEMP_LOWER_LIMIT (-40)

/* no idea where these constants come from */
#define TJ_1 (116)
#define TJ_3 (-41)

#define SENSOR_ADR_BASE (SENSOR_SOC_TEMP1 & SENSOR_SOC_TEMP2 & SENSOR_SOC_TEMP3)
#define SENSOR_ADR_MASK \
    ((SENSOR_SOC_TEMP1 | SENSOR_SOC_TEMP2 | SENSOR_SOC_TEMP3) - SENSOR_ADR_BASE)
#define IS_SENSOR_ADR(adr) ((adr & ~SENSOR_ADR_MASK) == SENSOR_ADR_BASE)
#define CV_ADR2INDEX(adr) (int)(((adr & SENSOR_ADR_MASK) >> 15) - 3)
#define ADR2INDEX(adr) (IS_SENSOR_ADR(adr) ? CV_ADR2INDEX(adr) : (-1))

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_REG_SENSOR_H */
