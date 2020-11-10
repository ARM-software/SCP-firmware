/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rcar_irq.h>
#include <config_rcar_pd_pmic_bd9571.h>
#include <config_power_domain.h>

#include <mod_rcar_pd_pmic_bd9571.h>

#include <fwk_element.h>
#include <fwk_module.h>

static struct fwk_element rcar_pd_pmic_element_table[] = {
    [RCAR_PD_PMIC_ELEMENT_IDX_PMIC_DDR_BKUP] = {
        .name = "pmic_ddr_bkup",
        .data = &((struct mod_rcar_pd_pmic_config) {
            .pd_type = RCAR_PD_TYPE_DEVICE,
            .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PMIC,
                RCAR_PMIC_CPU_DDR_BKUP),
            .always_on = false,
        }),
    },
    [RCAR_PD_PMIC_ELEMENT_IDX_COUNT] = { 0 }, /* Termination entry */
};


static const struct fwk_element *rcar_pd_pmic_get_element_table(fwk_id_t mod)
{
    return rcar_pd_pmic_element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_rcar_pd_pmic_bd9571 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_pd_pmic_get_element_table),
};
