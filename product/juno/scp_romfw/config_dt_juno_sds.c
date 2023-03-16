/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_id.h"
#include "juno_scc.h"
#include "juno_sds.h"
#include "juno_ssc.h"
#include "scp_config.h"
#include "software_mmap.h"

#include <mod_sds.h>

#include <fwk_assert.h>
#include <fwk_dt_config_common.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const uint32_t version_packed = FWK_BUILD_VERSION;
static struct juno_sds_platid platid;
static uint32_t reset_syndrome;
static uint32_t primary_cpu;

#ifdef FWK_MODULE_GEN_DYNAMIC_SDS
const struct fwk_element *dyn_get_element_table_sds(fwk_id_t module_id)
{
    int status;
    enum juno_idx_platform platform_id = JUNO_IDX_PLATFORM_COUNT;
    const struct fwk_element *sds_elem_table = _static_get_element_table_sds();
    struct mod_sds_structure_desc *sds_struct_desc;

    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");

    platid.platform_identifier = SSC->SSC_VERSION;

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return NULL;
    }
    platid.platform_type_identifier = (uint32_t)platform_id;

    /*
     * Make the reset syndrome available to the AP.
     *
     * On some platforms this is simply a case of copying from the
     * RESET_SYNDROME register but on Juno this is complicated by a hardware
     * defect which prevents the WDOGRESET_SCP and WDOGRESET_SYS bits [2:1] from
     * being set correctly when a watchdog reset occurs. To work around this
     * defect, the MCC on Juno is able to report whether the reset was caused by
     * a watchdog event using the RESET field in the GPR1.
     *
     * If the reset was caused by any of the watchdogs, the SCP firmware will
     * set the relevant bit (using the RESET_SYNDROME register layout) when
     * sharing the RESET_SYNDROME value with Trusted Firmware.
     */

    /* Start with the original reset syndrome */
    reset_syndrome = SCP_CONFIG->RESET_SYNDROME;

    /*
     * Adjust values in case MCC is reporting the reset was caused by one of the
     * watchdogs.
     */
    if ((SCC->GPR1 & SCC_GPR1_RESET) == SCC_GPR1_RESET_WDOG) {
        reset_syndrome &= ~SCP_CONFIG_RESET_SYNDROME_PORESET;
        reset_syndrome |= SCP_CONFIG_RESET_SYNDROME_WDOGRESET;
    }

    if (SCC->GPR1 & SCC_GPR1_BOOT_MAP_ENABLE) {
        primary_cpu =
            (SCC->GPR1 & SCC_GPR1_PRIMARY_CPU) >> SCC_GPR1_PRIMARY_CPU_POS;
    }

    /* override payload elements needed for Juno */
    sds_struct_desc = (struct mod_sds_structure_desc *)sds_elem_table[JUNO_SDS_ROM_VERSION_IDX].data;
    sds_struct_desc->payload = &version_packed;
    sds_struct_desc = (struct mod_sds_structure_desc *)sds_elem_table[JUNO_SDS_ROM_CPU_INFO_IDX].data;
    sds_struct_desc->payload = &primary_cpu;
    sds_struct_desc = (struct mod_sds_structure_desc *)sds_elem_table[JUNO_SDS_ROM_RESET_SYNDROME_IDX].data;
    sds_struct_desc->payload = &reset_syndrome;
    sds_struct_desc = (struct mod_sds_structure_desc *)sds_elem_table[JUNO_SDS_ROM_PLATFORM_ID_IDX].data;
    sds_struct_desc->payload = &platid;

    return sds_elem_table;
}
#else
#error("Error: Juno scp_romfw must use dynamic dts module access to override values");
#endif

