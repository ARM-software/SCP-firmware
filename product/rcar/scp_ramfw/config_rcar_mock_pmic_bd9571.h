/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_RCAR_MOCK_PMIC_H
#define CONFIG_RCAR_MOCK_PMIC_H

/* Element indexes for rcar PMIC */
enum mod_rcar_mock_pmic_element_idx {
    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_LITTLE,
    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_BIG,
    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_PMIC_DDR_BKUP,
    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_GPU,
    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_COUNT
};

#endif /* CONFIG_RCAR_MOCK_PMIC_BD9571_H */
