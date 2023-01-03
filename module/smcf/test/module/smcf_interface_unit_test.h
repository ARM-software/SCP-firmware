/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMCF_INTERFACE_UNIT_TEST_H
#define SMCF_INTERFACE_UNIT_TEST_H

#define MODE_REG_COUNT          3
#define MODE_REG_COUNT_HW_ERROR 5

static const fwk_id_t fwk_module_id_smcf =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SMCF);
static const fwk_id_t fwk_module_id_sensor =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SENSOR);

enum mgi_domains { MGI_IDX_0, MGI_DOMAIN_COUNT };

enum mli_sub_elements_mgi0 {
    MGI0_MLI_IDX_0,
    MGI0_MLI_IDX_1,
    MGI0_MLI_COUNT,
};

static const fwk_id_t mgi_0_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SMCF, MGI_IDX_0);

static struct mod_smcf_element_config config = { .reg_base = 0xFFFF0000 };

#endif /* SMCF_INTERFACE_UNIT_TEST_H */
