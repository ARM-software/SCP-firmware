/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_juno_ppu.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <fwk_dt_config_common.h>

#define PPU_COMPAT arm_juno_ppu
#define PPU_PROP(n, prop) FWK_DT_INST_PROP(PPU_COMPAT, n , prop)

/* init the interrupts to supplied values in config element table */
#define PPU_INTERRUPT_INIT(n) \
    .wakeup_irq = (unsigned int) FWK_DT_INST_IDX_PROP(PPU_COMPAT, n, irq_nums, 0), \
    .wakeup_fiq = (unsigned int) FWK_DT_INST_IDX_PROP(PPU_COMPAT, n, irq_nums, 1), \
    .warm_reset_irq = (unsigned int) FWK_DT_INST_IDX_PROP(PPU_COMPAT, n, irq_nums, 2),

/* zero init the interrupts values in config element table */
#define PPU_INTERRUPT_ZINIT() \
    .wakeup_irq = 0, \
    .wakeup_fiq = 0, \
    .warm_reset_irq = 0,

/* if the interrupts property exists, init to supplied values, else zero them */
#define PPU_INTERRUPT_OPT_INIT(n) \
    COND_CODE_1(DT_NODE_HAS_PROP(DT_INST(n, PPU_COMPAT), irq_nums), \
    (PPU_INTERRUPT_INIT(n)), \
    (PPU_INTERRUPT_ZINIT()) \
    )

/* single table element from an instance */
#define PPU_ELEM_INIT(n) \
    [PPU_PROP(n, elem_idx)] =  { \
        .name = FWK_DT_INST_PROP_DEFAULT(PPU_COMPAT, n, label, ""), \
        .data = &(const struct mod_juno_ppu_element_config) { \
            .reg_base = FWK_DT_INST_IDX_PROP(PPU_COMPAT, n, reg, 0), \
            .timer_id = FWK_DT_PH_OPT_IDX_SCP_ID_ELEM(PPU_COMPAT, n, 0), \
            .pd_type = FWK_DT_INST_PROP_DEFAULT(PPU_COMPAT, n, pd_type, 0), \
            PPU_INTERRUPT_OPT_INIT(n) \
        }, \
    },

/* table generator */
static struct fwk_element element_table[] = {
    /* Macro for array elements */
    DT_FOREACH_OKAY_INST_arm_juno_ppu(PPU_ELEM_INIT)
    /* last null element */
    [DT_N_INST_arm_juno_ppu_NUM_OKAY] = { 0 },
};

/* access data table using function */
const struct fwk_element *_static_get_element_table_juno_ppu() {
    /* 
     * using elem_idx property to bind table array elements to 
     * framework indexes. Check these result in a properly formed table
     * which is not too small for the largest index.
     * This will cause a build error if table is incorrect.
     */
    FWK_DT_BUILD_BUG_ON(!FWK_DT_TABLE_CHECK(element_table, DT_N_INST_arm_juno_ppu_NUM_OKAY + 1));

    return element_table;
}

/* config structure */
struct fwk_module_config config_dt_juno_ppu = {
#ifdef FWK_MODULE_GEN_DYNAMIC_MOCK_PSU
    /* use custom external element callback to allow runtime modification */
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_juno_ppu),
#else
    /* use local access function */
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(_static_get_element_table_juno_ppu),
#endif
};
