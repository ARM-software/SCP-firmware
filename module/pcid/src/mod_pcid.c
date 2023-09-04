/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_pcid.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

bool mod_pcid_check_registers(
    const struct mod_pcid_registers *registers,
    const struct mod_pcid_registers *expected,
    uint32_t valid_pcid_registers)
{
    FWK_R uint32_t *pcid_reg, *pcid_expt;

    fwk_assert(registers != NULL);
    fwk_assert(expected != NULL);

    if (valid_pcid_registers == MOD_PCID_REGISTER_ALL) {
        return !memcmp(registers, expected, sizeof(*registers));
    } else {
        valid_pcid_registers = valid_pcid_registers & MOD_PCID_REGISTER_ALL;
        pcid_reg = &registers->PID4;
        pcid_expt = &expected->PID4;

        while (valid_pcid_registers) {
            if ((valid_pcid_registers & 1) == PCID_REG_VALID) {
                if (*pcid_reg++ != *pcid_expt++) {
                    return false;
                }
            }
            valid_pcid_registers >>= 1;
        }
        return true;
    }
}

static int pcid_init(fwk_id_t module_id,
                     unsigned int element_count,
                     const void *data)
{
    return FWK_SUCCESS;
}

const struct fwk_module_config config_pcid = { 0 };
const struct fwk_module module_pcid = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = pcid_init,
};
