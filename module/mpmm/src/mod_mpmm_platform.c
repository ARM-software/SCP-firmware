/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      MPMM platform-specific functions.
 *      The implementation in this file are weak and returns an error.
 *      The platform code must override these functions with platform-specific
 *      implementation. For traffic cop feature the implementation is not
 *      required.
 */

#include <mod_mpmm.h>

FWK_WEAK int mpmm_core_check_enabled(
    fwk_id_t mpmm_domain_id,
    fwk_id_t core_id,
    bool *enabled)
{
    *enabled = true;
    return FWK_E_SUPPORT;
}

FWK_WEAK int mpmm_core_set_threshold(
    fwk_id_t mpmm_domain_id,
    fwk_id_t core_id,
    uint32_t threshold)
{
    return FWK_E_SUPPORT;
}

FWK_WEAK int mpmm_core_counter_read(
    fwk_id_t mpmm_domain_id,
    fwk_id_t core_id,
    uint32_t counter_idx,
    uint32_t *counter_value)
{
    *counter_value = 0;
    return FWK_E_SUPPORT;
}
