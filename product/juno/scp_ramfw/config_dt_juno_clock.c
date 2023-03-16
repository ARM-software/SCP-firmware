/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_clock.h"
#include "config_power_domain.h"
#include "juno_clock.h"
#include "juno_id.h"

#include <mod_clock.h>
#include <mod_juno_cdcel937.h>
#include <mod_juno_hdlcd.h>
#include <mod_juno_soc_clock_ram.h>
#include <mod_mock_clock.h>
#include <mod_power_domain.h>

#include <fwk_dt_config_dyn.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>


#ifdef FWK_MODULE_GEN_DYNAMIC_CLOCK
const struct fwk_element *dyn_get_element_table_clock(fwk_id_t module_id)
{
    int status;
    enum juno_idx_platform platform_id = JUNO_IDX_PLATFORM_COUNT;
    struct mod_clock_dev_config *dev_cfg;
    const struct fwk_element *clock_dev_desc_table;

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return NULL;
    }

    /*
     * When running under an FVP the I2S clock does not fully function,
     * as it requires the I2C hardware interrupt to properly work. This
     * is replaced with the mock clock.
     *
     * Referencing and dereferencing is required to bypass the const qualifier.
     */
    clock_dev_desc_table = _static_get_element_table_clock();
    dev_cfg = (struct mod_clock_dev_config
                   *)(clock_dev_desc_table[JUNO_CLOCK_IDX_I2SCLK].data);
    if (platform_id == JUNO_IDX_PLATFORM_FVP) {
        *((fwk_id_t *)(&dev_cfg->api_id)) = (fwk_id_t)FWK_ID_API_INIT(
            FWK_MODULE_IDX_MOCK_CLOCK, MOD_MOCK_CLOCK_API_TYPE_DRIVER);
        *((fwk_id_t *)(&dev_cfg->driver_id)) = (fwk_id_t)FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_MOCK_CLOCK, MOD_MOCK_CLOCK_ELEMENT_IDX_I2S);
    }
    return clock_dev_desc_table;
}
#endif
