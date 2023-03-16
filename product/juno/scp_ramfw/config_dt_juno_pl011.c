/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_dt_config_dyn.h>


/* example dynamic module config table implementation function */
#ifdef FWK_MODULE_GEN_DYNAMIC_PL011
const struct fwk_element *dyn_get_element_table_pl011(fwk_id_t module_id)
{
    /* in this case we are simple going to return the static table */
    return _static_get_element_table_pl011();
}
#endif
