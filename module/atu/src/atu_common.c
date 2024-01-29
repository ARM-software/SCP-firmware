/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Common utility functions for ATU driver.
 */

#include <internal/atu.h>
#include <internal/atu_common.h>

#include <mod_atu.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_status.h>

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
/* Output bus attribute names */
static const char *atu_roba_name[MOD_ATU_ROBA_COUNT] = {
    [MOD_ATU_ROBA_AXNSE] = "NSE",       [MOD_ATU_ROBA_AXCACHE3] = "CACHE3",
    [MOD_ATU_ROBA_AXCACHE2] = "CACHE2", [MOD_ATU_ROBA_AXCACHE1] = "CACHE1",
    [MOD_ATU_ROBA_AXCACHE0] = "CACHE0", [MOD_ATU_ROBA_AXPROT2] = "PROT2",
    [MOD_ATU_ROBA_AXPROT1] = "PROT1",   [MOD_ATU_ROBA_AXPROT0] = "PROT0",
};
#endif

int atu_validate_region_attributes(uint32_t attributes)
{
    uint8_t offset;
    uint32_t attribute_value;

    /* Validate output bus attributes */
    for (int i = MOD_ATU_ROBA_AXPROT0; i < MOD_ATU_ROBA_COUNT; i++) {
        /*
         * Increment offset value by 2 as each attribute is represented by 2
         * bits in the ATUROBA register.
         */
        offset = (i * 2);
        attribute_value = (attributes >> offset) & ATU_ROBA_ATTRIBUTE_MASK;

        /* Check if the attribute is being set to reserved value */
        if (attribute_value == MOD_ATU_ROBA_RESERVED) {
            FWK_LOG_ERR(
                "[ATU] Error! %s cannot be set to reserved"
                "value",
                atu_roba_name[i]);
            return FWK_E_PARAM;
        }

        /* Check if AxNSE is being set to passthrough */
        if ((offset == ATU_ATUROBA_AXNSE_OFFSET) &&
            (attribute_value == MOD_ATU_ROBA_PASSTHROUGH)) {
            FWK_LOG_ERR(
                "[ATU] Error! AxNSE attribute cannot be set to"
                "passthrough");
            return FWK_E_PARAM;
        }
    }

    return FWK_SUCCESS;
}
