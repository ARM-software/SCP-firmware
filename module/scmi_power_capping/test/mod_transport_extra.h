/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI power capping unit test support.
 */
#include <mod_transport.h>

#include <fwk_id.h>

int transport_get_fch_address(
    fwk_id_t fch_id,
    struct mod_transport_fast_channel_addr *fch_address);

int transport_get_fch_interrupt_type(
    fwk_id_t fch_id,
    enum mod_transport_fch_interrupt_type *fch_interrupt_type);

int transport_get_fch_doorbell_info(
    fwk_id_t fch_id,
    struct mod_transport_fch_doorbell_info *doorbell_info);

int transport_get_fch_rate_limit(fwk_id_t fch_id, uint32_t *fch_rate_limit);

int transport_fch_register_callback(
    fwk_id_t fch_id,
    uintptr_t param,
    void (*fch_callback)(uintptr_t param));
