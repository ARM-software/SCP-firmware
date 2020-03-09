/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "config_sensor.h"
#include "juno_id.h"
#include "juno_irq.h"
#include "juno_pvt.h"
#include "pvt_sensor_calibration.h"

#include <mod_juno_pvt.h>
#include <mod_sensor.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/* Constants for additional calibration on Juno R0 */
#define CAL_45_JUNO_R0      455
#define CAL_85_JUNO_R0      (CAL_45_JUNO_R0 + 1520)

static struct juno_group_desc sensor_group[] = {
    [JUNO_PVT_GROUP_BIG] = {
        .regs = PVT_REG_BIG,
        .irq = BIG_PVT_IRQ,
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                     POWER_DOMAIN_IDX_BIG_SSTOP),
    },
    [JUNO_PVT_GROUP_LITTLE] = {
        .regs = PVT_REG_LITTLE,
        .irq = LITTLE_PVT_IRQ,
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                     POWER_DOMAIN_IDX_LITTLE_SSTOP),
    },
    [JUNO_PVT_GROUP_GPU] = {
        .regs = PVT_REG_GPU,
        .irq = GPU_PVT_IRQ,
        .sensor_count = 2,
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                     POWER_DOMAIN_IDX_GPUTOP),
    },
    [JUNO_PVT_GROUP_SOC] = {
        .regs = PVT_REG_SOC,
        .irq = SOC_PVT_IRQ,
        .sensor_count = 1,
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                     POWER_DOMAIN_IDX_SYSTOP),
    },
    [JUNO_PVT_GROUP_STDCELL] = {
        .regs = PVT_REG_STDCELL,
        .irq = STD_CELL_PVT_IRQ,
        .sensor_count = 1,
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                     POWER_DOMAIN_IDX_SYSTOP),
    },
};

static struct mod_sensor_info info_pvt_temp = {
    .type = MOD_SENSOR_TYPE_DEGREES_C,
    .update_interval = 0,
    .update_interval_multiplier = 0,
    .unit_multiplier = 0,
};

static struct mod_sensor_info info_pvt_volt = {
    .type = MOD_SENSOR_TYPE_VOLTS,
    .update_interval = 0,
    .update_interval_multiplier = 0,
    .unit_multiplier = 0,
};

static const struct mod_juno_pvt_dev_config dev_config_big[] = {
    [0] = {
        .group = &sensor_group[JUNO_PVT_GROUP_BIG],
        .index = 1,
        .type = JUNO_PVT_TYPE_VOLT,
        .info = &info_pvt_volt,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G3_S1_810MV_85C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G3_S1_900MV_85C,
    },
    [1] = {
        .group = &sensor_group[JUNO_PVT_GROUP_BIG],
        .index = 0,
        .type = JUNO_PVT_TYPE_TEMP,
        .info = &info_pvt_temp,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G3_S0_900MV_45C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G3_S0_900MV_85C,
    },
};

static const struct mod_juno_pvt_dev_config dev_config_little[] = {
    [0] = {
        .group = &sensor_group[JUNO_PVT_GROUP_LITTLE],
        .index = 1,
        .type = JUNO_PVT_TYPE_VOLT,
        .info = &info_pvt_volt,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G4_S1_810MV_85C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G4_S1_900MV_85C,
    },
    [1] = {
        .group = &sensor_group[JUNO_PVT_GROUP_LITTLE],
        .index = 0,
        .type = JUNO_PVT_TYPE_TEMP,
        .info = &info_pvt_temp,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G4_S0_900MV_45C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G4_S0_900MV_85C,
    },
};

static const struct mod_juno_pvt_dev_config dev_config_gpu[] = {
    [0] = {
        .group = &sensor_group[JUNO_PVT_GROUP_GPU],
        .index = 0,
        .type = JUNO_PVT_TYPE_TEMP,
        .info = &info_pvt_temp,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G2_S0_900MV_45C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G2_S0_900MV_85C,
    },
    [1] = {
        .group = &sensor_group[JUNO_PVT_GROUP_GPU],
        .index = 1,
        .type = JUNO_PVT_TYPE_TEMP,
        .info = &info_pvt_temp,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G2_S1_900MV_45C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G2_S1_900MV_85C,
    }
};

static const struct mod_juno_pvt_dev_config dev_config_soc_r1_r2[] = {
    [0] = {
        .group = &sensor_group[JUNO_PVT_GROUP_SOC],
        .index = 0,
        .type = JUNO_PVT_TYPE_TEMP,
        .info = &info_pvt_temp,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G1_S0_900MV_45C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G1_S0_900MV_85C,
    },
};

static const struct mod_juno_pvt_dev_config dev_config_soc_r0[] = {
    [0] = {
        .group = &sensor_group[JUNO_PVT_GROUP_SOC],
        .index = 0,
        .type = JUNO_PVT_TYPE_TEMP,
        .info = &info_pvt_temp,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G1_S0_900MV_45C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G1_S0_900MV_45C,
        .offset_cal_reg_a = CAL_45_JUNO_R0,
        .offset_cal_reg_b = CAL_85_JUNO_R0,
    },
};

static const struct mod_juno_pvt_dev_config dev_config_stdcell[] = {
    [0] = {
        .group = &sensor_group[JUNO_PVT_GROUP_STDCELL],
        .index = 1,
        .type = JUNO_PVT_TYPE_VOLT,
        .info = &info_pvt_volt,
        .cal_reg_a = (uint16_t *)&JUNO_PVT_CALIBRATION->G0_S1_810MV_85C,
        .cal_reg_b = (uint16_t *)&JUNO_PVT_CALIBRATION->G0_S1_900MV_85C,
    },
};

#if USE_FULL_SET_SENSORS
static const struct fwk_element pvt_juno_element_table_r1_r2[] = {
    [JUNO_PVT_GROUP_BIG] = {
        .name = "",
        .data = &dev_config_big,
        .sub_element_count = 2,
    },
    [JUNO_PVT_GROUP_LITTLE] = {
        .name = "",
        .data = &dev_config_little,
        .sub_element_count = 2,
    },
    [JUNO_PVT_GROUP_GPU] = {
        .name = "",
        .data = &dev_config_gpu,
        .sub_element_count = 2,
    },
    [JUNO_PVT_GROUP_SOC] = {
        .name = "",
        .data = &dev_config_soc_r1_r2,
        .sub_element_count = 1,
    },
    [JUNO_PVT_GROUP_STDCELL] = {
        .name = "",
        .data = &dev_config_stdcell,
        .sub_element_count = 1,
    },
    [JUNO_PVT_GROUP_COUNT] = { 0 },
};
#endif

static struct fwk_element pvt_juno_element_table[] = {
    [JUNO_PVT_GROUP_BIG] = {
        .name = "",
        .data = &dev_config_big,
        .sub_element_count = 1,
    },
    [JUNO_PVT_GROUP_LITTLE] = {
        .name = "",
        .data = &dev_config_little,
        .sub_element_count = 1,
    },
    [JUNO_PVT_GROUP_GPU] = {
        .name = "",
        .data = &dev_config_gpu,
        .sub_element_count = 0,
    },
    [JUNO_PVT_GROUP_SOC] = {
        .name = "",
        .data = &dev_config_soc_r0,
        .sub_element_count = 1,
    },
    [JUNO_PVT_GROUP_STDCELL] = {
        .name = "",
        .data = &dev_config_stdcell,
        .sub_element_count = 1,
    },
    [JUNO_PVT_GROUP_COUNT] = { 0 },
};

static const struct fwk_element *get_pvt_juno_element_table(fwk_id_t id)
{
    int status;
    enum juno_idx_revision rev;

    status = juno_id_get_revision(&rev);
    if (status != FWK_SUCCESS)
        return NULL;

    #if USE_FULL_SET_SENSORS

    if (rev == JUNO_IDX_REVISION_R0) {
        sensor_group[JUNO_PVT_GROUP_BIG].sensor_count = 1;
        sensor_group[JUNO_PVT_GROUP_LITTLE].sensor_count = 1;

        return pvt_juno_element_table;
    } else {
        sensor_group[JUNO_PVT_GROUP_BIG].sensor_count = 2;
        sensor_group[JUNO_PVT_GROUP_LITTLE].sensor_count = 2;

        return pvt_juno_element_table_r1_r2;
    }
    #else
    sensor_group[JUNO_PVT_GROUP_BIG].sensor_count = 1;
    sensor_group[JUNO_PVT_GROUP_LITTLE].sensor_count = 1;

    if (rev != JUNO_IDX_REVISION_R0)
        pvt_juno_element_table[JUNO_PVT_GROUP_SOC].data = &dev_config_soc_r1_r2;

    return pvt_juno_element_table;
    #endif
}

struct fwk_module_config config_juno_pvt = {
    .get_element_table = get_pvt_juno_element_table,
};
